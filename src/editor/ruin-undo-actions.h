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
#ifndef RUIN_EDITOR_ACTIONS_H
#define RUIN_EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>

#include "undo-action.h"
#include "ruin.h"
#include "undo-mgr.h"

//! A record of an event in the ruin editor
/** 
 * The purpose of these classes is to implement undo/redo in the ruin
 * editor.
 */

class RuinEditorAction: public UndoAction
{
    public:

        enum Type
          {
            NAME = 1,
            RANDOMIZE_NAME = 2,
            DESCRIPTION = 3,
            RANDOM_KEEPER = 4,
            KEEPER = 5,
            ONLY_SEEN_BY = 6,
            ONLY_SEEN_PLAYER = 7,
            TYPE = 8,
            RANDOM_REWARD = 9,
            REWARD = 10
          };

	//! Default constructor.
        RuinEditorAction(Type type, UndoAction::AggregateType aggregate = UndoAction::AGGREGATE_NONE) : UndoAction (aggregate), d_type(type) {}

        Type getType() const {return d_type;}

    protected:

        Type d_type;
};

class RuinEditorAction_Ruin: public RuinEditorAction
{
    public:
        RuinEditorAction_Ruin (Type t, Ruin *r, bool agg = false)
          : RuinEditorAction (t, agg ? UndoAction::AGGREGATE_DELAY : UndoAction::AGGREGATE_NONE), d_ruin (new Ruin (*r)) { }
        ~RuinEditorAction_Ruin () { delete d_ruin; }

        Ruin *getRuin () const {return d_ruin;}
    private:
        Ruin *d_ruin;
};

class RuinEditorAction_Name : public RuinEditorAction_Ruin, public UndoCursor
{
    public:
        RuinEditorAction_Name (Ruin *r, UndoMgr *u, Gtk::Entry *e)
          :RuinEditorAction_Ruin (NAME, r, true), UndoCursor (u, e) {}
        ~RuinEditorAction_Name () {}

        Glib::ustring getActionName () const {return "Name";}
};
class RuinEditorAction_RandomizeName : public RuinEditorAction_Ruin
{
    public:
        RuinEditorAction_RandomizeName (Ruin *r)
          :RuinEditorAction_Ruin (RANDOMIZE_NAME, r, false) {}
        ~RuinEditorAction_RandomizeName () {}

        Glib::ustring getActionName () const {return "RandomizeName";}
};
class RuinEditorAction_Description : public RuinEditorAction_Ruin, public UndoCursor
{
    public:
        RuinEditorAction_Description (Ruin *r, UndoMgr *u, Gtk::Entry *e)
          :RuinEditorAction_Ruin (DESCRIPTION, r, true), UndoCursor (u, e) {}
        ~RuinEditorAction_Description () {}

        Glib::ustring getActionName () const {return "Description";}
};
class RuinEditorAction_RandomKeeper : public RuinEditorAction_Ruin
{
    public:
        RuinEditorAction_RandomKeeper (Ruin *r)
          :RuinEditorAction_Ruin (RANDOM_KEEPER, r, false) {}
        ~RuinEditorAction_RandomKeeper () {}

        Glib::ustring getActionName () const {return "RandomKeeper";}
};
class RuinEditorAction_Keeper : public RuinEditorAction_Ruin
{
    public:
        RuinEditorAction_Keeper (Ruin *r)
          :RuinEditorAction_Ruin (KEEPER, r, false) {}
        ~RuinEditorAction_Keeper () {}

        Glib::ustring getActionName () const {return "Keeper";}
};
class RuinEditorAction_OnlySeenBy : public RuinEditorAction_Ruin
{
    public:
        RuinEditorAction_OnlySeenBy (Ruin *r)
          :RuinEditorAction_Ruin (ONLY_SEEN_BY, r, false) {}
        ~RuinEditorAction_OnlySeenBy () {}

        Glib::ustring getActionName () const {return "OnlySeenBy";}
};
class RuinEditorAction_OnlySeenPlayer : public RuinEditorAction_Ruin
{
    public:
        RuinEditorAction_OnlySeenPlayer (Ruin *r)
          :RuinEditorAction_Ruin (ONLY_SEEN_PLAYER, r, false) {}
        ~RuinEditorAction_OnlySeenPlayer () {}

        Glib::ustring getActionName () const {return "OnlySeenPlayer";}
};
class RuinEditorAction_Type : public RuinEditorAction_Ruin
{
    public:
        RuinEditorAction_Type (Ruin *r)
          :RuinEditorAction_Ruin (TYPE, r, false) {}
        ~RuinEditorAction_Type () {}

        Glib::ustring getActionName () const {return "Type";}
};
class RuinEditorAction_RandomReward : public RuinEditorAction_Ruin
{
    public:
        RuinEditorAction_RandomReward (Ruin *r, bool state)
          :RuinEditorAction_Ruin (RANDOM_REWARD, r, false), d_active (state) {}
        ~RuinEditorAction_RandomReward () {}

        Glib::ustring getActionName () const {return "RandomReward";}
        bool getActive () const {return d_active;}
    private:
        bool d_active;
};
class RuinEditorAction_Reward : public RuinEditorAction_Ruin
{
    public:
        RuinEditorAction_Reward (Ruin *r)
          :RuinEditorAction_Ruin (REWARD, r, false) {}
        ~RuinEditorAction_Reward () {}

        Glib::ustring getActionName () const {return "Reward";}
};
#endif //RUIN_EDITOR_ACTIONS_H
