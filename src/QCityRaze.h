//  Copyright (C) 2007, 2008, 2009, 2014, 2021 Ben Asselstine
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
#ifndef QUEST_CITY_RAZE_H
#define QUEST_CITY_RAZE_H

#include <sigc++/trackable.h>

#include <list>
#include "Quest.h"

class City;
class XML_Helper;

//! A Quest where the Hero must raze a City owned by another Player.
/**
 * A hero that receives this quest has to burn a specific city to fulfill
 * it.  The Quest is completed when this happens, but the quest is expired if
 * the user conquers the correct city but forgets to raze the city.
 */
class QuestCityRaze: public Quest, public sigc::trackable
{
    public:
	//! Default constructor.
	/**
	 * Make a new city burning quest.
	 *
	 * @param q_mgr  The quests manager to associate this quest with.
	 * @param hero   The Id of the Hero who is responsible for the quest.
	 */
        QuestCityRaze(QuestsManager& q_mgr, guint32 hero);

        //! Copy constructor.
        QuestCityRaze (const QuestCityRaze &q);

        //! Destructor.
        ~QuestCityRaze() {};

	//! Loading constructor.
	/**
	 * @param q_mgr   The quests manager to associate this quest with.
	 * @param helper  The opened saved-game file to load this quest from.
	 */
        QuestCityRaze(QuestsManager& q_mgr, XML_Helper* helper);

        // Construct from remote action.
        QuestCityRaze(QuestsManager& q_mgr, guint32 hero, guint32 target);
        

	// Get Methods

        /**
         * \brief Get progress information 
         *
         * \param s here we append the progress information
         */
        Glib::ustring getProgress() const;

	//! Return a description of how well the city razing quest is going.
        void getSuccessMsg(std::queue<Glib::ustring>& msgs) const;

	//! Return a queue of strings to show when the quest is compeleted.
        void getExpiredMsg(std::queue<Glib::ustring>& msgs) const;

        //! Returns the id of the City object to be razed.
        guint32 getCityId() const {return d_city;}


	// Methods that operate on the class data but do not modify the class.

        //! Returns a pointer to the City object to be razed.
        City* getCity() const;

        //! Saves the occupy quest data to an opened saved-game file.
        bool save(XML_Helper* helper) const;


	// Methods that need to be implemented from the superclass.

	//! Callback for when an Army object is killed.
	/**
	 * @note This method is not used.
	 */
	void armyDied(Army *a, bool heroIsCulprit);

	//! Callback for when a City object is defeated.
	/**
	 * This method notifies the Quest that a City has fallen, and what the 
	 * conquering action (pillage/sack/raze/occupy) was.  It also notifies
	 * whether or not the hero responsible for this quest was involved in 
	 * the conquering, and how much gold was taken as a result.
	 *
	 * If the city isn't razed then the Quest is expired.
	 * If the city is razed then the Quest is completed.
	 *
	 * @param city           The City object that has been conquered.
	 * @param action         What action was taken by the Player.  See
	 *                       CityDefeatedAction for more information.
	 * @param heroIsCulprit  Whether or not the Hero object associated with
	 *                       this Quest object is responsible for 
	 *                       conquering the given City object.
	 * @param gold           How many gold pieces were taken as a result
	 *                       of the action.
	 */
	void cityAction(City *city, CityDefeatedAction action, 
			bool heroIsCulprit, int gold);

	// Static Methods

	//! Returns whether or not this quest is impossible.
        /**
	 * Scans all City objects in the Citylist to see if there is one the 
	 * active player can raze.
	 *
	 * @note This method is static because it is executed before the
	 *       Quest is instantiated.  It is also called from within the
	 *       instantiated Quest.
	 *
	 * @param heroId  The Id of the Hero responsible for the razing quest.
	 *
	 * @return Whether or not the quest is possible.
         */
        static bool isFeasible(guint32 heroId);

    private:

	//! Make a quest description about the city that needs to be razed.
        void initDescription();
         
	//! Return a pointer to a random city not owned by the given player.
	/**
	 * Find a city to raze.
	 *
	 * Scan through all of the City objects in the Citylist for a city
	 * that is not owned by the given player or by neutral.  Pick a random
	 * one that isn't already razed and return it.
	 *
	 * @param player  The player whose City objects are exempt from being
	 *                selected as a target for razing.
	 *
	 * @return A pointer to a City object that can be razed by the Hero.
	 *         If no valid City objects are found, this method returns NULL.
	 */
        static City* chooseToRaze(Player *p);

	//! The Id of the target City object to raze.
        guint32 d_city;

};

#endif
