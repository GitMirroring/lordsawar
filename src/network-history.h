//  Copyright (C) 2008, 2009, 2026 Ben Asselstine
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

#pragma once
#ifndef NETWORK_HISTORY_H
#define NETWORK_HISTORY_H

#include "history.h"
#include "owner-id.h"

//! A history object that's owned by a player, to be sent to another player.
class NetworkHistory: public OwnerId
{
public:
    //! The xml tag of this object in a network stream.
    static Glib::ustring d_tag; 

     //! Default constructor.
     NetworkHistory(History *history, guint32 owner);

     //! Loading constructor.
     NetworkHistory(XML_Helper* helper);

     //! Destructor.
     ~NetworkHistory();

     //! Returns debug information.
     Glib::ustring toString() const;

     //!Saving the network history to an xml stream.
     bool save(XML_Helper* helper) const;

     History * getHistory() const {return d_history;};
     void setHistory (History *history) {d_history = history;};

 private:
     History *d_history;
};
#endif
