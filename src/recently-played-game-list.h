//  Copyright (C) 2008, 2011, 2014, 2026 Ben Asselstine
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
#ifndef RECENTLYPLAYEDGAMELIST_H
#define RECENTLYPLAYEDGAMELIST_H

#include <gtkmm.h>
#include <list>
#include <sigc++/trackable.h>

class XML_Helper;
class GameScenario;
class RecentlyPlayedGame;
class Profile;

//! A list of games that we've recently played.
/** 
 * This is only used for network games at the moment.
 * It is implemented as a singleton, but this class may also exist outside
 * of the the singleton.
 * The singleton refers to the recently played games file.
 * Recently played game lists can also be used in the game list server, to
 * indicate a list of advertised games that others can connect to.
 *
 */
class RecentlyPlayedGameList: public std::list<RecentlyPlayedGame*>, public sigc::trackable
{
    public:

	//! The xml tag of this object in a recently played game file.
	static Glib::ustring d_tag; 
	
	static const int TWO_WEEKS_OLD = 1209600; /* seconds */


	// Methods that operate on the class data and do not modify the class.

        //! Save recently played game list to the recently played games file.
        bool save() const;

	//! Save the recently played game list to an opened file.
	bool save(XML_Helper* helper) const;


	// Methods that operate on the class data and modify the class.

	//! Load the recently game list from the recently played games file.
        bool load();


	//! Add a game entry to the list of recently played games.
	void addEntry(GameScenario *game_scenario, Profile *p, Glib::ustring filename);

	//! Add a networked game entry to the list of recently played games.
	void addNetworkedEntry(GameScenario *game_scenario, Profile *p, Glib::ustring host, guint32 port);

	//! Touch the game in the recently played list.
	void updateEntry(GameScenario *game_scenario);

	//! Removes all networked games from the list.
	void removeAllNetworkedGames();

	//! Removes games from the list that are too old, or just too numerous.
	void pruneGames(int max_number_of_games = 10);

	// Static Methods

        //! return the singleton instance of this class.
        static RecentlyPlayedGameList * instance();

        //! Loads the singleton instance from an opened file.
        static RecentlyPlayedGameList * instance(XML_Helper *helper);

        //! Explicitly delete the singleton instance of this class.
        static void deleteInstance();

        //! Rewrite an old recently played game list file.
        static bool upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version);
        static void support_backward_compatibility();

        //! Default Constructor.
        RecentlyPlayedGameList();

	//! Loading constructor
        RecentlyPlayedGameList(XML_Helper *helper);
        
        //! Destructor.
        ~RecentlyPlayedGameList();

    private:
        //! Callback for loading recentlyplayedgames into this list.
	bool load_tag(Glib::ustring tag, XML_Helper *helper);

	//! Helper method to sort the list by it's last-played time.
	static bool orderByTime(RecentlyPlayedGame*rhs, RecentlyPlayedGame *lhs);

	//! Remove the old games from the list.
	void pruneOldGames(int stale = TWO_WEEKS_OLD);

	//! Remove extraneous games from the list.
	void pruneTooManyGames(int too_many = 10);

        void pruneSameNamedAndSameHostGames();

        void pruneGamesBelongingToRemovedProfiles();
	//! Load the recently game list from the given file.
	bool loadFromFile(Glib::ustring filename);

	//! Save the recently played game list to the given file.
	bool saveToFile(Glib::ustring filename) const;


        void remove_all();
	// DATA

        //! A static pointer for the singleton instance.
        static RecentlyPlayedGameList* s_instance;
};

#endif
