// Copyright (C) 2004 John Farrell
// Copyright (C) 2004, 2005 Ulf Lorenz
// Copyright (C) 2005, 2006 Andrea Paternesi
// Copyright (C) 2006 Vibhu Rishi
// Copyright (C) 2007, 2008, 2009, 2010, 2014, 2021 Ben Asselstine
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
#ifndef AI_SMART_H
#define AI_SMART_H

#include <list>
#include <gtkmm.h>

#include "real_player.h"

class Threatlist;
class Threat;
class XML_Helper;
class ArmyProdBase;
class ArmyProto;
class City;
class Location;

//! A more complex artificial intelligence Player.
/** 
 * After examining if it buys additional production, this AI uses the classical
 * strategy technique of:
 * - analyse the game situation - this involves identifying stacks and cities as
 *   threats, and deciding which are the greatest threats to which cities.
 * - allocate resources to deal with the threats to our best advantage.
 *
 * TODO:  Ruins are also identified as threats, though they are not handled yet.
 * TODO:  The Ai only buys basic productions
 * TODO:  The AI doesn't rally make use of the multifight, and it cannot really
 * handle the results (not checked)
 * TODO:  The Ai should be able to upgrade cities (Increases their income etc.)
 * TODO:  AI is way too defensive (in fact, the fast AI tends to win games)
 *
 * The code is split up in several classes. Be sure to read the comments there
 * before trying to read the code. :)
 *
 * - AI_Analysis includes the code for the assessment of the game situation
 * - AI_Allocation distributes the AI's ressources the engage threats
 * - Threat/Threatlist contains the code for the definition of single threats
 *   and the container class for threats
 * - AICityInfo is used to collect the threats and reinforcements etc. of a
 *   single city of the AI.
 *
 * Also see the Player class for the derivation scheme of players.
 */

class AI_Smart : public RealPlayer
{
    public:
        /** 
	 * Make a new AI_Smart player.
         *
         * @param name         The name of the player.
         * @param armyset      The Id of the player's Armyset.
         * @param colors       The player's colors.
	 * @param width        The width of the player's FogMap.
	 * @param height       The height of the player's FogMap.
	 * @param player_no    The Id of the player.  If this value is -1,
	 *                     the next free Id it used.
         */
	//! Default constructor.
        AI_Smart(Glib::ustring name, guint32 armyset,
                 std::vector<Gdk::RGBA> colors, int width, int height,
                 int player_no = -1);

        //! Copy constructor.
        AI_Smart(const Player&, bool sync_ids = false);
        //! Loading constructor. See XML_Helper for an explanation.
        AI_Smart(XML_Helper* helper);
	//! Destructor.
        ~AI_Smart() {};

	virtual bool isComputer() const {return true;};
	virtual void abortTurn();
        virtual bool startTurn();
        virtual void invadeCity(City* c);
        virtual bool chooseHero(HeroProto *hero, City* c, int gold);
        virtual Reward *chooseReward(Ruin *ruin, Sage *sage, Stack *stack);
        virtual void heroGainsLevel(Hero * a);
	virtual bool chooseTreachery (Stack *stack, Player *player, Vector <int> pos);
        virtual Army::Stat chooseStat(Hero *hero);
        virtual bool chooseQuest(Hero *hero);
        virtual bool computerChooseVisitRuin(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns);
        virtual bool computerChoosePickupBag(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns);
        virtual bool computerChooseVisitTempleForBlessing(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns);
        virtual bool computerChooseVisitTempleForQuest(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns);
        virtual bool computerChooseContinueQuest(Stack *stack, Quest *quest, Vector<int> dest, guint32 moves, guint32 turns);

    private:
        // Choose a new type of army to buy production for.
        int chooseArmyTypeToBuy(City *c, bool quick);

        // Consider buying new production for this city
        int maybeBuyProduction(City *c, bool quick);
        
        // Set the city to produce the best armies possible
        void setProduction(City *c);
        int setBestProduction(City *c);
        int setQuickProduction(City *c);
        
        // assign a score to an army type to try to figure out which is best
        // which is best for buying
        int scoreBestArmyType(const ArmyProto *proto);
        //which is best for producing
        int scoreBestArmyType(const ArmyProdBase *a);
        //which is quickest for producing.
        int scoreQuickArmyType(const ArmyProdBase *proto);
        //which is quickest for buying
        int scoreQuickArmyType(const ArmyProto *proto);

        // suggest somewhere that a hero stack might like to visit
        Location *getAlternateHeroTarget(Stack *s);
        
        // what is the biggest danger to this city?
        Threat *getBiggestDangerTo(City *city, Threatlist *threats);
        
        // examine cities to see if we need to change production
        void examineCities();

	// buy a scout if we need to
	void maybeBuyScout();

        bool cityNewlyTaken(City *city, guint32 turns = 2) const;

        // DATA
        int d_mustmakemoney;  // used to avoid to buy new production 
                              // and to reinforce cities to earn more money

};

#endif // AI_SMART_H
