//  Copyright (C) 2007, 2008, 2011, 2014, 2015, 2021 Ben Asselstine
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
#ifndef BRIDGE_H
#define BRIDGE_H

#include "Location.h"

//! A bridge on the game map.
/** 
 * A bridge is a place on the map that simultaneously acts like a Road object
 * and a Port object.  Stack objects can move more efficiently on a bridge
 * tile, and the Stack can also use it as a jumping off point into the water.
 * A bridge object is built on a Tile with a terrain kind of Tile::WATER.
 */
class Bridge: public Location
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

        enum Type 
	  {
	    CONNECTS_TO_EAST = 0,
	    CONNECTS_TO_NORTH = 1,
	    CONNECTS_TO_WEST = 2,
	    CONNECTS_TO_SOUTH = 3
	  };
	static Glib::ustring bridgeTypeToString(const Bridge::Type type);
	static Bridge::Type bridgeTypeFromString(const Glib::ustring str);

	//! Default constructor.
        /**
         * @param pos          The location of the bridge.
         * @param type         The type of bridge.  0=e,1=n, 2=w, 3=s.
         */
        Bridge(Vector<int> pos, int type = 0);

	//! Copy constructor.
        Bridge(const Bridge&, bool sync_id = false);

        Bridge(const Bridge&, Vector<int> pos);

        //! Loading constructor.
	/**
	 * Load the bridge from the opened saved-game file.
	 * @param helper  The opened saved-game file to load the bridge from.
	 */
        Bridge(XML_Helper* helper);

	//! Destructor.
        ~Bridge() {};

        //! Returns the type of the bridge.
        int getType() const {return d_type;};

        //! Return the point at which a road would connect to this bridge.
        Vector<int> getRoadEntryPoint() const;

        //! Sets the type of the bridge.
        void setType(int type) {d_type = type;};

        //! Save the bridge data to the opened saved-game file.
        bool save(XML_Helper* helper) const;

    protected:
	//! The type of the bridge.
	/**
	 * The type of the bridge refers to it's look on the map.  It can be
	 * one of the following values:
	 *
	 * 0 = The bridge connects to a road to the west, and another bridge
	 *     to the east.
	 * 1 = The bridge connects to a road to the south, and another bridge
	 *     to the north.
	 * 2 = The bridge connects to a road to the east, and another bridge
	 *     to the west.
	 * 3 = The bridge connects to a road to the north, and another bridge
	 *     to the south.
	 */
	int d_type;

};

#endif // BRIDGE_H
