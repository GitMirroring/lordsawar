//  Copyright (C) 2017, 2021 Ben Asselstine
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

#include "stone.h"
#include "GameMap.h"
#include "xmlhelper.h"
#include "rnd.h"

Glib::ustring Stone::d_tag = "stone";

Stone::Stone(Vector<int> pos, int type)
  :Location(pos), d_type(type)
{
    //mark the location on the game map as occupied by a stone
    if (GameMap::getInstance()->getTile(getPos())->getBuilding() !=
        Maptile::ROAD)
      GameMap::getInstance()->getTile(getPos())->setBuilding(Maptile::STONE);
}

Stone::Stone(XML_Helper* helper)
    :Location(helper)
{
  Glib::ustring type_str;
  helper->getData(type_str, "type");
  d_type = stoneTypeFromString(type_str);
    
  //mark the location on the game map as occupied by a stone
  Maptile *mtile = GameMap::getInstance()->getTile(getPos());
  if (mtile->getBuilding() != Maptile::ROAD)
    mtile->setBuilding(Maptile::STONE);
}

Stone::Stone(const Stone& s, bool sync_id)
  :Location(s, sync_id), d_type(s.d_type)
{
}

Stone::Stone(const Stone& s, Vector<int> pos)
  :Location(s, pos), d_type(s.d_type)
{
}

bool Stone::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(Stone::d_tag);
    retval &= helper->saveData("id", d_id);
    retval &= helper->saveData("x", getPos().x);
    retval &= helper->saveData("y", getPos().y);
    Glib::ustring type_str = stoneTypeToString(Stone::Type(d_type));
    retval &= helper->saveData("type", type_str);
    retval &= helper->closeTag();
    
    return retval;
}

std::vector<Stone::Type> Stone::getSuitableTypes (const Road::Type type)
{
  std::vector<Stone::Type> stones;
  switch (type)
    {
    case Road::CONNECTS_EAST_AND_WEST:
        {
          stones.push_back (ROAD_E_AND_W_STONE_N);
          stones.push_back (ROAD_E_AND_W_STONE_S);
          stones.push_back (ROAD_E_AND_W_STONES_N_AND_S);
        }
      break;
    case Road::CONNECTS_NORTH_AND_SOUTH:
          stones.push_back (ROAD_N_AND_S_STONE_W);
          stones.push_back (ROAD_N_AND_S_STONE_E);
          stones.push_back (ROAD_N_AND_S_STONES_W_AND_E);
      break;
    case Road::CONNECTS_ALL_DIRECTIONS:
          stones.push_back (ROAD_ALL_DIRECTIONS_STONE_NW);
          stones.push_back (ROAD_ALL_DIRECTIONS_STONE_NE);
          stones.push_back (ROAD_ALL_DIRECTIONS_STONE_SW);
          stones.push_back (ROAD_ALL_DIRECTIONS_STONE_SE);
          stones.push_back (ROAD_ALL_DIRECTIONS_STONES_NW_NE);
          stones.push_back (ROAD_ALL_DIRECTIONS_STONES_SW_SE);
          stones.push_back (ROAD_ALL_DIRECTIONS_STONES_NW_SW);
          stones.push_back (ROAD_ALL_DIRECTIONS_STONES_NE_SE);
          stones.push_back (ROAD_ALL_DIRECTIONS_STONES_NW_SE);
          stones.push_back (ROAD_ALL_DIRECTIONS_STONES_NE_SW);
          stones.push_back (ROAD_ALL_DIRECTIONS_STONES_NE_SW_SE);
          stones.push_back (ROAD_ALL_DIRECTIONS_STONES_NW_SW_SE);
          stones.push_back (ROAD_ALL_DIRECTIONS_STONES_NW_NE_SE);
          stones.push_back (ROAD_ALL_DIRECTIONS_STONES_NW_NE_SW);
          stones.push_back (ROAD_ALL_DIRECTIONS_STONES_NW_NE_SW_SE);
      break;
    case Road::CONNECTS_NORTH_AND_WEST:
          stones.push_back (ROAD_N_AND_W_STONE_NW);
          stones.push_back (ROAD_N_AND_W_STONE_SE);
          stones.push_back (ROAD_N_AND_W_STONES_NW_SE);
      break;
    case Road::CONNECTS_NORTH_AND_EAST:
          stones.push_back (ROAD_N_AND_E_STONE_NE);
          stones.push_back (ROAD_N_AND_E_STONE_SW);
          stones.push_back (ROAD_N_AND_E_STONES_NE_SW);
      break;
    case Road::CONNECTS_SOUTH_AND_EAST:
          stones.push_back (ROAD_S_AND_E_STONE_NW);
          stones.push_back (ROAD_S_AND_E_STONE_SE);
          stones.push_back (ROAD_S_AND_E_STONES_NW_SE);
      break;
    case Road::CONNECTS_WEST_AND_SOUTH:
          stones.push_back (ROAD_W_AND_S_STONE_SW);
          stones.push_back (ROAD_W_AND_S_STONE_NE);
          stones.push_back (ROAD_W_AND_S_STONES_SW_NE);
      break;
    case Road::CONNECTS_NORTH_AND_SOUTH_AND_EAST:
          stones.push_back (ROAD_N_AND_S_AND_E_STONE_NE);
          stones.push_back (ROAD_N_AND_S_AND_E_STONE_SE);
          stones.push_back (ROAD_N_AND_S_AND_E_STONE_W);
          stones.push_back (ROAD_N_AND_S_AND_E_STONES_SE_W);
          stones.push_back (ROAD_N_AND_S_AND_E_STONES_NE_W);
          stones.push_back (ROAD_N_AND_S_AND_E_STONES_NE_SE);
          stones.push_back (ROAD_N_AND_S_AND_E_STONES_NE_SE_W);
      break;
    case Road::CONNECTS_EAST_WEST_AND_NORTH:
          stones.push_back (ROAD_E_W_AND_N_STONE_NW);
          stones.push_back (ROAD_E_W_AND_N_STONE_NE);
          stones.push_back (ROAD_E_W_AND_N_STONE_S);
          stones.push_back (ROAD_E_W_AND_N_STONES_NE_S);
          stones.push_back (ROAD_E_W_AND_N_STONES_NW_S);
          stones.push_back (ROAD_E_W_AND_N_STONES_NW_NE);
          stones.push_back (ROAD_E_W_AND_N_STONES_NW_NE_S);
      break;
    case Road::CONNECTS_EAST_WEST_AND_SOUTH:
          stones.push_back (ROAD_E_W_AND_S_STONE_SW);
          stones.push_back (ROAD_E_W_AND_S_STONE_SE);
          stones.push_back (ROAD_E_W_AND_S_STONE_N);
          stones.push_back (ROAD_E_W_AND_S_STONES_SE_N);
          stones.push_back (ROAD_E_W_AND_S_STONES_SW_N);
          stones.push_back (ROAD_E_W_AND_S_STONES_SW_SE);
          stones.push_back (ROAD_E_W_AND_S_STONES_SW_SE_N);
      break;
    case Road::CONNECTS_NORTH_SOUTH_AND_WEST:
          stones.push_back (ROAD_N_S_AND_W_STONE_NW);
          stones.push_back (ROAD_N_S_AND_W_STONE_SW);
          stones.push_back (ROAD_N_S_AND_W_STONE_E);
          stones.push_back (ROAD_N_S_AND_W_STONES_SW_E);
          stones.push_back (ROAD_N_S_AND_W_STONES_NW_E);
          stones.push_back (ROAD_N_S_AND_W_STONES_NW_SW);
          stones.push_back (ROAD_N_S_AND_W_STONES_NW_SW_E);
      break;
    case Road::CONNECTS_NORTH:
          stones.push_back (ROAD_N_STONE_W);
          stones.push_back (ROAD_N_STONE_E);
          stones.push_back (ROAD_N_STONE_S);
          stones.push_back (ROAD_N_STONES_E_S);
          stones.push_back (ROAD_N_STONES_W_S);
          stones.push_back (ROAD_N_STONES_W_E);
          stones.push_back (ROAD_N_STONES_W_E_S);
      break;
    case Road::CONNECTS_SOUTH:
          stones.push_back (ROAD_S_STONE_W);
          stones.push_back (ROAD_S_STONE_E);
          stones.push_back (ROAD_S_STONE_N);
          stones.push_back (ROAD_S_STONES_E_W);
          stones.push_back (ROAD_S_STONES_N_W);
          stones.push_back (ROAD_S_STONES_N_E);
          stones.push_back (ROAD_S_STONES_N_E_W);
      break;
    case Road::CONNECTS_EAST:
          stones.push_back (ROAD_E_STONE_N);
          stones.push_back (ROAD_E_STONE_S);
          stones.push_back (ROAD_E_STONE_W);
          stones.push_back (ROAD_E_STONES_S_W);
          stones.push_back (ROAD_E_STONES_N_W);
          stones.push_back (ROAD_E_STONES_N_S);
          stones.push_back (ROAD_E_STONES_N_S_W);
      break;
    case Road::CONNECTS_WEST:
          stones.push_back (ROAD_W_STONE_N);
          stones.push_back (ROAD_W_STONE_S);
          stones.push_back (ROAD_W_STONE_E);
          stones.push_back (ROAD_W_STONES_S_E);
          stones.push_back (ROAD_W_STONES_N_E);
          stones.push_back (ROAD_W_STONES_N_S);
          stones.push_back (ROAD_W_STONES_N_S_E);
      break;
    }
  return stones;
}

std::vector<Stone::Type> Stone::getSuitableTypes ()
{
  std::vector<Stone::Type> stones;
  stones.push_back (ROAD_E_AND_W_STONE_N);
  stones.push_back (ROAD_E_AND_W_STONE_S);
  stones.push_back (ROAD_N_AND_S_STONE_W);
  stones.push_back (ROAD_N_AND_S_STONE_E);
  stones.push_back (ROAD_ALL_DIRECTIONS_STONE_NW);
  stones.push_back (ROAD_ALL_DIRECTIONS_STONE_NE);
  stones.push_back (ROAD_ALL_DIRECTIONS_STONE_SW);
  stones.push_back (ROAD_ALL_DIRECTIONS_STONE_SE);
  stones.push_back (ROAD_N_AND_W_STONE_NW);
  stones.push_back (ROAD_N_AND_W_STONE_SE);
  stones.push_back (ROAD_N_AND_E_STONE_NE);
  stones.push_back (ROAD_N_AND_E_STONE_SW);
  stones.push_back (ROAD_S_AND_E_STONE_NW);
  stones.push_back (ROAD_S_AND_E_STONE_SE);
  stones.push_back (ROAD_W_AND_S_STONE_SW);
  stones.push_back (ROAD_W_AND_S_STONE_NE);
  stones.push_back (ROAD_N_AND_S_AND_E_STONE_NE);
  stones.push_back (ROAD_N_AND_S_AND_E_STONE_SE);
  stones.push_back (ROAD_N_AND_S_AND_E_STONE_W);
  stones.push_back (ROAD_E_W_AND_N_STONE_NW);
  stones.push_back (ROAD_E_W_AND_N_STONE_NE);
  stones.push_back (ROAD_E_W_AND_N_STONE_S);
  stones.push_back (ROAD_E_W_AND_S_STONE_SW);
  stones.push_back (ROAD_E_W_AND_S_STONE_SE);
  stones.push_back (ROAD_E_W_AND_S_STONE_N);
  stones.push_back (ROAD_N_S_AND_W_STONE_NW);
  stones.push_back (ROAD_N_S_AND_W_STONE_SW);
  stones.push_back (ROAD_N_S_AND_W_STONE_E);
  stones.push_back (ROAD_W_STONE_N);
  stones.push_back (ROAD_W_STONE_S);
  stones.push_back (ROAD_W_STONE_E);
  stones.push_back (ROAD_N_STONE_W);
  stones.push_back (ROAD_N_STONE_E);
  stones.push_back (ROAD_N_STONE_S);
  stones.push_back (ROAD_E_STONE_N);
  stones.push_back (ROAD_E_STONE_S);
  stones.push_back (ROAD_E_STONE_W);
  stones.push_back (ROAD_S_STONE_W);
  stones.push_back (ROAD_S_STONE_E);
  stones.push_back (ROAD_S_STONE_N);
  return stones;
}

int Stone::getRandomType (const Road::Type type)
{
  std::vector<Stone::Type> stones = Stone::getSuitableTypes(type);

  if (stones.empty())
    return Stone::ROAD_E_AND_W_STONE_N;
  return stones[Rnd::rand() % stones.size()];
}

int Stone::getRandomType ()
{
  std::vector<Stone::Type> stones = Stone::getSuitableTypes();

  if (stones.empty())
    return Stone::ROAD_E_AND_W_STONE_N;
  return stones[Rnd::rand() % stones.size()];
}

std::vector<Stone::Type> Stone::getTypes()
{
  std::vector<Stone::Type> stones;
  for (int i = 0; i < int(ROAD_S_STONES_N_E_W); i++)
    stones.push_back (Stone::Type(i));
  return stones;
}

Glib::ustring Stone::stoneTypeToString(const Stone::Type type)
{
  switch (type)
    {
    case Stone::ROAD_E_AND_W_STONE_N:
      return "Stone::ROAD_E_AND_W_STONE_N";
    case Stone::ROAD_E_AND_W_STONE_S:
      return "Stone::ROAD_E_AND_W_STONE_S";
    case Stone::ROAD_E_AND_W_STONES_N_AND_S:
      return "Stone::ROAD_E_AND_W_STONES_N_AND_S";
    case Stone::ROAD_N_AND_S_STONE_W:
      return "Stone::ROAD_N_AND_S_STONE_W";
    case Stone::ROAD_N_AND_S_STONE_E:
      return "Stone::ROAD_N_AND_S_STONE_E";
    case Stone::ROAD_N_AND_S_STONES_W_AND_E:
      return "Stone::ROAD_N_AND_S_STONES_W_AND_E";
    case Stone::ROAD_ALL_DIRECTIONS_STONE_NW:
      return "Stone::ROAD_ALL_DIRECTIONS_STONE_NW";
    case Stone::ROAD_ALL_DIRECTIONS_STONE_NE:
      return "Stone::ROAD_ALL_DIRECTIONS_STONE_NE";
    case Stone::ROAD_ALL_DIRECTIONS_STONE_SW:
      return "Stone::ROAD_ALL_DIRECTIONS_STONE_SW";
    case Stone::ROAD_ALL_DIRECTIONS_STONE_SE:
      return "Stone::ROAD_ALL_DIRECTIONS_STONE_SE";
    case Stone::ROAD_ALL_DIRECTIONS_STONES_NW_NE:
      return "Stone::ROAD_ALL_DIRECTIONS_STONES_NW_NE";
    case Stone::ROAD_ALL_DIRECTIONS_STONES_SW_SE:
      return "Stone::ROAD_ALL_DIRECTIONS_STONES_SW_SE";
    case Stone::ROAD_ALL_DIRECTIONS_STONES_NW_SW:
      return "Stone::ROAD_ALL_DIRECTIONS_STONES_NW_SW";
    case Stone::ROAD_ALL_DIRECTIONS_STONES_NE_SE:
      return "Stone::ROAD_ALL_DIRECTIONS_STONES_NE_SE";
    case Stone::ROAD_ALL_DIRECTIONS_STONES_NW_SE:
      return "Stone::ROAD_ALL_DIRECTIONS_STONES_NW_SE";
    case Stone::ROAD_ALL_DIRECTIONS_STONES_NE_SW:
      return "Stone::ROAD_ALL_DIRECTIONS_STONES_NE_SW";
    case Stone::ROAD_ALL_DIRECTIONS_STONES_NE_SW_SE:
      return "Stone::ROAD_ALL_DIRECTIONS_STONES_NE_SW_SE";
    case Stone::ROAD_ALL_DIRECTIONS_STONES_NW_SW_SE:
      return "Stone::ROAD_ALL_DIRECTIONS_STONES_NW_SW_SE";
    case Stone::ROAD_ALL_DIRECTIONS_STONES_NW_NE_SE:
      return "Stone::ROAD_ALL_DIRECTIONS_STONES_NW_NE_SE";
    case Stone::ROAD_ALL_DIRECTIONS_STONES_NW_NE_SW:
      return "Stone::ROAD_ALL_DIRECTIONS_STONES_NW_NE_SW";
    case Stone::ROAD_ALL_DIRECTIONS_STONES_NW_NE_SW_SE:
      return "Stone::ROAD_ALL_DIRECTIONS_STONES_NW_NE_SW_SE";
    case Stone::ROAD_N_AND_W_STONE_NW:
      return "Stone::ROAD_N_AND_W_STONE_NW";
    case Stone::ROAD_N_AND_W_STONE_SE:
      return "Stone::ROAD_N_AND_W_STONE_SE";
    case Stone::ROAD_N_AND_W_STONES_NW_SE:
      return "Stone::ROAD_N_AND_W_STONES_NW_SE";
    case Stone::ROAD_N_AND_E_STONE_NE:
      return "Stone::ROAD_N_AND_E_STONE_NE";
    case Stone::ROAD_N_AND_E_STONE_SW:
      return "Stone::ROAD_N_AND_E_STONE_SW";
    case Stone::ROAD_N_AND_E_STONES_NE_SW:
      return "Stone::ROAD_N_AND_E_STONES_NE_SW";
    case Stone::ROAD_S_AND_E_STONE_NW:
      return "Stone::ROAD_S_AND_E_STONE_NW";
    case Stone::ROAD_S_AND_E_STONE_SE:
      return "Stone::ROAD_S_AND_E_STONE_SE";
    case Stone::ROAD_S_AND_E_STONES_NW_SE:
      return "Stone::ROAD_S_AND_E_STONES_NW_SE";
    case Stone::ROAD_W_AND_S_STONE_SW:
      return "Stone::ROAD_W_AND_S_STONE_SW";
    case Stone::ROAD_W_AND_S_STONE_NE:
      return "Stone::ROAD_W_AND_S_STONE_NE";
    case Stone::ROAD_W_AND_S_STONES_SW_NE:
      return "Stone::ROAD_W_AND_S_STONES_SW_NE";
    case Stone::ROAD_N_AND_S_AND_E_STONE_NE:
      return "Stone::ROAD_N_AND_S_AND_E_STONE_NE";
    case Stone::ROAD_N_AND_S_AND_E_STONE_SE:
      return "Stone::ROAD_N_AND_S_AND_E_STONE_SE";
    case Stone::ROAD_N_AND_S_AND_E_STONE_W:
      return "Stone::ROAD_N_AND_S_AND_E_STONE_W";
    case Stone::ROAD_N_AND_S_AND_E_STONES_SE_W:
      return "Stone::ROAD_N_AND_S_AND_E_STONES_SE_W";
    case Stone::ROAD_N_AND_S_AND_E_STONES_NE_W:
      return "Stone::ROAD_N_AND_S_AND_E_STONES_NE_W";
    case Stone::ROAD_N_AND_S_AND_E_STONES_NE_SE:
      return "Stone::ROAD_N_AND_S_AND_E_STONES_NE_SE";
    case Stone::ROAD_N_AND_S_AND_E_STONES_NE_SE_W:
      return "Stone::ROAD_N_AND_S_AND_E_STONES_NE_SE_W";
    case Stone::ROAD_E_W_AND_N_STONE_NW:
      return "Stone::ROAD_E_W_AND_N_STONE_NW";
    case Stone::ROAD_E_W_AND_N_STONE_NE:
      return "Stone::ROAD_E_W_AND_N_STONE_NE";
    case Stone::ROAD_E_W_AND_N_STONE_S:
      return "Stone::ROAD_E_W_AND_N_STONE_S";
    case Stone::ROAD_E_W_AND_N_STONES_NE_S:
      return "Stone::ROAD_E_W_AND_N_STONES_NE_S";
    case Stone::ROAD_E_W_AND_N_STONES_NW_S:
      return "Stone::ROAD_E_W_AND_N_STONES_NW_S";
    case Stone::ROAD_E_W_AND_N_STONES_NW_NE:
      return "Stone::ROAD_E_W_AND_N_STONES_NW_NE";
    case Stone::ROAD_E_W_AND_N_STONES_NW_NE_S:
      return "Stone::ROAD_E_W_AND_N_STONES_NW_NE_S";
    case Stone::ROAD_E_W_AND_S_STONE_SW:
      return "Stone::ROAD_E_W_AND_S_STONE_SW";
    case Stone::ROAD_E_W_AND_S_STONE_SE:
      return "Stone::ROAD_E_W_AND_S_STONE_SE";
    case Stone::ROAD_E_W_AND_S_STONE_N:
      return "Stone::ROAD_E_W_AND_S_STONE_N";
    case Stone::ROAD_E_W_AND_S_STONES_SE_N:
      return "Stone::ROAD_E_W_AND_S_STONES_SE_N";
    case Stone::ROAD_E_W_AND_S_STONES_SW_N:
      return "Stone::ROAD_E_W_AND_S_STONES_SW_N";
    case Stone::ROAD_E_W_AND_S_STONES_SW_SE:
      return "Stone::ROAD_E_W_AND_S_STONES_SW_SE";
    case Stone::ROAD_E_W_AND_S_STONES_SW_SE_N:
      return "Stone::ROAD_E_W_AND_S_STONES_SW_SE_N";
    case Stone::ROAD_N_S_AND_W_STONE_NW:
      return "Stone::ROAD_N_S_AND_W_STONE_NW";
    case Stone::ROAD_N_S_AND_W_STONE_SW:
      return "Stone::ROAD_N_S_AND_W_STONE_SW";
    case Stone::ROAD_N_S_AND_W_STONE_E:
      return "Stone::ROAD_N_S_AND_W_STONE_E";
    case Stone::ROAD_N_S_AND_W_STONES_SW_E:
      return "Stone::ROAD_N_S_AND_W_STONES_SW_E";
    case Stone::ROAD_N_S_AND_W_STONES_NW_E:
      return "Stone::ROAD_N_S_AND_W_STONES_NW_E";
    case Stone::ROAD_N_S_AND_W_STONES_NW_SW:
      return "Stone::ROAD_N_S_AND_W_STONES_NW_SW";
    case Stone::ROAD_N_S_AND_W_STONES_NW_SW_E:
      return "Stone::ROAD_N_S_AND_W_STONES_NW_SW_E";
    case Stone::ROAD_W_STONE_N:
      return "Stone::ROAD_W_STONE_N";
    case Stone::ROAD_W_STONE_S:
      return "Stone::ROAD_W_STONE_S";
    case Stone::ROAD_W_STONE_E:
      return "Stone::ROAD_W_STONE_E";
    case Stone::ROAD_W_STONES_S_E:
      return "Stone::ROAD_W_STONES_S_E";
    case Stone::ROAD_W_STONES_N_E:
      return "Stone::ROAD_W_STONES_N_E";
    case Stone::ROAD_W_STONES_N_S:
      return "Stone::ROAD_W_STONES_N_S";
    case Stone::ROAD_W_STONES_N_S_E:
      return "Stone::ROAD_W_STONES_N_S_E";
    case Stone::ROAD_N_STONE_W:
      return "Stone::ROAD_N_STONE_W";
    case Stone::ROAD_N_STONE_E:
      return "Stone::ROAD_N_STONE_E";
    case Stone::ROAD_N_STONE_S:
      return "Stone::ROAD_N_STONE_S";
    case Stone::ROAD_N_STONES_E_S:
      return "Stone::ROAD_N_STONES_E_S";
    case Stone::ROAD_N_STONES_W_S:
      return "Stone::ROAD_N_STONES_W_S";
    case Stone::ROAD_N_STONES_W_E:
      return "Stone::ROAD_N_STONES_W_E";
    case Stone::ROAD_N_STONES_W_E_S:
      return "Stone::ROAD_N_STONES_W_E_S";
    case Stone::ROAD_E_STONE_N:
      return "Stone::ROAD_E_STONE_N";
    case Stone::ROAD_E_STONE_S:
      return "Stone::ROAD_E_STONE_S";
    case Stone::ROAD_E_STONE_W:
      return "Stone::ROAD_E_STONE_W";
    case Stone::ROAD_E_STONES_S_W:
      return "Stone::ROAD_E_STONES_S_W";
    case Stone::ROAD_E_STONES_N_W:
      return "Stone::ROAD_E_STONES_N_W";
    case Stone::ROAD_E_STONES_N_S:
      return "Stone::ROAD_E_STONES_N_S";
    case Stone::ROAD_E_STONES_N_S_W:
      return "Stone::ROAD_E_STONES_N_S_W";
    case Stone::ROAD_S_STONE_W:
      return "Stone::ROAD_S_STONE_W";
    case Stone::ROAD_S_STONE_E:
      return "Stone::ROAD_S_STONE_E";
    case Stone::ROAD_S_STONE_N:
      return "Stone::ROAD_S_STONE_N";
    case Stone::ROAD_S_STONES_E_W:
      return "Stone::ROAD_S_STONES_E_W";
    case Stone::ROAD_S_STONES_N_W:
      return "Stone::ROAD_S_STONES_N_W";
    case Stone::ROAD_S_STONES_N_E:
      return "Stone::ROAD_S_STONES_N_E";
    case Stone::ROAD_S_STONES_N_E_W:
      return "Stone::ROAD_S_STONES_N_E_W";
    }
  return "Stone::ROAD_E_AND_W_STONE_N";
}

Stone::Type Stone::stoneTypeFromString(const Glib::ustring str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return Stone::Type(atoi(str.c_str()));
  if (str == "Stone::ROAD_E_AND_W_STONE_N")
    return Stone::ROAD_E_AND_W_STONE_N;
  else if (str == "Stone::ROAD_E_AND_W_STONE_S")
    return Stone::ROAD_E_AND_W_STONE_S;
  else if (str == "Stone::ROAD_E_AND_W_STONES_N_AND_S")
    return Stone::ROAD_E_AND_W_STONES_N_AND_S;
  else if (str == "Stone::ROAD_N_AND_S_STONE_W")
    return Stone::ROAD_N_AND_S_STONE_W;
  else if (str == "Stone::ROAD_N_AND_S_STONE_E")
    return Stone::ROAD_N_AND_S_STONE_E;
  else if (str == "Stone::ROAD_N_AND_S_STONES_W_AND_E")
    return Stone::ROAD_N_AND_S_STONES_W_AND_E;
  else if (str == "Stone::ROAD_ALL_DIRECTIONS_STONE_NW")
    return Stone::ROAD_ALL_DIRECTIONS_STONE_NW;
  else if (str == "Stone::ROAD_ALL_DIRECTIONS_STONE_NE")
    return Stone::ROAD_ALL_DIRECTIONS_STONE_NE;
  else if (str == "Stone::ROAD_ALL_DIRECTIONS_STONE_SW")
    return Stone::ROAD_ALL_DIRECTIONS_STONE_SW;
  else if (str == "Stone::ROAD_ALL_DIRECTIONS_STONE_SE")
    return Stone::ROAD_ALL_DIRECTIONS_STONE_SE;
  else if (str == "Stone::ROAD_ALL_DIRECTIONS_STONES_NW_NE")
    return Stone::ROAD_ALL_DIRECTIONS_STONES_NW_NE;
  else if (str == "Stone::ROAD_ALL_DIRECTIONS_STONES_SW_SE")
    return Stone::ROAD_ALL_DIRECTIONS_STONES_SW_SE;
  else if (str == "Stone::ROAD_ALL_DIRECTIONS_STONES_NW_SW")
    return Stone::ROAD_ALL_DIRECTIONS_STONES_NW_SW;
  else if (str == "Stone::ROAD_ALL_DIRECTIONS_STONES_NE_SE")
    return Stone::ROAD_ALL_DIRECTIONS_STONES_NE_SE;
  else if (str == "Stone::ROAD_ALL_DIRECTIONS_STONES_NW_SE")
    return Stone::ROAD_ALL_DIRECTIONS_STONES_NW_SE;
  else if (str == "Stone::ROAD_ALL_DIRECTIONS_STONES_NE_SW")
    return Stone::ROAD_ALL_DIRECTIONS_STONES_NE_SW;
  else if (str == "Stone::ROAD_ALL_DIRECTIONS_STONES_NE_SW_SE")
    return Stone::ROAD_ALL_DIRECTIONS_STONES_NE_SW_SE;
  else if (str == "Stone::ROAD_ALL_DIRECTIONS_STONES_NW_SW_SE")
    return Stone::ROAD_ALL_DIRECTIONS_STONES_NW_SW_SE;
  else if (str == "Stone::ROAD_ALL_DIRECTIONS_STONES_NW_NE_SE")
    return Stone::ROAD_ALL_DIRECTIONS_STONES_NW_NE_SE;
  else if (str == "Stone::ROAD_ALL_DIRECTIONS_STONES_NW_NE_SW")
    return Stone::ROAD_ALL_DIRECTIONS_STONES_NW_NE_SW;
  else if (str == "Stone::ROAD_ALL_DIRECTIONS_STONES_NW_NE_SW_SE")
    return Stone::ROAD_ALL_DIRECTIONS_STONES_NW_NE_SW_SE;
  else if (str == "Stone::ROAD_N_AND_W_STONE_NW")
    return Stone::ROAD_N_AND_W_STONE_NW;
  else if (str == "Stone::ROAD_N_AND_W_STONE_SE")
    return Stone::ROAD_N_AND_W_STONE_SE;
  else if (str == "Stone::ROAD_N_AND_W_STONES_NW_SE")
    return Stone::ROAD_N_AND_W_STONES_NW_SE;
  else if (str == "Stone::ROAD_N_AND_E_STONE_NE")
    return Stone::ROAD_N_AND_E_STONE_NE;
  else if (str == "Stone::ROAD_N_AND_E_STONE_SW")
    return Stone::ROAD_N_AND_E_STONE_SW;
  else if (str == "Stone::ROAD_N_AND_E_STONES_NE_SW")
    return Stone::ROAD_N_AND_E_STONES_NE_SW;
  else if (str == "Stone::ROAD_S_AND_E_STONE_NW")
    return Stone::ROAD_S_AND_E_STONE_NW;
  else if (str == "Stone::ROAD_S_AND_E_STONE_SE")
    return Stone::ROAD_S_AND_E_STONE_SE;
  else if (str == "Stone::ROAD_S_AND_E_STONES_NW_SE")
    return Stone::ROAD_S_AND_E_STONES_NW_SE;
  else if (str == "Stone::ROAD_W_AND_S_STONE_SW")
    return Stone::ROAD_W_AND_S_STONE_SW;
  else if (str == "Stone::ROAD_W_AND_S_STONE_NE")
    return Stone::ROAD_W_AND_S_STONE_NE;
  else if (str == "Stone::ROAD_W_AND_S_STONES_SW_NE")
    return Stone::ROAD_W_AND_S_STONES_SW_NE;
  else if (str == "Stone::ROAD_N_AND_S_AND_E_STONE_NE")
    return Stone::ROAD_N_AND_S_AND_E_STONE_NE;
  else if (str == "Stone::ROAD_N_AND_S_AND_E_STONE_SE")
    return Stone::ROAD_N_AND_S_AND_E_STONE_SE;
  else if (str == "Stone::ROAD_N_AND_S_AND_E_STONE_W")
    return Stone::ROAD_N_AND_S_AND_E_STONE_W;
  else if (str == "Stone::ROAD_N_AND_S_AND_E_STONES_SE_W")
    return Stone::ROAD_N_AND_S_AND_E_STONES_SE_W;
  else if (str == "Stone::ROAD_N_AND_S_AND_E_STONES_NE_W")
    return Stone::ROAD_N_AND_S_AND_E_STONES_NE_W;
  else if (str == "Stone::ROAD_N_AND_S_AND_E_STONES_NE_SE")
    return Stone::ROAD_N_AND_S_AND_E_STONES_NE_SE;
  else if (str == "Stone::ROAD_N_AND_S_AND_E_STONES_NE_SE_W")
    return Stone::ROAD_N_AND_S_AND_E_STONES_NE_SE_W;
  else if (str == "Stone::ROAD_E_W_AND_N_STONE_NW")
    return Stone::ROAD_E_W_AND_N_STONE_NW;
  else if (str == "Stone::ROAD_E_W_AND_N_STONE_NE")
    return Stone::ROAD_E_W_AND_N_STONE_NE;
  else if (str == "Stone::ROAD_E_W_AND_N_STONE_S")
    return Stone::ROAD_E_W_AND_N_STONE_S;
  else if (str == "Stone::ROAD_E_W_AND_N_STONES_NE_S")
    return Stone::ROAD_E_W_AND_N_STONES_NE_S;
  else if (str == "Stone::ROAD_E_W_AND_N_STONES_NW_S")
    return Stone::ROAD_E_W_AND_N_STONES_NW_S;
  else if (str == "Stone::ROAD_E_W_AND_N_STONES_NW_NE")
    return Stone::ROAD_E_W_AND_N_STONES_NW_NE;
  else if (str == "Stone::ROAD_E_W_AND_N_STONES_NW_NE_S")
    return Stone::ROAD_E_W_AND_N_STONES_NW_NE_S;
  else if (str == "Stone::ROAD_E_W_AND_S_STONE_SW")
    return Stone::ROAD_E_W_AND_S_STONE_SW;
  else if (str == "Stone::ROAD_E_W_AND_S_STONE_SE")
    return Stone::ROAD_E_W_AND_S_STONE_SE;
  else if (str == "Stone::ROAD_E_W_AND_S_STONE_N")
    return Stone::ROAD_E_W_AND_S_STONE_N;
  else if (str == "Stone::ROAD_E_W_AND_S_STONES_SE_N")
    return Stone::ROAD_E_W_AND_S_STONES_SE_N;
  else if (str == "Stone::ROAD_E_W_AND_S_STONES_SW_N")
    return Stone::ROAD_E_W_AND_S_STONES_SW_N;
  else if (str == "Stone::ROAD_E_W_AND_S_STONES_SW_SE")
    return Stone::ROAD_E_W_AND_S_STONES_SW_SE;
  else if (str == "Stone::ROAD_E_W_AND_S_STONES_SW_SE_N")
    return Stone::ROAD_E_W_AND_S_STONES_SW_SE_N;
  else if (str == "Stone::ROAD_N_S_AND_W_STONE_NW")
    return Stone::ROAD_N_S_AND_W_STONE_NW;
  else if (str == "Stone::ROAD_N_S_AND_W_STONE_SW")
    return Stone::ROAD_N_S_AND_W_STONE_SW;
  else if (str == "Stone::ROAD_N_S_AND_W_STONE_E")
    return Stone::ROAD_N_S_AND_W_STONE_E;
  else if (str == "Stone::ROAD_N_S_AND_W_STONES_SW_E")
    return Stone::ROAD_N_S_AND_W_STONES_SW_E;
  else if (str == "Stone::ROAD_N_S_AND_W_STONES_NW_E")
    return Stone::ROAD_N_S_AND_W_STONES_NW_E;
  else if (str == "Stone::ROAD_N_S_AND_W_STONES_NW_SW")
    return Stone::ROAD_N_S_AND_W_STONES_NW_SW;
  else if (str == "Stone::ROAD_N_S_AND_W_STONES_NW_SW_E")
    return Stone::ROAD_N_S_AND_W_STONES_NW_SW_E;
  else if (str == "Stone::ROAD_W_STONE_N")
    return Stone::ROAD_W_STONE_N;
  else if (str == "Stone::ROAD_W_STONE_S")
    return Stone::ROAD_W_STONE_S;
  else if (str == "Stone::ROAD_W_STONE_E")
    return Stone::ROAD_W_STONE_E;
  else if (str == "Stone::ROAD_W_STONES_S_E")
    return Stone::ROAD_W_STONES_S_E;
  else if (str == "Stone::ROAD_W_STONES_N_E")
    return Stone::ROAD_W_STONES_N_E;
  else if (str == "Stone::ROAD_W_STONES_N_S")
    return Stone::ROAD_W_STONES_N_S;
  else if (str == "Stone::ROAD_W_STONES_N_S_E")
    return Stone::ROAD_W_STONES_N_S_E;
  else if (str == "Stone::ROAD_N_STONE_W")
    return Stone::ROAD_N_STONE_W;
  else if (str == "Stone::ROAD_N_STONE_E")
    return Stone::ROAD_N_STONE_E;
  else if (str == "Stone::ROAD_N_STONE_S")
    return Stone::ROAD_N_STONE_S;
  else if (str == "Stone::ROAD_N_STONES_E_S")
    return Stone::ROAD_N_STONES_E_S;
  else if (str == "Stone::ROAD_N_STONES_W_S")
    return Stone::ROAD_N_STONES_W_S;
  else if (str == "Stone::ROAD_N_STONES_W_E")
    return Stone::ROAD_N_STONES_W_E;
  else if (str == "Stone::ROAD_N_STONES_W_E_S")
    return Stone::ROAD_N_STONES_W_E_S;
  else if (str == "Stone::ROAD_E_STONE_N")
    return Stone::ROAD_E_STONE_N;
  else if (str == "Stone::ROAD_E_STONE_S")
    return Stone::ROAD_E_STONE_S;
  else if (str == "Stone::ROAD_E_STONE_W")
    return Stone::ROAD_E_STONE_W;
  else if (str == "Stone::ROAD_E_STONES_S_W")
    return Stone::ROAD_E_STONES_S_W;
  else if (str == "Stone::ROAD_E_STONES_N_W")
    return Stone::ROAD_E_STONES_N_W;
  else if (str == "Stone::ROAD_E_STONES_N_S")
    return Stone::ROAD_E_STONES_N_S;
  else if (str == "Stone::ROAD_E_STONES_N_S_W")
    return Stone::ROAD_E_STONES_N_S_W;
  else if (str == "Stone::ROAD_S_STONE_W")
    return Stone::ROAD_S_STONE_W;
  else if (str == "Stone::ROAD_S_STONE_E")
    return Stone::ROAD_S_STONE_E;
  else if (str == "Stone::ROAD_S_STONE_N")
    return Stone::ROAD_S_STONE_N;
  else if (str == "Stone::ROAD_S_STONES_E_W")
    return Stone::ROAD_S_STONES_E_W;
  else if (str == "Stone::ROAD_S_STONES_N_W")
    return Stone::ROAD_S_STONES_N_W;
  else if (str == "Stone::ROAD_S_STONES_N_E")
    return Stone::ROAD_S_STONES_N_E;
  else if (str == "Stone::ROAD_S_STONES_N_E_W")
    return Stone::ROAD_S_STONES_N_E_W;
  return Stone::ROAD_E_AND_W_STONE_N;
}
