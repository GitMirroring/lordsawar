// Copyright (C) 2002 Vibhu Rishi
// Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004 David Barnsdale
// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2014, 2015, 2017,
// 2020 Ben Asselstine
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

#include <iostream>
#include <algorithm>
#include <math.h>  
#include <set>

#include "MapGenerator.h"
#include "army.h"
#include "GameMap.h"
#include "stack.h"
#include "path.h"
#include "File.h"
#include "citylist.h"
#include "city.h"
#include "roadlist.h"
#include "road.h"
#include "stonelist.h"
#include "stone.h"
#include "portlist.h"
#include "port.h"
#include "ruinlist.h"
#include "ruin.h"
#include "templelist.h"
#include "temple.h"
#include "bridgelist.h"
#include "bridge.h"
#include "armysetlist.h"
#include "tilesetlist.h"
#include "vector.h"
#include "RoadPathCalculator.h"
#include "cityset.h"
#include "overviewmap.h"
#include "rnd.h"
#include "signpostlist.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)
#define offmap(bx,by) (by<0)||(by>=d_height)||(bx<0)||(bx>=d_width)

MapGenerator::MapGenerator()
 :d_terrain(0), d_building(0), d_width (MAP_SIZE_NORMAL_WIDTH),
    d_height (MAP_SIZE_NORMAL_HEIGHT), d_pswamp(2), d_pwater(25), d_pforest(3),
    d_phills(5), d_pmountains(5), d_nocities(11), d_notemples(9), d_noruins(20),
    d_nosignposts(30), d_nostones (40), d_stone_road_chance (ROAD_STONE_CHANCE),
    cityset(NULL)
{
  d_xdir[0]=0;d_xdir[1]=-1;d_xdir[2]=-1;d_xdir[3]=-1;d_xdir[4]=0;d_xdir[5]=1;d_xdir[6]=1;d_xdir[7]=1;
  d_ydir[0]=-1;d_ydir[1]=-1;d_ydir[2]=0;d_ydir[3]=1;d_ydir[4]=1;d_ydir[5]=1;d_ydir[6]=0;d_ydir[7]=-1;
}

MapGenerator::~MapGenerator()
{
    if (d_terrain)
        delete[] d_terrain;
    if (d_building)
        delete[] d_building;
}

int MapGenerator::setNoCities(int nocities)
{
    if (nocities < 0)
        return -1;

    int tmp = d_nocities;
    d_nocities = nocities;
    return tmp;
}

int MapGenerator::setNoRuins(int noruins)
{
    if (noruins < 0)
        return -1;

    int tmp = d_noruins;
    d_noruins = noruins;
    return tmp;
}

int MapGenerator::setNoSignposts(int nosignposts)
{
    if (nosignposts < 0)
        return -1;

    int tmp = d_nosignposts;
    d_nosignposts = nosignposts;
    return tmp;
}

int MapGenerator::setNoStones (int nostones)
{
    if (nostones< 0)
        return -1;

    int tmp = d_nostones;
    d_nostones = nostones;
    return tmp;
}

int MapGenerator::setChanceOfStoneOnRoad (int chance)
{
    if (chance <= 0)
        return -1;

    int tmp = d_stone_road_chance;
    d_stone_road_chance = chance;
    return tmp;
}

int MapGenerator::setNoTemples(int notemples)
{
    if (notemples < 0)
        return -1;

    int tmp = d_notemples;
    d_notemples = notemples;
    return tmp;
}

void MapGenerator::setPercentages(int pwater, int pforest, int pswamp,
                                    int phills, int pmountains)
{
    if ((pswamp < 0) || (pwater < 0) || (pforest < 0) || (phills < 0)
        || (pmountains < 0))
        return;

    if (pswamp + pwater + pforest + phills + pmountains > 100)
        return;

    d_pswamp = pswamp;
    d_pwater = pwater;
    d_pforest = pforest;
    d_phills = phills;
    d_pmountains = pmountains;
}

void MapGenerator::cleanupRoads()
{
  /*
   * in rare cases, we can make roads on water.  this routine gets rid of them.
   */
  for (int i = 0; i < d_height; i++)
    for (int j = 0; j < d_width; j++)
      {
        if (d_building[i*d_width + j] == Maptile::ROAD &&
            d_terrain[i*d_width + j] == Tile::WATER)
          d_building[i*d_width + j] = Maptile::NONE;
      }
}

/** 
 * Generates a random map.  The map is stored as a char array of size 
 * 100x100 or whatever.  Each character stands for something.  We use:
 * M = mountains
 * h = hills
 * ~ = water
 * $ = forest
 * . = plains
 * _ = swamps
 *
 * The buildings are handled separately.
 *
 * See printMap() which is used for debugging maps.
 */
void MapGenerator::makeMap(int width, int height, bool roads)
{
    d_width = width;
    d_height = height;
    d_road_stones.clear ();

    //initialize terrain and building arrays
    d_terrain = new Tile::Type[width*height];
    d_building = new Maptile::Building[width*height];
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            d_building[i*width + j] = Maptile::NONE;

    debug("Making random map:");
   
    // create the terrain
    debug("flattening plains");
    progress.emit(.070);
    makePlains();

    debug("raining water");
    progress.emit(.140);
    makeTerrain(Tile::WATER, d_pwater, true);  
    makeStreamer(Tile::WATER, d_pwater/3, 3);
    rescueLoneTiles(Tile::WATER, Tile::GRASS,true);
    makeRivers();
    verifyIslands();

    debug("raising hills");
    progress.emit(.210);
    makeTerrain(Tile::HILLS, d_phills, false);

    debug("raising mountains");
    progress.emit(.280);
    makeTerrain(Tile::MOUNTAIN, d_pmountains, false);
    makeStreamer(Tile::MOUNTAIN, d_pmountains/3, 3);
    rescueLoneTiles(Tile::MOUNTAIN,Tile::GRASS,false);
    surroundMountains(0, d_width, 0, d_height);

    debug("planting forest");
    progress.emit(.350);
    makeTerrain(Tile::FOREST, d_pforest, false);

    debug("watering swamps");
    progress.emit(.420);
    makeTerrain(Tile::SWAMP, d_pswamp, false);

    debug("normalizing terrain");
    progress.emit(.490);
    normalize();

    // place buildings
    debug("building cities");
    progress.emit(.560);
    makeBuildings (d_nocities, cityset->getCityTileWidth (),
                   sigc::mem_fun(this, &MapGenerator::placeCity));

    if (roads)
      {
	debug("paving roads");
	progress.emit(.630);
	makeRoads();
      }

    debug("ruining ruins");
    progress.emit(.700);
    makeBuildings (d_noruins, cityset->getRuinTileWidth (),
                   sigc::mem_fun(this, &MapGenerator::placeRuin));

    debug("spawning temples");
    progress.emit(.770);
    makeBuildings (d_notemples, cityset->getTempleTileWidth (),
                   sigc::mem_fun(this, &MapGenerator::placeTemple));

    debug("building bridges");
    if (roads == true)
      {
        progress.emit(.840);
        makeBridges();
      }
    cleanupRoads();

    debug("raising signs");
    progress.emit(.910);
    makeBuildings (d_nosignposts, 1,
                   sigc::mem_fun(this, &MapGenerator::placeSign));

    rescueLoneTiles(Tile::MOUNTAIN, Tile::HILLS, false);

    progress.emit(.980);
    makeCitiesAccessible();

    progress.emit(.990);
    d_road_stones = makeStandingStones(roads);

    progress.emit(1.0);
    debug("Done making map.");
}

#define NORTH_SOUTH_BRIDGE 1
#define EAST_WEST_BRIDGE 2

void MapGenerator::placeBridge(Vector<int> pos, int type)
{
  if (type == NORTH_SOUTH_BRIDGE)
    {
      d_building[pos.y*d_width + pos.x] = Maptile::BRIDGE;
      d_building[(pos.y + 1)*d_width + pos.x] = Maptile::BRIDGE;
      Bridgelist::getInstance()->add(new Bridge(Vector<int>(pos.x, pos.y)));
      Bridgelist::getInstance()->add(new Bridge(Vector<int>(pos.x+1, pos.y)));
    }
  else if (type == EAST_WEST_BRIDGE)
    {
      d_building[pos.y*d_width + pos.x] = Maptile::BRIDGE;
      d_building[pos.y*d_width + pos.x + 1] = Maptile::BRIDGE;
      Bridgelist::getInstance()->add(new Bridge(Vector<int>(pos.x, pos.y)));
      Bridgelist::getInstance()->add(new Bridge(Vector<int>(pos.x, pos.y+1)));
    }
  GameMap::getInstance()->calculateBlockedAvenues();
}

bool MapGenerator::findBridgePurpose(Vector<int> pos, int type, 
				     Vector<int> &src, Vector<int> &dest)
{
  if (type == EAST_WEST_BRIDGE)
    {
      src = GameMap::getInstance()->findNearestObjectToTheWest(pos);
      dest = GameMap::getInstance()->findNearestObjectToTheEast(pos);
    }
  else if (type == NORTH_SOUTH_BRIDGE)
    {
      src = GameMap::getInstance()->findNearestObjectToTheNorth(pos);
      dest = GameMap::getInstance()->findNearestObjectToTheSouth(pos);
    }
  if (src == Vector<int>(-1,-1) || dest == Vector<int>(-1,-1))
    return false;
  if (dist (src, dest) <= (int)cityset->getCityTileWidth())
    return false;
  if ((unsigned int)GameMap::getWidth() > MAP_SIZE_NORMAL_WIDTH ||
      (unsigned int)GameMap::getHeight() > MAP_SIZE_NORMAL_HEIGHT)
    {
      int d = dist (src, dest);
      if (d >
          std::max (GameMap::getWidth() / 6, GameMap::getHeight() / 6))
        return false;
    }
  return true;
}

bool MapGenerator::canPlaceBridge(Vector<int> pos, int type, Vector<int> &src, Vector<int> &dest)
{
  if (d_building[pos.y*d_width + pos.x] == Maptile::NONE &&
      findBridgePurpose(pos, type, src, dest) == true)
    return true;
  return false;
}

std::vector<Vector<int> > MapGenerator::makeStandingStones(bool also_roads)
{
  std::vector<Vector<int> > road_stones;
  Glib::ustring orig_tileset = GameMap::getInstance()->getTilesetBaseName();
  Glib::ustring orig_shieldset = GameMap::getInstance()->getShieldsetBaseName();
  Glib::ustring orig_cityset = GameMap::getInstance()->getCitysetBaseName();
  Citylist::deleteInstance ();
  Roadlist::deleteInstance();
  Ruinlist::deleteInstance ();
  Templelist::deleteInstance ();
  Signpostlist::deleteInstance ();
  Stonelist::deleteInstance();

  GameMap::setWidth(d_width);
  GameMap::setHeight(d_height);
  GameMap::getInstance("default", "default", "default")->fill(this);

  //the game map class smooths the map, so let's take what it smoothed.
  for (int y = 0; y < d_height; y++)
    for (int x = 0; x < d_width; x++)
      d_terrain[y*d_width + x] = 
        GameMap::getInstance()->getTile(x, y)->getType();
  for (int y = 0; y < d_height; y++)
    for (int x = 0; x < d_width; x++)
      {
        Vector<int> pos = Vector<int>(x,y);
	if (d_building[y*d_width + x] == Maptile::CITY)
	  Citylist::getInstance()->add
	    (new City(pos, cityset->getCityTileWidth()));
	else if (d_building[y*d_width + x] == Maptile::ROAD)
	  Roadlist::getInstance()->add(new Road(pos));
	else if (d_building[y*d_width + x] == Maptile::RUIN)
	  Ruinlist::getInstance()->add
	    (new Ruin(pos, cityset->getRuinTileWidth()));
	else if (d_building[y*d_width + x] == Maptile::TEMPLE)
	  Templelist::getInstance()->add
	    (new Temple(pos, cityset->getTempleTileWidth()));
	else if (d_building[y*d_width + x] == Maptile::SIGNPOST)
	  Signpostlist::getInstance()->add(new Signpost(pos));
      }

  std::vector<Vector<int> > grass;
  for (int i = 0; i < d_height; i++)
    for (int j = 0; j < d_width; j++)
      {
        Vector<int> pos = Vector<int> (j, i);
        bool city = Citylist::getInstance ()->getObjectAt (pos) != NULL;
        bool temple = Templelist::getInstance ()->getObjectAt (pos) != NULL;
        bool ruin = Ruinlist::getInstance ()->getObjectAt (pos) != NULL;
        if (d_terrain[i*d_width + j] == Tile::GRASS &&
            GameMap::getInstance()->getBuilding (pos) == Maptile::NONE &&
            !city && !temple && !ruin)
          grass.push_back (pos);
      }

  std::random_shuffle (grass.begin (), grass.end ());
  unsigned int limit = d_nostones;
  if (grass.size () < limit)
    limit = grass.size ();
  for (unsigned int k = 0; k < limit; k++)
    {
      Vector<int> pos = grass[k];
      int i = pos.x;
      int j = pos.y;
      d_building[j*d_width + i] = Maptile::STONE;
    }

  if (also_roads && d_stone_road_chance > 0)
    {
      std::vector<Vector<int> > roads;
      for (int i = 0; i < d_height; i++)
        for (int j = 0; j < d_width; j++)
          if (d_terrain[i*d_width + j] == Tile::GRASS &&
              d_building[i*d_width + j] == Maptile::ROAD)
            roads.push_back (Vector<int>(j, i));
      for (auto pos : roads)
        if (Rnd::rand() % d_stone_road_chance == 0)
          road_stones.push_back (pos);
    }
  Roadlist::deleteInstance();
  Ruinlist::deleteInstance();
  Templelist::deleteInstance();
  GameMap::deleteInstance();
  Citylist::deleteInstance();
  Signpostlist::deleteInstance();
  Stonelist::deleteInstance();
  Bridgelist::deleteInstance();
  GameMap::getInstance(orig_tileset, orig_shieldset, orig_cityset);
  return road_stones;
}


void MapGenerator::makeBridges()
{
  Glib::ustring orig_tileset = GameMap::getInstance()->getTilesetBaseName();
  Glib::ustring orig_shieldset = GameMap::getInstance()->getShieldsetBaseName();
  Glib::ustring orig_cityset = GameMap::getInstance()->getCitysetBaseName();
  GameMap::deleteInstance();
  Citylist::deleteInstance();
  Roadlist::deleteInstance();
  Ruinlist::deleteInstance();
  Templelist::deleteInstance();
  Portlist::deleteInstance();
  Bridgelist::deleteInstance();

  GameMap::setWidth(d_width);
  GameMap::setHeight(d_height);
  GameMap::getInstance("default", "default", "default")->fill(this);

  //the game map class smooths the map, so let's take what it smoothed.
  for (int y = 0; y < d_height; y++)
    for (int x = 0; x < d_width; x++)
      d_terrain[y*d_width + x] = 
        GameMap::getInstance()->getTile(x, y)->getType();

  //load up the roadlist, and stuff.

  for (int y = 0; y < d_height; y++)
    for (int x = 0; x < d_width; x++)
      {
	if (d_building[y*d_width + x] == Maptile::CITY)
	  Citylist::getInstance()->add
	    (new City(Vector<int>(x,y), cityset->getCityTileWidth()));
	else if (d_building[y*d_width + x] == Maptile::ROAD)
	  Roadlist::getInstance()->add(new Road(Vector<int>(x,y)));
	else if (d_building[y*d_width + x] == Maptile::RUIN)
	  Ruinlist::getInstance()->add
	    (new Ruin(Vector<int>(x,y), cityset->getRuinTileWidth()));
	else if (d_building[y*d_width + x] == Maptile::TEMPLE)
	  Templelist::getInstance()->add
	    (new Temple(Vector<int>(x,y), cityset->getTempleTileWidth()));
	else if (d_building[y*d_width + x] == Maptile::PORT)
	  Portlist::getInstance()->add(new Port(Vector<int>(x,y)));
      }
  GameMap::getInstance()->calculateBlockedAvenues();

  Vector<int> src, dest;
  std::vector<std::pair<int , Vector<int> > >  bridges;
  bridges = findBridgePlaces();
  int bridges_laid = 0;
  for (std::vector<std::pair<int, Vector<int> > >::iterator it = bridges.begin();
       it != bridges.end(); ++it)
    {
      Vector<int> pos = (*it).second + Vector<int>(1,1);
      Vector<int> edge1;
      Vector<int> edge2;
      if ((*it).first == NORTH_SOUTH_BRIDGE)
        {
          edge1 = pos - Vector<int>(0, 1);
          edge2 = pos + Vector<int>(0, 2);
        }
      else if ((*it).first == EAST_WEST_BRIDGE)
        {
          edge1 = pos - Vector<int>(1, 0);
          edge2 = pos + Vector<int>(2, 0);
        }
      if (offmap(edge1.x, edge1.y) || offmap(edge2.x, edge2.y))
        continue;

      if (canPlaceBridge((*it).second + Vector<int>(1,1), (*it).first, src, 
                         dest) == true)
        {
          RoadPathCalculator pc_src (src);
          RoadPathCalculator pc_dest (dest);
          guint32 leg1 = 0, leg2 = 0, shortcut = 0;
          guint32 original_route = pc_src.calculate_moves (dest);
          Path *path_leg1 = pc_src.calculate (edge1, leg1);
          Path *path_leg2 = pc_dest.calculate (edge2, leg2);
          shortcut = leg1 + leg2 + 2;
          bool construct_bridge_and_roads = true;
          if (leg1 == 0 || leg2 == 0)
            construct_bridge_and_roads = false;
          /* we haven't made everything accessible yet,
             so leg1 or leg2 can be 0 */
          else if (shortcut > 0 && original_route < shortcut)
            {
              /* the idea here is that we know the bridge doesn't make it
                 faster, but we want to make bridges anyway, but not
                 when it is stupefyingly needless */
              construct_bridge_and_roads = false;
              if (original_route / 2 < shortcut)
                if (Rnd::rand() % 2 == 0)
                  construct_bridge_and_roads = true;
            }

          if (construct_bridge_and_roads)
            {
              makeRoad2 (path_leg1);
              makeRoad2 (path_leg2);
              placeBridge (pos, (*it).first);
              bridges_laid++;
              if (bridges_laid > 7)
                break;
            }
          if (path_leg1)
            delete path_leg1;
          if (path_leg2)
            delete path_leg2;
        }
    }

  Roadlist::deleteInstance();
  Ruinlist::deleteInstance();
  Templelist::deleteInstance();
  GameMap::deleteInstance();
  Citylist::deleteInstance();
  Portlist::deleteInstance();
  Bridgelist::deleteInstance();
  GameMap::getInstance(orig_tileset, orig_shieldset, orig_cityset);
}

void MapGenerator::printMap(int j, int i)
{
  char ch = '?';
  switch(d_terrain[j*d_width + i])
    {
    case Tile::MOUNTAIN:
      ch = 'M'; break;
    case Tile::HILLS:
      ch = 'h'; break;
    case Tile::WATER:
      ch = '~'; break;
    case Tile::FOREST:
      ch = '$'; break;
    case Tile::GRASS:
      ch = '.'; break;
    case Tile::SWAMP:
      ch = '_'; break;
    }
  std::cout << ch;
}

void MapGenerator::printMap()
{
  for(int j = 0; j < d_height; j++)
    {
      for(int i = 0; i < d_width; i++)
        printMap(j, i);
      std::cout << "\n";
    }
  std::cout << "\n";
}

const Tile::Type* MapGenerator::getMap(int& width, int& height) const
{
    width = d_width;
    height = d_height;
    return d_terrain;
}

const Maptile::Building* MapGenerator::getBuildings(int& width, int& height) const
{
    width = d_width;
    height = d_height;
    return d_building;
}

void MapGenerator::makePlains()
{
    for(int j = 0; j < d_height; j++)
        for(int i = 0; i < d_width; i++)
            d_terrain[j*d_width + i] = Tile::GRASS;
}

void MapGenerator::connectWithWater(Vector<int> from, Vector<int> to)
{
    Vector<float> delta = from - to;
    if (dist<float>(from,to) > (float)(d_width*0.4))
        // we don't want to mess up whole map with straight lines
        return;

    int kind(Rnd::rand()%4);
    delta /= length(delta)*2;
    for(Vector<float>path = Vector<float>(from)+delta*4 ; dist<float>(path,Vector<float>(to)-delta*4) > 0.5 ; path -= delta)
    {
        int j = (int)(path.x);
        int i = (int)(path.y);

        if(Rnd::rand()%3 == 0) 
            kind = Rnd::rand()%4;
        switch(kind)
        {
            case 0:
                if((!(offmap(i,j))) && (!(offmap(i-1,j-1))))
                {
                    d_terrain[(j  )*d_width + i  ] = Tile::WATER;
                    d_terrain[(j-1)*d_width + i-1] = Tile::WATER;
                    d_terrain[(j  )*d_width + i-1] = Tile::WATER;
                    d_terrain[(j-1)*d_width + i  ] = Tile::WATER;
                }; break;

            case 1:
                if((!(offmap(i,j))) && (!(offmap(i+1,j+1))))
                {
                    d_terrain[(j  )*d_width + i  ] = Tile::WATER;
                    d_terrain[(j+1)*d_width + i+1] = Tile::WATER;
                    d_terrain[(j  )*d_width + i+1] = Tile::WATER;
                    d_terrain[(j+1)*d_width + i  ] = Tile::WATER;
                }; break;

            case 2:
                if((!(offmap(i,j))) && (!(offmap(i-1,j+1))))
                {
                    d_terrain[(j  )*d_width + i  ] = Tile::WATER;
                    d_terrain[(j+1)*d_width + i-1] = Tile::WATER;
                    d_terrain[(j  )*d_width + i-1] = Tile::WATER;
                    d_terrain[(j+1)*d_width + i  ] = Tile::WATER;
                }; break;

            case 3:
                if((!(offmap(i,j))) && (!(offmap(i+1,j-1))))
                {
                    d_terrain[(j  )*d_width + i  ] = Tile::WATER;
                    d_terrain[(j-1)*d_width + i+1] = Tile::WATER;
                    d_terrain[(j  )*d_width + i+1] = Tile::WATER;
                    d_terrain[(j-1)*d_width + i  ] = Tile::WATER;
                }; break;
        }
    }
}

void MapGenerator::findAreasOf(Tile::Type THIS_TILE,std::vector<std::vector<int> >& box,int& how_many)
{
    box.resize(d_height);
    for(int j = 0; j < d_height; j++)
        box[j].resize(d_width,0);

    // find all enclosed areas by scanning the map
    // distinct areas have different numbers in box
    for(int j = 1; j < d_height-1; j++)
        for(int i = 1; i < d_width-1; i++)
            if (box[j][i]==0 &&
                    d_terrain[j*d_width + i] == THIS_TILE &&
                    (
                     (d_terrain[(j-1)*d_width + i-1] == THIS_TILE &&
                      d_terrain[(j  )*d_width + i-1] == THIS_TILE &&
                      d_terrain[(j-1)*d_width + i  ] == THIS_TILE)  ||

                     (d_terrain[(j-1)*d_width + i  ] == THIS_TILE &&
                      d_terrain[(j-1)*d_width + i+1] == THIS_TILE &&
                      d_terrain[(j  )*d_width + i+1] == THIS_TILE) ||

                     (d_terrain[(j  )*d_width + i+1] == THIS_TILE &&
                      d_terrain[(j+1)*d_width + i+1] == THIS_TILE &&
                      d_terrain[(j+1)*d_width + i  ] == THIS_TILE) ||

                     (d_terrain[(j+1)*d_width + i  ] == THIS_TILE &&
                      d_terrain[(j+1)*d_width + i-1] == THIS_TILE &&
                      d_terrain[(j  )*d_width + i-1] == THIS_TILE))
               )
            {
                box[j][i]=++how_many+3000;
                int counter=1;
                while(counter != 0)
                {
                    counter=0;
                    for(int J = 1; J < d_height-1; J++)
                        for(int I = 1; I < d_width-1; I++)
                        {
                            if(d_terrain[J*d_width + I] == THIS_TILE &&
                                    box[J][I]    ==0 &&
                                    (box[J-1][I  ]==how_many+3000 ||
                                     box[J  ][I-1]==how_many+3000 ||
                                     box[J  ][I+1]==how_many+3000 ||
                                     box[J+1][I  ]==how_many+3000))
                            {
                                ++counter;
                                box[J][I]=how_many+2000;
                            }
                        }
                    for(int J = 0; J < d_height; J++)
                        for(int I = 0; I < d_width; I++)
                        {
                            if (box[J][I]==how_many+3000)
                                box[J][I]=how_many;
                            if (box[J][I]==how_many+2000)
                                box[J][I]=how_many+3000;
                        }
                }
            }
}

void MapGenerator::verifyIslands()
{
    int how_many=0;
    std::vector<std::vector<int> > box;
    findAreasOf(Tile::GRASS,box,how_many);

    // count the size of each area
    std::vector<float> counts;
    counts.resize(how_many+2,0);
    for(int j = 0; j < d_height; j++)
        for(int i = 0; i < d_width; i++)
            if(box[j][i] != 0)
                counts[box[j][i]] += 1;

    // find four largest land areas
    std::set<int> largest;largest.clear();
    int max;
    for(int z=0 ; z<4 ; ++z)
    {
        max = -1;
        for(size_t i=0 ; i<counts.size() ; ++i)
        {
            if(counts[i] > max && largest.find(counts[i]) == largest.end())
                max = counts[i];
        }
        largest.insert(max);
    }

    // largest are good. Also one/third of all others is good:
    std::set<int> good(largest);
    for(size_t i=0 ; i<counts.size() ; ++i)
        if(Rnd::rand()%3 == 0) // that's one/third here
            good.insert(counts[i]);

    // now, eliminate all land that is not good
    for(int I=0 ; I<(int)(counts.size()) ; ++I)
        if(good.find(counts[I]) == good.end())
            for(int j = 1; j < d_height-1; j++)
                for(int i = 1; i < d_width-1; i++)
                    if(box[j][i] == I)
                        d_terrain[j*d_width + i] = Tile::WATER;
}

void MapGenerator::makeRivers()
{
    // river style:
    //  1 - plenty of short rivers and islands
    //  2 - longer rivers, less islands
    //  3 - even longer rivers, even less islands
    int river_style=Rnd::rand()%3+1;

    // count how many separate bodies of water were found
    int how_many;

    int iter=0; // avoid deadlocks
    //while(++iter < 20)
    while(++iter < 4)
    {
        how_many=0;

        std::vector<std::vector<int> > box;
        
        findAreasOf(Tile::WATER,box,how_many);

        // this loop allows maximum 3 distinctly separated bodies of water
        // so no need to continue the algorithm
        if(how_many<4)
            break;

        // find two biggest bodies of water, and calculate centers for all of them
        std::vector< Vector<float> > centers;
        centers.resize(how_many+2,Vector<float>(0,0));
        std::vector<float> counts;
        counts.resize(how_many+2,0);
        if (how_many > 40) //trying to speed things up by adding this limit
          how_many = 40;
        for(int j = 0; j < d_height; j++)
            for(int i = 0; i < d_width; i++)
                if(box[j][i] != 0)
                {
                    counts[box[j][i]] += 1;
                    centers[box[j][i]] += Vector<float>(j,i);
                }
        // divide sum by counts to get a center
        int max_count=0,max_count_2=0;
        for(int h = 0; h < how_many+2; ++h)
        {
            if(counts[h]>0)
            {
                centers[h] /= counts[h];
                if(max_count < (int)(counts[h]))
                    max_count = (int)(counts[h]);
                if(max_count_2 < (int)(counts[h]) && (int)(counts[h]) != max_count)
                    max_count_2 = (int)(counts[h]);
                int J=(int)(centers[h].x), I=(int)(centers[h].y);
                if(box[J][I] != h)
                // center doesn't necessarily fall on water tile, so fix this.
                {
                    int i_up=0,i_dn=0,j_up=0,j_dn=0;
                    while((I+i_up <  d_width-1 ) && (box[J     ][I+i_up] != h)) ++i_up;
                    while((I-i_dn >  0         ) && (box[J     ][I-i_dn] != h)) ++i_dn;
                    while((J+j_up <  d_height-1) && (box[J+j_up][I     ] != h)) ++j_up;
                    while((J-j_dn >  0         ) && (box[J-j_dn][I     ] != h)) ++j_dn;

                    int shortest = std::min( std::min(i_up,i_dn) , std::min(j_up,j_dn));

                    if(shortest == i_up && I+i_up <  d_width)
                        centers[h] = Vector<float>( J      , I+i_up );
                    else
                        if(shortest == i_dn && I-i_dn >= 0      )
                            centers[h] = Vector<float>( J      , I-i_dn );
                        else
                            if(shortest == j_up && J+j_up <  d_height)
                                centers[h] = Vector<float>( J+j_up , I      );
                            else
                                if(shortest == j_dn && J-j_dn >= 0       )
                                    centers[h] = Vector<float>( J+j_dn , I      );
                                else
                                {
                                    std::cout << "Sages are wondering about unforeseen mysteries behind the edge of the world.\n";
                                    counts[h] = -1; // that's ok, but an interesting case. I'd like to see a map with such water :)
                                    // FIXME - can you make a message box here?
                                    //MessageBox("Message from author: this is algorithmically a very interesting map, please make screenshot and send to cosurgi@gmail.com");
                                }
                }
            }
        }

        // determine what are the biggest bodies of water here
        int the_biggest_area=0,second_biggest_area=0;
        for(int h = 0; h < how_many+2; ++h)
        {
            if(counts[h]==max_count   && max_count   != 0)
                the_biggest_area = h;
            if(counts[h]==max_count_2 && max_count_2 != 0)
                second_biggest_area = h;
        }

        // find shortest distances between areas
        std::vector<std::vector<std::pair<float, std::pair<Vector<int>, Vector<int> > > > > distances;
        distances.resize(how_many+2);
        int step = GameMap::calculateTilesPerOverviewMapTile();
        for(int h = 0; h < how_many+2; ++h)
        {
            distances[h].resize(how_many+3,std::make_pair(0,std::make_pair(Vector<int>(0,0),Vector<int>(0,0))));
            for(int k = h+1; k < how_many+2; ++k)
            {
                if(counts[h] > 0 && counts[k] > 0) // h and k are two different areas
                {
                    // find tile from area h closest to the center of k 
                    float min_dist = d_height*d_height;
                    float min_h_j=0,min_h_i=0;
                    for(int j = 1; j < d_height-1; j+=step)
                        for(int i = 1; i < d_width-1; i+=step)
                            if(box[j][i] == h)
                            {
                                float dj = j - centers[k].x;
                                float di = i - centers[k].y;
                                float dist = dj*dj + di*di;
                                if(dist < min_dist)
                                {
                                    min_dist = dist;
                                    min_h_j = j;
                                    min_h_i = i;
                                }
                            }

                    // then find tile from area k closest to that tile from h
                    min_dist = d_height * d_height;
                    float min_k_j=0,min_k_i=0;
                    for(int j = 1; j < d_height-1; j+=step)
                        for(int i = 1; i < d_width-1; i+=step)
                            if(box[j][i] == k)
                            {
                                float dj = j - min_h_j;
                                float di = i - min_h_i;
                                float dist = dj*dj + di*di;
                                if(dist < min_dist)
                                {
                                    min_dist = dist;
                                    min_k_j = j;
                                    min_k_i = i;
                                }
                            }

                    if (min_k_j != 0 && 
                            min_h_j != 0 && 
                            min_k_i != 0 && 
                            min_h_i != 0)
                    {
                        float dj = min_k_j - min_h_j;
                        float di = min_k_i - min_h_i;
                        distances[h][k] = std::make_pair(dj*dj + di*di , std::make_pair(Vector<int>(min_h_j,min_h_i) , Vector<int>(min_k_j,min_k_i)) );
                    }
                }
            }
        }

        for(int connect_some_closest=0; connect_some_closest<14; connect_some_closest+=river_style)
        {
            // if river_style is 1 then
            //   connect 10 closest to each other, and 4 closest to two biggest bodies of water
            // otherwise skip some - connect fewer of them.
            int closest_h=-1,closest_k=-1,min=d_height*d_height;
            int start_h=0;
            if(connect_some_closest < 2 ) start_h=the_biggest_area;
            else
                if(connect_some_closest < 4) start_h=second_biggest_area;
            for(int h = start_h; h < ((connect_some_closest >= 4) ? (how_many+2) : start_h+1); ++h)
                for(int k = h+1; k < how_many+2; ++k)
                    if(counts[h] > 0 && counts[k] > 0)
                        if(distances[h][k].first > 0 && min > distances[h][k].first)
                        {
                            min = distances[h][k].first;
                            closest_h = h;
                            closest_k = k;
                        }
            if (closest_h != -1 &&
                closest_k != -1)
            {
                connectWithWater(distances[closest_h][closest_k].second.first , distances[closest_h][closest_k].second.second);
                // mark as done:
                distances[closest_h][closest_k].first = d_height*d_height;
            }
        }
    }
}

/**
 * Makes Terrains.
 * The algorithm is as follows :
 * 1. Find a random starting location
 * 2. chose a random direction , if x is the starting location, the direction
 *    can be from 0-7 as follows :
 *    +-+-+-+
 *    |0|1|2|
 *    +-+-+-+
 *    |7|x|3|
 *    +-+-+-+
 *    |6|5|4|
 *    +-+-+-+
 * 3. Check if there is some other terrain there ('.' = plains is okay)
 * 4. Move one tile in this direction, mutate the tile and continue with 2
 * 5. If we hit a dead end (all directions non-grass), continue with 1
 */
void MapGenerator::makeTerrain(Tile::Type t, int percent, bool contin)
{
    int tries = 0;
    // calculate the total number of tiles for this terrain
    int terrain = d_width*d_height*percent / 100;
    int placed = 0;  // total of current terrain placed so far 
    
    while(placed != terrain)
    {
        // find a random starting position
        int x = Rnd::rand() % d_width;
        int y = Rnd::rand() % d_height;
        if (seekPlain(x, y) == false)
        {
          tries++;
          if (tries > 30000)
            break;
          continue;
        }


        // now go on until we hit a dead end
        while (placed < terrain)
        {
            // if we are on grass, modify this tile first
            if (d_terrain[y*d_width + x] == Tile::GRASS)
            {
                d_terrain[y*d_width + x] = t;
                placed++;
                continue;
            }
            
            // from a random direction, check all directions for further progress
            int loop = 0;
            for (int dir = Rnd::rand()%8; loop < 8; loop++, dir = (dir+1)%8)
            {
                int tmpx = x + d_xdir[dir];
                int tmpy = y + d_ydir[dir];
                
                // reject invalid data
                if (offmap(tmpx, tmpy) || d_terrain[tmpy*d_width + tmpx] != Tile::GRASS)
                    continue;

                // else move our region of interest by one tile
                x = tmpx;
                y = tmpy;
                d_terrain[y*d_width + x] = t;
                placed++;
                break;
            }

            // we have hit a dead end, i.e. we are only surrounded by non-grass
            // tiles. Either choose a new random starting point or, if contin
            // is set, find a close one via seekPlain()
            if (loop == 8)
            {
                if (contin)
                  {
                    if (seekPlain(x, y) == false)
                      continue;
                  }
                else
                    break;
            }
        }
    }
}

/**
 * Makes streaming terrain features.
 * The algorithm is as follows :
 * 1. Find a random starting location
 * 2. chose a random direction , if x is the starting location, the direction
 *    can be from 0-7 as follows :
 *    +-+-+-+
 *    |0|1|2|
 *    +-+-+-+
 *    |7|x|3|
 *    +-+-+-+
 *    |6|5|4|
 *    +-+-+-+
 * 3. Drop the tile and move in the direction
 * 4. Change the direction every so often
 * 5. Keep doing this until we go off the map or we've dropped enough tiles
 *
 */
void MapGenerator::makeStreamer(Tile::Type t, int percent, int thick)
{
    // calculate the total number of tiles for this terrain
    int terrain = d_width*d_height*percent / 100;
    int placed = 0;  // total of current terrain placed so far 
    int dir;
    int i;
    
    while(placed < terrain)
    {
        // find a random starting position
        int x = Rnd::rand() % d_width;
        int y = Rnd::rand() % d_height;
        if (seekPlain(x, y) == false)
          continue;
        dir = Rnd::rand()%8; // pick a random direction
        // now go on until we hit a dead end
        while (placed < terrain)
        {
            // if we are on grass, modify this tile first
            if (d_terrain[y*d_width + x] == Tile::GRASS)
            {
                d_terrain[y*d_width + x] = t;
                placed++;
                continue;
            }
            
            if (Rnd::rand() % 2 == 0)
              {
                if (Rnd::rand() % 2 == 0)
                  {
                    dir++;
                    if (dir > 7)
                      dir = 0;
                  }
                else
                  {
                    dir--;
                    if (dir < 0)
                      dir = 7;
                  }
            }

            {
                int tmpx = x + d_xdir[dir];
                int tmpy = y + d_ydir[dir];
                
                // reject invalid data
                if (offmap(tmpx, tmpy))// || d_terrain[tmpy*d_width + tmpx] != Tile::GRASS)
                    break;

                // else move our region of interest by one tile
                x = tmpx;
                y = tmpy;
                d_terrain[y*d_width + x] = t;
                placed++;
                switch (dir)
                  {
                    case 1: case 2: case 6: case 5:
                      {
                        for (i = 1; i <= thick ; i++)
                          {
                            if (offmap(x+i, y))
                              continue;
                            d_terrain[y*d_width + x+i] = t;
                            placed++;
                          }
                      }
                      break;
                    case 7: case 3: case 0: case 4:
                      {
                        for (i = 1; i <= thick; i++)
                          {
                            if (offmap(x, y+i))
                              continue;
                            d_terrain[(y+i)*d_width + x] = t;
                            placed++;
                          }
                      }
                      break;
                  }
            }
        }
    }
}

bool MapGenerator::seekPlain(int& x, int& y)
{
    int orig_x = x;
    int orig_y = y;
    /* The algorithm here uses a large list of tiles to be checked.
     * In the beginning, it is filled with the tiles surrounding the starting
     * tile. Each tile is then checked if it contains grass. If not, all
     * surrounding tiles are added to the list (we have to take some care to
     * avoid infinite loops).
     *
     * Another way of describing it: The algorithm checks all tiles around the
     * position for the existence of grass. It then checks the tiles in larger
     * and larger circles around the position until it finds a grass tile.
     */
    if (d_terrain[y*d_width + x] == Tile::GRASS)
        return true;

    std::deque<Vector<int> > tiles;
    
    // fill the list with initial values; the rand is there to avoid a bias
    // (i.e. prefer a certain direction)
    for (int dir = Rnd::rand() % 8, i = 0; i < 8; i++, dir = (dir+1)%8)
        tiles.push_back(Vector<int>(x + d_xdir[dir], y + d_ydir[dir]));
    
    // now loop until all tiles were checked (should hardly happen)
    while (!tiles.empty())
    {
        Vector<int> p = tiles.front();
        tiles.pop_front();

        if (offmap(p.x, p.y))
            continue;
        
        // if we have found a patch of grass, we are lucky and return
        if (d_terrain[p.y*d_width + p.x] == Tile::GRASS)
        {
            x = p.x;
            y = p.y;
            return true;
        }
        
        // not found? Well, then append the surrounding tiles. To avoid double-
        // checking (and therefore an infinite loop), only certain surrounding
        // tiles are appended. See the following sketch:
        //
        //              ebbbe
        //              b   b
        //              b x b
        //              b   b
        //              ebbbe
        //
        // This is a circle of radius 2 around the position x. In the case of border
        // tiles, only the tile directly outerwards is appended, the edge tiles 
        // (which can be identified by distx == disty) append two new border and
        // one new edge tile.
        int dx = p.x - x;
        int dy = p.y - y;
        
        // edge tile; append three new tiles
        if (abs(dx) == abs(dy))
        {
            int newx = p.x - 1;
            int newy = p.y - 1;

            if (dx > 0)
                newx = p.x + 1;
            if (dy > 0)
                newy = p.y + 1;
            
            tiles.push_back(Vector<int>(newx, newy));
            tiles.push_back(Vector<int>(newx, p.y));
            tiles.push_back(Vector<int>(p.x, newy));
        }
        else
        {
            if (abs(dx) > abs(dy) && dx > 0)        // right border
                tiles.push_back(Vector<int>(p.x + 1, p.y));
            else if (abs(dx) > abs(dy) && dx < 0)   //left border
                tiles.push_back(Vector<int>(p.x - 1, p.y));
            else if (abs(dx) < abs(dy) && dy > 0)   // top border
                tiles.push_back(Vector<int>(p.x, p.y + 1));
            else if (abs(dx) < abs(dy) && dy < 0)   // lower border
                tiles.push_back(Vector<int>(p.x, p.y - 1));
        }
    }

    // if this line is ever reached, we haven't found a free grass tile
    // (should only happen under really exceptional circumstances)
    x = orig_x;
    y = orig_y;
    return false;
}

bool MapGenerator::canPlaceBuilding(Vector<int> pos, guint32 width, const std::vector<Tile::Type> &allowed)
{
  for (unsigned int i = 0; i < width; i++)
    for (unsigned int j = 0; j < width; j++)
      if (canPutBuildingTile(pos + Vector<int>(i,j), width, allowed) == false)
        return false;
        
  return true;
}

bool MapGenerator::canPutBuildingTile(Vector<int> pos, guint32 width, const std::vector<Tile::Type> &allowed)
{
  int found = false;
  for (auto t : allowed)
    if (d_terrain[pos.y * d_width + pos.x] == t)
      {
        found = true;
        break;
      }
  if (!found)
    return false;

  int tooclose;
  tooclose = GameMap::calculateTilesPerOverviewMapTile(d_width, d_height);
  tooclose++;
  //if the building is close to the map boundaries, return false
  if (pos.x <= tooclose || pos.x >= (d_width - tooclose) || 
      pos.y <= tooclose || pos.y >= (d_height - tooclose))
    return false;

  int dist = width + tooclose;
  //if there is another building too close, return false
  for (int locx = pos.x - dist; locx <= pos.x + dist; locx++)
    for (int locy = pos.y - dist; locy <= pos.y + dist; locy++)
      {
        if (offmap(locx, locy))
          continue;
        if (d_building[locy * d_width + locx] != Maptile::NONE)
          return false;
      }  
  // everything okay here! return true
  return true;

}

void MapGenerator::makeBuildings(int total, int width,
                                 sigc::slot<void,Vector<int> > place)
{
  int count = 0;

  std::vector<Vector<int> > points;

  for (int i = width; i < d_height - width; i++)
    for (int j = width; j < d_width - width; j++)
      if (d_building[j*d_width + i] == Maptile::NONE) 
        points.push_back (Vector<int>(j, i));
  std::random_shuffle (points.begin (), points.end ());

  std::vector<Vector<int> > positions;
  std::vector<Tile::Type> allowed;
  for (int phase = 0; phase < 3; phase++)
    {
      positions.clear ();
      switch (phase)
        {
        case 0:
          // first we check the free grassy areas
          allowed.push_back (Tile::GRASS);
          break;
        case 1:
          // then we check all land
          allowed.push_back (Tile::FOREST);
          allowed.push_back (Tile::HILLS);
          allowed.push_back (Tile::MOUNTAIN);
          allowed.push_back (Tile::SWAMP);
          break;
        case 2:
          // finally we stoop to making islands
          allowed.push_back (Tile::WATER);
          break;
        }
      for (auto pos : points)
        if (canPlaceBuilding (pos, width, allowed))
          positions.push_back (pos);

      std::random_shuffle (positions.begin (), positions.end ());

      unsigned int limit = total;
      if (positions.size () + count < limit)
        limit = positions.size () + count;

      for (unsigned int i = count, j = 0; i < limit; i++, j++)
        if (canPlaceBuilding (positions[j], width, allowed))
          {
            place(positions[j]);
            count++;
          }
      if (count >= total)
        break;
    }
}

void MapGenerator::placeBldg (Vector<int> pos, Maptile::Building b,
                              guint32 width)
{
  int y = pos.y;
  int x = pos.x;
  d_building[y*d_width + x] = b;

  //ruins shall only sit on grass tiles
  for (unsigned int i = 0; i < width; i++)
    for (unsigned int j = 0; j < width; j++)
      d_terrain[(y+i)*d_width + (x+j)] = Tile::GRASS;

  //ruins cannot neighbor with mountain tiles
  for (int Y = -1; Y <= (int)width; ++Y )
    for (int X = -1; X <= (int)width; ++X)
      if (d_terrain[(y+Y)*d_width + x+X] == Tile::MOUNTAIN)
        d_terrain[(y+Y)*d_width + x+X] = Tile::HILLS;
}

void MapGenerator::placeCity(Vector<int> pos)
{
  placeBldg (pos, Maptile::CITY, cityset->getCityTileWidth ());
}


void MapGenerator::placeRuin(Vector<int> pos)
{
  placeBldg (pos, Maptile::RUIN, cityset->getRuinTileWidth ());
}

void MapGenerator::placeTemple (Vector<int> pos)
{
  placeBldg (pos, Maptile::TEMPLE, cityset->getTempleTileWidth ());
}

void MapGenerator::placeSign (Vector<int> pos)
{
  placeBldg (pos, Maptile::SIGNPOST, 1);
}

void MapGenerator::normalize()
{
    std::map<guint32,guint32> ajacentTer;
    Tile::Type curTer=Tile::NONE, ajTer=Tile::NONE;

    // that was 40 before. Now with rivers, the smaller the value - the more connected rivers we got.
    unsigned int center_tiles = Rnd::rand()%40;
    //std::cerr << center_tiles << "\% chance of disconnecting rivers.\n";

    // Go through every tile bar the outer edge
    for(int globy = 1; globy < (d_height-2); globy++)
        for(int globx = 1; globx < (d_width-2); globx++)
        {
            curTer = d_terrain[globy*d_width + globx];

            // reset all counters
            ajacentTer[Tile::GRASS] = 0;
            ajacentTer[Tile::WATER] = 0;
            ajacentTer[Tile::FOREST] = 0;
            ajacentTer[Tile::HILLS] = 0;
            ajacentTer[Tile::MOUNTAIN] = 0;
            ajacentTer[Tile::SWAMP] = 0;

            // count how many neighbours of each type we have
            for(int locx = globx - 1; locx <= globx+1; locx++)
                for(int locy = globy - 1; locy <= globy+1; locy++)
                { 
                     ajTer = d_terrain[locy*d_width +locx];
                     ajacentTer[ajTer] += 1;
                }

            // we have counted our own tile as well
            ajacentTer[curTer] -= 1;

            if (curTer==Tile::WATER) // For the moment only water is normalized
            {
                if (ajacentTer[curTer]==0)
                    d_terrain[globy*d_width +globx] = Tile::GRASS;
                else if ((ajacentTer[curTer]==1) && (Rnd::rand()%100 < 95 ))
                    d_terrain[globy*d_width +globx] = Tile::GRASS;
                else if ((ajacentTer[curTer]==2) && (Rnd::rand()%100 < 70 ))
                    d_terrain[globy*d_width +globx] = Tile::GRASS;
                else if ((ajacentTer[curTer]==3) && (Rnd::rand()%100 < center_tiles ))
                    d_terrain[globy*d_width +globx] = Tile::GRASS;
            }
            else 
            {
                if (ajacentTer[Tile::WATER]==8)
                    d_terrain[globy*d_width +globx] = Tile::WATER;
                else if ((ajacentTer[Tile::WATER]==7) && (Rnd::rand()%100 < 70 ))
                    d_terrain[globy*d_width +globx] = Tile::WATER;
                else if ((ajacentTer[Tile::WATER]==6) && (Rnd::rand()%100 < 40 ))
                    d_terrain[globy*d_width +globx] = Tile::WATER;
             }
        }
}

void MapGenerator::calculateBlockedAvenue(int x, int y)
{
  for (int i = x - 1; i <= x + 1; i++)
    {
      for (int j = y - 1; j <= y + 1; j++)
	{
	  if (i < 0 || i >= d_width)
	    continue;
	  if (j < 0 || j >= d_height)
	    continue;
	  GameMap::getInstance()->calculateBlockedAvenue(i, j);
	}
    }
}

bool MapGenerator::placePort(int x, int y)
{
  //if (Citylist::getInstance()->getNearestCity(Vector<int>(x, y), 2) == NULL)
    {
      if (d_building[y*d_width + x] == Maptile::NONE)
	{
	  d_building[y*d_width + x] = Maptile::PORT;
          Portlist::getInstance()->add(new Port(Vector<int>(x, y)));
	  calculateBlockedAvenue(x, y);
	  return true;
	}
    }
  return false;
}

bool MapGenerator::makeRoad(Vector<int> src, Vector<int>dest)
{
  bool retval = true;

  RoadPathCalculator rpc(src);
  Path *p = rpc.calculate(dest);

  if (p->size() > 0)
    {
      for (auto it: *p)
	{
	  if (GameMap::getInstance()->getTile(it)->getType() == Tile::WATER &&
	      GameMap::getInstance()->getTile(it)->getBuilding() != Maptile::BRIDGE)
	    {
	      retval = false;
	      break;
	    }
	  if (Citylist::getInstance()->getObjectAt(it) == NULL)
	    {
	      if (d_building[it.y*d_width + it.x] == Maptile::NONE)
		{
		  d_building[it.y*d_width + it.x] = Maptile::ROAD;
                  Roadlist::getInstance()->add(new Road(Vector<int>(it)));
		  calculateBlockedAvenue(it.x, it.y);
		}
	    }
	}

    }
  else
    retval = false;
  delete p;

  return retval;
}

bool MapGenerator::makeRoad2(Path *p)
{
  bool retval = true;

  if (p->size() > 0)
    {
      for (auto it: *p)
	{
	  if (GameMap::getInstance()->getTile(it)->getType() == Tile::WATER &&
	      GameMap::getInstance()->getTile(it)->getBuilding() != Maptile::BRIDGE)
	    {
	      retval = false;
	      break;
	    }
	  if (Citylist::getInstance()->getObjectAt(it) == NULL)
	    {
	      if (d_building[it.y*d_width + it.x] == Maptile::NONE)
		{
		  d_building[it.y*d_width + it.x] = Maptile::ROAD;
                  Roadlist::getInstance()->add(new Road(Vector<int>(it)));
		  calculateBlockedAvenue(it.x, it.y);
		}
	    }
	}

    }
  else
    retval = false;

  return retval;
}

bool MapGenerator::makeAccessible(RoadPathCalculator *pc_land, RoadPathCalculator *pc_fly, Vector<int> dest)
{
  bool retval = true;

  Path *p = pc_fly->calculate (dest);

  if (p)
    {
      Path::reverse_iterator it = p->rbegin();
      Path::reverse_iterator nextit = it;
      ++nextit;
      for ( ; nextit != p->rend(); ++it, ++nextit)
	{
	  int x = (*it).x;
	  int y = (*it).y;
	  int nextx = (*nextit).x;
	  int nexty = (*nextit).y;
	  if (d_terrain[y*d_width + x] == Tile::MOUNTAIN)
	    {
	      d_terrain[y*d_width +x] = Tile::HILLS;
              GameMap::getInstance()->setTileIndex(x, y, GameMap::getTileset()->lookupIndexByType(Tile::HILLS));
	      calculateBlockedAvenue(x, y);
	    }
	  if (d_terrain[y*d_width + x] == Tile::WATER &&
	      d_terrain[nexty*d_width + nextx] != Tile::WATER)
	    {
	      if (placePort(x, y) == true)
                {
                  pc_land->regenerate();
                  if (pc_land->calculate_moves (Vector<int>(x, y)) != 0)
                    {
                      retval = true;
                      break;
                    }
                }
	    }
	  else if (d_terrain[y*d_width + x] != Tile::WATER &&
		   d_terrain[nexty*d_width + nextx] == Tile::WATER)
	    {
	      if (placePort(nextx, nexty) == true)
                {
                  pc_land->regenerate();
                  if (pc_land->calculate_moves (Vector<int>(x, y)) != 0)
                    {
                      retval = true;
                      break;
                    }
                }
	    }

	}
      delete p;
    }
  else
    retval = false;

  return retval;
}

std::vector<std::pair<int , Vector<int> > > MapGenerator::findBridgePlaces()
{
    std::vector<std::pair<int , Vector<int> > > result;
    result.clear();

    for(int j = 1; j < d_height-5; j++)
        for(int i = 1; i < d_width-5; i++)
        {
          /*
           *   G
           *  WWW
           *  WWW
           *   G
           */
            if (
                d_terrain[(j  )*d_width + i+1] != Tile::WATER &&
                d_terrain[(j+1)*d_width + i+1] == Tile::WATER &&
                d_terrain[(j+2)*d_width + i+1] == Tile::WATER &&
                d_terrain[(j+3)*d_width + i+1] != Tile::WATER &&
                d_terrain[(j+1)*d_width + i  ] == Tile::WATER &&
                d_terrain[(j+2)*d_width + i  ] == Tile::WATER &&
                d_terrain[(j+1)*d_width + i+2] == Tile::WATER &&
                d_terrain[(j+2)*d_width + i+2] == Tile::WATER
                )
            {
                int count_left =
                (int)(d_terrain[(j  )*d_width + i  ] == Tile::WATER) +
                (int)(d_terrain[(j+1)*d_width + i  ] == Tile::WATER) +
                (int)(d_terrain[(j+2)*d_width + i  ] == Tile::WATER) +
                (int)(d_terrain[(j+3)*d_width + i  ] == Tile::WATER) +
                (int)(d_terrain[(j  )*d_width + i-1] == Tile::WATER) +
                (int)(d_terrain[(j+1)*d_width + i-1] == Tile::WATER) +
                (int)(d_terrain[(j+2)*d_width + i-1] == Tile::WATER) +
                (int)(d_terrain[(j+3)*d_width + i-1] == Tile::WATER);
                int count_right =
                (int)(d_terrain[(j  )*d_width + i+2] == Tile::WATER) +
                (int)(d_terrain[(j+1)*d_width + i+2] == Tile::WATER) +
                (int)(d_terrain[(j+2)*d_width + i+2] == Tile::WATER) +
                (int)(d_terrain[(j+3)*d_width + i+2] == Tile::WATER) +
                (int)(d_terrain[(j  )*d_width + i+3] == Tile::WATER) +
                (int)(d_terrain[(j+1)*d_width + i+3] == Tile::WATER) +
                (int)(d_terrain[(j+2)*d_width + i+3] == Tile::WATER) +
                (int)(d_terrain[(j+3)*d_width + i+3] == Tile::WATER);
                
                if(count_left > 5 && count_right > 5)
                    result.push_back(std::make_pair(NORTH_SOUTH_BRIDGE, Vector<int>(i,j) ));
            }
          /*
           *  WW
           * GWWG
           *  WW
           *
           */
            if (
                d_terrain[(j+1)*d_width + i  ] != Tile::WATER &&
                d_terrain[(j+1)*d_width + i+1] == Tile::WATER &&
                d_terrain[(j+1)*d_width + i+2] == Tile::WATER &&
                d_terrain[(j+1)*d_width + i+3] != Tile::WATER &&
                d_terrain[(j  )*d_width + i+1] == Tile::WATER &&
                d_terrain[(j  )*d_width + i+2] == Tile::WATER &&
                d_terrain[(j+2)*d_width + i+1] == Tile::WATER &&
                d_terrain[(j+2)*d_width + i+2] == Tile::WATER
                )
            {
                int count_top =
                (int)(d_terrain[(j  )*d_width + i  ] == Tile::WATER) +
                (int)(d_terrain[(j  )*d_width + i+1] == Tile::WATER) +
                (int)(d_terrain[(j  )*d_width + i+2] == Tile::WATER) +
                (int)(d_terrain[(j  )*d_width + i+3] == Tile::WATER) +
                (int)(d_terrain[(j-1)*d_width + i  ] == Tile::WATER) +
                (int)(d_terrain[(j-1)*d_width + i+1] == Tile::WATER) +
                (int)(d_terrain[(j-1)*d_width + i+2] == Tile::WATER) +
                (int)(d_terrain[(j-1)*d_width + i+3] == Tile::WATER);

                int count_bottom =
                (int)(d_terrain[(j+2)*d_width + i  ] == Tile::WATER) +
                (int)(d_terrain[(j+2)*d_width + i+1] == Tile::WATER) +
                (int)(d_terrain[(j+2)*d_width + i+2] == Tile::WATER) +
                (int)(d_terrain[(j+2)*d_width + i+3] == Tile::WATER) +
                (int)(d_terrain[(j+3)*d_width + i  ] == Tile::WATER) +
                (int)(d_terrain[(j+3)*d_width + i+1] == Tile::WATER) +
                (int)(d_terrain[(j+3)*d_width + i+2] == Tile::WATER) +
                (int)(d_terrain[(j+3)*d_width + i+3] == Tile::WATER);

                if(count_top > 5 && count_bottom > 5)
                    result.push_back(std::make_pair(EAST_WEST_BRIDGE, Vector<int>(i,j) ));
            }
        }
    // randomize
    std::random_shuffle(result.begin(),result.end());

    // remove those that are too close to each other
    std::set<int> bad;bad.clear();
    for(size_t r = 0; r<result.size() ; ++r)
        for(size_t s = r+1; s<result.size() ; ++s)
            if(dist(Vector<float>(result[r].second),Vector<float>(result[s].second)) < 4.5)
                bad.insert(r);
    std::vector<std::pair<int , Vector<int> > > filter;filter.clear();
    for(size_t r = 0; r<result.size() ; ++r)
        if(bad.find(r) == bad.end())
            filter.push_back(result[r]);
    result=filter;

    return result;
}

void MapGenerator::makeCitiesAccessible()
{
  Glib::ustring orig_tileset = GameMap::getInstance()->getTilesetBaseName();
  Glib::ustring orig_shieldset = GameMap::getInstance()->getShieldsetBaseName();
  Glib::ustring orig_cityset = GameMap::getInstance()->getCitysetBaseName();
  GameMap::deleteInstance();
  Citylist::deleteInstance();
  Roadlist::deleteInstance();
  Portlist::deleteInstance();

  GameMap::setWidth(d_width);
  GameMap::setHeight(d_height);
  GameMap::getInstance("default", "default", cityset->getBaseName())->fill(this);
  //the game map class smooths the map, so let's take what it smoothed.
  for (int y = 0; y < d_height; y++)
    for (int x = 0; x < d_width; x++)
      d_terrain[y*d_width + x] =
	GameMap::getInstance()->getTile(x, y)->getType();

  for (int y = 0; y < d_height; y++)
    for (int x = 0; x < d_width; x++)
      {
	if (d_building[y*d_width + x] == Maptile::CITY)
	  Citylist::getInstance()->add
	    (new City(Vector<int>(x,y), cityset->getCityTileWidth()));
      }
  GameMap::getInstance()->calculateBlockedAvenues();

  //make all cities accessible by allowing movement to a central city
  City *center = Citylist::getInstance()->getNearestCity(GameMap::getCenterOfMap());
  RoadPathCalculator pc_land (center->getPos());
  RoadPathCalculator pc_fly (center->getPos(), true);
  for (auto it: *Citylist::getInstance())
    {
      if (center == it)
	continue;
      if (pc_land.calculate_moves (it->getPos()) == 0)
        {
          makeAccessible(&pc_land, &pc_fly, it->getPos());
          pc_land.regenerate();
          pc_fly.regenerate();
        }
    }

  Roadlist::deleteInstance();
  GameMap::deleteInstance();
  Citylist::deleteInstance();
  Portlist::deleteInstance();
  GameMap::getInstance(orig_tileset, orig_shieldset, orig_cityset);
}

void MapGenerator::makeRoads()
{
  Glib::ustring orig_tileset = GameMap::getInstance()->getTilesetBaseName();
  Glib::ustring orig_shieldset = GameMap::getInstance()->getShieldsetBaseName();
  Glib::ustring orig_cityset = GameMap::getInstance()->getCitysetBaseName();
  GameMap::deleteInstance();
  Citylist::deleteInstance();
  Roadlist::deleteInstance();
  Portlist::deleteInstance();

  GameMap::setWidth(d_width);
  GameMap::setHeight(d_height);
  GameMap::getInstance("default", "default", cityset->getBaseName())->fill(this);
  //the game map class smooths the map, so let's take what it smoothed.
  for (int y = 0; y < d_height; y++)
    for (int x = 0; x < d_width; x++)
      d_terrain[y*d_width + x] = 
	GameMap::getInstance()->getTile(x, y)->getType();

  for (int y = 0; y < d_height; y++)
    for (int x = 0; x < d_width; x++)
      {
	if (d_building[y*d_width + x] == Maptile::CITY)
	  Citylist::getInstance()->add
	    (new City(Vector<int>(x,y), cityset->getCityTileWidth()));
      }
  GameMap::getInstance()->calculateBlockedAvenues();

  guint32 roads_built = 0;
  for (auto it: *Citylist::getInstance())
    {
      City *c = Citylist::getInstance()->getNearestCityPast(it->getPos(), 13);
      Vector<int> dest = c->getPos();
      Vector<int> src = it->getPos();
      //does it already have a road going to it?
      if (Roadlist::getInstance()->getNearestObjectBefore(dest, c->getSize() + 1))
	continue;

      if (makeRoad(src, dest))
        {
          roads_built++;
          if (roads_built > Citylist::getInstance()->size()/3)
            break;
        }
    }

  Roadlist::deleteInstance();
  GameMap::deleteInstance();
  Citylist::deleteInstance();
  Portlist::deleteInstance();
  GameMap::getInstance(orig_tileset, orig_shieldset, orig_cityset);
}

void MapGenerator::rescueLoneTiles(Tile::Type FIND_THIS, Tile::Type REPLACE, bool grow)
{
    int box[3][3];
    memset (box, 0, sizeof (box));

    if(grow)
    {
        for(int j = 1; j < d_height-1; j++)
            for(int i = 1; i < d_width-1; i++)
            {
                if (d_terrain[j*d_width + i] == FIND_THIS &&
                   (d_terrain[(j-1)*d_width + i-1] == FIND_THIS &&
                    d_terrain[(j  )*d_width + i-1] == FIND_THIS &&
                    d_terrain[(j-1)*d_width + i  ] != FIND_THIS))
                    d_terrain[(j-1)*d_width + i  ] =  FIND_THIS;
            }
    }

    for(int iteration=0; iteration <8 ;++iteration)
    {
        for(int j = 0; j < d_height; j++)
            for(int i = 0; i < d_width; i++)
            {
                if(d_terrain[j*d_width + i] == FIND_THIS)
                { 
                    for (int I = -1; I <= +1; ++I)
                        for (int J = -1; J <= +1; ++J)
                            if (!(offmap(i+I,j+J)))
                                box[J+1][I+1] = (d_terrain[(j+J)*d_width + (i+I)] == d_terrain[j*d_width + i]);
                            else
                                box[J+1][I+1] = 0;

                    if (!box[0][2] && !box[1][2] && /***********/ 
                        /***********/  box[1][1] &&  box[2][1] && 
                        !box[0][0] && !box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;
                    if (/***********/ !box[1][2] && !box[2][2] && 
                         box[0][1] &&  box[1][1] && /***********/
                        /***********/ !box[1][0] && !box[2][0])
                            d_terrain[j*d_width + i] = REPLACE;
                    if (!box[0][2] && /***********/ !box[2][2] && 
                        !box[0][1] &&  box[1][1] && !box[2][1] && 
                        /***********/  box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;
                    if (/***********/  box[1][2] && /***********/
                        !box[0][1] &&  box[1][1] && !box[2][1] && 
                        !box[0][0] && /***********/ !box[2][0])
                            d_terrain[j*d_width + i] = REPLACE;

                    if (/***********/ !box[1][2] && /***********/ 
                        /***********/  box[1][1] &&  box[2][1] && 
                        !box[0][0] && !box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;
                    if (/***********/ !box[1][2] && /***********/ 
                         box[0][1] &&  box[1][1] && /***********/
                        /***********/ !box[1][0] && !box[2][0])
                            d_terrain[j*d_width + i] = REPLACE;
                    if (/***********/ /***********/ !box[2][2] && 
                        !box[0][1] &&  box[1][1] && !box[2][1] && 
                        /***********/  box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;
                    if (/***********/  box[1][2] && /***********/
                        !box[0][1] &&  box[1][1] && !box[2][1] && 
                        /***********/ /***********/ !box[2][0])
                            d_terrain[j*d_width + i] = REPLACE;

                    if (!box[0][2] && !box[1][2] && /***********/ 
                        /***********/  box[1][1] &&  box[2][1] && 
                        /***********/ !box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;
                    if (/***********/ !box[1][2] && !box[2][2] && 
                         box[0][1] &&  box[1][1] && /***********/
                        /***********/ !box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;
                    if (!box[0][2] && /***********/ /***********/ 
                        !box[0][1] &&  box[1][1] && !box[2][1] && 
                        /***********/  box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;
                    if (/***********/  box[1][2] && /***********/
                        !box[0][1] &&  box[1][1] && !box[2][1] && 
                        !box[0][0]    /***********/ /***********/)
                            d_terrain[j*d_width + i] = REPLACE;

                    if (/***********/ !box[1][2] && /***********/ 
                        !box[0][1] &&  box[1][1] &&  box[2][1] && 
                        /***********/ !box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;
                    if (/***********/ !box[1][2] && /***********/ 
                         box[0][1] &&  box[1][1] && !box[2][1] &&
                        /***********/ !box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;
                    if (/***********/ !box[1][2] && /***********/ 
                        !box[0][1] &&  box[1][1] && !box[2][1] && 
                        /***********/  box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;
                    if (/***********/  box[1][2] && /***********/
                        !box[0][1] &&  box[1][1] && !box[2][1] && 
                        /***********/ !box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;

                    if ( box[0][2] && !box[1][2] && /***********/
                        !box[0][1] &&  box[1][1] && !box[2][1] && 
                        /***********/ !box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;
                    if (/***********/ !box[1][2] &&  box[2][2] && 
                        !box[0][1] &&  box[1][1] && !box[2][1] && 
                        /***********/ !box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;
                    if (/***********/ !box[1][2] && /***********/ 
                        !box[0][1] &&  box[1][1] && !box[2][1] && 
                         box[0][0] && !box[1][0]    /***********/)
                            d_terrain[j*d_width + i] = REPLACE;
                    if (/***********/ !box[1][2] && /***********/  
                        !box[0][1] &&  box[1][1] && !box[2][1] && 
                        /***********/ !box[1][0] &&  box[2][0])
                            d_terrain[j*d_width + i] = REPLACE;


                    if ( box[0][2] && !box[1][2] &&  box[2][2] &&  
                         box[0][1] &&  box[1][1] &&  box[2][1] && 
                         box[0][0] && !box[1][0] &&  box[2][0])
                            d_terrain[j*d_width + i+(Rnd::rand()%2?+1:-1)] = FIND_THIS;
                    if ( box[0][2] &&  box[1][2] &&  box[2][2] &&  
                        !box[0][1] &&  box[1][1] && !box[2][1] && 
                         box[0][0] &&  box[1][0] &&  box[2][0])
                            d_terrain[(j+(Rnd::rand()%2?+1:-1))*d_width + i] = FIND_THIS;

                    if ( box[0][2] && !box[1][2] && !box[2][2] &&  
                         box[0][1] &&  box[1][1] && !box[2][1] && 
                        !box[0][0] &&  box[1][0] &&  box[2][0])
                            d_terrain[j*d_width + i] = REPLACE;
                    if (!box[0][2] && !box[1][2] &&  box[2][2] &&  
                        !box[0][1] &&  box[1][1] &&  box[2][1] && 
                         box[0][0] &&  box[1][0] && !box[2][0])
                            d_terrain[j*d_width + i] = REPLACE;
                    if ( box[0][2] &&  box[1][2] && !box[2][2] &&  
                        !box[0][1] &&  box[1][1] &&  box[2][1] && 
                        !box[0][0] && !box[1][0] &&  box[2][0])
                            d_terrain[j*d_width + i] = REPLACE;
                    if (!box[0][2] &&  box[1][2] &&  box[2][2] &&  
                         box[0][1] &&  box[1][1] && !box[2][1] && 
                         box[0][0] && !box[1][0] && !box[2][0])
                            d_terrain[j*d_width + i] = REPLACE;
                }
            }
    }
}

void MapGenerator::surroundMountains(int minx, int maxx, int miny, int maxy)
{
  for(int j = miny; j < maxy; j++)
    for(int i = minx; i < maxx; i++)
      if(d_terrain[j*d_width + i] == Tile::MOUNTAIN)
	for(int J = -1; J <= +1; ++J)
	  for(int I = -1; I <= +1; ++I)
	    if((!(offmap(i+I,j+J))) &&
	       (d_terrain[(j+J)*d_width + (i+I)] != Tile::MOUNTAIN))
	      {
		if(d_terrain[(j+J)*d_width + (i+I)] != Tile::WATER)
		  d_terrain[(j+J)*d_width + (i+I)] = Tile::HILLS;
		else 
		  // water has priority here, there was some work done to conenct bodies of water
		  // so don't break those connections.
		  d_terrain[(j  )*d_width + (i  )] = Tile::HILLS;
	      }
}
