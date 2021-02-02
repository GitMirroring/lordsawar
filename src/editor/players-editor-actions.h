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
#ifndef PLAYERS_EDITOR_ACTIONS_H
#define PLAYERS_EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>

#include "undo-action.h"
#include "heroproto.h"
#include "undo-mgr.h"

//! A record of an event in the players dialog
/** 
 * The purpose of these classes is to implement undo/redo in the players
 * dialog.
 */

class PlayersEditorAction: public UndoAction
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

        PlayersEditorAction(Type type, UndoAction::AggregateType aggregate = UndoAction::AGGREGATE_NONE) :UndoAction (aggregate), d_type(type) {}

        Type getType() const {return d_type;}

    protected:

        Type d_type;
};

class PlayersEditorAction_PlayerIndex: public PlayersEditorAction
{
    public:
        PlayersEditorAction_PlayerIndex (Type t, guint32 i, bool agg = false)
          : PlayersEditorAction (t, agg ? UndoAction::AGGREGATE_DELAY : UndoAction::AGGREGATE_NONE), d_index (i) {}
        ~PlayersEditorAction_PlayerIndex () {}

        guint32 getIndex () {return d_index;}
    private:
        guint32 d_index;
};
class PlayersEditorAction_Name: public PlayersEditorAction_PlayerIndex, public UndoCursor
{
    public:
        PlayersEditorAction_Name (guint32 i, Glib::ustring n, UndoMgr *m,
                                  Gtk::Entry *e)
          : PlayersEditorAction_PlayerIndex (NAME, i, true), UndoCursor (m, e),
          d_name (n) {}
        ~PlayersEditorAction_Name () {}

        Glib::ustring getActionName () const {return "Name";}
        Glib::ustring getName () {return d_name;}

    private:
        Glib::ustring d_name;
};

class PlayersEditorAction_Type : public PlayersEditorAction_PlayerIndex
{
    public:
        PlayersEditorAction_Type (guint32 i, int t)
          :PlayersEditorAction_PlayerIndex (TYPE, i), d_player_type (t) {}
        ~PlayersEditorAction_Type () {}

        Glib::ustring getActionName () const {return "Type";}

        int getPlayerType () {return d_player_type;}
    private:
        int d_player_type;
};

class PlayersEditorAction_Gold : public PlayersEditorAction_PlayerIndex
{
    public:
        PlayersEditorAction_Gold (guint32 i, guint32 gp)
          :PlayersEditorAction_PlayerIndex (GOLD, i, true), d_gold (gp) {}
        ~PlayersEditorAction_Gold () {}

        Glib::ustring getActionName () const {return "Gold";}

        guint32 getGold () {return d_gold;}
    private:
        guint32 d_gold;
};

class PlayersEditorAction_RandomizeGold : public PlayersEditorAction
{
    public:
        PlayersEditorAction_RandomizeGold (std::list<guint32> players_gold)
          :PlayersEditorAction (RANDOMIZE_GOLD), d_players_gold (players_gold)
          {}
        ~PlayersEditorAction_RandomizeGold () {}

        Glib::ustring getActionName () const {return "RandomizeGold";}

        std::list<guint32> getPlayersGold () {return d_players_gold;}
    private:
        std::list<guint32> d_players_gold;
        Player::Type d_player_type;
};

class PlayersEditorAction_Heroes : public PlayersEditorAction_PlayerIndex
{
    public:
        PlayersEditorAction_Heroes (guint32 i, std::vector<HeroProto*> h)
          :PlayersEditorAction_PlayerIndex (HEROES, i, true), d_heroes (h) {}
        ~PlayersEditorAction_Heroes ()
          {
            for (auto h : d_heroes)
              delete h;
          }

        Glib::ustring getActionName () const {return "Heroes";}

        void clearHeroes () {d_heroes.clear ();}
        std::vector<HeroProto *> getHeroes () const {return d_heroes;}
    private:
        std::vector<HeroProto *> d_heroes;
};

#endif //PLAYERS_EDITOR_ACTIONS_H
