// Copyright (C) 2008 Ole Laursen
// Copyright (C) 2011, 2014, 2015, 2017, 2021 Ben Asselstine
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
#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

#include <config.h>

#include <list>
#include <glibmm.h>
#include <memory>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
class NetworkAction;
class NetworkHistory;

#include "game-station.h"

class NetworkConnection;
class Player;

//! A remotely connected user in a networked game.
class GameClient: public GameStation
{
public:
        
  //! Returns the singleton instance.  Creates a new one if neccessary.
  static GameClient* getInstance();

  //! Deletes the singleton instance.
  static void deleteInstance();

  void start(Glib::ustring host, guint32 port, Glib::ustring profile_id, Glib::ustring nick);
  void disconnect();
  void request_seat_manifest();

  sigc::signal<void> client_connected;
  sigc::signal<void> client_disconnected; 
  sigc::signal<void> client_forcibly_disconnected; //server went away
  sigc::signal<void> client_could_not_connect;
  sigc::signal<void, int, int> payload_progress;
  
  void sit_down (Player *player);
  void stand_up (Player *player);
  void change_name(Player *player, Glib::ustring name);
  void change_type(Player *player, int type);
  void chat(Glib::ustring message);

  Glib::ustring getHost() const{return d_host;};
  guint32 getPort() const{return d_port;};

  void sendRoundOver();

protected:
  GameClient();
  ~GameClient();

private:
  NetworkConnection* network_connection;
  int player_id;

  void sit_or_stand (Player *player, bool sit);
  void onConnected();
  void onConnectionLost();
  bool onGotMessage(int type, Glib::ustring message);
  void on_torn_down();

  void onActionDone(Action *action, guint32 id);
  void sendActions();

  void onHistoryDone(History *history, guint32 id);
  void sendHistories();

  void gotTurnOrder (Glib::ustring payload);
  void gotKillPlayer(Player *player);
  void gotOffPlayer(Player *player);

  void sat_down(Player *player, Glib::ustring nickname);
  void stood_up(Player *player, Glib::ustring nickname);
  void name_changed (Player *player, Glib::ustring name);
  void type_changed (Player *player, int type);

  bool on_ping_timeout();

  std::list<NetworkAction*> actions;
  std::list<NetworkHistory*> histories;
  //! A static pointer for the singleton instance.
  static GameClient * s_instance;
  bool d_connected;
  Glib::ustring d_host;
  guint32 d_port;
  sigc::connection d_ping_timer;
  bool first_ping;
};

#endif
