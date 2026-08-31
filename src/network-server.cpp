//  Copyright (C) 2008 Ole Laursen
//  Copyright (C) 2008, 2011, 2015, 2017, 2026 Ben Asselstine
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

#include <stdio.h> //benfix
#include "network-server.h"
#include <iostream>
#include <sigc++/bind.h>
#include <giomm.h>

#include "network-common.h"
#include "network-connection.h"
#include "connection-manager.h"

NetworkServer::NetworkServer()
{
  connections.clear();
}

NetworkServer::~NetworkServer()
{
  for (auto i: connections)
    i->tear_down_connection();
  connections.clear ();
  if (server)
    {
      if (server->is_active())
        {
        server->stop();
        }
    }
}

void NetworkServer::startListening(int port)
{
  server = Gio::SocketService::create();

  server->signal_incoming().connect(sigc::hide(sigc::mem_fun(*this, &NetworkServer::gotClientConnection)), false);
  try 
    {
      server->add_inet_port (port);
    }
  catch(const Glib::Error &ex)
    {
      server->stop();
      port_in_use.emit(port);
      return;
    }

  server->start();
}

void NetworkServer::send(void *c, int type, const Glib::ustring &payload)
{
  NetworkConnection *conn = static_cast<NetworkConnection *>(c);
  conn->send(type, payload);
}

void NetworkServer::sendFile(void *c, int type, const Glib::ustring &payload)
{
  NetworkConnection *conn = static_cast<NetworkConnection *>(c);
  conn->sendFile(type, payload);
}

bool NetworkServer::gotClientConnection(const Glib::RefPtr<Gio::SocketConnection>& c)
{
  if (c) 
    {
      c->reference();
      NetworkConnection *conn = ConnectionManager::create_connection(c);
      connections.push_back(conn);

      conn->connection_lost.connect
        (sigc::bind(sigc::mem_fun
                    (connection_lost, 
                     &sigc::signal<void(void *)>::emit), conn));

      connection_made.emit(conn);
      
      conn->got_message.connect
        (sigc::bind<0>(sigc::mem_fun(got_message, 
                                     &sigc::signal<bool(void *, 
                                     int, Glib::ustring)>::emit), conn));

      return true;
    }
  return false;
}

void NetworkServer::onConnectionLost(void *conn)
{
  NetworkConnection *c = reinterpret_cast<NetworkConnection*>(conn);
  if (std::find (connections.begin(), connections.end(), c) != connections.end())
    connections.remove(c);
}
  
bool NetworkServer::isRunning()
{
  return server->is_active();
}

void NetworkServer::stop()
{
  server->close();
  return server->stop();
}
  
Glib::ustring NetworkServer::get_hostname(void *c) 
{
  NetworkConnection *conn = static_cast<NetworkConnection *>(c);
  return conn->get_peer_hostname();
}
  
bool NetworkServer::is_local_connection(void *conn)
{
  if (get_hostname(conn) == "127.0.0.1")
    return true;
  return false;
}
