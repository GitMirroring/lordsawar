//  Copyright (C) 2026 Ben Asselstine
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
#ifndef STARTING_ITEMS_UNDO_ACTIONS_H
#define STARTING_ITEMS_UNDO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include "undo-mgr.h"

//! A record of an event in the starting items dialog 
/** 
 * The purpose of these classes is to implement undo/redo in the starting
 * items dialog
 */

class StartingItemsUndoAction: public UndoAction
{
public:

    enum Type
      {
        ADD = 1,
        REMOVE = 2,
      };

    StartingItemsUndoAction(Type type, bool agg = false)
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

class StartingItemsUndoAction_Add: public StartingItemsUndoAction
{
public:
    StartingItemsUndoAction_Add (std::list<guint32> ids)
      :StartingItemsUndoAction (ADD), m_ids (ids)
      {
      }

    ~StartingItemsUndoAction_Add ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Add";
      }

    std::list<guint32> get_ids () const
      {
        return m_ids;
      }

private:
    std::list<guint32> m_ids;
};

class StartingItemsUndoAction_Remove: public StartingItemsUndoAction
{
public:
    StartingItemsUndoAction_Remove (std::list<guint32> ids)
      :StartingItemsUndoAction (REMOVE), m_ids (ids)
      {
      }

    ~StartingItemsUndoAction_Remove ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Remove";
      }

    std::list<guint32> get_ids () const
      {
        return m_ids;
      }

private:
    std::list<guint32> m_ids;
};

#endif
