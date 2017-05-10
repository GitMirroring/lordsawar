// Copyright (C) 2009, 2010, 2014, 2015 Ben Asselstine
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
#ifndef STACKREFLIST_H
#define STACKREFLIST_H

#include <list>
#include <gtkmm.h>
class Stack;
class Player;
class Stacklist;

//! lightweight list of stacks.
class StackReflist: public std::list<Stack*>
{
public:
    //! Default Constructor.
    StackReflist();

    //! Alternate constructor.  Create the list from a player's stacklist.
    StackReflist(Stacklist *, bool skip_parked_stacks = false);

    //! Alternate constructor.
    StackReflist(std::list<Stack*> stacks, bool skip_parked_stacks = false);

    //! Destructor.
    ~StackReflist() {};

    void addStack(Stack *s);

    //! Return true if the stack with the given id was deleted from the list.
    bool removeStack(guint32 stack_id);

    //! Return true if this list contains the given stack id.
    bool contains(guint32 stack_id) const;

    guint32 countArmies() const;

    StackReflist::iterator eraseStack(StackReflist::iterator it);
    StackReflist::iterator eraseStack(StackReflist::iterator it, guint32 id);

    void changeOwnership(Player *new_owner);

    bool getIdOfStack(Stack *stack, guint32 &id);
    Stack *getStackById(guint32 id) const;
private:


    typedef std::map<guint32, Stack*> IdMap;
    //! A map to quickly lookup the stack by it's unique id.
    IdMap d_id;

};
#endif
