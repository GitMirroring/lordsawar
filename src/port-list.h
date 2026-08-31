// Copyright (C) 2007, 2008, 2009, 2014, 2021 Ben Asselstine
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
#ifndef PORTLIST_H
#define PORTLIST_H

#include "port.h"
#include <sigc++/trackable.h>
#include "LocationList.h"

class XML_Helper;

//! A list of the Port objects on the game map.
/** 
 * The portlist keeps track of the ports located on the game map. It
 * is implemented as a singleton because many classes use it for looking up
 * ports.
 */
class Portlist : public LocationList<Port*>, public sigc::trackable
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

        //! Makes a copy of the portlist.
        Portlist* copy () {return new Portlist (*this, true);}

	// Methods that operate on the class data but do not modify the class.

        //! Saves the list of Port objects to the opened saved-game file.
        bool save(XML_Helper* helper) const;


	// Static Methods

        //! Return the singleton instance.  Create a new one if needed.
        static Portlist* getInstance();

        //! Load the singleton instance from the opened saved-game file.
        static Portlist* getInstance(XML_Helper* helper);

        //! Explicitly delete the singleton instance.
        static void deleteInstance();

        //! Replace the current portlist with another.
        static void reset (Portlist *p);

        //! Destructor.
        ~Portlist();
    protected:

        //! Default constructor.
        Portlist();

        //! Loading constructor.
	/**
	 * Load the list of Port objects from the opened saved-game file.
	 *
	 * @param helper  The opened saved-game file to load Port objects from.
	 */
        Portlist(XML_Helper* helper);

        //! Copy constructor
        Portlist (const Portlist &p, bool sync_ids = false);

    private:
        //! Callback for loading Port objects into the list of ports.
        bool load(Glib::ustring tag, XML_Helper* helper);

	// DATA

        //! A static pointer for the singleton instance.
        static Portlist* s_instance;
};

#endif
