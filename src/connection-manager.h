//  Copyright (C) 2015, 2026 Ben Asselstine
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
#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include <list>
#include <gtkmm.h>
#include <map>
#include <thread>

//! A store of NetworkConnection objects.
/**
 * this class deletes network-connection objects as they finish processing
 * messages.
 *
 */
class NetworkConnection;

class ConnectionManager: public std::list<NetworkConnection*>
{
    public:


	// Static Methods

        static NetworkConnection *create_connection(const Glib::RefPtr<Gio::SocketConnection> &c);
        static NetworkConnection *create_connection();
        //! Returns the singleton instance. Creates a new one if required.
        static ConnectionManager* instance();

        //! Explicitly deletes the singleton instance.
        static void deleteInstance();

        //! Go get a builder object by name.
        static void manage(NetworkConnection*conn);

    protected:    

	// Constructor.
        ConnectionManager();

	//! Destructor.
        ~ConnectionManager();

    private:

        void on_messages_flushed (NetworkConnection *conn);

        void launch_thread(NetworkConnection *nc);
        void join(NetworkConnection *nc);
	// DATA

        static ConnectionManager * s_instance;
        std::map<NetworkConnection*,std::thread*> threads;

};

#endif
