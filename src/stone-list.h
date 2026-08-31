//  Copyright (C) 2017, 2021 Ben Asselstine
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
#ifndef STONELIST_H
#define STONELLIST_H

#include "LocationList.h"
#include <sigc++/trackable.h>
#include "stone.h"

class XML_Helper;
//! A list of Stone objects on the game map.
/** 
 * The stonelist keeps track of the stones located on the game map. It
 * is implemented as a singleton because many classes use it for looking up
 * stones.
 */
class Stonelist : public LocationList<Stone*>, public sigc::trackable
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

        //! Makes a copy of the stonelist.
        Stonelist* copy () {return new Stonelist (*this, true);}

	// Methods that operate on the class data but do not modify the class.

        //! Saves the list of stones to the opened saved-game file.
        bool save(XML_Helper* helper) const;

	// Static Methods

        //! Return the singleton instance.  Create a new one if needed.
        static Stonelist* getInstance();

        //! Load the singleton instance from the opened saved-game file.
        static Stonelist* getInstance(XML_Helper* helper);

        //! Explicitly delete the singleton instance.
        static void deleteInstance();
        
        //! Replace the current stonelist with another.
        static void reset (Stonelist *s);

        //! Destructor.
        ~Stonelist();

    protected:
        //! Default constructor.
        Stonelist();

        //! Loading constructor.
	/**
	 * Make a new stonelist object by loading it from the opened saved-game
	 * file.
	 *
	 * @param helper  The opened saved game file to load the list of stones
	 *                from.
	 */
        Stonelist(XML_Helper* helper);

        //! Copy constructor
        Stonelist (const Stonelist &s, bool sync_ids = false);

    private:
        //! Callback for loading stone objects into the list.
        bool load(Glib::ustring tag, XML_Helper* helper);

        //! A static pointer for the singleton instance.
        static Stonelist* s_instance;
};

#endif
