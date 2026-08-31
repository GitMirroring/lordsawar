// Copyright (C) 2000, 2001 Michael Bartl
// Copyright (C) 2001, 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2004 John Farrell
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2014, 2020, 2021 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
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

#include <sigc++/functors/mem_fun.h>

#include "ruinlist.h"
#include "xmlhelper.h"
#include "playerlist.h"
#include "reward.h"
#include "GameMap.h"
#include "cityset.h"
#include "citysetlist.h"
#include "keeper.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Glib::ustring Ruinlist::d_tag = "ruinlist";

Ruinlist* Ruinlist::s_instance = 0;

Ruinlist* Ruinlist::getInstance()
{
    if (s_instance == 0)
        s_instance = new Ruinlist();

    return s_instance;
}

Ruinlist* Ruinlist::getInstance(XML_Helper* helper)
{
    if (s_instance)
        deleteInstance();

    s_instance = new Ruinlist(helper);
    return s_instance;
}

void Ruinlist::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

Ruinlist::Ruinlist()
{
}

Ruinlist::~Ruinlist()
{
  for (iterator it = begin(); it != end(); ++it)
    delete *it;
  d_object.clear();
  d_id.clear();
}

Ruinlist::Ruinlist (const Ruinlist &r, bool sync_ids)
 : LocationList<Ruin*> (), sigc::trackable (r)
{
  for (auto ruin : r)
    add (new Ruin (*ruin, sync_ids));
}

Ruinlist::Ruinlist(XML_Helper* helper)
{
    helper->registerTag(Ruin::d_tag, sigc::mem_fun(this, &Ruinlist::load));
}

bool Ruinlist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(Ruinlist::d_tag);

    for (const_iterator it = begin(); it != end(); ++it)
        retval &= (*it)->save(helper);
    
    retval &= helper->closeTag();

    return retval;
}

bool Ruinlist::load(Glib::ustring tag, XML_Helper* helper)
{
    // Shouldn't happen, but one never knows...
    if (tag != Ruin::d_tag)
        return false;

    Cityset *cityset = GameMap::getCityset();
    guint32 width = cityset->getRuinTileWidth();
    add(new Ruin(helper, width));

    //! since the ruin has only now been copied to its final state, we need
    //to register the callback for the occupants here.
    helper->registerTag(Stack::d_tag, sigc::mem_fun(*back(), &Ruin::load));
    // same with rewards in ruins
    helper->registerTag(Reward::d_tag, sigc::mem_fun(*back(), &Ruin::load));

    return true;
}

static bool isHiddenAndNotOwnedByActivePlayer(void *r)
{
  Ruin *ruin = ((Ruin *)r);
  if (ruin->isHidden() == true && 
      ruin->getOwner() != Playerlist::getInstance()->getActiveplayer())
    return true;
  return false;
}

static bool isFogged(void *r)
{
  return ((Ruin*)r)->isVisible(Playerlist::getViewingplayer()) == false;
}

static bool isSearched(void *r)
{
  return ((Ruin*)r)->isSearched();
}

Ruin* Ruinlist::getNearestUnsearchedRuin(const Vector<int>& pos) const
{
  std::list<bool (*)(void *)> filters;
  filters.push_back(isHiddenAndNotOwnedByActivePlayer);
  filters.push_back(isSearched);
  return getNearestObject(pos, &filters);
}
Ruin* Ruinlist::getNearestUnsearchedRuin(const Vector<int>& pos, int dist) const
{
  Ruin *r = getNearestUnsearchedRuin(pos);
  if (!r)
    return NULL;
  if (r->getPos().x <= pos.x + dist && r->getPos().x >= pos.x - dist &&
      r->getPos().y <= pos.y + dist && r->getPos().y >= pos.y - dist)
    return r;
  return NULL;
}

Ruin* Ruinlist::getNearestRuin(const Vector<int>& pos) const
{
  std::list<bool (*)(void *)> filters;
  filters.push_back(isHiddenAndNotOwnedByActivePlayer);
  return getNearestObject(pos, &filters);
}

Ruin* Ruinlist::getNearestRuin(const Vector<int>& pos, int dist) const
{
  Ruin *r = getNearestRuin(pos);
  if (r->getPos().x <= pos.x + dist && r->getPos().x >= pos.x - dist &&
      r->getPos().y <= pos.y + dist && r->getPos().y >= pos.y - dist)
    return r;
  return NULL;
}

Ruin* Ruinlist::getNearestVisibleRuin(const Vector<int>& pos) const
{
  std::list<bool (*)(void *)> filters;
  filters.push_back(isFogged);
  filters.push_back(isHiddenAndNotOwnedByActivePlayer);
  return getNearestObject(pos, &filters);
}

Ruin* Ruinlist::getNearestVisibleRuin(const Vector<int>& pos, int dist) const
{
  Ruin *r = getNearestVisibleRuin(pos);
  if (r->getPos().x <= pos.x + dist && r->getPos().x >= pos.x - dist &&
      r->getPos().y <= pos.y + dist && r->getPos().y >= pos.y - dist)
    return r;
  return NULL;
}

void Ruinlist::changeOwnership(Player *old_owner, Player *new_owner)
{
  for (iterator it = begin(); it != end(); ++it)
    if ((*it)->getOwner() == old_owner)
      (*it)->setOwner(new_owner);
}

guint32 Ruinlist::countUnexploredRuins(Player *owner) const
{
  guint32 count = 0;
  for (const_iterator it = begin(); it != end(); ++it)
    {
      Ruin *ruin = *it;
      if (ruin->isHidden() == true && ruin->getOwner() != owner)
        continue;
      if (ruin->isSearched() == false)
        count++;
    }
  return count;
}

guint32 Ruinlist::countExploredRuins(Player *owner) const
{
  guint32 count = 0;
  for (const_iterator it = begin(); it != end(); ++it)
    {
      Ruin *ruin = *it;
      if (ruin->isHidden() == true && ruin->getOwner() != owner)
        continue;
      if (ruin->isSearched() == true && ruin->getOwner() == owner)
        count++;
    }
  return count;
}

guint32 Ruinlist::countUnamedRuins () const
{
  guint32 count = 0;
  for (const_iterator it = begin (); it != end (); ++it)
    if ((*it)->getName () == DEFAULT_RUIN_NAME)
      count++;
  return count;
}

guint32 Ruinlist::countKeepers () const
{
  guint32 count = 0;
  for (const_iterator it = begin (); it != end (); ++it)
    if ((*it)->getOccupant ())
      {
        if ((*it)->getOccupant ()->getStack ())
          count++;
      }
    else
      count++;
  return count;
}

guint32 Ruinlist::countEmptyKeepers () const
{
  guint32 count = 0;
  for (const_iterator it = begin (); it != end (); ++it)
    if ((*it)->getOccupant ())
      {
        if ((*it)->getOccupant ()->getStack () == NULL)
          count++;
      }
  return count;
}

void Ruinlist::reset (Ruinlist *r)
{
  delete s_instance;
  s_instance = r;
}
