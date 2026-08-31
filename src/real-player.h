//  Copyright (C) 2003 Michael Bartl
//  Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
//  Copyright (C) 2004 Andrea Paternesi
//  Copyright (C) 2004 John Farrell
//  Copyright (C) 2004 Bryan Duff
//  Copyright (C) 2006, 2007, 2008, 2009, 2014, 2021, 2026 Ben Asselstine
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
#ifndef REAL_PLAYER_H
#define REAL_PLAYER_H

#include <list>
#include <gtkmm.h>

#include "player.h"

class XML_Helper;
class City;
class HeroProto;
class Sage;
class Ruin;
class Stack;

//! A local human Player.
/** 
 * This class implements the abstract Player class in a reasonable manner
 * for local players. It is suitable for local human players, and AI players
 * can derive from this class and overwrite the start_turn and other 
 * callback methods for their own purposes.  For complete descriptions of
 * the callback functions see the Player class.
 */

class RealPlayer : public Player
{
    public:

	//! Default constructor.
        RealPlayer(Glib::ustring name, guint32 armyset,
                   Shield::Color shield, int width, int height,
                   Player::Type type = Player::HUMAN);

	//! Copy constructor.
        RealPlayer(const Player&, bool sync_ids = false);

	//! Loading constructor.
        RealPlayer(XML_Helper* helper);

	//! Destructor.
        virtual ~RealPlayer() {};

	virtual bool isComputer() const {return false;};

        virtual bool save(XML_Helper* helper) const;

	virtual void abortTurn();

        virtual void startTurn(sigc::slot<void(bool)> finish);

        virtual void endTurn();

        virtual void invadeCity(City* c);

        virtual bool chooseHero(HeroProto *hero, City* c, int gold);

        virtual Reward *chooseReward(Ruin *ruin, Sage *sage, Stack *stack);

        virtual void heroGainsLevel(Hero * a, Army::Stat stat);

	virtual bool chooseTreachery (Stack *stack, Player *player, Vector <int> pos);
        virtual Army::Stat chooseStat(Hero *hero);
        virtual CityDefeatedChoice chooseCityDefeatedAction (City *c, Stack *s);
        
        virtual bool chooseQuest(Hero *hero);
        virtual bool chooseVisitRuin(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns);
        virtual bool choosePickupBag(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns);
        virtual bool chooseVisitTempleForBlessing(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns);
        virtual bool chooseVisitTempleForQuest(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns);
        virtual bool chooseContinueQuest(Stack *stack, Quest *quest, Vector<int> dest, guint32 moves, guint32 turns);

	bool d_abort_requested;

};

#endif
