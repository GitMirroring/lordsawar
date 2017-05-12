// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2007, 2008, 2014 Ben Asselstine
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
#ifndef NEXT_TURN_H
#define NEXT_TURN_H

#include <sigc++/trackable.h>
#include <sigc++/connection.h>

class Player;
/**
   \brief The class to pass turns around the players
 
   \note    This class takes care of calling the correct players in the correct
            order. The problem is, no other class really has the scope of doing
            this whole stuff. The playerlist and the game object aren't the
            right candidates (they are busy with other things) and the former
            algorithm of each player calling the next one produces a huge
            stackload if two ai players fight each other. Plus, you want to do
            several actions at the end or the beginning of each player's turn
            or each round and therefore want a central place for this code.
 */

class NextTurn: public sigc::trackable
{
    public:
        /**
           \brief constructor

           @param   turnmode    setting for the private variable d_turnmode
           @param   random_turns change the order every round
         */
        NextTurn(bool turnmode, bool random_turns);

        /**
           \brief destructor
         */
        virtual ~NextTurn() {};
        
        /**
           \brief start a new game
          
           This function starts with the currently active player, or the first
           if there is none active. For starting a game. This should be the
           lowest of all scenario-related functions in the stack.
         */
        virtual void start()=0;

        /**
           \brief go on to the next player
           
           This function starts the next to the active player's turn. Used when
           a human player has pushed the next_turn button.
         */
        virtual void endTurn()=0;

	void stop();

	void nextPlayer();

        void setContinuingTurn() { continuing_turn = true; };

        /**
           \brief signals for announcing events
         */
        sigc::signal<void, Player*> splayerStart;

	// emitted whenever a new player's turn starts.
        sigc::signal<void, Player*> snextTurn;
        
        //! Signal which is emitted whenever a new round starts
        sigc::signal<void> snextRound;

        //! Signal as a workaround for a display bug; updates the screen
        sigc::signal<void> supdating;

	//! Signal when we're done doing next-turn duties.
        sigc::signal<void> srequestAbort;

    protected:

        /** \brief determines whether armies are healed/produced at the
          * beginning of a round or at the beginning of each player's turn.
          *
          * If the value is set to true, the production/healing of armies takes
          * place at the beginning of each player's turn (which is fairer); else
          * all armies of all playes are healed when a new game round starts.
          * The latter setting is a bit unfair, because the last player knows
          * that his armies are healed immediately when he has finished his turn
          * while the other player's armies may have to survive some attacks,
          * but it may be useful in some circumstances.
          */
        bool d_turnmode;

        bool d_random_turns;

        //! If set to true, the game is interrupted at the next occasion
        bool d_stop;

        // whether we're starting a turn again from loading a game
        bool continuing_turn;

    protected:

	sigc::connection abort;
};

#endif //NEXT_TURN_H
