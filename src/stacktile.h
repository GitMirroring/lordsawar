// Copyright (C) 2009, 2014, 2015, 2017, 2021 Ben Asselstine
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
#ifndef STACKTILE_H
#define STACKTILE_H
#include <list>
#include <gtkmm.h>
#include "vector.h"
class Stack;
class Player;
//! A StackTile helper class for a single stack on a tile.
struct StackTileRecord
{
  guint32 stack_id;
  guint32 player_id;
};

//! Manages a set of Stack objects that share a Maptile on the map.
/**
 * A stacktile object is a temporary object that holds a number of non-empty 
 * stack objects belonging to the same player where their total number of 
 * armies doesn't exceed 8.  The actual value is related to the 
 * MAX_ARMIES_ON_A_SINGLE_TILE constant.
 *
 * This means that there can be up to 8 army units belong to the green player,
 * and another 8 army units belonging to the red player.  Stacks of differing 
 * sides only share a tile when fighting.
 *
 * The stacktile object is not saved to disk, instead it is reconstituted 
 * based on the loading of stacklists.
 *
 * Like map backpack objects that exist on every tile, whether they have any 
 * items in them or not, a stacktile object exists on every tile whether any 
 * stacks are present or not.  the parent object in both cases is the maptile 
 * object.
 *
 * A stack doesn't know what stacktile it's in but it knows Where it is on the 
 * map.  game map has a quick lookup of position to stacktile.
 */
class StackTile: public std::list<StackTileRecord>
{
public:
    //! Constructor.
    StackTile(Vector<int> pos);

    //! Copy constructor.
    StackTile (const StackTile &s);

    //! Destructor.
    ~StackTile() {};

    // Methods that operate on the class data and modify the class.

    //! Check to see if the given stack can be added to this tile.
    bool canAdd(const Stack *stack);

    //! Check to see if a stack with the given size and owner can be added here.
    bool canAdd(guint32 siz, Player *owner);

    //! Remove the given stack from this stacktile.
    bool leaving(Stack *stack);

    //! Add the given stack to this stacktile.
    void arriving(Stack *stack);

    //! Add the given stack to this stacktile.
    void add(Stack *stack);

    //! Remove the given stack from this stacktile.
    bool remove(Stack* stack);

    //! Set all stacks on this tile to be defending.
    void setDefending(Player *owner, bool defending);

    //! Set all stacks on this tile to be parked.
    void setParked(Player *owner, bool parked);

    //! Merge all stacks on this tile (for the editor)
    void group ();

    //! Merge all stacks on this tile belonging to the given player.
    Stack *group(Player *owner);

    //! Merge all stacks on this tile belonging to the given player into S.
    void group(Player *owner, Stack *s);

    //! Split all army units belonging to the given player into stacks.
    void ungroup(Player *owner);


    // Methods that operate on the class data and do not modify the class.

    //! Return the first stack on this tile belonging to the given player.
    Stack *getFriendlyStack(Player *owner) const;

    //! Return all of the stacks on this tile belonging to the given player.
    std::vector<Stack *> getFriendlyStacks(Player *owner) const;

    //! Return all stacks on this tile.
    std::vector<Stack *> getStacks() const;

    //! Return the first stack on this tile not belonging to the given player.
    Stack *getEnemyStack(Player *notowner) const;

    //! Return all of the stacks on this tile not belonging to the given player.
    std::vector<Stack *> getEnemyStacks(Player *owner) const;

    //! Return the first stack on this tile.
    Stack *getStack() const;

    void dump () const;

    //! Return true if this tile contains the given stack id.
    bool contains(guint32 stack_id) const;

    Vector<int> getTile() const {return tile;};

    //! Return the number of army units on this tile owned by the given player.
    guint32 countNumberOfArmies(Player *owner) const;
private:

    Stack *groupStacks(Player *owner, Stack *s);
    //! Return the position of the given stack in our list of records.
    StackTile::const_iterator findStack(const Stack *s) const;
    StackTile::iterator findStack(Stack *s);

    // DATA

    // Where on the game map this stack tile is.
    Vector<int> tile;
};
#endif
