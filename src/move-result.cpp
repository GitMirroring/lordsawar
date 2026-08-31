//  Copyright (C) 2004 John Farrell
//  Copyright (C) 2005 Ulf Lorenz
//  Copyright (C) 2009, 2010, 2014, 2026 Ben Asselstine
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

#include <iostream>
#include "move-result.h"
#include "fight.h"
#include "stack.h"
#include "path.h"

#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::flush<<std::endl;}
//#define debug(x)

MoveResult::MoveResult()
    : d_result (false), d_fight(false), d_stepCount(0), d_out_of_moves(false), 
    d_reached_end(false), d_treachery(false), d_considered_treachery(false),
    d_too_large_stack_in_the_way(false), d_fightResult(FightResult::DRAW),
    d_move_aborted(false), d_computer_searched_temple(false), 
    d_computer_searched_ruin(false), d_computer_got_quest(false),
    d_ruinfightResult(FightResult::DRAW), d_computer_picked_up_bag(false)
{
}

void MoveResult::fillData(Stack *s, int stepCount)
{
  if (s->getPath()->size() == 0)
    d_reached_end = true;

  if (s->enoughMoves() == false)
    d_out_of_moves = true;

  d_stepCount = stepCount;
}


void MoveResult::setFightOutcome(FightResult::Outcome fightResult)
{
    d_fight = true;
    d_fightResult = fightResult;
}
