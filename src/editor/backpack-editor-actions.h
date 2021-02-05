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
#ifndef BACKPACK_EDITOR_ACTIONS_H
#define BACKPACK_EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include "Backpack.h"
#include "undo-mgr.h"
#include "Item.h"

//! A record of an event in the backpack editor
/** 
 * The purpose of these classes is to implement undo/redo in the backpack
 * editor.
 */

class BackpackEditorAction: public UndoAction
{
public:

    enum Type {
      ADD = 1,
      REMOVE = 2,
      EDIT = 3,
    };

    BackpackEditorAction(Type type, bool agg = false)
     : UndoAction (agg ? UndoAction::AGGREGATE_DELAY :
                   UndoAction::AGGREGATE_NONE), d_type (type) {}

    Type getType() const {return d_type;}

protected:

    Type d_type;
};

class BackpackEditorAction_Index: public BackpackEditorAction
{
    public:
        BackpackEditorAction_Index (Type t, guint32 i, bool agg = false)
          : BackpackEditorAction (t, agg), d_index (i) {}
        ~BackpackEditorAction_Index () {}

        guint32 getIndex () {return d_index;}
    private:
        guint32 d_index;
};

class BackpackEditorAction_Edit: public BackpackEditorAction_Index
{
    public:
        BackpackEditorAction_Edit (guint32 i, Item *item)
          : BackpackEditorAction_Index (EDIT, i), d_item (new Item (*item)) {}
        ~BackpackEditorAction_Edit () {delete d_item;}

        Glib::ustring getActionName () const {return "Edit";}
        Item *getItem () const {return d_item;}
    private:
        Item *d_item;
};

class BackpackEditorAction_Save : public BackpackEditorAction
{
    public:
        BackpackEditorAction_Save (Type t, Backpack *b)
          :BackpackEditorAction (t, false), d_backpack (new Backpack (*b)) {}
        ~BackpackEditorAction_Save ()
          {
            delete d_backpack;
          }

        Backpack *getBackpack() const {return d_backpack;}
    private:
        Backpack *d_backpack;
};

class BackpackEditorAction_Add: public BackpackEditorAction_Save
{
    public:
        BackpackEditorAction_Add (Backpack *b)
          :BackpackEditorAction_Save (ADD, b) {}
        ~BackpackEditorAction_Add () {}

        Glib::ustring getActionName () const {return "Add";}
};

class BackpackEditorAction_Remove: public BackpackEditorAction_Save
{
    public:
        BackpackEditorAction_Remove (Backpack *b)
          :BackpackEditorAction_Save (REMOVE, b) {}
        ~BackpackEditorAction_Remove () {}

        Glib::ustring getActionName () const {return "Remove";}
};
#endif //BACKPACK_EDITOR_ACTIONS_H
