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
#ifndef REWARDLIST_EDITOR_ACTIONS_H
#define REWARDLIST_EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include "rewardlist.h"
#include "undo-mgr.h"
#include "reward.h"

//! A record of an event in the rewards editor
/** 
 * The purpose of these classes is to implement undo/redo in the rewards
 * editor.
 */

class RewardlistEditorAction: public UndoAction
{
public:

    enum Type {
      ADD = 1,
      REMOVE = 2,
      EDIT = 3,
    };

    RewardlistEditorAction(Type type, bool agg = false)
     : UndoAction (agg ? UndoAction::AGGREGATE_DELAY :
                   UndoAction::AGGREGATE_NONE), d_type (type) {}

    Type getType() const {return d_type;}

protected:

    Type d_type;
};

class RewardlistEditorAction_Index: public RewardlistEditorAction
{
    public:
        RewardlistEditorAction_Index (Type t, guint32 i, bool agg = false)
          : RewardlistEditorAction (t, agg), d_index (i) {}
        ~RewardlistEditorAction_Index () {}

        guint32 getIndex () {return d_index;}
    private:
        guint32 d_index;
};

class RewardlistEditorAction_Edit: public RewardlistEditorAction_Index
{
    public:
        RewardlistEditorAction_Edit (guint32 i, Reward *r)
          : RewardlistEditorAction_Index (EDIT, i), d_reward (Reward::copy (r))
          {}
        ~RewardlistEditorAction_Edit () {delete d_reward;}

        Glib::ustring getActionName () const {return "Edit";}
        Reward *getReward () const {return d_reward;}
    private:
        Reward *d_reward;
};

class RewardlistEditorAction_Save : public RewardlistEditorAction
{
    public:
        RewardlistEditorAction_Save (Type t, Rewardlist *r)
          :RewardlistEditorAction (t, false), d_rewardlist (r->copy ()) {}
        ~RewardlistEditorAction_Save ()
          {
            if (d_rewardlist)
              delete d_rewardlist;
          }

        Rewardlist *getRewards() const { return d_rewardlist; }
        void clearRewards () {d_rewardlist = NULL;}
    private:
        Rewardlist *d_rewardlist;
};

class RewardlistEditorAction_Add: public RewardlistEditorAction_Save
{
    public:
        RewardlistEditorAction_Add (Rewardlist *r)
          :RewardlistEditorAction_Save (ADD, r) {}
        ~RewardlistEditorAction_Add () {}

        Glib::ustring getActionName () const {return "Add";}
};

class RewardlistEditorAction_Remove: public RewardlistEditorAction_Save
{
    public:
        RewardlistEditorAction_Remove (Rewardlist *r)
          :RewardlistEditorAction_Save (REMOVE, r) {}
        ~RewardlistEditorAction_Remove () {}

        Glib::ustring getActionName () const {return "Remove";}
};
#endif //REWARDLIST_EDITOR_ACTIONS_H
