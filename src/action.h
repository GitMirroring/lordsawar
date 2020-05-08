// Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2007, 2008, 2010, 2011, 2014, 2015, 2017, 2020 Ben Asselstine
// Copyright (C) 2008 Ole Laursen
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
#ifndef ACTION_H
#define ACTION_H

#include <gtkmm.h>
#include "vector.h"
#include <sigc++/trackable.h>

#include "fight.h"
#include "army.h"
#include "player.h"
#include "ucompose.hpp"
#include "armyprodbase.h"
#include "heroproto.h"

class Quest;
class Stack;
class City;
class Signpost;
class Ruin;
class Temple;
class XML_Helper;

//! A temporary record of an event during gameplay.
/** 
 * The purpose of the action classes is to keep track of what a player has
 * done. This information can be sent over the network, so that a networked 
 * player then just has to decode and repeat the remote player's actions so
 * that the game state is synchronised.
 * 
 * Each Player has an Actionlist to which these actions belong.
 */

class Action
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

	//! An Action can be one of the following kinds.
        enum Type {
	        /** A stack has moved. */
                STACK_MOVE = 1,
		/** A stack has separated into two parts. */
                STACK_SPLIT = 2,
		/** A stack is fighting a city or another stack. */
                STACK_FIGHT = 3,
		/** A stack has merged with another stack. */
                STACK_JOIN = 4,
		/** A stack containing a hero has examined a ruin. */
                RUIN_SEARCH = 5,
		/** A stack has examined a temple. */
                TEMPLE_SEARCH = 6,
		/** A stack has defeated a city and occupied it. */
                CITY_OCCUPY = 7,
		/** A stack has defeated a city and pillaged it. */
                CITY_PILLAGE = 8,
		/** A stack has defeated a city and razed it. */
                CITY_RAZE = 9,

                /* = 10 is not used */

		/** A player has purchased a new Army unit to be produced 
		 * in a city. */
                CITY_BUY = 11,
		/** A player has changed production in a city to another 
		 * Army unit.*/
                CITY_PROD = 12,
		/** A stack has received a reward. */
                REWARD = 13,
		/** A hero has received a new quest. */
                QUEST = 14,
		/** A hero has picked up or dropped an item. */
                HERO_EQUIP = 15,
		/** An Army unit has advanced to a new level (Only used for 
		 * heroes). */
                UNIT_ADVANCE = 16,
		/** A stack has defeated a city and sacked it. */
                CITY_SACK = 17,
		/** A player has removed a stack. */
                STACK_DISBAND = 18,
		/** A player has changed what a signpost says. */
                MODIFY_SIGNPOST = 19,
		/** A player has changed the name of a city. */
                CITY_RENAME = 20,
		/** A player has vectored Army units from one city to
		 * another. */
                CITY_VECTOR = 21,
		/** A player has changed the order in which Army units do
		 * battle. */
                FIGHT_ORDER = 22,
		/** A player has surrendered. */
		RESIGN = 23,
		/** A hero has planted an item in the ground. */
		ITEM_PLANT = 24,
		/** A newly produced Army unit arrives on the map. */
		PRODUCE_UNIT = 25,
		/** A new vectored Army unit has shown up at a location. */
		PRODUCE_VECTORED_UNIT = 26,
		/** The player's diplomatic relations with respect to another
		 * player has changed. */
		DIPLOMATIC_STATE = 27,
		/** The player's diplomatic proposal with respect to another
		 * player has changed. */
		DIPLOMATIC_PROPOSAL = 28,
		/** The player's diplomatic score with respect to another
		 * player has changed. */
		DIPLOMATIC_SCORE = 29,
                /** The player has ended their turn.
                 */
                END_TURN = 30,
                /** A player has taken over a city.
                 */
                CITY_CONQUER = 31,
                /** A player has agreed to take on a hero who has offered
                 * their service.
                 */
                RECRUIT_HERO = 32,

                /* = 33 is not used */

                /** A city's production has been turned off because the player
                 * lacks gold.
                 */
		CITY_DESTITUTE = 34,
                /** A player has started their turn.
                 */
		INIT_TURN = 35,
                /** A player has looted a city.
                 */
		CITY_LOOT = 36,
                /** A hero uses one of their items.
                 */
                USE_ITEM = 37,
                /** The armies in a stack have been reordered.
                 */
                STACK_ORDER = 38,
                /** A player's stacks have their movement points and hit points
                 * refreshed.
                 */
                STACKS_RESET = 39,
                /** Keepers in ruins have their hit points refreshed.
                 */
                RUINS_RESET = 40,
                /** A player gets their income from cities and pays their army
                 * units.
                 */
                COLLECT_TAXES_AND_PAY_UPKEEP = 41,
                /** A player has been defeated.
                 */
                KILL_PLAYER = 42,
                /** A stack goes into defend mode.
                 */
                STACK_DEFEND = 43,
                /** A stack goes out of defend mode.
                 */
                STACK_UNDEFEND = 44,
                /** A stack goes into parked mode.
                 */
                STACK_PARK = 45,
                /** A stack goes out of parked mode.
                 */
                STACK_UNPARK = 46,
                /** A player selected a stack.
                 */
                STACK_SELECT = 47,
                /** A player unselected/deselected a stack.
                 */
                STACK_DESELECT = 48
        };
	static Glib::ustring actionTypeToString(Action::Type type);
	static Action::Type actionTypeFromString(Glib::ustring str);

	//! Default constructor.
        Action(Type type);

	//! Copy constructor (shallow).
	Action(const Action &action);

	//! Loading constructor.
        Action(XML_Helper *helper);

	//! Destructor.
        virtual ~Action() {};

        //! Returns debug information. Needs to be overwritten by derivatives.
        virtual Glib::ustring dump() const = 0;

        //! Save function. See XML_Helper for information about saving.
        bool save(XML_Helper* helper) const;
	bool saveContents(XML_Helper* helper) const;
        
        /** 
	 * static load function (see XML_Helper)
         * 
         * Whenever an action item is loaded, this function is called. It
         * examines the stored id and calls the constructor of the appropriate
         * action class.
         *
         * @param helper       the XML_Helper instance for the savegame
         */
	//! Load the action from an opened saved-game file.
        static Action* handle_load(XML_Helper* helper);

        //! Make a new action from an existing one.
        static Action* copy(const Action* a);

        //! Returns the Action::Type for this action.
        Type getType() const {return d_type;}

    protected:
        virtual bool doSave(XML_Helper* helper) const = 0;
        
        Type d_type;
};

//-----------------------------------------------------------------------------

//! A temporary record of a Stack moving.
/**
 * The purpose of the Action_Move class is to record when a stack has
 * moved to a new position on the map.
 */
class Action_Move : public Action
{
    public:
	//! Make a new move action.
	/**
         * Populate the move action with the stack and it's new position.
         */
        Action_Move(Stack* s, Vector<int> dest);
	//! Copy constructor
        Action_Move(const Action_Move &action);
	//! Load a new move action from an opened saved-game file.
        Action_Move(XML_Helper* helper);
	//! Destroy a move action.
        ~Action_Move() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose
              ("Stack %1 moved to %2,%3 w/ %4 mp left, ship=%5 (was %6)\n",
               d_stack, d_dest.x, d_dest.y, d_moves_left, d_has_ship,
               d_had_ship);
          }

	//! Save this move action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	guint32 getStackId() const {return d_stack;};
	Vector<int> getEndingPosition() const {return d_dest;};
	Vector<int> getPositionDelta() const {return d_delta;};
        void setMovesLeft(guint32 mp) {d_moves_left = mp;}
        guint32 getMovesLeft() const {return d_moves_left;};
        void setHasShip(bool s) {d_has_ship = s;}
        bool getHasShip() const {return d_has_ship;};
        void setHadShip(bool s) {d_had_ship = s;}
        bool getHadShip() const {return d_had_ship;};

        private:
        guint32 d_stack;
        Vector<int> d_dest;
	Vector<int> d_delta;
        guint32 d_moves_left;
        bool d_has_ship;
        bool d_had_ship;
};

//-----------------------------------------------------------------------------

//! A temporary record of a Stack being split into two.
/**
 * The purpose of the Action_Split class is to record when a Stack has been
 * separated into two parts.  This happens when the Player groups only some
 * of the Army units in the stack (not all), and then moves them to a new 
 * position on the map.  When a split is completed there is the original
 * stack (the remaining Army units in the stack that didn't change position), 
 * and a new stack added to the game (the stack containing the grouped army 
 * units).
 */
class Action_Split : public Action
{
    public:
	//! Make a new stack split action.
        /** 
	 * Populate the Action_Split class with the original Stack, and the 
	 * new stack that has been added to the game.  Please note that the 
	 * stacks have to be already split before this call.
	 */
        Action_Split(Stack* orig, Stack* added);
	//! Copy constructor
	Action_Split(const Action_Split &action);
	//! Load a new stack split action from an opened saved-game file.
        Action_Split(XML_Helper* helper);
	//! Destroy a stack split action.
        ~Action_Split() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            Glib::ustring s =
              String::ucompose("Stack %1 split with new stack %2.\n", d_orig,
                               d_added);
            s += "moved these armies:";

            for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
              s += String::ucompose("%1 ", d_armies_moved[i]);
            s += "\n";

            return s;
          }

	//! Save this split action to an opened saved-game file.
        bool doSave (XML_Helper* helper) const;

	guint32 getStackId() const {return d_orig;};
	guint32 getNewStackId() const {return d_added;};
	guint32 getGroupedArmyId(int idx) const {return d_armies_moved[idx];};
        private:
        guint32 d_orig, d_added;
        guint32 d_armies_moved[MAX_STACK_SIZE];
};

//! A temporary record of a Stack being disbanded.
/**
 * The purpose of the Action_Disband class is to record when a stack has
 * removed from the game.  Disbanding is done to primarily to save the 
 * gold pieces paid out every turn as upkeep for the Army units in the Stack.
 */
class Action_Disband: public Action
{
    public:
	//! Make a new disband action.
	/**
         * Populate the action with the Stack being removed.
         */
        Action_Disband(Stack *s);
	//! Copy constructor
	Action_Disband(const Action_Disband &action);
	//! Load a new disband action from an opened saved-game file.
        Action_Disband(XML_Helper* helper);
	//! Destroy a disband action.
        ~Action_Disband() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose("Stack %1 disbanded.\n", d_stack);
          }

	//! Save this disband action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	guint32 getStackId() const {return d_stack;};

        private:
        guint32 d_stack;
};


//-----------------------------------------------------------------------------

//! A temporary record of a fight between opposing Stack objects.
/**
 * The purpose of the Action_Fight class is to record the results of a
 * fight between two Players.
 */
class Action_Fight : public Action
{
    public:
	//! Make a new fight action.
	/**
	 * Populate the action with the Fight.  Please note that the
	 * Fight must have already been faught.
	 */
        Action_Fight(const Fight* f);
	//! Copy constructor
	Action_Fight(const Action_Fight &action);
	//! Load a new fight action from an opened saved-game file.
        Action_Fight(XML_Helper* helper);
	//! Destroy a fight action.
        ~Action_Fight() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const;

	//! Save this fight action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	std::list<FightItem> getBattleHistory() const {return d_history;};
	std::list<guint32> getAttackerStackIds() const {return d_attackers;};
	std::list<guint32> getDefenderStackIds() const {return d_defenders;};
	std::list<guint32> getAttackerArmyIds() const {return d_attacker_army_ids;};
	std::list<guint32> getDefenderArmyIds() const {return d_defender_army_ids;};

        bool is_army_id_in_stacks(guint32 id, const std::list<guint32> &stack_ids) const;
        private:
        
        std::list<FightItem> d_history;
        std::list<guint32> d_attackers;
        std::list<guint32> d_defenders;
        std::list<guint32> d_attacker_army_ids;
        std::list<guint32> d_defender_army_ids;

        bool stack_ids_to_stacks(std::list<guint32> stack_ids, std::list<Stack*> &stacks, guint32 &stack_id) const;

        bool loadItem(XML_Helper* helper);
};

//-----------------------------------------------------------------------------

//! A temporary record of two Stack objects merging into one.
/**
 * The purpose of the Action_Join class is to record a Stack has had 
 * another Stack merged into it.
 */
class Action_Join : public Action
{
    public:
	//! Make a new stack join action.
        /** 
	 * Populate the Action_Join class with the original Stack, and the 
	 * stack that is merging into the original one.  Please note that
	 * this method must be called before the merge takes place.
	 */
        Action_Join(Stack* orig, Stack* joining);
	//! Copy constructor
	Action_Join(const Action_Join &action);
	//! Load a new stack join action from an opened saved-game file.
        Action_Join(XML_Helper* helper);
	//! Destroy a stack join action.
        ~Action_Join() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose("Stack %1 joined stack %2\n",
                                    d_joining_id, d_orig_id);
          }

	//! Save this stack join action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	guint32 getReceivingStackId() const {return d_orig_id;};
	guint32 getJoiningStackId() const {return d_joining_id;};
        private:
        guint32 d_orig_id, d_joining_id;
};

//-----------------------------------------------------------------------------

//! A temporary record of what happened when a Stack searched a Ruin.
/**
 * The purpose of the Action_Ruin class is to record what happens when a
 * Stack containing a Hero attempts to search a Ruin.
 */
class Action_Ruin : public Action
{
    public:
	//! Make a new ruin search attempted action.
	/**
	 * Populate the Action_Ruin class with the Stack containing the
	 * Hero Army unit, and the Ruin being searched.
	 */
        Action_Ruin(Ruin* r, Stack* explorers);
	//! Copy constructor
	Action_Ruin(const Action_Ruin&action);
	//! Load a new ruin search attempted action from a saved-game file.
        Action_Ruin(XML_Helper* helper);
	//! Destroy a ruin search attempted action.
        ~Action_Ruin() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            Glib::ustring s = String::ucompose("Ruin %1 searched by stack %2.",
                                               d_ruin, d_stack);
            s + "  ";
            if (d_searched)
              s += "Ruin has been searched.\n";
            else
              s += "Ruin has not been searched.\n";

            return s;
          }

	//! Save this ruin search attempted action to a saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	guint32 getRuinId() const {return d_ruin;};
	guint32 getStackId() const {return d_stack;};
	bool getSearchSuccessful() const {return d_searched;};

        private:
        guint32 d_ruin;
        guint32 d_stack;
        bool d_searched;
};

//-----------------------------------------------------------------------------

//! A temporary record of what happened when a Stack visited a Temple.
/**
 * The purpose of the Action_Temple class is to record what happens when
 * a Stack visits a Temple.  The Stack may be getting blessed, or instead it
 * might be a Stack containing a Hero who is obtaining a new Quest.
 */
class Action_Temple : public Action
{
    public:
	//! Make a new temple search action.
	/**
	 * Populate the Action_Temple class with the Stack and the Temple
	 * being searched.
	 */
        Action_Temple(Temple* t, Stack* s);
	//! Copy constructor
	Action_Temple(const Action_Temple &action);
	//! Load a new temple search action from a saved-game file.
        Action_Temple(XML_Helper* helper);
	//! Destroy a temple search action.
        ~Action_Temple() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose("Stack %1 visited temple %2.\n", d_stack,
                                    d_temple);
          }

	//! Save this temple search attempted action to a saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	guint32 getTempleId() const {return d_temple;};
	guint32 getStackId() const {return d_stack;};

        private:
        guint32 d_temple;
        guint32 d_stack;
};


//-----------------------------------------------------------------------------

//! A temporary record of what happened when a Player occupied a City.
/**
 * The purpose of the Action_Occupy class is to record when a Player has
 * defeated a City and has occupied it.  Ocuppying differs from sacking
 * and pillaging in that none of the existing Army units in the City are
 * exchanged for gold pieces.
 */
class Action_Occupy : public Action
{
    public:
	//! Make a new city occupy action.
        Action_Occupy(City *c);
	//! Copy constructor.
	Action_Occupy(const Action_Occupy &action);
	//! Load a new city occupied action from an opened saved-game file.
        Action_Occupy(XML_Helper* helper);
	//! Destroy a city occupy action.
        ~Action_Occupy() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose("City %1 occupied.\n", d_city);
          }

	//! Save this city occupied action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	guint32 getCityId() const {return d_city;};

        private:
        guint32 d_city;
};

//-----------------------------------------------------------------------------

//! A temporary record of what happened when a Player pillaged a City.
/**
 * The purpose of the Action_Pillage class is to record when a Player has
 * defeated a City and has pillaged it.  Pillaging a city results in the
 * strongest Army unit being produced in that city being exchanged for an 
 * amount of gold pieces.
 */
class Action_Pillage : public Action
{
    public:
	//! Make a new city pillaged action.
        Action_Pillage(City *c);
	//! Copy constructor
	Action_Pillage(const Action_Pillage &action);
	//! Load a new city pillaged action from an opened saved-game file.
        Action_Pillage(XML_Helper* helper);
	//! Destroy a city pillaged action.
        ~Action_Pillage() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose("City %1 pillaged.\n", d_city);
          }

	//! Save this city pillaged action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	guint32 getCityId() const {return d_city;};

        private:
        guint32 d_city;
};

//-----------------------------------------------------------------------------

//! A temporary record of what happened when a Player sacked a City.
/**
 * The purpose of the Action_Sack class is to record when a Player has
 * defeated a City and has sacked it.  Sacking a city results in all 
 * Army units, except the weakest being exchanged for an amount of gold 
 * pieces.
 */
class Action_Sack : public Action
{
    public:
	//! Make a new city sacked action.
        Action_Sack(City *c);
	//! Copy constructor
	Action_Sack(const Action_Sack &action);
	//! Load a new city sacked action from an opened saved-game file.
        Action_Sack(XML_Helper* helper);
	//! Destroy a city sacked action.
        ~Action_Sack() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose("City %1 sacked.\n", d_city);
          }

	//! Save this city sacked action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	guint32 getCityId() const {return d_city;};

        private:
        guint32 d_city;
};

//-----------------------------------------------------------------------------

//! A temporary record of what happened when a Player razed a City.
/**
 * The purpose of the Action_Raze class is to record when a Player has
 * defeated a City and has razed it.  Razing a city results in that
 * city becoming uninhabitable, and it ceases to produce new Army units.
 */
class Action_Raze : public Action
{
    public:
	//! Make a new city razed action.
        Action_Raze(City *c);
	//! Copy constructor
	Action_Raze(const Action_Raze &action);
	//! Load a new city razed action from an opened saved-game file.
        Action_Raze(XML_Helper* helper);
	//! Destroy a city razed action.
        ~Action_Raze() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose("City %1 razed.\n", d_city);
          }

	//! Save this city razed action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	guint32 getCityId() const {return d_city;};

        private:
        guint32 d_city;
};

//-----------------------------------------------------------------------------

//! A temporary record of more production being added to a City.
/**
 * The purpose of the Action_Buy class is to record when a Player purchases
 * a new Army unit for production in a City.  When this happens the player
 * specifies a production slot to hold the new Army unit type.  Any existing
 * Army unit type in that slot before the buy is removed.
 * The idea here is that the city may produce one Army unit type from a set 
 * of available Army units, and this action records what happens when we make 
 * a new kind of Army unit available for production in the City.
 * The army unit is taken from the Player's Armyset.
 */
class Action_Buy : public Action
{
    public:
	//! Make a new city buy production action.
	/**
	 * Populate the Action_Buy with City where the buy has happened.
	 * Also populate it with the City's production slot that is now
	 * producing the new Army unit type.  Lastly, populate the Action_Buy 
	 * with the new Army unit type being produced.
	 */
        Action_Buy(City* c, int slot, const ArmyProto *prod);
	//! Copy constructor
	Action_Buy(const Action_Buy &action);
	//! Load a new city buy production action from a saved-game file.
        Action_Buy(XML_Helper* helper);
	//! Destroy a city buy production action.
        ~Action_Buy() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return
              String::ucompose ("Production %1 bought in city %2 slot: %3.\n",
                                d_prod, d_city, d_slot);
          }

	//! Save this city buy production action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	guint32 getCityId() const {return d_city;};
	int getProductionSlot() const {return d_slot;};
	int getBoughtArmyTypeId() const {return d_prod;};
        private:
        guint32 d_city;
        int d_slot, d_prod;
};

//-----------------------------------------------------------------------------

//! A temporary record of a change in production strategy in a City.
/**
 * The purpose of the Action_Production class is to record when the Player
 * changes the production of new Army units within a City.  The idea here
 * is that the City has a set of available Army units that it may produce,
 * and the Player has selected a different Army unit to produce, or has
 * stopped production of new Army units altogether.
 */
class Action_Production : public Action
{
    public:
	//! Make a new city change production action.
	/**
	 * Populate the Action_Production with City where the change
	 * in production has taken place.  Also populate it with the newly
	 * active production slot (-1 means stopped).
	 */
        Action_Production(City* c, int slot);
	//! Copy constructor
	Action_Production (const Action_Production &action);
	//! Load a new city change production action from a saved-game file.
        Action_Production(XML_Helper* helper);
	//! Destroy a city change production action.
        ~Action_Production() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose ("Production in city %1 changed to %2.\n",
                                     d_city, d_prod);
          }

	//! Save this city change production action to a saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	guint32 getCityId() const {return d_city;};
	int getSlot() const {return d_prod;};

        private:
        guint32 d_city;
        int d_prod;
};

//-----------------------------------------------------------------------------

//! A temporary record of a Player or Stack getting a Reward.
/**
 * The purpose of the Action_Reward class is to record when the Player
 * has been given a Reward.
 * It could be that the player has been given: Some gold pieces, a map that 
 * makes more of the map visible or information about the location of a new
 * ruin.  It could also be that the player's active stack has been given
 * a number of allies.  It could also be that the Player's active Stack
 * contains a Hero, and the Reward is an Item for the Hero to carry.
 */
class Action_Reward : public Action
{
    public:
	//! Make a new player rewarded action.
        Action_Reward(Stack *stack, Reward *r);
	//! Copy constructor
	Action_Reward (const Action_Reward &action);
	//! Load a new player rewarded action from a saved-game file.
        Action_Reward(XML_Helper* helper);
	//! Destroy a player rewarded action.
        ~Action_Reward();
        
	//! Return some debug information about this action.
        Glib::ustring dump() const;

	//! Save this player rewarded action to a saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	Reward *getReward() const {return d_reward;};
	guint32 getStackId() const {return d_stack;};

        private:
	Reward *d_reward;
        guint32 d_stack;

        bool load(Glib::ustring tag, XML_Helper *helper);
};

//-----------------------------------------------------------------------------

//! A temporary record of a Hero initiating a new Quest.
/**
 * The purpose of the Action_Quest class is to record when a Player's
 * Hero has gone to a Temple and initiated a new Quest.
 */
class Action_Quest : public Action
{
    public:
	//! Make a new hero quest assigned action.
        Action_Quest(Quest* q);
	//! Copy constructor
	Action_Quest (const Action_Quest &action);
	//! Load a new hero quest assigned action from a saved-game file.
        Action_Quest(XML_Helper* helper);
	//! Destroy a hero quest assigned action.
        ~Action_Quest() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose
              ("Hero %1 has got quest of type %2 with data %3 to fulfill\n",
               d_hero, d_questtype, d_data);
          }

	//! Save this hero quest assigned action to a saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	guint32 getHeroId() const {return d_hero;};
	guint32 getQuestType() const {return d_questtype;};
	guint32 getData() const {return d_data;};
	guint32 getVictimPlayerId() const {return d_victim_player;};

        private:
        guint32 d_hero;
        guint32 d_questtype;
        guint32 d_data;
	guint32 d_victim_player; //victim player, only KILLARMIES uses this
};

//-----------------------------------------------------------------------------

//! A temporary record of a Hero picking up or dropping an Item.
/**
 * The purpose of the Action_Equip class is to record when a Player's Hero
 * has picked up an Item or dropped it onto the ground.  Heroes pick up
 * and drop Items one at a time.
 */
class Action_Equip : public Action
{
    public:
        enum Slot {
	    // The Item is going neither to the ground or the backpack.
            NONE = 0,
	    //! The Item has gone into the Hero's Backpack.
            BACKPACK = 1,
	    //! The Item has been dropped onto the ground.
            GROUND = 2};
        
	//! Make a new item equipped action.
	/**
	 * Populate the Action_Equip class with the Id of the Hero,
	 * the Id of the Item and the destination of the Item in terms of
	 * it's Action_Equip::Slot.
	 */
        Action_Equip(Hero *hero, Item *item, Slot slot, Vector<int> pos);
	//! Copy constructor
	Action_Equip (const Action_Equip &action);
	//! Load a new item equipped action from an opened saved-game file.
        Action_Equip(XML_Helper* helper);
	//! Destroy an item equipped action.
        ~Action_Equip() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose
              ("Hero %1 moved item %2 to slot %3 at tile %4,%5.\n",
               d_hero, d_item, d_slot, d_pos.x, d_pos.y);
          }

	//! Save this item equipped action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	guint32 getHeroId() const {return d_hero;};
	guint32 getItemId() const {return d_item;};
	guint32 getToBackpackOrToGround() const {return d_slot;};
	Vector<int> getItemPos() const {return d_pos;};

        private:
        guint32 d_hero;
        guint32 d_item;
        guint32 d_slot;
	Vector<int> d_pos;
};

//-----------------------------------------------------------------------------

//! A temporary record of a Hero gaining a new level.
/**
 * The purpose of the Action_Level class is to record when a Player's Hero
 * advances a level and subsequently gains a stat.
 * Stats that may get increased are: Strength, Moves, and Sight (for use on 
 * a hidden map).
 */
class Action_Level : public Action
{
    public:
	//! Make a new level advancement action.
	/**
	 * Populate the Action_Level class with the the Id of the Hero
	 * Army unit, and also the Hero's stat that has been raised as a
	 * result of the level advancement.
	 */
        Action_Level(Army *unit, Army::Stat raised);
	//! Copy constructor
	Action_Level (const Action_Level &action);
	//! Load a new level advancement action from an opened saved-game file.
        Action_Level(XML_Helper* helper);
	//! Destroy a level advancement action.
        ~Action_Level() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose
              ("Army unit %1 advanced level and increased stat type %2",
               d_army, d_stat);
          }

	//! Save this level advancement action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	guint32 getArmyId() const {return d_army;};
	guint32 getStatToIncrease() const {return d_stat;};

        private:
        guint32 d_army;
        guint32 d_stat;
};

//-----------------------------------------------------------------------------

//! A temporary record of a Player changing the contents of a Signpost.
/**
 * The purpose of the Action_ModifySignpost is to record when a Signpost
 * has been altered by a player to have a different message on it.  The
 * idea here is that we're playing on a hidden map and a Player wants to
 * thwart an opponent by changing what signs say before he can read them.
 */
class Action_ModifySignpost: public Action
{
    public:
	//! Make a new change signpost action.
	/**
         * Populate the action with the signpost and the new message.
         */
        Action_ModifySignpost(Signpost * s, Glib::ustring message);
	//! Copy constructor
	Action_ModifySignpost(const Action_ModifySignpost &action);
	//! Load a new change signpost action from an opened saved-game file.
        Action_ModifySignpost(XML_Helper* helper);
	//! Destroy a change signpost action.
        ~Action_ModifySignpost() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose("Signpost %1 modified to read %2.\n",
                                    d_signpost, d_message);
          }

	//! Save this change signpost action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	guint32 getSignpostId() const {return d_signpost;};
	Glib::ustring getSignContents() const {return d_message;};

        private:
        guint32 d_signpost;
	Glib::ustring d_message;
};

//-----------------------------------------------------------------------------

//! A temporary record of a Player changing the name of a City.
/**
 * The purpose of the Action_RenameCity class is to record when a Player has
 * changed the name of a City.  The idea here is that a Player wants
 * to gloat by renaming a newly conquered City.
 */
class Action_RenameCity: public Action
{
    public:
	//! Make a new city rename action.
	/**
         * Populate the action with the city being renamed and the new name.
         */
        Action_RenameCity(City* c, Glib::ustring name);
	//! Copy constructor
	Action_RenameCity(const Action_RenameCity &action);
	//! Load a new city rename action from an opened saved-game file.
        Action_RenameCity(XML_Helper* helper);
	//! Destroy a city rename action.
        ~Action_RenameCity() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose("City %1 renamed to %2.\n", d_city, d_name);
          }

	//! Save this city rename action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	guint32 getCityId() const {return d_city;};
	Glib::ustring getNewCityName() const {return d_name;};
        private:
        guint32 d_city;
	Glib::ustring d_name;
};

//-----------------------------------------------------------------------------

//! A temporary record of a Player changing vectoring strategies for a City.
/**
 * The purpose of the Action_Vector class is to record when a Player has
 * changed the vectoring policy of a City.  The City's Army units can
 * be vectored to another City or to a Hero's planted standard (Item).
 * When units are vectored they take 2 turns to appear at their destination.
 * While the vectored units are en route, they are invisible.
 */
class Action_Vector: public Action
{
    public:
	//! Make a new city vector action.
	/**
	 * Populate the Action_Vector class with the City being vectored
	 * from, and the destination position for the vectored units.
	 */
        Action_Vector(City* src, Vector<int> dest);
	//! Copy constructor
	Action_Vector(const Action_Vector &action);
	//! Load a new city vector action from an opened saved-game file.
        Action_Vector(XML_Helper* helper);
	//! Destroy a city vector action.
        ~Action_Vector() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose
              ("Vectoring new army units from city %1 to %2,%3.\n",
               d_city, d_dest.x, d_dest.y);
          }

	//! Save this city vector action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	guint32 getCityId() const {return d_city;};
	Vector<int> getVectoringDestination() const {return d_dest;};

        private:
        guint32 d_city;
        Vector<int> d_dest;
};

//-----------------------------------------------------------------------------

//! A temporary record of a Player changing the fight order of an Armyset.
/**
 * The purpose of the Action_FightOrder action is to record when a Player
 * changes the order in which Army units fight in battle.
 */
class Action_FightOrder: public Action
{
    public:
	//! Make a new fight order action.
	/**
         * Populate the action with a list of ranks, one per Army unit type.
         */
        Action_FightOrder(const std::list<guint32> &order);
	//! Copy constructor
	Action_FightOrder(const Action_FightOrder &action);
	//! Load a new fight order action from an opened saved-game file.
        Action_FightOrder(XML_Helper* helper);
	//! Destroy a fight order action.
        ~Action_FightOrder() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            Glib::ustring s = "Changed fight order to: ";
            for (std::list<guint32>::const_iterator it = d_order.begin(); 
                 it != d_order.end(); ++it)
              s += String::ucompose("%1 ", (*it));
            s += "\n";
            return s;
          }

	//! Save this fight order action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	std::list<guint32> getFightOrder() const {return d_order;};

        private:
	std::list<guint32> d_order;
};

//-----------------------------------------------------------------------------

//! A temporary record of a Player surrendering.
/**
 * The purpose of the Action_Resign class is to record when a Player has
 * resigned from the game.  Because these actions are held in a Player's
 * Actionlist, we do not have to store the player's Id.
 */
class Action_Resign: public Action
{
    public:
	//! Make a new player resignation action.
        Action_Resign() : Action(Action::RESIGN) {}
	//! Copy constructor
	Action_Resign(const Action_Resign &action) : Action (action) {}
	//! Load a new player resignation action from an opened saved-game file.
        Action_Resign(XML_Helper* helper) : Action(helper) {}
	//! Destroy a player resignation action.
        ~Action_Resign() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const { return "This player resigns.\n"; }

	//! Save this player resignation action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const
          {
            if (helper)
              return true;
            return false;
          }
};

//-----------------------------------------------------------------------------

//! A temporary record of a Hero planting a standard into the ground.
/**
 * The purpose of the Action_Plant class is to record when a Hero has 
 * planted a standard (e.g. a flag Item) into the ground, so that Army units
 * can be vectored there.
 */
class Action_Plant: public Action
{
    public:
	//! Make a new item planted action.
	/**
         * Populate the action with the Id of the Hero and the Id of the Item.
         */
        Action_Plant(Hero *hero, Item *item);
	//! Copy constructor
	Action_Plant(const Action_Plant &action);
	//! Load a new item planted action from an opened saved-game file.
        Action_Plant(XML_Helper* helper);
	//! Destroy a item planted action.
        ~Action_Plant() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose ("Hero %1 plants item %2.\n",
                                     d_hero, d_item);
          }

	//! Save this item planted action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	guint32 getHeroId() const {return d_hero;};
	guint32 getItemId() const {return d_item;};

        private:
        guint32 d_hero;
        guint32 d_item;
};

//-----------------------------------------------------------------------------

//! A temporary record of a new Army unit showing up at a city.
/**
 * The purpose of the Action_Produce class is to record when a new Army unit
 * is created.  The idea here is that a City has produced a new Army unit.
 * The City might be vectoring elsewhere, so the unit doesn't show up in the
 * City that produced it.  If vectoring is not enabled the Army unit shows
 * up right away in the host City.
 * This action is used primarily for reporting purposes.  The player's 
 * production report shows which army units were created in what cities.
 * The army unit is taken from the Player's Armyset.
 */
class Action_Produce: public Action
{
    public:
	//! Make a new unit produced action.
	/**
	 * Populate the Action_Produce action with the army type being
	 * produced, the City in which it has arrived, and also whether
	 * or not this unit was prevented from showing up here because
	 * it is being vectored elsewhere.
	 *
	 * pos is the vectored location if we're vectoring
	 * or it's the position on the map where the newly produced army ended 
	 * up.
	 */
        Action_Produce(const ArmyProdBase *army, City *city, bool vectored, 
                       Vector<int> pos, guint32 army_id, guint32 stack_id);
	//! Copy constructor
	Action_Produce(const Action_Produce &action);
	//! Load a new unit produced action from an opened saved-game file.
        Action_Produce(XML_Helper* helper);
	//! Destroy a unit produced action.
        ~Action_Produce();

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            Glib::ustring s =
              String::ucompose ("Army id %1 of type %2 shows up at city %3 ",
                                d_army_id, d_army->getTypeId(), d_city);
            if (d_vectored)
              s += String::ucompose("but it is vectored to another city at %1,%2.",
                                    d_dest.x, d_dest.y);
            else
              s += String::ucompose("at position %1,%2 in stack %3.", d_dest.x,
                                    d_dest.y, d_stack_id);
            s+= "\n";
            return s;
          }

	//! Save this unit produced action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Get the production details of the army that was produced.
	ArmyProdBase * getArmy() const {return d_army;}

	//! Get the Id of the City that produced the Army unit.
	guint32 getCityId() const {return d_city;}

	//! Get whether or not the Army unit is being vectored elsewhere.
	bool getVectored() const {return d_vectored;}

	//! Get where this army ends up on the map.
	Vector<int> getDestination() const {return d_dest;}

	//! Get the id of the army instance that was created.
	guint32 getArmyId() const {return d_army_id;}

	//! Get the id of the stack that the army instance that was created in.
	guint32 getStackId() const {return d_stack_id;}
    private:
	ArmyProdBase *d_army;
        guint32 d_city;
        bool d_vectored;
	Vector<int> d_dest;
	guint32 d_army_id;
        guint32 d_stack_id;

	bool load(Glib::ustring tag, XML_Helper *helper);
};

        
//-----------------------------------------------------------------------------

//! A temporary record of a vectored Army unit showing up at a city.
/**
 * The purpose of the Action_ProduceVectored class is to record when a new
 * vectored Army unit arrives at it's destination.  The idea here is that
 * two turns have passed since a unit was vectored, and now the unit has
 * shown up.
 * This action is used primarily for reporting purposes.  The player's 
 * production report shows which vectored army units arrived in what cities.
 * The army unit is taken from the Player's Armyset.
 */
class Action_ProduceVectored: public Action
{
    public:
	//! Make a new vector arrival action.
	/**
	 * Populate the Action_ProduceVectored with the Id of the army
	 * unit type being produced, and the position on the map where
	 * it has showing up.
	 */
        Action_ProduceVectored(ArmyProdBase *army, Vector<int> dest, 
                               Vector<int> src, guint32 target_army_id,
                               guint32 target_stack_id);
	//! Copy constructor
	Action_ProduceVectored(const Action_ProduceVectored &action);
	//! Load a new vector arrival action from an opened saved-game file.
        Action_ProduceVectored(XML_Helper* helper);
	//! Destroy a vector arrival action.
        ~Action_ProduceVectored();

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose
              ("Vectored army of type %1 shows up at %2,%3 from %4,%5, as army id %6 in stack id %7.\n",
               d_army->getTypeId(), d_dest.x, d_dest.y, d_src.x, d_src.y,
               d_target_army_id, d_target_stack_id);
          }

	//! Save this vector arrival action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Get the army type Id that has shown up.
	ArmyProdBase *getArmy() const {return d_army;};

	//! Get the position on the map where the army showed up.
	Vector<int> getDestination() const {return d_dest;}

	//! Get the position on the map where the army is coming from.
	Vector<int> getOrigination() const {return d_src;}

        //! Get the expected army id of the newly arrived unit.
        guint32 getTargetArmyId() const {return d_target_army_id;}

        //! Get the expected stack id of the newly arrived unit.
        guint32 getTargetStackId() const {return d_target_stack_id;}
    private:
	ArmyProdBase *d_army;
        Vector<int> d_dest;
        Vector<int> d_src;
        guint32 d_target_army_id;
        guint32 d_target_stack_id;
	bool load(Glib::ustring tag, XML_Helper *helper);
};

//-----------------------------------------------------------------------------

//! A temporary record of the diplomatic state changing.
/**
 * The purpose of the Action_DiplomacyState action is to record our
 * diplomatic state with other players has it changes.  The idea here is 
 * that every player has a diplomatic status with every other player.
 * Although we might propose war on a given turn, we would achieve the
 * state of being at war on a later turn.
 */
class Action_DiplomacyState: public Action
{
    public:
	//! Make a new diplomatic state action.
	/**
	 * Populate the Action_DiplomacyState class with the Player for
	 * which we are in a state with.  Also populate the action with
	 * the new diplomatic state.
	 */
        Action_DiplomacyState(Player *p, Player::DiplomaticState state);
	//! Copy constructor
	Action_DiplomacyState(const Action_DiplomacyState &action);
	//! Load a new diplomatic state action from an opened saved-game file.
        Action_DiplomacyState(XML_Helper* helper);
	//! Destroy a diplomatic state action.
        ~Action_DiplomacyState() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            Glib::ustring s = "declaring ";
            switch (d_diplomatic_state)
              {
              case Player::AT_WAR: s+= "war"; break;
              case Player::AT_WAR_IN_FIELD: s += "war in the field"; break;
              case Player::AT_PEACE: s += "peace"; break;
              }
            s += String::ucompose(" with player %1.\n", d_opponent_id);
            return s;
          }

	//! Save this diplomatic state action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Get the Id of the Player that we have entered a new state for.
	guint32 getOpponentId() const {return d_opponent_id;}

	//! Get the state that we're in with the other Player.
	Player::DiplomaticState getDiplomaticState() const
	  {return d_diplomatic_state;};
    private:
	guint32 d_opponent_id;
	Player::DiplomaticState d_diplomatic_state;
};

//-----------------------------------------------------------------------------

//! A temporary record of a diplomatic proposal.
/**
 * The purpose of the Action_DiplomacyProposal action is to record our
 * diplomatic proposals to other players.  The idea here is that the player
 * wishes to go to war with another Player and so offers/proposes war to a 
 * prospective enemy.
 */
class Action_DiplomacyProposal: public Action
{
    public:
	//! Make a new diplomatic proposal action.
	/**
	 * Populate the Action_DiplomacyProposal class with the Player for
	 * which we have a new proposal for.  Also populate the action with
	 * the new diplomatic proposal.
	 */
        Action_DiplomacyProposal(Player *p, Player::DiplomaticProposal proposal);
	//! Copy constructor
	Action_DiplomacyProposal(const Action_DiplomacyProposal &action);
	//! Load a new diplomatic proposal action from a saved-game file.
        Action_DiplomacyProposal(XML_Helper* helper);
	//! Destroy a diplomatic proposal action.
        ~Action_DiplomacyProposal() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            Glib::ustring s = "proposing ";
            switch (d_diplomatic_proposal)
              {
              case Player::NO_PROPOSAL: s +="nothing"; break;
              case Player::PROPOSE_WAR: s +="war"; break;
              case Player::PROPOSE_WAR_IN_FIELD: s +="war in the field"; break;
              case Player::PROPOSE_PEACE: s +="peace"; break;
              }
            s += String::ucompose(" with player %1.\n", d_opponent_id);
            return s;
          }

	//! Save this diplomatic proposal action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Get the Id of the Player that our proposal is for.
	guint32 getOpponentId() const {return d_opponent_id;}

	//! Get the proposal that we're offering.
	Player::DiplomaticProposal getDiplomaticProposal() const
	  {return d_diplomatic_proposal;};
    private:
	guint32 d_opponent_id;
	Player::DiplomaticProposal d_diplomatic_proposal;
};

//-----------------------------------------------------------------------------

//! A temporary record of the diplomatic score.
/**
 * The purpose of the Action_DiplomacyScore is to record when a Player's
 * diplomatic opinion of another Player has changed.  The idea here is that
 * an enemy player has razed a city and now our opinion of that player
 * deteriorates.
 */
class Action_DiplomacyScore: public Action
{
    public:
	//! Make a new diplomatic score action.
	/**
	 * Populate the Action_DiplomacyScore class with the Player for
	 * which we have changed our opinion of.  Also populate the action
	 * with the amount of change for that Player.  The change can be 
	 * negative, and is added to the existing score to get the new 
	 * score.
	 */
        Action_DiplomacyScore(Player *p, int amount);
	//! Copy constructor.
	Action_DiplomacyScore(const Action_DiplomacyScore &action);
	//! Load a new diplomatic score action from an opened saved-game file.
        Action_DiplomacyScore(XML_Helper* helper);
	//! Destroy a diplomatic score action.
        ~Action_DiplomacyScore() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            if (d_amount > 0)
              return String::ucompose("Adding %1 to player %2.\n",
                                      d_amount, d_opponent_id);
            else
              return String::ucompose("Subtracting %1 from player %2.\n",
                                      d_amount, d_opponent_id);
          }

	//! Save this diplomatic score action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Get the Id of the Player that our opinion has changed of.
	guint32 getOpponentId() const {return d_opponent_id;}

	//! Get the amount of the opinion change.
	int getAmountChange() const {return d_amount;};
    private:
	guint32 d_opponent_id;
	int d_amount;
};

//-----------------------------------------------------------------------------

//! A temporary record representing the ending of a turn.
class Action_EndTurn: public Action
{
    public:
	//! Make a new end turn action.
        Action_EndTurn() :Action(Action::END_TURN) {}
	//! Copy constructor
	Action_EndTurn(const Action_EndTurn &action) : Action(action) {}
	//! Load a new end turn action from an opened saved-game file.
        Action_EndTurn(XML_Helper* helper) :Action(helper) {}
	//! Destroy a end turn action.
        ~Action_EndTurn() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const { return "ending turn\n";}

	//! Save this action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const
          {
            if (helper)
              return true;
            return false;
          }
};

//-----------------------------------------------------------------------------

//! A temporary record representing the taking-over of a city.
class Action_ConquerCity : public Action
{
    public:
	//! Make a new city conquer action.
	/** Populate the action with the City being conquered.
         */
        Action_ConquerCity(City *c);
	//! Copy constructor
	Action_ConquerCity(const Action_ConquerCity &action);
	//! Load a new city conquer action from an opened saved-game file.
        Action_ConquerCity(XML_Helper* helper);
	//! Destroy a city conquer action.
        ~Action_ConquerCity() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose("City %1 occupied.\n", d_city);
          }

	//! Save this city occupied action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	guint32 getCityId() const {return d_city;};
        private:
        guint32 d_city;
};

//-----------------------------------------------------------------------------

//! A temporary record representing a new hero showing up in a city.
class Action_RecruitHero : public Action
{
    public:
	//! Make a new recruit hero action.
        Action_RecruitHero(HeroProto* h, City *c, int cost, int alliesCount, const ArmyProto *ally);
	//! Copy a new recruit hero action
	Action_RecruitHero(const Action_RecruitHero &action);
	//! Load a new recruit hero action from an opened saved-game file.
        Action_RecruitHero(XML_Helper* helper);
	//! Destroy a recruit hero action.
        ~Action_RecruitHero();

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose("Hero %1 recruited with %2 allies.\n",
                                    d_hero->getName(), d_allies);
          }

	//! Save this city occupied action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	HeroProto* getHero() const {return d_hero;};
	guint32 getCityId() const {return d_city;};
	guint32 getCost() const {return d_cost;};
	guint32 getNumAllies() const {return d_allies;};
	guint32 getAllyArmyType() const {return d_ally_army_type;};

        private:
        HeroProto *d_hero;
        guint32 d_city, d_cost, d_allies, d_ally_army_type;

        bool load(Glib::ustring tag, XML_Helper *helper);
};

//-----------------------------------------------------------------------------

//! A temporary record representing a unit production failure due to bankruptcy.
class Action_CityTooPoorToProduce: public Action
{
    public:
	//! Make a new city-too-poor action
        Action_CityTooPoorToProduce(City* c, const ArmyProdBase *army);
	//! Copy constructor
	Action_CityTooPoorToProduce(const Action_CityTooPoorToProduce &action);
	//! Load a new too-poor action from an opened saved-game file.
        Action_CityTooPoorToProduce(XML_Helper* helper);
	//! Destroy a too-poor action.
        ~Action_CityTooPoorToProduce() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return
              String::ucompose("City %1 is too poor to produce army type %2.\n",
                               d_city, d_army_type);
          }

	//! Save this city occupied action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	guint32 getCityId() const {return d_city;}
	guint32 getArmyType() const {return d_army_type;}

        private:
	guint32 d_city;
	guint32 d_army_type;
};

//-----------------------------------------------------------------------------

//! A temporary record representing the beginning of a turn.
class Action_InitTurn: public Action
{
    public:
	//! Make a new initialize turn action.
        Action_InitTurn(guint32 order);
	//! Copy constructor
	Action_InitTurn(const Action_InitTurn &action);
	//! Load a new initialize turn action from an opened saved-game file.
        Action_InitTurn(XML_Helper* helper);
	//! Destroy a initialize turn action.
        ~Action_InitTurn() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose("Initializing turn!  order is %1.\n",
                                    d_order);
          }

	//! Save this action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

        //! Return the turn order
        guint32 getTurnOrder() const {return d_order;}
    private:
        guint32 d_order;
};

//-----------------------------------------------------------------------------

//! A temporary record of what happened when a Player loots a City.
/**
 * The purpose of the Action_Loot class is to record when a Player has
 * defeated a City and has looted it.  Looting a city results in the
 * looting player's coffers gaining some gold pieces while the looted
 * player's coffers decrease.
 */
class Action_Loot : public Action
{
    public:
	//! Make a new city looting action.
	/**
         * Populate the action with the players involved and the amounts.
         */
        Action_Loot(Player *looting_player, Player *looted_player, 
                    guint32 amount_to_add, guint32 amount_to_subtract);
	//! Copy constructor
	Action_Loot(const Action_Loot &action);
	//! Load a new city looting action from an opened saved-game file.
        Action_Loot(XML_Helper* helper);
	//! Destroy a city looting action.
        ~Action_Loot() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose("Player %1 took %2 gp from player %3 who lost %4 in total.\n",
                                    d_looting_player_id, d_gold_added,
                                    d_looted_player_id, d_gold_removed);
          }

	//! Save this city looting action to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	guint32 getAmountToAdd() const { return d_gold_added;};
	guint32 getAmountToSubtract() const { return d_gold_removed;};
	guint32 getLootingPlayerId() const {return d_looting_player_id;};
	guint32 getLootedPlayerId() const {return d_looted_player_id;};

        private:
	guint32 d_looting_player_id;
	guint32 d_looted_player_id;
	guint32 d_gold_added;
	guint32 d_gold_removed;
};

//-----------------------------------------------------------------------------

//! A temporary record of a Hero using an item.
/**
 * The purpose of the Action_UseItem class is to record when a Player's
 * Hero has used an item.
 */
class Action_UseItem: public Action
{
    public:
	//! Make a new use item action.
        Action_UseItem(Hero *hero, Item *item, Player *victim, City *friendly_city, City *enemy_city, City *neutral_city, City *city);
	//! Copy constructor
	Action_UseItem(const Action_UseItem &action);
	//! Load a new use item action from a saved-game file.
        Action_UseItem(XML_Helper* helper);
	//! Destroy a use item assigned action.
        ~Action_UseItem() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose
              ("Hero %1 uses item %2 and targets player id %3 friendly city %4, enemy city %5, neutral city %6, city %7.\n",
               d_hero, d_item, d_victim_player, d_friendly_city, d_enemy_city,
               d_neutral_city, d_city);
          }

	//! Save this use item action to a saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	guint32 getHeroId() const {return d_hero;};
	guint32 getItemId() const {return d_item;};
        guint32 getVictimPlayerId() const {return d_victim_player;};
        guint32 getFriendlyCityId() const {return d_friendly_city;};
        guint32 getEnemyCityId() const {return d_enemy_city;};
        guint32 getNeutralCityId() const {return d_neutral_city;};
        guint32 getCityId() const {return d_city;};

        private:
        guint32 d_hero;
        guint32 d_item;
        guint32 d_victim_player;
        guint32 d_friendly_city;
        guint32 d_enemy_city;
        guint32 d_neutral_city;
        guint32 d_city;
};

//-----------------------------------------------------------------------------

//! A temporary record of the armies in a stack being reordered.
/**
 * The purpose of the Action_ReorderArmies class is to record when a Player
 * changes the ordering of a given stack.
 */
class Action_ReorderArmies: public Action
{
    public:
	//! Make a new reorder armies action.
	/**
         * Populate the Action_ReorderArmies with the stack.
         */
        Action_ReorderArmies(Stack *s);
	//! Copy constructor
	Action_ReorderArmies(const Action_ReorderArmies &action);
	//! Load a new reorder armies action from a saved-game file.
        Action_ReorderArmies(XML_Helper* helper);
	//! Destroy a reorder armies action.
        ~Action_ReorderArmies() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            Glib::ustring s =
              String::ucompose("Stack %1 belonging to player id %2 has a new order: ",
                               d_stack_id, d_player_id);
            for (std::list<guint32>::const_iterator i = d_army_ids.begin(); 
                 i != d_army_ids.end(); ++i)
              s += String::ucompose("%1 ", (*i));
            s += "\n";
            return s;
          }

	//! Save this reorder armies action to a saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

        std::list<guint32> getArmyIds() const {return d_army_ids;};
	guint32 getStackId() const {return d_stack_id;};
	guint32 getPlayerId() const {return d_player_id;};

        private:
        guint32 d_stack_id;
        guint32 d_player_id;
        std::list<guint32> d_army_ids;
};

//-----------------------------------------------------------------------------

//! A temporary record of the players stacks being healed and moves reset.
/**
 * The purpose of the Action_ResetStacks class is to record when a Player
 * has it's stacks healed and it's movement points recharged.
 */
class Action_ResetStacks: public Action
{
    public:
	//! Make a new reset stacks action.
        Action_ResetStacks(Player *p);
	//! Copy constructor
	Action_ResetStacks(const Action_ResetStacks &action);
	//! Load a new reset stacks action from a saved-game file.
        Action_ResetStacks(XML_Helper* helper);
	//! Destroy a reset stacks action.
        ~Action_ResetStacks() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose
              ("Stacks for player id %1 are being recharged.\n", d_player_id);
          }

	//! Save this reset stacks action to a saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	guint32 getPlayerId() const {return d_player_id;};

        private:
        guint32 d_player_id;
};

//-----------------------------------------------------------------------------

//! A temporary record of the monsters in ruins being recharged..
/**
 * The purpose of the Action_ResetRuins class is to record when the neutral
 * player heals up the monsters in ruins.
 */
class Action_ResetRuins: public Action
{
    public:
	//! Make a new reset ruins action.
        Action_ResetRuins();
	//! Copy constructor
	Action_ResetRuins(const Action_ResetRuins &action);
	//! Load a new reset ruins action from a saved-game file.
        Action_ResetRuins(XML_Helper* helper);
	//! Destroy a reset ruins action.
        ~Action_ResetRuins() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const {return "Ruins are being recharged.\n";}

	//! Save this reset ruins action to a saved-game file.
        virtual bool doSave(XML_Helper* helper) const;
};

//-----------------------------------------------------------------------------

//! A temporary record of a player making and paying out money.
/**
 * The purpose of the Action_CollectTaxesAndPayUpkeeps class is to record when 
 * a player makes money from her cities, and her stacks, and her heroes 
 * magical items and pays out money to her stacks.
 */
class Action_CollectTaxesAndPayUpkeep: public Action
{
    public:
	//! Make a new collect taxes and pay upkeep action.
        /**
         * @param fromGold is the amount of gold pieces beforehand
         * @param toGold is our new treasury afterwards
         */
        Action_CollectTaxesAndPayUpkeep(int fromGold, int toGold);
	//! Copy constructor
	Action_CollectTaxesAndPayUpkeep(const Action_CollectTaxesAndPayUpkeep &action);
	//! Load a new collect taxes and pay upkeep action from a saved-game file.
        Action_CollectTaxesAndPayUpkeep(XML_Helper* helper);
	//! Destroy a collect taxes and pay upkeep action.
        ~Action_CollectTaxesAndPayUpkeep() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose("went from %1 to %2 gp after collecting taxes from cities and paying the troops.\n",
                                    d_from_gold, d_to_gold);
          }

        int getFromGold () const {return d_from_gold;}
        int getToGold () const {return d_to_gold;}

	//! Save this collect taxes and pay upkeep action to a saved-game file.
        virtual bool doSave(XML_Helper* helper) const;
    private:
        int d_from_gold;
        int d_to_gold;
};

//-----------------------------------------------------------------------------

//! A temporary record of a player dying.
/**
 * The purpose of the Action_Kill class is to record when a player has been
 * vanquished by foes.
 */
class Action_Kill: public Action
{
    public:
	//! Make a kill action.
        Action_Kill() : Action(Action::KILL_PLAYER) {}
	//! Copy constructor
	Action_Kill(const Action_Kill &action) : Action(action) {}
	//! Load a new kill action from a saved-game file.
        Action_Kill(XML_Helper* helper) : Action(helper) {}
	//! Destroy a kill action.
        ~Action_Kill() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const { return "player is vanquished.\n";}

	//! Save this kill action to a saved-game file.
        virtual bool doSave(XML_Helper* helper) const
          {
            if (helper)
              return true;
            return false;
          }
};

//-----------------------------------------------------------------------------

//! A temporary record of a player putting a stack into defend mode.
/**
 * The purpose of the Action_DefendStack class is to record when a player puts
 * a stack into defensive mode.  This makes the stack appear as a tower.
 */
class Action_DefendStack: public Action
{
    public:
	//! Make a defend stack action.
	/**
         * Supply the stack that is being put into defend mode.
         */
        Action_DefendStack(Stack *s);
	//! Copy constructor
	Action_DefendStack (const Action_DefendStack &action);
	//! Load a new defend stack action from a saved-game file.
        Action_DefendStack (XML_Helper* helper);
	//! Destroy a defend stack action.
        ~Action_DefendStack() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose("Stack %1 is going into defend mode.\n",
                                    d_stack_id);
          }

	//! Save this defend stack action to a saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

        guint32 getStackId() const {return d_stack_id;};
    private:
        guint32 d_stack_id;
};

//-----------------------------------------------------------------------------

//! A temporary record of a player taking a stack out of defend mode.
/**
 * The purpose of the Action_UndefendStack class is to record when a player
 * takes a stack out of defensive mode.  This means that the stack is no longer
 * depicted by a tower.
 */
class Action_UndefendStack: public Action
{
    public:
	//! Make an undefend stack action.
	/**
         * Supply the stack that is being taken out of defend mode.
         */
        Action_UndefendStack(Stack *s);
	//! Copy constructor
	Action_UndefendStack (const Action_UndefendStack &action);
	//! Load a new undefend stack action from a saved-game file.
        Action_UndefendStack (XML_Helper* helper);
	//! Destroy a undefend stack action.
        ~Action_UndefendStack() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose("Stack %1 is going out of defend mode.\n",
                                    d_stack_id);
          }

	//! Save this undefend stack action to a saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

        guint32 getStackId() const {return d_stack_id;};
    private:
        guint32 d_stack_id;
};

//-----------------------------------------------------------------------------

//! A temporary record of a player putting a stack into parked mode.
/**
 * The purpose of the Action_ParkStack class is to record when a player puts
 * a stack into parked mode.
 */
class Action_ParkStack: public Action
{
    public:
	//! Make a park stack action.
	/**
         * Supply the stack that is being put into parked mode.
         */
        Action_ParkStack(Stack *s);
	//! Copy constructor
	Action_ParkStack (const Action_ParkStack &action);
	//! Load a new park stack action from a saved-game file.
        Action_ParkStack (XML_Helper* helper);
	//! Destroy a park stack action.
        ~Action_ParkStack() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose("Stack %1 is going into parked mode.\n",
                                    d_stack_id);
          }

	//! Save this park stack action to a saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

        guint32 getStackId() const {return d_stack_id;};
    private:
        guint32 d_stack_id;
};

//-----------------------------------------------------------------------------

//! A temporary record of a player taking a stack out of parked mode.
/**
 * The purpose of the Action_UnparkStack class is to record when a player
 * takes a stack out of parked mode (a stationary condition that means we
 * don't want to move this stack any more this turn).
 */
class Action_UnparkStack: public Action
{
    public:
	//! Make a unpark stack action.
	/**
         * Supply the stack that is being taken out of parked mode.
         */
        Action_UnparkStack(Stack *s);
	//! Copy constructor
	Action_UnparkStack (const Action_UnparkStack &action);
	//! Load a new unpark stack action from a saved-game file.
        Action_UnparkStack (XML_Helper* helper);
	//! Destroy an unpark stack action.
        ~Action_UnparkStack() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose("Stack %1 is going out of parked mode.\n",
                                    d_stack_id);
          }

	//! Save this unpark stack action to a saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

        guint32 getStackId() const {return d_stack_id;};
    private:
        guint32 d_stack_id;
};

//! A temporary record of a player selecting a stack to work with.
/**
 * The purpose of the Action_SelectStack class is to record when a player
 * grabs a stack to work with.  Only one stack can be selected at a time.
 */
class Action_SelectStack: public Action
{
    public:
	//! Make a select stack action.
	/** 
         * Supply the stack that is being selected
         */
        Action_SelectStack(Stack *s);
	//! Copy constructor
	Action_SelectStack (const Action_SelectStack &action);
	//! Load a new select stack action from a saved-game file.
        Action_SelectStack (XML_Helper* helper);
	//! Destroy a select stack action.
        ~Action_SelectStack() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const
          {
            return String::ucompose("Stack %1 is selected.\n", d_stack_id);
          }

	//! Save this select stack action to a saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

        guint32 getStackId() const {return d_stack_id;};
    private:
        guint32 d_stack_id;
};

//-----------------------------------------------------------------------------

//! A temporary record of a player deselecting a stack.
/**
 * The purpose of the Action_DeselectStack class is to record when a player
 * removes focus from any and all stacks.
 */
class Action_DeselectStack: public Action
{
    public:
	//! Make a deselect stack action.
        Action_DeselectStack();
	//! Copy constructor
	Action_DeselectStack (const Action_DeselectStack &action);
	//! Load a new deselect stack action from a saved-game file.
        Action_DeselectStack (XML_Helper* helper);
	//! Destroy a deselect stack action.
        ~Action_DeselectStack() {};

	//! Return some debug information about this action.
        Glib::ustring dump() const { return "Deselecting stack.\n";}

	//! Save this deslect stack action to a saved-game file.
        virtual bool doSave(XML_Helper* helper) const;
};

#endif //ACTION_H
