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
#ifndef RUIN_UNDO_ACTIONS_H
#define RUIN_UNDO_ACTIONS_H

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

class RuinUndoAction: public UndoAction
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
    RuinUndoAction (Type type,
                    UndoAction::AggregateType aggregate =
                    UndoAction::AGGREGATE_NONE)
      : UndoAction (aggregate), m_type (type)
      {
      }

    Type get_type () const
      {
        return m_type;
      }

protected:

    Type m_type;
};

class RuinUndoAction_Ruin: public RuinUndoAction
{
public:
    RuinUndoAction_Ruin (Type t, Ruin *r, bool agg = false)
      : RuinUndoAction (t,
                        agg ? UndoAction::AGGREGATE_DELAY :
                        UndoAction::AGGREGATE_NONE),
      m_ruin (new Ruin (*r))
        {
        }

    ~RuinUndoAction_Ruin ()
      {
        delete m_ruin;
      }

    Ruin *get_ruin () const
      {
        return m_ruin;
      }
private:
    Ruin *m_ruin;
};

class RuinUndoAction_Name : public RuinUndoAction_Ruin, public UndoCursor
{
public:
    RuinUndoAction_Name (Ruin *r, UndoMgr *u, Gtk::Entry *e)
      : RuinUndoAction_Ruin (NAME, r, true), UndoCursor (u->get_pos (e), e)
      {
      }

    ~RuinUndoAction_Name ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Name";
      }
};

class RuinUndoAction_RandomizeName : public RuinUndoAction_Ruin
{
public:
    RuinUndoAction_RandomizeName (Ruin *r)
      : RuinUndoAction_Ruin (RANDOMIZE_NAME, r, false)
      {
      }

    ~RuinUndoAction_RandomizeName ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "RandomizeName";
      }
};

class RuinUndoAction_Description : public RuinUndoAction_Ruin, public UndoCursor
{
public:
    RuinUndoAction_Description (Ruin *r, UndoMgr *u, Gtk::Entry *e)
      :RuinUndoAction_Ruin (DESCRIPTION, r, true),
      UndoCursor (u->get_pos (e), e)
      {
      }

    ~RuinUndoAction_Description ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Description";
      }
};

class RuinUndoAction_RandomKeeper : public RuinUndoAction_Ruin
{
public:
    RuinUndoAction_RandomKeeper (Ruin *r, bool state)
      :RuinUndoAction_Ruin (RANDOM_KEEPER, r, false), m_active (state)
      {
      }

    ~RuinUndoAction_RandomKeeper ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "RandomKeeper";
      }

    bool get_active () const
      {
        return m_active;
      }
private:
    bool m_active;
};

class RuinUndoAction_Keeper : public RuinUndoAction_Ruin
{
public:
    RuinUndoAction_Keeper (Ruin *r)
      :RuinUndoAction_Ruin (KEEPER, r, false)
      {
      }

    ~RuinUndoAction_Keeper ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Keeper";
      }
};

class RuinUndoAction_OnlySeenBy : public RuinUndoAction_Ruin
{
public:
    RuinUndoAction_OnlySeenBy (Ruin *r, bool state)
      :RuinUndoAction_Ruin (ONLY_SEEN_BY, r, false), m_active (state)
      {
      }

    ~RuinUndoAction_OnlySeenBy ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "OnlySeenBy";
      }

    bool get_active () const
      {
        return m_active;
      }
private:
    bool m_active;
};

class RuinUndoAction_OnlySeenPlayer : public RuinUndoAction_Ruin
{
public:
    RuinUndoAction_OnlySeenPlayer (Ruin *r, int row)
      :RuinUndoAction_Ruin (ONLY_SEEN_PLAYER, r, false), m_row (row)
      {
      }

    ~RuinUndoAction_OnlySeenPlayer ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "OnlySeenPlayer";
      }

    int get_row () const
      {
        return m_row;
      }
private:
    int m_row;

};

class RuinUndoAction_Type : public RuinUndoAction_Ruin
{
public:
    RuinUndoAction_Type (Ruin *r)
      :RuinUndoAction_Ruin (TYPE, r, false)
      {
      }

    ~RuinUndoAction_Type ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Type";
      }
};

class RuinUndoAction_RandomReward : public RuinUndoAction_Ruin
{
public:
    RuinUndoAction_RandomReward (Ruin *r, bool state)
      :RuinUndoAction_Ruin (RANDOM_REWARD, r, false), m_active (state)
      {
      }

    ~RuinUndoAction_RandomReward ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "RandomReward";
      }

    bool get_active () const
      {
        return m_active;
      }
private:
    bool m_active;
};

class RuinUndoAction_Reward : public RuinUndoAction_Ruin
{
public:
    RuinUndoAction_Reward (Ruin *r)
      :RuinUndoAction_Ruin (REWARD, r, false)
      {
      }

    ~RuinUndoAction_Reward ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Reward";
      }
};
#endif
