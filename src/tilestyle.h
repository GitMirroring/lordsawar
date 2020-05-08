// Copyright (C) 2007, 2008, 2009, 2010, 2014, 2015 Ben Asselstine
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
#ifndef TILESTYLE_H
#define TILESTYLE_H

#include <gtkmm.h>

#include "PixMask.h"

class XML_Helper;

//! Describes the look of a Tile.
/**
  * A TileStyle describes a single look of a Tile.  It is responsible for
  * keeping the id, the image, and the kind of style.  The TileStyles for a 
  * given tile are defined in the Tileset configuration file.
  * If the kind of tile is `forest', then the TileStyles are defined in 
  * individual tileset.tile.tilestyleset.tilestyle XML entities.
  *
  * There are many different images that can represent a kind of Tile. 
  * Only certain tiles look right when positioned beside each other.  The
  * correct styles are assigned by the GameMap::calculatePreferredStyle method.
  * The id is important because it is referenced by the saved-game file 
  * (the lordsawar.map.styles XML entity).
  *
  * Every TileStyle belongs to a TileStyleSet, which belongs to a Tile.
  * Every Maptile refers to a TileStyle.
  */
class TileStyle
{
    public:
	//! The xml tag of this object in a tileset configuration file.
	static Glib::ustring d_tag; 

        //! Describe terrain tile styles.
	  /**
	   * All of the Tilestyle kinds can be described by using the
	   * following template:
	   * @verbatim
+-----+
|#####|
|#+-+#|
|#|o|#|
|#+-+#|
|#####|
+-----+
@endverbatim
           * Picture a terrain feature transitioning to grass on the outside 
	   * of the ring and on the inside of the ring also.
	   * In the very center of the template is a single feature that
	   * transitions to grass on all sides.
           */
	enum Type { 
	  /**
	   * @verbatim
ooooooo
ooooooo
ooooooo
oooxooo
ooooooo
ooooooo
ooooooo
@endverbatim
           */
	  LONE = 0,

	  /**
	   * @verbatim
xoooooo
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
@endverbatim
           */
	  OUTERTOPLEFT = 1, 
	  /**
	   * @verbatim
oxxxxxo
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
@endverbatim
           */
	  OUTERTOPCENTER = 2, 
	  /**
	   * @verbatim
oooooox
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
@endverbatim
           */
	  OUTERTOPRIGHT = 3,
	  /**
	   * @verbatim
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
xoooooo
@endverbatim
           */
	  OUTERBOTTOMLEFT = 4, 
	  /**
	   * @verbatim
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
oxxxxxo
@endverbatim
           */
	  OUTERBOTTOMCENTER = 5, 
	  /**
	   * @verbatim
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
oooooox
@endverbatim
           */
	  OUTERBOTTOMRIGHT = 6,
	  /**
	   * @verbatim
ooooooo
xoooooo
xoooooo
xoooooo
xoooooo
xoooooo
ooooooo
@endverbatim
           */
	  OUTERMIDDLELEFT = 7, 
	  /**
	   * @verbatim
ooooooo
oxxxxxo
oxoooxo
oxoooxo
oxoooxo
oxxxxxo
ooooooo
@endverbatim
           */
	  INNERMIDDLECENTER = 8, 
	  /**
	   * @verbatim
ooooooo
oooooox
oooooox
oooooox
oooooox
oooooox
ooooooo
@endverbatim
           */
	  OUTERMIDDLERIGHT = 9,
	  /**
	   * @verbatim
ooooooo
ooooooo
ooxoooo
ooooooo
ooooooo
ooooooo
ooooooo
@endverbatim
           */
	  INNERTOPLEFT = 10, 
	  /**
	   * @verbatim
ooooooo
ooooooo
ooooxoo
ooooooo
ooooooo
ooooooo
ooooooo
@endverbatim
           */
	  INNERTOPRIGHT = 11, 
	  /**
	   * @verbatim
ooooooo
ooooooo
ooooooo
ooooooo
ooxoooo
ooooooo
ooooooo
@endverbatim
           */
	  INNERBOTTOMLEFT = 12, 
	  /**
	   * @verbatim
ooooooo
ooooooo
ooooooo
ooooooo
ooooxoo
ooooooo
ooooooo
@endverbatim
           */
	  INNERBOTTOMRIGHT = 13,
	  /**
	   * Visually it's the merging of positions 1 and 2.
	   * @verbatim
1oooooo
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
oooooo2
@endverbatim
           */
	  TOPLEFTTOBOTTOMRIGHTDIAGONAL = 14, 
	  /**
	   * Visually it's the merging of positions 1 and 2.
	   * @verbatim
oooooo1
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
2oooooo
@endverbatim
           */
	  BOTTOMLEFTTOTOPRIGHTDIAGONAL = 15,
	  OTHER = 16,
          UNKNOWN = 17
	};

	//! Default constructor.
	TileStyle();

        //! Copy constructor.
        TileStyle(const TileStyle& t);

        //! Make a new tilestyle from an id, and a type.
        TileStyle(guint32 id, TileStyle::Type type);

        //! Loading constructor.
	/**
	 * Load the tileset.tile.tilestyles.tilestyle XML entities from the
	 * tileset configuration files.
	 */
        TileStyle(XML_Helper* helper);

	//! Destructor.
        ~TileStyle();

	// Get Methods

        //! Get the id for this tilestyle.
	/*
	 * The id is unique among all other tilestyles in the Tileset.
	 */
	guint32 getId() const {return d_id;}

        //! Get the style type of this tile style.
        Type getType() const {return d_type;}

        //! Get the picture for tile style.
	PixMask* getImage() const {return d_image;}

	//! Get the name of the current style.
	Glib::ustring getTypeName() const;


	// Set Methods

	//! Set the style type of this tile style.
	/**
	 * @note This method is only used in the tileset editor.
	 */
	void setType(Type type) {d_type = type;}
                
	//! Set the id for this tilestyle.
	void setId(guint32 id) {d_id = id;}
 
	//! Set the image for the tilestyle.
	void setImage(PixMask* image) {d_image = image;};

        //! Free up the image associated with this tilestyle.
        void uninstantiateImage();

	// Methods that operate on the class data but do not modify the class.

	//! Save a TileStyle to an opened tile configuration file.
	/**
	 * @param  The opened XML tile configuration file.
	 */
	bool save(XML_Helper *helper);

	// Static Methods

	//! Get the name of the TileStyle::Type in string form.
	static Glib::ustring getTypeName(Type type);

        //! Return how many digits the hex number should be for an id this big.
        /**
         * It returns 2, 3, 4 or 5.  e.g. 0x12345
         */
        static guint32 calculateHexDigits(guint32 id);

        //! Return the string representation of a tile style id.
        /**
         * This is a bit trickier than expected because the GameMap object
         * wants to save a series of tile style ids with the same width (in 
         * characters).
         */
        static Glib::ustring idToString(guint32 id, guint32 digits = 0);

    private:
        // DATA

	//! The image of this tilestyle.
	PixMask* d_image; 

	//! The type of the tilestyle.
        Type d_type;

	//! The unique id for this tilestyle.
	/**
	 * It must be unique among all other TileStyle objects in the 
	 * Tileset.
	 * This id shows up in the saved-game file in the lordsawar.map.styles
	 * XML entity as a hexidecimal number.
	 */
	guint32 d_id;
};

#endif // TILESTYLE_H

// End of file
