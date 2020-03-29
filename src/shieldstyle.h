//  Copyright (C) 2008, 2009, 2011, 2014, 2015 Ben Asselstine
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
#ifndef SHIELDSTYLE_H
#define SHIELDSTYLE_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "PixMask.h"


class XML_Helper;
class Shieldset;

//! A graphic of a shield.
/**
 * This class is the atom of every shield. It contains all data related to
 * a single ShieldStyle type of a Shield.  ShieldStyles come in three sizes: 
 * small, medium and large (ShieldStyle::Type).
 *
 * Every ShieldStyle object has an image and a mask.  The mask identifies the
 * portion of the ShieldStyle to shade in the Player's colour (Player::d_color).
 * The mask appears on the right side the shield image file.
 *
 */
class ShieldStyle : public sigc::trackable
{
    public:

	//! The xml tag of this object in a shieldset configuration file.
	static Glib::ustring d_tag; 

	//! The size of the shield.
	enum Type {
	  //! Small shields are shown on the OverviewMap object.
	  SMALL = 0, 
	  //! Medium shields are shown in the top right of the GameWindow.
	  MEDIUM = 1, 
	  //! Large shields are shown in the DiplomacyDialog and FightWindow.
	  LARGE = 2
	};

	//! Loading constructor.
        /**
	 * Make a new ShieldStyle object by readiang it in from an opened shieldset
	 * configuration file.
	 *
         * @param helper  The opened shieldset configuration file to read the
	 *                shield object from.
         */
        ShieldStyle(XML_Helper* helper);

        //! Copy constructor.
        ShieldStyle(const ShieldStyle& s);

	//! Default constructor.
	ShieldStyle(ShieldStyle::Type type);
        
	//! Destructor.
        ~ShieldStyle();

        
        // Get Methods
        
        //! Get the size of this shield.
        guint32 getType() const {return d_type;}

        //! Get the image of the shield.
	PixMask* getImage() const {return d_image;}

        //! Returns the mask of the shield.
	PixMask* getMask() const {return d_mask;}

	//! Returns the basename of the picture's filename.
	Glib::ustring getImageName() const {return d_image_name;}


        // Set Methods
        
        //! Set the basic image of the shield.
        void setImage(PixMask* image) {d_image = image;};

        //! Set the mask of the shield.
        void setMask(PixMask* mask) {d_mask = mask;}

	//! Set the basename of the shield picture's filename.
	void setImageName(Glib::ustring name) {d_image_name = name;}


	// Methods that operate on class data and modify the class.

	//! Load the images for this shieldstyle from the given file.
	void instantiateImages(Glib::ustring filename, Shieldset *s,
                               bool scale, bool &broke);

	//! Destroy the images associated with this shieldstyle.
	void uninstantiateImages();


	// Methods that operate on class data but do not modify the class.
	
	//! Save the shieldstyle to an opened shieldset configuration file.
	bool save(XML_Helper *helper) const;


	// Static Methods
	
	//! Convert a ShieldStyle::Type enumerated value to a string.
	static Glib::ustring shieldStyleTypeToString(const ShieldStyle::Type type);
        //! Convret a ShieldStyle::Type to a suitable string for display.
        static Glib::ustring shieldStyleTypeToFriendlyName(const ShieldStyle::Type type);

	//! Convert a ShieldStyle::Type string to an enumerated value.
	static ShieldStyle::Type shieldStyleTypeFromString(const Glib::ustring str);
    protected:

	//! The size of the shield. (small, medium, or large)
	/**
	 * Equates to the shieldset.shield.d_type XML entities in the shieldset
	 * configuration file.
	 * Equates to the ShieldStyle::Type enumeration.
	 */
        guint32 d_type;

	//! The unshaded image portion of the shield's picture.
	PixMask* d_image;

	//! The portion of the shield's image to shade in the player's colour.
	/**
	 * The mask appears to the right of the image in the shield's picture.
	 * The colour that shades the mask is dictated by Player::d_colour.
	 */
	PixMask* d_mask;

	//! The basename of the shield's picture file.
	/**
	 * Returns the filename that holds the image for this ShieldStyle.
	 * The filename does not have a path, and the filename does
	 * not have an extension (e.g. .png).
	 */
	Glib::ustring d_image_name;
};

#endif // SHIELDSTYLE_H
