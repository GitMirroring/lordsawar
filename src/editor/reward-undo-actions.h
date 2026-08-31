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
#ifndef REWARD_UNDO_ACTIONS_H
#define REWARD_UNDO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>

#include "undo-action.h"
#include "ruin.h"
#include "undo-mgr.h"

//! A record of an event in the reward editor
/** 
 * The purpose of these classes is to implement undo/redo in the reward
 * editor.
 */

class RewardUndoAction: public UndoAction
{
public:

    enum Type
      {
        TYPE = 1,
        GOLD_PIECES = 2,
        RANDOMIZE_GOLD = 3,
        ITEM = 4,
        RANDOMIZE_ITEM = 5,
        ALLY_TYPE = 6,
        RANDOMIZE_ALLY = 7,
        ALLY_COUNT = 8,
        XCOORD = 9,
        YCOORD = 10,
        WIDTH = 11,
        HEIGHT = 12,
        MAP_NAME = 13,
        RANDOMIZE_MAP = 14,
        HIDDEN_RUIN = 15,
        RANDOM_RUIN = 16
      };

    //! Default constructor.
    RewardUndoAction (Type type,
                        UndoAction::AggregateType aggregate =
                        UndoAction::AGGREGATE_NONE) :
        UndoAction (aggregate), m_type (type)
  {
  }

    Type get_type () const
      {
        return m_type;
      }

protected:

    Type m_type;
};

class RewardUndoAction_Reward: public RewardUndoAction
{
public:
    RewardUndoAction_Reward (Type t, Reward *r, bool agg = false)
      : RewardUndoAction (t, agg ? UndoAction::AGGREGATE_DELAY :
                            UndoAction::AGGREGATE_NONE),
      m_reward (Reward::copy (r))
        {
        }

    ~RewardUndoAction_Reward ()
      {
        delete m_reward;
      }

    Reward *get_reward () const
      {
        return m_reward;
      }
private:
    Reward *m_reward;
};

class RewardUndoAction_Type : public RewardUndoAction_Reward
{
public:
    RewardUndoAction_Type (Reward *r)
      : RewardUndoAction_Reward (TYPE, r, false)
      {
      }

    ~RewardUndoAction_Type ()
      {
      }

    Glib::ustring get_action_name () const {return "Type";}
};

class RewardUndoAction_Gold: public RewardUndoAction_Reward
{
public:
    RewardUndoAction_Gold (Reward *r)
      : RewardUndoAction_Reward (GOLD_PIECES, r, true)
      {
      }

    ~RewardUndoAction_Gold ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Gold";
      }
};

class RewardUndoAction_RandomizeGold : public RewardUndoAction_Reward
{
public:
    RewardUndoAction_RandomizeGold (Reward *r)
      : RewardUndoAction_Reward (RANDOMIZE_GOLD, r, false)
      {
      }

    ~RewardUndoAction_RandomizeGold ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "RandomizeGold";
      }
};

class RewardUndoAction_Item : public RewardUndoAction_Reward
{
public:
    RewardUndoAction_Item (Reward *r)
      : RewardUndoAction_Reward (ITEM, r, false)
      {
      }

    ~RewardUndoAction_Item ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Item";
      }
};

class RewardUndoAction_RandomizeItem : public RewardUndoAction_Reward
{
public:
    RewardUndoAction_RandomizeItem (Reward *r)
      : RewardUndoAction_Reward (RANDOMIZE_ITEM, r, false)
      {
      }

    ~RewardUndoAction_RandomizeItem ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "RandomizeItem";
      }
};

class RewardUndoAction_AllyType : public RewardUndoAction_Reward
{
public:
    RewardUndoAction_AllyType (Reward *r)
      : RewardUndoAction_Reward (ALLY_TYPE, r, false)
      {
      }

    ~RewardUndoAction_AllyType ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "AllyType";
      }
};

class RewardUndoAction_RandomizeAlly : public RewardUndoAction_Reward
{
public:
    RewardUndoAction_RandomizeAlly (Reward *r)
      : RewardUndoAction_Reward (RANDOMIZE_ALLY, r, false)
      {
      }

    ~RewardUndoAction_RandomizeAlly ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "RandomizeAlly";
      }
};

class RewardUndoAction_AllyCount : public RewardUndoAction_Reward
{
public:
    RewardUndoAction_AllyCount (Reward *r)
      : RewardUndoAction_Reward (ALLY_COUNT, r, true)
      {
      }

    ~RewardUndoAction_AllyCount ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "AllyCount";
      }
};

class RewardUndoAction_XCoord : public RewardUndoAction_Reward
{
public:
    RewardUndoAction_XCoord (Reward *r)
      : RewardUndoAction_Reward (XCOORD, r, true)
      {
      }

    ~RewardUndoAction_XCoord ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "XCoord";
      }
};

class RewardUndoAction_YCoord : public RewardUndoAction_Reward
{
public:
    RewardUndoAction_YCoord (Reward *r)
      : RewardUndoAction_Reward (YCOORD, r, true)
      {
      }

    ~RewardUndoAction_YCoord ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "YCoord";
      }
};

class RewardUndoAction_Width : public RewardUndoAction_Reward
{
public:
    RewardUndoAction_Width (Reward *r)
      : RewardUndoAction_Reward (WIDTH, r, true)
      {
      }

    ~RewardUndoAction_Width ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Width";
      }
};

class RewardUndoAction_Height : public RewardUndoAction_Reward
{
public:
    RewardUndoAction_Height (Reward *r)
      : RewardUndoAction_Reward (HEIGHT, r, true)
      {
      }

    ~RewardUndoAction_Height ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Height";
      }
};

class RewardUndoAction_MapName: public RewardUndoAction_Reward, public UndoCursor
{
public:
    RewardUndoAction_MapName (Reward *r, UndoMgr *u, Gtk::Entry *e)
      : RewardUndoAction_Reward (MAP_NAME, r, true),
      UndoCursor (u->get_pos (e), e)
  {
  }

    ~RewardUndoAction_MapName ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "MapName";
      }
};

class RewardUndoAction_RandomizeMap : public RewardUndoAction_Reward
{
public:
    RewardUndoAction_RandomizeMap (Reward *r)
      : RewardUndoAction_Reward (RANDOMIZE_MAP, r, false)
      {
      }

    ~RewardUndoAction_RandomizeMap ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "RandomizeMap";
      }
};

class RewardUndoAction_HiddenRuin : public RewardUndoAction_Reward
{
public:
    RewardUndoAction_HiddenRuin (Reward *r)
      : RewardUndoAction_Reward (HIDDEN_RUIN, r, false)
      {
      }

    ~RewardUndoAction_HiddenRuin ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "HiddenRuin";
      }
};

class RewardUndoAction_RandomRuin : public RewardUndoAction_Reward
{
public:
    RewardUndoAction_RandomRuin (Reward *r)
      : RewardUndoAction_Reward (RANDOM_RUIN, r, false)
      {
      }

    ~RewardUndoAction_RandomRuin ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "RandomRuin";
      }
};
#endif
