// Copyright (C) 2002, 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2003 Michael Bartl
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
#ifndef AI_DUMMY_H
#define AI_DUMMY_H

#include <list>
#include <gtkmm.h>

#include "real_player.h"

class XML_Helper;
class City;
class HeroProto;

//! A simple artificial intelligence Player suitable to be the neutral Player.
/** 
 * This class is a dummy AI used for the neutral player. It just does, well,
 * nothing.
 */

class AI_Dummy : public RealPlayer
{
    public:
        /** 
	 * Make a new AI_Dummy player.
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
        AI_Dummy (Glib::ustring name, guint32 armyset,
                 std::vector<Gdk::RGBA> colors, int width, int height,
                 int player_no = -1);

	//! Copy constructor.
        AI_Dummy(const Player& player, bool sync_ids = false);
        //! Loading constructor. See XML_Helper.
        AI_Dummy(XML_Helper* helper);
	//! Destructor.
        ~AI_Dummy() {};
        
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

	void setDefensiveProduction(City *city);
	void examineCities();

    private:
	//DATA
};

#endif // AI_DUMMY_H
