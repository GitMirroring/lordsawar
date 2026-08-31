// Copyright (C) 2009, 2010, 2014, 2015 Ben Asselstine
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
#include "stackreflist.h"
#include "stack.h"
#include "stacklist.h"
#include "player.h"

StackReflist::StackReflist()
{
}

StackReflist::StackReflist(Stacklist *sl, bool skip_parked_stacks)
{
  for (Stacklist::iterator it = sl->begin(); it != sl->end(); ++it)
    {
      if (skip_parked_stacks == true && (*it)->getParked() == true)
        continue;
      addStack(*it);
    }
}

StackReflist::StackReflist(std::list<Stack*> s, bool skip_parked_stacks)
{
  for (std::list<Stack*>::iterator it = s.begin(); it != s.end(); ++it)
    {
      if (skip_parked_stacks == true && (*it)->getParked() == true)
        continue;
      addStack(*it);
    }
}

Stack *StackReflist::getStackById(guint32 id) const
{
  IdMap::const_iterator it = d_id.find(id);
  if (it != d_id.end())
    return (*it).second;
  else
    return NULL;
}

void StackReflist::addStack(Stack *stack)
{
  push_back(stack);
  d_id[stack->getId()] = stack;
}

bool StackReflist::contains(guint32 id) const
{
  if (getStackById(id) != NULL)
    return true;
  return false;
}

bool StackReflist::removeStack(guint32 id)
{
  Stack *s = getStackById(id);
  if (s)
    {
      d_id.erase(d_id.find(s->getId()));
      remove(s);
      return true;
    }
  return false;
}

StackReflist::iterator StackReflist::eraseStack(StackReflist::iterator it)
{
  if (it != end())
    {
      Stack *s = *it;
      if (s)
        {
          IdMap::iterator i = d_id.find(s->getId());
          if (i != d_id.end())
            d_id.erase(i);
        }
    }
  return erase(it);
}

StackReflist::iterator StackReflist::eraseStack(StackReflist::iterator it, guint32 id)
{
  if (it != end())
    {
      IdMap::iterator i = d_id.find(id);
      if (i != d_id.end())
        d_id.erase(i);
    }
  return erase(it);
}

guint32 StackReflist::countArmies() const
{
  guint32 count = 0;
  for (const_iterator it = begin(); it != end(); ++it)
    count += (*it)->size();
  return count;
}

void StackReflist::changeOwnership(Player *new_player)
{
  for (IdMap::iterator it = d_id.begin(); it != d_id.end(); ++it)
    {
      guint32 id = (*it).first;
      Stack *new_stack = new_player->getStacklist()->getStackById(id);
      if (new_stack)
        (*it).second = new_stack;
    }
}

bool StackReflist::getIdOfStack(Stack *stack, guint32 &id)
{
  for (IdMap::iterator it = d_id.begin(); it != d_id.end(); ++it)
    {
      if ((*it).second == stack)
        {
          id = (*it).first;
          return true;
        }
    }
  return false;
}
