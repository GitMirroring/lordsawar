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
#ifndef REWARD_LIST_UNDO_ACTIONS_H
#define REWARD_LIST_UNDO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include "reward-list.h"
#include "undo-mgr.h"
#include "reward.h"

//! A record of an event in the reward list editor in the scenario builder
/**
 * The purpose of these classes is to implement undo/redo in the rewards
 * editor.
 */

class RewardListUndoAction: public UndoAction
{
public:

    enum Type
      {
        ADD = 1,
        REMOVE = 2,
        EDIT = 3,
      };

    RewardListUndoAction (Type type, bool agg = false)
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

class RewardListUndoAction_Index: public RewardListUndoAction
{
public:
    RewardListUndoAction_Index (Type t, guint32 i, bool agg = false)
      : RewardListUndoAction (t, agg), m_index (i)
      {
      }

    ~RewardListUndoAction_Index ()
      {
      }

    guint32 get_index () const
      {
        return m_index;
      }
private:
    guint32 m_index;
};

class RewardListUndoAction_Edit: public RewardListUndoAction_Index
{
public:
    RewardListUndoAction_Edit (guint32 i, Reward *r)
      : RewardListUndoAction_Index (EDIT, i), m_reward (Reward::copy (r))
      {
      }

    ~RewardListUndoAction_Edit ()
      {
        delete m_reward;
      }

    Glib::ustring get_action_name () const
      {
        return "Edit";
      }

    Reward *get_reward () const
      {
        return m_reward;
      }
private:
    Reward *m_reward;
};

class RewardListUndoAction_Save : public RewardListUndoAction
{
public:
    RewardListUndoAction_Save (Type t, Rewardlist *r)
      :RewardListUndoAction (t, false), m_rewardlist (r->copy ())
      {
      }

    ~RewardListUndoAction_Save ()
      {
        if (m_rewardlist)
          delete m_rewardlist;
      }

    Rewardlist *get_rewards () const
      {
        return m_rewardlist;
      }

    void clear_rewards ()
      {
        m_rewardlist = NULL;
      }
private:
    Rewardlist *m_rewardlist;
};

class RewardListUndoAction_Add: public RewardListUndoAction_Save
{
public:
    RewardListUndoAction_Add (Rewardlist *r)
      :RewardListUndoAction_Save (ADD, r)
      {
      }

    ~RewardListUndoAction_Add ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Add";
      }
};

class RewardListUndoAction_Remove: public RewardListUndoAction_Save
{
public:
    RewardListUndoAction_Remove (Rewardlist *r)
      :RewardListUndoAction_Save (REMOVE, r)
      {
      }

    ~RewardListUndoAction_Remove ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Remove";
      }
};
#endif
