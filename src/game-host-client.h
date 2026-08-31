// Copyright (C) 2011, 2014, 2015, 2021 Ben Asselstine
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
#ifndef GAMEHOST_CLIENT_H
#define GAMEHOST_CLIENT_H

#include <config.h>

#include <list>
#include <memory>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include <glibmm.h>

class XML_Helper;
class NetworkConnection;
class RecentlyPlayedGameList;
class Profile;
class GameScenario;

//! Sends queries to and receives responses from the GamehostServer.
class GamehostClient
{
public:
        
  //! Returns the singleton instance.  Creates a new one if neccessary.
  static GamehostClient* getInstance();

  //! Deletes the singleton instance.
  static void deleteInstance();

  void start(Glib::ustring host, guint32 port, Profile *p);
  void disconnect();

  void request_game_list();
  sigc::signal<void, RecentlyPlayedGameList*, Glib::ustring> received_game_list;

  void request_game_host(Glib::ustring scenario_id);
  sigc::signal<void, Glib::ustring, Glib::ustring> received_host_response;

  void send_map(GameScenario *game_scenario);
  void send_map_file(Glib::ustring file);
  sigc::signal<void, Glib::ustring, guint32, Glib::ustring> received_map_response;

  void request_game_unhost(Glib::ustring scenario_id);
  sigc::signal<void, Glib::ustring, Glib::ustring> received_unhost_response;

  void request_reload();
  sigc::signal<void, Glib::ustring> received_reload_response;

  void request_server_terminate();

  sigc::signal<void> client_connected;
  sigc::signal<void> client_disconnected; 
  sigc::signal<void> client_forcibly_disconnected; //server went away
  sigc::signal<void> client_could_not_connect;
  sigc::signal<void, int, int> payload_progress;
  
  Glib::ustring getProfileId() const {return d_profile_id;};
  Glib::ustring getHost() const{return d_host;};
  guint32 getPort() const{return d_port;};

protected:
  GamehostClient();
  ~GamehostClient();

private:
  NetworkConnection* network_connection;

  void onConnected();
  void onConnectionLost();
  bool onGotMessage(int type, Glib::ustring message);
  void on_torn_down();
  bool loadRecentlyPlayedGameList(Glib::ustring tag, XML_Helper *helper);

  static GamehostClient * s_instance;
  Glib::ustring d_host;
  guint32 d_port;
  bool d_connected;
  Glib::ustring d_profile_id;
  RecentlyPlayedGameList *d_recently_played_game_list;
 
};

#endif
