//  Copyright (C) 2017, 2020 Ben Asselstine
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
#ifndef TARTAN_H
#define TARTAN_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include "PixMask.h"

class XML_Helper;
class Shieldset;
class TarFileMaskedImage;

//! A single set of shields for a player
/**
 *
 * Tartans consist of three images:
 * 1) a leftmost portion
 * 2) a repeating center portion
 * 3) a rightpost portion
 *
 * Each image file contains two halves, an image and a mask that gets
 * drawn in the player's color.
 */
class Tartan
{
    public:

	//! The xml tag of this object in a shieldset configuration file.
	static Glib::ustring d_tartan_tag; 

        enum Type
          {
            LEFT = 0,
            CENTER = 1,
            RIGHT= 2,
          };

	//! Loading constructor.
        /**
	 * Make a new Tartan object by reading it in from an opened shieldset
	 * configuration file.
	 *
         * @param helper  The opened shieldset configuration file to read the
	 *                tartan object from.
         */
        Tartan (XML_Helper* helper);
        
        //! Copy constructor
        Tartan (const Tartan& s);

	//! Default constructor.
	Tartan();

	//! Destructor.
        virtual ~Tartan();


	// Get Methods

        //! Get the masked image object of a section of the tartan.
        TarFileMaskedImage * getTartanMaskedImage (Tartan::Type type) const;


	// Methods that operate on class data and do not modify the class.

	//! Save the shield to an opened shieldset configuration file.
	bool saveTartan(XML_Helper *helper) const;

        //! Convert the enum to a nice readable string
        static Glib::ustring tartanTypeToFriendlyName(const Tartan::Type type);

    protected:

        TarFileMaskedImage *d_left_mimage;
        TarFileMaskedImage *d_center_mimage;
        TarFileMaskedImage *d_right_mimage;
};

#endif // TARTAN_H
