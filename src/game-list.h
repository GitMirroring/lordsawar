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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#pragma once
#ifndef GAMELIST_H
#define GAMELIST_H

#include <gtkmm.h>
#include <list>
#include <sigc++/trackable.h>

class AdvertisedGame;
class HostedGame;
class RecentlyPlayedGameList;

//! A list of games that we've recently hosted or advertised.
/** 
 * It is implemented as a singleton.
 *
 */
class Gamelist: public std::list<HostedGame*>, public sigc::trackable
{
    public:

        //! Re-ping games that haven't been successfully pinged in 5 mins.
        static const int FIVE_MINUTES_OLD = 60 * 5;

	//! The xml tag of this object in a game list file.
	static Glib::ustring d_tag; 
	
	static const int TEN_DAYS_OLD = 864000; /* seconds */

	static const int MAX_NUMBER_OF_ADVERTISED_GAMES = 100;

	// Methods that operate on the class data and do not modify the class.

        //! Save game list to the game list file.
        bool save() const;

	//! Save the game list to the given file.
	bool saveToFile(Glib::ustring filename) const;

	//! Save the game list to an opened file.
	bool save(XML_Helper* helper) const;

        //! Get the list, with some identifying information removed.
        RecentlyPlayedGameList* getList(bool scrub_profile_id = true) const;

        HostedGame *findGameByScenarioId(Glib::ustring scenario_id) const;

	// Methods that operate on the class data and modify the class.

        bool add(HostedGame *g);

	//! Load the game list from the games file.
        bool load();

	//! Load the game list from the given file.
	bool loadFromFile(Glib::ustring filename);

	//! Add an entry to the list of games.
	void addEntry(AdvertisedGame *advertised_game);

	//! Touch the game in the games list.
	void updateEntry(Glib::ustring scenario_id, guint32 round);

	//! Removes games from the list that are too old, or just too numerous.
	void pruneGames();
	
        void pingGames();

	// Static Methods

        //! return the singleton instance of this class.
        static Gamelist * getInstance();

        //! Loads the singleton instance from an opened file.
        static Gamelist * getInstance(XML_Helper *helper);

        //! Explicitly delete the singleton instance of this class.
        static void deleteInstance();

        //! Rewrite an old file.
        static bool upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version);
        static void support_backward_compatibility();

    protected:
        //! Default Constructor.
        Gamelist();

	//! Loading constructor
        Gamelist(XML_Helper *helper);
        
        //! Destructor.
        ~Gamelist();

    private:
        //! Callback for loading the games into this list.
	bool load_tag(Glib::ustring tag, XML_Helper *helper);

	//! Helper method to sort the list by it's last-played time.
	static bool orderByTime(HostedGame*rhs, HostedGame *lhs);

	//! Remove the old games from the list.
	void pruneOldGames(int stale = TEN_DAYS_OLD);

        void pruneTooManyGames(int too_many = MAX_NUMBER_OF_ADVERTISED_GAMES);
  
        void pruneUnresponsiveGames();


        void remove_all();

        void on_could_not_ping_game(HostedGame *game);

	// DATA

        //! A static pointer for the singleton instance.
        static Gamelist* s_instance;
};

#endif // GAMELIST_H

