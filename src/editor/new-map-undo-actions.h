// Copyright (C) 2021 Ben Asselstine
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
#ifndef NEW_MAP_ACTIONS_H
#define NEW_MAP_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"

    struct Map
      {
        int fill_style;
        int width, height;
        int grass, water, swamp, forest, hills, mountains;
        int cities, ruins, temples;
        int signposts, stones;
        Glib::ustring tileset;
        Glib::ustring shieldset;
        Glib::ustring cityset;
        Glib::ustring armyset;
        bool generate_roads;
        bool random_names;
        int num_players;
        int stone_road_chance;
      };

//! A record of an event in the new map dialog
/** 
 * The purpose of these classes is to implement undo/redo in the new map
 * dialog.
 */

class NewMapAction: public UndoAction
{
public:

    //! A New Map Action can be one of the following kinds.
    enum Type {
      GRASS  = 1,
      WATER = 2,
      SWAMP = 3,
      FOREST = 4,
      HILLS = 5,
      MOUNTAINS= 6,
      CITIES = 7,
      RUINS = 8,
      TEMPLES = 9,
      SIGNPOSTS = 10,
      STONES = 11,
      MAP_SIZE = 12,
      WIDTH = 13,
      HEIGHT = 14,
      TILESET = 15,
      ARMYSET = 16,
      CITYSET = 17,
      SHIELDSET = 18,
      TILE_SIZE = 19,
      FILL_STYLE = 20,
      RANDOM_ROADS = 21,
      RANDOM_NAMES = 22,
      PLAYERS = 23,
      STONE_ROAD_CHANCE = 24,
    };

    //! Default constructor.
    NewMapAction(Type type, bool agg = false)
     : UndoAction (agg ? UndoAction::AGGREGATE_DELAY :
                   UndoAction::AGGREGATE_NONE), d_type (type) {}

    Type getType() const {return d_type;}

protected:

    Type d_type;
};

//-----------------------------------------------------------------------------
class NewMapAction_Scale: public NewMapAction
{
    public:
        NewMapAction_Scale (Type t, double v)
          : NewMapAction (t, true), d_value (v) {}
        double getValue () {return d_value;}
    private:
        double d_value;
};
//-----------------------------------------------------------------------------
class NewMapAction_Grass: public NewMapAction_Scale
{
    public:
        NewMapAction_Grass (double v)
          : NewMapAction_Scale (GRASS, v) {}
        ~NewMapAction_Grass () {}
        Glib::ustring getActionName () const {return "Grass";}
};
//-----------------------------------------------------------------------------
class NewMapAction_Water: public NewMapAction_Scale
{
    public:
        NewMapAction_Water (double v)
          : NewMapAction_Scale (WATER, v) {}
        ~NewMapAction_Water () {}
        Glib::ustring getActionName () const {return "Water";}
};
//-----------------------------------------------------------------------------
class NewMapAction_Swamp: public NewMapAction_Scale
{
    public:
        NewMapAction_Swamp (double v)
          : NewMapAction_Scale (SWAMP, v) {}
        ~NewMapAction_Swamp () {}
        Glib::ustring getActionName () const {return "Swamp";}
};

//-----------------------------------------------------------------------------
class NewMapAction_Forest: public NewMapAction_Scale
{
    public:
        NewMapAction_Forest (double v)
          : NewMapAction_Scale (FOREST, v) {}
        ~NewMapAction_Forest () {}
        Glib::ustring getActionName () const {return "Forest";}
};

//-----------------------------------------------------------------------------
class NewMapAction_Hills: public NewMapAction_Scale
{
    public:
        NewMapAction_Hills (double v)
          : NewMapAction_Scale (HILLS, v) {}
        ~NewMapAction_Hills () {}
        Glib::ustring getActionName () const {return "Hills";}
};

//-----------------------------------------------------------------------------
class NewMapAction_Mountains: public NewMapAction_Scale
{
    public:
        NewMapAction_Mountains (double v)
          : NewMapAction_Scale (MOUNTAINS, v) {}
        ~NewMapAction_Mountains () {}
        Glib::ustring getActionName () const {return "Mountains";}
};

//-----------------------------------------------------------------------------
class NewMapAction_Cities: public NewMapAction_Scale
{
    public:
        NewMapAction_Cities (double v)
          : NewMapAction_Scale (CITIES, v) {}
        ~NewMapAction_Cities () {}
        Glib::ustring getActionName () const {return "Cities";}
};

//-----------------------------------------------------------------------------
class NewMapAction_Ruins: public NewMapAction_Scale
{
    public:
        NewMapAction_Ruins (double v)
          : NewMapAction_Scale (RUINS, v) {}
        ~NewMapAction_Ruins () {}
        Glib::ustring getActionName () const {return "Ruins";}
};

//-----------------------------------------------------------------------------
class NewMapAction_Temples: public NewMapAction_Scale
{
    public:
        NewMapAction_Temples (double v)
          : NewMapAction_Scale (TEMPLES, v) {}
        ~NewMapAction_Temples () {}
        Glib::ustring getActionName () const {return "Temples";}
};

//-----------------------------------------------------------------------------
class NewMapAction_Signposts: public NewMapAction_Scale
{
    public:
        NewMapAction_Signposts (double v)
          : NewMapAction_Scale (SIGNPOSTS, v) {}
        ~NewMapAction_Signposts () {}
        Glib::ustring getActionName () const {return "Signposts";}
};

//-----------------------------------------------------------------------------
class NewMapAction_Stones: public NewMapAction_Scale
{
    public:
        NewMapAction_Stones (double v)
          : NewMapAction_Scale (STONES, v) {}
        ~NewMapAction_Stones () {}
        Glib::ustring getActionName () const {return "Stones";}
};

//-----------------------------------------------------------------------------
class NewMapAction_MapSize: public NewMapAction
{
    public:
        NewMapAction_MapSize (Map m, int ms)
          : NewMapAction (MAP_SIZE, false), d_map (m), d_map_size_id (ms) {}
        ~NewMapAction_MapSize () {}
        Glib::ustring getActionName () const {return "MapSize";}
        Map getMap () const {return d_map;}
        int getMapSizeId () const {return d_map_size_id;}
    private:
        Map d_map;
        int d_map_size_id;
};
//-----------------------------------------------------------------------------
class NewMapAction_Width: public NewMapAction
{
    public:
        NewMapAction_Width (int w)
          : NewMapAction (WIDTH, true), d_width (w) {}
        ~NewMapAction_Width () {}
        Glib::ustring getActionName () const {return "Width";}
        int getWidth () {return d_width;}
    private:
        int d_width;
};
//-----------------------------------------------------------------------------
class NewMapAction_Height: public NewMapAction
{
    public:
        NewMapAction_Height (int h)
          : NewMapAction (HEIGHT, true), d_height (h) {}
        ~NewMapAction_Height () {}
        Glib::ustring getActionName () const {return "Height";}
        int getHeight () {return d_height;}
    private:
        int d_height;
};
//-----------------------------------------------------------------------------
class NewMapAction_TileSet: public NewMapAction
{
    public:
        NewMapAction_TileSet (int i)
          : NewMapAction (TILESET, false), d_index (i) {}
        ~NewMapAction_TileSet () {}
        Glib::ustring getActionName () const {return "TileSet";}
        int getIndex () const {return d_index;}
    private:
        int d_index;
};
//-----------------------------------------------------------------------------
class NewMapAction_ArmySet: public NewMapAction
{
    public:
        NewMapAction_ArmySet (int i)
          : NewMapAction (ARMYSET, false), d_index (i) {}
        ~NewMapAction_ArmySet () {}
        Glib::ustring getActionName () const {return "ArmySet";}
        int getIndex () const {return d_index;}
    private:
        int d_index;
};
//-----------------------------------------------------------------------------
class NewMapAction_CitySet: public NewMapAction
{
    public:
        NewMapAction_CitySet (int i)
          : NewMapAction (CITYSET, false), d_index (i) {}
        ~NewMapAction_CitySet () {}
        Glib::ustring getActionName () const {return "CitySet";}
        int getIndex () const {return d_index;}
    private:
        int d_index;
};
//-----------------------------------------------------------------------------
class NewMapAction_ShieldSet: public NewMapAction
{
    public:
        NewMapAction_ShieldSet (int i)
          : NewMapAction (SHIELDSET, false), d_index (i) {}
        ~NewMapAction_ShieldSet () {}
        Glib::ustring getActionName () const {return "ShieldSet";}
        int getIndex () const {return d_index;}
    private:
        int d_index;
};
//-----------------------------------------------------------------------------
class NewMapAction_TileSize: public NewMapAction
{
    public:
        NewMapAction_TileSize (int a, int b, int c, int d)
          : NewMapAction (TILE_SIZE, false), d_tile_size_id (a),
          d_tileset_id (b), d_armyset_id (c), d_cityset_id (d) {}
        ~NewMapAction_TileSize () {}
        Glib::ustring getActionName () const {return "TileSize";}
        int getTileSizeId() const {return d_tile_size_id;}
        int getTileSetId() const {return d_tileset_id;}
        int getArmySetId() const {return d_armyset_id;}
        int getCitySetId() const {return d_cityset_id;}
    private:
        int d_tile_size_id;
        int d_tileset_id;
        int d_armyset_id;
        int d_cityset_id;
};
//-----------------------------------------------------------------------------
class NewMapAction_FillStyle: public NewMapAction
{
    public:
        NewMapAction_FillStyle (int i)
          : NewMapAction (FILL_STYLE, false), d_index (i) {}
        ~NewMapAction_FillStyle () {}
        Glib::ustring getActionName () const {return "FillStyle";}
        int getIndex () const {return d_index;}
    private:
        int d_index;
};
//-----------------------------------------------------------------------------
class NewMapAction_RandomRoads: public NewMapAction
{
    public:
        NewMapAction_RandomRoads (bool r)
          : NewMapAction (RANDOM_ROADS, false), d_value (r) {}
        ~NewMapAction_RandomRoads () {}
        Glib::ustring getActionName () const {return "RandomRoads";}
        bool getValue () const {return d_value;}
    private:
        bool d_value;
};
//-----------------------------------------------------------------------------
class NewMapAction_RandomNames: public NewMapAction
{
    public:
        NewMapAction_RandomNames (bool r)
          : NewMapAction (RANDOM_NAMES, false), d_value (r) {}
        ~NewMapAction_RandomNames () {}
        Glib::ustring getActionName () const {return "RandomNames";}
        bool getValue () const {return d_value;}
    private:
        bool d_value;
};
//-----------------------------------------------------------------------------
class NewMapAction_Players: public NewMapAction
{
    public:
        NewMapAction_Players (int n)
          : NewMapAction (PLAYERS, true), d_num (n) {}
        ~NewMapAction_Players () {}
        Glib::ustring getActionName () const {return "Players";}
        int getNumPlayers () {return d_num;}
    private:
        int d_num;
};
//-----------------------------------------------------------------------------
class NewMapAction_StoneRoadChance: public NewMapAction
{
    public:
        NewMapAction_StoneRoadChance (int n)
          : NewMapAction (STONE_ROAD_CHANCE, true), d_num (n) {}
        ~NewMapAction_StoneRoadChance () {}
        Glib::ustring getActionName () const {return "StoneRoadChance";}
        int getStoneRoadChance () {return d_num;}
    private:
        int d_num;
};
#endif //NEW_MAP_ACTIONS_H
