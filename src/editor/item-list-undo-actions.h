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
#ifndef ITEM_LIST_UNDO_ACTIONS_H
#define ITEM_LIST_UNDO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include "undo-mgr.h"

//! A record of an event in the itemlist editor
/** 
 * The purpose of these classes is to implement undo/redo in the itemlist
 * editor.
 */

class ItemListUndoAction: public UndoAction
{
public:

    enum Type
      {
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
        PICK_UP_BAGS = 16,
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

    ItemListUndoAction (Type type, bool agg = false)
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

class ItemListUndoAction_Save : public ItemListUndoAction
{
public:
    ItemListUndoAction_Save (Type t, Itemlist *i)
      :ItemListUndoAction (t, false), m_itemlist (i)
      {
      }

    ~ItemListUndoAction_Save ()
      {
        if (m_itemlist)
          delete m_itemlist;
      }

    Itemlist * get_item_list () const
      {
        return m_itemlist;
      }
private:
    Itemlist *m_itemlist;
};

class ItemListUndoAction_Add: public ItemListUndoAction_Save
{
public:
    ItemListUndoAction_Add (Itemlist *i, int idx)
      :ItemListUndoAction_Save (ADD, i), m_idx (idx)
      {
      }

    ~ItemListUndoAction_Add ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Add";
      }

    int get_index () const
      {
        return m_idx;
      }
private:
    int m_idx;
};

class ItemListUndoAction_Remove: public ItemListUndoAction_Save
{
public:
    ItemListUndoAction_Remove (Itemlist *i, int idx)
      :ItemListUndoAction_Save (REMOVE, i), m_idx (idx)
      {
      }

    ~ItemListUndoAction_Remove ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Remove";
      }

    int get_index () const
      {
        return m_idx;
      }
private:
    int m_idx;
};

class ItemListUndoAction_Index: public ItemListUndoAction
{
public:
    ItemListUndoAction_Index (Type t, guint32 i, bool agg = false)
      : ItemListUndoAction (t, agg), m_index (i)
      {
      }

    ~ItemListUndoAction_Index ()
      {
      }

    guint32 get_index ()
      {
        return m_index;
      }
private:
    guint32 m_index;
};

class ItemListUndoAction_BuildingTypeToSummonOn: public ItemListUndoAction_Index
{
public:
    ItemListUndoAction_BuildingTypeToSummonOn (guint32 i, int row)
      : ItemListUndoAction_Index (BUILDING_TYPE_TO_SUMMON_ON, i, false),
      m_row (row)
  {
  }
    ~ItemListUndoAction_BuildingTypeToSummonOn ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Building Type To Summon On";
      }

    int get_row () const
      {
        return m_row;
      }
private:
    int m_row;
};

class ItemListUndoAction_FlagIndex: public ItemListUndoAction_Index
{
public:
    ItemListUndoAction_FlagIndex (Type t, guint32 i, bool state)
      : ItemListUndoAction_Index (t, i), m_active (state)
      {
      }

    ~ItemListUndoAction_FlagIndex ()
      {
      }

    bool get_active () const
      {
        return m_active;
      }

private:
    bool m_active;
};

class ItemListUndoAction_DiseaseCity: public ItemListUndoAction_FlagIndex
{
public:
    ItemListUndoAction_DiseaseCity (guint32 i, bool state)
      : ItemListUndoAction_FlagIndex (DISEASE_CITY, i, state)
      {
      }

    ~ItemListUndoAction_DiseaseCity ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Disease City";
      }
};

class ItemListUndoAction_SpinIndex: public ItemListUndoAction_Index
{
public:
    ItemListUndoAction_SpinIndex (Type t, guint32 i, int num)
      : ItemListUndoAction_Index (t, i, true),
      m_num (num)
  {
  }

    ~ItemListUndoAction_SpinIndex ()
      {
      }

    int get_num () const
      {
        return m_num;
      }

private:
    int m_num;
};

class ItemListUndoAction_DiseaseArmies: public ItemListUndoAction_SpinIndex
{
public:
    ItemListUndoAction_DiseaseArmies (guint32 i, int num)
      : ItemListUndoAction_SpinIndex (DISEASE_ARMIES_PERCENT, i, num)
      {
      }

    ~ItemListUndoAction_DiseaseArmies ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Disease Armies";
      }
};

class ItemListUndoAction_RaiseDefenders: public ItemListUndoAction_FlagIndex
{
public:
    ItemListUndoAction_RaiseDefenders (guint32 i, bool state)
      : ItemListUndoAction_FlagIndex (RAISE_DEFENDERS, i, state)
      {
      }

    ~ItemListUndoAction_RaiseDefenders ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Raise Defenders";
      }
};

class ItemListUndoAction_NumDefenders: public ItemListUndoAction_SpinIndex
{
public:
    ItemListUndoAction_NumDefenders (guint32 i, int num)
      : ItemListUndoAction_SpinIndex (NUM_DEFENDERS, i, num)
      {
      }

    ~ItemListUndoAction_NumDefenders ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Number of Defenders";
      }
};
class ItemListUndoAction_PersuadeNeutralCity: public ItemListUndoAction_FlagIndex
{
public:
    ItemListUndoAction_PersuadeNeutralCity (guint32 i, bool state)
      : ItemListUndoAction_FlagIndex (PERSUADE_NEUTRAL_CITY, i, state)
      {
      }

    ~ItemListUndoAction_PersuadeNeutralCity ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Persuade Neutral City";
      }
};

class ItemListUndoAction_TeleportToCity: public ItemListUndoAction_FlagIndex
{
public:
    ItemListUndoAction_TeleportToCity (guint32 i, bool state)
      : ItemListUndoAction_FlagIndex (TELEPORT_TO_CITY, i, state)
      {
      }

    ~ItemListUndoAction_TeleportToCity ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Teleport To City";
      }
};

class ItemListUndoAction_AddStr: public ItemListUndoAction_Index
{
public:
    ItemListUndoAction_AddStr (guint32 i, int num, bool state)
      : ItemListUndoAction_Index (ADDSTR, i), m_num (num),
      m_active (state)
  {
  }
    ~ItemListUndoAction_AddStr ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "AddStr";
      }

    int get_num () const
      {
        return m_num;
      }

    bool get_active () const
      {
        return m_active;
      }

private:
    int m_num;
    bool m_active;
};

class ItemListUndoAction_AddStack: public ItemListUndoAction_Index
{
public:
    ItemListUndoAction_AddStack (guint32 i, int num, bool state)
      : ItemListUndoAction_Index (ADDSTACK, i), m_num (num),
      m_active (state)
  {
  }

    ~ItemListUndoAction_AddStack ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Add Stack";
      }

    int get_num () const
      {
        return m_num;
      }

    bool get_active () const
      {
        return m_active;
      }

private:
    int m_num;
    bool m_active;
};

class ItemListUndoAction_FlyStack: public ItemListUndoAction_FlagIndex
{
public:
    ItemListUndoAction_FlyStack (guint32 i, bool state)
      : ItemListUndoAction_FlagIndex (FLY_STACK, i, state)
      {
      }

    ~ItemListUndoAction_FlyStack ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Fly Stack";
      }
};

class ItemListUndoAction_DoubleMovementStack: public ItemListUndoAction_FlagIndex
{
public:
    ItemListUndoAction_DoubleMovementStack (guint32 i, bool state)
      : ItemListUndoAction_FlagIndex (DOUBLE_MOVEMENT_STACK, i, state)
      {
      }

    ~ItemListUndoAction_DoubleMovementStack ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Double Movement Stack";
      }
};

class ItemListUndoAction_AddGoldPerCity: public ItemListUndoAction_Index
{
public:
    ItemListUndoAction_AddGoldPerCity (guint32 i, int num, bool state)
      : ItemListUndoAction_Index (ADD_GOLD_PER_CITY, i), m_num (num),
      m_active (state)
  {
  }

    ~ItemListUndoAction_AddGoldPerCity ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Add Gold Per City";
      }

    int get_num () const
      {
        return m_num;
      }

    bool get_active () const
      {
        return m_active;
      }
private:
    int m_num;
    bool m_active;
};

class ItemListUndoAction_StealsGold: public ItemListUndoAction_FlagIndex
{
public:
    ItemListUndoAction_StealsGold (guint32 i, bool state)
      : ItemListUndoAction_FlagIndex (STEALS_GOLD, i, state)
      {
      }

    ~ItemListUndoAction_StealsGold ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Steals Gold";
      }
};

class ItemListUndoAction_PickUpBags: public ItemListUndoAction_FlagIndex
{
public:
    ItemListUndoAction_PickUpBags (guint32 i, bool state)
      : ItemListUndoAction_FlagIndex (PICK_UP_BAGS, i, state)
      {
      }

    ~ItemListUndoAction_PickUpBags ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Pick Up Bags";
      }
};

class ItemListUndoAction_AddMovement: public ItemListUndoAction_FlagIndex
{
public:
    ItemListUndoAction_AddMovement (guint32 i, bool state)
      : ItemListUndoAction_FlagIndex (ADD_MOVEMENT, i, state)
      {
      }

    ~ItemListUndoAction_AddMovement ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Add Movement";
      }
};

class ItemListUndoAction_SinkShips: public ItemListUndoAction_FlagIndex
{
public:
    ItemListUndoAction_SinkShips (guint32 i, bool state)
      : ItemListUndoAction_FlagIndex (SINKS_SHIPS, i, state)
      {
      }

    ~ItemListUndoAction_SinkShips ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Sink Ships";
      }
};

class ItemListUndoAction_BanishWorms: public ItemListUndoAction_FlagIndex
{
public:
    ItemListUndoAction_BanishWorms (guint32 i, bool state)
      : ItemListUndoAction_FlagIndex (BANISH_WORMS, i, state)
      {
      }

    ~ItemListUndoAction_BanishWorms ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Kill All Giant Worms";
      }
};

class ItemListUndoAction_BurnBridge: public ItemListUndoAction_FlagIndex
{
public:
    ItemListUndoAction_BurnBridge (guint32 i, bool state)
      : ItemListUndoAction_FlagIndex (BURN_BRIDGE, i, state)
      {
      }

    ~ItemListUndoAction_BurnBridge ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Burn Bridge";
      }
};

class ItemListUndoAction_CaptureKeeper: public ItemListUndoAction_FlagIndex
{
public:
    ItemListUndoAction_CaptureKeeper (guint32 i, bool state)
      : ItemListUndoAction_FlagIndex (CAPTURE_KEEPER, i, state)
      {
      }

    ~ItemListUndoAction_CaptureKeeper ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Capture Keeper";
      }
};

class ItemListUndoAction_SummonMonster: public ItemListUndoAction_FlagIndex
{
public:
    ItemListUndoAction_SummonMonster (guint32 i, bool state)
      : ItemListUndoAction_FlagIndex (SUMMON_MONSTER, i, state)
      {
      }

    ~ItemListUndoAction_SummonMonster ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Summon Monster";
      }
};

class ItemListUndoAction_Uses: public ItemListUndoAction_SpinIndex
{
public:
    ItemListUndoAction_Uses (guint32 i, int num)
      : ItemListUndoAction_SpinIndex (USES, i, num)
      {
      }

    ~ItemListUndoAction_Uses ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Number of Uses";
      }
};

class ItemListUndoAction_StealPercent: public ItemListUndoAction_SpinIndex
{
public:
    ItemListUndoAction_StealPercent (guint32 i, int num)
      : ItemListUndoAction_SpinIndex (STEAL_PERCENT, i, num)
      {
      }

    ~ItemListUndoAction_StealPercent ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Steal Percent";
      }
};

class ItemListUndoAction_AddMp: public ItemListUndoAction_SpinIndex
{
public:
    ItemListUndoAction_AddMp (guint32 i, int num)
      : ItemListUndoAction_SpinIndex (ADD_MP, i, num)
      {
      }

    ~ItemListUndoAction_AddMp ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Add Movement Points";
      }
};

class ItemListUndoAction_Plantable: public ItemListUndoAction_FlagIndex
{
public:
    ItemListUndoAction_Plantable (guint32 i, bool state)
      : ItemListUndoAction_FlagIndex (PLANTABLE, i, state)
      {
      }

    ~ItemListUndoAction_Plantable ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Plantable";
      }
};

class ItemListUndoAction_Name: public ItemListUndoAction_Index,
    public UndoCursor
{
public:
    ItemListUndoAction_Name (guint32 i, Glib::ustring n, UndoMgr *u,
                             Gtk::Entry *e)
      : ItemListUndoAction_Index (NAME, i, true),
      UndoCursor (u->get_pos (e), e), m_name (n)
  {
  }

    ~ItemListUndoAction_Name ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Name";
      }

    Glib::ustring get_name () const
      {
        return m_name;
      }

private:
    Glib::ustring m_name;
};

class ItemListUndoAction_Army: public ItemListUndoAction_Index
{
public:
    ItemListUndoAction_Army (Type t, guint32 i, guint32 a, bool s)
      : ItemListUndoAction_Index (t, i), m_army_type (a), m_present (s)
      {
      }

    ~ItemListUndoAction_Army ()
      {
      }

    bool get_present () const
      {
        return m_present;
      }

    guint32 get_army_type () const
      {
        return m_army_type;
      }
private:
    guint32 m_army_type;
    bool m_present;
};

class ItemListUndoAction_SelectBanishArmy: public ItemListUndoAction_Army
{
public:
    ItemListUndoAction_SelectBanishArmy (guint32 i, guint32 a, bool s)
      : ItemListUndoAction_Army (SELECT_BANISH_ARMY, i, a, s)
      {
      }

    ~ItemListUndoAction_SelectBanishArmy ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Select Army To Kill";
      }
};

class ItemListUndoAction_SelectSummonArmy: public ItemListUndoAction_Army
{
public:
    ItemListUndoAction_SelectSummonArmy (guint32 i, guint32 a, bool s)
      : ItemListUndoAction_Army (SELECT_SUMMON_ARMY, i, a, s)
      {
      }

    ~ItemListUndoAction_SelectSummonArmy ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Select Army To Summon";
      }
};

class ItemListUndoAction_SelectRaiseArmy: public ItemListUndoAction_Army
{
public:
    ItemListUndoAction_SelectRaiseArmy (guint32 i, guint32 a, bool s)
      : ItemListUndoAction_Army (SELECT_RAISE_ARMY, i, a, s)
      {
      }

    ~ItemListUndoAction_SelectRaiseArmy ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Select Army To Raise";
      }
};
#endif
