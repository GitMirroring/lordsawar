// Copyright (C) 2011, 2015 Ben Asselstine
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
#ifndef GAMEHOST_SERVER_H
#define GAMEHOST_SERVER_H

#include "config.h"

#include <memory>
#include <list>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include <glibmm.h>

#include "timing.h"
#include "network-ghs-common.h"

class NetworkServer;
class XML_Helper;
class Profile;
class GameScenario;
class HostedGame;
class HostGameRequest;

class GamehostServer
{
public:
        
  //! Returns the singleton instance.  Creates a new one if neccessary.
  static GamehostServer * getInstance();

  static const int TOO_MANY_PROFILES_AWAITING_MAPS = 100;
  static const int ONE_HOUR_OLD = 60 * 60;

  //! Deletes the singleton instance.
  static void deleteInstance();

  bool isListening();
  void start(int port);

  void reload();

  //get functions
  Glib::ustring getHostname() const {return hostname;}

  //set functions
  void setHostname(Glib::ustring h) {hostname = h;}
  void setMembers(const std::list<Glib::ustring> &profile_ids)
    {members = profile_ids;}

  // signals
  sigc::signal<void, int> port_in_use;
  sigc::signal<void> terminate_request_received;

  // statics
  static std::list<Glib::ustring> load_members_from_file(Glib::ustring file);

protected:
  GamehostServer();
  ~GamehostServer();

private:
  std::unique_ptr<NetworkServer> network_server;
  Glib::ustring hostname;
  std::list<HostGameRequest*> host_game_requests;
  std::list<Glib::ustring> members;

  bool onGotMessage(void *conn, int type, Glib::ustring message);
  void onConnectionLost();
  void onConnectionMade();
  sigc::connection on_timer_registered(Timing::timer_slot s, int msecs_interval);
  void on_connected_to_gamelist_server_for_advertising_removal(Glib::ustring scenario_id);
  void on_advertising_removal_response_received();
  void on_connected_to_gamelist_server_for_advertising(HostedGame *game);
  void on_advertising_response_received();
  void on_child_setup();
  bool loadProfile(Glib::ustring tag, XML_Helper *helper, Profile **profile);

  // helpers
  void sendList(void *conn);
  void unhost(void *conn, Glib::ustring profile_id, Glib::ustring scenario_id, Glib::ustring &err);
  HostedGame* host(GameScenario *game_scenario, Profile *profile, Glib::ustring &err);
  void run_game(GameScenario *game_scenario, Glib::Pid *child_pid, guint32 port, Glib::ustring &err);
  void get_profile_and_scenario_id(Glib::ustring payload, Profile **profile, Glib::ustring &scenario_id, Glib::ustring &err);
  guint32 get_free_port();
  bool is_member(Glib::ustring profile_id);

  void cleanup_old_profiles_awaiting_maps(int stale = ONE_HOUR_OLD);
  bool add_to_profiles_awaiting_maps(Profile *profile, Glib::ustring scenario_id);
  Profile *remove_from_profiles_awaiting_maps(Glib::ustring scenario_id);
  bool waitForGameToBeConnectable(guint32 port);

  //! A static pointer for the singleton instance.
  static GamehostServer * s_instance;

};

#endif
