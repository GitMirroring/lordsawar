// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2007-2011, 2014, 2017, 2020, 2021 Ben Asselstine
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
#ifndef TILESET_H
#define TILESET_H

#include <vector>
#include <sigc++/trackable.h>

#include "Tile.h"
#include "defs.h"
#include "set.h"
#include "SmallTile.h"

class XML_Helper;
class TarFileMaskedImage;
class TarFileImage;

//! A list of Tile objects in a terrain theme.
/** 
 * Tileset is a list of Tile objects.  It acts as the themeing mechanism for
 * the look (and partially the behaviour) of terrain objects in the game.
 * The Tileset dictates the pixel size of the tiles, and is used to lookup
 * Tile and TileStyle objects.  It is implemented as a singleton because many 
 * classes use it for looking up Tile and TileStyle objects.
 * 
 * Tileset objects are often referred to by their base name 
 * (Tileset::d_basename).
 *
 * Tileset objects reside on disk in the tilesets/ directory, each of which is
 * it's own .lwt file, which is a tar file.
 *
 * There are several images held in a tileset object, and all of them are
 * TarFileImage or TarFileMaskedImage objects, except for TileStyleSets which
 * are specially handled.  Images are referred to by basename, which are 
 * archive members
 */
class Tileset : public sigc::trackable, public std::vector<Tile*>, public Set
{
    public:
	//! The xml tag of this object in a tileset configuration file.
	static Glib::ustring d_tag; 

	//! The xml tag of the road smallmap section of the tileset.
	static Glib::ustring d_road_smallmap_tag; 

	//! The xml tag of the ruin smallmap section of the tileset.
	static Glib::ustring d_ruin_smallmap_tag; 

	//! The xml tag of the temple smallmap section of the tileset.
	static Glib::ustring d_temple_smallmap_tag; 

	//! tilesets have this extension. e.g. ".lwt".
	static Glib::ustring file_extension; 

	//! Default constructor.
	/**
	 * Make a new Tileset.
	 *
	 * @param id    A unique numeric identifier among all tilesets.
	 * @param name  The name of the Tileset.  Analagous to Tileset::d_name.
	 */
	Tileset(guint32 id, Glib::ustring name);

	//! Loading constructor.
	/**
	 * Make a new Tileset object by loading the data from a tileset
	 * configuration file.
	 *
	 * @param helper  The opened tileset configuration file to load the
	 *                tileset from.
	 */
        Tileset(XML_Helper* helper, Glib::ustring directory);

        //! Copy constructor.
        Tileset(const Tileset& tileset);

	//! Destructor.
        ~Tileset();


	// Get Methods

        //! Returns the frames of selector, either the large one or the small.
        TarFileMaskedImage *getSelector(bool large) {return d_selector[large ? 1 : 0];}

        //! Returns the frames of the flag
        TarFileMaskedImage *getFlags() {return d_flag;}

        //! Returns the explosion image
        TarFileImage *getExplosion() {return d_explosion;}

        //! Returns the road images
        TarFileImage *getRoad() {return d_road;}

        //! Returns the standing stone images
        TarFileImage *getStone() {return d_stone;}

        //! Returns the bridge images
        TarFileImage *getBridge() {return d_bridge;}

        //! Returns the fog images
        TarFileImage *getFog() {return d_fog;}

        //! Get the color associated with the road on the smallmap.
	Gdk::RGBA getRoadColor() const {return d_road_color;}

        //! Get the color associated with temples on the smallmap.
	Gdk::RGBA getTempleColor() const {return d_temple_color;}

        //! Get the color associated with ruins on the smallmap.
	Gdk::RGBA getRuinColor() const {return d_ruin_color;}

        //! Get the first tile that has a certain pattern on the small map.
        Tile *getFirstTile(SmallTile::Pattern pattern) const;

        int countTilesWithPattern(SmallTile::Pattern pattern) const;

        //! Return the object containing the flight movement bonus image.
        /**
         * We name it 'All' because it means all of the movement bonuses are on at
         * the same time.
         * This doesn't get all of the move bonus images, it gets a single one
         * representing a movement bonus over all tile types.
         */
        TarFileImage *getAllMoveBonus() const {return d_all_movebonus;}

        //! Return the object containing the water movement bonus image.
        /**
         * Shown when a stack is in a boat.
         */
        TarFileImage *getWaterMoveBonus() const {return d_water_movebonus;}

        //! Return the object containing the forest movement bonus image.
        TarFileImage *getForestMoveBonus() const {return d_forest_movebonus;}

        //! Return the object containing the hills movement bonus image.
        TarFileImage *getHillsMoveBonus() const {return d_hills_movebonus;}

        //! Return the object containing the mountains movement bonus image.
        TarFileImage *getMountainsMoveBonus() const
          {return d_mountains_movebonus;}

        //! Return the object containing the swamp movement bonus image.
        TarFileImage *getSwampMoveBonus() const {return d_swamp_movebonus;}


	// Set Methods

	//! Sets the color of the road on the smallmap.
	void setRoadColor(Gdk::RGBA color) {d_road_color = color;}

	//! Sets the color of the ruins on the smallmap.
	void setRuinColor(Gdk::RGBA color) {d_ruin_color = color;}

	//! Sets the color of the temples on the smallmap.
	void setTempleColor(Gdk::RGBA color) {d_temple_color = color;}

        //! clear the tileset and add the normal tiles to it.
        void populateWithDefaultTiles();

	//Methods that operate on class data and modify the class data.

	//! Destroy the images associated with this tileset.
	void uninstantiateImages();
        
        //! Destroy all images with this name.
        void uninstantiateSameNamedImages (Glib::ustring name);

        //! Load the images associated with this tileset.
        /**
         * Go get the image files from the tileset file and create the
         * various pixmask objects.
         *
         * @param scale   The images are clamped to the tile size or not.
         * @param broken  True when things went wrong reading the tileset file.
         */
	void instantiateImages(bool scale, bool &broken);

        //! Load the tileset again.
        void reload(bool &broken);
        
        //! make a new tilestyleset from an image and add it to the tile's list.
        bool addTileStyleSet(Tile *tile, Glib::ustring filename);

	//Methods that operate on class data and do not modify the class data.

        //! Returns the index to the given terrain type.
        int getIndex(Tile::Type type) const;
        int lookupIndexByType(Tile::Type type) const;

	//! Lookup tilestyle by it's id in this tileset.
	TileStyle *getTileStyle(guint32 id) const;

        //! Return how many movement bonus images are set.
        guint32 countMoveBonusImages () const;

        //! Return how many tilestyles are in this tileset.
        guint32 countTileStyles () const;

	//! Lookup a random tile style.
	/**
	 * Scan the TileStyles for the given Tile (given by index) for a
	 * TileStyle that matches the given style.  When there is more than
	 * one TileStyle to choose from, randomly pick one from all of the 
	 * matching TileStyle objects.
	 *
	 * @param index  The index of the Tile in this set to operate on.
	 * @param style  The kind of style we're looking for.
	 *
	 * @return A pointer to the matching TileStyle object, or NULL if no 
	 *         TileStyle could be found with that given style.
	 */
	TileStyle *getRandomTileStyle(guint32 index, TileStyle::Type style) const;

	//! Save a Tileset to an opened tile configuration file.
	/**
	 * @param  The opened XML tile configuration file.
	 */
	bool save(XML_Helper *helper) const;

        bool save(Glib::ustring filename, Glib::ustring extension) const;

	//! Get a unique tile style id among all tile syles in this tileset.
	int getFreeTileStyleId() const;

	//! Get the largest tile style id of all tile styles in this tileset.
	int getLargestTileStyleId() const;

	//! Check to see if this tileset is suitable for use within the game.
	bool validate() const;

        //! Determine the most common tile size in the graphic files.
        bool calculate_preferred_tile_size(guint32 &ts) const;

        //! Where does the given tile style live?
        bool getTileStyle(guint32 id, Tile **tile, TileStyleSet **set, TileStyle ** style) const;
	  
	// Static Methods

	//! Return the default height and width of a tile in the tileset.
	static guint32 getDefaultTileSize();

	//! Create a tileset from the given tileset configuration file.
	static Tileset *create(Glib::ustring file, bool &unsupported_version);
        
        static Tileset *copy (const Tileset *orig);

        //! Rewrite old tileset files.
        static bool upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version);
        static void support_backward_compatibility();
	
        static guint32 get_default_tile_size ();


    private:
        //! Callback to load Tile objects into the Tileset.
        bool loadTile(Glib::ustring, XML_Helper* helper);

        // DATA

        //! The object containing the small and large selector animation frames
        TarFileMaskedImage *d_selector[2];

        //! The object containing the set of images that comprise the flags
        TarFileMaskedImage *d_flag;

	//! The explosion image
	/**
	 * The explosion image appears on the bigmap when stacks are fighting,
	 * and it also appears in the fight window when an army unit dies.
	 *
	 */
        TarFileImage *d_explosion;

	//! The fog images.
	/**
	 * The fog images appear on the bigmap when playing with a hidden map.
	 *
	 * The number and order of frames in the image correlates to the
	 * FogMap::ShadeType enumeration.
	 *
	 */
        TarFileImage *d_fog;

	//! The road images.
	/**
	 * The road images appear on the bigmap overlaid on top of all kinds
	 * of tiles except for water.
	 *
	 * The number and order of frames in the image correlates to the
	 * Road::Type enumeration.
	 *
	 */
        TarFileImage *d_road;

	//! The standing stone images.
	/**
	 * The stone images appear on the bigmap overlaid on top of grass
         * tiles without buildings except for roads.
	 *
	 * The number and order of frames in the image correlates to the
	 * Stone::Type enumeration.
	 *
	 */
        TarFileImage *d_stone;

	//! The bridge images.
	/**
	 * The bridge images appear on the bigmap overlaid on top of certain
	 * water tiles.
	 *
	 * The number and order of frames in the image correlates to the
	 * Bridge::Type enumeration.
	 *
	 */
        TarFileImage *d_bridge;


        typedef std::map<guint32, TileStyle*> TileStyleIdMap;
	//! A map that provides a TileStyle when supplying a TileStyle id.
        TileStyleIdMap d_tilestyles;

        typedef std::map<Tile::Type, int> TileTypeIndexMap;
	//! A map that provides an index when supplying a type of Tile.
        TileTypeIndexMap d_tiletypes;

	//! The color of roads on the smallmap.
	Gdk::RGBA d_road_color;

	//! The color of ruins on the smallmap.
	Gdk::RGBA d_ruin_color;

	//! The color of temples on the smallmap.
	Gdk::RGBA d_temple_color;

        //! The movement bonus graphic for moving quickly over all tile types.
        TarFileImage *d_all_movebonus;

        //! The movement graphic for when a stack is in a boat.
        TarFileImage *d_water_movebonus;

        //! The movement bonus graphic for moving quickly through woods.
        TarFileImage *d_forest_movebonus;

        //! The movement bonus graphic for moving quickly through hills.
        TarFileImage *d_hills_movebonus;

        //! The movement graphic for moving quickly (or at all) thru mountains.
        TarFileImage *d_mountains_movebonus;

        //! The movement bonus graphic for moving quickly through marsh.
        TarFileImage *d_swamp_movebonus;

        std::vector<TarFileImage*> getImages ();
        std::vector<TarFileMaskedImage*> getMaskedImages ();
};
#endif // TILESET_H

// End of file
