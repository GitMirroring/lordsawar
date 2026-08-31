//  Copyright (C) 2008, 2014, 2017, 2021, 2026 Ben Asselstine
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

#include "network-action.h"

#include "xml-helper.h"

Glib::ustring NetworkAction::d_tag = "networkaction";

NetworkAction::NetworkAction(const Action *action, guint32 owner)
  : OwnerId(owner), d_action (Action::copy (action))
{
}

NetworkAction::NetworkAction(XML_Helper* helper)
  : OwnerId(helper), d_action (NULL)
{
  //d_action is filled in later
}

NetworkAction::~NetworkAction()
{
  delete d_action;
}

bool NetworkAction::save(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->open_tag(NetworkAction::d_tag);
  retval &= OwnerId::save(helper);
  d_action->save(helper);
  retval &= helper->close_tag();
  return retval;
}

Glib::ustring NetworkAction::toString() const
{
  std::stringstream s;
  Glib::ustring action= d_action->dump();
  s <<"Player \""<< d_owner_id << "\"--> ";
  s <<action;
    
  return s.str();
}
