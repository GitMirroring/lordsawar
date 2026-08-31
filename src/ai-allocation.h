//  Copyright (C) 2004 John Farrell
//  Copyright (C) 2004, 2005, 2006 Ulf Lorenz
//  Copyright (C) 2009, 2014, 2026 Ben Asselstine
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
#ifndef AI_ALLOCATION_H
#define AI_ALLOCATION_H

#include <gtkmm.h>

#include "stack-ref-list.h"

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
        void move(City *first_city, bool build_capacity, sigc::slot<void(bool/*moved*/)> finish);

        //! remove the stack from our consideration.
        static void deleteStack(Stack* s);
        static void deleteStack(guint32 id);
        StackReflist::iterator eraseStack(StackReflist::iterator it);

	//! Emitted whenever anything happens.
	sigc::signal<void()> sbusy;

    private:
        /** Assign stacks to defend cities
          *
          */
        void allocateDefensiveStacks(sigc::slot<void()> after);

        /** Allocate stacks to threats
          *
          */
        void allocateStacksToThreats(sigc::slot<void()> after);


        //! Target neutral cities and empty foreign cities.
        /**
         * capacity building refers to taking neutral cities, e.g. increasing
         * our capacity to create more army units
         */
        void allocateStacksToCapacityBuilding(sigc::slot<void()> after);

        // move armies within a city to try to make full stacks
        bool shuffleStacksWithinCity(City *city, Stack *stack,
				     Vector<int> diff = Vector<int>(0,0));

        using MoveStackCallback = sigc::slot<void(bool moved, bool fought, bool died)>;
        void moveStack(Stack *s, Glib::ustring reason, MoveStackCallback after);

        bool shuffleStack(Stack *stack, Vector<int> dest, bool split_if_necessary);

        // move stacks that we have no particular use for
        void defaultStackMovements(sigc::slot<void()> after);

        void attackNearbyEnemyCities (sigc::slot<void()> after);

        void attackNearbyEnemiesInField (sigc::slot<void()> after);

        void heroesLeaveCities (sigc::slot<void()> after);

        void fullStacksLeaveCities (sigc::slot<void()> after);

        bool checkAmbiguities();

        void emptyOutCities(sigc::slot<void()> after);

        void continueAttacks(sigc::slot<void()> after);

        void doBoons (sigc::slot<void ()> after);

        bool groupStacks(Stack *stack);

        void setParked(Stack *stack, bool force_park = false);

        // find the best attacker for the given threat
        Stack *findBestAttackerFor(Threat *threat, guint32 &num_city_defenders);

        // find the closest stack to the given position, but 0 if none within
        Stack *findClosestStackToCity(City *city);

        Stack *findClosestStackToEnemyCity(City *city, bool try_harder);

        Stack *findClosestStackToCity2(City *city, float strength);

        // find a position in the city that a stack can move to
        Vector<int> getFreeSpotInCity(City *city, int stackSize);

        // find a ANOTHER position in the city that the stack can move to
        Vector<int> getFreeOtherSpotInCity(City *city, Stack *stack);
        float get_city_defender_strength (City *c);

        bool should_do_boon (const Boon *b) const;

        static AI_Allocation* s_instance;

        Player *d_owner;
        AI_Analysis *d_analysis;
        StackReflist *d_stacks;
        const Threatlist *d_threats;

        bool m_take_neutrals;
        City *m_first_city;
        bool m_did_something;
        bool debugg;

        std::function<void(std::list<Stack*>)> m_continueAttacks;
        std::function<void(std::list<Stack*>)> m_attackNearbyEnemyCities;
        std::function<void(std::list<Stack*>)> m_attackNearbyEnemiesInField;
        std::function<void(std::list<Stack*>)> m_heroesLeaveCities;
        std::function<void(std::list<Stack*>)> m_fullStacksLeaveCities;
        std::function<void(std::list<Stack*>)> m_defaultStackMovements;
        std::function<void(std::list<City *>)> m_emptyOutCities;
        std::function<void(std::list<City *>)> m_allocateDefensiveStacks;
        std::function<void(std::list<Threat*>)> m_allocateStacksToCapacityBuilding;
        std::function<void(std::list<Threat*>)> m_allocateStacksToThreats;
};

#endif
