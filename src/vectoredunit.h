//  Copyright (C) 2007, 2008, 2009, 2014, 2021 Ben Asselstine
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
#ifndef VECTOREDUNIT_H
#define VECTOREDUNIT_H

#include <sigc++/trackable.h>
#include "armyprodbase.h"
#include "OwnerId.h"
#include "LocationBox.h"

//! An Army that is being vectored to another city.
/**
 *  When Army objects are "vectored" to another city, they disappear for two 
 *  turns.   While an Army is "in the air", it is represented in one of these 
 *  objects.
 */
class VectoredUnit: public OwnerId, public LocationBox, public sigc::trackable
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

	//! Default constructor.
        /** 
	 * Make a new vectored unit.
	 *
         * @param pos         The position of the source of the vectored unit.
         * @param dest        The destination location for the unit.
	 * @param army	      The Army prototype that is being vectored.
         * @param duration    How many turns it takes for the armytype to
	 *                    show up at dest.
	 * @param player      The player that owns the vectored Army unit.
         */
        VectoredUnit(Vector<int> pos, Vector<int> dest, ArmyProdBase *army, 
		     int duration, Player *player);

        //! Copy constructor.
	/**
	 * Make a new vectored unit by copying it from another one.
	 */
        VectoredUnit(const VectoredUnit&);

        //! Loading constructor.
	/**
	 * Make a new vectored unit by loading from an opened saved-game file.
	 * This method loads the lordsawar.vectoredunitlist.vectoredunit XML
	 * entities in the saved-game file.
	 *
	 * @param helper  The opened-saved game file to load the vectored unit
	 *                from.
	 */
        VectoredUnit(XML_Helper* helper);

	//! Destructor.
        ~VectoredUnit();

	// Get Methods

	//! Return the position of the destination for this vectored unit.
	/**
	 * @return The position of a tile on the game map where the vectored
	 *         unit will show up (eventually).
	 */
	Vector<int>getDestination() const {return d_destination;};

	//! Return how long it will take for the vectored unit to arrive.
	/**
	 * Returns the number of turns that it takes for this vectored unit
	 * to show up at the destination position on the game map.
	 */
	int getDuration () const { return d_duration; };

	//! Return a pointer to the Army prototype that is being vectored.
	/**
	 * @return A pointer to an Army in an Armyset.
	 */
	ArmyProdBase *getArmy() const { return d_army; };


	// Set Methods

	//! Set the position of the destination target for this vectored unit.
	/**
	 * @param dest  The position of a tile on the game map to have the
	 *              vectored unit show up at.
	 */
	void setDestination(Vector<int>dest) {d_destination = dest;};

	//! Sets how long it will take for the vectored unit to arrive.
	/**
	 * @param duration  The number of turns to take before showing up at
	 *                  the destination posititon on the game map.
	 */
	void setDuration(int duration) {d_duration = duration;};

	//! Set the Army prototype that is being vectored.
	void setArmy(ArmyProdBase *army) {d_army = army;}


	// Methods that operate on class data but do not modify the class

        //! Saves the vectored unit data to an opened saved-game file.
        bool save(XML_Helper* helper) const;

	//! Called when a vectored unit arrives at the destination.
	Army *armyArrives(Stack *&stack) const;


	// Methods that operate on class data and modify the class.

        //! Process the vectored unit at the start of a new turn.
	/**
	 * @return True when this vectored unit has shown up at the destination
	 *         position on the game map.  Otherwise false.
	 */
        bool nextTurn();

        //Statics

        //! Calculate the turns it takes to vector from SRC to DEST.
        static int get_travel_turns (Vector<int> src, Vector<int> dest);

    private:

        // DATA

	//!  The position on the game map that this vectored unit is going to.
	Vector<int> d_destination;

	//! A pointer to the Army prototype to vector.
	ArmyProdBase *d_army;

	//! The number of turns remaining until the Army shows up.
	int d_duration;
};

#endif // VECTOREDUNIT_H

