// Copyright (C) 2001, 2003 Michael Bartl
// Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2006 Andrea Paternesi
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
#ifndef TEMPLE_H
#define TEMPLE_H

#define DEFAULT_TEMPLE_NAME  "Shrine"

#include "NamedLocation.h"

//! A temple on the game map.
/** 
 * A temple is the place where heroes can get quests or have their armies
 * blessed.
 */
class Temple : public NamedLocation
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

	//! The kind of temple.
        enum Type
          {
            //! A columned temple.
            TEMPLE = 0, 
            //! A henge, e.g. of stones
            HENGE = 1,
          };
	//! Default constructor.
        /**
         * @param pos          The location of the temple on the game map.
	 * @param width        The span of tiles this temple covers.
         * @param name         The name of the temple.
	 * @param type         The type of the temple.  This should always
	 *                     be 0.
         */
        Temple(Vector<int> pos, guint32 width, Glib::ustring name = DEFAULT_TEMPLE_NAME, 
	       int type = 0);

	//! Copy constructor.
        Temple(const Temple&, bool sync_id = false);

	//! Alternative copying constructor that changes the temple position.
        Temple(const Temple&, Vector<int> pos);

        //! Loading constructor.
	/**
	 * @param helper  The opened saved-game file to load the temple from.
	 */
        Temple(XML_Helper* helper, guint32 width);

	//! Destructor.
        ~Temple() {};

	// Get Methods
        
	//! Returns the type of the temple.
        int getType() const {return d_type;};

        //! Returns whether or not the temple can be searched.
	/**
	 * @note Temples can always be searched in this game.
	 */
        bool searchable() const {return true;}


	// Set Methods

        //! Returns the type of the temple.
        void setType(int type) {d_type=type;};


	// Methods that operate on class data but do not modify the class.

        //! Save the temple to the opened saved-game file.
        bool save(XML_Helper* helper) const;

	//! Return true if the temple has the default temple name.
	bool isUnnamed() const {return getName() == getDefaultName() ? true : false;};

	// Static Methods

	//! Return the default name of any temple.
	static Glib::ustring getDefaultName() {return _(DEFAULT_TEMPLE_NAME);};

    protected:
	
	// DATA

	//! The type of the temple.
	/**
	 * The temple always has a type of 0, because there is only one kind
	 * of temple in the game.
	 */
        int d_type;
};

#endif // TEMPLE_H
