//  Copyright (C) 2008, 2009, 2011, 2014, 2015, 2020 Ben Asselstine
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
class TarFileMaskedImage;

//! A graphic of a shield.
/**
 * This class is the atom of every shield. It contains all data related to
 * a single ShieldStyle type of a Shield.  ShieldStyles come in three sizes: 
 * small, medium and large (ShieldStyle::Type).
 *
 * Every ShieldStyle object has an image and a mask.  The mask identifies the
 * portion of the ShieldStyle to shade in the Player's color (Player::d_color).
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

	//! Returns the masked image for this shield.
        TarFileMaskedImage *getMaskedImage () const {return d_mimage;}


	// Methods that operate on class data but do not modify the class.
	
	//! Save the shieldstyle to an opened shieldset configuration file.
	bool save(XML_Helper *helper) const;


	// Static Methods
	
	//! Convert a ShieldStyle::Type enumerated value to a string.
	static Glib::ustring shieldStyleTypeToString(const ShieldStyle::Type type);

        //! Convert a ShieldStyle::Type to a suitable string for display.
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

        //! The masked image object for this shield.
        TarFileMaskedImage * d_mimage;
};

#endif // SHIELDSTYLE_H
