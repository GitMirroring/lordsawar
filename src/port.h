// Copyright (C) 2007, 2008, 2014, 2021 Ben Asselstine
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

#pragma once
#ifndef PORT_H
#define PORT_H

#include "Location.h"

//! A port on the game map.
/** 
 * A port is place on the map that Stack objects can use to gain access to
 * the water.
 */
class Port: public Location
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

	//! Default constructor.
        /**
          * @param pos          The location of the port on the game map.
          */
        Port(Vector<int> pos);

	//! Copy constructor.
        Port(const Port&, bool sync_id = false);

	//! Alternative copying constructor that gives the port a new position.
        Port(const Port&, Vector<int> pos);

        //! Loading constructor.
	/**
	 * Load the port object from the opened saved-game file.
	 * @param helper  The opened saved-game file to load the port from.
	 */
        Port(XML_Helper* helper);

	//! Destructor.
        ~Port() {};

	// Methods that operate on the class data but do not modify the class.

        //! Save the port data to the opened saved-game file.
        bool save(XML_Helper* helper) const;

};

#endif // PORT_H
