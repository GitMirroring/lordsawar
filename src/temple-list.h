// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2001, 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2007, 2008, 2009, 2014, 2020, 2021 Ben Asselstine
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
#ifndef TEMPLELIST_H
#define TEMPLELIST_H

#include <sigc++/trackable.h>
#include "LocationList.h"
#include "temple.h"
class Stack;
class XML_Helper;

//! A list of Temple objects on the game map.
/** 
 * The templelist keeps track of the temples located on the game map. It
 * is implemented as a singleton because many classes use it for looking 
 * up temples.
 */
class Templelist : public LocationList<Temple*>, public sigc::trackable
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

        //! Makes a copy of the templelist.
        Templelist* copy () {return new Templelist (*this, true);}

	// Methods that operate on class data but do not modify the class.
	
        //! Saves the temple data to an opened saved-game file.
        bool save(XML_Helper* helper) const;

        //! Find the nearest temple that is not obscured by fog.
	/**
	 * Scan through all temples, searching for the closest one that is
	 * not covered by fog-of-war on a hidden map.
	 *
	 * @param pos  The position to find the nearest temple from.
	 *
	 * @return A pointer to the nearest temple that is not obscured by fog.
	 */
        Temple* getNearestVisibleTemple(const Vector<int>& pos) const;

	//! Find the nearest temple that is unobscured and is not too far away.
	/**
	 * Scan through all the temples, searching for the closest one that
	 * is not covered by fog-of-war on a hidden map, but is not farther
	 * away than a given distance.
	 *
	 * @param pos  The position to find the nearest temple from.
	 * @param dist The number of tiles away that is deemed "too far".
	 *
	 * @return A pointer to the nearest temple that is not obscured by fog 
	 *         and is within the prescribed number of tiles.  Returns NULL 
	 *         if no temple could be found.
	 */
        Temple* getNearestVisibleTemple(const Vector<int>& pos, int dist) const;

	//! Find the nearest temple that the given stack can get blessed at.
	/**
	 * @param stack  The stack that wants to get blessed at a temple.
	 * @param percent_can_be_blessed  
	 *               Consider temples where the minimum percentage of army 
	 *               units in the stack that have not been blessed at a
	 *               temple.
	 * @return A pointer to the nearest temple that is not obscured by fog
	 *         and has more than the given percentage of army units that
	 *         can be blessed there.  Returns NULL if no temple could be
	 *         found.
	 */
	Temple* getNearestVisibleAndUsefulTemple(Stack *stack, double percent_can_be_blessed) const;

	//! Find the nearest temple that the given stack can get blessed at.
	/**
	 * @param stack  The stack that wants to get blessed at a temple.
	 * @param percent_can_be_blessed  
	 *               Consider temples where the minimum percentage of army 
	 *               units in the stack that have not been blessed at a
	 *               temple.
	 * @param dist   Do not consider temples farther away than dist tiles.
	 *
	 * @return A pointer to the nearest temple that is not obscured by fog
	 *         and has more than the given percentage of army units that
	 *         can be blessed there, and is within a certain number of 
	 *         tiles from the stack's position on the map.  Returns NULL if 
	 *         no temple could be found.
	 */
	Temple* getNearestVisibleAndUsefulTemple(Stack *stack, double percent_can_be_blessed, int dist) const;


        //! Count the number of temples that have the default name.
        guint32 countUnamedTemples () const;

	// Static Methods

        //! Return the singleton instance.  Create a new one if needed.
        static Templelist* getInstance();

        //! Load the temple list from an opened saved-game file.
	/**
	 * @param helper  The opened saved-game file to load the list of 
	 *                temples from.
	 *
	 * @return The list of temples.
	 */
        static Templelist* getInstance(XML_Helper* helper);

        //! Explicitly delete the singleton instance.
        static void deleteInstance();

        //! Replace the current templelist with another.
        static void reset (Templelist *t);

        //! Destructor.
        ~Templelist();
    protected:
        //! Default constructor.
        Templelist();

        //! Copy constructor.
        Templelist (const Templelist &t, bool sync_ids = false);

        //! Loading constructor.
	/**
	 * Load the list of temples from an opened saved-game file.
	 *
	 * @param helper  The opened saved-game file to load the temples from.
	 */
        Templelist(XML_Helper* helper);

    private:
        //! Callback for loading temple objects from opened saved game files.
        bool load(Glib::ustring tag, XML_Helper* helper);

        //! A static pointer for the singleton instance.
        static Templelist* s_instance;
};

#endif
