// Copyright (C) 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004 Andrea Paternesi
// Copyright (C) 2007, 2008, 2014, 2021 Ben Asselstine
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
#ifndef ITEM_H
#define ITEM_H

#include <gtkmm.h>

#include "UniquelyIdentified.h"
#include "ItemProto.h"

class XML_Helper;
class Army;
class Player;

//! A carryable thing that confers special properties on it's holder.
/** 
 * This class describes an item.  Items are carried by heroes in a backpack.
 * When Items are carried they give special abilities to that hero, and 
 * perhaps the stack it is included in.
 * Items can be dropped onto the ground, and picked up from the ground.
 * When a hero dies, all of that hero's items get dropped onto the ground.
 *
 * There are "plantable items", that a hero can stick into the ground.  This
 * gives the ability to vector Army units to that location.  Every player
 * gets a plantable item, and the item is branded to be usable only by the
 * player that it belongs to.
 * 
 */


class Item: public ItemProto, public UniquelyIdentified
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

	//! Loading constructor.
        Item(XML_Helper* helper);

	//! Copy constructor.
        Item(const Item& orig, bool sync_id = false);

	//! Copy constructor.  make an item from a prototype.
	Item(const ItemProto &proto, guint32 type_id);

        //! Make an item from scratch.
        Item(Glib::ustring name, bool plantable, Player *plantable_owner);

        //! Destructor.
        ~Item();
        
	//! Emitted when an item is destroyed.
        sigc::signal<void, Item*> sdying;

        //! Save the item to the opened saved-game file.
        bool save(XML_Helper* helper) const;

	//! Return whether or not the Item is of a kind that can be vectored to.
        bool isPlantable() const {return d_plantable;}

	//! Set the planted status of the Item.
        void setPlanted(bool planted) {d_planted = planted;}

        //! Decrement uses left, and pass true when empty.
        bool use();

	//! Get the planted status of the Item.
        bool getPlanted() const {return d_planted;}

	//! Return the Player who can plant this particular Item.
	Player *getPlantableOwner() const; 

        //! Set which player has planted this.
        void setPlantableOwnerId (guint32 id) {d_plantable_owner_id = id;}

	//! Return the Player who started out with this item.
	Player *getPlantableOriginalOwner() const; 

        //! Set which player has originally owned this. (for the color)
        void setPlantableOriginalOwnerId (guint32 id) {d_plantable_orig_owner_id = id;}

	//! Return the type of this item.
	guint32 getType() const {return d_type;};

    private:

	//! non-default constructor to make an item with a particular id.
	Item(Glib::ustring name, bool plantable, Player *plantable_owner,
	     guint32 id);

	/**
	 * This value indicates if the type of this Item can potentially be
	 * planted into the ground on the GameMap, and subsequently have 
	 * Army units vectored to that position.
	 */
	bool d_plantable;

	/**
	 * If the Item is plantable, this value is used to determine if the
	 * correct Player is attempting to plant the Item into the ground.
	 * For example, the red player cannot plant the flag belonging to the
	 * yellow player.
	 */
	//! The Id of the Player who can plant this Item.
	guint32 d_plantable_owner_id;

	/**
	 * When the Item is planted, the player can vector Army units to 
	 * this item's position on the GameMap.
	 */
	//! Whether or not this Item is currently planted.
	bool d_planted;

	/**
	 * If the Item is plantable, this value is used to determine if the
	 * color of the flag in the ground.
	 */
	//! The Id of the Player who originally owned this item.
	guint32 d_plantable_orig_owner_id;

	//! The item was instantiated from the item prototype that has this id.
	guint32 d_type;
};

#endif //ITEM_H
