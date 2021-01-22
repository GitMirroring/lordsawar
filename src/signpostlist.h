//  Copyright (C) 2007, 2008, 2009, 2014, 2020, 2021 Ben Asselstine
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
#ifndef SIGNPOSTLIST_H
#define SIGNPOSTLIST_H

#include <sigc++/trackable.h>
#include "LocationList.h"
#include "signpost.h"

class XML_Helper;

//! A list of Signpost objects on the game map.
/** 
 * The signpostlist keeps track of the signs located on the game map. It
 * is implemented as a singleton because many classes use it for looking up
 * signposts.
 */
class Signpostlist : public LocationList<Signpost*>, public sigc::trackable
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

        //! Makes a copy of the signpostlist.
        Signpostlist* copy () {return new Signpostlist (*this, true);}

	// Methods that operate on the class data but do not modify the class.

        //! Saves the signpost list to the opened saved-game file.
        bool save(XML_Helper* helper) const;

        //! Count the number of signposts that have the default name.
        guint32 countUnamedSignposts () const;

	// Static Methods

        //! Return the singleton instance.  Create a new one if needed.
        static Signpostlist* getInstance();

        //! Load the singleton instance loaded from the opened saved-game file.
        static Signpostlist* getInstance(XML_Helper* helper);

        //! Explicitly delete the singleton instance.
        static void deleteInstance();

        //! Replace the current signpostlist with another.
        static void reset (Signpostlist *s);

        //! Destructor.
        ~Signpostlist();

    protected:
        //! Default constructor.
        Signpostlist();

        //! Copy constructor
        Signpostlist (const Signpostlist &s, bool sync_ids = false);

        //! Loading constructor.
	/**
	 * @param helper  The opened saved-game file to load the signposts from.
	 */
        Signpostlist(XML_Helper* helper);

    private:
        //! Callback for loading signpost objects into the list.
        bool load(Glib::ustring tag, XML_Helper* helper);

	// DATA

        //! A static pointer for the singleton instance.
        static Signpostlist* s_instance;
};

#endif
