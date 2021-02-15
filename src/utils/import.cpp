// Copyright (C) 2015, 2020, 2021 Ben Asselstine
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

#include <config.h>

#include <iostream>
#include <algorithm>
#include "Configuration.h"
#include "File.h"
#include "vector.h"
#include "ucompose.hpp"
#include "GameScenario.h"
#include "GameMap.h"
#include "Itemlist.h"
#include "playerlist.h"
#include "counter.h"
#include "armysetlist.h"
#include "armyset.h"
#include "shieldsetlist.h"
#include "shieldset.h"
#include "ai_dummy.h"
#include "CreateScenarioRandomize.h"
#include "tileset.h"
#include "bridge.h"
#include "bridgelist.h"
#include "road.h"
#include "roadlist.h"
#include "player.h"
#include "ruinlist.h"
#include "templelist.h"
#include "temple.h"
#include "ruin.h"
#include "citylist.h"
#include "city.h"
#include "signpostlist.h"
#include "signpost.h"
#include "portlist.h"
#include "port.h"
#include "ItemProto.h"
#include "Itemlist.h"
#include "armyprodbase.h"
#include "TarFileMaskedImage.h"
#include "TarFileImage.h"

int max_vector_width;
      
static GameScenario *
setup_new_map (Glib::ustring name)
{
  GameMap::setWidth(112);
  GameMap::setHeight(156);
  GameMap::getInstance("default", "default", "default");
  Itemlist::createStandardInstance();

  GameScenario *g =
    new GameScenario (name, String::ucompose(_("a scenario called %1 converted by lordsawar-import version %2"), name, VERSION));
  for (unsigned int i = 0; i < MAX_PLAYERS + 1; i++)
    fl_counter->getNextId();

  // fill the map with tile type
  Tileset *tset = GameMap::getTileset();
  for (unsigned int i = 0; i < tset->size(); ++i)
    {
      if ((*tset)[i]->getType() == 0)
        {
          GameMap::getInstance()->fill(i);
          break;
        }
    }
  GameMap::getInstance()->calculateBlockedAvenues();
  return g;
}

static bool
convert_terrain_code (unsigned char code, guint32 *t, guint32 *ts)
{
  switch (code)
    {
    case 0x10: *t = Tile::WATER;
               *ts = TileStyle::INNERTOPLEFT;
               break;
    case 0x11: *t = Tile::WATER;
               *ts = TileStyle::OUTERBOTTOMCENTER;
               break;
    case 0x12: *t = Tile::WATER;
               *ts = TileStyle::INNERTOPRIGHT;
               break;
    case 0x13: *t = Tile::WATER;
               *ts = TileStyle::INNERBOTTOMLEFT;
               break;
    case 0x14: *t = Tile::WATER;
               *ts = TileStyle::OUTERTOPCENTER;
               break;
    case 0x15: *t = Tile::WATER;
               *ts = TileStyle::INNERBOTTOMRIGHT;
               break;
    case 0x16: *t = Tile::WATER;
               *ts = TileStyle::OUTERMIDDLELEFT;
               break;
    case 0x17: *t = Tile::WATER;
               *ts = TileStyle::OTHER;
               break;
    case 0x18: *t = Tile::WATER;
               *ts = TileStyle::OUTERMIDDLERIGHT;
               break;
    case 0x19: *t = Tile::WATER;
               *ts = TileStyle::OUTERTOPLEFT;
               break;
    case 0x1a: *t = Tile::WATER;
               *ts = TileStyle::OUTERTOPRIGHT;
               break;
    case 0x1b: *t = Tile::WATER;
               *ts = TileStyle::OUTERBOTTOMLEFT;
               break;
    case 0x1c: *t = Tile::WATER;
               *ts = TileStyle::OUTERBOTTOMRIGHT;
               break;
    case 0x20: *t = Tile::WATER;
               *ts = TileStyle::INNERTOPLEFT;
               break;
    case 0x21: *t = Tile::WATER;
               *ts = TileStyle::OUTERBOTTOMCENTER;
               break;
    case 0x22: *t = Tile::WATER;
               *ts = TileStyle::INNERTOPRIGHT;
               break;
    case 0x23: *t = Tile::WATER;
               *ts = TileStyle::INNERBOTTOMLEFT;
               break;
    case 0x24: *t = Tile::WATER;
               *ts = TileStyle::OUTERTOPCENTER;
               break;
    case 0x25: *t = Tile::WATER;
               *ts = TileStyle::INNERBOTTOMRIGHT;
               break;
    case 0x26: *t = Tile::WATER;
               *ts = TileStyle::OUTERMIDDLELEFT;
               break;
    case 0x27: *t = Tile::WATER;
               *ts = TileStyle::INNERMIDDLECENTER;
               break;
    case 0x28: *t = Tile::WATER;
               *ts = TileStyle::OUTERMIDDLERIGHT;
               break;
    case 0x29: *t = Tile::WATER;
               *ts = TileStyle::OUTERTOPLEFT;
               break;
    case 0x2a: *t = Tile::WATER;
               *ts = TileStyle::OUTERTOPRIGHT;
               break;
    case 0x2b: *t = Tile::WATER;
               *ts = TileStyle::OUTERBOTTOMLEFT;
               break;
    case 0x2c: *t = Tile::WATER;
               *ts = TileStyle::OUTERBOTTOMRIGHT;
               break;
    case 0x2d: *t = Tile::WATER;
               *ts = TileStyle::INNERMIDDLECENTER;
               break;
    case 0x2f: *t = Tile::WATER;
               *ts = TileStyle::TOPLEFTTOBOTTOMRIGHTDIAGONAL;
               break;
    case 0x3f: *t = Tile::WATER;
               *ts = TileStyle::BOTTOMLEFTTOTOPRIGHTDIAGONAL;
               break;
      //grass
    case 0x00: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x01: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x02: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x03: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x04: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x05: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x06: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x07: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x08: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x09: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x0a: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x0b: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x0c: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x1f: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x60: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x61: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x70: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
    case 0x71: *t = Tile::GRASS;
               *ts = TileStyle::LONE;
               break;
      //forest
    case 0x30: *t = Tile::FOREST;
               *ts = TileStyle::OUTERTOPLEFT;
               break;
    case 0x31: *t = Tile::FOREST;
               *ts = TileStyle::OUTERTOPCENTER;
               break;
    case 0x32: *t = Tile::FOREST;
               *ts = TileStyle::OUTERTOPRIGHT;
               break;
    case 0x33: *t = Tile::FOREST;
               *ts = TileStyle::OUTERBOTTOMLEFT;
               break;
    case 0x34: *t = Tile::FOREST;
               *ts = TileStyle::OUTERBOTTOMCENTER;
               break;
    case 0x35: *t = Tile::FOREST;
               *ts = TileStyle::OUTERBOTTOMRIGHT;
               break;
    case 0x36: *t = Tile::FOREST;
               *ts = TileStyle::OUTERMIDDLELEFT;
               break;
    case 0x37: *t = Tile::FOREST;
               *ts = TileStyle::INNERMIDDLECENTER;
               break;
    case 0x38: *t = Tile::FOREST;
               *ts = TileStyle::OUTERMIDDLERIGHT;
               break;
    case 0x39: *t = Tile::FOREST;
               *ts = TileStyle::INNERTOPLEFT;
               break;
    case 0x3a: *t = Tile::FOREST;
               *ts = TileStyle::INNERMIDDLECENTER;
               break;
    case 0x3b: *t = Tile::FOREST;
               *ts = TileStyle::INNERBOTTOMLEFT;
               break;
    case 0x3c: *t = Tile::FOREST;
               *ts = TileStyle::BOTTOMLEFTTOTOPRIGHTDIAGONAL;
               *ts = TileStyle::INNERBOTTOMRIGHT;
               break;
    case 0x3d: *t = Tile::FOREST;
               *ts = TileStyle::LONE;
               break;
    case 0x3e: *t = Tile::FOREST;
               *ts = TileStyle::LONE;
               break;
    case 0x4e: *t = Tile::FOREST;
               *ts = TileStyle::INNERBOTTOMLEFT;
               break;
    case 0x87: *t = Tile::FOREST;
               *ts = TileStyle::OUTERBOTTOMLEFT;
               break;
    case 0x88: *t = Tile::FOREST;
               *ts = TileStyle::OUTERBOTTOMCENTER;
               break;
    case 0x89: *t = Tile::FOREST;
               *ts = TileStyle::OUTERBOTTOMRIGHT;
               break;
    case 0x8a: *t = Tile::FOREST;
               *ts = TileStyle::OUTERTOPLEFT;
               break;
    case 0x8b: *t = Tile::FOREST;
               *ts = TileStyle::OUTERTOPCENTER;
               break;
    case 0x8c: *t = Tile::FOREST;
               *ts = TileStyle::OUTERTOPRIGHT;
               break;
    case 0x8d: *t = Tile::FOREST;
               *ts = TileStyle::OUTERMIDDLELEFT;
               break;
    case 0x8e: *t = Tile::FOREST;
               *ts = TileStyle::INNERMIDDLECENTER;
               break;
    case 0x8f: *t = Tile::FOREST;
               *ts = TileStyle::OUTERMIDDLERIGHT;
               break;
    case 0x95: *t = Tile::FOREST;
               *ts = TileStyle::INNERTOPLEFT;
               break;
    case 0x96: *t = Tile::FOREST;
               *ts = TileStyle::INNERTOPRIGHT;
               break;
    case 0x97: *t = Tile::FOREST;
               *ts = TileStyle::INNERBOTTOMLEFT;
               break;
    case 0x98: *t = Tile::FOREST;
               *ts = TileStyle::INNERBOTTOMRIGHT;
               break;
    case 0x4f: *t = Tile::FOREST;
               *ts = TileStyle::BOTTOMLEFTTOTOPRIGHTDIAGONAL;
               break;
      //hills
    case 0x40: *t = Tile::HILLS;
               *ts = TileStyle::OUTERTOPLEFT;
               break;
    case 0x41: *t = Tile::HILLS;
               *ts = TileStyle::OUTERTOPCENTER;
               break;
    case 0x42: *t = Tile::HILLS;
               *ts = TileStyle::OUTERTOPRIGHT;
               break;
    case 0x43: *t = Tile::HILLS;
               *ts = TileStyle::OUTERBOTTOMLEFT;
               break;
    case 0x44: *t = Tile::HILLS;
               *ts = TileStyle::OUTERBOTTOMCENTER;
               break;
    case 0x45: *t = Tile::HILLS;
               *ts = TileStyle::OUTERBOTTOMRIGHT;
               break;
    case 0x46: *t = Tile::HILLS;
               *ts = TileStyle::OUTERMIDDLELEFT;
               break;
    case 0x47: *t = Tile::HILLS;
               *ts = TileStyle::INNERMIDDLECENTER;
               break;
    case 0x48: *t = Tile::HILLS;
               *ts = TileStyle::OUTERMIDDLERIGHT;
               break;
    case 0x49: *t = Tile::HILLS;
               *ts = TileStyle::INNERTOPLEFT;
               break;
    case 0x4a: *t = Tile::HILLS;
               *ts = TileStyle::INNERTOPRIGHT;
               break;
    case 0x4b: *t = Tile::HILLS;
               *ts = TileStyle::INNERBOTTOMLEFT;
               break;
    case 0x4c: *t = Tile::HILLS;
               *ts = TileStyle::INNERBOTTOMRIGHT;
               break;
    case 0x4d: *t = Tile::HILLS;
               *ts = TileStyle::LONE;
               break;
      //mountains
    case 0x50: *t = Tile::MOUNTAIN;
               *ts = TileStyle::OUTERTOPLEFT;
               break;
    case 0x51: *t = Tile::MOUNTAIN;
               *ts = TileStyle::OUTERTOPCENTER;
               break;
    case 0x52: *t = Tile::MOUNTAIN;
               *ts = TileStyle::OUTERTOPRIGHT;
               break;
    case 0x53: *t = Tile::MOUNTAIN;
               *ts = TileStyle::OUTERBOTTOMLEFT;
               break;
    case 0x54: *t = Tile::MOUNTAIN;
               *ts = TileStyle::OUTERBOTTOMCENTER;
               break;
    case 0x55: *t = Tile::MOUNTAIN;
               *ts = TileStyle::OUTERBOTTOMRIGHT;
               break;
    case 0x56: *t = Tile::MOUNTAIN;
               *ts = TileStyle::OUTERMIDDLELEFT;
               break;
    case 0x57: *t = Tile::MOUNTAIN;
               *ts = TileStyle::INNERMIDDLECENTER; //really a special
               break;
    case 0x58: *t = Tile::MOUNTAIN;
               *ts = TileStyle::OUTERMIDDLERIGHT;
               break;
    case 0x59: *t = Tile::MOUNTAIN;
               *ts = TileStyle::INNERMIDDLECENTER;
               break;
    case 0x5a: *t = Tile::MOUNTAIN;
               *ts = TileStyle::INNERTOPRIGHT;
               break;
    case 0x5b: *t = Tile::MOUNTAIN;
               *ts = TileStyle::INNERBOTTOMLEFT;
               break;
    case 0x5c: *t = Tile::MOUNTAIN;
               *ts = TileStyle::INNERBOTTOMRIGHT;
               break;
    case 0x5d: *t = Tile::MOUNTAIN;
               *ts = TileStyle::INNERMIDDLECENTER;
               break;
    case 0x5e: *t = Tile::MOUNTAIN;
               *ts = TileStyle::INNERMIDDLECENTER;
               break;
    case 0x5f: *t = Tile::MOUNTAIN;
               *ts = TileStyle::INNERMIDDLECENTER;
               break;
    case 0x7e: *t = Tile::MOUNTAIN;
               *ts = TileStyle::OUTERBOTTOMCENTER;
               break;
    case 0x6e: *t = Tile::MOUNTAIN;
               *ts = TileStyle::OUTERTOPCENTER;
               break;
      //swamp
    case 0x0d: *t = Tile::SWAMP;
               *ts = TileStyle::LONE;
               break;
    case 0x0e: *t = Tile::SWAMP;
               *ts = TileStyle::LONE;
               break;
    case 0x0f: *t = Tile::SWAMP;
               *ts = TileStyle::LONE;
               break;
    case 0x1d: *t = Tile::SWAMP;
               *ts = TileStyle::LONE;
               break;
    case 0x1e: *t = Tile::SWAMP;
               *ts = TileStyle::LONE;
               break;
    case 0x2e: *t = Tile::SWAMP;
               *ts = TileStyle::LONE;
               break;
      //bridges
    case 0x85: *t = Tile::WATER;
               *ts = TileStyle::OUTERMIDDLELEFT;
               break;
    case 0x86: *t = Tile::WATER;
               *ts = TileStyle::OUTERMIDDLERIGHT;
               break;
    case 0x94: *t = Tile::WATER;
               *ts = TileStyle::OUTERBOTTOMCENTER;
               break;
    case 0x84: *t = Tile::WATER;
               *ts = TileStyle::OUTERTOPCENTER;
               break;
    default:
      return false;
      break;
    }
  return true;
}

static void
import_terrain (FILE *map)
{
  short tiles[156][112];
  fread (tiles, sizeof (short), 0x4440, map);
  int i, j;
  for (i = 0; i < 156; i++)
    {
      for (j = 0; j < 112; j++)
        {
          short tile = tiles[i][j];
          unsigned char code;
          memcpy (&code, &tile, 1);
          guint32 type = 0;
          guint32 tilestyle = 0;
          bool success = convert_terrain_code (code, &type, &tilestyle);
          if (!success)
            {
              fprintf(stderr, 
                      _("Error: Terrain code %02hhx at %d,%d is unknown\n").c_str(), 
                      code, j, i);
            }
          else if (tilestyle == TileStyle::UNKNOWN)
            fprintf(stderr, 
                    _("Error: Terrain code %02hhx at %d,%d "
                      "is unknown type %s\n").c_str(), 
                    code, j, i, Tile::tileTypeToString(Tile::Type(type)).c_str());
          if (success)
            {
              Tileset *tileset = GameMap::getTileset();
              TileStyle *style = 
                tileset->getRandomTileStyle(tileset->getIndex(Tile::Type(type)), TileStyle::Type(tilestyle));

              Maptile *mtile = GameMap::getInstance()->getTile(Vector<int>(j,i));
              mtile->setIndex(tileset->getIndex(Tile::Type(type)));
              mtile->setTileStyleId(style->getId ());
            }

          char port;
          memcpy (&port, (((char *)&tile)+1), 1);
          if (port == (char)0x80)
            {
              Port *p = new Port (Vector<int>(j, i));
              Portlist::getInstance()->add(p);
            }
        }
    }
}

static bool
convert_bridge_code (unsigned char code, guint32 *type)
{
  switch (code)
    {
    case 0x85: *type = Bridge::CONNECTS_TO_EAST;
               break;
    case 0x86: *type = Bridge::CONNECTS_TO_WEST;
               break;
    case 0x94: *type = Bridge::CONNECTS_TO_NORTH;
               break;
    case 0x84: *type = Bridge::CONNECTS_TO_SOUTH;
               break;
    default:
               return false;
    }
  return true;
}

static void
import_bridges (FILE *map)
{
  short tiles[156][112];
  fread (tiles, sizeof (short), 0x4400, map);
  int i, j;
  for (i = 0; i < 156; i++)
    {
      for (j = 0; j < 112; j++)
        {
          short tile = tiles[i][j];
          unsigned char code;
          memcpy (&code, &tile, 1);
          guint32 type = 0;
          if (convert_bridge_code(code, &type))
            Bridgelist::getInstance()->add(new Bridge(Vector<int>(j, i), type));
        }
    }
}

static bool
convert_road_code (unsigned char code, guint32 *type)
{
  switch (code)
    {
    case 0x01: *type = Road::CONNECTS_EAST_AND_WEST;
               break;
    case 0x02: *type = Road::CONNECTS_NORTH_AND_SOUTH;
               break;
    case 0x03: *type = Road::CONNECTS_ALL_DIRECTIONS;
               break;
    case 0x04: *type = Road::CONNECTS_EAST_WEST_AND_SOUTH;
               break;
    case 0x05: *type = Road::CONNECTS_NORTH_SOUTH_AND_WEST;
               break;
    case 0x06: *type = Road::CONNECTS_EAST_WEST_AND_NORTH;
               break;
    case 0x07: *type = Road::CONNECTS_NORTH_AND_SOUTH_AND_EAST;
               break;
    case 0x08: *type = Road::CONNECTS_WEST_AND_SOUTH;
               break;
    case 0x09: *type = Road::CONNECTS_NORTH_AND_WEST;
               break;
    case 0x0a: *type = Road::CONNECTS_NORTH_AND_EAST;
               break;
    case 0x0b: *type = Road::CONNECTS_SOUTH_AND_EAST;
               break;
    case 0x0c: *type = Road::CONNECTS_WEST;
               break;
    case 0x0d: *type = Road::CONNECTS_SOUTH;
               break;
    case 0x0e: *type = Road::CONNECTS_EAST;
               break;
    case 0x0f: *type = Road::CONNECTS_NORTH;
               break;
    case 0x10: *type = Road::CONNECTS_EAST_AND_WEST;
               break;
    case 0x11: *type = Road::CONNECTS_NORTH_AND_SOUTH;
               break;
    default:
               return false;
    }
  return true;
}

static void
import_roads(FILE *rd)
{
  unsigned char roads[156][112];
  fread (roads, sizeof (unsigned char), 0x4440, rd);
  int i, j;
  for (i = 0; i < 156; i++)
    {
      for (j = 0; j < 112; j++)
        {
          unsigned char road = roads[i][j];
          guint32 type = 0;
          if (convert_road_code(road, &type))
            Roadlist::getInstance()->add(new Road(Vector<int>(j, i), type));
        }
    }
}

static int
convert_player_id (int id)
{
  int new_id = -1;
  switch (id)
    {
    case 0: new_id = Shield::WHITE; break;
    case 1: new_id = Shield::YELLOW; break;
    case 2: new_id = Shield::ORANGE; break;
    case 3: new_id = Shield::RED; break;
    case 4: new_id = Shield::GREEN; break;
    case 5: new_id = Shield::DARK_BLUE; break;
    case 6: new_id = Shield::LIGHT_BLUE; break;
    case 7: new_id = Shield::BLACK; break;
    default:
      break;
    }
  return new_id;
}

static void
import_players (FILE *scn, Armyset *armyset)
{
  CreateScenarioRandomize* d_random = new CreateScenarioRandomize();
  char names[8][21];
  memset (names, 0, sizeof (names));
  int i;
  for (i = 0; i < 8; i++)
    {
      fread (&names[i][0], sizeof(char), 20, scn);
      if (strcmp (names[i], "Not Used") == 0)
        continue;
  std::cout << String::ucompose (_("Importing player %1."), Glib::ustring(names[i])) << std::endl;
      Player *player = 
        new RealPlayer(Glib::ustring(names[i]),
                       armyset->getId(),
                       Shield::get_default_colors_for_no(convert_player_id(i)),
                       GameMap::getWidth(), GameMap::getHeight(), Player::HUMAN,
                       convert_player_id(i));
      int gold = 0;
      d_random->getBaseGold(100, &gold);
      gold = d_random->adjustBaseGold(gold);
      player->setGold(gold);
      Playerlist::getInstance()->add(player);
    }
  Glib::ustring neutral_name = d_random->getPlayerName(Shield::NEUTRAL);
  Player* neutral = 
    new AI_Dummy(neutral_name, armyset->getId(),
                 Shield::get_default_colors_for_neutral(), 
                 GameMap::getWidth(), GameMap::getHeight(), MAX_PLAYERS);
  Playerlist::getInstance()->add(neutral);
  Playerlist::getInstance()->setNeutral(neutral);
  delete d_random;
}

static void 
import_ruins_and_temples (FILE *scn, FILE *spc)
{
  fseek (scn, 0x80f, SEEK_CUR);
  unsigned short num_ruins = 0;
  fread (&num_ruins, sizeof (unsigned short), 1, scn);
  std::cout << String::ucompose (_("Importing %1 ruins & temples."), num_ruins) << std::endl;
  unsigned int i;
  for (i = 0; i < num_ruins; i++)
    {
      short x, y;
      unsigned short type;
      char name[21];
      char unused[5];
      fread (&x, sizeof (short), 1, scn);
      fread (&y, sizeof (short), 1, scn);
      memset (name, 0, sizeof (name));
      fread (name, sizeof (char), 20, scn);
      fread (&type, sizeof (unsigned short), 1, scn);
      fread (&unused, sizeof (char), 5, scn);
      fread (&unused, sizeof (char), 5, spc);
      char line[256];
      fgets (line, sizeof (line), spc);
      std::string m(line);
      std::replace (m.begin(), m.end(), '|', ' ');
      std::replace (m.begin(), m.end(), '\r', '\0');
      if (type == 1)
        {
          Temple *t = new Temple (Vector<int>(x,y), 1, Glib::ustring(name));
          t->setDescription(m);
          Templelist::getInstance()->add(t);
        }
      else if (type == 2)
        {
          Ruin *r = new Ruin (Vector<int>(x,y), 1, Glib::ustring(name));
          r->setDescription(m);
          Ruinlist::getInstance()->add(r);
        }
      else
        std::cerr << String::ucompose(_("Error: We got an unknown temple/ruin type of %1 for %2 at %3,%4"), type, Glib::ustring(name),x,y) << std::endl;
    }
}

int
compare_army_strengths(const void *lhs, const void *rhs)
{
  guint as = Playerlist::getInstance()->getNeutral()->getId();
  ArmyProto *left = Armysetlist::getInstance()->getArmy(as, *((char*)lhs));
  ArmyProto *right = Armysetlist::getInstance()->getArmy(as, *((char*)rhs));
  guint32 left_str = 9999;
  if (left)
    left->getStrength();
  guint32 right_str = 9999;
  if (right)
    right->getStrength();
  return left_str < right_str;
}

static void 
import_cities (FILE *scn)
{
  fseek (scn, 0x157b, SEEK_CUR);
  unsigned short num_cities = 0;
  fread (&num_cities, sizeof (unsigned short), 1, scn);
  std::cout << String::ucompose (_("Importing %1 cities."), num_cities) << std::endl;
  for (unsigned int i = 0; i < num_cities; i++)
    {
      short x, y;
      char name[17];
      char unused1[2];
      char armies[4];
      char unused2[16];
      short income;
      char unused3[21];
      fread (&x, sizeof (short), 1, scn);
      fread (&y, sizeof (short), 1, scn);
      memset (name, 0, sizeof (name));
      fread (name, sizeof (char), 16, scn);
      fread (&unused1, sizeof (char), 2, scn);
      fread (armies, sizeof (char), 4, scn);
      fread (&unused2, sizeof (char), 16, scn);
      fread (&income, sizeof (short), 1, scn);
      fread (&unused3, sizeof (char), 21, scn);
      City *city = new City (Vector<int>(x,y), 2, Glib::ustring(name), income);

      guint32 as = Playerlist::getInstance()->getNeutral()->getArmyset();
      if (as != 1)
        {
          // only bring in the armies if we also have an armyset.
          qsort (armies, 4, sizeof (char), compare_army_strengths);
          for (int j = 0; j < 4; j++)
            {
              if (armies[j] != (char) 0xff)
                {
                  ArmyProto *army = 
                    Armysetlist::getInstance()->getArmy(as, armies[j]);
                  if (army)
                    {
                      ArmyProdBase *prodbase = new ArmyProdBase(*army);
                      prodbase->setArmyset(as);
                      city->addProductionBase(j, prodbase);
                    }
                }
            }
        }
      city->setOwner(Playerlist::getInstance()->getNeutral());
      Citylist::getInstance()->add(city);
    }
}

static void
set_capital_cities (FILE *scn)
{
  fseek (scn, 0x189, SEEK_CUR);
  for (int i = 0; i < 8; i++)
    {
      short x, y;
      char unused[16];
      fread (&x, sizeof (x), 1, scn);
      fread (&y, sizeof (y), 1, scn);
      fread (unused, sizeof (char), 16, scn);

      Player *player = Playerlist::getInstance()->getPlayer(convert_player_id(i));
      if (player == NULL)
        continue;
      City *c = GameMap::getCity(Vector<int>(x,y));
      c->setOwner(player);
      c->setCapital(true);
      c->setCapitalOwner(player);
    }
}

static void
import_signposts (FILE *sg)
{
  unsigned short num_signs = 0;
  fread (&num_signs, sizeof (unsigned short), 1, sg);
  std::cout << String::ucompose (_("Importing %1 signposts."), num_signs) << std::endl;
  for (unsigned int i = 0; i < num_signs; i++)
    {
      short x, y;
      char line1[51];
      char line2[51];
      fread (&x, sizeof (short), 1, sg);
      fread (&y, sizeof (short), 1, sg);
      memset (line1, 0, sizeof (line1));
      fread (line1, sizeof (char), 50, sg);
      memset (line2, 0, sizeof (line2));
      fread (line2, sizeof (char), 50, sg);
      Signpost *s = 
        new Signpost (Vector<int>(x, y), 
                      Glib::ustring(line1) + "\n" + Glib::ustring(line2));
      Signpostlist::getInstance()->add(s);
    }
}

static long 
get_offset (Glib::ustring filename, Glib::ustring ext, bool prepend_basename)
{
  Glib::ustring name;
  if (prepend_basename)
    name = File::get_basename(filename) + ext;
  else
    name = ext;
  std::ifstream ifs(filename.c_str(), std::ios::binary);
  std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
  size_t pos = str.find(name);
  if (pos == std::string::npos)
    return 0;
  if (prepend_basename)
    return long(pos) + 18;

  if (ext[0] != '.')
    return long(pos) + 18;

  //we don't know where we matched in the filename.
  FILE *f = fopen (filename.c_str(), "rb");
  fseek (f, pos-9, SEEK_SET);
  char bytes[10];
  fread (bytes, 10, sizeof (char), f);
  for (int i = 0; i < 10; i++)
    {
      if (isalpha (bytes[i]))
        {
          pos = pos -9 + i;
          break;
        }
    }
  fclose (f);
  return long(pos) + 18;
}

static bool 
convert_item_code (int code, int value, ItemProto::Bonus &bonus)
{
  switch (code)
    {
    case 1: //battle
      switch (value)
        {
        case 1:
          bonus = ItemProto::ADD1STR; break;
        case 2:
          bonus = ItemProto::ADD2STR; break;
        case 3:
          bonus = ItemProto::ADD3STR; break;
        default:
          return false;
        }
      break;
    case 2: //command
      switch (value)
        {
        case 1:
          bonus = ItemProto::ADD1STACK; break;
        case 2:
          bonus = ItemProto::ADD2STACK; break;
        case 3:
          bonus = ItemProto::ADD3STACK; break;
        default:
          return false;
        }
      break;
    case 5:
      bonus = ItemProto::FLYSTACK;
      break;
    case 6:
      bonus = ItemProto::DOUBLEMOVESTACK;
      break;
    case 7:
      switch (value)
        {
        case 1:
          // whoopsie, better than nothing
          bonus = ItemProto::ADD2GOLDPERCITY; break;
        case 2:
          bonus = ItemProto::ADD2GOLDPERCITY; break;
        case 3:
          bonus = ItemProto::ADD3GOLDPERCITY; break;
        case 4:
          bonus = ItemProto::ADD4GOLDPERCITY; break;
        case 5:
          bonus = ItemProto::ADD5GOLDPERCITY; break;
        case 6:
          bonus = ItemProto::Bonus(ItemProto::ADD4GOLDPERCITY |
                                   ItemProto::ADD2GOLDPERCITY);
          break;
        default:
          return false;
        }
      break;
    default:
      return false;
    }
  return true;
}

static void
import_items (FILE *it)
{
  Itemlist::create();
  int num_items;
  fscanf (it, "%d", &num_items);
  std::cout << String::ucompose (_("Importing %1 items."), num_items) << std::endl;
  for (int i = 0; i < num_items; i++)
    {
      char name[21];
      int code, value;
      memset (name, 0, sizeof (name));
      fscanf (it, "%20s %d %d", name, &code, &value);
      std::string item_name = std::string(name);
      std::replace (item_name.begin(), item_name.end(), ' ', '\0');
      std::replace (item_name.begin(), item_name.end(), '_', ' ');
      ItemProto::Bonus bonus;
      if (convert_item_code (code, value, bonus))
        {
          ItemProto *item = new ItemProto(Glib::ustring(item_name));
          item->addBonus(bonus);
          Itemlist::getInstance()->add(item);
        }
      else
        std::cerr << String::ucompose(_("Error: couldn't convert item number %1"), i) << std::endl;
    }
}

struct army_t
{
  unsigned short idx;
  char name[20];
  unsigned short strength;
  unsigned short ptime;
  unsigned short cost;
  unsigned short moves;
  unsigned short newcost;
  unsigned short cityplus;
  unsigned short plainsplus;
  unsigned short woodsplus;
  unsigned short hillsplus;
  unsigned short allplus;
  unsigned short allplus2;
  unsigned short allplus3;
  unsigned short allplus4;
  unsigned short ally;
  unsigned short enemyminus;
  unsigned short cancel;
  unsigned short flight;
  unsigned short forest;
  unsigned short hills;
  unsigned short boat;
};

static int
compare_army_records(const void *lhs, const void *rhs)
{
  struct army_t *l = (struct army_t*) lhs;
  struct army_t *r = (struct army_t*) rhs;
  return l->idx > r->idx;
}

static void
copy_armyset_images (Armyset *armyset, Armyset *default_armyset, ArmyProto *army, ArmyProto *default_army)
{
  for (int i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
    {
      Glib::ustring f =
        default_armyset->getFileFromConfigurationFile(default_army->getMaskedImage(Shield::Color(i))->getName ());
      Glib::ustring outfile = "";
      armyset->addFileInCfgFile(f, outfile);
      army->getMaskedImage(Shield::Color(i))->setName (outfile);
    }
}

static bool
compare_army_strength (ArmyProto *lhs, ArmyProto *rhs)
{
  return lhs > rhs;
}

static void
set_ruin_defenders(Armyset *armyset)
{
  // we do ruins differently than wl2.
  // they have a stock set of ruin defenders, and we pick them from our
  // army set.
 
  std::list<ArmyProto*> armies;
  for (Armyset::iterator i = armyset->begin(); i != armyset->end(); ++i)
    armies.push_back(*i);

  armies.sort(compare_army_strength);
  //take the top 4 in terms of strength.  but hey, no heroes.
  int count = 0;
  for (std::list<ArmyProto*>::iterator j = armies.begin(); j != armies.end();
       ++j)
    {
      if ((*j)->getName().uppercase() == "HERO")
        continue;
      (*j)->setDefendsRuins(true);
      count++;
      if (count > 3)
        break;
    }
}

static bool
sort_by_index (ArmyProto *lhs, ArmyProto *rhs)
{
  return lhs->getId() < rhs->getId();
}

static void
copy_other_armyset_images (Armyset *default_armyset, Armyset *armyset)
{
  Glib::ustring f =
    default_armyset->getFileFromConfigurationFile(default_armyset->getBag()->getName());
  Glib::ustring outfile = "";
  armyset->addFileInCfgFile(f, outfile);
  armyset->getBag ()->setName(outfile);

  f = default_armyset->getFileFromConfigurationFile(default_armyset->getShip()->getName ());
  outfile = "";
  armyset->addFileInCfgFile(f, outfile);
  armyset->getShip()->setName(outfile);

  f = default_armyset->getFileFromConfigurationFile(default_armyset->getStandard()->getName());
  outfile = "";
  armyset->addFileInCfgFile(f, outfile);
  armyset->getStandard()->setName(outfile);
}

static Armyset* 
import_armyset (FILE *a, Glib::ustring name)
{
  std::cout << String::ucompose(_("Importing armyset %1."), name) << std::endl;
  Armyset * default_armyset = Armysetlist::getInstance()->get(1);
  Armyset *armyset = new Armyset(Armysetlist::getNextAvailableId(1), name);
  struct army_t armies[29];
  fread (armies, sizeof (struct army_t), 29, a);
  qsort (armies, 29, sizeof (struct army_t), compare_army_records);

  armyset->setDirectory(File::getCacheDir());
  armyset->setBaseName(name);
  armyset->setInfo(String::ucompose(_("An armyset called %1 converted by lordsawar-import %2."), name, VERSION));
  armyset->setTileSize(40);
  //gotta save now so the copying of images works later on.
  File::erase(File::getTempFile (File::getCacheDir(), name + ARMYSET_EXT));
  armyset->save(File::getTempFile (File::getCacheDir(), name), ARMYSET_EXT);
  for (int i = 0; i < 29; i++)
    {
      struct army_t ar = armies[i];
      if (ar.boat)
        continue;
      ArmyProto *army = new ArmyProto();
      army->setName(ar.name);
      army->setId(ar.idx);
      army->setProduction(ar.ptime);
      army->setStrength(ar.strength);
      army->setProductionCost(ar.cost);
      army->setUpkeep(ar.cost/2);
      army->setMaxMoves(ar.moves);
      if (ar.newcost < (unsigned short)1000000)
        army->setNewProductionCost(ar.newcost);
      guint32 move_bonus = Tile::NONE;
      if (ar.hills)
        move_bonus |= Tile::HILLS;
      if (ar.forest)
        move_bonus |= Tile::FOREST;
      if (ar.flight)
        move_bonus |= Tile::WATER | Tile::FOREST | Tile::HILLS | 
          Tile::MOUNTAIN | Tile::SWAMP;
      army->setMoveBonus(move_bonus);

      int army_bonus = 0;
      switch (ar.cityplus)
        {
        case 0: break;
        case 1: army_bonus |= ArmyBase::ADD1STRINCITY; break;
        case 2: army_bonus |= ArmyBase::ADD2STRINCITY; break;
        case 3: army_bonus |= ArmyBase::ADD1STRINCITY | ArmyBase::ADD2STRINCITY; break;
        default:
          std::cerr << String::ucompose(_("Warning: unrecognized city bonus of %1 for %2"), ar.cityplus, ar.name) << std::endl;
          army_bonus |= ArmyBase::ADD1STRINCITY;
          break;
        }
      switch (ar.plainsplus)
        {
        case 0: break;
        case 1: army_bonus |= ArmyBase::ADD1STRINOPEN; break;
        case 2: army_bonus |= ArmyBase::ADD2STRINOPEN; break;
        case 3: army_bonus |= ArmyBase::ADD1STRINOPEN | ArmyBase::ADD2STRINOPEN; break;
        default:
          std::cerr << String::ucompose(_("Warning: unrecognized open bonus of %1 for %2"), ar.plainsplus, ar.name) << std::endl;
          army_bonus |= ArmyBase::ADD1STRINOPEN;
          break;
        }
      switch (ar.woodsplus)
        {
        case 0: break;
        case 1: army_bonus |= ArmyBase::ADD1STRINFOREST; break;
        case 2: army_bonus |= ArmyBase::ADD2STRINFOREST; break;
        case 3: army_bonus |= ArmyBase::ADD1STRINFOREST | ArmyBase::ADD2STRINFOREST; break;
        default:
          std::cerr << String::ucompose(_("Warning: unrecognized forest bonus of %1 for %2"), ar.woodsplus, ar.name) << std::endl;
          army_bonus |= ArmyBase::ADD1STRINFOREST;
          break;
        }
      switch (ar.hillsplus)
        {
        case 0: break;
        case 1: army_bonus |= ArmyBase::ADD1STRINHILLS; break;
        case 2: army_bonus |= ArmyBase::ADD2STRINHILLS; break;
        case 3: army_bonus |= ArmyBase::ADD1STRINHILLS | ArmyBase::ADD2STRINHILLS; break;
        default:
          std::cerr << String::ucompose(_("Warning: unrecognized hills bonus of %1 for %2"), ar.hillsplus, ar.name) << std::endl;
          army_bonus |= ArmyBase::ADD1STRINHILLS;
          break;
        }
      if (ar.enemyminus)
        {
          int minus = ar.enemyminus ? 65536-ar.enemyminus : 0;
          switch (minus)
            {
            case 0: break;
            case 1: army_bonus |= ArmyBase::SUB1ENEMYSTACK; break;
            case 2: army_bonus |= ArmyBase::SUB2ENEMYSTACK; break;
            case 3: army_bonus |= ArmyBase::SUB1ENEMYSTACK | ArmyBase::SUB2ENEMYSTACK; break;
            default:
              std::cerr << String::ucompose(_("Warning: unrecognized enemy minus bonus of %1 for %2"), minus, ar.name) << std::endl;
              break;
            }
        }
      switch (ar.cancel)
        {
        case 0: break;
        case 1: army_bonus |= ArmyBase::SUBALLCITYBONUS; break;
        case 2: army_bonus |= ArmyBase::SUBALLHEROBONUS; break;
        case 3: army_bonus |= ArmyBase::SUBALLNONHEROBONUS; break;
        case 4: army_bonus |= ArmyBase::FORTIFY; break;
        default:
          std::cerr << String::ucompose(_("Warning: unrecognized cancel bonus of %1 for %2"), ar.cancel, ar.name) << std::endl;
          break;
        }
      int allplus = ar.allplus | ar.allplus2 | ar.allplus3 | ar.allplus4;
      switch (allplus)
        {
        case 0: break;
        case 1: army_bonus |= ArmyBase::ADD1STACK; break;
        case 2: army_bonus |= ArmyBase::ADD2STACK; break;
        case 3: army_bonus |= ArmyBase::ADD1STACK | ArmyBase::ADD2STACK; break;
        default:
          std::cerr << String::ucompose(_("Warning: unrecognized city plus flag of %1 for %2"), allplus, ar.name) << std::endl;
          break;
        }
      army->setArmyBonus(army_bonus);
      army->setXpReward(1);
      switch (ar.ally)
        {
        case 0: break;
        case 1: // ally
          army->setAwardable(true);
          army->setXpReward(10);
          break;
        case 2: // temple ally
          army->setAwardable(true);
          army->setXpReward(10);
          //FIXME what do we do about picking monsters to defend ruins?
          //army->setDefendsRuins(true); //not the best, but what the hey.
          break;
        default:
          std::cerr << String::ucompose(_("Warning: unrecognized ally flag of %1 for %2"), ar.ally, ar.name) << std::endl;
          break;
        }

      army->setSight(1);
      if (army->getName() == "Hero")
        army->setGender(Hero::MALE);

      ArmyProto *default_army =
        default_armyset->lookupArmyByName(army->getName());
      if (default_army)
        copy_armyset_images (armyset, default_armyset, army, default_army);
      armyset->push_back(army);
    }
  armyset->sort(sort_by_index);
  //now copy the bag, the stackship and the planted standard
  copy_other_armyset_images(default_armyset, armyset);
  set_ruin_defenders(armyset);
  return armyset;
}

static void
import_fight_order (FILE *scn, Armyset *armyset)
{
  fseek (scn, 0x60b, SEEK_CUR);
  char fight_order[29];
  char fight_order_no_boat[28];
  fread (fight_order, sizeof (char), 29, scn);

  int c = 0;
  for (int i = 0; i < 29; i++)
    {
      if (fight_order[i] == 28)
        continue;
      fight_order_no_boat[c] = fight_order[i];
      c++;
    }
  std::vector<int> order = std::vector<int>();
  order.reserve(29);
  for (int i = 0; i < 29; i++)
    order[i] = -1;
  c = 0;
  for (Armyset::iterator i = armyset->begin(); i != armyset->end(); ++i, c++)
    order[fight_order_no_boat[c]] = (*i)->getId();
  std::list<guint32> order_list;
  for (unsigned int i = 0; i < order.capacity(); i++)
    if (order[i] != -1)
      order_list.push_back(order[i]);
  Playerlist *pl = Playerlist::getInstance();
  for (Playerlist::iterator i = pl->begin(); i != pl->end(); ++i)
    (*i)->setFightOrder(order_list);
}

static bool compare_strength (const ArmyProto* first, const ArmyProto* second)
{
  int ffly = first->getMoveBonus() == Tile::isFlying();
  int sfly = second->getMoveBonus() == Tile::isFlying();
  int fhero = first->isHero();
  int rhero = second->isHero();
  int f = (first->getStrength() * 100) + (first->getProduction() * 101) + (ffly * 10) + (fhero * 100000);
  int s = (second->getStrength() * 100) + (second->getProduction() * 101) + (sfly * 10) + (rhero * 100000);
  if (f < s)
    return true;
  return false;
}

static void
import_initial_gold (FILE *scn)
{
  fseek(scn, 0x183, SEEK_CUR);
  struct rec_t
    {
      short player_id;
      short gold;
      char unused[16];
    };
  struct  rec_t recs[8];
  fread (recs, sizeof (struct rec_t), 8, scn);
  for (int i = 0; i < 8; i++)
    {
      int id = convert_player_id (recs[i].player_id);
      Player *p = Playerlist::getInstance()->getPlayer(id);
      if (p)
        p->setGold(recs[i].gold);
    }
}

static void 
importer (FILE *map, FILE *scn, FILE *rd, FILE *sg, FILE *it, FILE *sp, FILE *a, Glib::ustring name)
{
  GameScenario *g = setup_new_map (name);

  long at = ftell (map);
  import_terrain (map);
  fseek (map, at, SEEK_SET);
  import_bridges (map);
  import_roads(rd);

  Armyset *armyset = NULL;
  if (a)
    {
      armyset = import_armyset (a, name);
      armyset->save(File::getTempFile (File::getCacheDir(), name), ARMYSET_EXT);
      Armysetlist::getInstance()->add(armyset, File::getTempFile(File::getCacheDir(), name + ARMYSET_EXT));
    }
  if (!armyset)
    {
      std::cerr << _("Warning: no army file found.  Using default armyset.") << std::endl;
      armyset = Armysetlist::getInstance()->get(1);
    }

  at = ftell (scn);
  import_players (scn, armyset);
  fseek (scn, at, SEEK_SET);
  import_initial_gold(scn);
  fseek (scn, at, SEEK_SET);
  import_fight_order (scn, armyset);
  if (a)
    {
      //pretty hacky here.
      //we can sort it before we do fight order
      armyset->sort(compare_strength);
      armyset->save(File::getTempFile (File::getCacheDir(), name), ARMYSET_EXT);
      Armysetlist::getInstance()->add(armyset, File::getTempFile (File::getCacheDir(), name + ARMYSET_EXT));
    }
  fseek (scn, at, SEEK_SET);
  import_ruins_and_temples (scn, sp);
  fseek (scn, at, SEEK_SET);
  import_cities (scn);
  fseek (scn, at, SEEK_SET);
  set_capital_cities (scn);
  fseek (scn, at, SEEK_SET);
  import_signposts (sg);
  import_items (it);

  bool success = g->saveGame(name, MAP_EXT);
  if (!success)
    std::cerr << String::ucompose(_("Error: Could not save `%1%2'"), name, MAP_EXT) << std::endl;
  else
    std::cout << String::ucompose(_("Saved to %1.map."), name) << std::endl;
  delete g;
  return;
}

static std::string
read_armyset_name_from_armyname_file (FILE *a)
{
  char armyname[256];
  memset (armyname, 0, sizeof (armyname));
  fgets (armyname, sizeof (armyname)-1, a);
  if (strlen (armyname) == 0)
    return "";
  std::string name(armyname);
  std::transform(name.begin(), name.end(), name.begin(), ::toupper);
  return name +".DAT";
}

static FILE *
open_armyset_file (Glib::ustring directory, Glib::ustring name)
{
  FILE *a = NULL;
  Glib::ustring armyname_file =
    File::add_slash_if_necessary (directory) + "ARMYNAME.DAT";
  Glib::ustring armyset_file = "";
  if (File::exists (armyname_file))
    {
      a = fopen (armyname_file.c_str(), "rb");
      std::string n = read_armyset_name_from_armyname_file (a);
      fclose (a);
      armyset_file = File::add_slash_if_necessary (directory) + n;
      a = fopen (armyset_file.c_str(), "rb");
      if (!a)
        {
          //WL2 deluxe
          std::string upname = name;
          std::transform(upname.begin(), upname.end(), upname.begin(), ::toupper);
          armyset_file = 
            Glib::build_filename (directory, "..", "..", "ARMY", name, n);
          a = fopen (armyset_file.c_str(), "rb");
        }
    }
  if (!a)
    {
      //WL2
      armyset_file = File::add_slash_if_necessary (directory) + 
        "../TERRAIN0/ARMYTYPE.DAT";
      a = fopen (armyset_file.c_str(), "rb");
    }
  return a;
}

static long
get_armyset_offset (Glib::ustring filename)
{
  long offset = get_offset (filename, "ARMYNAME.DAT", false);
  if (!offset)
    return 0;
  FILE *a = fopen (filename.c_str(), "r");
  fseek (a, offset, SEEK_SET);
  std::string name = read_armyset_name_from_armyname_file (a);
  fclose (a);
  if (name.length() == 0)
    return 0;
  std::transform (name.begin(), name.end(), name.begin(), ::toupper);
  return get_offset (filename, name, false);
}

void show_help(Glib::ustring progname)
{
  std::cout << String::ucompose(_("Usage: %1 [OPTION]... FILE"), progname) << std::endl;
  std::cout << String::ucompose("  or:  %1 [OPTION]... DIRECTORY", progname) << std::endl << std::endl;
  std::cout << "LordsAWar! Warlords 2 Scenario Importing Tool " << _("version") << 
    " " << VERSION << std::endl << std::endl;
  std::cout << _("Options:") << std::endl << std::endl; 
  std::cout << "  -?, --help                 " << _("Display this help and exit") <<std::endl;
  std::cout << "  -a, --army-file FILE       " << _("Use this WL2 army file") <<std::endl;
  std::cout << std::endl;
  std::cout << _("Report bugs to") << " <" << PACKAGE_BUGREPORT ">." << std::endl;
}

int
main (int argc, char* argv[])
{
  Glib::ustring armyset_filename;
  Glib::ustring filename;
  initialize_configuration();
  Vector<int>::setMaximumWidth(1000);

  Glib::init();
  #if ENABLE_NLS
  setlocale(LC_ALL, Configuration::s_lang.c_str());
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
  #endif

  if (argc == 1)
    {
      show_help(argv[0]);
      exit (0);
    }

  if (argc > 1)
    {
      for (int i = 2; i <= argc; i++)
	{
          Glib::ustring parameter(argv[i-1]); 
	  if (parameter == "--help" || parameter == "-?")
	    {
              show_help(argv[0]);
	      exit(0);
	    }
          else if (parameter == "--army-file" || parameter == "-a")
            {
              i++;
              armyset_filename = argv[i-1];
              if (!File::exists (armyset_filename))
                {
                  std::cerr << String::ucompose(_("Error: Couldn't open `%1' for reading."), armyset_filename) << std::endl;
                  exit (EXIT_FAILURE);
                }
            }
	  else
	    filename = parameter;
	}
    }

  if (File::directory_exists (filename) == true)
    {
      //look for the files we need.
      std::list<Glib::ustring> map = File::scanForFiles(filename, ".MAP");
      if (map.size() == 0)
        {
          std::cerr << String::ucompose (_("Error: Could not find a .MAP file in %1"), filename) << std::endl;
          exit (EXIT_FAILURE);
        }
      std::list<Glib::ustring> scn = File::scanForFiles(filename, ".SCN");
      if (scn.size() == 0)
        {
          std::cerr << String::ucompose (_("Error: Could not find a .SCN file in `%1'"), filename) << std::endl;
          exit (EXIT_FAILURE);
        }
      std::list<Glib::ustring> rd = File::scanForFiles(filename, ".RD");
      if (rd.size() == 0)
        {
          std::cerr << String::ucompose (_("Error: Could not find a .RD file in `%1'"), filename) << std::endl;
          exit (EXIT_FAILURE);
        }
      std::list<Glib::ustring> signs = File::scanForFiles(filename, ".SGN");
      if (signs.size() == 0)
        {
          std::cerr << String::ucompose (_("Error: Could not find a .SGN file in `%1'"), filename) << std::endl;
          exit (EXIT_FAILURE);
        }
      std::list<Glib::ustring> items = File::scanForFiles(filename, ".ITM");
      if (items.size() == 0)
        {
          std::cerr << String::ucompose (_("Error: Could not find a .ITM file in `%1'"), filename) << std::endl;
          exit (EXIT_FAILURE);
        }
      std::list<Glib::ustring> spc = File::scanForFiles(filename, ".SPC");
      if (spc.size() == 0)
        {
          std::cerr << String::ucompose (_("Error: Could not find a .SPC file in `%1'"), filename) << std::endl;
          exit (EXIT_FAILURE);
        }
      FILE *m = fopen (map.front().c_str(), "rb");
      FILE *s = fopen (scn.front().c_str(), "rb");
      FILE *r = fopen (rd.front().c_str(), "rb");
      FILE *sg = fopen (signs.front().c_str(), "rb");
      FILE *it = fopen (items.front().c_str(), "rb");
      FILE *sp = fopen (spc.front().c_str(), "rb");
      Glib::ustring name = File::get_basename(map.front());
      FILE *a;
      if (armyset_filename != "")
        a = fopen (armyset_filename.c_str(), "rb");
      else
        a = open_armyset_file (filename, name);
      importer (m, s, r, sg, it, sp, a, name);
      fclose (m);
      fclose (s);
      fclose (r);
      fclose (sg);
      fclose (it);
      fclose (sp);
      if (a)
        fclose (a);
    }
  else if (File::exists (filename) == true)
    {
      Glib::ustring name = File::get_basename (filename);
      FILE *m = fopen (filename.c_str(), "rb");
      fseek (m, get_offset(filename, ".MAP", true), SEEK_SET);
      if (ftell (m) == 0)
        fseek (m, get_offset(filename, ".MAP", false), SEEK_SET);
      FILE *s = fopen (filename.c_str(), "rb");
      fseek (s, get_offset (filename, ".SCN", true), SEEK_SET);
      if (ftell (s) == 0)
        fseek (s, get_offset(filename, ".SCN", false), SEEK_SET);
      FILE *r = fopen (filename.c_str(), "rb");
      fseek (r, get_offset (filename, ".RD", true), SEEK_SET);
      if (ftell (r) == 0)
        fseek (r, get_offset(filename, ".RD", false), SEEK_SET);
      FILE *sg = fopen (filename.c_str(), "rb");
      fseek (sg, get_offset (filename, ".SGN", true), SEEK_SET);
      if (ftell (sg) == 0)
        fseek (sg, get_offset(filename, ".SGN", false), SEEK_SET);
      FILE *it = fopen (filename.c_str(), "rb");
      fseek (it, get_offset(filename, ".ITM", true), SEEK_SET);
      if (ftell (it) == 0)
        fseek (it, get_offset(filename, ".ITM", false), SEEK_SET);
      FILE *sp = fopen (filename.c_str(), "rb");
      fseek (sp, get_offset(filename, ".SPC", true), SEEK_SET);
      if (ftell (sp) == 0)
        fseek (sp, get_offset(filename, ".SPC", false), SEEK_SET);
      FILE *a;
      if (armyset_filename != "")
        a = fopen (armyset_filename.c_str(), "rb");
      else
        {
          a = fopen (filename.c_str(), "rb");
          fseek (a, get_armyset_offset(filename), SEEK_SET);
          if (ftell (a) == 0)
            {
              fclose (a);
              a = NULL;
            }
        }
      importer (m, s, r, sg, it, sp, a, name);
      fclose (m);
      fclose (s);
      fclose (r);
      fclose (it);
      fclose (sg);
      fclose (sp);
      if (a)
        fclose (a);
    }
  else
    {
      std::cerr << String::ucompose (_("Error: Could not open `%1'"), filename) << std::endl;
      exit (EXIT_FAILURE);
    }
  return EXIT_SUCCESS;
}
