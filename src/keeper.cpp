//  Copyright (C) 2020, 2021, 2026 Ben Asselstine
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
//  Foundation, Inc., 31 Milk Street #960789, Boston, MA 02196, USA.

#include "keeper.h"

#include "army-proto.h"
#include "player-list.h"
#include "army-set-list.h"
#include "xml-helper.h"

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
  Player *neutral = Playerlist::getNeutral ();
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
  helper->register_tag(Stack::d_tag, sigc::mem_fun(*this, &Keeper::load));
  d_stack = NULL;
}

bool Keeper::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->open_tag(Keeper::d_tag);
  retval &= helper->save("name", getName(false));
  if (d_stack)
    retval &= d_stack->save(helper);
  retval &= helper->close_tag();

  return retval;
}

Keeper::~Keeper ()
{
  clearStack ();
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
  Player *p = Playerlist::getNeutral();
  return
    Armysetlist::instance()->get(p->getArmyset())->getRandomRuinKeeper();
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

int Keeper::getTypeId () const
{
  int id = -1;
  if (d_stack)
    {
      if (d_stack->empty () == false)
        id = d_stack->front ()->getTypeId ();
    }
  return id;
}
