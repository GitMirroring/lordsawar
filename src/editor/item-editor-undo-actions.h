//  Copyright (C) 2021 Ben Asselstine
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
#ifndef ITEM_EDITOR_UNDO_ACTIONS_H
#define ITEM_EDITOR_UNDO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include "undo-mgr.h"

//! A record of an event in the item editor
/** 
 * The purpose of these classes is to implement undo/redo in the item
 * editor.
 */

class ItemEditorUndoAction: public UndoAction
{
public:

    enum Type
      {
        NAME = 1,
        USES = 2,
        BONUS = 3,
      };

    ItemEditorUndoAction(Type type, bool agg = false)
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

class ItemEditorUndoAction_Name: public ItemEditorUndoAction, public UndoCursor
{
public:
    ItemEditorUndoAction_Name (Glib::ustring n, UndoMgr *u, Gtk::Entry *e)
      : ItemEditorUndoAction (NAME, true), UndoCursor (u->get_pos (e), e),
      m_name (n)
  {
  }

    ~ItemEditorUndoAction_Name ()
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

class ItemEditorUndoAction_Uses: public ItemEditorUndoAction
{
public:
    ItemEditorUndoAction_Uses (guint32 u)
      : ItemEditorUndoAction (USES, true), m_uses (u)
      {
      }

    ~ItemEditorUndoAction_Uses ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Uses";
      }

    guint32 get_uses () const
      {
        return m_uses;
      }

private:
    guint32 m_uses;
};

class ItemEditorUndoAction_Bonus: public ItemEditorUndoAction
{
public:
    ItemEditorUndoAction_Bonus (Item *i)
      : ItemEditorUndoAction (BONUS, false), m_item (new Item (*i, true))
      {
      }

    ~ItemEditorUndoAction_Bonus ()
      {
        delete m_item;
      }

    Glib::ustring get_action_name () const
      {
        return "Bonus";
      }

    Item *get_item () const
      {
        return m_item;
      }

private:
    Item *m_item;
};
#endif
