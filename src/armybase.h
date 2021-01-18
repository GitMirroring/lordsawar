// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2007, 2008, 2009, 2014, 2015, 2020, 2021 Ben Asselstine
// Copyright (C) 2007, 2008 Ole Laursen
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
#ifndef ARMY_BASE_H
#define ARMY_BASE_H

#include <gtkmm.h>


class XML_Helper;

//! The most basic set of attributes belonging to an army unit or army type.
class ArmyBase
{
    public:

	//! The bitwise OR-able special bonus that the Army gives.
        enum Bonus {
	  //! Provides +1 strength to the Army when positioned in the open.
	  ADD1STRINOPEN      = 0x00000001,
	  //! Provides +2 strength to the Army when positioned in the open.
	  ADD2STRINOPEN      = 0x00000002,
	  //! Provides +1 strength to the Army when positioned in the forest.
	  ADD1STRINFOREST    = 0x00000004,
	  //! Provides +1 strength to the Army when positioned in the hills.
	  ADD1STRINHILLS     = 0x00000008, 
	  //! Provides +1 strength to the Army when positioned in a City.
	  ADD1STRINCITY      = 0x00000010,
	  //! Provides +2 strength to the Army when positioned in a City.
	  ADD2STRINCITY      = 0x00000020,
	  //! Provides +1 strength to the Stack when positioned in the hills.
	  ADD1STACKINHILLS   = 0x00000040,
	  //! Negate any City bonuses from an enemy Stack during a Fight.
	  SUBALLCITYBONUS    = 0x00000080,
	  //! Negates 1 strength point from an enemy Stack during a Fight.
	  SUB1ENEMYSTACK     = 0x00000100,
	  //! Provides +1 strength to all Army units in the Stack.
	  ADD1STACK          = 0x00000200,
	  //! Provides +2 strength to all Army units in the Stack.
	  ADD2STACK          = 0x00000400,
	  //! Negate all non-Hero bonuses in an enemy Stack during a Fight.
	  SUBALLNONHEROBONUS = 0x00000800,
	  //! Negate all Hero bonuses in an enemy Stack during a Fight.
	  SUBALLHEROBONUS    = 0x00001000, //0 enemy hero bonus
	  //! Provides a +1 strength to all Army units in a fortified Stack.
	  FORTIFY            = 0x00002000,
          //! Provides a +2 strength to army units in the woods.
	  ADD2STRINFOREST    = 0x00004000,
          //! Provides a +2 strength to army units in the hills.
	  ADD2STRINHILLS     = 0x00008000, 
	  //! Negates 2 strength points from an enemy Stack during a Fight.
	  SUB2ENEMYSTACK     = 0x00010000,
	  //! Stack gets whatever movement bonus this army unit has.
	  CONFER_MOVE_BONUS = 0x00020000
        };
        
	//! Various kinds of statistics that an instance of Army unit has.
	/**
	 * This enumeration assists in getting and setting of statistics in
	 * an instance of an Army unit.
	 */
        enum Stat {
	  //! How strong the Army unit is in battle.
	  STRENGTH = 0,
	  //! The maximum number of hitpoints that the Army unit can have.
	  HP = 3,
	  //! The maximum number of moves the Army unit has.
	  MOVES = 4,
	  //! The various Tile::Type that the Army moves efficiently in.
	  MOVE_BONUS = 5,
	  //! The special bonus the Army has (Army::Bonus).
	  ARMY_BONUS = 6,
	  //! How far the Army unit can see on a hidden map.
	  SIGHT = 7,
	  //! If the Army unit is in a boat or not.
	  SHIP = 8,
	  //! If the Army unit is having it's movement doubled/tripled or not.
	  MOVES_MULTIPLIER = 9
        };

	//! Copy constructor.
        ArmyBase(const ArmyBase& army);

	//! Loading constructor.
        ArmyBase(XML_Helper* helper);
        
	//! Create an empty army base.
	ArmyBase();

	//! Destructor.
        virtual ~ArmyBase() {};


        // Set Methods
        
        //! Set how much gold this unit requires per turn.
        void setUpkeep(guint32 upkeep){d_upkeep = upkeep;}
        
        //! Set the strength of the army.
        void setStrength(guint32 strength) {d_strength = strength;}

        //! Set how much XP this unit is worth when killed.
        void setXpReward(double xp_value){d_xp_value = xp_value;}

        //! Set the maximum number of movement points.  for the scenario editor.
        void setMaxMoves(guint32 max_moves) {d_max_moves = max_moves;}

        // Get Methods
        
        //! Returns how many gold pieces this Army needs per turn.
        guint32 getUpkeep() const {return d_upkeep;}
        

        //! Get the army bonus of the army.
        guint32 getArmyBonus() const {return d_army_bonus;}

        //! Get the move bonus.
	/**
	 * Get which kinds of terrain tiles this Army moves efficiently 
	 * over top of.
	 *
	 * @return A bitwise OR-ing of the values in Tile::Type.
	 */
        guint32 getMoveBonus() const {return d_move_bonus;}

        //! Get the move bonus of the army.
        guint32 getMaxMoves() const {return d_max_moves;}

        //! Get the strength of the army.
        guint32 getStrength() const {return d_strength;}

        //! Get the distance this army can see on a hidden map.
	/**
	 * As this army walks on the map, it defogs the map with this radius.
	 */
        guint32 getSight() const {return d_sight;}

	//! Gets an easy to read string that represents the army's bonuses.
	Glib::ustring getArmyBonusDescription() const;

        //! Returns the number of XP that killing this Army garners it's killer.
        double getXpReward() const {return d_xp_value;}

        //! Return an easy to read string that represents the move bonuses.
        Glib::ustring getMoveBonusDescription () const;

	// Static Methods

	//! Convert an ArmyBase::Bonus string to a bitwise OR'd value.
	/**
	 * Converts a string containing the string representations of one
	 * or more ArmyBase::Bonus values to a bitwise OR'd value of those
	 * ArmyBase::Bonus values.  The terms are separated with a pipe `|'.
	 */
	static guint32 bonusFlagsFromString(const Glib::ustring str);

	//! Convert a series of ArmyBase::Bonus enum values to a string.
	/**
	 * Converts a bitwise OR'd value that represents many ArmyBase::Bonus
	 * enumerated values into a string, where the terms are separated by a
	 * pipe `|'.
	 */
	static Glib::ustring bonusFlagsToString(const guint32 bonus);

	//! Convert an ArmyBase::Bonus string to it's enum value.
	/**
	 * Converts a string containing a string representation of an 
	 * ArmyBase::Bonus enumerated value, and converts it to it's enumerated
	 * value.
	 */
	static guint32 bonusFlagFromString(const Glib::ustring str);

	//! Convert an ArmyBase::Bonus enum value to a string.
	static Glib::ustring bonusFlagToString(const ArmyBase::Bonus bonus);

	//! Convert a Tile::Type string to a bitwise OR'd value.
	/**
	 * Converts a string containing the string representations of one
	 * or more Tile::Type values to a bitwise OR'd value of those
	 * Tile::Type values.  The terms are separated with a pipe `|'.
	 */
	static guint32 moveFlagsFromString(const Glib::ustring str);

	//! Convert a series of Tile::Type enumerated values to a string.
	/**
	 * Converts a bitwise OR'd value that represents many Tile::Type
	 * enumerated values into a string, where the terms are separated by a
	 * pipe `|'.
	 */
	static Glib::ustring moveFlagsToString(const guint32 move_bonus);

	//! Convert an ArmyBase::Bonus enum value to text suitable for display.
        static Glib::ustring bonusFlagToFriendlyName(const ArmyBase::Bonus bonus);
    protected:

        //! Generic method for saving Army base data.
        bool saveData(XML_Helper* helper) const;

	//! The amount it costs to maintain this Army unit for this turn.
	/**
	 * @note The amount is in gold pieces.
	 *
	 * This value does not change during gameplay.
	 *
	 * @note Some special units have an upkeep of 0, but usually this
	 * value is more than zero.
	 */
        guint32 d_upkeep;

	/**
	 * The strength of the Army unit is the prime factor when 
	 * calculating the outcome of a Fight.  This value should always
	 * be 1 or more, but not exceeding 15.
	 *
	 * This value can permanently increase when the Army unit increases
	 * it's level.
	 *
	 * Temporary increases due to the Army unit being on a certain kind 
	 * of terrain, or because another Army unit has conferred strength 
	 * on it (see Army::Bonus) are not reflected in d_strength.
	 *
	 * This value does not decrease during gameplay.
	 */
	//! The base strength of the Army unit.
        guint32 d_strength;

	//! The maximum number of movement points that this Army unit has.
	/**
	 * This value must always be above 1.  Sane values are above 7.
	 *
	 * This value can be permanently increased when the Army unit 
	 * increases it's level.
	 *
	 * This value does not decrease during gameplay.
	 *
	 * @note When an Army unit is having it's movement doubled, or even
	 * tripled due to a Hero carrying an Item, this value does not 
	 * reflect that doubling or tripling.
	 */
        guint32 d_max_moves;

	//! How far the Army unit can see on a hidden map.
	/**
	 * When a stack is moving on a hidden map, a certain number of
	 * tiles get illuminated or unshaded.  d_sight is the radius of
	 * tiles that this Army unit can illuminate.
	 *
	 * This value can be permanently increased when the Army unit 
	 * increases it's level.
	 *
	 * This value does not decrease during gameplay.
	 */
        guint32 d_sight;

	//! The movement bonus of the Army unit.
	/**
	 * d_move_bonus represents the terrain tiles that the Army unit
	 * can travel efficiently over.  Traveling efficiently entails
	 * that it only costs 2 movement points to travel over that kind
	 * of terrain, no matter what the actual terrain movement value is.
	 *
	 * The movement bonus is a bitwise OR-ing of the values in 
	 * Tile::Type.
	 *
	 * When each of the members of Tile::Type are included in the
	 * d_move_bonus value, the Army unit is flying.
	 *
	 * This value does not change during gameplay.
	 */
        guint32 d_move_bonus;

	/**
	 * d_army_bonus represents the special abilities this Army unit has.
	 * The special abilities are enumerated in Army::Bonus.
	 *
	 * The army bonus is a bitwise OR-ing of the values in Army::Bonus.
	 *
	 * This value does not change during gameplay.
	 */
	//! The special capbilities of the Army unit.
        guint32 d_army_bonus;

	//! The amount of XP this Army unit worth when killed by an assailant.
	/**
	 * When this Army unit is killed, d_xp_value is added to the killer's
	 * experience points.
	 *
	 * This value must be over 0.
	 *
	 * This value does not change during gameplay.
	 */
        double d_xp_value;


    private:
};

#endif // ARMY_BASE_H
