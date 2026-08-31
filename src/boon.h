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
#ifndef BOON_H
#define BOON_H

#include <gtkmm.h>
#include "vector.h"
#include "location-box.h"

class City;
class Ruin;
class Temple;
class Quest;
class MapBackpack;
class Stack;

//! Artificial intelligence for representing a benefit to a Player
/** 
  * this is for things like, the hero's quest, a ruin, a temple,
  * a bag of items, and an empty city.
  * things that a player might race to while they can.
  *
  * boon, noun:
  *  something extremely useful, helpful, or beneficial; a blessing or benefit
  */

class Boon: public LocationBox
{
    public:

        //! Copy Constructor.
        Boon (const Boon &b);

        //! Constructor.  Our boon is an empty enemy city.
        Boon (City *c);

        //! Constructor.  The boon is a temple.
        Boon (Temple *t);

        //! Constructor.  The boon is a ruin.
        Boon (Ruin *r);

        //! Constructor.  The boon is a bag of stuff.
        Boon (MapBackpack *b);

        //! Constructor.  The boon is a quest.
        Boon (Quest *q);

	//! Destructor.
        ~Boon ();

	// Methods that operate on class data and modify the class.

        //! Set the boon's value according to the given stack.
        void calculate (Stack *s);

	// Methods that operate on class data and do not modify the class.

        //! How good is this boon?
        float get_value () const
          {
            return m_value;
          }

        //! How far away is this boon in time
        float get_turns_away () const
          {
            return m_turn_factor;
          }

        bool is_backpack () const
          {
            return m_backpack != NULL;
          }

        bool is_quest () const
          {
            return m_quest != NULL;
          }

        bool is_city () const
          {
            return m_city != NULL;
          }

        bool is_ruin () const
          {
            return m_ruin != NULL;
          }

        bool is_temple () const
          {
            return m_temple != NULL;
          }

        /** Returns the closest point of a boon to a certain location
         * A city occupies 2x2, this gets the closest one in terms of tiles
         * (not the closest in terms of shortest path)
          */
        Vector<int> get_destination (Stack *s) const;

        //! Can be used for some general debug output
        Glib::ustring to_string () const;

    private:

	// DATA

	//! The city associated with this boon.
        City *m_city;

	//! The ruin associated with this boon.
        Ruin *m_ruin;

	//! The temple associated with this boon.
        Temple *m_temple;

	//! The backpack associated with this boon.
        MapBackpack *m_backpack;

        //! The quest associated with this boon.
        Quest *m_quest;

        //! The value of the boon.
        float m_value;

        //! The boon expires in a certain number of turns, 0 means no expiry.
        /**
         * for example if it takes more than 1 turn to get to an empty city, we
         * presume it won't be free for the taking.
         */
        guint32 m_expiry;

        //! How many turns it takes to get there
        float m_turn_factor;

        //! Figure out how valuable a boon is to a particular stack
        float calculate_value (Stack *stack, float &turn_factor) const;

        //! Figure out how valuable a city boon is to a particular stack
        float calculate_city_value (Stack *s) const;

        //! Figure out how valuable a ruin boon is to a particular stack
        float calculate_ruin_value (Stack *s) const;

        //! Figure out how valuable a temple boon is to a particular stack
        float calculate_temple_value (Stack *s) const;

        //! Figure out how valuable a backpack  boon is to a particular stack
        float calculate_backpack_value (Stack *s) const;

        //! Figure out how valuable a quest  boon is to a particular stack
        float calculate_quest_value (Stack *s) const;
};

#endif
