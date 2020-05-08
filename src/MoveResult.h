// Copyright (C) 2004 John Farrell
// Copyright (C) 2005, 2007 Ulf Lorenz
// Copyright (C) 2009, 2010, 2014, 2020 Ben Asselstine
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
#ifndef MOVE_RESULT_H
#define MOVE_RESULT_H

#include "fight.h"

class Stack;

/** 
  * This is needed by the AI so it can tell when a stack dies.
  */
//! The result of a move by a stack.
class MoveResult
{
    public:
        MoveResult();
        ~MoveResult() {};

        //! set the result of any fight that happened
        void setFightResult(Fight::Result d_fightResult);

        //! set how many steps were taken in this move
        void setStepCount(int stepCount) { d_stepCount = stepCount; }

        int getStepCount() const {return d_stepCount;};

        //! return the result of the fight, if there was one
        Fight::Result getFightResult() const { return d_fightResult; }

        //! did anything actually happen in this move?
        bool didSomething() const { return (d_fight || (d_stepCount > 0) ); }

	void setReachedEndOfPath(bool reached) {d_reached_end = reached;};
	bool getReachedEndOfPath() const {return d_reached_end;}

	bool getOutOfMoves() const {return d_out_of_moves;}

	void setTreachery(bool treachery) {d_treachery = treachery;}
	bool getTreachery() const {return d_treachery;}

	void setConsideredTreachery(bool considered) {d_considered_treachery = considered;}
	bool getConsideredTreachery() const {return d_considered_treachery;}

        void setTooLargeStackInTheWay(bool s) {d_too_large_stack_in_the_way=s;}
        bool getTooLargeStackInTheWay() const {return d_too_large_stack_in_the_way;}

        void setMoveAborted(bool a) {d_move_aborted = a;}
        bool getMoveAborted() const {return d_move_aborted;}

        void setComputerGotQuest(bool got_quest) {d_computer_got_quest = got_quest;}
        bool getComputerGotQuest() const {return d_computer_got_quest;}

        void setComputerSearchedTemple(bool searched) {d_computer_searched_temple = searched;}
        bool getComputerSearchedTemple() {return d_computer_searched_temple;}

        void setComputerSearchedRuin(bool searched) {d_computer_searched_ruin = searched;}
        bool getComputerSearchedRuin() {return d_computer_searched_ruin;}

        void setRuinFightResult(Fight::Result result) {d_ruinfightResult = result;}
        Fight::Result getRuinFightResult() const {return d_ruinfightResult;}

        void setComputerPickedUpBag(bool picked_up) {d_computer_picked_up_bag = picked_up;}
        bool getComputerPickedUpBag() {return d_computer_picked_up_bag;}
	//! fill up d_out_of_moves, d_reached_end, and d_stepCount
	void fillData(Stack *s, int stepCount, bool searched_temple, bool searched_ruin, bool got_quest, bool picked_up);

    private:
        bool d_result;
        bool d_fight;
        int d_stepCount;
	bool d_out_of_moves;
	bool d_reached_end;
	bool d_treachery;
	bool d_considered_treachery;
        //this is when we can't jump over a friendly stack.
        bool d_too_large_stack_in_the_way;
        Fight::Result d_fightResult;
        bool d_move_aborted;
        bool d_computer_searched_temple;
        bool d_computer_searched_ruin;
        bool d_computer_got_quest;
        Fight::Result d_ruinfightResult;
        bool d_computer_picked_up_bag;
};

#endif // MOVE_RESULT_H

// End of file
