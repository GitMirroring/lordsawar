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
#ifndef SWITCH_SETS_EDITOR_ACTIONS_H
#define SWITCH_SETS_EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include "undo-mgr.h"

//! A record of an event in the switch sets editor
/** 
 * The purpose of these classes is to implement undo/redo in the switch sets
 * editor.
 */

class SwitchSetsEditorAction: public UndoAction
{
public:

    enum Type {
      ARMYSET = 1,
      TILESET = 2,
      CITYSET = 3,
      SHIELDSET = 4,
      MAKE_SAME = 5,
      TILE_SIZE = 6,
    };

    SwitchSetsEditorAction(Type type, bool agg = false)
     : UndoAction (agg ? UndoAction::AGGREGATE_DELAY :
                   UndoAction::AGGREGATE_NONE), d_type (type) {}

    Type getType() const {return d_type;}

protected:

    Type d_type;
};

class SwitchSetsEditorAction_Rows: public SwitchSetsEditorAction
{
    public:
        SwitchSetsEditorAction_Rows (Type t, std::map<guint32,int> rows)
          : SwitchSetsEditorAction (t), d_rows (rows) {}
        ~SwitchSetsEditorAction_Rows () {}

        std::map<guint32,int> getArmysetRows () const {return d_rows;}

    private:
        std::map<guint32,int> d_rows;
};

class SwitchSetsEditorAction_Armyset: public SwitchSetsEditorAction_Rows
{
    public:
        SwitchSetsEditorAction_Armyset (std::map<guint32,int> rows)
          : SwitchSetsEditorAction_Rows (ARMYSET, rows) {}
        ~SwitchSetsEditorAction_Armyset () {}

        Glib::ustring getActionName () const {return "ArmySet";}
};

class SwitchSetsEditorAction_MakeSame: public SwitchSetsEditorAction_Rows
{
    public:
        SwitchSetsEditorAction_MakeSame (std::map<guint32,int> rows)
          : SwitchSetsEditorAction_Rows (MAKE_SAME, rows) {}
        ~SwitchSetsEditorAction_MakeSame () {}

        Glib::ustring getActionName () const {return "MakeSame";}
};

class SwitchSetsEditorAction_Row: public SwitchSetsEditorAction
{
    public:
        SwitchSetsEditorAction_Row (Type t, int row)
          : SwitchSetsEditorAction (t), d_row (row) {}
        ~SwitchSetsEditorAction_Row () {}

        int getRow () const {return d_row;}

    private:
        int d_row;
};

class SwitchSetsEditorAction_Set: public SwitchSetsEditorAction
{
    public:
        SwitchSetsEditorAction_Set (Type t, guint32 id)
          : SwitchSetsEditorAction (t), d_id (id) {}
        ~SwitchSetsEditorAction_Set () {}

        guint32 getId () const {return d_id;}

    private:
        guint32 d_id;
};

class SwitchSetsEditorAction_Tileset: public SwitchSetsEditorAction_Row
{
    public:
        SwitchSetsEditorAction_Tileset (int row)
          : SwitchSetsEditorAction_Row (TILESET, row) {}
        ~SwitchSetsEditorAction_Tileset () {}

        Glib::ustring getActionName () const {return "Tileset";}
};

class SwitchSetsEditorAction_Shieldset: public SwitchSetsEditorAction_Row
{
    public:
        SwitchSetsEditorAction_Shieldset (int row)
          : SwitchSetsEditorAction_Row (SHIELDSET, row) {}
        ~SwitchSetsEditorAction_Shieldset () {}

        Glib::ustring getActionName () const {return "Shieldset";}
};

class SwitchSetsEditorAction_Cityset: public SwitchSetsEditorAction_Row
{
    public:
        SwitchSetsEditorAction_Cityset (int row)
          : SwitchSetsEditorAction_Row (CITYSET, row) {}
        ~SwitchSetsEditorAction_Cityset () {}

        Glib::ustring getActionName () const {return "Cityset";}
};

class SwitchSetsEditorAction_TileSize: public SwitchSetsEditorAction
{
    public:
        SwitchSetsEditorAction_TileSize (int tilesize_row, int ts_row,
                                         int cs_row, int ss_row,
                                         std::map<guint32,int> map)
          : SwitchSetsEditorAction (TILE_SIZE), d_tilesize_row (tilesize_row),
          d_ts_row (ts_row), d_cs_row (cs_row), d_ss_row (ss_row), d_map (map)
  {}
        ~SwitchSetsEditorAction_TileSize () {}

        Glib::ustring getActionName () const {return "TileSize";}

        int getRow () const {return d_tilesize_row;}
        int getTilesetRow () {return d_ts_row;}
        int getCitysetRow () {return d_cs_row;}
        int getShieldsetRow () {return d_ss_row;}
        std::map<guint32,int> getArmysetRows() {return d_map;}

    private:
        int d_tilesize_row;
        int d_ts_row;
        int d_cs_row;
        int d_ss_row;
        std::map<guint32,int> d_map;
};

#endif //SWITCH_SETS_EDITOR_ACTIONS_H
