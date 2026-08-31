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
#ifndef ITEM_BONUS_EDITOR_UNDO_ACTIONS_H
#define ITEM_BONUS_EDITOR_UNDO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include "undo-mgr.h"

//! A record of an event in the item bonus editor
/** 
 * The purpose of these classes is to implement undo/redo in the item bonus
 * editor.
 */

class ItemBonusEditorUndoAction: public UndoAction
{
public:

    enum Type
      {
        BUILDING_TYPE_TO_SUMMON_ON = 1,
        DISEASE_CITY = 2,
        DISEASE_ARMIES_PERCENT = 3,
        RAISE_DEFENDERS = 4,
        NUM_DEFENDERS = 5,
        PERSUADE_NEUTRAL_CITY = 6,
        TELEPORT_TO_CITY = 7,
        ADDSTR = 8,
        ADDSTACK = 9,
        FLY_STACK = 10,
        DOUBLE_MOVEMENT_STACK = 11,
        ADD_GOLD_PER_CITY = 12,
        STEALS_GOLD = 13,
        PICK_UP_BAGS = 14,
        ADD_MOVEMENT = 15,
        SINKS_SHIPS = 16,
        BANISH_WORMS = 17,
        BURN_BRIDGE = 18,
        CAPTURE_KEEPER = 19,
        SUMMON_MONSTER = 20,
        STEAL_PERCENT = 21,
        ADD_MP = 22,
        PLANTABLE = 23,
        SELECT_BANISH_ARMY = 24,
        SELECT_SUMMON_ARMY = 25,
        SELECT_RAISE_ARMY = 26,
      };

    ItemBonusEditorUndoAction (Type type, bool agg = false)
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

class ItemBonusEditorUndoAction_Save: public ItemBonusEditorUndoAction
{
public:
    ItemBonusEditorUndoAction_Save (Type t, Item *i, bool agg = false)
      : ItemBonusEditorUndoAction (t, agg), m_item (new Item (*i, true))
      {
      }

    ~ItemBonusEditorUndoAction_Save ()
      {
        delete m_item;
      }

    Item * get_item ()
      {
        return m_item;
      }
private:
    Item *m_item;
};

class ItemBonusEditorUndoAction_BuildingTypeToSummonOn: public ItemBonusEditorUndoAction_Save
{
public:
    ItemBonusEditorUndoAction_BuildingTypeToSummonOn (Item *i, int row)
      : ItemBonusEditorUndoAction_Save (BUILDING_TYPE_TO_SUMMON_ON, i, false),
      m_row (row)
  {
  }
    ~ItemBonusEditorUndoAction_BuildingTypeToSummonOn ()
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

class ItemBonusEditorUndoAction_Flag: public ItemBonusEditorUndoAction_Save
{
public:
    ItemBonusEditorUndoAction_Flag (Type t, Item *i, bool state)
      : ItemBonusEditorUndoAction_Save (t, i), m_active (state)
      {
      }

    ~ItemBonusEditorUndoAction_Flag ()
      {
      }

    bool get_active () const
      {
        return m_active;
      }

private:
    bool m_active;
};

class ItemBonusEditorUndoAction_DiseaseCity: public ItemBonusEditorUndoAction_Flag
{
public:
    ItemBonusEditorUndoAction_DiseaseCity (Item *i, bool state)
      : ItemBonusEditorUndoAction_Flag (DISEASE_CITY, i, state)
      {
      }

    ~ItemBonusEditorUndoAction_DiseaseCity ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Disease City";
      }
};

class ItemBonusEditorUndoAction_Spin: public ItemBonusEditorUndoAction_Save
{
public:
    ItemBonusEditorUndoAction_Spin (Type t, Item *i, int num)
      : ItemBonusEditorUndoAction_Save (t, i),
      m_num (num)
  {
  }

    ~ItemBonusEditorUndoAction_Spin ()
      {
      }

    int get_num () const
      {
        return m_num;
      }

private:
    int m_num;
};

class ItemBonusEditorUndoAction_DiseaseArmies: public ItemBonusEditorUndoAction_Spin
{
public:
    ItemBonusEditorUndoAction_DiseaseArmies (Item *i, int num)
      : ItemBonusEditorUndoAction_Spin (DISEASE_ARMIES_PERCENT, i, num)
      {
      }

    ~ItemBonusEditorUndoAction_DiseaseArmies ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Disease Armies";
      }
};

class ItemBonusEditorUndoAction_RaiseDefenders: public ItemBonusEditorUndoAction_Flag
{
public:
    ItemBonusEditorUndoAction_RaiseDefenders (Item * i, bool state)
      : ItemBonusEditorUndoAction_Flag (RAISE_DEFENDERS, i, state)
      {
      }

    ~ItemBonusEditorUndoAction_RaiseDefenders ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Raise Defenders";
      }
};

class ItemBonusEditorUndoAction_NumDefenders: public ItemBonusEditorUndoAction_Spin
{
public:
    ItemBonusEditorUndoAction_NumDefenders (Item * i, int num)
      : ItemBonusEditorUndoAction_Spin (NUM_DEFENDERS, i, num)
      {
      }

    ~ItemBonusEditorUndoAction_NumDefenders ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Number of Defenders";
      }
};
class ItemBonusEditorUndoAction_PersuadeNeutralCity: public ItemBonusEditorUndoAction_Flag
{
public:
    ItemBonusEditorUndoAction_PersuadeNeutralCity (Item * i, bool state)
      : ItemBonusEditorUndoAction_Flag (PERSUADE_NEUTRAL_CITY, i, state)
      {
      }

    ~ItemBonusEditorUndoAction_PersuadeNeutralCity ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Persuade Neutral City";
      }
};

class ItemBonusEditorUndoAction_TeleportToCity: public ItemBonusEditorUndoAction_Flag
{
public:
    ItemBonusEditorUndoAction_TeleportToCity (Item * i, bool state)
      : ItemBonusEditorUndoAction_Flag (TELEPORT_TO_CITY, i, state)
      {
      }

    ~ItemBonusEditorUndoAction_TeleportToCity ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Teleport To City";
      }
};

class ItemBonusEditorUndoAction_AddStr: public ItemBonusEditorUndoAction_Save
{
public:
    ItemBonusEditorUndoAction_AddStr (Item * i, int num, bool state)
      : ItemBonusEditorUndoAction_Save (ADDSTR, i), m_num (num),
      m_active (state)
  {
  }
    ~ItemBonusEditorUndoAction_AddStr ()
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

class ItemBonusEditorUndoAction_AddStack: public ItemBonusEditorUndoAction_Save
{
public:
    ItemBonusEditorUndoAction_AddStack (Item * i, int num, bool state)
      : ItemBonusEditorUndoAction_Save (ADDSTACK, i), m_num (num),
      m_active (state)
  {
  }

    ~ItemBonusEditorUndoAction_AddStack ()
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

class ItemBonusEditorUndoAction_FlyStack: public ItemBonusEditorUndoAction_Flag
{
public:
    ItemBonusEditorUndoAction_FlyStack (Item * i, bool state)
      : ItemBonusEditorUndoAction_Flag (FLY_STACK, i, state)
      {
      }

    ~ItemBonusEditorUndoAction_FlyStack ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Fly Stack";
      }
};

class ItemBonusEditorUndoAction_DoubleMovementStack: public ItemBonusEditorUndoAction_Flag
{
public:
    ItemBonusEditorUndoAction_DoubleMovementStack (Item * i, bool state)
      : ItemBonusEditorUndoAction_Flag (DOUBLE_MOVEMENT_STACK, i, state)
      {
      }

    ~ItemBonusEditorUndoAction_DoubleMovementStack ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Double Movement Stack";
      }
};

class ItemBonusEditorUndoAction_AddGoldPerCity: public ItemBonusEditorUndoAction_Save
{
public:
    ItemBonusEditorUndoAction_AddGoldPerCity (Item * i, int num, bool state)
      : ItemBonusEditorUndoAction_Save (ADD_GOLD_PER_CITY, i), m_num (num),
      m_active (state)
  {
  }

    ~ItemBonusEditorUndoAction_AddGoldPerCity ()
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

class ItemBonusEditorUndoAction_StealsGold: public ItemBonusEditorUndoAction_Flag
{
public:
    ItemBonusEditorUndoAction_StealsGold (Item * i, bool state)
      : ItemBonusEditorUndoAction_Flag (STEALS_GOLD, i, state)
      {
      }

    ~ItemBonusEditorUndoAction_StealsGold ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Steals Gold";
      }
};

class ItemBonusEditorUndoAction_PickUpBags: public ItemBonusEditorUndoAction_Flag
{
public:
    ItemBonusEditorUndoAction_PickUpBags (Item * i, bool state)
      : ItemBonusEditorUndoAction_Flag (PICK_UP_BAGS, i, state)
      {
      }

    ~ItemBonusEditorUndoAction_PickUpBags ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Pick Up Bags";
      }
};

class ItemBonusEditorUndoAction_AddMovement: public ItemBonusEditorUndoAction_Flag
{
public:
    ItemBonusEditorUndoAction_AddMovement (Item * i, bool state)
      : ItemBonusEditorUndoAction_Flag (ADD_MOVEMENT, i, state)
      {
      }

    ~ItemBonusEditorUndoAction_AddMovement ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Add Movement";
      }
};

class ItemBonusEditorUndoAction_SinkShips: public ItemBonusEditorUndoAction_Flag
{
public:
    ItemBonusEditorUndoAction_SinkShips (Item * i, bool state)
      : ItemBonusEditorUndoAction_Flag (SINKS_SHIPS, i, state)
      {
      }

    ~ItemBonusEditorUndoAction_SinkShips ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Sink Ships";
      }
};

class ItemBonusEditorUndoAction_BanishWorms: public ItemBonusEditorUndoAction_Flag
{
public:
    ItemBonusEditorUndoAction_BanishWorms (Item * i, bool state)
      : ItemBonusEditorUndoAction_Flag (BANISH_WORMS, i, state)
      {
      }

    ~ItemBonusEditorUndoAction_BanishWorms ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Kill All Giant Worms";
      }
};

class ItemBonusEditorUndoAction_BurnBridge: public ItemBonusEditorUndoAction_Flag
{
public:
    ItemBonusEditorUndoAction_BurnBridge (Item * i, bool state)
      : ItemBonusEditorUndoAction_Flag (BURN_BRIDGE, i, state)
      {
      }

    ~ItemBonusEditorUndoAction_BurnBridge ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Burn Bridge";
      }
};

class ItemBonusEditorUndoAction_CaptureKeeper: public ItemBonusEditorUndoAction_Flag
{
public:
    ItemBonusEditorUndoAction_CaptureKeeper (Item * i, bool state)
      : ItemBonusEditorUndoAction_Flag (CAPTURE_KEEPER, i, state)
      {
      }

    ~ItemBonusEditorUndoAction_CaptureKeeper ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Capture Keeper";
      }
};

class ItemBonusEditorUndoAction_SummonMonster: public ItemBonusEditorUndoAction_Flag
{
public:
    ItemBonusEditorUndoAction_SummonMonster (Item * i, bool state)
      : ItemBonusEditorUndoAction_Flag (SUMMON_MONSTER, i, state)
      {
      }

    ~ItemBonusEditorUndoAction_SummonMonster ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Summon Monster";
      }
};

class ItemBonusEditorUndoAction_StealPercent: public ItemBonusEditorUndoAction_Spin
{
public:
    ItemBonusEditorUndoAction_StealPercent (Item * i, int num)
      : ItemBonusEditorUndoAction_Spin (STEAL_PERCENT, i, num)
      {
      }

    ~ItemBonusEditorUndoAction_StealPercent ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Steal Percent";
      }
};

class ItemBonusEditorUndoAction_AddMp: public ItemBonusEditorUndoAction_Spin
{
public:
    ItemBonusEditorUndoAction_AddMp (Item * i, int num)
      : ItemBonusEditorUndoAction_Spin (ADD_MP, i, num)
      {
      }

    ~ItemBonusEditorUndoAction_AddMp ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Add Movement Points";
      }
};

class ItemBonusEditorUndoAction_Plantable: public ItemBonusEditorUndoAction_Flag
{
public:
    ItemBonusEditorUndoAction_Plantable (Item * i, bool state)
      : ItemBonusEditorUndoAction_Flag (PLANTABLE, i, state)
      {
      }

    ~ItemBonusEditorUndoAction_Plantable ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Plantable";
      }
};

class ItemBonusEditorUndoAction_Army: public ItemBonusEditorUndoAction_Save
{
public:
    ItemBonusEditorUndoAction_Army (Type t, Item * i, guint32 a, bool s)
      : ItemBonusEditorUndoAction_Save (t, i), m_army_type (a), m_present (s)
      {
      }

    ~ItemBonusEditorUndoAction_Army ()
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

class ItemBonusEditorUndoAction_SelectBanishArmy: public ItemBonusEditorUndoAction_Army
{
public:
    ItemBonusEditorUndoAction_SelectBanishArmy (Item * i, guint32 a, bool s)
      : ItemBonusEditorUndoAction_Army (SELECT_BANISH_ARMY, i, a, s)
      {
      }

    ~ItemBonusEditorUndoAction_SelectBanishArmy ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Select Army To Kill";
      }
};

class ItemBonusEditorUndoAction_SelectSummonArmy: public ItemBonusEditorUndoAction_Army
{
public:
    ItemBonusEditorUndoAction_SelectSummonArmy (Item * i, guint32 a, bool s)
      : ItemBonusEditorUndoAction_Army (SELECT_SUMMON_ARMY, i, a, s)
      {
      }

    ~ItemBonusEditorUndoAction_SelectSummonArmy ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Select Army To Summon";
      }
};

class ItemBonusEditorUndoAction_SelectRaiseArmy: public ItemBonusEditorUndoAction_Army
{
public:
    ItemBonusEditorUndoAction_SelectRaiseArmy (Item * i, guint32 a, bool s)
      : ItemBonusEditorUndoAction_Army (SELECT_RAISE_ARMY, i, a, s)
      {
      }

    ~ItemBonusEditorUndoAction_SelectRaiseArmy ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Select Army To Raise";
      }
};
#endif
