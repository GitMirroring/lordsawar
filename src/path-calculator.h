// Copyright (C) 2009, 2014, 2020 Ben Asselstine
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
#ifndef PATH_CALCULATOR_H
#define PATH_CALCULATOR_H

#include <gtkmm.h>
#include "vector.h"

class Stack;
class Path;
class City;
class Player;
class ArmyProdBase;
class ArmyProto;

//! An object that calculates shortest paths on a weighted grid.
/** 
 */
class PathCalculator
{
public:

    //! Default constructor.
    PathCalculator(const Stack *s, bool zigzag = true, int enemy_city_avoidance = -1, int enemy_stack_avoidance = -1);

    //! Alternate constructor.  calculate with a copy of the stack.
    PathCalculator(const Stack &s, bool zigzag = true, int enemy_city_avoidance = -1, int enemy_stack_avoidance = -1);

    //! Alternate constructor.  calculate with a new stack of one army.
    PathCalculator(Player *p, Vector<int> src, const ArmyProdBase *prodbase = NULL, bool zigzag = true, int enemy_city_avoidance = -1, int enemy_stack_avoidance = -1);

    //! Copy constructor.
    PathCalculator(const PathCalculator&);

    //! Destructor.
    ~PathCalculator();

    bool isReachable(Vector<int> pos);

    Path* calculate(Vector<int> dest, guint32 &moves, guint32 &turns, guint32 &left, bool zigzag = true);

    Path* calculateToCity (City *c, guint32 &moves, guint32 &turns, guint32 &left, bool zigzag = true);
    int calculate(Vector<int> dest, bool zigzag = true);

    static bool isBlocked(const Stack *s, Vector<int> pos, bool enemy_cities_block, bool enemy_stacks_block);

    //! Return the positions on the map that are reachable in MP or less.
    std::list<Vector<int> > getReachablePositions(int mp = 0);
private:
    //! A PathCalculator helper struct for a weighted tile on the map.
    struct node
      {
	int moves;
	int turns;
	int moves_left;
      };
    struct node *nodes;
    const Stack *stack;
    bool flying;
    bool mountains;
    guint32 d_bonus;
    int land_reset_moves;
    int boat_reset_moves;
    bool zigzag;
    bool on_ship;
    int enemy_city_avoidance;
    int enemy_stack_avoidance;

    /** 
     * Checks how many movement points are needed to cross a tile from
     * an adjacent tile.
     * 
     * @param pos    This is the origin point that the Stack is moving from.
     *
     * @param dest   This is the destination point that we're checking if the 
     * 		     Stack can move to.
     *
     * @return The number of movement points required to traverse the
     *         destination tile, or -1 if movement not possible.
     */
    //! Calculates movement points to traverse an adjacent tile.
    int pointsToMoveTo(Vector<int> pos, Vector<int> next) const;

    bool load_or_unload(Vector<int> src, Vector<int> dest, bool &on_ship);

    std::list<Vector<int> > calcMoves(Vector<int> pos);
    bool calcMoves(Vector<int> pos, Vector<int> next);
    bool calcFinalMoves(Vector<int> pos);
    bool calcFinalMoves(Vector<int> pos, Vector<int> next);

    void populateNodeMap();
    /** 
     * Checks if the way to a given tile is blocked
     * 
     * This function returns whether a unit can pass over a tile from
     * another tile.  The idea here is that the "sides" of certain tiles
     * are blocked from entry.  E.g. when trying to go from water to
     * land, without going through a city.
     *
     * @param pos    This is the origin point that the Stack is moving from.
     *
     * @param dest   This is the destination point that we're checking if 
     * 		 the Stack can move to.
     *
     * @note The movement capabilities of a Stack are not taken into 
     *       account in this method.  This method should only be called 
     *       for Stack objects that are not flying.
     *
     * @return False if a non-flying stack may pass, true otherwise.  
     *         False if the two points are not adjacent.
     */
    //! Checks if the way between adjacent tiles is blocked.
    bool isBlockedDir(Vector<int> pos, Vector<int> next);

    //this method involves checking for enemy stacks, cities in the way.
    bool isBlocked(Vector<int> pos);

    void dumpNodeMap(Vector<int> dest);
    bool delete_stack;
    Stack *load_unload_stack;

};

#endif
