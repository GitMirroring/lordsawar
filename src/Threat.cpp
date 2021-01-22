// Copyright (C) 2004 John Farrell
// Copyright (C) 2004, 2005 Ulf Lorenz
// Copyright (C) 2005 Andrea Paternesi
// Copyright (C) 2007, 2008, 2009, 2010, 2014, 2015, 2020, 2021 Ben Asselstine
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

#include "Threat.h"
#include "stackreflist.h"
#include <iostream>
#include "city.h"
#include "ruin.h"
#include "stack.h"
#include "playerlist.h"
#include "AI_Analysis.h"
#include "player.h"

Threat::Threat (const Threat &t)
 : OwnerId (t), d_city (t.d_city), d_ruin (t.d_ruin),
    d_stacks (new StackReflist ()), d_danger (t.d_danger), d_value (t.d_value),
    d_strength (t.d_strength)

{
  for (StackReflist::const_iterator i = t.d_stacks->begin ();
       i != t.d_stacks->end (); ++i)
    d_stacks->addStack (new Stack (*(*i)));
}

Threat::Threat(City *c)
    :OwnerId(*c), d_city(c), d_ruin(0), d_danger(0), d_value(0), d_strength(0)
{
    d_stacks = new StackReflist();
    calculateStrength();
}

Threat::Threat(Stack *s)
    :OwnerId(*s), d_city(0), d_ruin(0), d_danger(0), d_value(0), d_strength(0)
{
    d_stacks = new StackReflist();
    d_stacks->addStack(new Stack(*s));
    d_strength += AI_Analysis::assessStackStrength(s);
}

Threat::Threat(Ruin *r)
    :OwnerId((Player *)0), d_city(0), d_ruin(r), d_danger(0), d_value(0), d_strength(0)
{
    d_stacks = new StackReflist();
}

Threat::~Threat()
{
  for (StackReflist::iterator i = d_stacks->begin(); i != d_stacks->end(); ++i)
    delete *i;
  d_stacks->clear();
  delete d_stacks;
}

Glib::ustring Threat::toString() const
{
    if (d_city)
    {
        return d_city->getName() + " owned by " + getOwner ()->getName();
    }
    else if (d_ruin)
    {
        return d_ruin->getName();
    }
    else
    {
        return "stack owned by " + getOwner ()->getName();
    }
}

bool Threat::Near(Vector<int> pos, Player *p) const
{
    if (p != getOwner ())
        return false;

    if (d_city)
    {
        return d_city->contains(pos);
    }
    else if (d_ruin)
    {
        return d_ruin->contains(pos);
    }
    else
        for (StackReflist::const_iterator it = d_stacks->begin();
            it != d_stacks->end(); ++it)
        {
            Vector<int> spos = (*it)->getPos();
            if (abs(pos.x - spos.x) <= 1 && abs(pos.y - spos.y) <= 1)
                return true;
        }

    return false;
}

void Threat::addStack(Stack *stack)
{
    d_stacks->addStack(new Stack (*stack));
    if (d_city && d_city->getOwner() != Playerlist::getInstance()->getNeutral())
      d_strength += AI_Analysis::assessStackStrength(stack);
}

// this is the strength of the threat to us
void Threat::calculateStrength()
{
  // neutral cities poses a small threat
  if (d_city)
    if (d_city->getOwner() == Playerlist::getInstance()->getNeutral())
      {
        d_strength = 0.3;
        return;
      }
  float score = 0.0;
  for (StackReflist::const_iterator i = d_stacks->begin(); 
       i != d_stacks->end(); ++i)
    {
      guint32 id = 0;
      if (d_stacks->getIdOfStack(*i, id) == false)
        continue;
        //FIXME: why can't this id be found? find out why.
        //it happens when we switch from computer to human and then fight a 
        //city.
      score += AI_Analysis::assessStackStrength(*i);
    }

  d_strength = score;
}

void Threat::calculateValue()
{
  float score = 0.0;
  score += d_danger;
  d_value = score;
}

Vector<int> Threat::getClosestPoint(Vector<int> location) const
{
  Vector<int> result(-1,-1);
  if (d_city)
    result = d_city->getNearestPos(location);
  else if (d_ruin)
    result = d_ruin->getPos();
  else
    {
      int min_dist = -1;
      for (StackReflist::const_iterator it = d_stacks->begin();
           it != d_stacks->end(); ++it)
        {
          Vector<int> spos = (*it)->getPos();

          int distance = dist(spos, location);
          if (distance < min_dist || min_dist == -1)
            {
              result = spos;
              min_dist = distance;
            }
        }
    }

  return result;
}

void Threat::deleteStack(guint32 id)
{
  Stack *s = d_stacks->getStackById(id);
  d_stacks->removeStack(id);
  delete s;
  if (d_city && d_city->getOwner() != Playerlist::getInstance()->getNeutral())
    calculateStrength();
}

void Threat::deleteStack(Stack* s)
{
  Stack *ss = d_stacks->getStackById(s->getId());
  d_stacks->removeStack(s->getId());
  delete ss;
  if (d_city && d_city->getOwner() != Playerlist::getInstance()->getNeutral())
    calculateStrength();
}

void Threat::addDanger(float danger)
{ 
  d_danger += danger; 
  calculateValue();
}
        
void Threat::changeOwnership(Player *old_owner, Player *new_owner)
{
    if (getOwner() == old_owner)
      setOwner(new_owner);
    if (d_stacks)
      d_stacks->changeOwnership(new_owner);
}
