// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2000, 2001, 2002, 2004, 2005 Ulf Lorenz
// Copyright (C) 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009, 2014, 2020 Ben Asselstine
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Library General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#include "LocationBox.h"
#include "army.h"
#include "player.h"
#include "playerlist.h"
#include "stacklist.h"
#include "stack.h"
#include "FogMap.h"
#include "GameMap.h"
#include "stacktile.h"

#include "xmlhelper.h"

LocationBox::LocationBox(Vector<int> pos, guint32 size)
    :Immovable(pos), d_size(size)
{
}

LocationBox::LocationBox(Vector<int> src, Vector<int> dest)
    :Immovable(getTopLeftBoundingBox (src, dest))
{
  if (dest.x > src.x)
    d_size =  dest.x - src.x + 1;
  else if (dest.x < src.x)
    d_size =  src.x - dest.x + 1;
  else
    {
      if (dest.y > src.y)
	d_size =  dest.y - src.y + 1;
      else
	d_size =  src.y - dest.y + 1;
    }
}

LocationBox::LocationBox(const LocationBox& loc)
  : Immovable(loc), d_size(loc.d_size)
{
}

LocationBox::LocationBox(const LocationBox& loc, Vector<int> pos)
  : Immovable(pos), d_size(loc.d_size)
{
}

LocationBox::LocationBox(XML_Helper* helper, guint32 size)
    :Immovable(helper), d_size (size)
{
}

Stack *LocationBox::addArmy(Army *a) const
{
  Vector<int> pos = Vector<int>(-1,-1);
  Stack* stack = getFreeStack(a->getOwner(), pos);

  //no stacks with enough room for one more army, found lets create one.
  if (!stack)
    {
      // No stack found in the entire location
      if (pos == Vector<int>(-1,-1))
	return NULL;
	    
      Player *p = a->getOwner();
      stack = new Stack(p, pos);
      stack->add(a);
      p->addStack(stack);
    }
  else
    stack->add(a);

  if (stack->size() > 1)
    stack->sortForViewing(true);
      
  stack->setDefending(false);
  stack->setParked(false);
  return stack;
}

bool LocationBox::isFull(Player *p) const
{
  for (unsigned int i = 0; i < d_size; i++)
    for (unsigned int j = 0; j < d_size; j++)
      {
	Vector<int> pos = getPos() + Vector<int>(j,i);
	StackTile *stile = GameMap::getInstance()->getTile(pos)->getStacks();
	if (stile->canAdd(1, p) == true)
	  return false;
      }
  return true;
}

Stack* LocationBox::getFreeStack(Player *p, Vector<int> &tile) const
{
  for (unsigned int i = 0; i < d_size; i++)
    for (unsigned int j = 0; j < d_size; j++)
      {
	Vector<int> pos = getPos() + Vector<int>(j,i);
	if (GameMap::canAddArmy(pos) == false)
	  continue;
	StackTile *stile = GameMap::getInstance()->getTile(pos)->getStacks();
	Stack *stack = stile->getFriendlyStack(p);
	if (stack == NULL)
	  {
	    tile = pos;
	    return NULL;
	  }
	else 
	  {
	    Stack *enemy = stile->getEnemyStack(p);
	    if (!enemy && stack->isFull() == false)
	      return stack;
	  }
        }
  tile = Vector<int>(-1,-1);
  return NULL;
}

bool LocationBox::isVisible(Player *player) const
{
  for (unsigned int i = 0; i < d_size; i++)
    for (unsigned int j = 0; j < d_size; j++)
      {
        Vector<int> pos;
        pos.x = getPos().x + i;
        pos.y = getPos().y + j;
	if (FogMap::isClear(pos, player))
	  return true;
      }
  return false;
}

void LocationBox::deFog() const
{
  Player *p = Playerlist::getActiveplayer();
  if (!p)
    return;
  FogMap *fogmap = p->getFogMap();
  fogmap->alterFogRadius (getPos(), 3, FogMap::OPEN);
}

void LocationBox::deFog(Player *p) const
{
  if (!p)
    return;
  FogMap *fogmap = p->getFogMap();
  if (!fogmap)
    return;
  fogmap->alterFogRadius (getPos(), 3, FogMap::OPEN);
}

bool LocationBox::contains(Vector<int> pos) const
{
    return (pos.x >= getPos().x) && (pos.x < getPos().x + (int) d_size) 
      && (pos.y >= getPos().y) && (pos.y < getPos().y + (int) d_size);
}

Vector<int> LocationBox::getNearestPos(Movable *m) const
{
  return getNearestPos(m->getPos());
}

Vector<int> LocationBox::getNearestPos(Vector<int> pos) const
{
  int min_dist = -1;
  Vector<int> closest_tile = Vector<int>(-1,-1);
  for (unsigned int i = 0; i < d_size; i++)
    for (unsigned int j = 0; j < d_size; j++)
      {
        Vector<int> target = Vector<int>(i,j) + getPos();
        int d = dist(pos, target);
        if (d < min_dist || min_dist == -1)
          {
            min_dist = d;
            closest_tile = target;
          }
      }
  return closest_tile;
}

Vector<int> LocationBox::getTopLeftBoundingBox(Vector<int> p1, Vector<int> p2)
{
  return Vector<int>(std::min (p1.x, p2.x), std::min (p1.y, p2.y));
}
