// Copyright (C) 2009, 2010, 2014 Ben Asselstine
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

#include "RoadPathCalculator.h"
#include "PathCalculator.h"
#include "stack.h"
#include "armysetlist.h"
#include "tileset.h"
#include "path.h"
#include "army.h"

#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::flush<<std::endl;}
//#define debug(x)

RoadPathCalculator::RoadPathCalculator(Vector<int> starting_point, bool fly)
{
  stack = new Stack(NULL, starting_point);

  ArmyProto *basearmy =
    fly ? ArmyProto::createBat () : ArmyProto::createScout();
  Army *a = Army::createNonUniqueArmy(*basearmy);
  delete basearmy;
  stack->add(a);
  path_calculator = new PathCalculator(stack, false);
}

RoadPathCalculator::RoadPathCalculator(const RoadPathCalculator &r)
{
  stack = new Stack(*r.stack);
  path_calculator = new PathCalculator(*r.path_calculator);
}

RoadPathCalculator::~RoadPathCalculator()
{
  delete stack;
  delete path_calculator;
}

guint32 RoadPathCalculator::calculate_moves(Vector<int> dest)
{
  guint32 moves = 0, turns = 0, left = 0;
  Path *p = path_calculator->calculate(dest, moves, turns, left, false);
  delete p;
  return moves;
}

Path* RoadPathCalculator::calculate(Vector<int> dest)
{
  guint32 moves = 0, turns = 0, left = 0;
  return path_calculator->calculate(dest, moves, turns, left, false);
}

Path* RoadPathCalculator::calculate(Vector<int> dest, guint32 &moves)
{
  guint32 turns = 0, left = 0;
  return path_calculator->calculate(dest, moves, turns, left, false);
}

void RoadPathCalculator::regenerate()
{
  if (path_calculator)
    delete path_calculator;
  path_calculator = new PathCalculator(stack, false);
}

Vector<int> RoadPathCalculator::getPos() const
{
  if (stack)
    return stack->getPos();
  return Vector<int>(-1,-1);
}
