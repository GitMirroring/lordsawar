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
#ifndef HERO_EDITOR_UNDO_ACTIONS_H
#define HERO_EDITOR_UNDO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include "undo-mgr.h"

//! A record of an event in the hero editor
/** 
 * The purpose of these classes is to implement undo/redo in the hero
 * editor.
 */

class HeroEditorUndoAction: public UndoAction
{
public:

    enum Type
      {
        NAME = 1,
        GENDER = 2,
        BACKPACK = 3,
        CHARACTER = 4,
      };

    HeroEditorUndoAction(Type type, bool agg = false)
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

class HeroEditorUndoAction_Name: public HeroEditorUndoAction, public UndoCursor
{
public:
    HeroEditorUndoAction_Name (Glib::ustring n, UndoMgr *u, Gtk::Entry *e)
      : HeroEditorUndoAction (NAME, true), UndoCursor (u->get_pos (e), e),
      m_name (n)
  {
  }

    ~HeroEditorUndoAction_Name ()
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

class HeroEditorUndoAction_Gender: public HeroEditorUndoAction
{
public:
    HeroEditorUndoAction_Gender (Hero::Gender g)
      : HeroEditorUndoAction (GENDER), m_gender (g)
      {
      }

    ~HeroEditorUndoAction_Gender ()
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

class HeroEditorUndoAction_Backpack: public HeroEditorUndoAction
{
public:
    HeroEditorUndoAction_Backpack (Backpack *b)
      : HeroEditorUndoAction (BACKPACK), m_backpack (new Backpack (*b))
      {
      }

    ~HeroEditorUndoAction_Backpack ()
      {
        delete m_backpack;
      }

    Glib::ustring get_action_name () const
      {
        return "Backpack";
      }

    Backpack * get_backpack () const
      {
        return m_backpack;
      }

private:
    Backpack *m_backpack;
};

class HeroEditorUndoAction_Character: public HeroEditorUndoAction
{
public:
    HeroEditorUndoAction_Character (guint32 id)
      : HeroEditorUndoAction (CHARACTER), m_character_id (id)
      {
      }

    ~HeroEditorUndoAction_Character ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Character";
      }

    guint32 get_character_id () const
      {
        return m_character_id;
      }

private:
    guint32 m_character_id;
};
#endif
