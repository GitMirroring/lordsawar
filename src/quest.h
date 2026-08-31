// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004 Andrea Paternesi
// Copyright (C) 2007, 2008, 2009, 2014, 2021 Ben Asselstine
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
#ifndef QUEST_H
#define QUEST_H

#include <list>
#include <queue>
#include "callback-enums.h"
#include "OwnerId.h"
#include "vector.h"

class XML_Helper;
class QuestsManager;
class Hero;
class Stack;
class City;
class Army;

//! Base class for Quest objects. Hero units go on quests for Reward objects.
/** 
 * Quest objects are started by Hero objects by visiting a Temple object.  The
 * Quest has a given Quest::Type.  If the Hero can successfully complete the
 * terms of the Quest, a Reward is given.  If the Hero cannot complete the 
 * Quest, because of death or because the Quest is now simply impossible, the 
 * Quest expires.
 * 
 * This class adds some basic functionality for quests, mainly code concerning
 * hero association.
 *
 * This object and the classes that derive from it equate to the 
 * lordsawar.questlist.quest XML entity in the saved-game file.
 *
 */
class Quest: public OwnerId 
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

	//! The various kinds of Quest objects.
        enum Type {
	  //! Find another Player's Hero and kill it.
	  KILLHERO = 1, 
	  //! Seek and destroy a number of another Player's armies.
	  KILLARMIES = 2,
	  //! Conquer and sack another Player's city.
	  CITYSACK = 3, 
	  //! Conquer and raze another Player's city.
	  CITYRAZE = 4, 
	  //! Conquer and occupy another Player's city.
	  CITYOCCUPY = 5,
	  //! Find an army of the given kind and kill it.
	  KILLARMYTYPE = 6, 
	  //! Sack and pillage a number of gold pieces from enemies.
	  PILLAGEGOLD = 7
	};

        //! Standard constructor.
	/**
	 * Make a new Quest object.  This object is not called directly -- it
	 * is only called by the derived classes.
	 *
	 * @param q_mgr The quest manager this quest is being associated with.
	 * @param hero  The Id of the Hero object that owns this quest.
	 * @param type  The type of Quest the Hero is going on.
	 */
        Quest(QuestsManager& q_mgr, guint32 hero, Type type);
        
        //! Copy constructor.
        Quest (const Quest &q);

        //! Loading constructor.
	/**
	 * Make a new quest by loading it in from an opened saved-game file.
	 * @note This only reads the data that is common to all quests.
	 *
	 * @param q_mgr   The quest manager to associate the new Quest with.
	 * @param helper  The opened saved-game file to load the Quest from.
	 */
        Quest(QuestsManager& q_mgr, XML_Helper* helper);

	//! Destructor.
        virtual ~Quest() {};
        
	// Get Methods

	//! Return the description of the Quest.
	/** 
         *  This is the 'static' part of the quest description,
         *  set once the quest has been initialized.  Another,
         *  dynamic part consists of the quest's status info,
         *  which is obtained by the Quest::getProgress method.
         */
        Glib::ustring getDescription() const { return d_description; }

	//! Returns if the Quest will be deleted at the end of the round.
        bool isPendingDeletion() const {return d_pending;}

	//! Return the Id of the Hero object responsible for this Quest object.
        guint32 getHeroId() const { return d_hero; }

        //! Returns the name of the Hero responsible for this Quest.
        Glib::ustring getHeroName() const {return d_hero_name;}

        //! Return the type of the quest (one of values listed in Quest::Type).
        guint32 getType() const { return d_type; }

	//! Return the targets for this Quest.
	/** 
	 * This method provides a list of positions that the hero is seeking.
	 * This method is called by the questmap object to assist in showing
	 * the quest on a map.
	 * Quest::PILLAGEGOLD does not have any targets.
	 *
	 * @return A list of positions on the map that the Hero is seeking.
	 */
	std::list< Vector<int> > getTargets() const {return d_targets;}


	// Set Methods

	//! Set the Quest as not mattering anymore.
        void deactivate() {d_pending = true;}

        //! Set quests manager
        void setQuestsManager (QuestsManager &q);
	
	// Methods that operate on the class data but do not modify the class.

        //! Return a pointer to the Hero object responsible for the Quest.
        Hero* getHero() const { return getHeroById(d_hero); }

	//! Determine the name of the hero, even if it's dead.
	Glib::ustring getHeroNameForDeadHero() const;

	//! Save the Quest to an opened saved-game file.
        /** 
          * @note This function is called by the actual quests and only saves
          * the common data. It does NOT open/close tags etc. This has to be
          * done by the derived classes.
	  *
	  * @param helper  The opened saved-game file to save the common Quest
	  *                data to.
          */
        virtual bool save(XML_Helper* helper) const;


	// Methods that need to be implemented by derived classes.

	//! Return the description of the progress the Hero has made.
        virtual Glib::ustring getProgress() const = 0;

	//! Return the completion text that is associated with this Quest.
        /**
	 * @param msgs  A queue of strings that represents the completion 
	 *              text to show.
         */
        virtual void getSuccessMsg(std::queue<Glib::ustring>& msgs) const = 0;

        /**
         * \brief Provide the lines of the message describing
                  the quest completion.
         */
	//! Return the text that is shown when the Quest has expired.
	/**
	 * @param msgs  A queue of strings that represents the text to show
	 *              when the Quest has expired.
	 */
        virtual void getExpiredMsg(std::queue<Glib::ustring>& msgs) const = 0;

	//! Callback whenever an Army dies.
	/**
	 * This method notifies the Quest that an army has died, and if the u
	 * hero responsible for this quest killed it or not.
	 *
	 * @param army           An Army object that has just died in the game.
	 * @param heroIsCulprit  Whether or not the Hero object associated with
	 *                       this Quest object is responsible for killing
	 *                       the given Army object.
	 */
	virtual void armyDied(Army *army, bool heroIsCulprit)=0;

	//! Callback whenever a city has been conquered.
	/**
	 * This method notifies the Quest that a City has fallen, and what the 
	 * conquering action (pillage/sack/raze/occupy) was.  It also notifies
	 * whether or not the hero responsible for this quest was involved in 
	 * the conquering, and how much gold was taken as a result.
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
	virtual void cityAction(City *city, CityDefeatedAction action, 
				bool heroIsCulprit, int gold)=0;


	// Static Methods

	//! Determine the name of a hero, given the id.
	static Glib::ustring getHeroNameForDeadHero(guint32 id);

	//! Convert a Quest::Type string to an enumerated value.
	static Quest::Type questTypeFromString(Glib::ustring str);

	//! Convert a Quest::Type enumerated value to a string.
	static Glib::ustring questTypeToString(const Quest::Type type);

	//! deep copy a quest into another one
	static Quest* copy(const Quest* q);

	//! Return the Stack and Hero of a Quest.
        /**
         * @param hero      The id of the Hero on this quest.
         * @param stack     This pointer is filled with a pointer to the stack 
	 *                  that the Hero is in.  If passed as NULL, it is not
	 *                  calculated at all.
	 *
         * @return A pointer to the Hero object or NULL if the Hero is dead.
         */
        static Hero* getHeroById(guint32 hero, Stack** stack = NULL);

    protected:
	// DATA

	//! The QuestsManager object that this Quest object is associated with.
        QuestsManager& d_q_mgr;

	//! A description of the Quest (this text does not change).
        /** 
         * This value is to be filled by the derived quest objects.
         */
        Glib::ustring d_description;

	//! The Id of the Hero object responsible for this Quest.
        guint32 d_hero;

        //! The type of the Quest (one of Quest::Type).
        guint32 d_type;

        //! If set to false, this quest is deactivated and not to be processed.
        bool d_pending;

	//! The name of the hero who is on the Quest.
	/**
	 * The name of the Hero must be saved so that after the Hero dies, we 
	 * can submit a history item that references the Hero's name.
	 */
        Glib::ustring d_hero_name;

	//! A list of targets to display on a questmap.
	/**
	 * The derived Quest classes fill in this value.
	 */
	std::list< Vector<int> > d_targets;
};

#endif
