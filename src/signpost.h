//  Copyright (C) 2007, 2008, 2009, 2014, 2021 Ben Asselstine
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
#ifndef SIGNPOST_H
#define SIGNPOST_H

#define DEFAULT_SIGNPOST "nowhere"

#include "Location.h"
#include "Renamable.h"

//!A signpost is a map feature where a human player can read a message.
/**
 * Signposts are generally useful on a hidden map, when they can direct a 
 * player to a nearby city that is obscured from view.
 *
 * Players can change the contents of the signpost.
 */
class Signpost: public Location, public Renamable
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

	//! Default constructor.
        /**
         * @param pos          The location of the signpost on the game map.
         * @param name         The contents of the sign.
         */
        Signpost(Vector<int> pos, Glib::ustring name = "nowhere");

	//! Copy constructor.
        Signpost(const Signpost&, bool sync_id = false);

	//! Alternative copy constructor that changes the signpost's position.
        Signpost(const Signpost&, Vector<int> pos);

        //! Loading constructor.
	/**
	 * @param helper  The opened saved-game file to load the signpost from.
	 */
        Signpost(XML_Helper* helper);

	//! Destructor.
        ~Signpost() {};

	// Methods that operate on the class data but do not modify the class

        //! Save the signpost data to an opened saved-game file.
        bool save(XML_Helper* helper) const;

};

#endif // SIGNPOST_H
