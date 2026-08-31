// Copyright (C) 2004, 2005 Ulf Lorenz
// Copyright (C) 2007, 2008, 2011, 2015, 2021 Ben Asselstine
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
#ifndef ITEMLIST_H
#define ITEMLIST_H

#include <map>
#include <sigc++/trackable.h>

#include "ItemProto.h"

//! A list of Item objects.
/** 
 * The Itemlist holds all item templates (i.e. types of items) together.
 * 
 * It is implemented as a singleton. Upon creation, it reads the item
 * description file and initialises an internal list.
 *
 * For easier access, the Itemlist is derived from map. Given an item index,
 * you can get the item belonging to the index by the []-operator using the
 * item index as index.
 */
class Itemlist : public std::map<guint32, ItemProto*>, public sigc::trackable
{
    public:

        //! Makes a copy of the Itemlist.
        Itemlist* copy () {return new Itemlist (*this);}

	//! The xml tag of this object.
	/** 
	 * @note This tag appears in the items configuration file, or in a 
	 * saved-game file.
	 */
	static Glib::ustring d_tag; 

        //! Returns the singleton instance.
	static Itemlist* getInstance();

	//! Reads in the itemlist from a file
        static Itemlist* getInstance(XML_Helper *helper);

        //! Creates a new singleton instance. Deletes an existing one.
	//This list of items comes from loading the items.xml file.
        static void createStandardInstance();

        //! Creates a new singleton instance. Deletes an existing one.
        static void create();

        //! Explicitly deletes the singleton instance.
        static void deleteInstance();
        
	//! Save the item data.  See XML_Helper for details.
	bool save(XML_Helper* helper) const;

	void remove(ItemProto *item);
        void add(ItemProto *item);

        static bool upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version);
        static void support_backward_compatibility();

        //! Replace the current Itemlist with another.
        static void reset (Itemlist *p);

	//! Destructor.
        ~Itemlist();

    protected:

	//! Default constructor.
	Itemlist();
	//! Loading constructor.
        Itemlist(XML_Helper* helper);
        //! Copy constructor
        Itemlist (const Itemlist &i);

    private:

        //! Callback for loading an Item from an opened saved-game file.
        bool loadItemProto(Glib::ustring tag, XML_Helper* helper);

        //! Erases an Item from the list and frees the Item too.
        void flErase(iterator it);

        //! Clears the itemlist of all it's items.
        void flClear();

        static Itemlist* d_instance;

};

#endif //ITEMLIST_H
