// Copyright (C) 2004 John Farrell
// Copyright (C) 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2009, 2014 Ben Asselstine
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
#ifndef AI_ALLOCATION_H
#define AI_ALLOCATION_H

#include <gtkmm.h>

#include "stackreflist.h"

class AI_Analysis;
class Player;
class Ruin;
class Citylist;
class Stack;
class Threat;
class StackReflist;
class Threatlist;
class City;
class Quest;

//! Artificial intelligence for assigning resources to goals.
/** An AI's allocation of resources to goals identified in the analysis.
  */

class AI_Allocation
{
    public:
        AI_Allocation(AI_Analysis *analysis, const Threatlist *threats, Player *owner);
        ~AI_Allocation();

        // make the player's moves - return the number of stacks which moved.
        int move(City *first_city, bool build_capacity);

        //! remove the stack from our consideration.
        static void deleteStack(Stack* s);
        static void deleteStack(guint32 id);
        StackReflist::iterator eraseStack(StackReflist::iterator it);

	//! Emitted whenever anything happens.
	sigc::signal<void> sbusy;

    private:
        /** Assign stacks to defend cities
          * 
          * This function checks if each city is properly defended and assigns
          * additional stacks from the environment as defenders if neccessary.
          *
          * @param allCities    list of cities to be checked
          * @param stacks       list of stacks available for the task. The
          *                     allocated stacks are removed from the list.
          * @return number of stacks moved
          */
        int allocateDefensiveStacks(Citylist *allCities);

        int allocateDefensiveStacksToCity(City *city);
        
        /** Allocate stacks to threats
          * 
          * @param stacks       the list of stacks which we can choose stacks from
          *                     stacks we have dealt with are removed from the list.
          * @return the number of stacks which moved
          */
        int allocateStacksToThreats();

        int allocateStacksToThreat(Threat *threat);
        

        //! Target neutral cities and empty foreign cities.
        int allocateStacksToCapacityBuilding(City *first_city, bool take_neutrals);

        int allocateStackToCapacityBuilding(Threat *threat, City *first_city, bool take_neutrals);

        // stack return to a safe city
        bool stackReinforce(Stack *s);
        
        // search a ruin
        void searchRuin(Stack *stack, Ruin *ruin);
        
        // move armies within a city to try to make full stacks
        bool shuffleStacksWithinCity(City *city, Stack *stack, 
				     Vector<int> diff);
        
        // tell the stack to move to the point
        //MoveResult *moveStack(Stack *stack, Vector<int> pos);


        bool moveStack(Stack *stack, bool &stack_died);
	bool moveStack(Stack *stack, Vector<int> dest, bool &stack_died);

        bool shuffleStack(Stack *stack, Vector<int> dest, bool split_if_necessary);

        bool groupStacks(Stack *stack);
        
        void setParked(Stack *stack, bool force_park = false);

        // find the best attacker for the given threat
        Stack *findBestAttackerFor(Threat *threat, guint32 &num_city_defenders);
        
        // find the closest stack to the given position, but 0 if none within
        Stack *findClosestStackToCity(City *city);

        Stack *findClosestStackToEnemyCity(City *city, bool try_harder);
        
        // find a position in the city that a stack can move to
        Vector<int> getFreeSpotInCity(City *city, int stackSize);

        // find a ANOTHER position in the city that the stack can move to
        Vector<int> getFreeOtherSpotInCity(City *city, Stack *stack);
        
        // move stacks that we have no particular use for
        int defaultStackMovements();

        int continueAttacks();

        int continueQuests();
        bool continueQuest(Quest *quest, Stack *stack);

        int attackNearbyEnemies();
        
        bool checkAmbiguities();

        bool emptyOutCities();

        int visitTemples(bool get_quests);

        int pickupItems();

        int visitRuins();

        static AI_Allocation* s_instance;
        
        Player *d_owner;
        AI_Analysis *d_analysis;
        StackReflist *d_stacks;
        const Threatlist *d_threats;
};

#endif // AI_ALLOCATION_H

// End of file
