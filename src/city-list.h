// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2004 John Farrell
// Copyright (C) 2005, 2006 Andrea Paternesi
// Copyright (C) 2007, 2008, 2009, 2014, 2015, 2020, 2021 Ben Asselstine
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
#ifndef CITYLIST_H
#define CITYLIST_H

#include "LocationList.h"
#include <sigc++/trackable.h>

class City;
class Player;
class XML_Helper;

//! A list of City objects on the game map.
/**
 * The citylist keeps track of the city objects located on the game map. It
 * is implemented as a singleton because many classes use it for looking 
 * up cities.
 */
class Citylist : public LocationList<City*>, public sigc::trackable
{
    public:

        //! Makes a copy of the citylist.
        Citylist* copy () {return new Citylist (*this, true);}

	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

	// Methods that operate on class data and modify the class.

        /**
         * This function loops through all cities belonging to player p and
         * processes them.  This method adds the income of each city to
         * the player's treasury and produces a new armies from the City
	 * object's active Army production base if they are due to be created.
         *
         * @param p        The player whose cities are processed.
         */
	//! Process all City objects for the next turn.
        void nextTurn(Player* p);

	//! Changes ownership of all cities owned by old owner, to a new owner.
	void changeOwnership(Player *old_owner, Player *new_owner);

	/**
	 * Scan through all of the City objects who are vectoring to the given
	 * city, and stop the vectoring.
	 *
	 * @param city  The city to search for vectoring to.
	 */
	//! Stops vectoring from any city to the specified city.
	void stopVectoringTo(City *city);


	// Methods that operate on class data and do not modify the class.

	/**
	 * Scan through all of the city objects for a city that is vectoring
	 * to the given city.
	 *
	 * @param target  The city to check if any other cities are vectoring 
	 *                to.
	 *
	 * @return True if another city is vectoring to the given city.
	 *         Otherwise false.
	 */
	//! Return whether or not any cities are vectoring to the given city.
	bool isVectoringTarget(City *target) const;

	//! Return the amount of money that the given player will pay next turn.
	guint32 calculateUpcomingUpkeep(Player *p) const;

	//! Return the number of cities vectoring to the given city.
	guint32 countCitiesVectoringTo(const City *dest) const;

        //! Save the list of City objects to an opened saved-game file.
        bool save(XML_Helper* helper) const;
       
        //! Count the number of functional cities that a given player owns. 
        int countCities(Player* p) const;

	//! Count the total number of inhabitable cities.
	int countCities() const;

        //! Count the number of cities with the default name.
        guint32 countUnamedCities () const;

	//! Have each of the cities owned by the given player, pay gold pieces.
	/**
	 * This method increases the player's treasury.
	 */
	void collectTaxes(Player* p) const;

        //! Returns the closest city that is owned by an enemy player.
	/**
	 * Scans through all of the City objects in the list for the nearest
	 * one that is at war with the active player.  Nearness is determined
         * by counting the tiles between POS and the enemy city.
	 *
	 * @note This method will not return a razed city.
	 *
	 * @param pos  The position on the game map to search for the nearest
	 *             enemy-owned City object from.
	 *
	 * @return A pointer to the nearest enemy-owned City object, or NULL 
	 *         if there aren't any City objects owned by any enemies.
	 */
        City* getNearestEnemyCity(const Vector<int>& pos) const;

        City* getClosestEnemyCity(const Stack *stack) const;

        //! Returns the closest city that isn't owned by the active player.
	/**
	 * Scans through all of the City objects in the list for the nearest
	 * one that owned by a Player who isn't the active player.
	 *
	 * @note This method will not return a razed city.
	 *
	 * @param pos  The position on the game map to search for the nearest
	 *             City object that isn't owned by the active player.
	 *
	 * @return A pointer to the nearest foreign-owned City object, or NULL 
	 *         if there aren't any City objects owned by any other Player.
	 */
	City* getNearestForeignCity(const Vector<int>& pos) const;

        City* getClosestForeignCity(const Stack *stack) const;

        //! Return the closest city owned by the active player.
	/**
	 * Scans through all of the City objects in the list for the nearest
	 * one that owned by the active player from the given position.
	 *
	 * @note This method will not return a razed city.
	 *
	 * @param pos  The position on the game map to search for the nearest
	 *             City object that is owned by the active player.
	 *
	 * @return A pointer to the nearest City object that is owned by the 
	 *         active player, or returns NULL if the active player doesn't 
	 *         own any City objects.
	 */
        City* getNearestFriendlyCity(const Vector<int>& pos) const;

        City* getClosestFriendlyCity(const Stack *stack) const;

        //! Find the closest city owned by the active player and isn't too far.
	/**
	 * Scans through all of the City objects in the list for the nearest
	 * one that owned by the active player and isn't farther away than the
	 * prescribed number of tiles.
	 *
	 * @note This method will not return a razed city.
	 *
	 * @param pos  The position on the game map to search for the nearest
	 *             City object that is owned by the active player.
	 * @param dist The number of tiles away that is deemed "too far".
	 *
	 * @return A pointer to the nearest City object that is owned by the 
	 *         active player, and is within the perscribed number of tiles.
	 *         Returns NULL if the active player doesn't own any City 
	 *         objects within of the prescribed number of tiles.
	 */
        City* getNearestFriendlyCity(const Vector<int>& pos, int dist) const;

        //! Find the closest city to the given position.
	/**
	 * Scans through all of the City objects in the list for the nearest
	 * one that isn't razed.
	 *
	 * @param pos  The position on the game map to search for the nearest
	 *             City object from.
	 *
	 * @return A pointer to the nearest City object.  Returns NULL if there
	 *        are not any City objects in the list.
	 */
        City* getNearestCity(const Vector<int>& pos) const;

        City* getClosestCity(const Stack *stack) const;

        //! Find the closest city that isn't too far away.
	/**
	 * Scans through all of the City objects in the list for the nearest
	 * one that isn't razed and isn't farther away than the presecribed
	 * number of tiles.
	 *
	 * @param pos  The position on the game map to search for the nearest
	 *             City object from.
	 * @param dist The number of tiles away that is deemed "too far".
	 *
	 * @return A pointer to the nearest city that is not razed and is 
	 *         within the prescribed number of tiles.  Returns NULL if
	 *         no city could be found.
	 */
        City* getNearestCity(const Vector<int>& pos, int dist) const;

        //! Find the nearest city that is not obscured by fog.
	/**
	 * Scan through all cities, searching for the closest one that is
	 * not covered by fog-of-war on a hidden map.
	 *
	 * @note This method will not return a city that has been razed.
	 *
	 * @param pos  The position to find the nearest city from.
	 *
	 * @return A pointer to the nearest city that is not obscured by fog.
	 */
        City* getNearestVisibleCity(const Vector<int>& pos) const;

	//! Find the nearest city that is unobscured and is not too far away.
	/**
	 * Scan through all the cities, searching for the closest one that
	 * is not covered by fog-of-war on a hidden map, but is not farther
	 * away than a given distance.
	 *
	 * @note This method will not return a city that has been razed.
	 *
	 * @param pos  The position to find the nearest city from.
	 * @param dist The number of tiles away that is deemed "too far".
	 *
	 * @return A pointer to the nearest city that is not obscured by fog 
	 *         and is within the prescribed number of tiles.  Returns NULL 
	 *         if no city could be found.
	 */
        City* getNearestVisibleCity(const Vector<int>& pos, int dist) const;

        //! Return the closest city owned by the given player.
	/**
	 * Scans through all of the City objects in the list for the nearest
	 * one that owned by the given player to the given position.
	 *
	 * @note This method will not return a razed city.
	 *
	 * @param pos    The position on the game map to search for the nearest
	 *               City object that is owned by the given player.
	 * @param player The player who owns the City object this method is
	 *               searching for.
	 *
	 * @return A pointer to the nearest City object that is owned by the 
	 *         given player, or returns NULL if the given player doesn't 
	 *         own any City objects.
	 */
	City* getNearestCity(const Vector<int>& pos, Player *player) const;

        City* getClosestCity(const Stack *stack, Player *p) const;

        //! Return the closest city owned by the neutral player.
	/**
	 * Scans through all of the City objects in the list for the nearest
	 * one that owned by the neutral player to the given position.
	 *
	 * @note This method will not return a razed city.
	 *
	 * @param pos    The position on the game map to search for the nearest
	 *               City object that is owned by the neutral player.
	 * @param player The player who owns the City object this method is
	 *               searching for.
	 *
	 * @return A pointer to the nearest City object that is owned by the 
	 *         neutral player, or returns NULL if the neutral player 
	 *         doesn't own any City objects.
	 */
        City* getNearestNeutralCity(const Vector<int>& pos) const;

        //! Find the nearest unobscured city that is owned by the active player.
	/**
	 * Scan through all cities, searching for the closest one that is
	 * not covered by fog-of-war on a hidden map and is owned by the
	 * active player.
	 *
	 * @note This method will not return a city that has been razed.
	 *
	 * @param pos  The position to find the nearest city from.
	 *
	 * @return A pointer to the nearest city that is not obscured by fog,
	 *         and is owned by the active player.
	 */
        City* getNearestVisibleFriendlyCity(const Vector<int>& pos) const;

        //! Get the nearest unfogged city of active player's that isn't too far.
	/**
	 * Scan through all cities, searching for the closest one that is
	 * not covered by fog-of-war on a hidden map and is owned by the
	 * active player, and is within the prescribed number of tiles.
	 *
	 * @note This method will not return a city that has been razed.
	 *
	 * @param pos  The position to find the nearest city from.
	 * @param dist The number of tiles away that is deemed "too far".
	 *
	 * @return A pointer to the nearest city that is not obscured by fog,
	 *         and is owned by the active player and is within the 
	 *         prescribed number of tiles.  Returns NULL if no city 
	 *         fitting those parameters could be found.
	 */
        City* getNearestVisibleFriendlyCity(const Vector<int>& pos, int dist) const;

	//! Return the list of cities vectoring to the given city.
	/**
	 * Scan through all of the city objects for cities that are vectoring
	 * to the given city.  Add pointers to these cities to a list.
	 *
	 * @param target  The city to check if any other cities are vectoring 
	 *                to.
	 *
	 * @return The list of pointers to city objects that are vectoring to
	 *         the given city.
	 */
	std::list<City*> getCitiesVectoringTo(City *target) const;

	//! Return the list of cities vectoring to the given spot on the map.
	std::list<City*> getCitiesVectoringTo(Vector<int> target) const;

	//! Get the nearest city that is farther than a given number tiles.
	/**
	 * Scans through all of the City objects in the list for the nearest
	 * one that isn't razed and is farther away then the given number of
	 * tiles.
	 *
	 * @param pos  The position on the game map to search for the nearest
	 *             City object from.
	 * @param dist The distance that is deemed "too close".
	 *
	 * @return A pointer to the nearest City object that is farther away
	 *         than the given distance.  Returns NULL if there are no 
	 *         cities that are farther away than the given distance.
	 */
	City* getNearestCityPast(const Vector<int>& pos, int dist) const;


	//! Get the nearest city to POS that can be vectored to.
	City* getNearestFriendlyVectorableCity(const Vector<int>& pos) const;

        //! Get our nearest cities in order of distance from our capital.
        std::list<City*> getNearestFriendlyCities(Player *player, Vector<int> pos = Vector<int>(-1,-1)) const;

        //! Get the capital city of the given player.
        City *getCapitalCity(Player *player) const;

        //! Pick a random city for a hero to be recruited in.
        City *getRandomCityForHero(Player *player) const;

	// Static Methods

        //! Returns the singleton instance.  Creates a new one if neccessary.
        static Citylist* getInstance();

        //! Loads the singleton instance from an opened saved-game file.
        static Citylist* getInstance(XML_Helper* helper);

        //! Deletes the singleton instance.
        static void deleteInstance();

        //! Replace the current citylist with another.
        static void reset (Citylist *c);
        
        //! Destructor
        ~Citylist ();

    protected:

	//! Default constructor.
        Citylist();

	//! Loading constructor.
	/**
	 * Make a new Citylist object by reading it from an opened saved-game 
	 * file.
	 *
	 * @param helper  The opened saved-game file to load the list of City
	 *                objects from.
	 */
        Citylist(XML_Helper* helper);

        //! Copy constructor
        Citylist (const Citylist &c, bool sync_ids = false);

    private:
        //! A callback for loading City objects into the list of cities.
        bool load(Glib::ustring tag, XML_Helper* helper);

        //! A static pointer for the singleton instance.
        static Citylist* s_instance;
};

#endif // CITYLIST_H

// End of file
