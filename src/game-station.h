// Copyright (C) 2008, 2011, 2014 Ben Asselstine
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
#ifndef GAME_STATION_H
#define GAME_STATION_H

#include <config.h>

#include <memory>
#include <list>
#include <map>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include <sigc++/connection.h>

#include "game-client-decoder.h"

//! A helper class for GameServer and GameClient objects.
class GameStation: public GameClientDecoder 
{
public:
        
  sigc::signal<void, Glib::ustring> remote_participant_joins;
  sigc::signal<void, Player*, Glib::ustring> player_sits;
  sigc::signal<void, Player*, Glib::ustring> player_stands;
  sigc::signal<void, Glib::ustring> remote_participant_departs;
  sigc::signal<void> playerlist_reorder_received;
  sigc::signal<void, Player*> local_player_moved;
  sigc::signal<void, Player*> local_player_died;
  sigc::signal<void, Player*> local_player_starts_move;
  sigc::signal<void, Player*, int> player_changes_type;
  sigc::signal<void, Glib::ustring, Glib::ustring> nickname_changed;
  sigc::signal<void, Player*> player_gets_turned_off;
  sigc::signal<void> round_begins;
  sigc::signal<void> round_ends;
  sigc::signal<void> game_may_begin;
  sigc::signal<void, Player *> start_player_turn;

  void listenForLocalEvents(Player *p);
  Glib::ustring getProfileId() const {return d_profile_id;};
protected:
  GameStation();
  virtual ~GameStation() {};

  virtual void onActionDone(Action *action, guint32 id) = 0;
  virtual void onHistoryDone(History *history, guint32 id) = 0;

  void clearNetworkActionlist(std::list<NetworkAction*> &actions);
  void clearNetworkHistorylist(std::list<NetworkHistory*> &histories);

  void stopListeningForLocalEvents(Player *p);
  void stopListeningForLocalEvents();

  static bool get_message_lobby_activity (Glib::ustring payload, 
                                          guint32 &player_id, 
                                          gint32 &action, bool &reported,
                                          Glib::ustring &nickname);

  void setProfileId(Glib::ustring id) {d_profile_id = id;};

private:
  std::map<guint32, sigc::connection> action_listeners;
  std::map<guint32, sigc::connection> history_listeners;
  Glib::ustring d_profile_id;
};

#endif
