//  Copyright (C) 2015 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>

#include <algorithm>
#include "connection-manager.h"
#include "network-connection.h"

ConnectionManager* ConnectionManager::s_instance = 0;

ConnectionManager* ConnectionManager::getInstance()
{
    if (s_instance == 0)
        s_instance = new ConnectionManager();

    return s_instance;
}

void ConnectionManager::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

ConnectionManager::ConnectionManager()
{
}

ConnectionManager::~ConnectionManager()
{
  for (iterator i = begin(); i != end(); ++i)
    (*i)->tear_down_connection();
}

void ConnectionManager::manage(NetworkConnection*conn)
{
  getInstance()->push_back(conn);
  conn->queue_flushed.connect(sigc::bind(sigc::mem_fun(getInstance(), &ConnectionManager::on_messages_flushed), conn));
}

NetworkConnection *ConnectionManager::create_connection(const Glib::RefPtr<Gio::SocketConnection> &c)
{
  NetworkConnection *nc = new NetworkConnection(c);
  Glib::signal_idle().connect_once(sigc::bind(sigc::mem_fun(*getInstance(), &ConnectionManager::launch_thread), nc));
  ConnectionManager::manage (nc);
  return nc;
}

NetworkConnection *ConnectionManager::create_connection()
{
  NetworkConnection *nc = new NetworkConnection();
  Glib::signal_idle().connect_once(sigc::bind(sigc::mem_fun(*getInstance(), &ConnectionManager::launch_thread), nc));

  ConnectionManager::manage (nc);
  return nc;
}

void ConnectionManager::launch_thread(NetworkConnection *nc)
{
  std::thread * consumer = new std::thread
    ([nc]
      {
        nc->send_queued_messages();
      });
  getInstance()->threads[nc] = consumer;
}

void ConnectionManager::on_messages_flushed(NetworkConnection*conn)
{
  iterator i = std::find(begin(), end(), conn);
  if (i != end())
    erase(i);
  if (threads[conn])
    Glib::signal_idle().connect_once(sigc::bind(sigc::mem_fun(this, &ConnectionManager::join), conn));
}

void ConnectionManager::join(NetworkConnection *nc)
{
  threads[nc]->join();
  delete nc;
}
// End of file
