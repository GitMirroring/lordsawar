//  Copyright (C) 2009, 2021 Ben Asselstine
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
#ifndef NAMELIST_H
#define NAMELIST_H

#include <map>
#include <vector>
#include <glibmm.h>
#include <sigc++/trackable.h>

class XML_Helper;

//! A list of names for ruins, temples and cities available to the game.
/** 
 * This class holds all of the possible names in the game for ruins, temples,
 * and cities.
 *
 */
class NameList : public std::vector<Glib::ustring>, public sigc::trackable
{
    public:

        //! Default Constructor.
	/**
	 * Loads all names it can find in the given file, and
	 * makes a new NameList object from what it finds.
	 */
        NameList(Glib::ustring filename, Glib::ustring item_tag);
        NameList (const NameList &l);

        //! Destructor.
        ~NameList() {};

        //! Reload the namelist and subtract the supplied NAMES that are used.
        /**
         * @return whether or not there are any unused names left.
         */
        bool repopulate (std::list<Glib::ustring> names);

	Glib::ustring popRandomName();

    private:

        //! Callback for loading names into the NameList.
	bool load(Glib::ustring tag, XML_Helper *helper);

        Glib::ustring d_filename;
	Glib::ustring d_item_tag;
};

#endif // NAMELIST_H

