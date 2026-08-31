// Copyright (C) 2000, 2001 Michael Bartl
// Copyright (C) 2001, 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2004 John Farrell
// Copyright (C) 2007, 2008, 2009, 2014, 2020, 2021 Ben Asselstine
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
#ifndef RUINLIST_H
#define RUINLIST_H

#include "ruin.h"
#include "LocationList.h"
#include <sigc++/trackable.h>

class XML_Helper;

//! A list of Ruin objects on the game map.
/** 
 * The ruinlist keeps track of the Ruin objects located on the game map. It
 * is implemented as a singleton because many classes use it for looking 
 * up ruins.
 */
class Ruinlist : public LocationList<Ruin*>, public sigc::trackable
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

        //! Makes a copy of the ruinlist.
        Ruinlist* copy () {return new Ruinlist (*this, true);}

	// Methods that operate on class data and modify the class.

	//! Change ownership of all Ruin objects in the list.
	/**
	 * Changes all ruins owned by old owner, to be owned by the new owner.
	 */
	void changeOwnership(Player *old_owner, Player *new_owner);

        
	// Methods that operate on class data and do not modify the class.

	//! Save the list of Ruin objects to the opened saved-game file.
        bool save(XML_Helper* helper) const;

        //! Find the nearest Ruin object that has not been searched.
	/**
	 * Scan through all of the Ruin objects searching for the closest one
	 * that has not already had a Hero successfully search it.
	 *
	 * @note This method does not return hidden ruins that do not belong
	 *       to the active player.
	 *
	 * @param pos  The position on the game map to search for the nearest
	 *             unsearched Ruin object from.
	 *
	 * @return A pointer to the nearest Ruin object that has not been 
	 *         successfully searched already.  Returns NULL when all Ruin 
	 *         objects have been searched.
	 */
        Ruin* getNearestUnsearchedRuin(const Vector<int>& pos) const;
        Ruin* getNearestUnsearchedRuin(const Vector<int>& pos, int dist) const;

        //! Find the nearest ruin.
	/**
	 * Scan through all of the Ruin objects searching for the closest one.
	 *
	 * @note This method does not return hidden ruins that do not belong
	 *       to the active player.
	 *
	 * @param pos  The position on the game map to search for the nearest
	 *             Ruin object from.
	 *
	 * @return A pointer to the nearest Ruin object.  Returns NULL when 
	 *         there are no Ruin object in this list.
	 */
        Ruin* getNearestRuin(const Vector<int>& pos) const;

        //! Find the nearest ruin that is not too far away.
	/**
	 * Scan through all of the Ruin objects searching for the closest one
	 * that is no far than the given distance.
	 *
	 * @note This method does not return hidden ruins that do not belong
	 *       to the active player.
	 *
	 * @param pos  The position on the game map to search for the nearest
	 *             Ruin object from.
	 * @param dist The number of tiles away that is deemed "too far".
	 *
	 * @return A pointer to the nearest Ruin object that isn't too far 
	 *         away.  If all of the Ruin objects in the list are too far 
	 *         away, this method returns NULL.
	 */
        Ruin* getNearestRuin(const Vector<int>& pos, int dist) const;

        //! Find the nearest Ruin object that is not obscured by fog.
	/**
	 * Scan through all ruins, searching for the closest one that is
	 * not covered by fog-of-war on a hidden map.
	 *
	 * @note This method does not return hidden ruins that do not belong
	 *       to the active player.
	 *
	 * @param pos  The position to find the nearest ruin from.
	 *
	 * @return A pointer to the nearest ruin that is not obscured by fog.
	 */
        Ruin* getNearestVisibleRuin(const Vector<int>& pos) const;

	//! Find the nearest ruin that is unobscured and is not too far away.
	/**
	 * Scan through all the ruins, searching for the closest one that
	 * is not covered by fog-of-war on a hidden map, but is not farther
	 * away than a given distance.
	 *
	 * @note This method does not return hidden ruins that do not belong
	 *       to the active player.
	 *
	 * @param pos  The position to find the nearest ruin from.
	 * @param dist The number of tiles away that is deemed "too far".
	 *
	 * @return A pointer to the nearest ruin that is not obscured by fog 
	 *         and is within the prescribed number of tiles.  Returns NULL 
	 *         if no ruin could be found.
	 */
        Ruin* getNearestVisibleRuin(const Vector<int>& pos, int dist) const;

        guint32 countUnexploredRuins(Player *owner) const;
        guint32 countExploredRuins(Player *owner) const;

        //! Count the number of ruins that have the default name.
        guint32 countUnamedRuins () const;

        //! Count the ruins that have a defender.
        guint32 countKeepers () const;

        //! Count the ruins that don't have a defender.
        guint32 countEmptyKeepers () const;

	// Static Methods
        
	//! Returns the singleton instance.  Creates a new one if required.
        static Ruinlist* getInstance();

        //! Loads the singleton instance from the opened saved-game file.
        static Ruinlist* getInstance(XML_Helper* helper);

        //! Explicitly deletes the singleton instance.
        static void deleteInstance();
        
        //! Replace the current ruinlist with another.
        static void reset (Ruinlist *r);

	//! Destructor.
        ~Ruinlist();
    protected:

	//! Default constructor.
        Ruinlist();

	//! Loading constructor.
	/**
	 * Make a new list of Road objects by loading it from an opened 
	 * saved-game file.
	 *
	 * @param helper  The opened saved-game file to load the Ruin objects
	 *                from.
	 */
        Ruinlist(XML_Helper* helper);

        //! Copy constructor
        Ruinlist (const Ruinlist &r, bool sync_ids = false);

    private:
        //! Loading callback for loading Ruin objects into the list.
        bool load(Glib::ustring tag, XML_Helper* helper);

	// DATA

        //! A static pointer for the singleton instance.
        static Ruinlist* s_instance;
};

#endif
