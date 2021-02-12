// Copyright (C) 2008, 2009, 2014, 2021 Ben Asselstine
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

#include <sstream>
#include "player.h"
#include "network-history.h"

#include "xmlhelper.h"

Glib::ustring NetworkHistory::d_tag = "networkhistory";

NetworkHistory::NetworkHistory(History *history, guint32 owner)
  :OwnerId(owner), d_history (History::copy (history))
{
}

NetworkHistory::NetworkHistory(XML_Helper* helper)
  : OwnerId(helper), d_history (NULL)
{
  //history is filled in later
}

NetworkHistory::~NetworkHistory()
{
  delete d_history;
}

bool NetworkHistory::save(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->openTag(NetworkHistory::d_tag);
  retval &= OwnerId::save(helper);
  d_history->save(helper);
  retval &= helper->closeTag();
  return retval;
}

Glib::ustring NetworkHistory::toString() const
{
  std::stringstream s;
  Glib::ustring history= d_history->dump();
  s <<"Player \""<< getOwner()->getName() << "\"--> ";
  s <<history;
    
  return s.str();
}
