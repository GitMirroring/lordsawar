//  Copyright (C) 2021, 2026 Ben Asselstine
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

#pragma once
#ifndef NEW_MAP_UNDO_ACTIONS_H
#define NEW_MAP_UNDO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"

//! A record of an event in the new map dialog
/** 
 * The purpose of these classes is to implement undo/redo in the new map
 * dialog.
 */

class NewMapUndoAction: public UndoAction
{
public:

    //! A New Map Action can be one of the following kinds.
    enum Type
      {
        TERRAIN = 1,
        CITIES = 2,
        RUINS = 3,
        TEMPLES = 4,
        SIGNPOSTS = 5,
        STONES = 6,
        MAP_SIZE = 7,
        WIDTH = 8,
        HEIGHT = 9,
        TILESET = 10,
        ARMYSET = 11,
        CITYSET = 12,
        SHIELDSET = 13,
        TILE_SIZE = 14,
        FILL_STYLE = 15,
        RANDOM_ROADS = 16,
        RANDOM_NAMES = 17,
        PLAYER = 18,
        STONE_ROAD_CHANCE = 19,
        MAKE_SAME = 20,
      };

    //! Default constructor.
    NewMapUndoAction (Type type, bool agg = false)
      : UndoAction (agg ? UndoAction::AGGREGATE_DELAY :
                    UndoAction::AGGREGATE_NONE), m_type (type)
        {
        }

    Type get_type () const
      {
        return m_type;
      }

protected:

    Type m_type;
};

class NewMapUndoAction_Terrain: public NewMapUndoAction
{
public:
    NewMapUndoAction_Terrain (double g, double w, double s, double f, double h,
                              double m)
      : NewMapUndoAction (TERRAIN, true), m_grass_value (g),
      m_water_value (w), m_swamp_value (s), m_forest_value (f),
      m_hills_value (h), m_mountains_value (m)
      {
      }

    double get_grass_value () const
      {
        return m_grass_value;
      }

    double get_water_value () const
      {
        return m_water_value;
      }

    double get_swamp_value () const
      {
        return m_swamp_value;
      }

    double get_forest_value () const
      {
        return m_forest_value;
      }

    double get_hills_value () const
      {
        return m_hills_value;
      }

    double get_mountains_value () const
      {
        return m_mountains_value;
      }

    Glib::ustring get_action_name () const
      {
        return "Terrain";
      }
private:
    double m_grass_value;
    double m_water_value;
    double m_swamp_value;
    double m_forest_value;
    double m_hills_value;
    double m_mountains_value;
};

class NewMapUndoAction_Scale: public NewMapUndoAction
{
    public:
        NewMapUndoAction_Scale (Type t, double v)
          : NewMapUndoAction (t, true), m_value (v)
          {
          }

        double get_value () const
          {
            return m_value;
          }
    private:
        double m_value;
};

class NewMapUndoAction_Cities: public NewMapUndoAction_Scale
{
public:
    NewMapUndoAction_Cities (double v)
      : NewMapUndoAction_Scale (CITIES, v)
      {
      }

    ~NewMapUndoAction_Cities ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Cities";
      }
};

class NewMapUndoAction_Ruins: public NewMapUndoAction_Scale
{
public:
    NewMapUndoAction_Ruins (double v)
      : NewMapUndoAction_Scale (RUINS, v)
      {
      }

    ~NewMapUndoAction_Ruins ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Ruins";
      }
};

class NewMapUndoAction_Temples: public NewMapUndoAction_Scale
{
public:
    NewMapUndoAction_Temples (double v)
      : NewMapUndoAction_Scale (TEMPLES, v)
      {
      }

    ~NewMapUndoAction_Temples ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Temples";
      }
};

class NewMapUndoAction_Signposts: public NewMapUndoAction_Scale
{
public:
    NewMapUndoAction_Signposts (double v)
      : NewMapUndoAction_Scale (SIGNPOSTS, v)
      {
      }

    ~NewMapUndoAction_Signposts ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Signposts";
      }
};

class NewMapUndoAction_Stones: public NewMapUndoAction_Scale
{
public:
    NewMapUndoAction_Stones (double v)
      : NewMapUndoAction_Scale (STONES, v)
      {
      }

    ~NewMapUndoAction_Stones ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Stones";
      }
};

class NewMapUndoAction_MapSize: public NewMapUndoAction
{
public:
    NewMapUndoAction_MapSize (int ms, int c, int r, int t, int si, int st,
                              int w, int h)
      : NewMapUndoAction (MAP_SIZE, false), m_map_size_row (ms),
      m_num_cities (c), m_num_ruins (r), m_num_temples (t),
      m_num_signposts (si), m_num_stones (st), m_width (w), m_height (h)
      {
      }

    ~NewMapUndoAction_MapSize ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "MapSize";
      }

    int get_map_size_row () const
      {
        return m_map_size_row;
      }

    int get_num_cities () const
      {
        return m_num_cities;
      }

    int get_num_ruins () const
      {
        return m_num_ruins;
      }

    int get_num_temples () const
      {
        return m_num_temples;
      }

    int get_num_signposts () const
      {
        return m_num_signposts;
      }

    int get_num_stones () const
      {
        return m_num_stones;
      }

    int get_width () const
      {
        return m_width;
      }

    int get_height () const
      {
        return m_height;
      }
private:
    int m_map_size_row;
    int m_num_cities;
    int m_num_ruins;
    int m_num_temples;
    int m_num_signposts;
    int m_num_stones;
    int m_width;
    int m_height;
};

class NewMapUndoAction_Width: public NewMapUndoAction
{
public:
    NewMapUndoAction_Width (int w)
      : NewMapUndoAction (WIDTH, true), m_width (w)
      {
      }

    ~NewMapUndoAction_Width ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Width";
      }

    int get_width () const
      {
        return m_width;
      }
private:
    int m_width;
};

class NewMapUndoAction_Height: public NewMapUndoAction
{
public:
    NewMapUndoAction_Height (int h)
      : NewMapUndoAction (HEIGHT, true), m_height (h)
      {
      }

    ~NewMapUndoAction_Height ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Height";
      }

    int get_height () const
      {
        return m_height;
      }
private:
    int m_height;
};

class NewMapUndoAction_TileSet: public NewMapUndoAction
{
public:
    NewMapUndoAction_TileSet (int i)
      : NewMapUndoAction (TILESET, false), m_index (i)
      {
      }

    ~NewMapUndoAction_TileSet ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "TileSet";
      }

    int get_index () const
      {
        return m_index;
      }
private:
    int m_index;
};

class NewMapUndoAction_ArmySet: public NewMapUndoAction
{
public:
    NewMapUndoAction_ArmySet (int a, int b, int c, int d, int e, int f, int g,
                              int h, int i)
      : NewMapUndoAction (ARMYSET, false), m_player1_row (a), m_player2_row (b),
      m_player3_row (c), m_player4_row (d), m_player5_row (e),
      m_player6_row (f), m_player7_row (g), m_player8_row (h), m_neutral_row (i)
      {
      }

    ~NewMapUndoAction_ArmySet ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "ArmySet";
      }

    int get_player1_row () const
      {
        return m_player1_row;
      }

    int get_player2_row () const
      {
        return m_player2_row;
      }

    int get_player3_row () const
      {
        return m_player3_row;
      }

    int get_player4_row () const
      {
        return m_player4_row;
      }

    int get_player5_row () const
      {
        return m_player5_row;
      }

    int get_player6_row () const
      {
        return m_player6_row;
      }

    int get_player7_row () const
      {
        return m_player7_row;
      }

    int get_player8_row () const
      {
        return m_player8_row;
      }

    int get_neutral_row () const
      {
        return m_neutral_row;
      }
private:
    int m_player1_row;
    int m_player2_row;
    int m_player3_row;
    int m_player4_row;
    int m_player5_row;
    int m_player6_row;
    int m_player7_row;
    int m_player8_row;
    int m_neutral_row;
};

class NewMapUndoAction_CitySet: public NewMapUndoAction
{
public:
    NewMapUndoAction_CitySet (int i)
      : NewMapUndoAction (CITYSET, false), m_index (i)
      {
      }

    ~NewMapUndoAction_CitySet ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "CitySet";
      }

    int get_index () const
      {
        return m_index;
      }
private:
    int m_index;
};

class NewMapUndoAction_ShieldSet: public NewMapUndoAction
{
public:
    NewMapUndoAction_ShieldSet (int i)
      : NewMapUndoAction (SHIELDSET, false), m_index (i)
      {
      }

    ~NewMapUndoAction_ShieldSet ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "ShieldSet";
      }

    int get_index () const
      {
        return m_index;
      }
private:
    int m_index;
};

class NewMapUndoAction_TileSize: public NewMapUndoAction
{
public:
    NewMapUndoAction_TileSize (int a, int b, int c, int d, int e, int f, int g,
                               int h, int i, int j, int k, int l)
      : NewMapUndoAction (TILE_SIZE, false), m_tile_size_row (a),
      m_tileset_row (b), m_player1_armyset_row (c),
      m_player2_armyset_row (d), m_player3_armyset_row (e),
      m_player4_armyset_row (f), m_player5_armyset_row (g),
      m_player6_armyset_row (h), m_player7_armyset_row (i),
      m_player8_armyset_row (j), m_neutral_armyset_row (k),
      m_cityset_row (l)
  {
  }

    ~NewMapUndoAction_TileSize ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "TileSize";
      }

    int get_tile_size_row () const
      {
        return m_tile_size_row;
      }

    int get_tileset_row () const
      {
        return m_tileset_row;
      }

    int get_cityset_row () const
      {
        return m_cityset_row;
      }

    int get_player1_armyset_row () const
      {
        return m_player1_armyset_row;
      }

    int get_player2_armyset_row () const
      {
        return m_player2_armyset_row;
      }

    int get_player3_armyset_row () const
      {
        return m_player3_armyset_row;
      }

    int get_player4_armyset_row () const
      {
        return m_player4_armyset_row;
      }

    int get_player5_armyset_row () const
      {
        return m_player5_armyset_row;
      }

    int get_player6_armyset_row () const
      {
        return m_player6_armyset_row;
      }

    int get_player7_armyset_row () const
      {
        return m_player7_armyset_row;
      }

    int get_player8_armyset_row () const
      {
        return m_player8_armyset_row;
      }

    int get_neutral_armyset_row () const
      {
        return m_neutral_armyset_row;
      }

private:
    int m_tile_size_row;
    int m_tileset_row;
    int m_player1_armyset_row;
    int m_player2_armyset_row;
    int m_player3_armyset_row;
    int m_player4_armyset_row;
    int m_player5_armyset_row;
    int m_player6_armyset_row;
    int m_player7_armyset_row;
    int m_player8_armyset_row;
    int m_neutral_armyset_row;
    int m_cityset_row;
};

class NewMapUndoAction_FillStyle: public NewMapUndoAction
{
public:
    NewMapUndoAction_FillStyle (int i)
      : NewMapUndoAction (FILL_STYLE, false), m_index (i)
      {
      }

    ~NewMapUndoAction_FillStyle ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "FillStyle";
      }

    int get_index () const
      {
        return m_index;
      }
private:
    int m_index;
};

class NewMapUndoAction_RandomRoads: public NewMapUndoAction
{
public:
    NewMapUndoAction_RandomRoads (bool r)
      : NewMapUndoAction (RANDOM_ROADS, false), m_value (r)
      {
      }

    ~NewMapUndoAction_RandomRoads ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "RandomRoads";
      }

    bool get_value () const
      {
        return m_value;
      }
private:
    bool m_value;
};

class NewMapUndoAction_RandomNames: public NewMapUndoAction
{
public:
    NewMapUndoAction_RandomNames (bool r)
      : NewMapUndoAction (RANDOM_NAMES, false), m_value (r)
      {
      }

    ~NewMapUndoAction_RandomNames ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "RandomNames";
      }

    bool get_value () const
      {
        return m_value;
      }
private:
    bool m_value;
};

class NewMapUndoAction_Player: public NewMapUndoAction
{
public:
    NewMapUndoAction_Player (int i, bool active)
      : NewMapUndoAction (PLAYER, true), m_player_id (i), m_active (active)
      {
      }

    ~NewMapUndoAction_Player ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Player";
      }

    int get_player_id () const
      {
        return m_player_id;
      }

    bool get_active () const
      {
        return m_active;
      }
private:
    int m_player_id;
    bool m_active;
};

class NewMapUndoAction_StoneRoadChance: public NewMapUndoAction
{
public:
    NewMapUndoAction_StoneRoadChance (int n)
      : NewMapUndoAction (STONE_ROAD_CHANCE, true), m_num (n)
      {
      }

    ~NewMapUndoAction_StoneRoadChance ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "StoneRoadChance";
      }

    int get_stone_road_chance () const
      {
        return m_num;
      }
private:
    int m_num;
};

class NewMapUndoAction_MakeSame: public NewMapUndoAction
{
public:
    NewMapUndoAction_MakeSame (int v, int a, int b, int c, int d, int e, int f,
                               int g, int h, int i)
      : NewMapUndoAction (MAKE_SAME, true), m_value (v), m_player1_row (a),
      m_player2_row (b), m_player3_row (c), m_player4_row (d),
      m_player5_row (e), m_player6_row (f), m_player7_row (g),
      m_player8_row (h), m_neutral_row (i)
      {
      }

    ~NewMapUndoAction_MakeSame ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "MakeSame";
      }

    int get_value () const
      {
        return m_value;
      }

    int get_player1_row () const
      {
        return m_player1_row;
      }

    int get_player2_row () const
      {
        return m_player2_row;
      }

    int get_player3_row () const
      {
        return m_player3_row;
      }

    int get_player4_row () const
      {
        return m_player4_row;
      }

    int get_player5_row () const
      {
        return m_player5_row;
      }

    int get_player6_row () const
      {
        return m_player6_row;
      }

    int get_player7_row () const
      {
        return m_player7_row;
      }

    int get_player8_row () const
      {
        return m_player8_row;
      }

    int get_neutral_row () const
      {
        return m_neutral_row;
      }
private:
    int m_value;
    int m_player1_row;
    int m_player2_row;
    int m_player3_row;
    int m_player4_row;
    int m_player5_row;
    int m_player6_row;
    int m_player7_row;
    int m_player8_row;
    int m_neutral_row;
};
#endif
