// Copyright (C) 2004 John Farrell
// Copyright (C) 2004 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2007, 2008, 2009, 2010, 2014, 2020, 2021 Ben Asselstine
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
#ifndef THREAT_H
#define THREAT_H

#include <gtkmm.h>
#include "vector.h"
#include "OwnerId.h"

class City;
class StackReflist;
class Stack;
class Player;
class Ruin;

//! Artificial intelligence for representing a threat to a Player.
/** 
  * The smart AI player tries to assess all enemy stacks and cities and all
  * ruins and store the results in threats. A threat has three characteristic
  * figures:
  *
  * - a strength which determines how strong the stack (or the stacks defending
  *   the city) are
  * - a danger which determines how close the enemy object is (if an enemy stack
  *   endangers two cities at once, the danger value doubles). The danger depends
  *   on the distance to the AI's cities as well as the strength of the stack.
  * - a value, which gives an assessment of how valuable it is to destroy the
  *   threat. As an example, taking over a dangerous enemy city is more valuable
  *   than destroying a stack because the AI gets an additional city. The value
  *   is the sum of the danger of the threat and some additional bonus.
  *
  * Furthermore, the threat class has some additional functions. They are e.g.
  * neccessary because the AI bundles several stacks which are close together
  * or a stack which is in an enemy city to one single threat.
  *
  * For more information about the smart AI, see ai_smart.h
  */

class Threat: public OwnerId
{
    public:

        //! Copy Constructor.
        Threat (const Threat &t);

        //! Constructor.  Our threat is an enemy city.
        Threat(City *c);

        //! Constructor.  The threat is an enemy stack.
        Threat(Stack *s);

        //! Constructor.  The "threat" is a ruin (the danger value is 0)
        Threat(Ruin *r);

	//! Destructor.
        ~Threat();

	// Methods that operate on class data and modify the class.

        //! Add a stack to this threat.
        void addStack(Stack *stack);

        //! Removes the stack s from the threat.
        void deleteStack(Stack* s);

	//! Removes the stack with the given id from the threat.
        void deleteStack(guint32 id);

        //! Increase the danger of this threat 
        void addDanger(float danger);

	// Methods that operate on class data and do not modify the class.

        /** Checks if a threat is close to a certain position and "belongs"
          * (i.e. is caused by) a certain player.
          *
          * The background is that threats close to each other are merged into
          * a single threat.
          *
          * @param pos  the position the threat has to be close to
          * @param p    the player who has to cause the threat
          * @return true if both the position and the player satisfy the
          * conditions, else return false
          */
        bool Near(Vector<int> pos, Player *p) const;

        //! How strong is this threat?
        float getStrength() const {return d_strength;}

        /** Returns the closest point of a threat to a certain location
          * (remember that a threat can consist of several single threats
          * or a city which covers more than one tile)
          *
          * If there are no dangers left, it returns the point (-1, -1).
          */
        Vector<int> getClosestPoint(Vector<int> location) const;


        //! return the danger posed by this threat to the current player
        float getDanger() const { return d_danger; }

        float getValue() const {return d_value;}

        //! Does this threat contain a city?
        bool isCity() const { return d_city != 0; }

        //! Is this threat a ruin?
        bool isRuin() const { return d_ruin != 0; }

        //! Can be used for some general debug output
        Glib::ustring toString() const;

        void changeOwnership(Player *old_owner, Player *new_owner);
    private:

        void calculateValue();
        void calculateStrength();
	// DATA

	//! The city associated with this threat.
        City *d_city;

	//! The ruin associated with this threat.
        Ruin *d_ruin;

	//! The list of stacks associated with this threat.
        StackReflist *d_stacks;

	//! The amount of danger this threat represents.
        float d_danger;

        //! Danger augmented by some other factors.
        float d_value;

        float d_strength;
};

#endif // THREAT_H

// End of file
