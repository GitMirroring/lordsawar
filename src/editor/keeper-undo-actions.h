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
#ifndef KEEPER_UNDO_ACTIONS_H
#define KEEPER_UNDO_ACTIONS_H

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

class KeeperUndoAction: public UndoAction
{
public:

    enum Type
      {
        NAME = 1,
        RANDOMIZE = 2,
        KEEPER = 3,
      };

    //! Default constructor.
    KeeperUndoAction (Type type,
                      UndoAction::AggregateType aggregate =
                      UndoAction::AGGREGATE_NONE) :
        UndoAction (aggregate), m_type(type)
  {
  }

    Type get_type () const
      {
        return m_type;
      }

protected:

    Type m_type;
};

class KeeperUndoAction_Occupant: public KeeperUndoAction
{
public:
    KeeperUndoAction_Occupant (Type t, Keeper *k, bool agg = false)
      : KeeperUndoAction (t,
                          agg ? UndoAction::AGGREGATE_DELAY :
                          UndoAction::AGGREGATE_NONE),
      m_keeper (new Keeper (*k))
        {
        }

    ~KeeperUndoAction_Occupant ()
      {
        delete m_keeper;
      }

    Keeper *get_keeper () const
      {
        return m_keeper;
      }
private:
    Keeper *m_keeper;
};

class KeeperUndoAction_Name : public KeeperUndoAction_Occupant,
    public UndoCursor
{
public:
    KeeperUndoAction_Name (Keeper *r, UndoMgr *u, Gtk::Entry *e)
      :KeeperUndoAction_Occupant (NAME, r, true),
      UndoCursor (u->get_pos (e), e)
  {
  }

    ~KeeperUndoAction_Name ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Name";
      }
};

class KeeperUndoAction_Randomize: public KeeperUndoAction_Occupant
{
public:
    KeeperUndoAction_Randomize (Keeper *r)
      :KeeperUndoAction_Occupant (RANDOMIZE, r, false)
      {
      }

    ~KeeperUndoAction_Randomize ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Randomize";
      }
};

class KeeperUndoAction_Keeper : public KeeperUndoAction_Occupant
{
public:
    KeeperUndoAction_Keeper (Keeper *r)
      :KeeperUndoAction_Occupant (KEEPER, r, false)
      {
      }

    ~KeeperUndoAction_Keeper ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Keeper";
      }
};
#endif
