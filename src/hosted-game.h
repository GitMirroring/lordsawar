//  Copyright (C) 2011, 2014, 2026 Ben Asselstine
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
#ifndef HOSTED_GAME_H
#define HOSTED_GAME_H

#include <gtkmm.h>


#include <sys/time.h>

class XML_Helper;
class AdvertisedGame;

//! A single game in the gamelist, either advertised or hosted.
/**
 *
 */
class HostedGame
{
    public:

        static Glib::ustring d_tag;
	//! Make a new hosted game entry.
	HostedGame(AdvertisedGame *advertised_game);

	//! Load a new hosted game entry from an opened file.
	HostedGame(XML_Helper *helper);

	//! Destroy a hosted game entry.
	~HostedGame();


        // Set Methods
        void setAdvertisedGame(AdvertisedGame *g) {d_advertised_game = g;};
        void setUnresponsive(bool resp) {unresponsive = resp;};
        void setPid(guint32 pid) {d_pid = pid;};

	// Get Methods
        AdvertisedGame * getAdvertisedGame() const {return d_advertised_game;};
        bool getUnresponsive() const {return unresponsive;};
        guint32 getPid() const {return d_pid;};

	// Methods that operate on the class data but do not modify it.

	//! Save the hosted game entry to an opened file.
        bool save(XML_Helper* helper) const;

	// Methods that operate on the class data and modify it.

        void ping();

        // Signals
  
        sigc::signal<void(HostedGame*)> cannot_ping_game;

    private:

	// DATA
        
        bool loadAdvertisedGame(Glib::ustring tag, XML_Helper *helper);
        void on_pinged(bool success);

        AdvertisedGame *d_advertised_game;
        guint32 d_pid;
	
        bool unresponsive;
    
};

#endif
