//  Copyright (C) 2008, 2010, 2014, 2021 Ben Asselstine
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
#ifndef BACKPACK_H
#define BACKPACK_H

#include <list>
#include <gtkmm.h>

class XML_Helper;
class Item;
class Player;

//! An object that carries items.
/**
 * Heroes have backpack objects that carry items.
 */
class Backpack: public std::list<Item*>
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

        //! Standard constructor: create a backpack.
	/**
	 */
        Backpack();

	//! Loading constructor.
	/**
	 * Load the backpack from a file.
	 * Backpacks are stored in the saved-game file at:
	 * lordsawar.playerlist.player.stacklist.stack.hero.backpack.
	 *
	 * @param helper  The opened saved-game file to load the backpack from.
	 */
        Backpack(XML_Helper* helper);

	//! Copy constructor.
	Backpack(const Backpack&);

	//! Destructor.
        ~Backpack();

	//! Save a backpack.
        /**
         * @param helper  The opened saved-game file to save the backpack to.
	 *
         * @return True if saving went well, false otherwise.
         */
        bool save(XML_Helper* helper) const;

	//! Save the contents of a backpack.
        /**
         * @param helper  The opened saved-game file to save the contents of
	 *                the backpack to.
	 *
         * @return True if saving went well, false otherwise.
         */
        bool saveData(XML_Helper* helper) const;

        //! Remove an Item from the backpack of the hero.
        /**
	 * Scan the hero's d_backpack for the Item, and remove it if it is
	 * found.
	 *
	 * @note This method removes the Item from the d_backpack, but does
	 *       not destroy the Item.
	 *
	 * @param item   The Item to look for.
	 *
	 * @return True if the Item was found and removed.
	 */
	bool removeFromBackpack(Item* item);

	//! Remove all items from the backpack.
	void removeAllFromBackpack();

	//! Add all of the items from the given backpack.
	void add(Backpack *backpack);

	//! Add an Item to the bottom of the hero's backpack. 
	bool addToBackpack(Item* item);

        //! Add an Item to the backpack of the Hero.
	/**
	 * @param item      The Item to add to the d_backpack.
	 * @param position  How deep into the backpack the Item is stored.
	 *                  Subsequent Items get pushed down to make room.
	 *                  This value starts at 0.
	 *
	 * This method is usually used to add an Item to the top of the
	 * hero's backpack (e.g. position == 0).
	 *
	 * @return Always returns true.
	 */
	bool addToBackpack(Item* item, int position);

        //! Use an item in the backpack.  removes it if it's spent.
        bool useItem(Item *item);

	//! Tally up the strength bonuses inferred by items in the backpack.
	guint32 countStrengthBonuses();

	//! Tally up the gold bonuses inferred by the items in the backpack.
	guint32 countGoldBonuses();

	//! Tally the stack strength bonuses inferred by items in the backpack.
	guint32 countStackStrengthBonuses();

	//! Count the number of items that double movement in the backpack.
	guint32 countMovementDoublers();

	//! Return the first plantable item that can be planted by player.
	Item *getPlantableItem(Player *player);

	//! Tally the plantable items in the backpack.
	guint32 countPlantableItems();

        //! Check if the backpack has any usable items.
        bool hasUsableItem() const;

        //! Get a list of all of the usable items in the backpack.
        void getUsableItems(std::list<Item*> &items) const;

	//! Return the item with the given id.
	Item *getItemById(guint32 id);

	//! Tally the items that let stacks fly.
	guint32 countStackFlightGivers();
    protected:
	bool loadItem(Glib::ustring tag, XML_Helper* helper);
    private:

};

#endif

// End of file
