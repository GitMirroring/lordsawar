//  Copyright (C) 2008, 2009, 2011, 2014, 2021 Ben Asselstine
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
#ifndef SHIELD_H
#define SHIELD_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>

#include "shieldstyle.h"
#include "tartan.h"

class XML_Helper;
class Shieldset;
class Tar_Helper;

//! A single set of shields for a player
/**
 *
 */
class Shield : public std::list<ShieldStyle*>, public Tartan, public sigc::trackable
{
    public:

	//! The xml tag of this object in a shieldset configuration file.
	static Glib::ustring d_tag; 

	//! The notional player that the Shield goes with.
	enum Color {WHITE = 0, GREEN = 1, YELLOW = 2, DARK_BLUE = 3, ORANGE = 4,
	  LIGHT_BLUE = 5, RED = 6, BLACK = 7, NEUTRAL = 8};

	//! Loading constructor.
        /**
	 * Make a new Shield object by reading it in from an opened shieldset
	 * configuration file.
	 *
         * @param helper  The opened shieldset configuration file to read the
	 *                shield object from.
         */
        Shield(XML_Helper* helper);
        
        //! Copy constructor
        Shield(const Shield& s);

	//! Default constructor.
	Shield(Shield::Color owner, std::vector<Gdk::RGBA> colors);

	//! Destructor.
        virtual ~Shield();


	// Get Methods

        //! Returns the player that this shield will belong to.
	guint32 getOwner() const {return d_owner;}

        //! Returns the color of the player shield.
	Gdk::RGBA getColor() const {return d_colors[0];}

        //! Returns the colors of the player shield.
        std::vector<Gdk::RGBA> getColors() const {return d_colors;}

	// Set Methods

	//! Sets the color of the player shield.
	void setColor(Gdk::RGBA c) {d_colors[0] = c;}

        //! Sets the colors of the player shield.
        void setColors (std::vector<Gdk::RGBA> l) {d_colors = l;}

	// Methods that operate on class data and do not modify the class.

	//! Save the shield to an opened shieldset configuration file.
	bool save(XML_Helper *helper) const;

	//! Get the first shieldstyle in the shield with the given type.
	ShieldStyle *getFirstShieldstyle(ShieldStyle::Type type);


	// Static Methods

	/**
	 * Get the default color for the Player with the given Id.
	 *
	 * @note This color is used to graphically shade Army, Shield, Flags,
	 * and selector pictures.
	 *
	 * @note This is not used to obtain the Neutral player's color.
	 *
	 * @param player_no  The player's Id for which we want the color.
	 *
	 * @return The default colors associated with the player.
	 */
	//! Get standard color for a player.
	static std::vector<Gdk::RGBA> get_default_colors_for_no(int player_no);

	//! Get standard color for the neutral player.
	static std::vector<Gdk::RGBA> get_default_colors_for_neutral();

	//! Convert the Shield::Color enumerated value to a string.
	static Glib::ustring colorToString(const Shield::Color c);

        //! Convert Shield::Color into a string suitable for display.
        static Glib::ustring colorToFriendlyName (const Shield::Color c);

        static guint32 get_next_shield(guint32 type);
    protected:

	//! The player of the shield.
	/**
	 * Equates to the shieldset.shield.d_color XML entities in the 
	 * shieldset configuration file.
	 * Equates to the Shield::Color enumeration.
	 */
	guint32 d_owner;

	//! The player's colors.
	/**
	 * Mask portions of images are shaded in these colors.
	 */
        std::vector<Gdk::RGBA> d_colors;

};

#endif // SHIELD_H
