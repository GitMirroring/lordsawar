// Copyright (C) 2011, 2014, 2015 Ben Asselstine
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
#ifndef GAMELIST_SERVER_H
#define GAMELIST_SERVER_H

#include "config.h"

#include <memory>
#include <list>
#include <glibmm.h>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>

#include "timing.h"
#include "network-gls-common.h"

class NetworkServer;
class XML_Helper;

class GamelistServer
{
public:
        
  //! Returns the singleton instance.  Creates a new one if neccessary.
  static GamelistServer * getInstance();

  //! Deletes the singleton instance.
  static void deleteInstance();

  bool isListening();
  void start(int port);

  void reload();

  sigc::signal<void, int> port_in_use;
  sigc::signal<void> terminate_request_received;

protected:
  GamelistServer();
  ~GamelistServer();

private:
  std::unique_ptr<NetworkServer> network_server;
  Glib::ustring datafile;

  bool onGotMessage(void *conn, int type, Glib::ustring message);
  void onConnectionLost();
  void onConnectionMade();
  sigc::connection on_timer_registered(Timing::timer_slot s, int msecs_interval);

  void unadvertise(void *conn, Glib::ustring profile_id, Glib::ustring scenario_id, Glib::ustring &err);
  void sendList(void *conn);
  bool loadAdvertisedGame(Glib::ustring tag, XML_Helper *helper, void *conn);

  //! A static pointer for the singleton instance.
  static GamelistServer * s_instance;
};

#endif
