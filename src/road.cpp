//  Copyright (C) 2007, 2008, 2014, 2021, 2026 Ben Asselstine
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
//  Foundation, Inc., 31 Milk Street #960789, Boston, MA 02196, USA.

#include "road.h"
#include "game-map.h"
#include "xml-helper.h"

Glib::ustring Road::d_tag = "road";

Road::Road(Vector<int> pos, Type type)
  :Location(pos), d_type(type)
{
    //mark the location on the game map as occupied by a road
    GameMap::instance()->getTile(getPos())->setBuilding(Maptile::ROAD);
}

Road::Road(XML_Helper* helper)
    :Location(helper)
{
  Glib::ustring type_str;
  helper->get(type_str, "type");
  d_type = roadTypeFromString(type_str);
    
  //mark the location on the game map as occupied by a road
  GameMap::instance()->getTile(getPos())->setBuilding(Maptile::ROAD);
}

Road::Road(const Road& s, bool sync_id)
  :Location(s, sync_id), d_type(s.d_type)
{
}

Road::Road(const Road& s, Vector<int> pos)
  :Location(s, pos), d_type(s.d_type)
{
}

bool Road::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->open_tag(Road::d_tag);
    retval &= helper->save("id", d_id);
    retval &= helper->save("x", getPos().x);
    retval &= helper->save("y", getPos().y);
    Glib::ustring type_str = roadTypeToString(d_type);
    retval &= helper->save("type", type_str);
    retval &= helper->close_tag();
    
    return retval;
}

Glib::ustring Road::roadTypeToString(const Road::Type type)
{
  switch (type)
    {
    case Road::CONNECTS_EAST_AND_WEST: return "Road::CONNECTS_EAST_AND_WEST";
    case Road::CONNECTS_NORTH_AND_SOUTH: return "Road::CONNECTS_NORTH_AND_SOUTH";
    case Road::CONNECTS_ALL_DIRECTIONS: return "Road::CONNECTS_ALL_DIRECTIONS";
    case Road::CONNECTS_NORTH_AND_WEST: return "Road::CONNECTS_NORTH_AND_WEST";
    case Road::CONNECTS_NORTH_AND_EAST: return "Road::CONNECTS_NORTH_AND_EAST";
    case Road::CONNECTS_SOUTH_AND_EAST: return "Road::CONNECTS_SOUTH_AND_EAST";
    case Road::CONNECTS_WEST_AND_SOUTH: return "Road::CONNECTS_WEST_AND_SOUTH";
    case Road::CONNECTS_NORTH_AND_SOUTH_AND_EAST: return "Road::CONNECTS_NORTH_AND_SOUTH_AND_EAST";
    case Road::CONNECTS_EAST_WEST_AND_NORTH: return "Road::CONNECTS_EAST_WEST_AND_NORTH";
    case Road::CONNECTS_EAST_WEST_AND_SOUTH: return "Road::CONNECTS_EAST_WEST_AND_SOUTH";
    case Road::CONNECTS_NORTH_SOUTH_AND_WEST: return "Road::CONNECTS_NORTH_SOUTH_AND_WEST";
    case Road::CONNECTS_NORTH: return "Road::CONNECTS_NORTH";
    case Road::CONNECTS_SOUTH: return "Road::CONNECTS_SOUTH";
    case Road::CONNECTS_EAST: return "Road::CONNECTS_EAST";
    case Road::CONNECTS_WEST: return "Road::CONNECTS_WEST";
    }
  return "Road::CONNECTS_EAST_AND_WEST";
}

Road::Type Road::roadTypeFromString(const Glib::ustring str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return Road::Type(atoi(str.c_str()));
  if (str == "Road::CONNECTS_EAST_AND_WEST") return Road::CONNECTS_EAST_AND_WEST;
  else if (str == "Road::CONNECTS_NORTH_AND_SOUTH") return Road::CONNECTS_NORTH_AND_SOUTH;
  else if (str == "Road::CONNECTS_ALL_DIRECTIONS") return Road::CONNECTS_ALL_DIRECTIONS;
  else if (str == "Road::CONNECTS_NORTH_AND_WEST") return Road::CONNECTS_NORTH_AND_WEST;
  else if (str == "Road::CONNECTS_NORTH_AND_EAST") return Road::CONNECTS_NORTH_AND_EAST;
  else if (str == "Road::CONNECTS_SOUTH_AND_EAST") return Road::CONNECTS_SOUTH_AND_EAST;
  else if (str == "Road::CONNECTS_WEST_AND_SOUTH") return Road::CONNECTS_WEST_AND_SOUTH;
  else if (str == "Road::CONNECTS_NORTH_AND_SOUTH_AND_EAST") return Road::CONNECTS_NORTH_AND_SOUTH_AND_EAST;
  else if (str == "Road::CONNECTS_EAST_WEST_AND_NORTH") return Road::CONNECTS_EAST_WEST_AND_NORTH;
  else if (str == "Road::CONNECTS_EAST_WEST_AND_SOUTH") return Road::CONNECTS_EAST_WEST_AND_SOUTH;
  else if (str == "Road::CONNECTS_NORTH_SOUTH_AND_WEST") return Road::CONNECTS_NORTH_SOUTH_AND_WEST;
  else if (str == "Road::CONNECTS_NORTH") return Road::CONNECTS_NORTH;
  else if (str == "Road::CONNECTS_SOUTH") return Road::CONNECTS_SOUTH;
  else if (str == "Road::CONNECTS_EAST") return Road::CONNECTS_EAST;
  else if (str == "Road::CONNECTS_WEST") return Road::CONNECTS_WEST;
  return Road::CONNECTS_EAST_AND_WEST;
}
        
std::vector<Road::Type> Road::getTypes ()
{
  std::vector<Road::Type> types;
  types.push_back (Road::CONNECTS_EAST_AND_WEST);
  types.push_back (Road::CONNECTS_NORTH_AND_SOUTH);
  types.push_back (Road::CONNECTS_ALL_DIRECTIONS);
  types.push_back (Road::CONNECTS_NORTH_AND_WEST);
  types.push_back (Road::CONNECTS_NORTH_AND_EAST);
  types.push_back (Road::CONNECTS_SOUTH_AND_EAST);
  types.push_back (Road::CONNECTS_WEST_AND_SOUTH);
  types.push_back (Road::CONNECTS_NORTH_AND_SOUTH_AND_EAST);
  types.push_back (Road::CONNECTS_EAST_WEST_AND_NORTH);
  types.push_back (Road::CONNECTS_EAST_WEST_AND_SOUTH);
  types.push_back (Road::CONNECTS_NORTH_SOUTH_AND_WEST);
  types.push_back (Road::CONNECTS_NORTH);
  types.push_back (Road::CONNECTS_SOUTH);
  types.push_back (Road::CONNECTS_EAST);
  types.push_back (Road::CONNECTS_WEST);
  return types;
}
