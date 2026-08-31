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
#ifndef PLANTED_STANDARD_UNDO_ACTIONS_H
#define PLANTED_STANDARD_UNDO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include "undo-mgr.h"

//! A record of an event in the planted standard editor
/**
 * The purpose of these classes is to implement undo/redo in the planted
 * standard editor.
 */

class PlantedStandardUndoAction: public UndoAction
{
public:

    enum Type
      {
        NAME = 1,
        OWNER = 2,
        ORIG_OWNER = 3,
      };

    PlantedStandardUndoAction(Type type, bool agg = false)
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

class PlantedStandardUndoAction_Name: public PlantedStandardUndoAction,
    public UndoCursor
{
    public:
        PlantedStandardUndoAction_Name (Glib::ustring n, UndoMgr *u,
                                          Gtk::Entry *e)
          : PlantedStandardUndoAction (NAME, true),
          UndoCursor (u->get_pos (e), e), m_name (n)
  {
  }

        ~PlantedStandardUndoAction_Name ()
          {
          }

        Glib::ustring get_action_name () const
          {
            return "Name";
          }

        Glib::ustring get_name ()
          {
            return m_name;
          }

    private:
        Glib::ustring m_name;
};

class PlantedStandardUndoAction_Owner: public PlantedStandardUndoAction
{
    public:
        PlantedStandardUndoAction_Owner (guint32 u)
          : PlantedStandardUndoAction (OWNER), m_owner_id (u)
          {
          }

        ~PlantedStandardUndoAction_Owner ()
          {
          }

        Glib::ustring get_action_name () const
          {
            return "Owner";
          }

        guint32 get_owner_id () const
          {
            return m_owner_id;
          }

    private:
        guint32 m_owner_id;
};

class PlantedStandardUndoAction_OrigOwner: public PlantedStandardUndoAction
{
    public:
        PlantedStandardUndoAction_OrigOwner (guint32 u)
          : PlantedStandardUndoAction (ORIG_OWNER), m_owner_id (u)
          {
          }

        ~PlantedStandardUndoAction_OrigOwner ()
          {
          }

        Glib::ustring get_action_name () const
          {
            return "OrigOwner";
          }

        guint32 get_owner_id () const
          {
            return m_owner_id;
          }

    private:
        guint32 m_owner_id;
};
#endif
