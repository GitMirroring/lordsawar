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
#ifndef ITEMLIST_EDITOR_ACTIONS_H
#define ITEMLIST_EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include "heroproto.h"
#include "herotemplates.h"
#include "undo-mgr.h"

//! A record of an event in the itemlist editor
/** 
 * The purpose of these classes is to implement undo/redo in the itemlist
 * editor.
 */

class ItemListEditorAction: public UndoAction
{
public:

    enum Type {
      ADD = 1,
      REMOVE = 2,
      BUILDING_TYPE_TO_SUMMON_ON = 3,
      DISEASE_CITY = 4,
      DISEASE_ARMIES_PERCENT = 5,
      RAISE_DEFENDERS = 6,
      NUM_DEFENDERS = 7,
      PERSUADE_NEUTRAL_CITY = 8,
      TELEPORT_TO_CITY = 9,
      ADDSTR = 10,
      ADDSTACK = 11,
      FLY_STACK = 12,
      DOUBLE_MOVEMENT_STACK = 13,
      ADD_GOLD_PER_CITY = 14,
      STEALS_GOLD = 15,
      PICKUP_BAGS = 16,
      ADD_MOVEMENT = 17,
      SINKS_SHIPS = 18,
      BANISH_WORMS = 19,
      BURN_BRIDGE = 20,
      CAPTURE_KEEPER = 21,
      SUMMON_MONSTER = 22,
      USES = 23,
      STEAL_PERCENT = 24,
      ADD_MP = 25,
      PLANTABLE = 26,
      NAME = 27,
      SELECT_BANISH_ARMY = 28,
      SELECT_SUMMON_ARMY = 29,
      SELECT_RAISE_ARMY = 30,
    };

    ItemListEditorAction(Type type, bool agg = false)
     : UndoAction (agg ? UndoAction::AGGREGATE_DELAY :
                   UndoAction::AGGREGATE_NONE), d_type (type) {}

    Type getType() const {return d_type;}

protected:

    Type d_type;
};

class ItemListEditorAction_Save : public ItemListEditorAction
{
    public:
        ItemListEditorAction_Save (Type t, Itemlist *i)
          :ItemListEditorAction (t, false), d_itemlist (i) {}
        ~ItemListEditorAction_Save ()
          {
            delete d_itemlist;
          }

        void clearItemList () {d_itemlist = NULL;}
        Itemlist * getItemList () const {return d_itemlist;}
    private:
        Itemlist *d_itemlist;
};

class ItemListEditorAction_Add: public ItemListEditorAction_Save
{
    public:
        ItemListEditorAction_Add (Itemlist *i)
          :ItemListEditorAction_Save (ADD, i) {}
        ~ItemListEditorAction_Add () {}

        Glib::ustring getActionName () const {return "Add";}
};

class ItemListEditorAction_Remove: public ItemListEditorAction_Save
{
    public:
        ItemListEditorAction_Remove (Itemlist *i)
          :ItemListEditorAction_Save (REMOVE, i) {}
        ~ItemListEditorAction_Remove () {}

        Glib::ustring getActionName () const {return "Remove";}
};

class ItemListEditorAction_Index: public ItemListEditorAction
{
    public:
        ItemListEditorAction_Index (Type t, guint32 i, bool agg = false)
          : ItemListEditorAction (t, agg), d_index (i) {}
        ~ItemListEditorAction_Index () {}

        guint32 getIndex () {return d_index;}
    private:
        guint32 d_index;
};

class ItemListEditorAction_BuildingTypeToSummonOn: public ItemListEditorAction_Index
{
    public:
        ItemListEditorAction_BuildingTypeToSummonOn (guint32 i, int row)
          : ItemListEditorAction_Index (BUILDING_TYPE_TO_SUMMON_ON, i, false),
          d_row (row) {}
        ~ItemListEditorAction_BuildingTypeToSummonOn () {}

        Glib::ustring getActionName () const {return "BuildingTypeToSummonOn";}

        int getRow () const {return d_row;}
    private:
        int d_row;
};

class ItemListEditorAction_FlagIndex: public ItemListEditorAction_Index
{
    public:
        ItemListEditorAction_FlagIndex (Type t, guint32 i, bool state)
          : ItemListEditorAction_Index (t, i), d_active (state) {}
        ~ItemListEditorAction_FlagIndex () {}

        bool getActive () const {return d_active;}

    private:
        bool d_active;
};
class ItemListEditorAction_DiseaseCity: public ItemListEditorAction_FlagIndex
{
    public:
        ItemListEditorAction_DiseaseCity (guint32 i, bool state)
          : ItemListEditorAction_FlagIndex (DISEASE_CITY, i, state) {}
        ~ItemListEditorAction_DiseaseCity () {}

        Glib::ustring getActionName () const {return "DiseaseCity";}
};
class ItemListEditorAction_SpinIndex: public ItemListEditorAction_Index
{
    public:
        ItemListEditorAction_SpinIndex (Type t, guint32 i, int num)
          : ItemListEditorAction_Index (t, i, true),
          d_num (num) {}
        ~ItemListEditorAction_SpinIndex () {}

        int getNum () const {return d_num;}

    private:
        int d_num;
};
class ItemListEditorAction_DiseaseArmies: public ItemListEditorAction_SpinIndex
{
    public:
        ItemListEditorAction_DiseaseArmies (guint32 i, int num)
          : ItemListEditorAction_SpinIndex (DISEASE_ARMIES_PERCENT, i, num) {}
        ~ItemListEditorAction_DiseaseArmies () {}

        Glib::ustring getActionName () const {return "DiseaseArmies";}
};
class ItemListEditorAction_RaiseDefenders: public ItemListEditorAction_FlagIndex
{
    public:
        ItemListEditorAction_RaiseDefenders (guint32 i, bool state)
          : ItemListEditorAction_FlagIndex (RAISE_DEFENDERS, i, state) {}
        ~ItemListEditorAction_RaiseDefenders () {}

        Glib::ustring getActionName () const {return "RaiseDefenders";}
};
class ItemListEditorAction_NumDefenders: public ItemListEditorAction_SpinIndex
{
    public:
        ItemListEditorAction_NumDefenders (guint32 i, int num)
          : ItemListEditorAction_SpinIndex (NUM_DEFENDERS, i, num) {}
        ~ItemListEditorAction_NumDefenders () {}

        Glib::ustring getActionName () const {return "NumDefenders";}
};
class ItemListEditorAction_PersuadeNeutralCity: public ItemListEditorAction_FlagIndex
{
    public:
        ItemListEditorAction_PersuadeNeutralCity (guint32 i, bool state)
          : ItemListEditorAction_FlagIndex (PERSUADE_NEUTRAL_CITY, i, state) {}
        ~ItemListEditorAction_PersuadeNeutralCity () {}

        Glib::ustring getActionName () const {return "PersuadeNeutralCity";}
};
class ItemListEditorAction_TeleportToCity: public ItemListEditorAction_FlagIndex
{
    public:
        ItemListEditorAction_TeleportToCity (guint32 i, bool state)
          : ItemListEditorAction_FlagIndex (TELEPORT_TO_CITY, i, state) {}
        ~ItemListEditorAction_TeleportToCity () {}

        Glib::ustring getActionName () const {return "TeleportToCity";}
};
class ItemListEditorAction_AddStr: public ItemListEditorAction_Index
{
    public:
        ItemListEditorAction_AddStr (guint32 i, int num, bool state)
          : ItemListEditorAction_Index (ADDSTR, i), d_num (num),
          d_active (state) {}
        ~ItemListEditorAction_AddStr () {}

        Glib::ustring getActionName () const {return "AddStr";}
        int getNum () const {return d_num;}
        bool getActive () const {return d_active;}

    private:
        int d_num;
        bool d_active;
};
class ItemListEditorAction_AddStack: public ItemListEditorAction_Index
{
    public:
        ItemListEditorAction_AddStack (guint32 i, int num, bool state)
          : ItemListEditorAction_Index (ADDSTACK, i), d_num (num),
          d_active (state) {}
        ~ItemListEditorAction_AddStack () {}

        Glib::ustring getActionName () const {return "AddStack";}
        int getNum () const {return d_num;}
        bool getActive () const {return d_active;}

    private:
        int d_num;
        bool d_active;
};
class ItemListEditorAction_FlyStack: public ItemListEditorAction_FlagIndex
{
    public:
        ItemListEditorAction_FlyStack (guint32 i, bool state)
          : ItemListEditorAction_FlagIndex (FLY_STACK, i, state) {}
        ~ItemListEditorAction_FlyStack () {}

        Glib::ustring getActionName () const {return "FlyStack";}
};
class ItemListEditorAction_DoubleMovementStack: public ItemListEditorAction_FlagIndex
{
    public:
        ItemListEditorAction_DoubleMovementStack (guint32 i, bool state)
          : ItemListEditorAction_FlagIndex (DOUBLE_MOVEMENT_STACK, i, state) {}
        ~ItemListEditorAction_DoubleMovementStack () {}

        Glib::ustring getActionName () const {return "DoubleMovementStack";}
};
class ItemListEditorAction_AddGoldPerCity: public ItemListEditorAction_Index
{
    public:
        ItemListEditorAction_AddGoldPerCity (guint32 i, int num, bool state)
          : ItemListEditorAction_Index (ADD_GOLD_PER_CITY, i), d_num (num),
         d_active (state) {}
        ~ItemListEditorAction_AddGoldPerCity () {}

        Glib::ustring getActionName () const {return "AddGoldPerCity";}
        int getNum () const {return d_num;}
        bool getActive () const {return d_active;}
    private:
        int d_num;
        bool d_active;
};
class ItemListEditorAction_StealsGold: public ItemListEditorAction_FlagIndex
{
    public:
        ItemListEditorAction_StealsGold (guint32 i, bool state)
          : ItemListEditorAction_FlagIndex (STEALS_GOLD, i, state) {}
        ~ItemListEditorAction_StealsGold () {}

        Glib::ustring getActionName () const {return "StealsGold";}
};
class ItemListEditorAction_PickupBags: public ItemListEditorAction_FlagIndex
{
    public:
        ItemListEditorAction_PickupBags (guint32 i, bool state)
          : ItemListEditorAction_FlagIndex (PICKUP_BAGS, i, state) {}
        ~ItemListEditorAction_PickupBags () {}

        Glib::ustring getActionName () const {return "PickupBags";}
};
class ItemListEditorAction_AddMovement: public ItemListEditorAction_FlagIndex
{
    public:
        ItemListEditorAction_AddMovement (guint32 i, bool state)
          : ItemListEditorAction_FlagIndex (ADD_MOVEMENT, i, state) {}
        ~ItemListEditorAction_AddMovement () {}

        Glib::ustring getActionName () const {return "AddMovement";}
};
class ItemListEditorAction_SinkShips: public ItemListEditorAction_FlagIndex
{
    public:
        ItemListEditorAction_SinkShips (guint32 i, bool state)
          : ItemListEditorAction_FlagIndex (SINKS_SHIPS, i, state) {}
        ~ItemListEditorAction_SinkShips () {}

        Glib::ustring getActionName () const {return "SinkShips";}
};
class ItemListEditorAction_BanishWorms: public ItemListEditorAction_FlagIndex
{
    public:
        ItemListEditorAction_BanishWorms (guint32 i, bool state)
          : ItemListEditorAction_FlagIndex (BANISH_WORMS, i, state) {}
        ~ItemListEditorAction_BanishWorms () {}

        Glib::ustring getActionName () const {return "BanishWorms";}
};
class ItemListEditorAction_BurnBridge: public ItemListEditorAction_FlagIndex
{
    public:
        ItemListEditorAction_BurnBridge (guint32 i, bool state)
          : ItemListEditorAction_FlagIndex (BURN_BRIDGE, i, state) {}
        ~ItemListEditorAction_BurnBridge () {}

        Glib::ustring getActionName () const {return "BurnBridge";}
};
class ItemListEditorAction_CaptureKeeper: public ItemListEditorAction_FlagIndex
{
    public:
        ItemListEditorAction_CaptureKeeper (guint32 i, bool state)
          : ItemListEditorAction_FlagIndex (CAPTURE_KEEPER, i, state) {}
        ~ItemListEditorAction_CaptureKeeper () {}

        Glib::ustring getActionName () const {return "CaptureKeeper";}
};
class ItemListEditorAction_SummonMonster: public ItemListEditorAction_FlagIndex
{
    public:
        ItemListEditorAction_SummonMonster (guint32 i, bool state)
          : ItemListEditorAction_FlagIndex (SUMMON_MONSTER, i, state) {}
        ~ItemListEditorAction_SummonMonster () {}

        Glib::ustring getActionName () const {return "SummonMonster";}
};
class ItemListEditorAction_Uses: public ItemListEditorAction_SpinIndex
{
    public:
        ItemListEditorAction_Uses (guint32 i, int num)
          : ItemListEditorAction_SpinIndex (USES, i, num) {}
        ~ItemListEditorAction_Uses () {}

        Glib::ustring getActionName () const {return "Uses";}
};
class ItemListEditorAction_StealPercent: public ItemListEditorAction_SpinIndex
{
    public:
        ItemListEditorAction_StealPercent (guint32 i, int num)
          : ItemListEditorAction_SpinIndex (STEAL_PERCENT, i, num) {}
        ~ItemListEditorAction_StealPercent () {}

        Glib::ustring getActionName () const {return "StealPercent";}
};
class ItemListEditorAction_AddMp: public ItemListEditorAction_SpinIndex
{
    public:
        ItemListEditorAction_AddMp (guint32 i, int num)
          : ItemListEditorAction_SpinIndex (ADD_MP, i, num) {}
        ~ItemListEditorAction_AddMp () {}

        Glib::ustring getActionName () const {return "AddMp";}
};
class ItemListEditorAction_Plantable: public ItemListEditorAction_FlagIndex
{
    public:
        ItemListEditorAction_Plantable (guint32 i, bool state)
          : ItemListEditorAction_FlagIndex (PLANTABLE, i, state) {}
        ~ItemListEditorAction_Plantable () {}

        Glib::ustring getActionName () const {return "Plantable";}
};
class ItemListEditorAction_Name: public ItemListEditorAction_Index, public UndoCursor
{
    public:
        ItemListEditorAction_Name (guint32 i, Glib::ustring n, UndoMgr *u,
                                   Gtk::Entry *e)
          : ItemListEditorAction_Index (NAME, i, true), UndoCursor (u, e),
          d_name (n) {}
        ~ItemListEditorAction_Name () {}

        Glib::ustring getActionName () const {return "Name";}

        Glib::ustring getName () {return d_name;}

    private:
        Glib::ustring d_name;
};

class ItemListEditorAction_Army: public ItemListEditorAction_Index
{
    public:
        ItemListEditorAction_Army (Type t, guint32 i, guint32 a, bool s)
          : ItemListEditorAction_Index (t, i), d_army_type (a), d_present (s) {}
        ~ItemListEditorAction_Army () {}

        bool getPresent () const {return d_present;}
        guint32 getArmyType () const {return d_army_type;}
    private:
        guint32 d_army_type;
        bool d_present;
};
class ItemListEditorAction_SelectBanishArmy: public ItemListEditorAction_Army
{
    public:
        ItemListEditorAction_SelectBanishArmy (guint32 i, guint32 a, bool s)
          : ItemListEditorAction_Army (SELECT_BANISH_ARMY, i, a, s) {}
        ~ItemListEditorAction_SelectBanishArmy () {}

        Glib::ustring getActionName () const {return "SelectBanishArmy";}
};
class ItemListEditorAction_SelectSummonArmy: public ItemListEditorAction_Army
{
    public:
        ItemListEditorAction_SelectSummonArmy (guint32 i, guint32 a, bool s)
          : ItemListEditorAction_Army (SELECT_SUMMON_ARMY, i, a, s) {}
        ~ItemListEditorAction_SelectSummonArmy () {}

        Glib::ustring getActionName () const {return "SelectSummonArmy";}
};
class ItemListEditorAction_SelectRaiseArmy: public ItemListEditorAction_Army
{
    public:
        ItemListEditorAction_SelectRaiseArmy (guint32 i, guint32 a, bool s)
          : ItemListEditorAction_Army (SELECT_RAISE_ARMY, i, a, s) {}
        ~ItemListEditorAction_SelectRaiseArmy () {}

        Glib::ustring getActionName () const {return "SelectRaiseArmy";}
};
#endif //ITEMLIST_EDITOR_ACTIONS_H
