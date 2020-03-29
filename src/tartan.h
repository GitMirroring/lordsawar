//  Copyright (C) 2017 Ben Asselstine
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

//! A single set of shields for a player
/**
 *
 * Tartans consist of three images:
 * 1) a leftmost portion
 * 2) a repeating center portion
 * 3) a rightpost portion
 *
 * Each image file contains two halves, an image and a mask that gets
 * drawn in the player's colour.
 */
class Tartan
{
    public:

	//! The xml tag of this object in a shieldset configuration file.
	static Glib::ustring d_tag; 

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

        //! Get the filename of the tartan image, minus the path and suffix.
        Glib::ustring getTartanImageName (Tartan::Type type) const;

        //! Get the image of the tartan (the left half)
        PixMask *getImage(Type t) const;

        //! Get the mask of the tartan (the right half)
        PixMask *getMask(Type t) const;


	// Set Methods

        //! Set the filenmame of the tartan image, minus the path and suffix.
        void setTartanImageName (Tartan::Type type, Glib::ustring name);

        //! Set the image of the tartan (the left half)
        void setImage(Type t, PixMask *i);

        //! Set the mask of the tartan (the right half)
        void setMask (Type t, PixMask *i);


	// Methods that operate on class data and do not modify the class.

	//! Save the shield to an opened shieldset configuration file.
	bool saveTartan(XML_Helper *helper) const;


	// Methods that operate on class data and modify the class.

	//! Load the images associated with this tartan.
	void instantiateTartanImages(Glib::ustring l, Glib::ustring c, Glib::ustring r, bool &broken);

        void instantiateTartanImage(Tartan::Type type, Glib::ustring file, bool &broken);

	//! Destroy the images associated with this tartan.
	void uninstantiateTartanImages();
        void uninstantiateTartanImage(Tartan::Type type);

        //! Convert the enum to a nice readable string
        static Glib::ustring tartanTypeToFriendlyName(const Tartan::Type type);

    protected:

        Glib::ustring d_left_tartan_name;
        Glib::ustring d_center_tartan_name;
        Glib::ustring d_right_tartan_name;
        PixMask *d_left_tartan_image;
        PixMask *d_left_tartan_mask;
        PixMask *d_center_tartan_image;
        PixMask *d_center_tartan_mask;
        PixMask *d_right_tartan_image;
        PixMask *d_right_tartan_mask;
};

#endif // TARTAN_H
