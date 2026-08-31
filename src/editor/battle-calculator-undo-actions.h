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
#ifndef BATTLE_CALCULATOR_ACTIONS_H
#define BATTLE_CALCULATOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include "undo-mgr.h"
#include "army.h"
#include "hero.h"

//! A record of an event in the hero editor
/** 
 * The purpose of these classes is to implement undo/redo in the battle
 * calculator.
 */

class BattleCalculatorAction: public UndoAction
{
public:

    enum Type {
      SIDES = 1,
      TERRAIN = 2,
      CITY = 3,
      FORTIFY = 4,
      DEFENDER_OWNER = 5,
      ATTACKER_OWNER = 6,
      DEFENDER_ADD = 7,
      DEFENDER_REMOVE = 8,
      DEFENDER_COPY = 9,
      DEFENDER_HERO_DETAILS = 10,
      DEFENDER_STRENGTH = 11,
      ATTACKER_ADD = 12,
      ATTACKER_REMOVE = 13,
      ATTACKER_COPY = 14,
      ATTACKER_HERO_DETAILS = 15,
      ATTACKER_STRENGTH = 16,
    };

    BattleCalculatorAction(Type type, bool agg = false)
     : UndoAction (agg ? UndoAction::AGGREGATE_DELAY :
                   UndoAction::AGGREGATE_NONE), d_type (type) {}

    Type getType() const {return d_type;}

protected:

    Type d_type;
};

class BattleCalculatorAction_Row
{
    public:
        BattleCalculatorAction_Row (int r)
          : d_row (r) {}
        ~BattleCalculatorAction_Row () {}

        int getRow () const {return d_row;}
    private:
        int d_row;
};

class BattleCalculatorAction_Active
{
    public:
        BattleCalculatorAction_Active (bool state)
          : d_active (state) {}
        ~BattleCalculatorAction_Active () {}

        bool getActive() const {return d_active;}
    private:
        bool d_active;
};

class BattleCalculatorAction_Sides: public BattleCalculatorAction, public BattleCalculatorAction_Row
{
    public:
        BattleCalculatorAction_Sides (int r)
          : BattleCalculatorAction (SIDES), BattleCalculatorAction_Row (r) {}
        ~BattleCalculatorAction_Sides () {}

        Glib::ustring getActionName () const {return "Sides";}
};

class BattleCalculatorAction_Terrain: public BattleCalculatorAction, public BattleCalculatorAction_Row
{
    public:
        BattleCalculatorAction_Terrain (int r)
          : BattleCalculatorAction (TERRAIN), BattleCalculatorAction_Row (r) {}
        ~BattleCalculatorAction_Terrain () {}

        Glib::ustring getActionName () const {return "Terrain";}
};

class BattleCalculatorAction_City: public BattleCalculatorAction, public BattleCalculatorAction_Row, public BattleCalculatorAction_Active
{
    public:
        BattleCalculatorAction_City (bool state, int terrain_row)
          : BattleCalculatorAction (CITY),
          BattleCalculatorAction_Row (terrain_row),
          BattleCalculatorAction_Active (state) {}
        ~BattleCalculatorAction_City () {}

        Glib::ustring getActionName () const {return "City";}
};

class BattleCalculatorAction_Fortify: public BattleCalculatorAction, public BattleCalculatorAction_Active
{
    public:
        BattleCalculatorAction_Fortify (bool s)
          : BattleCalculatorAction (FORTIFY), BattleCalculatorAction_Active (s)
          {}
        ~BattleCalculatorAction_Fortify () {}

        Glib::ustring getActionName () const {return "Fortify";}
};

class BattleCalculatorAction_DefenderOwner: public BattleCalculatorAction, public BattleCalculatorAction_Row
{
    public:
        BattleCalculatorAction_DefenderOwner (int r)
          : BattleCalculatorAction (DEFENDER_OWNER),
          BattleCalculatorAction_Row (r) {}
        ~BattleCalculatorAction_DefenderOwner () {}

        Glib::ustring getActionName () const {return "DefenderOwner";}
};

class BattleCalculatorAction_AttackerOwner: public BattleCalculatorAction, public BattleCalculatorAction_Row
{
    public:
        BattleCalculatorAction_AttackerOwner (int r)
          : BattleCalculatorAction (ATTACKER_OWNER),
          BattleCalculatorAction_Row (r) {}
        ~BattleCalculatorAction_AttackerOwner () {}

        Glib::ustring getActionName () const {return "AttackerOwner";}
};

class BattleCalculatorAction_Armies: public BattleCalculatorAction
{
    public:
        BattleCalculatorAction_Armies (Type t, std::list<Army*> armies)
          : BattleCalculatorAction (t)
          {
            for (auto a : armies)
              {
                if (a->isHero ())
                  {
                    Hero *h = dynamic_cast<Hero*>(a);
                    d_armies.push_back (new Hero (*h));
                  }
                else
                  d_armies.push_back (new Army (*a));
              }
          }
        ~BattleCalculatorAction_Armies ()
          {
            for (auto a : d_armies)
              delete a;
            d_armies.clear ();
          }
        std::list<Army*> getArmies () const {return d_armies;}
    private:
        std::list<Army*> d_armies;
};

class BattleCalculatorAction_DefenderAdd: public BattleCalculatorAction_Armies
{
public:
    BattleCalculatorAction_DefenderAdd (std::list<Army*> armies)
          : BattleCalculatorAction_Armies (DEFENDER_ADD, armies) {}
    ~BattleCalculatorAction_DefenderAdd () {}
    Glib::ustring getActionName () const {return "DefenderAdd";}
};

class BattleCalculatorAction_DefenderRemove: public BattleCalculatorAction_Armies
{
public:
    BattleCalculatorAction_DefenderRemove (std::list<Army*> armies)
          : BattleCalculatorAction_Armies (DEFENDER_REMOVE, armies) {}
    ~BattleCalculatorAction_DefenderRemove () {}
    Glib::ustring getActionName () const {return "DefenderRemove";}
};

class BattleCalculatorAction_DefenderCopy: public BattleCalculatorAction_Armies
{
public:
    BattleCalculatorAction_DefenderCopy (std::list<Army*> armies)
          : BattleCalculatorAction_Armies (DEFENDER_COPY, armies) {}
    ~BattleCalculatorAction_DefenderCopy () {}
    Glib::ustring getActionName () const {return "DefenderCopy";}
};

class BattleCalculatorAction_DefenderHeroDetails: public BattleCalculatorAction_Armies
{
public:
    BattleCalculatorAction_DefenderHeroDetails (std::list<Army*> armies)
          : BattleCalculatorAction_Armies (DEFENDER_HERO_DETAILS, armies) {}
    ~BattleCalculatorAction_DefenderHeroDetails () {}
    Glib::ustring getActionName () const {return "DefenderHeroDetails";}
};

class BattleCalculatorAction_AttackerAdd: public BattleCalculatorAction_Armies
{
public:
    BattleCalculatorAction_AttackerAdd (std::list<Army*> armies)
          : BattleCalculatorAction_Armies (ATTACKER_ADD, armies) {}
    ~BattleCalculatorAction_AttackerAdd () {}
    Glib::ustring getActionName () const {return "AttackerAdd";}
};

class BattleCalculatorAction_AttackerRemove: public BattleCalculatorAction_Armies
{
public:
    BattleCalculatorAction_AttackerRemove (std::list<Army*> armies)
          : BattleCalculatorAction_Armies (ATTACKER_REMOVE, armies) {}
    ~BattleCalculatorAction_AttackerRemove () {}
    Glib::ustring getActionName () const {return "AttackerRemove";}
};

class BattleCalculatorAction_AttackerCopy: public BattleCalculatorAction_Armies
{
public:
    BattleCalculatorAction_AttackerCopy (std::list<Army*> armies)
          : BattleCalculatorAction_Armies (ATTACKER_COPY, armies) {}
    ~BattleCalculatorAction_AttackerCopy () {}
    Glib::ustring getActionName () const {return "AttackerCopy";}
};

class BattleCalculatorAction_AttackerHeroDetails: public BattleCalculatorAction_Armies
{
public:
    BattleCalculatorAction_AttackerHeroDetails (std::list<Army*> armies)
          : BattleCalculatorAction_Armies (ATTACKER_HERO_DETAILS, armies) {}
    ~BattleCalculatorAction_AttackerHeroDetails () {}
    Glib::ustring getActionName () const {return "AttackerHeroDetails";}
};

class BattleCalculatorAction_Index: public BattleCalculatorAction
{
    public:
        BattleCalculatorAction_Index (Type t, guint32 i, bool agg = false)
          : BattleCalculatorAction (t, agg), d_index (i) {}
        ~BattleCalculatorAction_Index () {}

        guint32 getIndex () {return d_index;}
    private:
        guint32 d_index;
};

class BattleCalculatorAction_DefenderStrength: public BattleCalculatorAction_Index
{
public:
    BattleCalculatorAction_DefenderStrength (guint32 i, guint32 str)
      : BattleCalculatorAction_Index (DEFENDER_STRENGTH, i, true),
     d_str (str) {}
    ~BattleCalculatorAction_DefenderStrength () {}
    Glib::ustring getActionName () const {return "DefenderStrength";}
    guint32 getStrength () const {return d_str;}
private:
    guint32 d_str;
};

class BattleCalculatorAction_AttackerStrength: public BattleCalculatorAction_Index
{
public:
    BattleCalculatorAction_AttackerStrength (guint32 i, guint32 str)
      : BattleCalculatorAction_Index (ATTACKER_STRENGTH, i, true),
     d_str (str) {}
    ~BattleCalculatorAction_AttackerStrength () {}
    Glib::ustring getActionName () const {return "AttackerStrength";}
    guint32 getStrength () const {return d_str;}
private:
    guint32 d_str;
};
#endif //BATTLE_CALCULATOR_ACTIONS_H
