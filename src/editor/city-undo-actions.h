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
#ifndef CITY_EDITOR_ACTIONS_H
#define CITY_EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>

#include "undo-action.h"
#include "city.h"
#include "undo-mgr.h"

//! A record of an event in the city editor
/** 
 * The purpose of these classes is to implement undo/redo in the city
 * editor.
 */

class CityEditorAction: public UndoAction
{
    public:

        enum Type
          {
            OWNER = 1,
            CAPITAL = 2,
            RAZED = 3,
            NAME = 4,
            INCOME = 5,
            NEWPROD = 6,
            RANDOMIZE_NAME = 7,
            RANDOMIZE_INCOME = 8,
            ADD = 9,
            REMOVE = 10,
            RANDOMIZE = 11,
            DESCRIPTION = 12,
            STRENGTH = 13,
            TURNS = 14,
            MOVES = 15,
            UPKEEP = 16,
          };

	//! Default constructor.
        CityEditorAction(Type type, UndoAction::AggregateType aggregate = UndoAction::AGGREGATE_NONE) : UndoAction (aggregate), d_type(type) {}

        Type getType() const {return d_type;}

    protected:

        Type d_type;
};

class CityEditorAction_City: public CityEditorAction
{
    public:
        CityEditorAction_City (Type t, City *c, bool agg = false)
          : CityEditorAction (t, agg ? UndoAction::AGGREGATE_DELAY : UndoAction::AGGREGATE_NONE), d_city (new City (*c)) { }
        ~CityEditorAction_City () { delete d_city; }

        City *getCity () const {return d_city;}
    private:
        City *d_city;
};

class CityEditorAction_Name : public CityEditorAction_City, public UndoCursor
{
    public:
        CityEditorAction_Name (City *c, UndoMgr *u, Gtk::Entry *e)
          :CityEditorAction_City (NAME, c, true), UndoCursor (u, e) {}
        ~CityEditorAction_Name () {}

        Glib::ustring getActionName () const {return "Name";}
};
class CityEditorAction_Income : public CityEditorAction_City
{
    public:
        CityEditorAction_Income (City *c)
          :CityEditorAction_City (INCOME, c, true) {}
        ~CityEditorAction_Income () {}

        Glib::ustring getActionName () const {return "Income";}
};
class CityEditorAction_Owner : public CityEditorAction_City
{
    public:
        CityEditorAction_Owner (City *c)
          :CityEditorAction_City (OWNER, c, false) {}
        ~CityEditorAction_Owner () {}

        Glib::ustring getActionName () const {return "Owner";}
};
class CityEditorAction_Capital : public CityEditorAction_City
{
    public:
        CityEditorAction_Capital (City *c)
          :CityEditorAction_City (CAPITAL, c, false) {}
        ~CityEditorAction_Capital () {}

        Glib::ustring getActionName () const {return "Capital";}
};
class CityEditorAction_Razed : public CityEditorAction_City
{
    public:
        CityEditorAction_Razed (City *c)
          :CityEditorAction_City (RAZED, c, false) {}
        ~CityEditorAction_Razed () {}

        Glib::ustring getActionName () const {return "Razed";}
};
class CityEditorAction_NewProd : public CityEditorAction_City
{
    public:
        CityEditorAction_NewProd (City *c)
          :CityEditorAction_City (NEWPROD, c, false) {}
        ~CityEditorAction_NewProd () {}

        Glib::ustring getActionName () const {return "NewProd";}
};
class CityEditorAction_RandomizeIncome : public CityEditorAction_City
{
    public:
        CityEditorAction_RandomizeIncome (City *c)
          :CityEditorAction_City (RANDOMIZE_INCOME, c, false) {}
        ~CityEditorAction_RandomizeIncome () {}

        Glib::ustring getActionName () const {return "RandomizeIncome";}
};
class CityEditorAction_RandomizeName : public CityEditorAction_City
{
    public:
        CityEditorAction_RandomizeName (City *c)
          :CityEditorAction_City (RANDOMIZE_NAME, c, false) {}
        ~CityEditorAction_RandomizeName () {}

        Glib::ustring getActionName () const {return "RandomizeName";}
};

class CityEditorAction_Add : public CityEditorAction_City
{
    public:
        CityEditorAction_Add (City *c)
          :CityEditorAction_City (ADD, c, false) {}
        ~CityEditorAction_Add () {}

        Glib::ustring getActionName () const {return "Add";}
};

class CityEditorAction_Remove : public CityEditorAction_City
{
    public:
        CityEditorAction_Remove (City *c)
          :CityEditorAction_City (REMOVE, c, false) {}
        ~CityEditorAction_Remove () {}

        Glib::ustring getActionName () const {return "Remove";}
};

class CityEditorAction_Randomize : public CityEditorAction_City
{
    public:
        CityEditorAction_Randomize (City *c)
          :CityEditorAction_City (RANDOMIZE, c, false) {}
        ~CityEditorAction_Randomize () {}

        Glib::ustring getActionName () const {return "Randomize";}
};
class CityEditorAction_Description : public CityEditorAction_City
{
    public:
        CityEditorAction_Description (City *c)
          :CityEditorAction_City (DESCRIPTION, c, false) {}
        ~CityEditorAction_Description () {}

        Glib::ustring getActionName () const {return "Description";}
};
class CityEditorAction_Index: public CityEditorAction
{
    public:
        CityEditorAction_Index (Type t, guint32 i, bool agg = false)
          : CityEditorAction (t, agg ? UndoAction::AGGREGATE_DELAY : UndoAction::AGGREGATE_NONE), d_index (i) {}
        ~CityEditorAction_Index () {}

        guint32 getIndex () {return d_index;}
    private:
        guint32 d_index;
};

class CityEditorAction_Strength : public CityEditorAction_Index
{
    public:
        CityEditorAction_Strength (guint32 i, guint32 x)
          :CityEditorAction_Index (STRENGTH, i, true), d_str (x) {}
        ~CityEditorAction_Strength () {}

        Glib::ustring getActionName () const {return "Strength";}

        guint32 getStrength () {return d_str;}
    private:
        guint32 d_str;
};

class CityEditorAction_Turns : public CityEditorAction_Index
{
    public:
        CityEditorAction_Turns (guint32 i, guint32 x)
          :CityEditorAction_Index (TURNS, i, true), d_turns (x) {}
        ~CityEditorAction_Turns () {}

        Glib::ustring getActionName () const {return "Turns";}

        guint32 getTurns () {return d_turns;}
    private:
        guint32 d_turns;
};

class CityEditorAction_Moves : public CityEditorAction_Index
{
    public:
        CityEditorAction_Moves (guint32 i, guint32 x)
          :CityEditorAction_Index (MOVES, i, true), d_moves (x) {}
        ~CityEditorAction_Moves () {}

        Glib::ustring getActionName () const {return "Moves";}

        guint32 getMoves () {return d_moves;}
    private:
        guint32 d_moves;
};

class CityEditorAction_Upkeep : public CityEditorAction_Index
{
    public:
        CityEditorAction_Upkeep (guint32 i, guint32 x)
          :CityEditorAction_Index (UPKEEP, i, true), d_upkeep (x) {}
        ~CityEditorAction_Upkeep () {}

        Glib::ustring getActionName () const {return "Upkeep";}

        guint32 getUpkeep () {return d_upkeep;}
    private:
        guint32 d_upkeep;
};

#endif //CITY_EDITOR_ACTIONS_H
