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
#ifndef TEMPLE_EDITOR_UNDO_ACTIONS_H
#define TEMPLE_EDITOR_UNDO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include "undo-mgr.h"
#include "temple.h"

//! A record of an event in the temple editor
/** 
 * The purpose of these classes is to implement undo/redo in the temple
 * editor.
 */

class TempleEditorUndoAction: public UndoAction
{
public:

    enum Type
      {
        NAME = 1,
        RANDOMIZE_NAME = 2,
        DESCRIPTION = 3,
        TYPE = 4,
      };

    TempleEditorUndoAction(Type type, bool agg = false)
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

class TempleEditorUndoAction_Name: public TempleEditorUndoAction, public UndoCursor
{
public:
    TempleEditorUndoAction_Name (Glib::ustring n, UndoMgr *u, Gtk::Entry *e)
      : TempleEditorUndoAction (NAME, true), UndoCursor (u->get_pos (e), e),
      m_name (n)
  {
  }
    ~TempleEditorUndoAction_Name ()
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

class TempleEditorUndoAction_RandomizeName: public TempleEditorUndoAction
{
public:
    TempleEditorUndoAction_RandomizeName (Glib::ustring n)
      : TempleEditorUndoAction (RANDOMIZE_NAME), m_name (n)
      {
      }

    ~TempleEditorUndoAction_RandomizeName ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "RandomizeName";
      }

    Glib::ustring get_name () const
      {
        return m_name;
      }

private:
    Glib::ustring m_name;
};

class TempleEditorUndoAction_Description: public TempleEditorUndoAction, public UndoCursor
{
public:
    TempleEditorUndoAction_Description (Glib::ustring d, UndoMgr *u,
                                    Gtk::Entry *e)
      : TempleEditorUndoAction (DESCRIPTION, true), UndoCursor (u->get_pos (e), e),
      m_description (d)
  {
  }

    ~TempleEditorUndoAction_Description ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Description";
      }

    Glib::ustring get_description () const
      {
        return m_description;
      }

private:
    Glib::ustring m_description;
};

class TempleEditorUndoAction_Type: public TempleEditorUndoAction
{
public:
    TempleEditorUndoAction_Type (Temple::Type ty)
      : TempleEditorUndoAction (TYPE, false), m_temple_type (ty)
      {
      }

    ~TempleEditorUndoAction_Type ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Type";
      }

    Temple::Type get_temple_type () const
      {
        return m_temple_type;
      }

private:
    Temple::Type m_temple_type;
};
#endif
