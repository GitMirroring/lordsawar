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
#ifndef BATTLE_CALCULATOR_UNDO_ACTIONS_H
#define BATTLE_CALCULATOR_UNDO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include "undo-mgr.h"
#include "army.h"
#include "hero.h"

//! A record of an event in the battle calculator
/**
 * The purpose of these classes is to implement undo/redo in the battle
 * calculator.
 */

class BattleCalculatorUndoAction: public UndoAction
{
public:

    enum Type
      {
        SIDES = 1,
        TERRAIN = 2,
        CITY = 3,
        FORTIFY = 4,
        DEFENDER_ADD = 5,
        DEFENDER_REMOVE = 6,
        DEFENDER_COPY = 7,
        DEFENDER_HERO_DETAILS = 8,
        DEFENDER_STRENGTH = 9,
        ATTACKER_ADD = 10,
        ATTACKER_REMOVE = 11,
        ATTACKER_COPY = 12,
        ATTACKER_HERO_DETAILS = 13,
        ATTACKER_STRENGTH = 14,
      };

    BattleCalculatorUndoAction (Type type, bool agg = false)
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

class BattleCalculatorUndoAction_Row
{
public:
    BattleCalculatorUndoAction_Row (int r)
      : m_row (r)
      {
      }

    ~BattleCalculatorUndoAction_Row ()
      {
      }

    int get_row () const
      {
        return m_row;
      }
private:
    int m_row;
};

class BattleCalculatorUndoAction_Active
{
public:
    BattleCalculatorUndoAction_Active (bool state)
      : m_active (state)
      {
      }

    ~BattleCalculatorUndoAction_Active ()
      {
      }

    bool get_active () const
      {
        return m_active;
      }
private:
    bool m_active;
};

class BattleCalculatorUndoAction_Sides: public BattleCalculatorUndoAction,
    public BattleCalculatorUndoAction_Row
{
public:
    BattleCalculatorUndoAction_Sides (int r)
      : BattleCalculatorUndoAction (SIDES), BattleCalculatorUndoAction_Row (r)
      {
      }

    ~BattleCalculatorUndoAction_Sides ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Sides";
      }
};

class BattleCalculatorUndoAction_Terrain: public BattleCalculatorUndoAction, public BattleCalculatorUndoAction_Row
{
public:
    BattleCalculatorUndoAction_Terrain (int r)
      : BattleCalculatorUndoAction (TERRAIN), BattleCalculatorUndoAction_Row (r)
      {
      }

    ~BattleCalculatorUndoAction_Terrain ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Terrain";
      }
};

class BattleCalculatorUndoAction_City: public BattleCalculatorUndoAction,
    public BattleCalculatorUndoAction_Active
{
public:
    BattleCalculatorUndoAction_City (bool state)
      : BattleCalculatorUndoAction (CITY),
      BattleCalculatorUndoAction_Active (state)
  {
  }

    ~BattleCalculatorUndoAction_City ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "City";
      }
};

class BattleCalculatorUndoAction_Fortify: public BattleCalculatorUndoAction,
    public BattleCalculatorUndoAction_Active
{
public:
    BattleCalculatorUndoAction_Fortify (bool s)
      : BattleCalculatorUndoAction (FORTIFY),
      BattleCalculatorUndoAction_Active (s)
  {
  }

    ~BattleCalculatorUndoAction_Fortify ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Fortify";
      }
};

class BattleCalculatorUndoAction_Armies: public BattleCalculatorUndoAction
{
public:
    BattleCalculatorUndoAction_Armies (Type t, std::list<Army*> armies)
      : BattleCalculatorUndoAction (t)
      {
        for (auto a : armies)
          {
            if (a->isHero ())
              {
                Hero *h = dynamic_cast<Hero*>(a);
                m_armies.push_back (new Hero (*h));
              }
            else
              m_armies.push_back (new Army (*a));
          }
      }

    ~BattleCalculatorUndoAction_Armies ()
      {
        for (auto a : m_armies)
          delete a;
        m_armies.clear ();
      }

    std::list<Army*> get_armies () const
      {
        return m_armies;
      }
private:
    std::list<Army*> m_armies;
};

class BattleCalculatorUndoAction_DefenderAdd:
    public BattleCalculatorUndoAction_Armies
{
public:
    BattleCalculatorUndoAction_DefenderAdd (std::list<Army*> armies)
      : BattleCalculatorUndoAction_Armies (DEFENDER_ADD, armies)
      {
      }

    ~BattleCalculatorUndoAction_DefenderAdd ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "DefenderAdd";
      }
};

class BattleCalculatorUndoAction_DefenderRemove:
    public BattleCalculatorUndoAction_Armies
{
public:
    BattleCalculatorUndoAction_DefenderRemove (std::list<Army*> armies)
      : BattleCalculatorUndoAction_Armies (DEFENDER_REMOVE, armies)
      {
      }

    ~BattleCalculatorUndoAction_DefenderRemove ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "DefenderRemove";
      }
};

class BattleCalculatorUndoAction_DefenderCopy:
    public BattleCalculatorUndoAction_Armies
{
public:
    BattleCalculatorUndoAction_DefenderCopy (std::list<Army*> armies)
      : BattleCalculatorUndoAction_Armies (DEFENDER_COPY, armies)
      {
      }

    ~BattleCalculatorUndoAction_DefenderCopy ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "DefenderCopy";
      }
};

class BattleCalculatorUndoAction_DefenderHeroDetails:
    public BattleCalculatorUndoAction_Armies
{
public:
    BattleCalculatorUndoAction_DefenderHeroDetails (std::list<Army*> armies)
      : BattleCalculatorUndoAction_Armies (DEFENDER_HERO_DETAILS, armies)
      {
      }

    ~BattleCalculatorUndoAction_DefenderHeroDetails ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "DefenderHeroDetails";
      }
};

class BattleCalculatorUndoAction_AttackerAdd:
    public BattleCalculatorUndoAction_Armies
{
public:
    BattleCalculatorUndoAction_AttackerAdd (std::list<Army*> armies)
      : BattleCalculatorUndoAction_Armies (ATTACKER_ADD, armies)
      {
      }

    ~BattleCalculatorUndoAction_AttackerAdd ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "AttackerAdd";
      }
};

class BattleCalculatorUndoAction_AttackerRemove:
    public BattleCalculatorUndoAction_Armies
{
public:
    BattleCalculatorUndoAction_AttackerRemove (std::list<Army*> armies)
      : BattleCalculatorUndoAction_Armies (ATTACKER_REMOVE, armies)
      {
      }

    ~BattleCalculatorUndoAction_AttackerRemove ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "AttackerRemove";
      }
};

class BattleCalculatorUndoAction_AttackerCopy:
    public BattleCalculatorUndoAction_Armies
{
public:
    BattleCalculatorUndoAction_AttackerCopy (std::list<Army*> armies)
      : BattleCalculatorUndoAction_Armies (ATTACKER_COPY, armies)
      {
      }

    ~BattleCalculatorUndoAction_AttackerCopy ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "AttackerCopy";
      }
};

class BattleCalculatorUndoAction_AttackerHeroDetails:
    public BattleCalculatorUndoAction_Armies
{
public:
    BattleCalculatorUndoAction_AttackerHeroDetails (std::list<Army*> armies)
      : BattleCalculatorUndoAction_Armies (ATTACKER_HERO_DETAILS, armies)
      {
      }

    ~BattleCalculatorUndoAction_AttackerHeroDetails ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "AttackerHeroDetails";
      }
};

class BattleCalculatorUndoAction_Index: public BattleCalculatorUndoAction
{
public:
    BattleCalculatorUndoAction_Index (Type t, guint32 i, bool agg = false)
      : BattleCalculatorUndoAction (t, agg), m_index (i)
      {
      }

    ~BattleCalculatorUndoAction_Index ()
      {
      }

    guint32 get_index () const
      {
        return m_index;
      }
private:
    guint32 m_index;
};

class BattleCalculatorUndoAction_DefenderStrength:
    public BattleCalculatorUndoAction_Index
{
public:
    BattleCalculatorUndoAction_DefenderStrength (guint32 i, guint32 str)
      : BattleCalculatorUndoAction_Index (DEFENDER_STRENGTH, i, true),
      m_str (str)
  {
  }

    ~BattleCalculatorUndoAction_DefenderStrength ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "DefenderStrength";
      }

    guint32 get_strength () const
      {
        return m_str;
      }
private:
    guint32 m_str;
};

class BattleCalculatorUndoAction_AttackerStrength:
    public BattleCalculatorUndoAction_Index
{
public:
    BattleCalculatorUndoAction_AttackerStrength (guint32 i, guint32 str)
      : BattleCalculatorUndoAction_Index (ATTACKER_STRENGTH, i, true),
      m_str (str)
  {
  }

    ~BattleCalculatorUndoAction_AttackerStrength ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "AttackerStrength";
      }

    guint32 get_strength () const
      {
        return m_str;
      }
private:
    guint32 m_str;
};
#endif
