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
#ifndef SWITCH_SETS_UNDO_ACTIONS_H
#define SWITCH_SETS_UNDO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"

//! A record of an event in the switch sets dialog
/** 
 * The purpose of these classes is to implement undo/redo in the switch
 * sets dialog
 */

class SwitchSetsUndoAction: public UndoAction
{
public:

    //! A Switch Sets Action can be one of the following kinds.
    enum Type
      {
        TILESET = 1,
        ARMYSET = 2,
        CITYSET = 3,
        SHIELDSET = 4,
        TILE_SIZE = 5,
        MAKE_SAME = 6,
      };

    //! Default constructor.
    SwitchSetsUndoAction (Type type, bool agg = false)
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

class SwitchSetsUndoAction_TileSet: public SwitchSetsUndoAction
{
public:
    SwitchSetsUndoAction_TileSet (int i)
      : SwitchSetsUndoAction (TILESET, false), m_index (i)
      {
      }

    ~SwitchSetsUndoAction_TileSet ()
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

class SwitchSetsUndoAction_ArmySet: public SwitchSetsUndoAction
{
public:
    SwitchSetsUndoAction_ArmySet (int a, int b, int c, int d, int e, int f,
                                  int g, int h, int i)
      : SwitchSetsUndoAction (ARMYSET, false), m_player1_row (a),
      m_player2_row (b), m_player3_row (c), m_player4_row (d),
      m_player5_row (e), m_player6_row (f), m_player7_row (g),
      m_player8_row (h), m_neutral_row (i)
  {
  }

    ~SwitchSetsUndoAction_ArmySet ()
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

class SwitchSetsUndoAction_CitySet: public SwitchSetsUndoAction
{
public:
    SwitchSetsUndoAction_CitySet (int i)
      : SwitchSetsUndoAction (CITYSET, false), m_index (i)
      {
      }

    ~SwitchSetsUndoAction_CitySet ()
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

class SwitchSetsUndoAction_ShieldSet: public SwitchSetsUndoAction
{
public:
    SwitchSetsUndoAction_ShieldSet (int i)
      : SwitchSetsUndoAction (SHIELDSET, false), m_index (i)
      {
      }

    ~SwitchSetsUndoAction_ShieldSet ()
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

class SwitchSetsUndoAction_TileSize: public SwitchSetsUndoAction
{
public:
    SwitchSetsUndoAction_TileSize (int a, int b, int c, int d, int e, int f,
                                   int g, int h, int i, int j, int k, int l)
      : SwitchSetsUndoAction (TILE_SIZE, false), m_tile_size_row (a),
      m_tileset_row (b), m_player1_armyset_row (c),
      m_player2_armyset_row (d), m_player3_armyset_row (e),
      m_player4_armyset_row (f), m_player5_armyset_row (g),
      m_player6_armyset_row (h), m_player7_armyset_row (i),
      m_player8_armyset_row (j), m_neutral_armyset_row (k),
      m_cityset_row (l)
  {
  }

    ~SwitchSetsUndoAction_TileSize ()
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

class SwitchSetsUndoAction_MakeSame: public SwitchSetsUndoAction
{
public:
    SwitchSetsUndoAction_MakeSame (int v, int a, int b, int c, int d, int e,
                                   int f, int g, int h, int i)
      : SwitchSetsUndoAction (MAKE_SAME, true), m_value (v), m_player1_row (a),
      m_player2_row (b), m_player3_row (c), m_player4_row (d),
      m_player5_row (e), m_player6_row (f), m_player7_row (g),
      m_player8_row (h), m_neutral_row (i)
  {
  }

    ~SwitchSetsUndoAction_MakeSame ()
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
