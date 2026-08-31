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
#ifndef CHARACTER_UNDO_ACTIONS_H
#define CHARACTER_UNDO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include "hero-proto.h"
#include "hero-templates.h"
#include "undo-mgr.h"

//! A record of an event in the character editor
/** 
 * The purpose of these classes is to implement undo/redo in the character
 * editor.
 */

class CharacterUndoAction: public UndoAction
{
public:

    enum Type
      {
        NAME = 1,
        GENDER = 2,
        ADD = 3,
        REMOVE = 4,
        STRATEGY = 5,
        BACKPACK = 6,
        DESCRIPTION = 7,
      };

    CharacterUndoAction(Type type, bool agg = false)
      : UndoAction (agg ? UndoAction::AGGREGATE_DELAY :
                    UndoAction::AGGREGATE_NONE), m_type (type)
        {
        }

    Type get_type() const
      {
        return m_type;
      }

protected:

    Type m_type;
};

class CharacterUndoAction_Index: public CharacterUndoAction
{
public:
    CharacterUndoAction_Index (Type t, guint32 i, bool agg = false)
      : CharacterUndoAction (t, agg), m_index (i)
      {
      }

    ~CharacterUndoAction_Index ()
      {
      }

    guint32 get_index () const
      {
        return m_index;
      }
private:
    guint32 m_index;
};

class CharacterUndoAction_Name: public CharacterUndoAction_Index,
    public UndoCursor
{
public:
    CharacterUndoAction_Name (guint32 i, Glib::ustring n, UndoMgr *u,
                              Gtk::Entry *e)
      : CharacterUndoAction_Index (NAME, i, true),
      UndoCursor (u->get_pos (e), e), m_name (n)
  {
  }

    ~CharacterUndoAction_Name ()
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

class CharacterUndoAction_Gender: public CharacterUndoAction_Index
{
public:
    CharacterUndoAction_Gender (guint32 i, Hero::Gender g)
      : CharacterUndoAction_Index (GENDER, i), m_gender (g)
      {
      }

    ~CharacterUndoAction_Gender ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Gender";
      }

    Hero::Gender get_gender () const
      {
        return m_gender;
      }

private:
    Hero::Gender m_gender;
};

class CharacterUndoAction_Save : public CharacterUndoAction
{
    public:
        CharacterUndoAction_Save (Type t, HeroTemplates *h)
          :CharacterUndoAction (t, false), m_heroes (h)
          {
          }

        ~CharacterUndoAction_Save ()
          {
            delete m_heroes;
          }

        void clear_heroes ()
          {
            m_heroes = NULL;
          }

        HeroTemplates * get_heroes () const
          {
            return m_heroes;
          }
    private:
        HeroTemplates *m_heroes;
};

class CharacterUndoAction_Add: public CharacterUndoAction_Save
{
public:
    CharacterUndoAction_Add (HeroTemplates *t)
      :CharacterUndoAction_Save (ADD, t)
      {
      }

    ~CharacterUndoAction_Add ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Add";
      }
};

class CharacterUndoAction_Remove: public CharacterUndoAction_Save
{
public:
    CharacterUndoAction_Remove (HeroTemplates *t)
      :CharacterUndoAction_Save (REMOVE, t)
      {
      }

    ~CharacterUndoAction_Remove ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Remove";
      }
};

class CharacterUndoAction_Strategy: public CharacterUndoAction_Index
{
public:
    CharacterUndoAction_Strategy (guint32 i, HeroStrategy *s)
      : CharacterUndoAction_Index (STRATEGY, i),
      m_strategy (HeroStrategy::copy (s))
  {
  }

    ~CharacterUndoAction_Strategy ()
      {
        delete m_strategy;
      }

    Glib::ustring get_action_name () const
      {
        return "Strategy";
      }

    HeroStrategy *get_strategy () const
      {
        return m_strategy;
      }

private:
    HeroStrategy* m_strategy;
};

class CharacterUndoAction_Backpack: public CharacterUndoAction_Index
{
public:
    CharacterUndoAction_Backpack (guint32 i, std::list<guint32> items)
      : CharacterUndoAction_Index (BACKPACK, i), m_items (items)
      {
      }

    ~CharacterUndoAction_Backpack ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Backpack";
      }

    std::list<guint32> get_backpack () const
      {
        return m_items;
      }

private:
    std::list<guint32> m_items;
};

class CharacterUndoAction_Desc: public CharacterUndoAction_Index,
    public UndoCursor
{
public:
    CharacterUndoAction_Desc (guint32 i, Glib::ustring d, UndoMgr *u,
                              Gtk::Entry *e)
      : CharacterUndoAction_Index (DESCRIPTION, i, true),
      UndoCursor (u->get_pos (e), e), m_desc (d)
  {
  }

    ~CharacterUndoAction_Desc ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Description";
      }

    Glib::ustring get_description () const
      {
        return m_desc;
      }

private:
    Glib::ustring m_desc;
};

#endif
