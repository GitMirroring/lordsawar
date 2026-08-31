// Copyright (C) 2003, 2005 Ulf Lorenz
// Copyright (C) 2004 Andrea Paternesi
// Copyright (C) 2007, 2008, 2009, 2014, 2021 Ben Asselstine
// Copyright (C) 2007, 2008 Ole Laursen
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
#ifndef QUEST_MANAGER_H
#define QUEST_MANAGER_H

#include <queue>
#include <map>
#include <vector>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include "callback-enums.h"

#include "player.h"

class Quest;
class XML_Helper;
class Army;
class Reward;


//! Manages Quest objects.
/**
 * This class creates new quests and assigns them to heroes.  It also keeps 
 * track of pending quests and invalidates quests due to Hero death.  It
 * acts as a central place to catch army death events, and city conquered
 * events, and it passes these events on to the Quest objects it manages.
 *
 * This object equates to the lordsawar.questlist XML entity in the saved-game
 * file.
 *
 * This class is implemented as a singleton.
 *
 */
class QuestsManager : public sigc::trackable
{
    public:

	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

        //! Makes a copy of the quests manager.
        QuestsManager* copy () {return new QuestsManager (*this);}

	// Methods that operate on the class data and modify it.

	//! Create a new quest for the given hero.
	/**
	 * Check and see which quests are possible and give the specified hero
	 * a random quest.
	 *
	 * @param heroId  The Id of the Hero object to be responsible for the
	 *                new Quest.
	 * @param razing_possible  Whether or not razing cities is allowed in
	 *                         the scenario.  If this is set to true, the
	 *                         quests that involve razing will be
	 *                         considered.  Otherwise a razing quest will
	 *                         not be considered at all, and not returned.
	 *
	 * @return A pointer to the new Quest object.
	 */
        Quest* createNewQuest(guint32 heroId, bool razing_possible);

        //! Create new kill hero quest from remote action. 
	Quest* createNewKillHeroQuest(guint32 heroId, guint32 targetHeroId);

        //! Create new enemy armies quest from remote action. 
	Quest* createNewEnemyArmiesQuest(guint32 heroId, guint32 num_armies, 
					 guint32 victim_player_id);

        //! Create new city sacking quest from remote action. 
	Quest* createNewCitySackQuest(guint32 heroId, guint32 cityId);

        //! Create new city razing quest from remote action. 
	Quest* createNewCityRazeQuest(guint32 heroId, guint32 cityId);

        //! Create new city occupation quest from remote action. 
	Quest* createNewCityOccupyQuest(guint32 heroId, guint32 cityId);

        //! Create new kill enemy army type quest from remote action. 
	Quest* createNewEnemyArmytypeQuest(guint32 heroId, guint32 armyTypeId);

        //! Create new pillage gold quest from remote action. 
	Quest* createNewPillageGoldQuest(guint32 heroId, guint32 amount);
        
	//! Mark the Quest that the given Hero object is on to be completed.
        /**
         *  This method deactivates the quest and saves the completion 
	 *  notification message which will be presented to the player.
	 *
	 *  @param heroId  The id of the Hero object who has a Quest that we
	 *                 want to mark as complete.
         */
        void questCompleted(guint32 heroId);

	//! Mark the Quest that the given Hero object is on to be expired.
        /** 
         *  This method deactivates the quest and saves the expiry notification
         *  message which will be presented to the player.
	 *
	 *  @param heroId  The id of the Hero object who has a Quest that we
	 *                 want to mark as expired.
         */
        void questExpired(guint32 heroId);

	//! Callback when an Army object is killed.
        /** 
         *  Here we account for a dead army.   maybe it's our hero,
	 *  maybe it's a target hero, maybe we're somebody else's target hero
	 *  or maybe we're some other army we're supposed to kill.
	 *
	 *  @param army      The army who was killed.
	 *  @param culprits  The list of Army object Ids that were involved in
	 *                   killing the given army.
         */
	void armyDied(Army *army, std::vector<guint32>& culprits);

	//! Callback when a city is razed.
	/**
	 * Other classes call this to trigger this razing event.  Derived 
	 * classes of Quest catch this event via the Quest::cityAction 
	 * callback.
	 *
	 * @param city   A pointer to the City object being razed.
	 * @param stack  A pointer to the stack that conquered and is razing
	 *               the given city.
	 */
	void cityRazed(City *city, Stack *stack);

	//! Callback when a city is sacked.
	/**
	 * Other classes call this to trigger this sacking event.  Derived 
	 * classes of Quest catch this event via the Quest::cityAction 
	 * callback.
	 *
	 * @param city   A pointer to the City object being sacked.
	 * @param stack  A pointer to the stack that conquered and is sacking
	 *               the given city.
	 * @param gold   The number of gold pieces that the sacking resulted in.
	 */
	void citySacked(City *city, Stack *stack, int gold);

	//! Callback when a city is pillaged.
	/**
	 * Other classes call this to trigger this pillaging event.  Derived 
	 * classes of Quest catch this event via the Quest::cityAction 
	 * callback.
	 *
	 * @param city   A pointer to the City object being pillaged.
	 * @param stack  A pointer to the stack that conquered and is pillaging
	 *               the given city.
	 * @param gold   The number of gold pieces that the pillaging resulted 
	 *               in.
	 */
	void cityPillaged(City *city, Stack *stack, int gold);

	//! Callback when a city is occupied.
	/**
	 * Other classes call this to trigger this occupying event.  Derived 
	 * classes of Quest catch this event via the Quest::cityAction 
	 * callback.
	 *
	 * @param city   A pointer to the City object being occupied.
	 * @param stack  A pointer to the stack that conquered and is occupying
	 *               the given city.
	 */
	void cityOccupied(City *city, Stack *stack);

	//! Process the Quests at the start of every turn for the given player.
	/**
	 * @param player  The player to process Quest objects for.  The Hero
	 *                object must be owned by this player to be processed.
	 */
	void nextTurn(Player *player);


	// Methods that operate on the class data and do not modify it.

	//! Return a list of active Quest objects that belong to the player.
	/**
	 * @param player  The player to get active Quest objects for.
	 */
        std::vector<Quest*> getPlayerQuests(const Player *player) const;

        Quest* getHeroQuest(guint32 hero_id) const;

	//! Save the quests to an opened saved-game file.
	/**
	 * Saves the lordsawar.questlist XML entity to the saved-game file.
	 *
	 * @param helper  The opened saved-game file to save the Quest objects
	 *                to.
	 */
        bool save(XML_Helper* helper) const;
	

	// Signals

	//! Emitted when a Hero object completes a Quest.
	/**
	 * @param quest   A pointer to the Quest object that was successfully 
	 *                completed.
	 * @param reward  A pointer to the reward that the Hero is receiving.
	 */
	sigc::signal<void, Quest *, Reward *> quest_completed;

	//! Emitted when a Hero object fails to complete a Quest.
	/**
	 * @param quest  A pointer to the Ques tobject that was expired.
	 */
	sigc::signal<void, Quest *> quest_expired;


	// Static Methods

        //! Gets the singleton instance or creates a new one.
        static QuestsManager* getInstance();

	/**
	 * Make a new QuestsManager object by loading all Quest objects from
	 * an opened saved-game file.
	 *
	 * @param helper     The opened saved-game file to read from.
	 *
	 * @return A pointer to the new QuestsManager object.
	 */
        //! Loads the questlist from a saved-game file.
        static QuestsManager* getInstance(XML_Helper* helper);

        //! Explicitly deletes the singleton instance.
        static void deleteInstance();

        //! Replace the current quests manager  with another.
        static void reset (QuestsManager *q);
    protected:

	//! Default constructor.
        QuestsManager();

        //! Copy constructor.
        QuestsManager(const QuestsManager &a);

	//! Loading constructor.
	/**
	 * Loads the lordsawar.questlist XML entity from the saved-game file.
	 *
	 * @param helper  The opened saved-game file to load the questlist from.
	 */
        QuestsManager(XML_Helper* helper);

	//! Destructor.
	~QuestsManager();

    private:

	//! Definition of a function pointer for the Quest::isFeasible methods.
        typedef bool (*QFeasibilityType)(guint32);

	//! Callback for loading Quest objects into the QuestsManager.
        bool load(Glib::ustring tag, XML_Helper* helper);

        //! Does some setup that has to be done on loading as well as creation.
        void sharedInit();

        //! Deactivates a given quest, i.e. marks it as 'to-delete'.
	/**
	 * @note A Hero can only have one quest, so giving the heroId is as
	 *       good as specifying a particular quest.
	 */
        void deactivateQuest(guint32 heroId);

        //! This method performs cleanup of the marked quests
	/**
	 * Remove the quests marked as deactivated.
	 */
        void cleanup();

	//! Callback when a city is conquered.
	/**
	 * This method calls the other simlarly named methods in the derived
	 * Quest classes.
	 *
	 * @param city   A pointer to the city that was defeated.
	 * @param stack  A pointer to the stack doing the conquering.
	 * @param action What action was taken: pillaging, sacking, razing, or
	 *               occupying.
	 * @param gold   The number of gold pieces achieved in the sacking or
	 *               pillaging.
	 */
	void cityAction(City *c, Stack *s, CityDefeatedAction action, int gold);


        std::vector<Quest*> getActiveQuests ();
        // DATA
        
	//! A hash of all Quests in this QuestsManager.  Lookup by HeroId.
        std::map<guint32,Quest*> d_quests;

        //! A list of quests that have been marked as expiring
        std::list<Quest*> d_inactive_quests;

        //! A list of quests that have been marked as completed
        std::list<Quest*> d_completed_quests;

	//! A vector of isFeasible function pointers.
        /** 
	 * This list of function pointers is used to see if it makes sense to
	 * give out a quest of a particular kind (Quest::Type).
         */
        std::vector<QFeasibilityType> d_questsFeasible;

        //! A static pointer for the singleton instance.
        static QuestsManager * s_instance;
};

#endif
