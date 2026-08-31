//  Copyright (C) 2007, 2008, 2010, 2011, 2014, 2020 Ben Asselstine
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
#ifndef TILESTYLESET_H
#define TILESTYLESET_H

#include <vector>
#include <gtkmm.h>
#include <sigc++/trackable.h>

#include "tilestyle.h"

class XML_Helper;
class Tileset;

/** 
 * TileStyleSet is an array of tilestyles (the look of terrain tile objects).
 * All of the TileStyles describe one of the many looks of a particular kind
 * of Tile.  e.g. `Forest'.
 * Every TileStyleSet belongs to a Tile.
 * The TileStyleSet images are located in .lwt files in the Tileset's directory.
 */
//! This class manages a set of TileStyle objects.
class TileStyleSet : public sigc::trackable, public std::vector<TileStyle*>
{
    public:

	//! The xml tag of this object in a tileset configuration file.
	static Glib::ustring d_tag; 

	//! The default constructor.
        TileStyleSet();

        //! Copy constructor.
        TileStyleSet(const TileStyleSet& t);

        //! Make a new tilestyleset from an image.
        /**
         * convenience constructor.
         * tile style ids will be overlapping and need to be given values.
         * */
        TileStyleSet(Glib::ustring pngfilename, guint32 tilesize, bool &success, TileStyle::Type type = TileStyle::UNKNOWN);

	//! The loading constuctor loads the TileStyleSet from the config file.
	/**
	 * Read the tileset.tile.tilestyleset XML entities in the tileset
	 * configuration file.
	 *
	 * @param helper  The opened tileset configuration file.
	 */
        TileStyleSet(XML_Helper* helper);

	//! Destructor.
        ~TileStyleSet();


	// Get Methods

	//! Get the name of this tilestyleset.
	/**
	 * Returns the text loaded from a tileset.tile.tilestyles.d_name
	 * XML entity of the tileset configuration flie.
	 * This name refers to the filename that holds the imagery for this
	 * tilestyleset.  It is a basename of the filename.  It doesn't
	 * contain any slashes, or an ending file extension.  eg. ".png".
	 */
	Glib::ustring getName() const {return d_name;}


	// Set Methods

	//! Set the name of this tilestyleset.
	void setName(Glib::ustring name) {d_name = name;}


	//Methods that operate on the class data but do not modify the class.

	//! Save a TileStyleSet to an opened tile configuration file.
	/**
	 * @param  The opened XML tile configuration file.
	 */
	bool save(XML_Helper *helper) const;

	//! Return a list of all of the tilestyle types in this tilestyleset.
	void getUniqueTileStyleTypes(std::list<TileStyle::Type> &types) const;

	//! Check to see if this tilestyleset is usable in the game.
	bool validate() const;


	//Methods that operate on the class data and modify the class.

	//! Instantiate the tilestyleset's images from the given file.
	void loadImages(int tilesize, Glib::ustring image_filename,
                        bool scale, bool &broken);

	//! Destroy the images associated with this tilestyleset.
	void uninstantiateImages();

        static bool validate_image(Glib::ustring filename);

        bool instantiateImages (Tileset *set);

    private:

	// DATA

	//! The name of the tilestyleset.
	/**
	 * This is the basename of the image that contains a row of
	 * cells where each cell is tilesize pixels high, and tilesize
	 * pixels wide.  Each cell is another image of a tilestyle.  There is
	 * one cell per TileStyle in this TileStyleSet.
	 * The tilesize comes from the TileStyleSet::instantiateImages
	 * method.
	 * The name does not contain a path, and does not contain an
	 * extension (e.g. .png).  It must refer to a PNG file.
	 */
        Glib::ustring d_name;

};

#endif // TILESTYLESET_H

// End of file
