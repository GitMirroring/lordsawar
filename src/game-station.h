//  Copyright (C) 2008, 2011, 2014, 2026 Ben Asselstine
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

  sigc::signal<void(Glib::ustring, bool)> signal_remote_participant_joins ()
    {
      return m_remote_participant_joins;
    }

  sigc::signal<void(Shield::Color, Glib::ustring)> signal_player_sits ()
    {
      return m_player_sits;
    }

  sigc::signal<void(Shield::Color, Glib::ustring)> signal_player_stands ()
    {
      return m_player_stands;
    }

  sigc::signal<void(Glib::ustring)> signal_remote_participant_departs ()
    {
      return m_remote_participant_departs;
    }

  sigc::signal<void()> signal_playerlist_reorder_received ()
    {
      return m_playerlist_reorder_received;
    }

  sigc::signal<void(Player*)> signal_local_player_moved ()
    {
      return m_local_player_moved;
    }

  sigc::signal<void(Player*)> signal_local_player_died ()
    {
      return m_local_player_died;
    }

  sigc::signal<void(Player*)> signal_local_player_starts_move ()
    {
      return m_local_player_starts_move;
    }

  sigc::signal<void(Player*, int)> signal_player_changes_type ()
    {
      return m_player_changes_type;
    }

  sigc::signal<void(Glib::ustring, Glib::ustring)> signal_nickname_changed ()
    {
      return m_nickname_changed;
    }

  sigc::signal<void(Player*)> signal_player_gets_turned_off ()
    {
      return m_player_gets_turned_off;
    }

  sigc::signal<void()> signal_round_begins ()
    {
      return m_round_begins;
    }

  sigc::signal<void()> signal_round_ends ()
    {
      return m_round_ends;
    }

  sigc::signal<void()> signal_game_begin ()
    {
      return m_game_begin;
    }

  sigc::signal<void()> signal_game_can_begin ()
    {
      return m_game_can_begin;
    }

  sigc::signal<void(int)> signal_waiting_for_ready ()
    {
      return m_waiting_for_ready;
    }

  sigc::signal<void(int)> signal_countdown ()
    {
      return m_countdown;
    }

  sigc::signal<void(Player *)> signal_start_player_turn ()
    {
      return m_start_player_turn;
    }

  sigc::signal<void(Glib::ustring)> signal_moderator_assigned ()
    {
      return m_moderator_assigned;
    }

  void hosted_player_sits (Shield::Color shield)
    {
      m_hosted_player_sits.emit (shield);
    }

  void hosted_player_stands (Shield::Color shield)
    {
      m_hosted_player_stands.emit (shield);
    }

  sigc::signal <void (Shield::Color)> signal_hosted_player_sits ()
    {
      return m_hosted_player_sits;
    }

  sigc::signal <void (Shield::Color)> signal_hosted_player_stands ()
    {
      return m_hosted_player_stands;
    }

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
                                          Shield::Color &shield,
                                          Glib::ustring &profile_id, 
                                          gint32 &action, bool &reported,
                                          Glib::ustring &nickname);

  void setProfileId(Glib::ustring id) {d_profile_id = id;};

  sigc::signal<void(Glib::ustring, bool)> m_remote_participant_joins;
  sigc::signal<void(Shield::Color, Glib::ustring)> m_player_sits;
  sigc::signal<void(Shield::Color, Glib::ustring)> m_player_stands;
  sigc::signal<void(Glib::ustring)> m_remote_participant_departs;
  sigc::signal<void()> m_playerlist_reorder_received;
  sigc::signal<void(Player*)> m_local_player_moved;
  sigc::signal<void(Player*)> m_local_player_died;
  sigc::signal<void(Player*)> m_local_player_starts_move;
  sigc::signal<void(Player*, int)> m_player_changes_type;
  sigc::signal<void(Glib::ustring, Glib::ustring)> m_nickname_changed; /* profile id, new name */
  sigc::signal<void(Player*)> m_player_gets_turned_off;
  sigc::signal<void()> m_round_begins;
  sigc::signal<void()> m_round_ends;
  sigc::signal<void()> m_game_begin;
  sigc::signal<void()> m_game_can_begin;
  sigc::signal<void(int)> m_waiting_for_ready;
  sigc::signal<void(int)> m_countdown;
  sigc::signal<void(Player *)> m_start_player_turn;
  sigc::signal<void(Glib::ustring)> m_moderator_assigned;
  sigc::signal <void (Shield::Color)> m_hosted_player_sits;
  sigc::signal <void (Shield::Color)> m_hosted_player_stands;
private:
  std::map<guint32, sigc::connection> action_listeners;
  std::map<guint32, sigc::connection> history_listeners;
  Glib::ustring d_profile_id;

};

#endif
