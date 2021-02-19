// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2007, 2008, 2014, 2017, 2020, 2021 Ben Asselstine
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
#ifndef HERO_H
#define HERO_H

#include <vector>
#include <list>

#include "army.h"

class Backpack;
class HeroProto;

//! A Hero is an Army unit capable of carrying items, going on quests and more.
/** 
 * The Hero class is just like the Army class except that the Hero gets
 * it's name, and gender set, and it's Army:Stat statistics are augmented by 
 * Item objects that are carried in a d_backpack.
 *
 * Heroes emerge in a City usually along with some powerful allies for a 
 * certain amount of gold pieces.
 *
 * Heroes are special because they can get a new Quest from a Temple, and
 * search Ruin objects.  They can pick up and drop Item objects, and can
 * also plant standards into the ground.
 *
 * Heroes are also unique in that they can increase in experience levels.
 */
class Hero : public Army
{
    public:

	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_hero_tag; 

        //! The different genders a Hero unit can have.
	/**
	 * The purpose of this enumeration is to show the correct 
	 * recruitment picture for Hero units when they emerge in a 
	 * City, and the Player has to decide if they want it or not.
	 */
        enum Gender {
	  //! The hero has no gender (Not used).
	  NONE = 0, 
	  //! The hero is male.
	  MALE = 1, 
	  //! The hero is female.
	  FEMALE = 2
	};

        /**
	 * Copies the prototype hero and creates a hero from it.
         */
	//! Default constructor.
        Hero(const HeroProto& a);

        /**
         * This performs a deep copy, including the Hero's items.
         */
	//! Copy constructor.
        Hero(Hero& h, bool sync_id = false);

        /** 
	 * @param helper   The opened saved-file to read the Hero from.
	 */
	//! Load a Hero from an opened saved-game file.
        Hero(XML_Helper* helper);

	//! Destructor.
        ~Hero();


	// Get Methods

	//! Return the name of this hero.
	virtual Glib::ustring getName() const {return d_name;};

        //! Return the hero blurb.
        Glib::ustring getDescription () const;

	//! Return that this object is a hero.
	bool isHero() const {return true;};

        //! Return the gender of the hero.
        guint32 getGender() const {return d_gender;}

        //! Return the id of the heroproto in hero templates
        guint32 getHeroTypeId () const {return d_hero_type_id;}

        //! Can the hero fly?
        bool isFlyer() const;

        /**
	 * Returns a stat of the hero.  See Army::Stat, and Army::getStat.
         * 
         * If modified is set to false, return the "raw", i.e. inherent
         * value of the hero. Otherwise, all items are checked for a 
         * bonus.
         */
        guint32 getStat(Army::Stat stat, bool modified = true) const;

        //! Returns the backpack of the hero.
        Backpack* getBackpack() {return d_backpack;}

	
	// Set Methods

	void setName(Glib::ustring name) {d_name = name;};

        //! Set the gender of the hero.
        void setGender(Gender gender){d_gender = gender;}

        //! Set the character/hero proto that the hero is based on.
        void setHeroTypeId (guint32 id) {d_hero_type_id = id;}

	// Methods that operate on class data and do not modify the class.

        //! Saves the Hero to a saved-game file.
        bool save(XML_Helper* helper) const;

        //! Is this hero on a quest?
        bool hasQuest() const;
        
	//! Return the natural command of the hero.
	/**
	 * Natural command is used for bonus calculations during a Fight.
	 *
	 * @return A number that is added to the strength to other Army and
	 *         Hero units in the Stack. 
	 */
	guint32 calculateNaturalCommand();


	// Static methods

	//! Convert a Hero::Gender string to an enumerated value.
	static Hero::Gender genderFromString(const Glib::ustring str);

	//! Convert a Hero::Gender enumerated value to a string.
	static Glib::ustring genderToString(const Hero::Gender gender);

        //! Convert a Hero::Gender to an easy to read string.
        static Glib::ustring genderToFriendlyName (const Hero::Gender gender);

        /** 
	 * Increase the Hero unit's level, and increase one of three stats;
	 * Stat::STRENGTH, Stat::MOVES, or Stat::SIGHT.
	 *
         * @param stat     The stat to increase.
	 *
         * @return How much the statistic increases or -1 upon error 
	 *         (e.g. because the XP is not enough).
         */
	//! Increase the Army's level, and increase a given stat.
        int gainLevel(Stat stat);

	/**
	 * Calculate how much a stat is increased because the Hero unit
	 * has increased it's level.
	 *
	 * @param stat  One of Stat::STRENGTH, Stat::MOVES, or Stat::SIGHT.
	 *
	 * @return The new value of the stat after it is increased.
	 */
	//! Return how much the stat would be boosted by gaining a level.
        int computeLevelGain(Stat stat) const;

        //! Checks whether or not the Hero unit can advance a level.
        bool canGainLevel() const;

	//! Returns how many experience points the next level requires.
        guint32 getXpNeededForNextLevel() const;

    private:
        
	//! Callback for loading the backpack from a saved-game file.
	bool loadBackpack(Glib::ustring tag, XML_Helper* helper);

	//! The hero's backpack that holds any number of Item objects.
        Backpack *d_backpack;

	//! The name of the hero.
	Glib::ustring d_name;

	//! Gender of the hero
	Hero::Gender d_gender;

        //! The hero id of the hero
        guint32 d_hero_type_id;
};

#endif //HERO_H
