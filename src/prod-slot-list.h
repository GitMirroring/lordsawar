// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2002 Mark L. Amidon
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009, 2014, 2015 Ben Asselstine
// Copyright (C) 2008 Ole Laursen
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
#ifndef PRODSLOTLIST_H
#define PRODSLOTLIST_H

#include <gtkmm.h>
#include <list>
#include <vector>
#include "prodslot.h"
#include "defs.h"

class ArmyProdBase;
class ArmyProto;
class Army;

//! A set of production slots.
/**
 * This object is the set of production slots that may or may not hold
 * army production base objects.
 * This object remembers which slot is "active", and how far along the unit
 * is to being completed.
 */
class ProdSlotlist: public std::vector<ProdSlot*>
{
    public:

	//! Default constructor.
        /** 
          * Make a new set of production slots.
	  *
	  * @param numslots  The number of production slots in the list.
          */
        ProdSlotlist(guint32 numslots = MAX_PRODUCTION_SLOTS_IN_A_CITY);

	//! Copy constructor.
        ProdSlotlist(const ProdSlotlist&);

        //! Loading constructor.
	/**
	 * Make a new set of production slots by reading it from a saved-game 
	 * file.
	 *
	 * @param helper The opened saved-game file to load the set of 
	 *               production slots from.
	 */
        ProdSlotlist(XML_Helper* helper);

	//! Destructor.
        ~ProdSlotlist();

        
	// Set Methods

	//! Set the active production base of the list.
        /**
	 * Make the Army production base in particular slot active, so that
	 * the Army starts being produced.
	 *
         * @param index  The index of the production slot to activate. 
	 *               -1 means no production at all.   This must be a value
	 *               between -1 and 3.
         */
        void setActiveProductionSlot(int index);


	// Get Methods


	// Methods that operate on the class data but do not modify the class.

        //! Save the set of production slots to an opened saved-game file.
        bool save(XML_Helper* helper) const;
        
        //! Returns whether or not the given army type is in the list.
	/**
	 * This method scans the production slots for the given army 
	 * prototype.
	 *
	 * @param type      The index of the Army prototype in the Armyset.
	 * @return True if the given army prototype is already a production
	 *         base in the list.  Otherwise false.
	 */
        bool hasProductionBase(int type) const;

        //! Returns the maximum number of production bases in the list.
	/**
	 * The list has this many production slots in total.  This value 
	 * should always return 4 (defs.h:MAX_PRODUCTION_SLOTS_IN_A_CITY)
	 *
	 * @return The maximum number of Army production bases that this list
	 *         can have.
	 */
        guint32 getMaxNoOfProductionBases() const {return size();};


        //! Returns true if the list already contains this production type.
        bool hasProductionBase(const ArmyProto * army) const;

        //! Return the first slot that doesn't have a production base.
	int getFreeSlot() const;

        //! Return the number of production bases in the list.
	/**
	 * Scan the production slots and count how many are filled with an
	 * Army production base.
	 *
	 * @return The current number of used slots that the list has.
	 */
        guint32 getNoOfProductionBases() const;

        //! Get the number of turns until current production base is finished.
        int getDuration() const {return d_duration;}
        
        //! Return the index of the active production slot.
	/**
	 * @return The index of the active production slot, or -1 if the list 
	 *         does not have an active production slot.
	 */
        int getActiveProductionSlot() const {return d_active_production_slot;}
        
        //! Return the index of the army in the given slot.
	/**
	 * @param slot  The production slot to return the army type for.  This
	 *              value ranges between 0 and 3.
	 *
	 * @return The index of the Army prototype unit within it's Armyset,
	 *         or -1 if no production base is allocated to that slot.
	 */
        int getArmytype(int slot) const;

        //! Return the army production base of the given slot.
        const ArmyProdBase * getProductionBase(int slot) const;

	//! Return the army production base this list is producing.
	const ArmyProdBase *getActiveProductionBase() const;

	//! Scan the list for an army production base of the given type.
	const ArmyProdBase * getProductionBaseBelongingTo(const Army *army) const;
        

	// Methods that operate on the class data and modify the class.

	//! Add an Army production base to a production slot.
        /**
	 * This method is called when a new army production base has been
	 * purchased/bought.
	 *
         * @note This method overwrites the production slot if neccessary.
         * 
         * @param index        The index of the production slot; if set to -1,
         *                     the object will try to find a free production 
	 *                     slot. This must be a value between -1 and 3.
	 * @param army         The Army production base to add.  Look at the
	 *                     Army class to find out what a production base is.
         */
        void addProductionBase(int index, ArmyProdBase *army);

        //! Clears the basic production of a given slot.
	/**
	 * @param index  The slot to remove the Army production base from.
	 *               This method deletes the Army production base object.  
	 *               This parameter must be a a value between 0 and 3.
	 */
        void removeProductionBase(int index);

        //! axe any army types that are no longer in the armyset.
        bool removeArmyProdBasesWithoutAType(guint32 armyset);

    protected:

	//! Callback method to help in loading the armyprodbases into the list.
	bool load(Glib::ustring tag, XML_Helper *helper);

        // DATA

	//! The active production slot.
	/**
	 * The Army production base in this slot is the Army unit that the
	 * list is currently busy creating.
	 */
        int d_active_production_slot;

	//! Number of turns until the next Army is produced.
	/**
	 *  Number of turns required to finish the current production.
	 *  When this value hits 0, the new Army unit is created.
	 */
        int d_duration;

};

#endif // PRODSLOTLIST_H
