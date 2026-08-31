//  Copyright (C) 2017, 2021, 2026 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>

#include "turn-action-list.h"
#include "xml-helper.h"

Glib::ustring TurnActionlist::d_tag = "turn";
//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

TurnActionlist::TurnActionlist(const Player *p, const std::list<Action*> &actions)
  : OwnerId(p->getId())
{
  add (actions);
}

TurnActionlist::TurnActionlist (const TurnActionlist &t)
 : std::list<Action *> (), OwnerId (t), sigc::trackable (t)
{
  for (auto action: t)
    push_back (Action::copy (action));
}

TurnActionlist::~TurnActionlist()
{
  for (TurnActionlist::iterator it = begin(); it != end(); ++it)
    delete *it;
  clear();
}

TurnActionlist::TurnActionlist(XML_Helper* helper)
  : OwnerId(helper)
{
  helper->register_tag(Action::d_tag, sigc::mem_fun(*this, &TurnActionlist::load));

}

bool TurnActionlist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->open_tag(TurnActionlist::d_tag);

    retval &= OwnerId::save(helper);
    for (TurnActionlist::const_iterator it = begin(); it != end(); ++it)
      retval &= (*it)->save(helper);
    
    retval &= helper->close_tag();

    return retval;
}

bool TurnActionlist::load(Glib::ustring tag, XML_Helper* helper)
{
  if (tag == Action::d_tag)
    {
      push_back(Action::handle_load(helper));
      return true;
    }

    return false;
}

void TurnActionlist::add(const std::list<Action*> &actions)
{
  for (auto a : actions)
    add (a);
}

void TurnActionlist::add(const Action* action)
{
  push_back (Action::copy(action));
}
