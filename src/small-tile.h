// Copyright (C) 2008, 2009, 2010, 2011, 2012, 2014, 2015 Ben Asselstine
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
#ifndef SMALLTILE_H
#define SMALLTILE_H

#include <gtkmm.h>

class XML_Helper;

//! Describes the appearance of a tile on the miniature map.
/** 
 */
class SmallTile 
{
    public:
	//! The xml tag of this object in a tileset configuration file.
	static Glib::ustring d_tag; 

	//! The terrain tile's appearance as seen on the OverviewMap.
	enum Pattern { 

	  //! The terrain feature is shown as a single solid color.
	  SOLID = 0, 

	  //! The terrain feature is checkered with two alternating colors.
	  /**
	   * The stippled pattern looks something like this:
	   * @verbatim
xoxoxoxo
oxoxoxox
xoxoxoxo
oxoxoxox
@endverbatim
	   *
	   * It is currently used for Type::FOREST, and Type::HILLS.
	   */
	  STIPPLED = 1, 

	  //! The feature is random pixels with three different colors.
	  /**
	   * The random pattern looks something like this:
	   * @verbatim
xoexooxo
exoxxeox
xoeoxoxx
eoxeooex
@endverbatim
	   *
	   * It is currently used for Type::MOUNTAINS.
	   */
	  RANDOMIZED = 2, 

	  //! The feature is shaded on the bottom and on the left.
	  /**
	   * The sunken pattern looks something like this:
	   * @verbatim
xxxxxxxo
xxxxxxxo
xxxxxxxo
oooooooo
@endverbatim
	   *
	   * It is currently used for Type::WATER.
	   */
	  SUNKEN = 3,

	  //! The feature is shown as a 3 color pattern.
	  /**
	   * The tablecloth pattern looks something like this:
	   * @verbatim
xexexexe
eoeoeoeo
xexexexe
eoeoeoeo
@endverbatim
	   *
	   * It is currently used for Type::SWAMP.
	   */
	  TABLECLOTH = 4,
	  DIAGONAL = 5, 
	  CROSSHATCH = 6, 
	  //! The feature is shaded on the top and on the left, and striped too.
	  /**
	   * The sunken striped pattern looks something like this:
	   * @verbatim
oooooooo
oeeeeeee
oxxxxxxx
oeeeeeee
@endverbatim
	   *
	   * It is currently used for Type::WATER.
	   */
	  SUNKEN_STRIPED = 7,

          SUNKEN_RADIAL = 8
	};
                    

	//! Default constructor.
	SmallTile();

        //! Constructor to set some of the values.
        SmallTile(SmallTile::Pattern pattern, Gdk::RGBA first, Gdk::RGBA second, Gdk::RGBA third);

        //! Copying constructor.
        SmallTile(const SmallTile &orig);

        //! Loading constructor.
	/**
	 * Loads the tileset.tile XML entities in the tileset configuration 
	 * files.
	 * */
        SmallTile(XML_Helper* helper);

	//! Destructor.
        ~SmallTile() {};


	// Get Methods

        //! Get the color associated with this tile for the smallmap.
	Gdk::RGBA getColor() const {return d_color;}

        //! Get the alternate color associated with this tile's pattern.
	/**
	 * This "second" color gets used when SmallTile::Pattern is
	 * STIPPLED, RANDOMIZED, SUNKEN, or TABLECLOTH.
	 */
	Gdk::RGBA getSecondColor() const {return d_second_color;}

        //! Get another alternate color associated with this tile's pattern.
	/**
	 * This "third" color gets used when SmallTile::Pattern is
	 * RANDOMIZED, DIAGONAL, CROSSHATCH, or TABLECLOTH.
	 */
	Gdk::RGBA getThirdColor() const {return d_third_color;}

        //! Get the pattern (solid, stippled, random) of this type.
        Pattern getPattern() const {return d_pattern;}


	// Set Methods

        //! Set the color associated with this tile for the smallmap.
	void setColor(Gdk::RGBA clr) {d_color = clr;}

        //! Set the alternate color associated with this tile's pattern.
        void setSecondColor(Gdk::RGBA color) {d_second_color = color;}

        //! Set another alternate color associated with this tile's pattern.
        void setThirdColor(Gdk::RGBA color) {d_third_color = color;}

        //! set the pattern (solid, stippled, random) of this type.
	void setPattern(Pattern pattern) {d_pattern = pattern;}


	// Methods that operate on class data but do not modify the class.

	//! Save a SmallTile to an opened tile configuration file.
	/**
	 * @param  The opened XML tile configuration file.
	 */
	bool save(XML_Helper *helper) const;

        // Static methods
        static SmallTile* get_default_grass();
        static SmallTile* get_default_water();
        static SmallTile* get_default_forest();
        static SmallTile* get_default_hills();
        static SmallTile* get_default_mountains();
        static SmallTile* get_default_swamp();
    private:

	//! The general appearance of the terrain tile on the OverviewMap.
	/**
	 * Equates to the tileset.tile.smallmap.d_pattern XML entities in the 
	 * tileset configuration file.
	 */
	Pattern d_pattern;

	//! First color.
	/**
	 * Equates to the following XML entities in the tileset configuration
	 * file:
	 * tileset.tile.smallmap.d_red 
	 * tileset.tile.smallmap.d_green
	 * tileset.tile.smallmap.d_blue
	 */
	Gdk::RGBA d_color;

	//! Second color.
	/**
	 * Only used when SmallTile::Pattern is one of: STIPPLED, 
	 * RANDOMIZED, SUNKEN, TABLECLOTH, DIAGONAL, or CROSSHATCH.
	 *
	 * Equates to the following XML entities in the tileset configuration
	 * file:
	 * tileset.tile.smallmap.d_2nd_red 
	 * tileset.tile.smallmap.d_2nd_green
	 * tileset.tile.smallmap.d_2nd_blue
	 */
	Gdk::RGBA d_second_color;

	//! Third color.
	/**
	 * Only used when Tile::Pattern is Tile::RANDOMIZED, or 
	 * Tile::TABLECLOTH.
	 *
	 * Equates to the following XML entities in the tileset configuration
	 * file:
	 * tileset.tile.smallmap.d_3rd_red 
	 * tileset.tile.smallmap.d_3rd_green
	 * tileset.tile.smallmap.d_3rd_blue
	 */
	Gdk::RGBA d_third_color;

};

#endif // SMALLTILE_H

// End of file
