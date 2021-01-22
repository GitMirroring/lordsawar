//  Copyright (C) 2020, 2021 Ben Asselstine
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

#include "keeper.h"

#include "armyproto.h"
#include "playerlist.h"
#include "armysetlist.h"
#include "xmlhelper.h"

Glib::ustring Keeper::d_tag = "keeper";

Keeper::Keeper(const ArmyProto *army, Vector<int> pos)
  : Renamable("")
{
  d_stack = NULL;
  if (army)
    add (army, pos);
}

void Keeper::add (const ArmyProto *army, Vector<int> pos)
{
  Player *neutral = Playerlist::getInstance ()->getNeutral ();
  clearStack ();
  d_stack = new Stack (neutral, pos);
  Army *a = new Army(*army, neutral);
  d_stack->push_back(a);
  rename ();
}

void Keeper::rename ()
{
  // e.g. some Giants, etc
  setName ("");
  if (d_stack && d_stack->empty () == false)
    setName(String::ucompose (_("some %1"), d_stack->front ()->getName ()));
}

Keeper::Keeper(const Keeper& object)
  : Renamable(object)
{
  if (object.d_stack)
    d_stack = new Stack(*object.d_stack);
  else
    d_stack = NULL;
}

Keeper::Keeper(XML_Helper* helper)
  :Renamable(helper)
{
  helper->registerTag(Stack::d_tag, sigc::mem_fun(this, &Keeper::load));
  d_stack = NULL;
}

bool Keeper::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag(Keeper::d_tag);
  retval &= helper->saveData("name", getName(false));
  if (d_stack)
    retval &= d_stack->save(helper);
  retval &= helper->closeTag();

  return retval;
}

void Keeper::clearStack ()
{
  if (d_stack)
    {
      delete d_stack;
      d_stack = NULL;
    }
}

const ArmyProto* Keeper::randomRuinDefender()
{
  Player *p = Playerlist::getInstance()->getNeutral();
  return
    Armysetlist::getInstance()->get(p->getArmyset())->getRandomRuinKeeper();
}

bool Keeper::load (Glib::ustring tag, XML_Helper *helper)
{
  if (tag == Stack::d_tag)
    {
      d_stack = new Stack (helper);
      return true;
    }
  return false;
}
