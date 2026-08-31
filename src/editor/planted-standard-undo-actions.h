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
#ifndef PLANTED_STANDARD_EDITOR_ACTIONS_H
#define PLANTED_STANDARD_EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include "undo-mgr.h"

//! A record of an event in the planted standard editor
/** 
 * The purpose of these classes is to implement undo/redo in the planted
 * standard editor.
 */

class PlantedStandardEditorAction: public UndoAction
{
public:

    enum Type {
      NAME = 1,
      OWNER = 2,
      ORIG_OWNER = 3,
    };

    PlantedStandardEditorAction(Type type, bool agg = false)
     : UndoAction (agg ? UndoAction::AGGREGATE_DELAY :
                   UndoAction::AGGREGATE_NONE), d_type (type) {}

    Type getType() const {return d_type;}

protected:

    Type d_type;
};

class PlantedStandardEditorAction_Name: public PlantedStandardEditorAction, public UndoCursor
{
    public:
        PlantedStandardEditorAction_Name (Glib::ustring n, UndoMgr *u, Gtk::Entry *e)
          : PlantedStandardEditorAction (NAME, true), UndoCursor (u, e),
          d_name (n) {}
        ~PlantedStandardEditorAction_Name () {}

        Glib::ustring getActionName () const {return "Name";}

        Glib::ustring getName () {return d_name;}

    private:
        Glib::ustring d_name;
};

class PlantedStandardEditorAction_Owner: public PlantedStandardEditorAction
{
    public:
        PlantedStandardEditorAction_Owner (guint32 u)
          : PlantedStandardEditorAction (OWNER), d_owner_id (u) {}
        ~PlantedStandardEditorAction_Owner () {}

        Glib::ustring getActionName () const {return "Owner";}

        guint32 getOwnerId () {return d_owner_id;}

    private:
        guint32 d_owner_id;
};

class PlantedStandardEditorAction_OrigOwner: public PlantedStandardEditorAction
{
    public:
        PlantedStandardEditorAction_OrigOwner (guint32 u)
          : PlantedStandardEditorAction (ORIG_OWNER), d_owner_id (u) {}
        ~PlantedStandardEditorAction_OrigOwner () {}

        Glib::ustring getActionName () const {return "OrigOwner";}

        guint32 getOwnerId () {return d_owner_id;}

    private:
        guint32 d_owner_id;
};
#endif //PLANTED_STANDARD_EDITOR_ACTIONS_H
