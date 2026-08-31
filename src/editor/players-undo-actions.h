//  Copyright (C) 2021 Ben Asselstine
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
#ifndef PLAYERS_UNDO_ACTIONS_H
#define PLAYERS_UNDO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>

#include "undo-action.h"
#include "hero-proto.h"
#include "undo-mgr.h"

//! A record of an event in the players dialog
/** 
 * The purpose of these classes is to implement undo/redo in the players
 * dialog.
 */

class PlayersUndoAction: public UndoAction
{
public:

    enum Type
      {
        NAME = 1,
        TYPE = 2,
        GOLD = 3,
        RANDOMIZE_GOLD = 4,
        HEROES = 5,
      };

    PlayersUndoAction (Type type,
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

class PlayersUndoAction_PlayerIndex: public PlayersUndoAction
{
public:
    PlayersUndoAction_PlayerIndex (Type t, guint32 i, bool agg = false)
      : PlayersUndoAction (t, agg ?
                           UndoAction::AGGREGATE_DELAY :
                           UndoAction::AGGREGATE_NONE), m_index (i)
        {
        }

    ~PlayersUndoAction_PlayerIndex ()
      {
      }

    guint32 get_index () const
      {
        return m_index;
      }
private:
    guint32 m_index;
};

class PlayersUndoAction_Name: public PlayersUndoAction_PlayerIndex,
    public UndoCursor
{
    public:
        PlayersUndoAction_Name (guint32 i, Glib::ustring n, UndoMgr *m,
                                Gtk::Entry *e)
          : PlayersUndoAction_PlayerIndex (NAME, i, true),
          UndoCursor (m->get_pos (e), e), m_name (n)
  {
  }

        ~PlayersUndoAction_Name ()
          {
          }

        Glib::ustring get_action_name () const
          {
            return "Name";
          }

        Glib::ustring get_name () const
          {
            return m_name;
          }

    private:
        Glib::ustring m_name;
};

class PlayersUndoAction_Type : public PlayersUndoAction_PlayerIndex
{
public:
    PlayersUndoAction_Type (guint32 i, int t)
      :PlayersUndoAction_PlayerIndex (TYPE, i), m_player_type (t)
      {
      }

    ~PlayersUndoAction_Type ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Type";
      }

    int get_player_type ()
      {
        return m_player_type;
      }
private:
    int m_player_type;
};

class PlayersUndoAction_Gold : public PlayersUndoAction_PlayerIndex
{
public:
    PlayersUndoAction_Gold (guint32 i, guint32 gp)
      :PlayersUndoAction_PlayerIndex (GOLD, i, true), m_gold (gp)
      {
      }

    ~PlayersUndoAction_Gold ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Gold";
      }

    guint32 get_gold () const
      {
        return m_gold;
      }
private:
    guint32 m_gold;
};

class PlayersUndoAction_RandomizeGold : public PlayersUndoAction
{
public:
    PlayersUndoAction_RandomizeGold (std::vector<guint32> players_gold)
      :PlayersUndoAction (RANDOMIZE_GOLD), m_players_gold (players_gold)
      {
      }

    ~PlayersUndoAction_RandomizeGold ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "RandomizeGold";
      }

    std::vector<guint32> get_players_gold () const
      {
        return m_players_gold;
      }
private:
    std::vector<guint32> m_players_gold;
};

class PlayersUndoAction_Heroes : public PlayersUndoAction_PlayerIndex
{
public:
    PlayersUndoAction_Heroes (guint32 i, std::vector<Character*> h)
      :PlayersUndoAction_PlayerIndex (HEROES, i, true), m_heroes (h)
      {
      }

    ~PlayersUndoAction_Heroes ()
      {
        for (auto h : m_heroes)
          delete h;
      }

    Glib::ustring get_action_name () const
      {
        return "Heroes";
      }

    void clear_heroes ()
      {
        m_heroes.clear ();
      }

    std::vector<Character *> get_heroes () const
      {
        return m_heroes;
      }
private:
    std::vector<Character *> m_heroes;
};

#endif
