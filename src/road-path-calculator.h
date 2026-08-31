// Copyright (C) 2009 Ben Asselstine
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
#ifndef ROADPATH_CALCULATOR_H
#define ROADPATH_CALCULATOR_H

#include <gtkmm.h>
#include "vector.h"

class Path;
class Stack;
class PathCalculator;

//! An object that calculates shortest paths on a weighted grid.
/** 
 */
class RoadPathCalculator
{
 public:

     //! Default constructor.
     RoadPathCalculator(Vector<int> starting_point, bool fly = false);

     //! Copy constructor.
     RoadPathCalculator(const RoadPathCalculator&);

     //! Destructor.
     ~RoadPathCalculator();

     // Get Methods
     Vector<int> getPos() const;

     // Methods that operate on the class data and modify the class.
 
     //! Return a calculated path from the starting point to the given position.
     Path* calculate(Vector<int> dest);
     Path* calculate(Vector<int> dest, guint &moves);
     guint32 calculate_moves(Vector<int> dest);

     void regenerate();

 private:

     // DATA

     //! The stack with the movement characteristics to make the road with.
     Stack *stack;

     //! The path calculator that does the hard work.
     PathCalculator *path_calculator;

};

#endif
