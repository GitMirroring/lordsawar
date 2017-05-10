// Copyright (C) 2004 John Farrell
// Copyright (C) 2004, 2005 Ulf Lorenz
// Copyright (C) 2004, 2006 Andrea Paternesi
// Copyright (C) 2007, 2009, 2014 Ben Asselstine
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
#ifndef THREATLIST_H
#define THREATLIST_H

#include <list>
#include "Threat.h"

class Stack;
class Ruin;
class AICityInfo;

//! Artificial intelligence for representing a list of threats to a Player.
/** List of threats.
  */
class Threatlist : public std::list<Threat*>
{
    public:

	//! Default Constructor.
        Threatlist();

	//! Destructor.
        ~Threatlist();

	// Methods that operate on class data and modify the class.

        //! Add a ruin as a threat
        void addRuin(Ruin *ruin);
        
        //! Adds a stack as a threat. 
	/**
	 * If other threats posed by the owner of the stack are close by, they 
	 * are merged to a single threat.
	 */
        void addStack(Stack *stack);

        //! Searches through the threat list and deletes the stack
        void deleteStack(Stack* s);

	//! deletes the stack in the threat list that has the given id.
	void deleteStack(guint32 id);

        // how much danger does this set of threats pose to the given city?
        void findThreats(AICityInfo *info) const;

        //! sort into a list of most dangerous first
        void sortByValue();

        //! sort into list by closest first
        void sortByDistance(Vector<int> pos);

        //! sort into a list with value divded by distance.
        void sortByDistanceAndValue(Vector<int> pos);

        //! Behaves like std::list::erase(), but frees pointers as well
        iterator flErase(iterator object);

        //! Behaves like std::list::remove(), but frees pointers as well
        bool flRemove(Threat* object);

	// Methods that operate on class data but do not modify the class

        //! return some debugging information
        Glib::ustring toString() const;
        
        void changeOwnership(Player *old_owner, Player *new_owner);

    private:

        //! Behaves like std::list::clear(), but frees pointers as well
        void flClear();

        static bool compareValue(const Threat *lhs, const Threat *rhs);
};

#endif // THREATLIST_H

// End of file
