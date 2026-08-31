// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2007, 2008, 2009, 2010, 2014, 2015, 2017, 2020,
// 2021 Ben Asselstine
// Copyright (C) 2008 Ole Laursen
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

#include "maptile.h"
#include <iostream>
#include "tileset.h"
#include "MapBackpack.h"
#include "stacktile.h"
#include "army.h"
#include "GameMap.h"
#include "playerlist.h"

Maptile::Maptile()
        :Movable(Vector<int>(-1,-1)), d_index(0), d_building(NONE)
{
    d_tileStyle = NULL;
    d_tilestyle_id = 0;
    d_stacktile = NULL;
    d_backpack = NULL;
    memset (d_blocked, 0, sizeof (d_blocked));
}

Maptile::Maptile(int x, int y, guint32 index)
    :Movable (Vector<int>(x, y)), d_index(index), d_building(NONE)
{
    d_tileStyle = NULL;
    d_tilestyle_id = 0;
    d_stacktile = NULL;
    d_backpack = NULL;
    memset (d_blocked, 0, sizeof (d_blocked));
}

Maptile::Maptile(int x, int y, Tile::Type type)
    : Movable (Vector<int>(x, y)), d_index(GameMap::getTileset()->lookupIndexByType (type)), d_building(NONE)
{
    d_tileStyle = NULL;
    d_tilestyle_id = 0;
    d_stacktile = NULL;
    d_backpack = NULL;
    memset (d_blocked, 0, sizeof (d_blocked));
}

Maptile::~Maptile()
{
  if (d_backpack)
    delete d_backpack;
  d_backpack = NULL;
  if (d_stacktile)
    delete d_stacktile;
  d_stacktile = NULL;
}

Gdk::RGBA Maptile::getColor() const
{
  Tileset *ts = GameMap::getTileset();
  return (*ts)[d_index]->getSmallTile()->getColor();
}

SmallTile::Pattern Maptile::getPattern() const
{
  Tileset *ts = GameMap::getTileset();
  return (*ts)[d_index]->getSmallTile()->getPattern();
}

Gdk::RGBA Maptile::getSecondColor() const
{
  Tileset *ts = GameMap::getTileset();
  return (*ts)[d_index]->getSmallTile()->getSecondColor();
}

Gdk::RGBA Maptile::getThirdColor() const
{
  Tileset *ts = GameMap::getTileset();
  return (*ts)[d_index]->getSmallTile()->getThirdColor();
}

MapBackpack *Maptile::getBackpack()
{
  if (!d_backpack)
    d_backpack = new MapBackpack (getPos(), Playerlist::getActiveplayer ());
  return d_backpack;
}

StackTile *Maptile::getStacks()
{
  if (!d_stacktile)
    d_stacktile = new StackTile (getPos());
  return d_stacktile;
}

Tile::Type Maptile::getType() const
{
  return (*GameMap::getTileset())[d_index]->getType();
}

guint32 Maptile::getMoves()
{
    if (d_building == Maptile::CITY)
        return 1;
    else if (d_building == Maptile::ROAD)
        return 1;
    else if (d_building == Maptile::BRIDGE)
        return 1;

    Tileset *ts = GameMap::getTileset ();
    if ((*ts)[d_index]->getType() == Tile::WATER)
      {
	// if we're sailing and we're not on shore, then we move faster
	if (getTileStyle (ts)->getType() == TileStyle::INNERMIDDLECENTER)
	  return (*ts)[d_index]->getMoves() / 2;
      }
    return (*ts)[d_index]->getMoves();
}

void Maptile::setIndex(guint32 index)
{
  Tileset *ts = GameMap::getTileset();
  Tile *tile = (*ts)[index];
  if (!tile)
    return;
  d_index = index;
}

bool Maptile::isCityTerrain()
{
  if (getBuilding() == Maptile::CITY || getBuilding() == Maptile::RUIN ||
      getBuilding() == Maptile::TEMPLE)
    return true;
  return false;
}

bool Maptile::isOpenTerrain()
{
  if (isCityTerrain())
    return false;
  if ((getType() == Tile::HILLS || getType() == Tile::FOREST ||
      getType() == Tile::MOUNTAIN) && getBuilding() == Maptile::NONE)
    return false;
  /* swamp and water are open terrain too */
  if ((getType() == Tile::GRASS || getType() == Tile::SWAMP ||
       getType() == Tile::WATER) || getBuilding() != Maptile::BRIDGE)
    return true;
  return false;
}

bool Maptile::isHillyTerrain()
{
  if (isCityTerrain())
    return false;
  if ((getType() == Tile::HILLS || getType() == Tile::MOUNTAIN))
    return true;
  return false;
}

bool Maptile::hasLandBuilding() const
{
  switch (d_building)
    {
    case Maptile::NONE:
      return false;
      break;
    case Maptile::CITY:
    case Maptile::RUIN:
    case Maptile::TEMPLE:
    case Maptile::SIGNPOST:
    case Maptile::ROAD:
    case Maptile::STONE:
      return true;
      break;
    case Maptile::PORT:
    case Maptile::BRIDGE:
      return false;
      break;
    }
  return false;
}

bool Maptile::hasWaterBuilding() const
{
  switch (d_building)
    {
    case Maptile::NONE:
      return false;
      break;
    case Maptile::CITY:
    case Maptile::RUIN:
    case Maptile::TEMPLE:
    case Maptile::SIGNPOST:
    case Maptile::ROAD:
    case Maptile::STONE:
      return false;
      break;
    case Maptile::PORT:
    case Maptile::BRIDGE:
      return true;
      break;
    }
  return false;
}

Maptile::Building Maptile::buildingFromString(Glib::ustring str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return Maptile::Building(atoi(str.c_str()));
  if (str == "Maptile::NONE") return Maptile::NONE;
  else if (str == "Maptile::CITY") return Maptile::CITY;
  else if (str == "Maptile::RUIN") return Maptile::RUIN;
  else if (str == "Maptile::TEMPLE") return Maptile::TEMPLE;
  else if (str == "Maptile::SIGNPOST") return Maptile::SIGNPOST;
  else if (str == "Maptile::ROAD") return Maptile::ROAD;
  else if (str == "Maptile::PORT") return Maptile::PORT;
  else if (str == "Maptile::BRIDGE") return Maptile::BRIDGE;
  else if (str == "Maptile::STONE") return Maptile::STONE;
  return Maptile::NONE;
}

Glib::ustring Maptile::buildingToString(const Maptile::Building bldg)
{
  switch (bldg)
    {
    case Maptile::NONE: return "Maptile::NONE";
    case Maptile::CITY: return "Maptile::CITY";
    case Maptile::RUIN: return "Maptile::RUIN";
    case Maptile::TEMPLE: return "Maptile::TEMPLE";
    case Maptile::SIGNPOST: return "Maptile::SIGNPOST";
    case Maptile::ROAD: return "Maptile::ROAD";
    case Maptile::PORT: return "Maptile::PORT";
    case Maptile::BRIDGE: return "Maptile::BRIDGE";
    case Maptile::STONE: return "Maptile::STONE";
    }
  return "Maptile::NONE";
}

Glib::ustring Maptile::buildingToFriendlyName(const guint32 bldg)
{
  switch (Building(bldg))
    {
    case Maptile::NONE: return _("None");
    case Maptile::CITY: return _("City");
    case Maptile::RUIN: return _("Ruin");
    case Maptile::TEMPLE: return _("Temple");
    case Maptile::SIGNPOST: return _("Signpost");
    case Maptile::ROAD: return _("Road");
    case Maptile::PORT: return _("Port");
    case Maptile::BRIDGE: return _("Bridge");
    case Maptile::STONE: return _("Stone");
    }
  return _("None");
}

Maptile::Maptile (const Maptile &m, bool sync_id)
 : Movable (m)
{
  d_index = m.d_index;
  d_building = m.d_building;
  d_tilestyle_id = m.d_tilestyle_id;
  d_tileStyle = NULL;

  if (m.d_backpack != NULL)
    d_backpack = new MapBackpack (*m.d_backpack, sync_id);
  else
    d_backpack = NULL;

  if (m.d_stacktile != NULL)
    d_stacktile = new StackTile (*m.d_stacktile);
  else
    d_stacktile = NULL;

  for (int i = 0; i < 2; i++)
    for (int j = 0; j < 8; j++)
      d_blocked[i][j] = m.d_blocked[i][j];
}

void Maptile::copy (Maptile *m, bool sync_ids)
{
  d_index = m->d_index;
  d_building = m->d_building;
  d_tilestyle_id = m->d_tilestyle_id;
  d_tileStyle = NULL;

  setPos (m->getPos ());

  if (m->d_backpack != NULL)
    {
      MapBackpack *new_backpack = new MapBackpack (*m->d_backpack, sync_ids);
      d_backpack = new_backpack;
    }
  else
    d_backpack = NULL;

  if (m->d_stacktile != NULL)
    d_stacktile = new StackTile (*m->d_stacktile);
  else
    d_stacktile = NULL;

  for (int i = 0; i < 2; i++)
    for (int j = 0; j < 8; j++)
      d_blocked[i][j] = m->d_blocked[i][j];
}

TileStyle * Maptile::getTileStyle (Tileset *tileset)
{
  if (d_tileStyle == NULL)
    d_tileStyle = tileset->getTileStyle (d_tilestyle_id);
  return d_tileStyle;
}
// End of file
