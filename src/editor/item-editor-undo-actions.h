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
#ifndef ITEM_EDITOR_ACTIONS_H
#define ITEM_EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include "undo-mgr.h"

//! A record of an event in the item editor
/** 
 * The purpose of these classes is to implement undo/redo in the item
 * editor.
 */

class ItemEditorAction: public UndoAction
{
public:

    enum Type {
      NAME = 1,
      USES = 2,
    };

    ItemEditorAction(Type type, bool agg = false)
     : UndoAction (agg ? UndoAction::AGGREGATE_DELAY :
                   UndoAction::AGGREGATE_NONE), d_type (type) {}

    Type getType() const {return d_type;}

protected:

    Type d_type;
};

class ItemEditorAction_Name: public ItemEditorAction, public UndoCursor
{
    public:
        ItemEditorAction_Name (Glib::ustring n, UndoMgr *u, Gtk::Entry *e)
          : ItemEditorAction (NAME, true), UndoCursor (u, e),
          d_name (n) {}
        ~ItemEditorAction_Name () {}

        Glib::ustring getActionName () const {return "Name";}

        Glib::ustring getName () {return d_name;}

    private:
        Glib::ustring d_name;
};

class ItemEditorAction_Uses: public ItemEditorAction
{
    public:
        ItemEditorAction_Uses (guint32 u)
          : ItemEditorAction (USES, true), d_uses (u) {}
        ~ItemEditorAction_Uses () {}

        Glib::ustring getActionName () const {return "Uses";}

        guint32 getUses () {return d_uses;}

    private:
        guint32 d_uses;
};
#endif //ITEM_EDITOR_ACTIONS_H
