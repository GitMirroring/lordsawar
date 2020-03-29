// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2007-2011, 2014, 2017, 2020 Ben Asselstine
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
 * it's own .lwt file.
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

	//! Returns the basename of the file containing big selector images.
	Glib::ustring getLargeSelectorFilename() const
          {return d_large_selector;}

	//! Returns the basename of the file containing small selector images.
	Glib::ustring getSmallSelectorFilename() const
          {return d_small_selector;}

	//! Returns the basename of the file containing the explosion image.
	Glib::ustring getExplosionFilename() const {return d_explosion;}

	//! Returns the basename of the file containing the road images.
	Glib::ustring getRoadsFilename() const {return d_roads;}

	//! Returns the basename of the file containing the stone images.
	Glib::ustring getStonesFilename() const {return d_standing_stones;}

	//! Returns the basename of the file containing the bridge images.
	Glib::ustring getBridgesFilename() const {return d_bridges;}

	//! Returns the basename of the file containing the fog images.
	Glib::ustring getFogFilename() const {return d_fog;}

	//! Returns the basename of the file containing the flag images.
	Glib::ustring getFlagsFilename() const {return d_flags;}

        //! Get the colour associated with the road on the smallmap.
	Gdk::RGBA getRoadColor() const {return d_road_color;}

        //! Get the colour associated with temples on the smallmap.
	Gdk::RGBA getTempleColor() const {return d_temple_color;}

        //! Get the colour associated with ruins on the smallmap.
	Gdk::RGBA getRuinColor() const {return d_ruin_color;}

	//! Get the explosion image.
	PixMask *getExplosionImage() {return explosion;}

	//! Get a road image.  Pass in the index.
	PixMask *getRoadImage(guint32 i) {return roadpic[i];}

	//! Get a standing stone image.  Pass in the index.
	PixMask *getStoneImage(guint32 i) {return stonepic[i];}

	//! Get a bridge image.  Pass in the index.
	PixMask *getBridgeImage(guint32 i) {return bridgepic[i];}

	//! Get a flag image.  Pass in the index.
	PixMask *getFlagImage(guint32 i) {return flagpic[i];}

	//! Get the flag mask.  Pass in the index.
	PixMask *getFlagMask(guint32 i) {return flagmask[i];}

	//! Get the fog image.  Passin the index.
	PixMask *getFogImage(guint32 i) {return fogpic[i];}

	//! Get the big selector image.  Pass in the index.
	PixMask *getSelectorImage(guint32 i) {return selector[i];}

	//! Get the big selector mask.  Pass in the index.
	PixMask *getSelectorMask(guint32 i) {return selectormask[i];}

	//! Get the small selector image.  Pass in the index.
	PixMask *getSmallSelectorImage(guint32 i) {return smallselector[i];}

	//! Get the small selector mask.  Pass in the index.
	PixMask *getSmallSelectorMask(guint32 i) {return smallselectormask[i];}

	//! Get the number of animation frames in the big selector image.
	guint32 getNumberOfSelectorFrames() {return number_of_selector_frames;}

	//! Get the number of animation frames in the small selector image.
	guint32 getNumberOfSmallSelectorFrames() {return number_of_small_selector_frames;}

        //! Get the first tile that has a certain pattern on the small map.
        Tile *getFirstTile(SmallTile::Pattern pattern) const;

        int countTilesWithPattern(SmallTile::Pattern pattern) const;

	// Set Methods

	//! Sets the basename of the file containing the big selector images.
	void setLargeSelectorFilename(Glib::ustring p){d_large_selector = p;}

	//! Sets the basename of the file containing the small selector images.
	void setSmallSelectorFilename(Glib::ustring p){d_small_selector = p;}

	//! Sets the basename of the file containing the explosion image.
	void setExplosionFilename(Glib::ustring p){d_explosion = p;}

	//! Sets the basename of the file containing the road images.
	void setRoadsFilename(Glib::ustring p){d_roads = p;}

	//! Sets the basename of the file containing the standing stone images.
	void setStonesFilename(Glib::ustring p){d_standing_stones = p;}

	//! Sets the basename of the file containing the bridge images.
	void setBridgesFilename(Glib::ustring p){d_bridges = p;}

	//! Sets the basename of the file containing the fog images.
	void setFogFilename(Glib::ustring p){d_fog = p;}

	//! Sets the basename of the file containing the flag images.
	void setFlagsFilename(Glib::ustring p){d_flags = p;}

	//! Sets the colour of the road on the smallmap.
	void setRoadColor(Gdk::RGBA color) {d_road_color = color;}

	//! Sets the colour of the ruins on the smallmap.
	void setRuinColor(Gdk::RGBA color) {d_ruin_color = color;}

	//! Sets the colour of the temples on the smallmap.
	void setTempleColor(Gdk::RGBA color) {d_temple_color = color;}

	//! Sets the explosion image.
	void setExplosionImage(PixMask *p) {explosion = p;}

	//! Sets a road image.
	void setRoadImage(guint32 i, PixMask *p) {roadpic[i] = p;}

	//! Sets a stpone image.
	void setStoneImage(guint32 i, PixMask *p) {stonepic[i] = p;}

	//! Sets a bridge image.
	void setBridgeImage(guint32 i, PixMask *p) {bridgepic[i] = p;}

	//! Sets a flag image.
	void setFlagImage(guint32 i, PixMask *p) {flagpic[i] = p;}

	//! Sets a flag mask.
	void setFlagMask(guint32 i, PixMask *p) {flagmask[i] = p;}

	//! Sets a fog image.
	void setFogImage(guint32 i, PixMask *p) {fogpic[i] = p;}

	//! Sets a big selector image.
	void setSelectorImage(guint32 i, PixMask *p) {selector[i] = p;}

	//! Sets a big selector mask.
	void setSelectorMask(guint32 i, PixMask *p) {selectormask[i] = p;}

	//! Sets a small selector image.
	void setSmallSelectorImage(guint32 i, PixMask *p) {smallselector[i] = p;}
	//! Sets a small selector mask.
	void setSmallSelectorMask(guint32 i, PixMask *p) {smallselectormask[i] = p;}

	//! Sets the number of animation frames in the big selector.
	void setNumberOfSelectorFrames(guint32 s) {selector.reserve(s); selectormask.reserve(s); number_of_selector_frames = s;}

	//! Sets the number of animation frames in the small selector.
	void setNumberOfSmallSelectorFrames(guint32 s) {smallselector.reserve(s);smallselectormask.reserve(s); number_of_small_selector_frames = s;}

        void clearRoadsImage (bool clear_name = true);
        void clearStonesImage (bool clear_name = true);
        void clearBridgesImage (bool clear_name = true);
        void clearFlagsImage (bool clear_name = true);
        void clearSmallSelectorImage (bool clear_name = true);
        void clearLargeSelectorImage (bool clear_name = true);
        void clearExplosionImage (bool clear_name = true);
        void clearFogImages (bool clear_name = true);
        bool instantiateRoadImages();
        bool instantiateStoneImages();
        bool instantiateFlagImages();
        bool instantiateBridgeImages();
        bool instantiateSmallSelectorImages();
        bool instantiateLargeSelectorImages();
        bool instantiateExplosionImage();
        bool instantiateFogImages();

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

	//! Load the various images from the given filenames.
	void instantiateImages(Glib::ustring explosion_filename,
			       Glib::ustring roads_filename,
			       Glib::ustring stones_filename,
			       Glib::ustring bridges_filename,
			       Glib::ustring fog_filename,
			       Glib::ustring flags_filename,
			       Glib::ustring selector_filename,
			       Glib::ustring small_selector_filename,
                               bool scale, bool &broken);
        // DATA

	//! The basename of the small selector image.
	/**
	 * The small selector is the graphic that appears on the bigmap when
	 * a stack is selected that only has one army unit in it.
	 *
	 * The image contains many animation frames, and is masked.
	 *
	 * This basename does not contain any slashes, and it does not contain
	 * a file extension.  It refers to a png file in the directory of
	 * tileset.
	 */
	Glib::ustring d_small_selector;

	//! The basename of the large selector image.
	/**
	 * The large selector is the graphic that appears on the bigmap when
	 * a stack is selected that only has more than one army unit in it.
	 *
	 * The image contains many animation frames, and is masked.
	 *
	 * This basename does not contain any slashes, and it does not contain
	 * a file extension.  It refers to a png file in the directory of
	 * tileset.
	 */
	Glib::ustring d_large_selector;

	//! The basename of the explosion image.
	/**
	 * The explosion image appears on the bigmap when stacks are fighting,
	 * and it also appears in the fight window when an army unit dies.
	 *
	 * This basename does not contain any slashes, and it does not contain
	 * a file extension.  It refers to a png file in the directory of
	 * tileset.
	 */
	Glib::ustring d_explosion;

	//! The basename of the fog image.
	/**
	 * The fog images appear on the bigmap when playing with a hidden map.
	 *
	 * The number and order of frames in the image correlates to the
	 * FogMap::ShadeType enumeration.
	 *
	 * This basename does not contain any slashes, and it does not contain
	 * a file extension.  It refers to a png file in the directory of
	 * tileset.
	 */
	Glib::ustring d_fog;

	//! The basename of the road image.
	/**
	 * The road images appear on the bigmap overlaid on top of all kinds
	 * of tiles except for water.
	 *
	 * The number and order of frames in the image correlates to the
	 * Road::Type enumeration.
	 *
	 * This basename does not contain any slashes, and it does not contain
	 * a file extension.  It refers to a png file in the directory of
	 * tileset.
	 */
	Glib::ustring d_roads;

	//! The basename of the standing stone image.
	/**
	 * The stone images appear on the bigmap overlaid on top of grass
         * tiles without buildings except for roads.
	 *
	 * The number and order of frames in the image correlates to the
	 * Stone::Type enumeration.
	 *
	 * This basename does not contain any slashes, and it does not contain
	 * a file extension.  It refers to a png file in the directory of
	 * tileset.
	 */
	Glib::ustring d_standing_stones;

	//! The basename of the bridge image.
	/**
	 * The bridge images appear on the bigmap overlaid on top of certain
	 * water tiles.
	 *
	 * The number and order of frames in the image correlates to the
	 * Bridge::Type enumeration.
	 *
	 * This basename does not contain any slashes, and it does not contain
	 * a file extension.  It refers to a png file in the directory of
	 * tileset.
	 */
	Glib::ustring d_bridges;

	//! The basename of the flag image.
	/**
	 * The flag images appear on the bigmap beside a stack to indicate the
	 * number of army units in the stack.
	 *
	 * The number of frames in the image corresponds to the maximum number
	 * of army units in a stack.  See the FLAG_TYPES constant in defs.h.
	 *
	 * This basename does not contain any slashes, and it does not contain
	 * a file extension.  It refers to a png file in the directory of
	 * tileset.
	 */
	Glib::ustring d_flags;

        typedef std::map<guint32, TileStyle*> TileStyleIdMap;
	//! A map that provides a TileStyle when supplying a TileStyle id.
        TileStyleIdMap d_tilestyles;

        typedef std::map<Tile::Type, int> TileTypeIndexMap;
	//! A map that provides an index when supplying a type of Tile.
        TileTypeIndexMap d_tiletypes;

	//! The colour of roads on the smallmap.
	Gdk::RGBA d_road_color;

	//! The colour of ruins on the smallmap.
	Gdk::RGBA d_ruin_color;

	//! The colour of temples on the smallmap.
	Gdk::RGBA d_temple_color;

	//! The road images.
        PixMask* roadpic[ROAD_TYPES];

	//! The standing stone images.
        PixMask* stonepic[STONE_TYPES];

	//! The bridge images.
        PixMask* bridgepic[BRIDGE_TYPES];

	//! The flag images.
        PixMask* flagpic[FLAG_TYPES];

	//! The flag masks.
        PixMask* flagmask[FLAG_TYPES];

	//! The number of animation frames in the big selector.
	guint32 number_of_selector_frames;

	//! The image frames in the big selector.
	std::vector<PixMask* > selector;

	//! The mask frames of the big selector.
	std::vector<PixMask* > selectormask;

	//! The number of animation frames in the small selector.
	guint32 number_of_small_selector_frames;

	//! The image frames of the small selector.
	std::vector<PixMask* > smallselector;

	//! The mask frames of the small selector.
	std::vector<PixMask* > smallselectormask;

	//! The exposion image.
	PixMask* explosion;

	//! The fog images.
	PixMask*fogpic[FOG_TYPES];
};
#endif // TILESET_H

// End of file
