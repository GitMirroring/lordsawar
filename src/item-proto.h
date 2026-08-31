// Copyright (C) 2008, 2009, 2010, 2011, 2014, 2015, 2020 Ben Asselstine
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
#ifndef ITEM_PROTO_H
#define ITEM_PROTO_H

#include <gtkmm.h>

#include "Renamable.h"
class XML_Helper;

//! A carryable type of thing that confers special properties on it's holder.
/**
 * This class describes an item prototype.  Items are carried by heroes in a
 * backpack, and each item has a "kind".  The item prototype is this kind.
 * When Items are carried they give special abilities to that hero, and
 * perhaps the stack it is included in.
 * Items can be dropped onto the ground, and picked up from the ground.
 * When a hero dies, all of that hero's items get dropped onto the ground.
 *
 */

class ItemProto: public Renamable
{
    public:

	//! The xml tag of this object in an itemlist configuration file.
	/**
	 * @note This tag appears in the item configuration file, and in
	 * saved-game files.
	 */
	static Glib::ustring d_itemproto_tag;

	// The item can confer these special properties.
        enum Bonus {

	  //! Add 1 to the strength of the wearer.
	  ADD1STR         = 0x00000001,
	  //! Add 2 to the strength of the wearer.
	  ADD2STR         = 0x00000002,
	  //! Add 3 to the strength of the wearer.
	  ADD3STR         = 0x00000004,
	  //! Add 1 to the strength of the Stack.
	  ADD1STACK       = 0x00000008,
	  //! Add 2 to the strength of the Stack.
	  ADD2STACK       = 0x00000010,
	  //! Add 3 to the strength of the Stack.
	  ADD3STACK       = 0x00000020,
	  //! Provides the gift of flight to the Stack.
	  FLYSTACK        = 0x00000040,
	  //! Makes the stack go two times as far.
	  DOUBLEMOVESTACK = 0x00000080,
	  //! Add 2 gold to the Player's treasury per City it holds.
	  ADD2GOLDPERCITY = 0x00000100,
	  //! Add 3 gold to the Player's treasury per City it holds.
	  ADD3GOLDPERCITY = 0x00000200,
	  //! Add 4 gold to the Player's treasury per City it holds.
	  ADD4GOLDPERCITY = 0x00000400,
	  //! Add 5 gold to the Player's treasury per City it holds.
	  ADD5GOLDPERCITY = 0x00000800,
          //! Steal half of a player's gold.
          STEAL_GOLD      = 0x00001000,
          //! Sink all of a player's boats.
          SINK_SHIPS      = 0x00002000,
          //! Pick up any bags of items that are on the ground.
          PICK_UP_BAGS    = 0x00004000,
          //! Provide 2 movement points to the stack.
          ADD_2MP_STACK   = 0x00008000,
          //! Kill all of the giant worms
          BANISH_WORMS    = 0x00010000,
          //! Burn Bridge
          BURN_BRIDGE     = 0x00020000,
          //! Persuade a monster from a ruin into joining the stack.
          CAPTURE_KEEPER  = 0x00040000,
          //! Summon a monster.
          SUMMON_MONSTER  = 0x00080000,
          //! Target a city and kill a percentage of army units there.
          DISEASE_CITY    = 0x00100000,
          //! Make some defenders show up in a friendly city.
          RAISE_DEFENDERS = 0x00200000,
          //! Coerce a neutral city into flying your flag.
          PERSUADE_NEUTRALS = 0x00400000,
          //! Take the stack to a tile outside of a given city.
          TELEPORT_TO_CITY = 0x00800000,
          //! Plantable in the ground.
          PLANT_TO_VECTOR = 0x01000000
        };

        enum UsableItems {
          USABLE = STEAL_GOLD | SINK_SHIPS | PICK_UP_BAGS | ADD_2MP_STACK
            | BANISH_WORMS | BURN_BRIDGE | CAPTURE_KEEPER | SUMMON_MONSTER
            | DISEASE_CITY | RAISE_DEFENDERS | PERSUADE_NEUTRALS |
            TELEPORT_TO_CITY
        };


	static guint32 bonusFlagsFromString(const Glib::ustring str);
	static Glib::ustring bonusFlagsToString(const guint32 bonus);

	//! Loading constructor.
        ItemProto(XML_Helper* helper);

	//! Copy constructor.
        ItemProto(const ItemProto& orig);

	//! Creates a new Item Prototype from scratch.
        ItemProto(Glib::ustring name);

        //! Destructor.
        virtual ~ItemProto() {};

        //! Save the item to the opened saved-game file.
        bool save(XML_Helper* helper) const;
        //! Save the item, but not the enclosing d_tag.
        bool saveContents(XML_Helper* helper) const;

        //! Returns whether or not the Item has a particular special bonus.
        guint32 getBonus() const {return d_bonus;};

        //! Returns whether or not the Item has a particular special bonus.
        bool getBonus(ItemProto::Bonus bonus) const;

	//! Add a bonus to the Item.
	void addBonus(ItemProto::Bonus bonus);

	//! Remove a bonus from the Item.
	void removeBonus(ItemProto::Bonus bonus);

	//! Return some text describing the item's special abilities.
        Glib::ustring getBonusDescription() const;

        //! Return if the item is usable or not.
        bool isUsable() const {return d_bonus & USABLE;}

        guint32 getNumberOfUsesLeft() const {return d_uses_left;}

        //! Set the number of uses left.
        void setNumberOfUsesLeft(guint32 uses_left) {d_uses_left = uses_left;}

        bool usableOnVictimPlayer() const { if (d_bonus & SINK_SHIPS || d_bonus & STEAL_GOLD) return true; else return false;}

        bool usableOnEnemyCity() const { if (d_bonus & DISEASE_CITY) return true; else return false;}
        bool usableOnFriendlyCity() const { if (d_bonus & RAISE_DEFENDERS) return true; else return false;}
        bool usableOnNeutralCity() const { if (d_bonus & PERSUADE_NEUTRALS) return true; else return false;}
        bool usableOnAnyCity() const { if (d_bonus & TELEPORT_TO_CITY) return true; else return false;}

        bool hasArmyTypeToKill () const {return d_has_army_type_to_kill;}
        void clearArmyTypeToKill ()
          {d_has_army_type_to_kill = false; d_army_type_to_kill = 0;}
        guint32 getArmyTypeToKill() const {return d_army_type_to_kill;}
        void setArmyTypeToKill(guint32 type)
          {d_army_type_to_kill = type; d_has_army_type_to_kill = true;}

        double getPercentGoldToSteal() const {return d_steal_gold_percent;}
        void setPercentGoldToSteal(double p) {d_steal_gold_percent = p;}

        bool hasArmyTypeToSummon() const {return d_has_army_type_to_summon;}
        void clearArmyTypeToSummon ()
          {d_has_army_type_to_summon = false; d_army_type_to_summon = 0;}
        guint32 getArmyTypeToSummon() const {return d_army_type_to_summon;}
        void setArmyTypeToSummon(guint32 type)
          {d_army_type_to_summon = type; d_has_army_type_to_summon = true;}

        guint32 getBuildingTypeToSummonOn() const {return d_building_type_to_summon_on;}
        void setBuildingTypeToSummonOn(guint32 type) {d_building_type_to_summon_on = type;}

        bool isCurrentlyUsable(guint32 building, bool bags_on_map, bool victims_left, bool ruin_has_occupant, bool friendly_cities_present, bool enemy_cities_present, bool neutral_cities_present);
        double getPercentArmiesToKill() const {return d_percent_armies_to_kill;};
        void setPercentArmiesToKill(double p) {d_percent_armies_to_kill = p;}

        guint32 getMovementPointsToAdd() const {return d_mp_to_add;}
        void setMovementPointsToAdd(guint32 mp) {d_mp_to_add = mp;}

        bool hasArmyTypeToRaise () const {return d_has_army_type_to_raise;}
        void clearArmyTypeToRaise ()
          {d_has_army_type_to_raise = false; d_army_type_to_raise = 0;}
        guint32 getArmyTypeToRaise() const {return d_army_type_to_raise;}
        void setArmyTypeToRaise(guint32 type)
          {d_army_type_to_raise = type; d_has_army_type_to_raise = true;}

        guint32 getNumberOfArmiesToRaise() const {return d_num_armies_to_raise;}
        void setNumberOfArmiesToRaise(guint32 num) {d_num_armies_to_raise = num;}

    protected:
	//! The item's bonus.
	/**
	 * This value is a bitwise OR-ing of the values in ItemProto::Bonus.
	 */
        guint32 d_bonus;

        //! The number of uses this item has before it is spent.
        guint32 d_uses_left;

        //! Which army type to kill if d_bonus includes BANISH_WORMS.
        guint32 d_army_type_to_kill;

        //! How much gold to steal if d_bonus includes STEAL_GOLD.
        double d_steal_gold_percent;

        //! Which army type to summon when d_bonus includes SUMMON_MONSTER.
        guint32 d_army_type_to_summon;

        //! The building type to allow summoning in SUMMON_MONSTER.
        /**
         * When this value is 0 (Building::NONE), it means the monster can
         * be summoned on any tile.
         */
        guint32 d_building_type_to_summon_on;

        //! The percentage of army units to kill in DISEASE_CITY.
        double d_percent_armies_to_kill;

        //! How many movement points to add in ADD_2MP_STACK.
        guint32 d_mp_to_add;

        //! Which army type to create when RAISE_DEFENDERS is used.
        guint32 d_army_type_to_raise;

        //! How many armies to create when RAISE_DEFENDERS is used.
        guint32 d_num_armies_to_raise;

        //! Whether or not d_army_type_to_kill has a value
        bool d_has_army_type_to_kill;

        //! Whether or not d_army_type_to_summon has a value
        bool d_has_army_type_to_summon;

        //! Whether or not d_army_type_to_raise has a value
        bool d_has_army_type_to_raise;
    private:

	static Glib::ustring bonusFlagToString(ItemProto::Bonus type);
	static guint32 bonusFlagFromString(Glib::ustring str);

};

#endif //ITEM_PROTO_H
