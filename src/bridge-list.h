//  Copyright (C) 2007, 2008, 2009, 2011, 2014, 2021 Ben Asselstine
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
#ifndef BRIDGELIST_H
#define BRIDGELIST_H

#include <sigc++/trackable.h>
#include "LocationList.h"

class Bridge;
class XML_Helper;

//! A list of Bridge objects located on the game map.
/** 
 * The bridgelist keeps track of the bridges located on the game map. It
 * is implemented as a singleton because many classes use it for looking up
 * bridges.
 */
class Bridgelist : public LocationList<Bridge*>, public sigc::trackable
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

        //! Makes a copy of the bridgelist.
        Bridgelist* copy () {return new Bridgelist (*this, true);}

	// Methods that operate on the class data but do not modify the class.

        //! Saves the list of Bridge objects to the opened saved-game file.
        bool save(XML_Helper* helper) const;

	//! Determines what the right Bridge::Type should be for the given tile.
	/**
	 * Scans the surrounding tiles to see which bridge picture fits best.
	 *
	 * @param tile  The position on the game map to calculate a bridge type
	 *              for.
	 *
	 * @return The Bridge::Type that makes the most sense for the given 
	 *         tile.
	 */
	int calculateType(Vector<int> t) const;

        //! Give it a bridge, and this method returns the other half.
        Bridge* getOtherSide(Bridge *bridge);

        //! Get the two points at which roads would connect to this bridge.
        std::list<Vector<int> > getRoadEntryPoints(Bridge *bridge);

	// Static Methods

        //! Return the singleton instance.  Create a new one if needed.
        static Bridgelist* getInstance();

        //! Load the singleton instance from the opened saved-game file.
        static Bridgelist* getInstance(XML_Helper* helper);

        //! Explicitly delete the singleton instance.
        static void deleteInstance();

        //! Replace the current bridgelist with another.
        static void reset (Bridgelist *b);

        //! Destructor.
        ~Bridgelist();

    protected:
        //! Default constructor.
        Bridgelist();

        //! Loading constructor.
	/**
	 * Load the list of Bridge objects from the opened saved-game file.
	 *
	 * @param helper  The opened saved-game file to load the Bridge objects
	 *                from.
	 */
        Bridgelist(XML_Helper* helper);

        //! Copy constructor
        Bridgelist (const Bridgelist &b, bool sync_ids = false);

    private:
        //! Callback for loading Bridge objects into the list of bridges.
        bool load(Glib::ustring tag, XML_Helper* helper);

	// DATA

        //! A static pointer for the singleton instance.
        static Bridgelist* s_instance;
};

#endif
