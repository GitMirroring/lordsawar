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
#ifndef ROADLIST_H
#define ROADLIST_H

#include "LocationList.h"
#include <sigc++/trackable.h>
#include "road.h"

class XML_Helper;
//! A list of Road objects on the game map.
/** 
 * The roadlist keeps track of the roads located on the game map. It
 * is implemented as a singleton because many classes use it for looking up
 * roads.
 */
class Roadlist : public LocationList<Road*>, public sigc::trackable
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

        //! Makes a copy of the roadlist.
        Roadlist* copy () {return new Roadlist (*this, true);}

	// Methods that operate on the class data but do not modify the class.

        //! Saves the list of roads to the opened saved-game file.
        bool save(XML_Helper* helper) const;

	//! Determines what the right Road::Type should be for the given tile.
	/**
	 * Scans the surrounding tiles to see which road picture fits best.
	 *
	 * @param tile  The position on the game map to calculate a road type
	 *              for.
	 *
	 * @return The Road::Type that makes the most sense for the given tile.
	 */
	int calculateType (Vector<int> tile) const;

	// Static Methods

        //! Return the singleton instance.  Create a new one if needed.
        static Roadlist* getInstance();

        //! Load the singleton instance from the opened saved-game file.
        static Roadlist* getInstance(XML_Helper* helper);

        //! Explicitly delete the singleton instance.
        static void deleteInstance();
        
        //! Replace the current roadlist with another.
        static void reset (Roadlist *r);

        //! Destructor.
        ~Roadlist();

    protected:
        //! Default constructor.
        Roadlist();

        //! Loading constructor.
	/**
	 * Make a new roadlist object by loading it from the opened saved-game
	 * file.
	 *
	 * @param helper  The opened saved game file to load the list of roads
	 *                from.
	 */
        Roadlist(XML_Helper* helper);

        //! Copy constructor
        Roadlist (const Roadlist &c, bool sync_ids = false);
    private:
        //! Callback for loading road objects into the list.
        bool load(Glib::ustring tag, XML_Helper* helper);

        //! A static pointer for the singleton instance.
        static Roadlist* s_instance;
};

#endif
