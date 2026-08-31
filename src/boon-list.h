//  Copyright (C) 2026 Ben Asselstine
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
#ifndef BOONLIST_H
#define BOONLIST_H

#include <list>
#include "boon.h"


class City;
class Ruin;
class Temple;
class MapBackpack;
class Quest;
class Stack;
//! Artificial intelligence for representing a list of boons for a Player.
/** List of Boon objects.
 * Things like, empty cities, temples, ruins, quest completions.
  */
class Boonlist : public std::list<Boon*>
{
    public:

	//! Default Constructor.
        Boonlist ();

	//! Destructor.
        ~Boonlist ();

	// Methods that operate on class data and modify the class.

        //! Add a city as a boon.  e.g. an empty one.
        void add (City *city);

        //! Add a ruin as a boon
        void add (Ruin *ruin);

        //! Add a temple as a boon
        void add (Temple *temple);

        //! Add a backpack as a boon
        void add (MapBackpack *backpack);

        //! Add a quest as a boon
        void add (Quest *quest);
        
        //! Behaves like std::list::erase (), but frees pointers as well
        iterator fl_erase (iterator object);

        //! Behaves like std::list::remove (), but frees pointers as well
        bool fl_remove (Boon* object);

        //! Calculate values relative to the position and properties of a stack
        void calculate (Stack *s);

        //! Return the best boons, but only one of each kind.
        std::list<Boon> get_best_boons (Stack *s);

        std::list<Boon> get_best_ruin_boons () const;
        std::list<Boon> get_best_temple_boons () const;
        std::list<Boon> get_best_city_boons () const;
        std::list<Boon> get_best_backpack_boons () const;
        std::list<Boon> get_best_quest_boons () const;

	// Methods that operate on class data but do not modify the class


        //! return some debugging information
        Glib::ustring to_string () const;
        
        Boon* getClosestBoon (Stack *s, std::vector<Vector<int>> points);
    private:

        //! Behaves like std::list::clear (), but frees pointers as well
        void fl_clear ();

        //! Return the first city boon in the list
        Boon *get_first_city_boon ();

        //! Return the first ruin boon in the list
        Boon *get_first_ruin_boon ();

        //! Return the first temple boon in the list
        Boon *get_first_temple_boon ();

        //! Return the first quest boon in the list
        Boon *get_first_quest_boon ();

        //! Return the first backpack in the list
        Boon *get_first_backpack_boon ();

        //! Sort into a list of most valuable first
        void sort_by_value ();

        static bool compare_value (const Boon *lhs, const Boon *rhs);

};

#endif
