//  Copyright (C) 2008 Ole Laursen
//  Copyright (C) 2008, 2011, 2014, 2015, 2026 Ben Asselstine 
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
#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#include <config.h>

#include <map>
#include <giomm.h>
#include <giomm/socketservice.h>
#include <glibmm.h>
#include <sigc++/signal.h>

#include "network-common.h"

class NetworkConnection;

//! A helper class to GameServer.
class NetworkServer
{
public:
  NetworkServer();
  ~NetworkServer();

  bool isRunning();
  void startListening(int port);
  void send(void *conn, int type, const Glib::ustring &payload);
  void sendFile(void *c, int type, const Glib::ustring &payload);
  Glib::ustring get_hostname(void *conn);
  bool is_local_connection(void *conn);

  void stop();
  sigc::signal<bool(void *, int, Glib::ustring)> got_message;
  sigc::signal<void(void *)> connection_made;
  sigc::signal<void(void *)> connection_lost;
  sigc::signal<void(int)> port_in_use;
  
  // private callback
  bool gotClientConnection(const Glib::RefPtr<Gio::SocketConnection>& c);

  void onConnectionLost(void *conn);
private:
  
  Glib::RefPtr<Gio::SocketService> server;
  std::list<NetworkConnection *> connections;
};

#endif
