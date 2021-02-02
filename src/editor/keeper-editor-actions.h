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
#ifndef KEEPER_EDITOR_ACTIONS_H
#define KEEPER_EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>

#include "undo-action.h"
#include "ruin.h"
#include "undo-mgr.h"

//! A record of an event in the keeper editor
/** 
 * The purpose of these classes is to implement undo/redo in the keeper
 * editor.
 */

class KeeperEditorAction: public UndoAction
{
    public:

        enum Type
          {
            NAME = 1,
            RANDOMIZE = 2,
            KEEPER = 3,
          };

	//! Default constructor.
        KeeperEditorAction(Type type, UndoAction::AggregateType aggregate = UndoAction::AGGREGATE_NONE) : UndoAction (aggregate), d_type(type) {}

        Type getType() const {return d_type;}

    protected:

        Type d_type;
};

class KeeperEditorAction_Occupant: public KeeperEditorAction
{
    public:
        KeeperEditorAction_Occupant (Type t, Keeper *k, bool agg = false)
          : KeeperEditorAction (t, agg ? UndoAction::AGGREGATE_DELAY : UndoAction::AGGREGATE_NONE), d_keeper (new Keeper (*k)) { }
        ~KeeperEditorAction_Occupant () { delete d_keeper; }

        Keeper *getKeeper () const {return d_keeper;}
    private:
        Keeper *d_keeper;
};

class KeeperEditorAction_Name : public KeeperEditorAction_Occupant, public UndoCursor
{
    public:
        KeeperEditorAction_Name (Keeper *r, UndoMgr *u, Gtk::Entry *e)
          :KeeperEditorAction_Occupant (NAME, r, true), UndoCursor (u, e) {}
        ~KeeperEditorAction_Name () {}

        Glib::ustring getActionName () const {return "Name";}
};
class KeeperEditorAction_Randomize: public KeeperEditorAction_Occupant
{
    public:
        KeeperEditorAction_Randomize (Keeper *r)
          :KeeperEditorAction_Occupant (RANDOMIZE, r, false) {}
        ~KeeperEditorAction_Randomize () {}

        Glib::ustring getActionName () const {return "Randomize";}
};
class KeeperEditorAction_Keeper : public KeeperEditorAction_Occupant
{
    public:
        KeeperEditorAction_Keeper (Keeper *r)
          :KeeperEditorAction_Occupant (KEEPER, r, false) {}
        ~KeeperEditorAction_Keeper () {}

        Glib::ustring getActionName () const {return "Keeper";}
};
#endif //KEEPER_EDITOR_ACTIONS_H
