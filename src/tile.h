// Copyright (C) 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2007, 2008, 2010, 2011, 2014, 2015, 2020, 2021 Ben Asselstine
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
#ifndef TILE_H
#define TILE_H

#include <gtkmm.h>

#include "tilestyleset.h"

class XML_Helper;
class SmallTile;
class Tar_Helper;
//! Describes a kind of tile that a Stack can traverse.
/** 
 * Many tiles are put together to form a tileset. Thus, a tile describes a
 * single terrain type. It keeps the name, movement points, type (grass, water
 * etc.) and also keeps the images together.
 *
 * Each tile holds a list of TileStyleSet objects which hold a list of 
 * TileStyle objects.  Each TileStyleSet holds a bunch of pictures of
 * what this tile can look like.  A TileStyle is a single picture.
 * These TileStyle pictures are displayed on the BigMap using the MapRenderer.
 *
 * Tile objects are held in a Tileset object.
 * Maptile objects refer to Tile objects.
 */
class Tile : public std::list<TileStyleSet*>
{
    public:
	//! The xml tag of this object in a tileset configuration file.
	static Glib::ustring d_tag; 

        //! Enumerate the kinds of terrain that a Stack can potentially move on.
        enum Type { 
	  //! Synomymous with GRASS.
	  NONE = 0, 
	  //! Grassy plain.  Flat.  Open.  Easy to pass through.
	  GRASS = NONE, 
	  //! Lake, ocean, river, puddle, moat, or anything else watery.
	  WATER = 1, 
	  //! Trees in great abundance, also includes shrubberies.
	  FOREST = 2, 
	  //! Hilly terrain, generally passable.
	  HILLS = 4,
	  //! Very hilly terrain, generally not passable except by flight.
	  MOUNTAIN = 8, 
	  //! Marshy terrain.
	  SWAMP = 16
	};

	//! Default constructor.
	Tile();

        //! Constructor to fill out some of the Tile's values.
        /**
         * the pointer to the SmallTile object is taken.  A duplicate SmallTile
         * object is not copied from it.
         */
        Tile(Tile::Type type, Glib::ustring name, guint32 moves, SmallTile *s);

        //! Loading constructor.
	/**
	 * Loads the tileset.tile XML entities in the tileset configuration 
	 * files.
	 * */
        Tile(XML_Helper* helper);

        //! Copy constructor.
        Tile(const Tile& t);

	//! Destructor.
        ~Tile();

	// Get Methods

        //! Get the number of movement points needed to cross this tile
        guint32 getMoves() const {return d_moves;}

        //! Get the type (grass, hill,...) of this tile type.
        Type getType() const {return d_type;}

	//! Get the name of this kind of tile (used in the editor).
	Glib::ustring getName() const {return d_name;}

	int getTypeIndex() {return getTypeIndexForType(d_type);}

	SmallTile * getSmallTile() {return d_smalltile;};


	// Set Methods

	void setType(Type type) {d_type = type;}

	//! Set the name of this kind of tile (used in the editor).
	void setName(Glib::ustring name) {d_name = name;}

	void setTypeByIndex(int idx);

        //! Set the number of moves to cross this terrain w/o a bonus.
        void setMoves (guint32 mp) {d_moves = mp;}

	//! Set the SmallTile object associated with this tile.
	void setSmallTile(SmallTile *smalltile) {d_smalltile = smalltile;};


	// Methods the operate on the class data but do not modify the class

	//! Save a Tile to an opened tile configuration file.
	/**
	 * @param  The opened XML tile configuration file.
	 */
	bool save(XML_Helper *helper) const;

	//! Check to see if this tile is suitable for use within the game.
	bool validate() const;

	//! Check to see if the tilestylesets only contain simple tilestyles.
	bool consistsOnlyOfLoneAndOtherStyles() const;

	//! Lookup a random tile style for this tile.
	/**
	 * Scan the TileStyles for this Tile for a TileStyle that matches 
	 * the given style.  When there is more than one TileStyle to choose 
	 * from, randomly pick one from all of the matching TileStyle objects.
	 *
	 * @param style  The kind of style we're looking for.
	 *
	 * @return A pointer to the matching TileStyle object, or NULL if no 
	 *         TileStyle could be found with that given style.
	 */
	TileStyle *getRandomTileStyle (TileStyle::Type style) const;

        guint32 countTileStyles(TileStyle::Type type) const;

        std::list<TileStyle*> getTileStyles(TileStyle::Type type) const;

        std::list<std::pair<guint32, TileStyle::Type> >getAllTileStyleTypes () const;

        TileStyle* getTileStyle(guint32 id) const;

	// Methods the operate on the class data and modify the class

	//! Destroy the images associated with this tile.
	void uninstantiateImages();

	//! Load the images associated with this tile.
	void instantiateImages(int tilesize, Tar_Helper *t, bool scale,
                               bool &broken);


	// Static Methods

	//! Convert a Tile::Type enumerated value to a string.
	static Glib::ustring tileTypeToString(const Tile::Type type);

        //! Convert a Tile::Type into a string that is nice to display.
        static Glib::ustring tileTypeToFriendlyName(const Tile::Type type);

	//! Convert a string represenation of a Tile::Type to an enum value.
	static guint32 tileTypeFromString(const Glib::ustring str);

	//! If an army unit can move on these kinds of terrains, it is flying.
	static guint32 isFlying() 
	  {return FOREST | HILLS | WATER | SWAMP | MOUNTAIN;};

	static int getTypeIndexForType(Tile::Type type);

        static Tile * get_default_grass();
        static Tile * get_default_water();
        static Tile * get_default_forest();
        static Tile * get_default_hills();
        static Tile * get_default_mountains();
        static Tile * get_default_swamp();
	//! Check to see if the grass tilestyles are suitable for in-game use.
	bool validateGrass(std::list<TileStyle::Type> types) const;

	//! Check to see if the other tilestyles are suitable for in-game use.
	bool validateFeature(std::list<TileStyle::Type> types) const;

    private:


        // DATA

	//! The name of this kind of a tile.
	/**
	 * The name is taken from the tileset configuration file.
	 * This value doesn't change during gameplay.
	 * It used in the scenario editor, but not used in the game.
	 * Equates to the tileset.tile.d_name XML entities in the tileset
	 * configuration file.
	 */
	Glib::ustring d_name;

	//! The number of movement points required to cross this tile.
	/**
	 * If an Army unit cannot traverse the tile efficiently it pays
	 * this number of movement points to walk over this tile.
	 * This value doesn't change during gameplay.
	 * Equates to the tileset.tile.d_moves XML entities in the tileset
	 * configuration file.
	 */
        guint32 d_moves;
	
	//! The kind of terrain tile this instance represents.
	/**
	 * Equates to the tileset.tile.d_type XML entities in the tileset
	 * configuration file.
	 */
        Type d_type;

	//! What this Tile looks like when it's shown on the miniature map.
	SmallTile *d_smalltile;

};

#endif // TILE_H

// End of file
