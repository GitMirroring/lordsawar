// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2007, 2008, 2009, 2011, 2014, 2015, 2020, 2021 Ben Asselstine
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
#ifndef STACK_H
#define STACK_H

#include <list>
#include <vector>
#include "vector.h"
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include <sstream>

#include "UniquelyIdentified.h"
#include "OwnerId.h"
#include "Movable.h"

class Player;
class Path;
class Army;
class XML_Helper;
class Hero;
class Item;

//! A set of up to eight Army units that move as a single entity on the map.
/** 
 * While Army units are the actual troops you command, they always belong to a
 * stack. The stack holds these armies together in one object. The player
 * usually doesn't command the armies but the stack, so all functionality and
 * data which affects player's controls is bundled in the stack class. Among
 * this is the location of the units, the intended movement path, and more.
 */

class Stack : public ::UniquelyIdentified, public Movable, public OwnerId, public std::list<Army*>, public sigc::trackable
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

        /** 
	 * Make a new stack.
         * 
         * @param player       A pointer to the owning player.
         * @param pos          The position on the map where the stack is to 
	 *                     be created.
         */
	//! Default constructor.
        Stack(Player* player, Vector<int> pos);

	/**
	 * Copy the whole stack into a new stack.  This method performs a 
	 * deep copy of the stack's Army units.
	 */
        //! Copy constructor.
        Stack(const Stack& s, bool unique = false);

        //! Loading constructor.
        Stack(XML_Helper* helper);

	//! Destructor.
        ~Stack();

	// Get Methods

        //! Returns the minimum number of movement points of all Army units.
        guint32 getMoves() const;

	//! Returns the maximum MP the stack would have if it were on land
        guint32 getMaxLandMoves() const;

	//! Returns the max MP the stack would have if it were in the water
        guint32 getMaxBoatMoves() const;

        //! Returns the maximum number of movements points for this stack.
        guint32 getMaxMoves() const;

        //! Returns the Path object of the stack.
        Path* getPath() const {return d_path;}

	//! Return true if any of the Army units in the stack are fortified.
	bool getFortified() const;
	
	//! Calculate the number of gold pieces this stack costs this turn.
	guint32 getUpkeep() const;

        //! How many army units can be put into this stack?
        guint32 getMaxArmiesToJoin() const;

        bool hasDeadArmies() const;

	// Set Methods

        //! Change the loyalty of the stack.
        void setPlayer(Player* p);

        /** 
	 * Sets the defending value.  Defending entails that this stack is 
	 * ignored when a user cycles through the list of stacks with 
	 * Stacklist::getNextMovable().  If a stack stays defending to the
	 * next round, it gets a fortify bonus in battle.
	 */
	//! Set the defending status of the stack.
        void setDefending(bool defending){d_defending = defending;}

        /** 
	 * Sets the parked value. Parking entails that this stack is ignored
         * when a player cycles through his list of stacks with 
	 * Stacklist::getNextMovable().  This value behaves just like 
	 * defending, but there's no bonus conferred if a stack remains in 
	 * this state.
         */
	//! Set the parked status of the stack.
        void setParked(bool parked){d_parked = parked;}

	//! Sets the path object for this stack.
	void setPath(const Path p);

	//! Set all Army units in the stack to have this fortified state.
	void setFortified(bool fortified);



	// Methods that operate on class data and modify the class
	
	/**
	 * This method is used to calculate stack bonuses, moves, paths, and 
         * hp for every army in the stack.
	 */
        //!Recharge all of the armies in this stack with movement points and hp.
        void reset(bool recalculate_path = true);

        //! Reduces movement points of the Army units in the Stack.
        void decrementMoves(guint32 moves);

        //! Increases movement points of the Army units in the Stack.
        void incrementMoves(guint32 moves);

        //! Removes all movement from all Army units in the stack.
        void drainMovement();

        //! Sets the stack's position to the next point in it's Path.
        void moveOneStep(bool skipping = false);

        void moveToDest(Vector<int> dest, bool skipping = false);

	/**
	 * Adds one to the strength of each Army unit in the stack.
	 * If the Army unit has already visited the temple co-located with
	 * the stack's position, no strength bonus will be added.
	 *
	 * @return The number of Army units blessed.
	 */
	//! Bless the Army units in the stack.
        int bless();

	//! Uncovers some of the hidden map around this stack.
	void deFog();

        //! Erase the stack, deleting the Army units too.
        void flClear();

        /** 
	 * Erase an Army unit from the Stack, and free the contents of 
	 * the Army unit too (e.g. Items a Hero might be carrying).
	 *
	 * @param it   The place in the Stack to erase.
	 *
	 * @return The place in the stack that was erased.
         */
	//! Erase an Army unit from the list.
        iterator flErase(iterator object);

	/**
	 * Alter the order of the Army units in the stack according to each
	 * unit's groupedness, and fight order.
	 *
	 * The purpose of this sorting is to show the units in the stack
	 * info window.
	 *
	 * @param reverse     Invert the sort.
	 */
	//! Sort the Army units in the stack.
	void sortForViewing(bool reverse);

        void sortByStrength(bool reverse);

	//! Have the stack collect it's upkeep from a given player (owner).
	void payUpkeep(Player *p);

	//! Merge the given stack with this stack.
	void join(Stack *join);

	//! Return a new stack that holds the given armies from this stack.
	Stack *splitArmies(const std::list<Army*> &armies);

	//! Return a new stack that holds the given armies from this stack.
	Stack *splitArmies(const std::list<guint32> &armies);

	// Return a new stack holds the given army from this stack.
	Stack *splitArmy(Army *army);

        //! Drown the non-flying units over water.  sets hitpoints to zero.
        bool killArmyUnitsInBoats();

        //! Kill the armies of a given type.  sets hitpoints to zero.
        bool killArmies(guint32 army_type);

        //! Sets the hitpoints of all army units in the stack to zero.
        void kill();

	//! Add an army to this stack.
	/**
	 * This method should be used instead of push_back.
	 */
	void add(Army *army);

	//! Remove this stack's path.  Return true if anything was cleared.
	bool clearPath();

        //! Sort the armies in this stack in the order shown by ids.
        void sortByIds(const std::list<guint32> &ids);

        //! Puts the stack into or out of a ship, depending on the tile at dest.
        /*
         * If the destination tile is water tile, and it doesn't have a bridge
         * on it, then put the army units of this stack in a ship.
         * Otherwise they're on land.
         * This method must not be called on stacks that are flying.
         */
        void updateShipStatus(Vector<int> dest);

        bool removeArmiesWithoutArmyType(guint32 armyset_id);

	// Methods that operate on class and do not modify the class

        //! Returns true if the stack has any points in it's path.
        bool hasPath() const;

        //! Is there at least one hero in this stack who has a quest?
        bool hasQuest() const;

        //! Does the stack contain this kind of army?
        bool hasArmyType(guint32 army_type) const;

        //! Save the stack to an opened saved-game file.
        bool save(XML_Helper* helper) const;

	/**
         * @return True if the stack has enough moves to traverse to
	 * the next step in it's Path.  Otherwise, false.
	 */
	//! Returns whether or not the stack can move.
        bool enoughMoves() const;

	/**
	 * @return Whether or not the stack has enough moves to travel to
	 *         an adjacent tile.  The adjacent tile does not have to be
	 *         in the stack's Path.
	 */
	//! Returns whether the stack can move in any direction.
	bool canMove() const;
        
	/**
	 * Scan all adjacent tiles relative to the stack's position and 
	 * see how much a move would cost in terms of movement points.
	 * Determine the minimum amount of movement points to make a move.
	 * 
	 * @return The minimum number of movement points to travel to the
	 *         cheapest adjacent tile that the stack can afford to
	 *         move to.  If the stack cannot afford to move there, this
	 *         method returns -1.
	 */
        int getMinTileMoves() const;

        //! Return the Army unit in the Stack that has the best strength value.
        Army* getStrongestArmy() const;

        //! Return the Hero unit in the Stack that has the best strength value.
        Army* getStrongestHero() const;

	//! Go find the army with this identifier in the stack and return it.
        Army* getArmyById(guint32 id) const;
        
        //! True if the stack contains a Hero unit.  Otherwise, false.
        bool hasHero() const;

        //! Return the first Hero unit in the stack, or NULL if no Hero found.
        Army* getFirstHero() const;

        //! Return the first hero unit in the stack that is on a quest.
        Hero *getFirstHeroWithAQuest() const;

        //! Return the first hero unit in the stack that isn't questing.
        Hero *getFirstHeroWithoutAQuest() const;

        //! Returns the ids of all (living) heroes in the stack in the dst reference
	/**
	 * Scan the Army units in the Stack for heroes that have more than
	 * zero hitpoints.
	 *
	 * @param dst       Passed in as an empty or non-empty list, and
	 *                  filled up with the Ids belonging to Hero army 
	 *                  units in the stack.
	 */
	// Return the Ids of all of the Hero units in the Stack.
        void getHeroes(std::vector<guint32>& dst) const;

        //! Return the defending status of the stack.
        bool getDefending() const {return d_defending;}

        //! Return the parked status of the stack.
        bool getParked() const {return d_parked;}

        //! Returns whether the stack is being deleted.
        bool getDeleting() const {return d_deleting;}

        //! Return the maximum sight of the stack.
        guint32 getMaxSight() const;

       /** 
	* Determine which terrain kinds (Tile::Type) the Stack can travel 
	* efficiently on.  When one Army unit is good at traveling through 
	* the forest, and another in the same stack is good at traveling 
	* through the hills, the movement capabilities of each individual 
	* army is given to the other Army units in the Stack.  This means 
	* the whole stack can move well through hills and forest.
	* Traveling efficently on a tile means it takes 2 movement points
	* to traverse.
	*
	* The calculation also takes into account a movement-changing Item
	* that the Hero may be carrying (e.g. `Wings of Flying', or 
	* `Swamp Boots').
	*
	* This calculation also lets Hero units `ride' flying Army units;
	* meaning that the Hero doesn't have the ability to fly, but it
	* has the special ability to ride on the back of another flying
	* Army unit.
	*
	* @return A bitwise OR-ing of the values in Tile::Type.
        */
	//! Calculate the move bonus for the Stack.
        guint32 calculateMoveBonus() const;

	//! Calculate if the Stack has the gift of flight.
        bool isFlying () const;

        //! Check if the stack has the ability to traverse mountains or not.
        bool canMoveThroughMountains () const;

        //! Check if splitting a stack would strand army units.
        bool fliesWithItemAndNonFlyersOverWaterOrMountains() const;

	//! Calculate if the Stack is in a boat.
        bool hasShip () const;

        //! Check to see if the stack has any items that can be used.
        bool hasUsableItem() const;

        void getUsableItems(std::list<Item*> &items) const;
	/**
	 * Calculate the number of movement points it costs for the Stack
	 * to move to an adjacent tile.
	 *
	 * @note This is not a distance calculation.
	 *
	 * @param pos    The adjacent tile to calculate the movement points for.
	 *
	 * @return The number of movement points, or -1 if moving to the
	 *         adjacent tile is impossible.
	 */
	//! Return the movement points it costs to travel to an adjacent tile.
	guint32 calculateTileMovementCost(Vector<int> pos) const;

	//! Returns true if this stack can join the given stack.
	/**
	 * @note This is not a distance calculation.  It checks to see if
	 * the stack sizes are such that the amalgamated stack would be less
	 * than 8.
	 */
	bool canJoin(const Stack *stack) const;

	//! Return a list of army Ids in the stack that can reach the given 
	//! destination.
	std::list<guint32> determineReachableArmies(Vector<int> dest) const;

        //! Return a list of army ids whose strength totals strength.
        std::list<guint32> determineWeakArmies(float strength) const;
        std::list<guint32> determineStrongArmies(float strength) const;

	//! Returns how many armies in the stack have visited the given temple.
	guint32 countArmiesBlessedAtTemple(guint32 temple_id) const;
        
        //! Returns how many items this stack has.
        guint32 countItems() const;

	//!If this stack were at the given pos, would it move in/out of a ship?
	/**
	 * The on_ship paramater holds whether or not the stack is in a ship
	 * at the given position.  This is an out-parameter so that we can
	 * subsequently call this method for a series of points on a path.
	 */
	bool isMovingToOrFromAShip(Vector<int> dest, bool &on_ship) const;

	//! Get the starting point in the stack's intended path.
	/**
	 * Returns a position of -1,-1 if there isn't a path.
	 */
	Vector<int> getFirstPointInPath() const;

	//! Get the final point in the stack's intended path.
	/**
	 * Returns a position of -1,-1 if there isn't a path.
	 */
	Vector<int> getLastPointInPath() const;

	//! Gets the final point in the stack's path that we have mp to reach.
	/**
	 *
	 * This method checks how many movement points the stack currently 
	 * has, and calculates how far along it's intended path it can go.
	 *
	 * Returns the final reachable spot in the path, or returns a 
	 * position of -1,-1 if there isn't a path, or none are reachable.
	 */
	Vector<int> getLastReachablePointInPath() const;

	//! Does everything in this stack look okay?
	bool validate() const;

	//! Does this stack have 8 units in it?
	bool isFull() const;

        //! Return the hero that owns this given item.
        Hero* getHeroWithItem(Item *item) const;

	// Signals

	//! Emitted when this stack dies.
        sigc::signal<void, Stack*> sdying;

	//! Emitted when this stack is about to move one step
	sigc::signal<void, Stack*> smoving;

	//! Emitted when this stack has finished moving that one step
	sigc::signal<void, Stack*> smoved;

	//! Emitted when this stack is grouped or ungrouped
	sigc::signal<void, Stack*, bool> sgrouped;


	// Static Methods
        
	/**
	 * This comparator function compares the fight order of two Army units.
	 *
	 * @param left    An army that we want to sort by fight order.
	 * @param right   An army that we want to sort by fight order.
	 *
	 * @return True if the fight order of the left army is more than
	 *         the fight order of the right army.
	 */
	//! Comparator function to assist in sorting the armies in the stack.
	static bool armyCompareFightOrder (const Army *left, const Army *right);
	static bool armyCompareStrength (const Army *left, const Army *right);

	//! Create a stack with an id that isn't unique.
	static Stack* createNonUniqueStack(Player *player, Vector<int> pos);

        bool isOnCity() const;
    private:    

        std::list<guint32> determineArmiesByStrength(float strength) const;

static bool compareIds(const Army *lhs, const Army *rhs);
	//! Private constructor.
	Stack(guint32 id, Player* player, Vector<int> pos);

        //! Callback for loading the object from an opened saved-game file.
        bool load(Glib::ustring tag, XML_Helper* helper);
    
	//! Helper method for returning strongest army.
	Army* getStrongestArmy(bool hero) const;

        // DATA

	//! The stack's intended path.
        Path* d_path;

	//! Whether or not the stack is defending.
        bool d_defending;

	//! Whether or not the stack is parked.
        bool d_parked;
        
	/**
	 * True if the stack is currently being deleted. This is neccessary as
	 * some things may happen in the destructor of the contained armies and
	 * we don't want bigmap to draw the stack when it is being removed.
	 */
	//! Whether or not this stack is in the midst of being deleted.
        bool d_deleting;
};

guint32 getFightOrder(std::list<guint32> values, guint32 value);

#endif // STACK_H

// End of file
