// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004 Andrea Paternesi
// Copyright (C) 2004 John Farrell
// Copyright (C) 2004 Bryan Duff
// Copyright (C) 2006, 2007, 2008, 2009, 2014, 2021 Ben Asselstine
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
                   std::vector<Gdk::RGBA> colors, int width, int height,
                   Player::Type type = Player::HUMAN, int player_no = -1);

	//! Copy constructor.
        RealPlayer(const Player&, bool sync_ids = false);

	//! Loading constructor.
        RealPlayer(XML_Helper* helper);

	//! Destructor.
        virtual ~RealPlayer() {};

	virtual bool isComputer() const {return false;};

        virtual bool save(XML_Helper* helper) const;

	virtual void abortTurn();

        virtual bool startTurn();

        virtual void endTurn();

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

	bool d_abort_requested;

};

#endif // REAL_PLAYER_H

// End of file
