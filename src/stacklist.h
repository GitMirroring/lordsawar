// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2004 Andrea Paternesi
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2014, 2021 Ben Asselstine
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
#ifndef STACKLIST_H
#define STACKLIST_H

#include <list>
#include <map>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm.h>
#include "vector.h"
#include <sstream>
#include <sigc++/signal.h>

class City;
class Stack;
class XML_Helper;
class Player;
class Hero;
class Item;

//! A list of Stack objects for a single player.
/** 
 * All stacks owned by a Player are contained in a Stacklist.  This class
 * covers the loading and saving of stack lists, and also some methods for
 * getting and managing groups of stacks.
 */

class Stacklist : public std::list<Stack*>, public sigc::trackable
{
    public:

	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

	//! Default constructor.
        Stacklist();

	//! Copy constructor.
        Stacklist(Stacklist *stacklist);

	//! Loading constructor.
        Stacklist(XML_Helper* helper);

	//! Destructor.
        ~Stacklist();

	// Set Methods

	//!Set the stack the player is currently working with.
        /** 
	 * Sets the currently selected stack. The purpose of this method is 
         * to designate a stack to be the one the player is currently touching.
         * It is important to use this method because several functions
	 * expect that there is an active stack.
         *
         * @param activestack      The stack currently selected by the player.
         */
        void setActivestack(Stack* activestack);


	// Get Methods

        //! Returns the currently selected stack.
        Stack* getActivestack() const {return d_activestack;}


	// Methods that operate on class data and do not modify the class.

        //! Returns the total number of armies in the list.
        unsigned int countArmies() const;

	/**
	 * Scan through the list of stacks to find one that is not defending, 
	 * and not parked, and can move to another tile.
	 *
	 * @return A pointer to the next moveable stack or NULL if no more
	 *         stacks can move.
	 */
        //! Return the next moveable stack in the list.
        Stack* getNextMovable() const;

	/**
         * @return True if any stacks in the list have enough moves for 
	 * it's next step along it's Path.  Otherwise, false.
	 */
	//! Returns whether or not any stacks can move.
        bool enoughMoves() const;

        //! Save the data to an opened saved-game file.
        bool save(XML_Helper* helper) const;

	//! Find a stack in this stacklist given this id.
        Stack *getStackById(guint32 id) const;

	//! Find the stack in this stacklist that contains an army with this id.
        Stack *getArmyStackById(guint32 army) const;

	//! Collect gold pieces from army units in the list that give money.
	/**
	 * Heroes in a stacklist can provide gold pieces by carrying items.
	 * These items are +3 gold per city, etc so the number of cities
	 * must be passed into this method.
	 *
	 * This method increases the given player's treasury.
	 */
	void collectTaxes(Player *p, guint32 num_cities) const;


        //! Determine how much it costs for the stacks in our list.
        guint32 calculateUpkeep() const;

	//! Return a list of the heroes in the stacklist.
	std::list<Hero*> getHeroes() const;

	//! Return a hero from this stacklist that is closest to the given pos.
	/**
	 * The hero must be within a given distance, provided by the dist
	 * parameter.  The distance is measured in tiles (not movement points).
	 */
	Hero *getNearestHero(Vector<int> pos, int dist) const;
	
        //! Return a list of all usable items held by heroes in the stacklist.
        std::list<Item*> getUsableItems() const;

        //! Return whether or not the stacklist has heroes holding usable items.
        bool hasUsableItem() const;
        
        //! Return which stack and hero the item belongs to.
        bool getItemHolder(Item *item, Stack **stack, Hero **hero) const;

        //! Count the stacks that can move.
        guint32 countMovableStacks() const;

	// Methods that operate on class data and modify the class.

	//! remove all movement points from every army in every stack.
	void drainAllMovement();

        //! Callback method executed at the end of every turn.
        void nextTurn();

        //! Charge up the movement points and heal every army of the stack.
        void resetStacks();

	//! Have everyone in this stacklist collect their upkeep from a player.
	/**
	 * This method decreases the given player's treasury.
	 */
	void payUpkeep(Player *p);

        //! Erase all stacks from the list, and their contents too.
        void flClear();

	//! Add a stack to the list.
	void add(Stack *stack);

        //! Army units in boats are disbanded.  return list of affected stacks.
        std::list<Stack*> killArmyUnitsInBoats();

        //! Kill all the army units in the stack list.
        std::list<Stack*> kill();

        //! Kill this kind of army unit in the stack list.
        std::list<Stack*> killArmies(guint32 army_type);

        /** 
	 * Erase a Stack from the list, and free the contents of the Stack.
	 *
	 * @param it   The place in the Stacklist to erase.
	 *
	 * @return The place in the list that was erased.
         */
	//! Erase a stack from the list.
        iterator flErase(iterator object);

        /** 
	 * Scan the list of stacks for a particular stack.  If it is found,
	 * remove it from the list of stacks and free it's contents.
	 *
	 * @param stack  The stack in the Stacklist to remove.
	 *
	 * @return Whether or not the stack was found and deleted.
         */
        //! Erase a stack from the list.
        bool flRemove(Stack* stack);

	//! Erase a stack from the list, given the stack id.
        bool flRemove(guint32 id);

	//! Callback for when a stack has been killed, or disbanded.
        /**
         * this is needed to be public so that we can update d_id from
         * the editor without setting up signals.
         */
	void on_stack_died (Stack *stack);

	// Signals

	//! The stack in the stacklist has been grouped or ungrouped.
	sigc::signal<void, Stack*, bool> sgrouped;

	//! The stack in the stacklist is arriving on a new tile.
	sigc::signal<void, Stack*, Vector<int> > snewpos;

	//! The stack in the stacklist is leaving an old tile.
	sigc::signal<void, Stack*, Vector<int> > soldpos;

        //! A stack died
        sigc::signal<void> sstackDied;

	// Static Methods

        //! Searches through the all players Stacklists and deletes the stack.
        static bool deleteStack(Stack* stack);

	//! Searches for the stack id in all lists, and deletes it.
	static bool deleteStack(guint32 id);

        /** 
	 * Scan each tile occupied by the given city and return a list of
	 * stacks who are in the city.
         *
         * When a city is attacked, all stacks which occupy a city tile are
         * regarded as defenders.  The purpose of this function is to 
	 * enumerate the defending stacks when a stack has attacked a city.
         *
         * @param city        The city to search for stacks in.
	 *
         * @return A list of all stacks defending the city.
         */
	//! Return a list of stacks defending a city.
        static std::vector<Stack*> getDefendersInCity(const City* city);

        //! Returns the total number of stacks owned by all players.
        static unsigned int getNoOfStacks();

        //! Returns the total number of armies owned by all players.
        static unsigned int getNoOfArmies();

	/**
	 * This method removes the given stack from it's owner's stacklist and
	 * places it in the new owner's stacklist.
	 */
	//! Change allegience of the given stack to a new owner.
	static Stack* changeOwnership(Stack *stack, Player *new_owner);

        //! change allegience for all stacks in the stacklist.
        static void changeOwnership(Player *old_owner, Player *new_owner);

	//! Can this stack reach a point where it is not blocked by friendlies?
	/**
	 * The idea here is that the stack is blocked by a friendly stack on
	 * it's path.  This method checks to see if the stack has enough 
	 * movement points to proceed through the friendly stacks (that are
	 * too big to join with), and land on a point in it's path beyond it.
	 */
	static bool canJumpOverTooLargeStack(Stack *s);

        //! Return position of an Army with the given id in this stacklist.
	/**
	 * Scan through all stacks in the list, and then through each Army 
	 * unit of every Stack for an Army unit with a particular Id.
	 *
	 * @param id     The Id of the Army unit that we're looking for.
	 *
	 * @return The position of the Army unit.  If no Army unit could be
	 *         found with the given Id, the position of (-1,-1) is
	 *         returned.
	 */
        static Vector<int> getPosition(guint32 id);


        std::list<Vector<int> > getPositions() const;

        std::list<Stack*> getStacksWithItems() const;

        bool check();
    private:

        //! Callback for loading this object from an opened saved-game file.
        bool load(Glib::ustring tag, XML_Helper* helper);

	//! Callback for when a stack initiates movement after being stopped.
	void on_stack_starts_moving (Stack *s);

	//! Callback for when a stack halts.
	void on_stack_stops_moving (Stack *s);

	//! Callback for when a stack has been grouped or ungrouped.
	void on_stack_grouped (Stack *stack, bool grouped);

	void getHeroes(std::vector<guint32>& dst) const;

	//! Notify the game map that a stack is leaving it's tile.
	bool deletePositionFromMap(Stack *stack) const;

	//! Notify the game map that a stack is arriving on a tile.
	bool addPositionToMap(Stack *s) const;

	// DATA

	//! A pointer to the currently selected Stack.
        Stack* d_activestack;

	typedef std::map<Stack *, std::list<sigc::connection> > ConnectionMap;
	// A set of signal connections for each stack in our list.
	/**
	 * We can use this map to lookup the outstanding signals connections
	 * that a given stack has.  This is so that we can destroy them
	 * when the stack is deleted.
	 */
	ConnectionMap d_connections;
	
	typedef std::map<guint32, Stack*> IdMap;
	//! A map to quickly lookup the stack by it's unique id.
	IdMap d_id;

};

#endif // STACKLIST_H

// End of file
