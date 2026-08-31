//  Copyright (C) 2007, 2008, 2009, 2014, 2015 Ben Asselstine
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
#include <sstream>
#include <string>

#include "FogMap.h"
#include "SightMap.h"

#include "playerlist.h"
#include "xmlhelper.h"
#include "GameScenarioOptions.h"

Glib::ustring FogMap::d_tag = "fogmap";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<flush;}
#define debug(x)


FogMap::FogMap(int width, int height)
{
    debug("FogMap()");
    d_width = width;
    d_height = height;

    d_fogmap = new FogType[d_width*d_height];
    shademap = new ShadeType[d_width*d_height];

    fill(OPEN);
}

FogMap::FogMap(XML_Helper* helper)
{
    Glib::ustring t;
    
    helper->getData(d_width, "width");
    helper->getData(d_height, "height");
    helper->getData(t, "map");
    std::string types = t.raw();
    types.erase (std::remove(types.begin(), types.end(), '\n'), types.end());
    types.erase (std::remove(types.begin(), types.end(), '\r'), types.end());

    //create the map
    d_fogmap = new FogType[d_width*d_height];

    for (int y = 0; y < d_height; y++)
    {
        for (int x = 0; x < d_width; x++)
        {
            //due to the circumstances, types is a long stream of
            //numbers, so read it character for character (no \n's or so)
            d_fogmap[y*d_width + x] = FogType(types[y*d_width + x] - '0');
        }
    }
    shademap = new ShadeType[d_width*d_height];
    calculateShadeMap();
}

FogMap::FogMap(const FogMap& fogmap)
    :d_width(fogmap.d_width), d_height(fogmap.d_height)
{
    //create the map
    d_fogmap = new FogType[d_width*d_height];

    for (int y = 0; y < d_height; y++)
    {
        for (int x = 0; x < d_width; x++)
        {
            d_fogmap[y*d_width + x] = fogmap.d_fogmap[y*d_width + x];
        }
    }
    shademap = new ShadeType[d_width*d_height];
    for (int y = 0; y < d_height; y++)
    {
        for (int x = 0; x < d_width; x++)
        {
            shademap[y*d_width + x] = fogmap.shademap[y*d_width + x];
        }
    }
}

FogMap::~FogMap()
{
    delete[] d_fogmap;
    delete[] shademap;
}

bool FogMap::fill(FogType type)
{
    for (int i = 0; i < d_width*d_height; i++)
        d_fogmap[i] = type;

    calculateShadeMap();
    return true;
}

bool FogMap::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(FogMap::d_tag);
    retval &= helper->saveData("width", d_width);
    retval &= helper->saveData("height", d_height);

    std::stringstream types;
    types << std::endl;
    for (int y = 0; y < d_height; y++)
    {
        for (int x = 0; x < d_width; x++)
        {
            types << static_cast<int>(d_fogmap[y*d_width + x]);
        }
        types << std::endl;
    }

    retval &= helper->saveData("map", types.str());
    retval &= helper->closeTag();

    return retval;
}

FogMap::FogType FogMap::getFogTile(Vector<int> pos) const
{
    return d_fogmap[pos.y * d_width + pos.x];
}

FogMap::ShadeType FogMap::getShadeTile(Vector<int> pos) const
{
    return shademap[pos.y * d_width + pos.x];
}

void FogMap::alterFogRadius(Vector<int> pt, int radius, FogType new_type)
{
    if (GameScenarioOptions::s_hidden_map == false)
      return;
    // this doesn't draw a circle, it draws a square
    // it isn't a bug, except for being badly named
    int x = pt.x - radius;
    int y = pt.y - radius;
    int size = 2 * radius + 1;
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            if ((x+i) < 0 || (y+j) < 0 || (x+i) >= d_width || (y+j) >= d_height)
                continue;
            d_fogmap[(y+j)*d_width + (x+i)] = new_type;
        }
    }
    calculateShadeMap();
}

void FogMap::alterFogRectangle(Vector<int> pt, int height, int width, FogType new_type)
{
    if (GameScenarioOptions::s_hidden_map == false)
      return;
    int x = pt.x;
    int y = pt.y;
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            if ((x+i) < 0 || (y+j) < 0 || (x+i) >= d_width || (y+j) >= d_height)
                continue;
            d_fogmap[(y+j)*d_width + (x+i)] = new_type;
        }
    }
    calculateShadeMap();
}

bool FogMap::isCompletelyObscuredFogTile(Vector<int> pos) const
{
  if (shademap[pos.y * d_width + pos.x] == ALL)
    return true;
  else
    return false;
  return false;
}

bool FogMap::isLoneFogTile(Vector<int> pos)
{
  bool west_open = false;
  bool east_open = false;
  //are east-west adjacent squares open?
  if (pos.x + 1 >= d_width || d_fogmap[pos.y*d_width + pos.x + 1] == OPEN)
    west_open = true;
  if (pos.x - 1 < 0 || d_fogmap[pos.y*d_width + pos.x - 1] == OPEN)
    east_open = true;
  bool north_open = false;
  bool south_open = false;
  //are north-south adjacent squares open?
  if (pos.y + 1 >= d_height || d_fogmap[(pos.y+1)*d_width + pos.x] == OPEN)
    south_open = true;
  if (pos.y - 1 < 0 || d_fogmap[(pos.y-1)*d_width + pos.x] == OPEN)
    north_open = true;
  if (east_open && west_open)
    return true;
  if (north_open && south_open)
    return true;
  return false;
}

void FogMap::smooth()
{
    for (int y = 0; y < d_height; y++)
    {
        for (int x = 0; x < d_width; x++)
        {
            if (d_fogmap[y*d_width + x] == CLOSED)
              {
		Vector<int> pos;
		pos.x = x;
		pos.y = y;
                if (isLoneFogTile (pos))
                  d_fogmap[y*d_width + x] = OPEN;
              }
        }
    }

    calculateShadeMap();
}

bool FogMap::isFogged(Vector <int> pos)
{
  if (getFogTile(pos) == FogMap::CLOSED)
    return true;

  if (isLoneFogTile(pos) == true)
    return false;

  return false;
}

bool FogMap::isClear(Vector <int> pos, Player *player)
{
  //is this tile visible, or not?
  FogMap *fogmap = player->getFogMap();
  if (fogmap->getFogTile(pos) == FogMap::OPEN)
    return true;

  return false;
}

void FogMap::alterFog(SightMap *sightmap)
{
  return alterFogRectangle(sightmap->pos, sightmap->h, sightmap->w, OPEN);
}

/*
 * fog display algorithm
 *
 * smallmap shows fog placement
 * - it is a peek into the data model
 *
 * bigmap shows a rendering of that
 * if a tile on the bigmap is partially fogged, then it is completely fogged on the small map.
 *
 * this means that partially fogged tiles depend on adjacent tiles being fogged
 * completely fogged tiles depends on adjacent tiles being fogged
 *
 * when one tile is fogged and is not surrounded by adjacent fogged tiles it is shown as not fogged on the bigmap, while it is shown as fogged on the small map.  it is then marked as defogged at the start of the next turn.
 *
 * every tile has 4 faces:
 * it can connect to an adjacent tile darkly, lightly, or not at all
 * a dark face means the whole side is black
 * a light face means the side is a gradient *
 *
 *
 * graphics cache
 * fog types:
 * 1 = light corner se: connects lightly to south and east
 * 2 = light corner sw: connects lightly to south and west
 * 3 = light corner nw: connects lightly to north and west
 * 4 = light corner ne: connects lightly to north and east
 * 5 = dark corner nw: connects darkly to north and west, lightly to south and east
 * 6 = dark corner ne: connects darkly to north and east, lightly to south and west
 * 7 = dark corner se: connects darkly to east and south, lightly to north and west
 * 8 = dark corner sw: connects darkly to south and west,  lightly to north and east
 * 9 = bottom to top: connects darkly to south, connects lightly to east and west
 * 10 =  top to bottom: connects darkly to north, connects lightly to east and west
 * 11 = right to left: connects darkly to west, connects lightly to north and south
 * 12 = left to right: connects darkly to east, connects lightly to north and south
 * 13 = all black: connects darkly to north, south, east and west
 *
 * bigmap tile processing algorithm:
 * for each tile currently being shown, examine each tile in normal order
 *
 * here are the cases that we can handle for fogging a tile:
 * the sets are read as follows:
 *
 * 876
 * 5x4 = (fog tile type)
 * 321
 * (bit count)
 *
 * the most significant bit is in the 1st position, and the least sigificant bit
 * is in the 8th position
 *
 * we check each position and if it's a fogged tile, then we add a 1 to that
 * bit position.
 *
 * 111
 * 1x1 = 13
 * 111
 * (255)  (all 8 bits on is 255)
 *
 * 111      111      011     110
 * 1x1 = 5  1x1 = 6  1x1 = 7 1x1 = 8
 * 110      011      111     111
 * (127)    (223)    (254)   (251) (e.g. 251 == 11111011)
 *
 * 101      111      111     111
 * 1x1 = 9  1x0 = 12 1x1 =10 0x1 = 11
 * 111      111      101     111
 * (253)    (239)    (191)   (247) (e.g. 247 == 11110111)
 *
 * 001      111      100     111
 * 1x1 = 9  1x1 = 10 1x1 = 9 1x1 = 10
 * 111      001      111     100
 * (252)    (159)    (249)   (63)
 *
 * 011      111      110     111
 * 0x1 = 11 0x1 = 11 1x0 =12 1x0 = 12
 * 111      011      111     110
 * (246)    (215)    (235)   (111)
 *
 * 000      000      110      011
 * 0x1 = 1  1x0 = 2  1x0 = 3  0x1 = 4
 * 011      110      000      000
 * (208)    (104)    (11)     (22)
 *
 *
 * 000      111      011      110
 * 1x1 = 9  1x1 = 10 0x1 = 11 1x0 = 12
 * 111      000      011      110
 * (248)    (31)     (214)    (107)
 *
 *
 * 001      111      100     111
 * 0x1 = 1  0x1 = 4  1x0 = 2 1x0 = 3
 * 111      001      111     100
 * (244)    (151)    (233)   (47)
 *
 * 000      011      000     111
 * 0x1 = 1  0x1 = 4  1x0 = 2 1x0 = 3
 * 111      001      111     000
 * (240)    (150)    (232)   (15)
 *
 * 100      110      001     111
 * 1x0 = 2  1x0 = 3  0x1 = 1 0x1 = 4
 * 110      100      011     000
 * (105)    (43)     (232)   (15)
 *
 * 011      110
 * 1x1 = 14 1x1 = 15
 * 110      011
 * (126)    (219)
 *
 *special note:
 *none of these sets contain a so-called "lone" tile.
 *a lone tile is a fogged tile surrounded by two unfogged tiles on either side.
**/
FogMap::ShadeType FogMap::calculateShade(Vector<int> tile)
{
  int idx = 0;
  int count = 0;
  bool foggyTile;
  for (int i = tile.x - 1; i <= tile.x + 1; i++)
    for (int j = tile.y - 1; j <= tile.y + 1; j++)
      {
	foggyTile = false;
	if (i == tile.x && j == tile.y)
	  continue;
	if (i < 0 || j < 0 || i >= d_width || j >= d_height)
	  foggyTile = true;
	else
	  {
	    Vector<int> pos;
	    pos.x = i;
	    pos.y = j;
	    foggyTile = isFogged(pos);
	  }
	if (foggyTile)
	  {
	    switch (count)
	      {
	      case 0: idx += 1; break;
	      case 1: idx += 2; break;
	      case 2: idx += 4; break;
	      case 3: idx += 8; break;
	      case 4: idx += 16; break;
	      case 5: idx += 32; break;
	      case 6: idx += 64; break;
	      case 7: idx += 128; break;
	      }
	  }

	count++;
      }

  //now idx relates to a particular fog picture
  ShadeType type = NONE;
  switch (idx)
    {
    case 208: case 212: case 240: case 244: case 242: case 216: case 220: case 210: case 217: case 211: case 218: case 209: type = LIGHTLY_TO_SOUTH_AND_EAST; break;
    case 104: case 105: case 232: case 233: case 121: case 120: case 110: case 106: case 122: case 124: case 234: case 108: type = LIGHTLY_TO_SOUTH_AND_WEST; break;
    case  11: case 15: case 43: case 47: case 59: case 27: case 79: case 75: case 155: case 203: case 139: case 91: type = LIGHTLY_TO_NORTH_AND_WEST; break;
    case  22: case 150: case 151: case 23: case 87: case 86: case 158: case 118:case 94: case 30: case 62: case 54: type = LIGHTLY_TO_NORTH_AND_EAST; break;
    case 127: type = DARKLY_TO_NORTH_AND_WEST_LIGHTLY_TO_SOUTH_AND_EAST; break;
    case 223: type = DARKLY_TO_NORTH_AND_EAST_LIGHTLY_TO_SOUTH_AND_WEST; break;
    case 254: type = DARKLY_TO_SOUTH_AND_EAST_LIGHTLY_TO_NORTH_AND_WEST; break;
    case 251: type = DARKLY_TO_SOUTH_AND_WEST_LIGHTLY_TO_NORTH_AND_EAST; break;
    case 248: case 249: case 252: case 253: case 250: type = DARKLY_TO_SOUTH_LIGHTLY_TO_EAST_AND_WEST; break;
    case  31: case 63: case 159: case 191: case 95: type = DARKLY_TO_NORTH_LIGHTLY_TO_EAST_AND_WEST; break;
    case 214: case 215: case 246: case 247: case 222: type = DARKLY_TO_WEST_LIGHTLY_TO_NORTH_AND_SOUTH; break;
    case 107: case 111: case 235: case 239: case 123: type = DARKLY_TO_EAST_LIGHTLY_TO_NORTH_AND_SOUTH; break;
    case 126: type = DARKLY_TO_SOUTH_AND_WEST_DARKLY_TO_NORTH_AND_EAST; break;
    case 219: type = DARKLY_TO_NORTH_AND_WEST_DARKLY_TO_SOUTH_AND_EAST; break;
    case 255: type = ALL; break;
    }
  if (type)
    {
      switch (type) //fixme: figure out why this flipping is necessary!
	{
	case DARKLY_TO_EAST_LIGHTLY_TO_NORTH_AND_SOUTH: 
	  type = DARKLY_TO_NORTH_LIGHTLY_TO_EAST_AND_WEST; break;
	case DARKLY_TO_NORTH_LIGHTLY_TO_EAST_AND_WEST: 
	  type = DARKLY_TO_EAST_LIGHTLY_TO_NORTH_AND_SOUTH; break;
	case DARKLY_TO_SOUTH_LIGHTLY_TO_EAST_AND_WEST: 
	  type = DARKLY_TO_WEST_LIGHTLY_TO_NORTH_AND_SOUTH; break;
	case DARKLY_TO_WEST_LIGHTLY_TO_NORTH_AND_SOUTH: 
	  type = DARKLY_TO_SOUTH_LIGHTLY_TO_EAST_AND_WEST; break;
	case DARKLY_TO_NORTH_AND_EAST_LIGHTLY_TO_SOUTH_AND_WEST: 
	  type = DARKLY_TO_SOUTH_AND_WEST_LIGHTLY_TO_NORTH_AND_EAST; break;
	case DARKLY_TO_SOUTH_AND_WEST_LIGHTLY_TO_NORTH_AND_EAST: 
	  type = DARKLY_TO_NORTH_AND_EAST_LIGHTLY_TO_SOUTH_AND_WEST; break;
	case LIGHTLY_TO_SOUTH_AND_WEST: 
	  type = LIGHTLY_TO_NORTH_AND_EAST; break;
	case LIGHTLY_TO_NORTH_AND_EAST: 
	  type = LIGHTLY_TO_SOUTH_AND_WEST; break;
	default:break;
	}
    }
  return type;
}

void FogMap::calculateShadeMap()
{
  for (int i = 0; i < d_width; i++)
    for (int j = 0; j < d_height; j++)
      shademap[j * d_width + i] = calculateShade(Vector<int>(i,j));

  for (int i = 0; i < d_width; i++)
    for (int j = 0; j < d_height; j++)
      if (isFogged(Vector<int>(i,j)) == false)
	  shademap[j * d_width + i] = NONE;
}
// End of file
