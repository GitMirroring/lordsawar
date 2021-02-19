//  Copyright (C) 2007, 2008, 2009, 2014, 2015, 2020, 2021 Ben Asselstine
//  Copyright (C) 2008 Ole Laursen
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
#ifndef HERO_TEMPLATES_H
#define HERO_TEMPLATES_H

#include <vector>
#include "hero.h"
#include "defs.h"
#include "character.h"

class ArmyProto;
class HeroProto;
class XML_Helper;

//! A list of Hero Template objects.
/**
 * The HeroTemplates holds all hero templates together.
 *
 * It is implemented as a singleton. Upon creation, it reads the hero
 * description file and initialises an internal list.
 *
 * We can also load it from a saved game file if present.
 */
class HeroTemplates
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag;

        //! Makes a copy of the hero templates.
        HeroTemplates* copy () {return new HeroTemplates (*this);}

        //! Returns the singleton instance.
	static HeroTemplates* getInstance();

        //! Instantiate the object from a saved-game file.
	static HeroTemplates* getInstance(XML_Helper *helper);

        //! Explicitly deletes the singleton instance.
        static void deleteInstance();

        //! Get all the heroes belonging to the player with the given id.
        /**
         * The caller is responsible for freeing the returned objects.
         * (or pass them back with replaceHeroes)
         */
        std::vector<Character*> getHeroes (guint32 player_id);

        //! Replace all the heroes belonging to the player with the given id.
        /**
         * HeroTemplates takes control of the Character objects passed in.
         * This means the caller doesn't free them.
         */
        void replaceHeroes (guint32 player_id, std::vector<Character*> he);

        HeroProto *getRandomHero(int player_id);

	HeroProto *getRandomHero(Hero::Gender gender, int player_id);

        Character*getCharacterById (guint32 hero_id);

        //! Save the list of hero templates to a saved-game file.
        bool save(XML_Helper* helper) const;

        bool isDefault () const;

        //! itemlist changed, does it affect us?
        bool removeItemAffectsStartingItemIds (guint32 idx);

        //! Replace the current hero templates with another.
        static void reset (HeroTemplates *t);

	//! Destructor.
        ~HeroTemplates();

        //! meld Character and ArmyProtos objects into HeroProtos
        void populateHeroProtos ();

        guint32 getNextAvailableId () const;

    protected:
	//! Default constructor. The function reads in the heronames file and produces a set of hero templates to be randomly selected from.
	HeroTemplates();
        //! Copy constructor.
	HeroTemplates (const HeroTemplates &h);
        //! Construct the object from an opened saved-game file.
	HeroTemplates(XML_Helper *helper);

	bool load(Glib::ustring tag, XML_Helper *helper);

    private:
        //! the contents of the heronames data file
        std::list<Character*> d_characters;

        /* the hero protos we made from the character data */
        std::vector<HeroProto*> d_herotemplates[MAX_PLAYERS];

	//a list of male hero prototypes contained in the the army set.
	std::vector<ArmyProto*> d_male_heroes;
	//a list of female hero prototypes contained in the the army set.
	std::vector<ArmyProto*> d_female_heroes;

        static HeroTemplates* d_instance;

        void loadHeroTemplates(XML_Helper *helper);

        void loadHeroesFromArmysets ();

};

#endif
