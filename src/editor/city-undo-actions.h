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
#ifndef CITY_UNDO_ACTIONS_H
#define CITY_UNDO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>

#include "undo-action.h"
#include "city.h"
#include "undo-mgr.h"

//! A record of an event in the city editor of the scenario builder
/**
 * The purpose of these classes is to implement undo/redo in the city
 * editor.
 */

class CityUndoAction: public UndoAction
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
        ADD = 7,
        REMOVE = 8,
        RANDOMIZE = 9,
        DESCRIPTION = 10,
        STRENGTH = 11,
        TURNS = 12,
        MOVES = 13,
        UPKEEP = 14,
        ORDER = 15,
      };

    //! Default constructor.
    CityUndoAction (Type type,
                      UndoAction::AggregateType aggregate =
                      UndoAction::AGGREGATE_NONE)
      : UndoAction (aggregate), m_type (type)
      {
      }

    Type get_type () const
      {
        return m_type;
      }

protected:

    Type m_type;
};

class CityUndoAction_City: public CityUndoAction
{
public:
    CityUndoAction_City (Type t, City *c, bool agg = false)
      : CityUndoAction (t,
                          agg ? UndoAction::AGGREGATE_DELAY :
                          UndoAction::AGGREGATE_NONE),
      m_city (new City (*c))
        {
        }

    ~CityUndoAction_City ()
      {
        delete m_city;
      }

    City *get_city () const
      {
        return m_city;
      }
private:
    City *m_city;
};

class CityUndoAction_Name : public CityUndoAction_City, public UndoCursor
{
public:
    CityUndoAction_Name (City *c, UndoMgr *u, Gtk::Entry *e)
      : CityUndoAction_City (NAME, c, true), UndoCursor (u->get_pos (e), e)
      {
      }

    ~CityUndoAction_Name ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Name";
      }
};

class CityUndoAction_Income : public CityUndoAction_City
{
public:
    CityUndoAction_Income (City *c)
      : CityUndoAction_City (INCOME, c, true)
      {
      }

    ~CityUndoAction_Income ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Income";
      }
};

class CityUndoAction_Owner : public CityUndoAction_City
{
public:
    CityUndoAction_Owner (City *c)
      : CityUndoAction_City (OWNER, c, false)
      {
      }

    ~CityUndoAction_Owner ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Owner";
      }
};

class CityUndoAction_Capital : public CityUndoAction_City
{
    public:
        CityUndoAction_Capital (City *c)
          : CityUndoAction_City (CAPITAL, c, false)
          {
          }

        ~CityUndoAction_Capital ()
          {
          }

        Glib::ustring get_action_name () const
          {
            return "Capital";
          }
};

class CityUndoAction_Razed : public CityUndoAction_City
{
public:
    CityUndoAction_Razed (City *c)
      : CityUndoAction_City (RAZED, c, false)
      {
      }

    ~CityUndoAction_Razed ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Razed";
      }
};

class CityUndoAction_NewProd : public CityUndoAction_City
{
public:
    CityUndoAction_NewProd (City *c)
      : CityUndoAction_City (NEWPROD, c, false)
      {
      }

    ~CityUndoAction_NewProd ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "NewProd";
      }
};

class CityUndoAction_Add : public CityUndoAction_City
{
public:
    CityUndoAction_Add (City *c)
      : CityUndoAction_City (ADD, c, false)
      {
      }

    ~CityUndoAction_Add ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Add";
      }
};

class CityUndoAction_Remove : public CityUndoAction_City
{
public:
    CityUndoAction_Remove (City *c)
      : CityUndoAction_City (REMOVE, c, false)
      {
      }

    ~CityUndoAction_Remove ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Remove";
      }
};

class CityUndoAction_Randomize : public CityUndoAction_City
{
public:
    CityUndoAction_Randomize (City *c)
      : CityUndoAction_City (RANDOMIZE, c, false)
      {
      }

    ~CityUndoAction_Randomize ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Randomize";
      }
};

class CityUndoAction_Description : public CityUndoAction_City, public UndoCursor
{
public:
    CityUndoAction_Description (City *c, UndoMgr *u, Gtk::Entry *e)
      : CityUndoAction_City (DESCRIPTION, c, true),
      UndoCursor (u->get_pos (e), e)
      {
      }

    ~CityUndoAction_Description ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Description";
      }
};


class CityUndoAction_Index: public CityUndoAction
{
public:
    CityUndoAction_Index (Type t, guint32 i, bool agg = false)
      : CityUndoAction (t,
                        agg ? UndoAction::AGGREGATE_DELAY :
                        UndoAction::AGGREGATE_NONE), m_index (i)
        {
        }

    ~CityUndoAction_Index ()
      {
      }

    guint32 get_index () const
      {
        return m_index;
      }
private:
    guint32 m_index;
};

class CityUndoAction_Strength : public CityUndoAction_Index
{
public:
    CityUndoAction_Strength (guint32 i, guint32 x)
      : CityUndoAction_Index (STRENGTH, i, true), m_str (x)
      {
      }

    ~CityUndoAction_Strength ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Strength";
      }

    guint32 get_strength () const
      {
        return m_str;
      }
private:
    guint32 m_str;
};

class CityUndoAction_Turns : public CityUndoAction_Index
{
public:
    CityUndoAction_Turns (guint32 i, guint32 x)
      : CityUndoAction_Index (TURNS, i, true), m_turns (x)
      {
      }

    ~CityUndoAction_Turns ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Turns";
      }

    guint32 get_turns () const
      {
        return m_turns;
      }
private:
    guint32 m_turns;
};

class CityUndoAction_Moves : public CityUndoAction_Index
{
public:
    CityUndoAction_Moves (guint32 i, guint32 x)
      : CityUndoAction_Index (MOVES, i, true), m_moves (x)
      {
      }

    ~CityUndoAction_Moves ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Moves";
      }

    guint32 get_moves () const
      {
        return m_moves;
      }
private:
    guint32 m_moves;
};

class CityUndoAction_Upkeep : public CityUndoAction_Index
{
public:
    CityUndoAction_Upkeep (guint32 i, guint32 x)
      : CityUndoAction_Index (UPKEEP, i, true), m_upkeep (x)
      {
      }

    ~CityUndoAction_Upkeep ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Upkeep";
      }

    guint32 get_upkeep () const
      {
        return m_upkeep;
      }
private:
    guint32 m_upkeep;
};

class CityUndoAction_Order : public CityUndoAction_City
{
public:
    CityUndoAction_Order (City *c)
      : CityUndoAction_City (ORDER, c, false)
      {
      }

    ~CityUndoAction_Order ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Order";
      }
};

#endif
