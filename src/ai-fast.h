//  Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
//  Copyright (C) 2003 Michael Bartl
//  Copyright (C) 2004 Andrea Paternesi
//  Copyright (C) 2007, 2008, 2009, 2010, 2014, 2020, 2021, 2026 Ben Asselstine
//  Copyright (C) 2007, 2008 Ole Laursen
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
#ifndef AI_FAST_H
#define AI_FAST_H

#include <list>

#include "real-player.h"
#include "ai-analysis.h"
#include "ai-diplomacy.h"
class XML_Helper;
class City;


//! A simple artificial intelligence Player.
/** 
 * This AI has two modes. In normal modes it basically assembles stacks of
 * 8 units each and sends them to the next city, reinforcing them in own cities
 * if neccessary. In maniac mode, however (meant for wandering monsters etc.),
 * this AI will attack everything that is close up or take the nearest city if
 * no enemies are close. When it takes over an enemy city, it razes it.
 * 
 */

class AI_Fast : public RealPlayer
{
    public:
        /** 
	 * Make a new AI_Fast player.
         * 
         * @param name         The name of the player.
         * @param armyset      The Id of the player's Armyset.
         * @param shield       The player's index into the Shieldset.
	 * @param width        The width of the player's FogMap.
	 * @param height       The height of the player's FogMap.
         */
	//! Default constructor.
        AI_Fast(Glib::ustring name, guint32 armyset,
                Shield::Color shield, int width, int height);

        //! Copy constructor.
        AI_Fast(const Player&, bool sync_ids = false);

        //! Loading constructor. See XML_Helper for an explanation.
        AI_Fast(XML_Helper* helper);

	//! Destructor.
        ~AI_Fast();
        
	virtual bool isComputer() const {return true;};

        //! Saves data, the method is for saving additional data.
        bool save(XML_Helper* helper) const;

	virtual void abortTurn();
        virtual void startTurn(sigc::slot<void(bool)> finish);
        virtual void invadeCity(City* c);
        virtual bool chooseHero(HeroProto *hero, City* c, int gold);
        virtual Reward *chooseReward(Ruin *ruin, Sage *sage, Stack *stack);
        virtual void heroGainsLevel(Hero * a, Army::Stat stat);
	virtual bool chooseTreachery (Stack *stack, Player *player, Vector <int> pos);
        virtual Army::Stat chooseStat(Hero *hero);
        virtual bool chooseQuest(Hero *hero);
        virtual CityDefeatedChoice chooseCityDefeatedAction (City *c, Stack *s);
        virtual bool chooseVisitRuin(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns);
        virtual bool choosePickupBag(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns);
        virtual bool chooseVisitTempleForBlessing(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns);
        virtual bool chooseVisitTempleForQuest(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns);
        virtual bool chooseContinueQuest(Stack *stack, Quest *quest, Vector<int> dest, guint32 moves, guint32 turns);

    private:
        //! The core function of the ai's logic.
        /*
         * it runs a set of numbered steps, seen as methods below.
         */
        void initComputerTurn ();
        void computerTurn(sigc::slot<void()> finish);

	//! search through our stacklist for a stack we can join
	Stack *findNearOwnStackToJoin(Stack *s, int max_distance);

        //! produce the best low-turn high strength army unit.
        int setBestProduction(City *c);

        int scoreArmyType(const ArmyProdBase *a);

	//! Determines whether to join units or move them separately.
        bool d_join;

	//! Maniac mode: kill and raze everything you encounter.
        bool d_maniac;

        AI_Analysis* d_analysis;
        AI_Diplomacy *d_diplomacy;

        Stack* get_next_stack_from_list ();
        void step1 (Stack *s, sigc::slot<void()> next);
        void step2 (Stack *s, sigc::slot<void()> next);
        void step3 (Stack *s, sigc::slot<void()> next);
        void step4 (Stack *s, sigc::slot<void()> next);
        void step5 (Stack *s, sigc::slot<void()> next);
        bool should_do_boon (const Boon *b) const;

        std::list<Vector<int>> m_stack_points;
        std::list<Vector<int>>::iterator m_stack_points_iterator;
        bool m_did_something; // did something with current stack
        bool m_dirty; // we did something with at least one stack in our list
        bool debugg;

};

#endif
