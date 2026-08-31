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
#ifndef BACKPACK_EDITOR_UNDO_ACTIONS_H
#define BACKPACK_EDITOR_UNDO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include "backpack.h"
#include "undo-mgr.h"
#include "item.h"

//! A record of an event in the backpack editor
/** 
 * The purpose of these classes is to implement undo/redo in the backpack
 * editor.
 */

class BackpackEditorUndoAction: public UndoAction
{
public:

    enum Type
      {
        ADD = 1,
        REMOVE = 2,
        EDIT = 3,
      };

    BackpackEditorUndoAction(Type type, bool agg = false)
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

class BackpackEditorUndoAction_Index: public BackpackEditorUndoAction
{
public:
    BackpackEditorUndoAction_Index (Type t, guint32 i, bool agg = false)
      : BackpackEditorUndoAction (t, agg), m_index (i)
      {
      }

    ~BackpackEditorUndoAction_Index ()
      {
      }

    guint32 get_index ()
      {
        return m_index;
      }
private:
    guint32 m_index;
};

class BackpackEditorUndoAction_Edit: public BackpackEditorUndoAction_Index
{
public:
    BackpackEditorUndoAction_Edit (guint32 i, Item *item)
      : BackpackEditorUndoAction_Index (EDIT, i), m_item (new Item (*item))
      {
      }

    ~BackpackEditorUndoAction_Edit ()
      {
        delete m_item;
      }

    Glib::ustring get_action_name () const
      {
        return "Edit";
      }

    Item *get_item () const
      {
        return m_item;
      }
private:
    Item *m_item;
};

class BackpackEditorUndoAction_Save : public BackpackEditorUndoAction
{
public:
    BackpackEditorUndoAction_Save (Type t, Backpack *b)
      :BackpackEditorUndoAction (t, false), m_backpack (new Backpack (*b))
      {
      }

    ~BackpackEditorUndoAction_Save ()
      {
        delete m_backpack;
      }

    Backpack *get_backpack() const
      {
        return m_backpack;
      }
private:
    Backpack *m_backpack;
};

class BackpackEditorUndoAction_Add: public BackpackEditorUndoAction_Save
{
public:
    BackpackEditorUndoAction_Add (Backpack *b)
      :BackpackEditorUndoAction_Save (ADD, b)
      {
      }

    ~BackpackEditorUndoAction_Add ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Add";
      }
};

class BackpackEditorUndoAction_Remove: public BackpackEditorUndoAction_Save
{
public:
    BackpackEditorUndoAction_Remove (Backpack *b)
      :BackpackEditorUndoAction_Save (REMOVE, b)
      {
      }

    ~BackpackEditorUndoAction_Remove ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Remove";
      }
};
#endif
