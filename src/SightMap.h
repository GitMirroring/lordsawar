// Copyright (C) 2008, 2014, 2020 Ben Asselstine
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
#ifndef SIGHTMAP_H
#define SIGHTMAP_H

#include <gtkmm.h>
#include "rectangle.h"
#include "Renamable.h"

class XML_Helper;

//! An object that is given to a player that defogs a portion of the fog map.
/** 
 * This class describes a sightmap.  It is similar to an item except is not 
 * carried by a hero, and it is not retained in a backpack.  The sightmap is 
 * used once by a player to defog a portion of the hidden map.
 */

class SightMap: public LwRectangle, public Renamable
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

	//! Loading constructor.
        SightMap(XML_Helper* helper);

	//! Copy constructor.
        SightMap(const SightMap& orig);

	//! Default Constructor.  Creates a new sightmap from scratch.
	/**
	 * @param pos  The top-left corner of the sightmap on the game map.
	 */
        SightMap(Glib::ustring name, Vector<int> pos, guint32 height, guint32 width);

        //! Destructor.
        ~SightMap() {};
        

	// Methods that operate on class data but do not modify the class.

        //! Save the sightmap to the opened saved-game file.
        bool save(XML_Helper* helper) const;

    private:

};

#endif //SIGHTMAP_H
