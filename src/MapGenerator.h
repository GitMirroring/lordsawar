// Copyright (C) 2002 Vibhu Rishi
// Copyright (C) 2002, 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2004 David Barnsdale
// Copyright (C) 2004 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2014 Ben Asselstine
// Copyright (C) 2008 Janek Kozicki
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
#ifndef MAPGENERATOR_H
#define MAPGENERATOR_H

#include <deque>
#include "vector.h"
#include <vector>
#include <sigc++/signal.h>

// we need the enums from these classes
#include "Tile.h"
#include "maptile.h"

class Cityset;
class RoadPathCalculator;
class Path;

//! Randomly generate terrain and buildings on a map.
/** This class creates a map including the buildings (cities, ruins, temples).
  * It does NOT care about player setup and such. This is done in a second step
  * by the CreateScenario class. The parameters for map creation are set by
  * GamePreferencesDialog::fillData().
  * GamePreferencesDialog does use the MapConfDialog but this is
  * just a skeleton - GamePreferencesDialog does all the work.
  *
  * The data is stored in two variables:
  *   - the terrain map stores the terrain information
  *   - the building map stores the information about building placement
  *
  * For historical reasons *cough*, the maps are long character arrays. The
  * definition of the single characters can be found further down; missing
  * buildings are denoted by a space. As a normal user, you will propably never
  * need to worry about the format anyway. It is read and "translated" by the
  * CreateScenario class which kind of supervises the scenario creation.
  *
  * Map generators with different styles (e.g. islands), derived from
  * this class are possible and will hpefully be done in the
  * future. 
  * 
  * When designating sites for cities, it takes care to ensure that 
  * the first cities on any land mass will be ports on those seas
  * that connect with other land masses and islands. In doing so, it
  * creates two data structures that may be useful elsewhere.
  * 
  */
class MapGenerator
{
    public :
        MapGenerator();
        ~MapGenerator();

	//! Set the cityset that will be used to generate the map.
	/**
	 * the cityset tells us how many tiles a city takes up.
	 */
	void setCityset(Cityset *cs) {cityset = cs;};

        //! Set the number of cities
        int setNoCities(int nocities);

        //! Set the number of ruins
        int setNoRuins(int noruins);

        //! Set the number of ruins
        int setNoSignposts(int nosignposts);

        //! Set the number of temples
        int setNoTemples(int notemples);

        /** Set terrain distribution. If the sum of the percentages is less than
          * 100, the rest is filled with grass.
          */
        void setPercentages(int pwater, int pforest, int pswamp,
                            int phills, int pmountains);


        //! Get number of cities
        int getNoCities() const {return d_nocities;}

        //! Get number of ruins
        int getNoRuins() const {return d_noruins;}

        //! Get number of signposts
        int getNoSignposts() const {return d_nosignposts;}

        //! Get number of temples
        int getNoTemples() const {return d_notemples;}


        /** Creates a map
          * 
          * Use this function to start off the map generation. This
          * implementation will first distribute the terrain in random patches,
          * normalize it and then distribute the buildings.
          *
          * @param width        the width of the map to be generated
          * @param height       the height of the map to be generated
          */
        void makeMap(int width, int height, bool roads);

        /** A debug function, prints map to std::cout, using convention mentioned
          * at MapGenerator::makeMap
          */
        void printMap(int j, int i);
        void printMap();

        /** Get the array for the terrain map (shallow copy)
          * 
          * @param width        is set to the width of the generated map
          * @param height       is set to the height of the generated map
          * @return char array which represents the terrain map
          */
        const Tile::Type* getMap(int& width, int& height) const;

        /** Get the buildings map (shallow copy)
          * 
          * @param width        is set to the width of the generated map
          * @param height       is set to the height of the generated map
          * @return char array which represents the building map
          */
        const Maptile::Building* getBuildings(int& width, int& height) const;

        /**
         * @param fraction How far along the progress bar should be.
         * @param status   A description of what's being generated.
         */
        //! Emitted when the generator generates something
        sigc::signal<void, double, Glib::ustring> progress;

    protected:
        //! Fills the terrain map with grass
        void makePlains();

        /** Spreads terrain over the map
          * 
          * This function randomly places a given terrain type over the map.
          * It only "overwrites" grass tiles, so terrain is not repeatedly 
          * modified.
          *
          * @param t        terrain to be placed
          * @param percent  amount of tiles to be modified (percentage of the
          *                 whole map)
          * @param contin   if set to true (used for water tiles), the algorithm
          *                 tries to create continuous areas instead of leaving
          *                 patches when it hits a dead end.
          */
        void makeTerrain(Tile::Type t, int percent, bool contin);
        void makeStreamer(Tile::Type type, int percent, int width);

        /**
          * Water is special - in real world we usually have one big ocean, and
          * everything flowing into it with rivers. To avoid generating lots of
          * unconnected bodies of water (those should be swamps in general,
          * maybe a pond) we need to find biggest body of water and connect
          * others with it, using rivers. Ponds are allowed, but not too much of them.
          */
        void makeRivers();
        /**
          * This helper function searches whole map for enclosed ares of
          * THIS_TILE. Each such area gets a subsequent number which is
          * assigned to corresponding cell in box. Maximum number of areas
          * found is how_many and equals to highest number found in box.
          */ 
        void findAreasOf(Tile::Type THIS_TILE,std::vector<std::vector<int> >& box,int& how_many);
        /**
          * Too much randomness and rivers can create too many small islands
          * which normally would be eroded by water along the centuries of
          * passing time. This function eliminates those islands keeping only
          * few of them. The randomly generated map looks more realistic in this way.
          */
        void verifyIslands();

        void makeBridges();
	bool findBridgePurpose(Vector<int> pos, int type, Vector<int> &src, Vector<int> &dest);
	bool canPlaceBridge(Vector<int> pos, int type, Vector<int> &src, Vector<int> &dest);
	void placeBridge(Vector<int> pos, int type);
        

        //! put standing stones on grassy tiles.
        void  makeStandingStones();

        /**
          * Once makeRivers() finds a connection path between two bodies of water
          * it calls this function to put water on that path.
          */
        void connectWithWater(Vector<int> from, Vector<int> to);

        /** Mountains are specific - they must always be surrounded by hills,
          * othwerwise the map graphically looks bad. Besides who has ever seen
          * a mountain without even a tiny amount of hills around?
          *
          * This means that a lone montains becomes surrounded by hills and
          * is not lone anymore. See GameMap::are_those_tiles_similar().
          */
        void surroundMountains(int minx, int maxx, int miny, int maxy);

        /** Paving roads and putting cities can create lone mountains.
          * Making rivers can also create lone tiles.
          * We need to rescue them!
          */
        void rescueLoneTiles(Tile::Type FIND_THIS, Tile::Type REPLACE, bool grow);

        /** Tries to find the nearest grass tile from a given location.
          *
          * This function becomes e.g. neccessary if you want to create 
          * connected water areas, but the algorithm in makeTerrain() gets
          * stuck.
          *
          * @param x    the x position where we are to look
          * @param y    the y position where we are to look
          *
          * @note if the function finds a free place, it modifies x and y.
          * @return true if search succeeded.
          */
        bool seekPlain(int& x,int& y);

        /** Designates the places where the cities will be by setting characters 
          * on the building map.  Creates an island if it
          * finds only water or occupied places.
          *
          * @param cities       the number of cities to distribute
          */
        void makeCities(int cities);

        //! Returns true if position (x,y) is free for a city
        bool canPutCity(int x, int y);

        //! Places a city at a certain location
        void putCity(int x, int y, int& city_count);

        /** Designates the location of buildings across the map. Creates an island 
          * if there is only water or occupied places.
          *
          * @param b            the type of the building
          * @param building     the number of buildings to distribute
          */
        void makeBuildings(Maptile::Building b, int building);

        //! Returns true if position (x,y) is free for buildings
        bool canPutBuilding(int x, int y);

        /** Tries to place a city around a certain position.
          * 
          * This function is used internally for placing ports, that
          * is why some of the parameters may be a bit strange.
          * 
          * @param px           x coordinate of position
          * @param py           y coordinate
          * @param city_count   increased if the city is placed
          */
        bool tryToPlaceCity(int px, int py, int& city_count);


        /** Normalizes the terrain
          * 
          * With normalization, we mean that if a tile is surrounded mainly
          * by tiles different from it's own type, then there is a chance
          * that the tile in question is modified itself. This leads to
          * "better" terrain distribution.
          *
          * @note At the moment, only water is normalized
          */
        void normalize();

        void cleanupRoads();
	bool makeRoad(Vector<int> src, Vector<int>dest);
        bool makeRoad2(Path *p);
	bool makeAccessible(RoadPathCalculator *pcl, RoadPathCalculator *pcf, Vector<int> dest);
	void makeRoads();
        void makeCitiesAccessible();

        /** Find all places where it's possible to place a bridge:
          *
          * Bridge length is 2 tiles. So we can find two options:
          *
          * type=1       type=2
          *
          * +-+-+-+      +-+-+-+-+
          * |0|.|.|      |0|=|=|.|     x-----> width - i
          * +-+-+-+      +-+-+-+-+     |
          * |=|=|=|      |.|=|=|.|     | 
          * +-+-+-+  or  +-+-+-+-+    \|/
          * |=|=|=|      |.|=|=|.|     '
          * +-+-+-+      +-+-+-+-+   height - j
          * |.|.|.|               
          * +-+-+-+               
          *
          * this functions returns a randommly sorted vector of all possible
          * places for bridges in
          *
          * std::vector<std::pair<int , Vector<int> > >
          *    .first is type, 
          *    .second is (j,i) coordinates of '0' point in there.
          *
          * The vector is randomly sorted, so just pick the first few - as much
          * as you need, and they will be in random (probably in not close to
          * each other locations).
          *
          * The output is checked and "duplicate" brigdes (which are close to
          * each other - because the river is straight) are removed.
          */ 
        std::vector<std::pair < int , Vector<int> > > findBridgePlaces();
	bool placePort(int x, int y);
	void calculateBlockedAvenue(int x, int y);
	bool inhospitableTerrain(int x, int y, unsigned int width);


        //Data
        int d_xdir[8];
        int d_ydir[8];
        Tile::Type* d_terrain; // the map of the terrain
        Maptile::Building* d_building;
        int d_width;
        int d_height;
        int d_pswamp, d_pwater, d_pforest, d_phills, d_pmountains;
        unsigned int d_nocities, d_notemples, d_noruins, d_nosignposts;
	Cityset *cityset;
};

#endif

// End of file
