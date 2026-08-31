//  Copyright (C) 2020, 2026 Ben Asselstine
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
#ifndef SCENARIO_DETAILS_H
#define SCENARIO_DETAILS_H

#include <gtkmm.h>


#include <sys/time.h>

#include "game-scenario.h"

//! A single entry in the scenario list.
/**
 * Holds the name, id, etc of the scenario along with the filename.
 * It's similar to a RecentlyPlayedGame object but there's no backing store.
 *
 */
class ScenarioDetails
{
    public:

	//! Default constructor.
        ScenarioDetails (Glib::ustring id, guint32 num_cities, 
                         guint32 num_players, Glib::ustring name,
                         Glib::ustring desc, Glib::ustring filename);

        //! Loading constructor.
        /**
         * Read in the scenario file to populate the data members.
         * When there's something wrong with the file, broken is set
         * to true.
         */
        ScenarioDetails (Glib::ustring filename, bool &broken);

	//! Destructor.
        ~ScenarioDetails() {}

	// Get Methods

        //! Get the scenario id of the recently played game entry.
	Glib::ustring get_id() const {return d_id;}

	//! Get the number of cities in the game.
	guint32 get_number_of_cities () const {return d_number_of_cities;}

	//! Get the number of players in the game.
	guint32 get_number_of_players () const {return d_number_of_players;}

	//! Get the name of the scenario.
	Glib::ustring get_name () const {return d_name;}

	//! Get the description of the scenario.
	Glib::ustring get_description () const {return d_desc;}

	//! Get the name of the scenario.
	Glib::ustring get_filename () const {return d_filename;}

    protected:

	// DATA
	
	//! The id of the game.
	Glib::ustring d_id;

	//! How many cities the game has.
	guint32 d_number_of_cities;

	//! How many players the game had at the start of the game.
	guint32 d_number_of_players;

	//! The name of the game.
	Glib::ustring d_name;

	//! The description of the game.
	Glib::ustring d_desc;

	//! The filename of the map.
	Glib::ustring d_filename;
};

#endif
