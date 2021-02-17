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
#ifndef REWARD_EDITOR_ACTIONS_H
#define REWARD_EDITOR_ACTIONS_H

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

class RewardEditorAction: public UndoAction
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
            RANDOMIZE_MAP = 13,
            HIDDEN_RUIN = 14,
            RANDOM_RUIN = 15
          };

	//! Default constructor.
        RewardEditorAction(Type type, UndoAction::AggregateType aggregate = UndoAction::AGGREGATE_NONE) : UndoAction (aggregate), d_type(type) {}

        Type getType() const {return d_type;}

    protected:

        Type d_type;
};

class RewardEditorAction_Reward: public RewardEditorAction
{
    public:
        RewardEditorAction_Reward (Type t, Reward *r, bool agg = false)
          : RewardEditorAction (t, agg ? UndoAction::AGGREGATE_DELAY : UndoAction::AGGREGATE_NONE), d_reward (Reward::copy (r)) { }
        ~RewardEditorAction_Reward () { delete d_reward; }

        Reward *getReward () const {return d_reward;}
    private:
        Reward *d_reward;
};

class RewardEditorAction_Type : public RewardEditorAction_Reward
{
    public:
        RewardEditorAction_Type (Reward *r)
          :RewardEditorAction_Reward (TYPE, r, false) {}
        ~RewardEditorAction_Type () {}

        Glib::ustring getActionName () const {return "Type";}
};
class RewardEditorAction_Gold: public RewardEditorAction_Reward
{
    public:
        RewardEditorAction_Gold (Reward *r)
          :RewardEditorAction_Reward (GOLD_PIECES, r, true) {}
        ~RewardEditorAction_Gold () {}

        Glib::ustring getActionName () const {return "Gold";}
};
class RewardEditorAction_RandomizeGold : public RewardEditorAction_Reward
{
    public:
        RewardEditorAction_RandomizeGold (Reward *r)
          :RewardEditorAction_Reward (RANDOMIZE_GOLD, r, false) {}
        ~RewardEditorAction_RandomizeGold () {}

        Glib::ustring getActionName () const {return "RandomizeGold";}
};
class RewardEditorAction_Item : public RewardEditorAction_Reward
{
    public:
        RewardEditorAction_Item (Reward *r)
          :RewardEditorAction_Reward (ITEM, r, false) {}
        ~RewardEditorAction_Item () {}

        Glib::ustring getActionName () const {return "Item";}
};
class RewardEditorAction_RandomizeItem : public RewardEditorAction_Reward
{
    public:
        RewardEditorAction_RandomizeItem (Reward *r)
          :RewardEditorAction_Reward (RANDOMIZE_ITEM, r, false) {}
        ~RewardEditorAction_RandomizeItem () {}

        Glib::ustring getActionName () const {return "RandomizeItem";}
};
class RewardEditorAction_AllyType : public RewardEditorAction_Reward
{
    public:
        RewardEditorAction_AllyType (Reward *r)
          :RewardEditorAction_Reward (ALLY_TYPE, r, false) {}
        ~RewardEditorAction_AllyType () {}

        Glib::ustring getActionName () const {return "AllyType";}
};
class RewardEditorAction_RandomizeAlly : public RewardEditorAction_Reward
{
    public:
        RewardEditorAction_RandomizeAlly (Reward *r)
          :RewardEditorAction_Reward (RANDOMIZE_ALLY, r, false) {}
        ~RewardEditorAction_RandomizeAlly () {}

        Glib::ustring getActionName () const {return "RandomizeAlly";}
};
class RewardEditorAction_AllyCount : public RewardEditorAction_Reward
{
    public:
        RewardEditorAction_AllyCount (Reward *r)
          :RewardEditorAction_Reward (ALLY_COUNT, r, true) {}
        ~RewardEditorAction_AllyCount () {}

        Glib::ustring getActionName () const {return "AllyCount";}
};
class RewardEditorAction_XCoord : public RewardEditorAction_Reward
{
    public:
        RewardEditorAction_XCoord (Reward *r)
          :RewardEditorAction_Reward (XCOORD, r, true) {}
        ~RewardEditorAction_XCoord () {}

        Glib::ustring getActionName () const {return "XCoord";}
};
class RewardEditorAction_YCoord : public RewardEditorAction_Reward
{
    public:
        RewardEditorAction_YCoord (Reward *r)
          :RewardEditorAction_Reward (YCOORD, r, true) {}
        ~RewardEditorAction_YCoord () {}

        Glib::ustring getActionName () const {return "YCoord";}
};
class RewardEditorAction_Width : public RewardEditorAction_Reward
{
    public:
        RewardEditorAction_Width (Reward *r)
          :RewardEditorAction_Reward (WIDTH, r, true) {}
        ~RewardEditorAction_Width () {}

        Glib::ustring getActionName () const {return "Width";}
};
class RewardEditorAction_Height : public RewardEditorAction_Reward
{
    public:
        RewardEditorAction_Height (Reward *r)
          :RewardEditorAction_Reward (HEIGHT, r, true) {}
        ~RewardEditorAction_Height () {}

        Glib::ustring getActionName () const {return "Height";}
};

class RewardEditorAction_RandomizeMap : public RewardEditorAction_Reward
{
    public:
        RewardEditorAction_RandomizeMap (Reward *r)
          :RewardEditorAction_Reward (RANDOMIZE_MAP, r, false) {}
        ~RewardEditorAction_RandomizeMap () {}

        Glib::ustring getActionName () const {return "RandomizeMap";}
};
class RewardEditorAction_HiddenRuin : public RewardEditorAction_Reward
{
    public:
        RewardEditorAction_HiddenRuin (Reward *r)
          :RewardEditorAction_Reward (HIDDEN_RUIN, r, false) {}
        ~RewardEditorAction_HiddenRuin () {}

        Glib::ustring getActionName () const {return "HiddenRuin";}
};
class RewardEditorAction_RandomRuin : public RewardEditorAction_Reward
{
    public:
        RewardEditorAction_RandomRuin (Reward *r)
          :RewardEditorAction_Reward (RANDOM_RUIN, r, false) {}
        ~RewardEditorAction_RandomRuin () {}

        Glib::ustring getActionName () const {return "RandomRuin";}
};
#endif //REWARD_EDITOR_ACTIONS_H
