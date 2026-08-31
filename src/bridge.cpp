//  Copyright (C) 2007, 2008, 2011, 2014, 2021, 2026 Ben Asselstine
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

#include "bridge.h"
#include "game-map.h"
#include "xml-helper.h"

Glib::ustring Bridge::d_tag = "bridge";

Bridge::Bridge(Vector<int> pos, Type type)
  :Location(pos), d_type(type)
{
    //mark the location on the game map as occupied by a bridge
    GameMap::instance()->getTile(getPos())->setBuilding(Maptile::BRIDGE);
}

Bridge::Bridge(XML_Helper* helper)
    :Location(helper)
{
  Glib::ustring type_str;
  helper->get(type_str, "type");
  d_type = bridgeTypeFromString(type_str);
    
  //mark the location on the game map as occupied by a bridge
  GameMap::instance()->getTile(getPos())->setBuilding(Maptile::BRIDGE);
}

Bridge::Bridge(const Bridge& s, bool sync_id)
  :Location(s, sync_id), d_type(s.d_type)
{
}

Bridge::Bridge(const Bridge& s, Vector<int> pos)
  :Location(s, pos), d_type(s.d_type)
{
}

bool Bridge::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->open_tag(Bridge::d_tag);
    retval &= helper->save("id", d_id);
    retval &= helper->save("x", getPos().x);
    retval &= helper->save("y", getPos().y);
    Glib::ustring type_str = bridgeTypeToString(d_type);
    retval &= helper->save("type", type_str);
    retval &= helper->close_tag();
    
    return retval;
}

Glib::ustring Bridge::bridgeTypeToString(const Bridge::Type type)
{
  switch (type)
    {
    case Bridge::CONNECTS_TO_EAST: return "Bridge::CONNECTS_TO_EAST";
    case Bridge::CONNECTS_TO_NORTH: return "Bridge::CONNECTS_TO_NORTH";
    case Bridge::CONNECTS_TO_WEST: return "Bridge::CONNECTS_TO_WEST";
    case Bridge::CONNECTS_TO_SOUTH: return "Bridge::CONNECTS_TO_SOUTH";
    }
  return "Bridge::CONNECTS_TO_EAST";
}

Bridge::Type Bridge::bridgeTypeFromString(const Glib::ustring str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return Bridge::Type(atoi(str.c_str()));
  if (str == "Bridge::CONNECTS_TO_EAST") return Bridge::CONNECTS_TO_EAST;
  else if (str == "Bridge::CONNECTS_TO_NORTH") return Bridge::CONNECTS_TO_NORTH;
  else if (str == "Bridge::CONNECTS_TO_WEST") return Bridge::CONNECTS_TO_WEST;
  else if (str == "Bridge::CONNECTS_TO_SOUTH") return Bridge::CONNECTS_TO_SOUTH;
  return Bridge::CONNECTS_TO_EAST;
}

Vector<int> Bridge::getRoadEntryPoint() const
{
  switch (getType())
    {
    case Bridge::CONNECTS_TO_NORTH:
      return getPos() + Vector<int>(0, 1);
    case Bridge::CONNECTS_TO_SOUTH:
      return getPos() + Vector<int>(0, -1);
    case Bridge::CONNECTS_TO_EAST:
      return getPos() + Vector<int>(-1, 0);
    case Bridge::CONNECTS_TO_WEST:
      return getPos() + Vector<int>(1, 0);
    }
  return getPos();
}
