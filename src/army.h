// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2007, 2008, 2011, 2014, 2015, 2017, 2021 Ben Asselstine
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
#ifndef ARMY_H
#define ARMY_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>

#include "OwnerId.h"
#include "armybase.h"
#include "UniquelyIdentified.h"

class Player;
class Temple;
class XML_Helper;
class ArmyProto;
class ArmyProdBase;

//! An instance of an Army unit, an Army unit type, or an Army production base.
 /**
  * This class is the atom of every army.  It contains values such as
  * strength, movement points, upkeep, and so on.
  *
  * The purpose of the Army class is three-fold; an Army class can hold a
  * an Army prototype, or a production base, or an Army instance.
  *
  * The Army instance is the most frequently used purpose of the Army class.
  * An Army instance has a unique Id among all other game objects, has an 
  * owner (Player), and is included in a Stack.
  *
  * The Army unit prototype purpose is the second-most frequently used 
  * purpose of the Army class.  These types originate from a configuration 
  * file; for example: army/default/default.xml.  Every Army unit type has an 
  * type value that makes it unique among all other Army unit types in an 
  * Armyset.
  *
  * The production base purpose refers to the Army units that are included
  * in the City class as potential Army units that the City can produce.
  * This purpose is exactly the same as the Army unit prototype purpose, 
  * except it knows which Armyset it comes from, and it knows which Army
  * unit type it derives from.
  *
  * Maybe these three purposes will be split up into three or more classes 
  * in the future.
  */
class Army :public ArmyBase, public UniquelyIdentified, public OwnerId, public sigc::trackable
{
    public:

	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

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
	  MOVES_MULTIPLIER = 9,
          //! If SHIP, then this is how strong the army is on a boat.
          BOAT_STRENGTH = 10
        };

	//! Copy constructor. 
        Army(const ArmyProdBase& armyprodbase, Player* owner = 0);

	//! Copy constructor. 
        Army(const ArmyProto& armyproto, Player* owner = 0);

	//! Copy constructor. 
        Army(const Army& army, bool sync_id = false, Player *owner = 0);

        /** 
	 * Load an Army from an opened saved-game file or from an opened
	 * armyset configuration file.
         *
         * The constructor has to care for all three cases. Sometimes, an army
         * prototype is loaded, from which other units are cloned, sometimes
         * actual Army instances have to be loaded, and sometimes we load
	 * up a production base when loading the Army units that a City
	 * can produce.
         *
         * @param helper       The opened saved-game file to load from.
         */
	//! Loading constructor.
        Army(XML_Helper* helper);
        
	/**  
	 * Creates an empty prototype Army unit.  This constructor is only
	 * used in the ArmySetWindow (the Armyset editor).
	 */
	//! Create an empty army.
	Army();

	//! Destructor.
        virtual ~Army();

	static Army* createNonUniqueArmy(const ArmyProto& a, Player *p=NULL);
	static Army* createNonUniqueArmy(const ArmyProdBase& a, Player *p=NULL);


        // Set Methods
        
        //! Set the Id of Armyset and type that this Army belongs to.
        void setArmyset(guint32 armyset, guint32 type);
	
	//! Change the armyset that the army type for this army belongs to.
	void setArmyset(guint32 armyset_id) {d_armyset = armyset_id;};

        //! Set an Army statistic.
        void setStat(Stat stat, guint32 value);

        //! Set the current number of hitpoints of this army.
        void setHP(guint32 hp) {d_hp = hp;}

        //! Set the current number of hitpoints of this army to zero.
        void kill() {setHP(0);}

        //! Sets whether or not the Army has a particular medal.
        void setMedalBonus(guint32 index, bool value) 
	  {d_medal_bonus[index]=value;}
        
        //! Sets the number of battles the Army unit participated in.
        void setBattlesNumber(guint32 value) {d_battles_number=value;}
         
        //! Sets the number of hits this Army unit has scored against a foe.
        void setNumberHasHit(double value) {d_number_hashit=value;}

        //! Sets the number of hits this Army unit has suffered against a foe.
        void setNumberHasBeenHit(double value) {d_number_hasbeenhit=value;}

	//! Sets whether or not this Army unit is in a boat.
	void setInShip (bool s);

	//! Sets whether or not this Army unit is in a tower.
	void setFortified (bool f);

        // Get Methods 
        
	//! Get the Id of the Armyset to which the Army's type belongs.
        guint32 getArmyset() const {return d_armyset;}
        
        //! Get the type of this army.
	/**
	 * The type of the Army is the index of it's type in the Armyset.
	 */
        guint32 getTypeId() const {return d_type_id;}

        /** 
         * If modified is set to false, you get the raw, inherent value of
         * the army. Set it to true to get the modified one. This is not
         * important for generic armies, but heroes can have their stats
         * modified by wearing items.
	 *
	 * @param stat     The statistic to get the value of.
	 * @param modified Whether or not we get the modified stat value.
	 *
	 * @return The value of the statistic.
         */
	//! Returns the value of the given stat for the Army.
        virtual guint32 getStat(Stat stat, bool modified=true) const;

        //! Get the current number of hitpoints that the Army has.
        guint32 getHP() const {return d_hp;}

        //! Get the current number of movement points that the Army has.
        guint32 getMoves() const {return d_moves;}

        //! Get the current number of experience points that the Army unit has.
        double getXP() const {return d_xp;}

        //! Get the current level of the Army.
        guint32 getLevel() const {return d_level;}

        //! Return which medals this Army unit has.
        bool* getMedalBonuses() const {return (bool*)&d_medal_bonus;}

        //! Return whether or not the Army has a particular medal.
        bool getMedalBonus(guint32 index) const {return d_medal_bonus[index];}

        //! Returns the number of battles the Army unit participated in.
        guint32 getBattlesNumber() const {return d_battles_number;}

        //! Returns the number of blows the Army unit has scored against a foe.
        double getNumberHasHit() const {return d_number_hashit;}

        //! Returns the number of hits this Army unit has suffered.
        double getNumberHasBeenHit() const {return d_number_hasbeenhit;}

	//! Return whether or not the Army is in a tower.
	bool getFortified () const;

	//! Is this army a hero?
	/**
	 * isHero is overridden by the Hero class.
	 */
	virtual bool isHero() const {return false;};

	//! This army is of an army type that can be awarded as a reward.
	bool getAwardable() const;

	//! This army is of an army type that can be the keeper in a ruin.
	bool getDefendsRuins() const;

	//! This army is of an army type that has this name.
	virtual Glib::ustring getName() const;

        //! Does this army unit have wings?
        virtual bool isFlyer() const;

	//Methods that operate on class data and modify the class data
        /** 
	 * Regenerate an amount of the Army unit's hitpoints but not 
	 * exceeding the maximum number of hitpoints.
	 *
         * @param hp       The amount of hitpoints to heal.  Set to zero for 
	 *                 "natural" healing -- but this feature is not 
	 *                 currently used because wounded army units are always 
	 *                 fully healed after battle.
         */
	//! Heal the Army unit.
        void heal(guint32 hp = 0);

        /** 
	 * Decrease the number of hitpoints that this Army has.
         * 
         * @param damageDone       The amount of damage that the Army suffers.
	 *
         * @return True if the Army unit has died, otherwise false.
         */
	//! Damage the Army.
        bool damage(guint32 damageDone);

        /** 
	 * Reduce the number of moves that the Army unit has.
	 * @note This method doesn't reduce the maximum number of movement 
	 * points; it reduces the current number of movement points, which 
	 * get restored at the start of the next turn.
         * 
         * @param moves      The number of movement points to consume.
         */
	//! Consume some movement points.
        void decrementMoves(guint32 moves);

        /** 
	 * Add to the number of moves that the Army unit has.
	 * @note This method doesn't increase the maximum number of movement 
	 * points; it adds to the current number of movement points, which 
	 * get restored at the start of the next turn.
         * 
         * @param moves      The number of movement points to add.
         */
	//! Add some movement points.
        void incrementMoves(guint32 moves);

        //! Restores the number of movement points to the maximum level.
        void resetMoves();

	/**
	 * Add 1 to the strength of the Army unit if it has not already
	 * visited the Temple at which it's parent stack is currently
	 * sitting on.
	 *
	 * @param temple   The temple that the army is being blessed at.
	 *
	 * @return True if the Army unit was blessed, otherwise false.
	 */
	//! Bless the Army unit if it hasn't already visited this Temple.
        bool bless(Temple *temple);

        //! Increases the experience points of the Army by the given amount.
        void gainXp(double n);

	//! Make this army look and behave like another one.
	void morph(const ArmyProto *armyproto);

	//Methods that operate on class data and do not modify the class data

	//! Returns whether or not the army was blessed at the given temple.
        bool blessedAtTemple(guint32 temple_id) const;

        //! Saves the Army to an opened saved-game file.
        virtual bool save(XML_Helper* helper) const;

	//signals
        
	/**
	 * @note This signal is static because sometimes the army doesn't 
	 * exist yet when the signal is connected.
	 *
	 * @param army  The army that has died.
	 */
	//! Emitted when an Army has died.
        static sigc::signal<void, Army*> sdying;

    protected:

        //! Generic method for saving Army data.  Useful to the Hero class.
        bool saveData(XML_Helper* helper) const;

	//! The index of the Army unit's type in it's Armyset.
        guint32 d_type_id;

	//! The Id of the Armyset that the Army prototype belongs to.
        guint32 d_armyset;

	/**
	 * The maximum number of hitpoints is the secondmost important
	 * factor when calculating the outgoing of a Fight.
	 *
	 * This value should always be 2.
	 *
	 * This value does not change during gameplay.
	 */
	//! The maximum number of hitpoints this Army unit has.
        guint32 d_max_hp;

	//! Movement point multiplier of the Army unit.
	/**
	 * If an Army unit is being affected by a Hero unit's Item that
	 * confers the Item::DOUBLEMOVESTACK Item::Bonus, the effect is
	 * stored in d_max_moves_multiplier.
	 *
	 * This value must be 1 or more.
	 *
	 * This value typically changes from 1 to 2, and back to 1 during
	 * gameplay.
	 */
	guint32 d_max_moves_multiplier;

	//! Movement point bonus due to resting.
	/**
	 * When an army unit doesn't use all of it's movement points in a
	 * turn, some of those points get held-over until the following turn.
	 * If a unit has 3 movement points remaining, the bonus is 2.  If the
	 * unit has 2 movement points remaining, the bonus is 2.  If the unit
	 * has 1 movement point remaining, the bonus is 1.
	 *
	 * This value is a number between 0 and 2.
	 */
	guint32 d_max_moves_rest_bonus;

	/**
	 * Being in a ship affects the Army's strength in battle.
	 * Every army has a strength of 4 when fighting on a boat.
	 * It also affects the number of moves that the Army has.
	 * See MAX_BOAT_MOVES.
	 *
	 * This value gets set and unset as Army unit's stack goes in
	 * and out of the water.
	 */
	//! Whether or not this Army unit is afloat in a boat.
	bool d_ship;

	//! The current number of hitpoints that the Army unit has.
	/**
	 * When this value is 0 it means the Army unit is dead.
	 * 
	 * During a Fight this value gets decremented as the Army unit
	 * suffers attacks by enemy Army units.
	 *
	 * After a Fight, this value gets restored to d_max_hp.
	 *
	 * d_hp does not exceed d_max_hp.
	 */
        guint32 d_hp;

	//! The current number of movement points that the Army unit has.
	/**
	 * As the Army unit moves around in a Stack on the GameMap, it travels
	 * over certain terrain tiles.  As the Army unit moves over a
	 * particular terrain Tile, d_moves dwindles in value.
	 *
	 * At the end of a round, this value gets restored to d_max_moves.
	 *
	 * d_moves does not exceed d_moves_hp.
	 */
        guint32 d_moves;

	//! The current level of experience points the Army unit has.
	/**
	 * This value increases as the Army unit assists in killing enemy
	 * Army units.  This value does not decrease during gameplay.
	 *
	 * This value affects what d_level the Army unit is.
	 */
        double d_xp;

	//! The experience level the Army unit has attained.
	/**
	 * This value increases as the Army unit increases it's experience
	 * points.  d_level increases when a multiple of Army::xp_per_level 
	 * is surpassed.  d_level does not decrease during gameplay.
	 *
	 * d_level is not factored into any calculations that affect the
	 * outcome of the game.  It's just for show.
	 *
	 * @note Only Hero units advance in levels.
	 */
        guint32 d_level;

	/**
	 * There are three different medals that an Army unit can win.
	 *
	 * Merciless Medal: medal for being extremely merciless.  A unit 
	 *                  gets this medal if in a combat it scores more 
	 *                  than 90% of hits.
	 * Defender Medal: medal for being very good in defense.  A unit 
	 *                 gets this medal if in a combat is never hit.
	 * Veteran Medal: medal for being very good in combat.  A unit gets 
	 *                this medal if it survives 10 battles.
	 *
	 * Medals do not affect the game outcome and are just for show.
	 */
	//! The medals that the Army unit has been given.
        bool d_medal_bonus[3];

        //! The total number of battles that this Army unit has fought in.
	/**
	 * d_battles_number is a counter that is used to know when to
	 * award the Veteran's medal to this Army unit.
	 *
	 * This value does not decrease during gameplay.
	 */
        guint32 d_battles_number;

        //! The weighted number of hits per battle for this Army unit.
	/**
	 * d_number_hashit is a counter that is used to know when to award
	 * the Merciless medal to this Army unit.
	 *
	 * This value does not decrease during gameplay.
	 */
        double d_number_hashit;

        //! The weighted number of times the Army unit has been hit in a battle.
	/**
	 * d_number_hasbeenhit is a counter that is used to know when to award
	 * the Defender medal to this Army unit.
	 *
	 * This value does not decrease during gameplay.
	 */
        double d_number_hasbeenhit;

	//! A list of the Ids of Temples the Army unit has visited.
	/**
	 * As the Army unit gets blessed at various Temple objects, this
	 * list grows with unique Temple Ids.
	 * The purpose of the list is to prevent the Army unit from being
	 * blessed at the same Temple more than once.
	 *
	 * The length of the list does not decrease during gameplay.
	 */
        std::list<guint32> d_visitedTemples;

	//! The number of experience points per experience level.
	/**
	 * When an Army unit's d_xp surpasses a multiple of xp_per_level,
	 * it increases it's d_level by 1.
	 */
        static const int xp_per_level = 10;

    private:

	//! Create an army with a non-unique id from an army prototype.
	Army(const ArmyProto& a, guint32 id, Player *player = NULL);
	//! Create an army with a non-unique id from an army production base.
	Army(const ArmyProdBase& a, guint32 id, Player *player = NULL);

};

#endif // ARMY_H
