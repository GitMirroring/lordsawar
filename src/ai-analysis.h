//  Copyright (C) 2004 John Farrell
//  Copyright (C) 2004, 2005 Ulf Lorenz
//  Copyright (C) 2006 Andrea Paternesi
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
#ifndef AI_ANALYSIS_H
#define AI_ANALYSIS_H

#include <gtkmm.h>
#include <map>
#include "vector.h"
#include "ai-city-info.h"

class Threatlist;
class Boonlist;
class Boon;
class Player;
class City;
class Stack;
class Army;
class StackReflist;

typedef std::map<guint32, AICityInfo *> AICityMap;

//! Artificial intelligence for determining goals.
/** An AI's analysis of the game situation.
  *
  * The class has an active part as it identifies enemy stacks, cities and ruins
  * as threats to the AI's cities, categorizes them and assigns them attributes
  * (namely a rough estimate of the stack strength). Later, it is used by
  * AI_Allocation (which does the allocation of the AI's troops) as a kind of
  * container.
  *
  * See ai-smart.h for some more details about the smart AI.
  */

class AI_Analysis
{
    public:
        // Initializes the object and examines the game situation.
        AI_Analysis(Player *owner);
        ~AI_Analysis();

        
        /** Since during the AI's turn it may battle and defeat enemy stacks, it
          * neccessary to remove destroyed stacks as threats. This is done by this
          * more or less callback.
          */
        static void deleteStack(Stack* s);

	static void deleteStack(guint32 id);

        void deleteBoon (Boon *b);
        // guess the strength of the given stack. Note: next to useless outside
        // of computer turn.
        static float assessStackStrength(const Stack *stack);
        static float assessArmyStrength(const Army *army);

        
        /** get an ordered list of threats (most dangerous first)
          * 
          * @note The returned threatlist ist a pointer to the internal
          * threatlist, so don't toy around with it!
          */
        const Threatlist* getThreatsInOrder();

        /** get an ordered list of threats (closest first)
          * 
          * @param pos  the position around which the threats should be ordered
          */
        const Threatlist* getThreatsInOrder(Vector<int> pos);
        
        /** get an ordered list of boons (most valuable first)
          * 
          * only one of each kind of boon is provided
          *
          * @note The returned boonlist has boons that point into
          * the internal boonlist, so don't toy around with it!
          */
        std::list<Boon> getBoonsInOrder(Stack *s);


        std::list<std::pair<Boon,Stack*>> getBoonsInOrder ();

        // get the danger that this friendly city is in
        float getCityDanger(City *city);

        // get the number of army units in the city.
        int getNumberOfDefendersInCity(City *city);

        // returns the City that is in the higher 
        void getCityWorstDangers(float dangers[3]);

        // notify the analysis that we are sending stack to reinforce city
        void reinforce(City *city, Stack *stack, int movesToArrive);

        // return an estimate of the amount of strength needed to reinforce
        // city properly
        float reinforcementsNeeded(City *city);
        
        static void changeOwnership (Player * old_player, Player * new_player);
    private:
        // identifies and evaluates enemy cities in the citylist as threats
        void examineCities();
        
        // examine the stack list for potential threats
        void examineStacks();
        
        // examine the ruin list for potential boons
        void examineRuins();

        // examine the temple list for potential boons
        void examineTemples();
        
        // examine quests for potential boons
        void examineQuests();

        // examine backpacks on the ground as boons
        void examineBags();
        
        // calculate danger to all of our cities, populates cityInfo
        void calculateDanger();

        // the analysis currently in use
        static AI_Analysis *instance;
       
        // DATA
        // the threats to the AI
        Threatlist *d_threats;
        Boonlist *d_boons;
        Player *d_owner;
        StackReflist *d_stacks;
        AICityMap d_cityInfo;

        Boon* getBoonAlongTheWay (Boon b, Stack *s);
};

#endif
