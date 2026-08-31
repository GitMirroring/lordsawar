//  Copyright (C) 2000, 2001, 2003 Michael Bartl
//  Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
//  Copyright (C) 2004, 2005 Andrea Paternesi
//  Copyright (C) 2004 John Farrell
//  Copyright (C) 2007, 2008, 2009, 2010, 2026 Ben Asselstine
//  Copyright (C) 2008 Ole Laursen
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
#ifndef PATH_H
#define PATH_H

#include <gtkmm.h>
#include <list>
#include "vector.h"

class Stack;
class XML_Helper;
class City;

//! A list of waypoint coordinates (Vector<int>) on the GameMap.
/** 
 * The path class is used to store movement paths, determine new movement 
 * paths, and to query existing movement paths.
 *
 */
class Path : public std::list<Vector<int> >
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

        //! Default constructor.
        Path();

	//! Copy constructor.
	Path(const Path& p);

        //! Make a new path by loading it from an opened saved-game file.
        Path(XML_Helper* helper);
	//! Destructor.
        ~Path();

        //! Save the path to an opened saved-game file.
        bool save(XML_Helper* helper) const;

        /** 
         * Check if the path is blocked for some reason, and recalculate it
	 * if necessary.
         *
         * @param stack        The Stack whose path we validate.
	 * @param enemy_city_avoidance Return false if a path's point is on an
	 * enemy city, or not if this value is non-negative.
	 * @param enemy_stack_avoidance Return false if a path's point is on an
	 * enemy stack, or not if this value is non-negative.
	 *
         * @return True if path is valid, False if path is blocked and could
         *         not be recalculated, True if the path was invalid but was
	 *         recalculated succesfully.
         */
	//! Validate an existing path.
        bool checkPath(Stack* stack, int enemy_city_avoidance = -1, int enemy_stack_avoidance = -1);

        /** 
	 * Calculates the path from the stack's position to a destination.
         *
         * The calculated path is stored within the instance (remember: each
         * stack has a path instance). During calculation, bonuses and other
         * specialities are taken into account.
         *
         * @param stack        The stack whose path we calculate.
         * @param dest         The destination point to calculate for.
	 * @param zigzag       Whether we're using the normal way to
	 *                     calculate paths or not.  False means we never 
	 *                     go diagonally.  True means we do.
	 * @param turns        This variable gets filled up with the number of
	 *                     turns it takes to get to the destination.  If
	 *                     the destination can be reached in this turn,
	 *                     the value returned is 0.
	 *
         * @return The number of movement points needed to destination or 0
         *         if no path is possible.
         */
	//! Calculate a Stack object's Path to a destination on the GameMap.
        guint32 calculate(Stack* stack, Vector<int> dest, guint32 &turns, bool zigzag = true);
        guint32 calculate(Stack* stack, Vector<int> dest, bool zigzag = true);

	//! Recalculate a Stack object's Path.
	void recalculate (Stack* s);

	//! Return the number of points the stack can move along it's path.
	guint32 getMovesExhaustedAtPoint() {return d_moves_exhausted_at_point;}

	/**
	 * Set the point at which the stack can't move along it's path.
	 * If the first point in the stack's path cannot be moved to,
	 * this method should return 0.  If the second point can't be moved 
	 * to, then this method should return 1, etc.
	 * 
	 * The purpose of this method is to assist in drawing the waypoint
	 * graphics.
	 *
	 * @param index   The index of the point in the stack's path that
	 *                cannot be moved to.
	 */
	//! Set the number of points the stack can move along it's path.
	void setMovesExhaustedAtPoint(guint32 index) 
	  {d_moves_exhausted_at_point = index;}

        void eraseFirstPoint();
        
	//! find which tile in the city is quickest to move to.
	guint32 calculateToCity (Stack *s, City *c, bool zigzag = true);
	void calculate (Stack* s, Vector<int> dest, guint32 &mp, guint32 &turns, guint32 &left, bool zigzag = true);

        //! return tile positions that are DIST away from a point on the path
        std::vector<Vector<int>> getNearbyPoints (int dist) const;
    private:

        int pointsToMoveTo(const Stack *s, int x, int y, int destx, int desty) const;

	bool load_or_unload(Stack *s, Vector<int> src, Vector<int> dest, bool &on_ship);

        // Data

	//! The point in the path that can't be reached.
	guint32 d_moves_exhausted_at_point;

};

#endif
