//  Copyright (C) 2011, 2014 Ben Asselstine
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
#ifndef ADVERTISED_GAME_H
#define ADVERTISED_GAME_H

#include <gtkmm.h>
#include <sys/time.h>

#include "recently-played-game.h"
class XML_Helper;
class Profile;
class NetworkConnection;

//! A single game entry in the advertised games list.
/**
 *
 */
class AdvertisedGame : public RecentlyPlayedNetworkedGame
{
    public:

        static Glib::ustring d_tag_name;
	//! Make a new advertised game entry.
	AdvertisedGame(GameScenario *game_scenario, Profile *p);

	AdvertisedGame(const RecentlyPlayedNetworkedGame &orig, Profile *p);

        //! Copy constructor
	AdvertisedGame(const AdvertisedGame &orig);

	//! Load a new advertised game entry from an opened file.
	AdvertisedGame(XML_Helper *helper);

	//! Destroy an advertised game entry.
	~AdvertisedGame();


	// Get Methods
        Glib::DateTime getGameCreatedOn() const {return d_creation_date;};
        Glib::DateTime getGameLastPingedOn() const {return d_last_pinged_date;};

        Profile * getProfile() const {return d_profile;};
	
	// Methods that operate on the class data but do not modify it.

	//! Save the advertised game entry to an opened file.
	virtual bool doSave(XML_Helper *helper) const;

        bool saveEntry(XML_Helper* helper) const;


	// Methods that operate on the class data and modify it.
        void ping();

        //signals
  
        sigc::signal<void(bool)> pinged;

    private:

	// DATA
        
        Glib::DateTime d_creation_date;
        Glib::DateTime d_last_pinged_date;
        Profile *d_profile;

        bool loadProfile(Glib::ustring tag, XML_Helper *helper);

        void on_connected_to_game(NetworkConnection *conn);
        void on_could_not_connect_to_game(NetworkConnection *conn);
	
};

#endif
