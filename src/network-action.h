//  Copyright (C) 2008, 2014, 2017, 2026 Ben Asselstine
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
#ifndef NETWORK_ACTION_H
#define NETWORK_ACTION_H

#include "action.h"
#include "owner-id.h"

//! Just like Action, but explicitly associated with a Player.
class NetworkAction: public OwnerId
{
public:
	
    //! The xml tag of this object in a network stream.
    static Glib::ustring d_tag; 

     //! Default constructor.
     NetworkAction(const Action *action, guint32 owner);

     //! Loading constructor.
     NetworkAction(XML_Helper* helper);

     //! Destructor.
     ~NetworkAction();

     //! Returns debug information.
     Glib::ustring toString() const;

     //!Saving the network action to an xml stream.
     bool save(XML_Helper* helper) const;

     Action * getAction() const {return d_action;};

     void setAction (Action *action) {d_action = action;};
 private:
     Action *d_action;
};
#endif
