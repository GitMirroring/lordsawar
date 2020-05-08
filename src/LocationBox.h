// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2000, 2001, 2002, 2004, 2005 Ulf Lorenz
// Copyright (C) 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009, 2014, 2020 Ben Asselstine
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
#ifndef LOCATION_BOX_H
#define LOCATION_BOX_H

#include <gtkmm.h>
#include "vector.h"
#include "Immovable.h"
#include "rectangle.h"


class Movable;
class Player;
class Stack;
class Army;
//! A reference to a rectangular place on the game map.
/** 
 * A LocationBox is a place on the map that has a size.   The size is how many
 * tiles the place is tall and wide.
 */
class LocationBox : public Immovable
{
 public:

     //! Default constructor.
     /**
      * @param pos     The top-right corner of the feature is located at this
      *                position on the game map.
      * @param size    The number of tiles wide and high the feature is.
      */
     LocationBox(Vector<int> pos, guint32 size = 1);

     //! Non-Standard constructor.
     /**
      * Make a LocationBox from two points.
      */
     LocationBox(Vector<int> src, Vector<int> dest);

     //! Copy constructor.
     LocationBox(const LocationBox&);

     //! Alternative copy constructor that gives the object a new position.
     LocationBox(const LocationBox&, Vector<int> pos);

     //! Loading constructor.
     /**
      * Load the location box from an opened saved-game file.
      *
      * @param helper  The opened saved-game file to read the location from.
      * @param size    The size of the place.  This value is not read in
      *                from the saved-game file.
      */
     LocationBox(XML_Helper* helper, guint32 size = 1);

     //! Destructor.
    virtual ~LocationBox() {};
    
    // Set Methods

    void setSize(guint32 size) {d_size = size;}


    // Get Methods

    //! Return the size of the location.
    guint32 getSize() const {return d_size;}

    //! Returns a rectangle that describes the location.
    LwRectangle getArea() const
	{ return LwRectangle(getPos().x, getPos().y, d_size, d_size); }


    // Methods that operate on the class data and do not modify the class.
   
    //! Add an army to a tile that is included in this location.
    /**
     * @param army    The army instance to add to a tile in the location.
     *
     * @return A pointer to the stack where the Army was added.  Returns NULL
     *         when the Army couldn't be added because the location is full.
     */
    Stack *addArmy(Army *army) const;

    //! Returns whether this location is at least partially viewable.
    /**
     * This method returns true if the location has parts that are completely
     * defogged.
     * If the location is completely fogged or partially fogged then this 
     * method returns false.
     *
     * @param player The player whose map to query.
     */
    bool isVisible(Player *player) const;

    //! Returns whether or not the Location contains the given point?
    bool contains(Vector<int> pos) const;

    //! Unobscures the view of this location in the active player's FogMap.
    void deFog() const;

    //! Unobscures the view of this location in the given player's FogMap.
    void deFog(Player *p) const;

    //! Which tile of the location is the fewest number of tiles away from pos.
    Vector<int> getNearestPos(Vector<int> pos) const;

    Vector<int> getNearestPos(Movable *m) const;

    static Vector<int> getTopLeftBoundingBox(Vector<int> p1, Vector<int> p2);
 protected:

    //! Obtains a stack in the location to put an Army unit in.
    /**
     * This method scans the tiles of the location for a place to put a new
     * Army unit.  If a stack containing fewer than eight Army units is found, 
     * that stack is returned.  If there is an open spot in the location where
     * no Stack exists already, then the TILE parameter is filled up with that
     * location.  If no open spots could be found at all, and no stacks with 
     * fewer than eight army units could be found, NULL is returned
     *
     * @param owner  The player to own the new stack if one needs to be created.
     * @param tile   This position on the map is filled up if no stacks with
     *               enough space for one more army unit could be found in the
     *               location.
     *
     * @return The stack that has room for one Army unit in the Location.  If
     *         an available stack could not be found, NULL is returned.
     */
    Stack* getFreeStack(Player *owner, Vector<int> &tile) const;

    //! Check the location to see if a player can fit another army unit here.
    bool isFull(Player *owner) const;

    //DATA

    //! The size of the location.
    /**
     * This size is the number tiles high and wide the location is.
     * This value is always 1, except for City objects which are always 2.
     */
    guint32 d_size;
};

#endif
