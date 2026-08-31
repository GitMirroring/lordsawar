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
#ifndef STACK_UNDO_ACTIONS_H
#define STACK_UNDO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include "undo-mgr.h"
#include "stack.h"
#include "hero.h"

class Player;

//! A record of an event in the scenario builder's stack editor
/**
 * The purpose of these classes is to implement undo/redo in the stack
 * editor.
 */

class StackUndoAction: public UndoAction
{
public:

    enum Type
      {
        OWNER = 1,
        FORTIFY = 2,
        ADD = 3,
        REMOVE = 4,
        COPY = 5,
        STRENGTH = 6,
        MOVES = 7,
        UPKEEP = 8,
        HERO_DETAILS = 9,
      };

    StackUndoAction (Type type, bool agg = false)
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

class StackUndoAction_Fortify: public StackUndoAction
{
public:
    StackUndoAction_Fortify (bool state)
      : StackUndoAction (FORTIFY, false), m_active (state)
      {
      }

    ~StackUndoAction_Fortify ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Fortify";
      }

    bool get_fortify () const
      {
        return m_active;
      }
private:
    bool m_active;
};

class StackUndoAction_Owner: public StackUndoAction
{
public:
    StackUndoAction_Owner (Player *o)
      : StackUndoAction (OWNER, false), m_owner (o)
      {
      }

    ~StackUndoAction_Owner ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Owner";
      }

    Player *get_owner () const
      {
        return m_owner;
      }

private:
    Player *m_owner;
};

class StackUndoAction_Index: public StackUndoAction
{
public:
    StackUndoAction_Index (Type t, guint32 i, bool agg = false)
      : StackUndoAction (t, agg), m_index (i)
      {
      }

    ~StackUndoAction_Index ()
      {
      }

    guint32 get_index () const
      {
        return m_index;
      }
private:
    guint32 m_index;
};

class StackUndoAction_Strength: public StackUndoAction_Index
{
public:
    StackUndoAction_Strength (guint32 i, guint32 s)
      : StackUndoAction_Index (STRENGTH, i), m_strength (s)
      {
      }

    ~StackUndoAction_Strength ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Strength";
      }

    guint32 get_strength () const
      {
        return m_strength;
      }

private:
    guint32 m_strength;
};

class StackUndoAction_Moves: public StackUndoAction_Index
{
public:
    StackUndoAction_Moves (guint32 i, guint32 m)
      : StackUndoAction_Index (MOVES, i), m_moves (m)
      {
      }

    ~StackUndoAction_Moves ()
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

class StackUndoAction_Upkeep: public StackUndoAction_Index
{
public:
    StackUndoAction_Upkeep (guint32 i, guint32 u)
      : StackUndoAction_Index (UPKEEP, i), m_upkeep (u)
      {
      }

    ~StackUndoAction_Upkeep ()
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

class StackUndoAction_HeroDetails: public StackUndoAction_Index
{
public:
    StackUndoAction_HeroDetails (guint32 i, Hero *h)
      : StackUndoAction_Index (HERO_DETAILS, i), m_hero (new Hero (*h))
  {
  }

    ~StackUndoAction_HeroDetails ()
      {
        delete m_hero;
      }

    Glib::ustring get_action_name () const
      {
        return "HeroDetails";
      }

    Hero *get_hero () const
      {
        return m_hero;
      }

private:
    Hero *m_hero;
};

class StackUndoAction_Save : public StackUndoAction
{
public:
    StackUndoAction_Save (Type t, Stack *s)
      : StackUndoAction (t, false), m_stack (new Stack (*s))
      {
      }

    ~StackUndoAction_Save ()
      {
        delete m_stack;
      }

    Stack* get_stack () const
      {
        return m_stack;
      }
private:
    Stack *m_stack;
};

class StackUndoAction_Add: public StackUndoAction_Save
{
public:
    StackUndoAction_Add (Stack *s)
      : StackUndoAction_Save (ADD, s)
      {
      }

    ~StackUndoAction_Add ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Add";
      }
};

class StackUndoAction_Remove: public StackUndoAction_Save
{
public:
    StackUndoAction_Remove (Stack *s)
      : StackUndoAction_Save (REMOVE, s)
      {
      }

    ~StackUndoAction_Remove ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Remove";
      }
};

class StackUndoAction_Copy: public StackUndoAction_Save
{
public:
    StackUndoAction_Copy (Stack *s)
      : StackUndoAction_Save (COPY, s)
      {
      }

    ~StackUndoAction_Copy ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Copy";
      }
};
#endif
