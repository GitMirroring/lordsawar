// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2002 Mark L. Amidon
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009, 2011, 2014, 2021 Ben Asselstine
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
#ifndef CITY_H
#define CITY_H

#include <list>
#include <vector>
#include "NamedLocation.h"
#include "OwnerId.h"
#include "Renamable.h"
#include "prodslotlist.h"

class Player;
class Stack;
class Army;
class ArmyProdBase;

#define DEFAULT_CITY_NAME "Noname"
#define DEFAULT_CITY_INCOME 20

//! A City on the game map.
/**
 * Players vie for control of City objects on the game map.  The main goal
 * of the game is to conquer these City objects.  Cities can be also be razed,
 * making them uninhabitable and unconquerable.
 *
 * A city can produce armies, provide income, place produced armies on it's 
 * tiles and buy production as well as have it's name changed.  Cities can
 * also vector their produced units to another position on the map.
 *
 * A City has 4 production slots.  A production slot is a production 
 * capability of a city.  Every one of the slots can be filled with an 
 * Army production base.  Cities can be assigned a random set of Army
 * production bases.  The name of a City can also be randomly set from a 
 * list of potential City names.
 *
 * Some City objects are capital cities.  Every player has a single capital
 * city.  Conquering another player's capital city doesn't give any bonus
 * except for bragging rights.
 */
class City : public OwnerId, public NamedLocation, public ProdSlotlist
{
    public:

	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

	//! Default constructor.
        /** 
          * Make a new city object.
	  *
          * @param pos       The location of the city on the game map.
	  * @param width     The number of tiles this city spans.
          * @param name      The name of the city.
          * @param gold      The amount of gold the city produces each turn.
	  * @param numslots  The number of production slots for this city.
          */
        City(Vector<int> pos, guint32 width, 
	     Glib::ustring name = DEFAULT_CITY_NAME, 
	     guint32 gold = DEFAULT_CITY_INCOME, 
	     guint32 numslots = MAX_PRODUCTION_SLOTS_IN_A_CITY);

	//! Copy constructor.
        City(const City&, bool sync_id = false);

	//! Alternative copy constructor.
	/**
	 * Make a city by copying another one, but it has a different position.
	 */
        City(const City&, Vector<int> pos);

        //! Loading constructor.
	/**
	 * Make a new city object by reading it from a saved-game file.
	 *
	 * @param helper The opened saved-game file to load the City from.
	 */
        City(XML_Helper* helper, guint32 width);

	//! Destructor.
        ~City() {};

	//Get Methods
	
        //! Return if the city can produce new production lines.
        bool getBuildProduction() const {return d_build_production;}

        //! Return the defense level of the city.
        int getDefenseLevel() const;

        //! Return the income of the city per turn.
        guint32 getGold() const {return d_gold;}

        //! Returns whether or not the city has been destroyed.
        bool isBurnt() const {return d_burnt;}

        //! Returns whether or not the city is a capital city.
        bool isCapital() const {return d_capital;}

        //! Returns the original owner of this capital city.
        Player *getCapitalOwner() const {return d_capital_owner;}

        //! Get the position where the city will send produced armies.
	/**
	 * @return The position on the map where the city will send it's newly
	 *         produced armies.  Returns (-1,-1) if the city is not
	 *         vectoring.
	 */
        Vector<int> getVectoring() const {return d_vector;}

	bool isUnnamed() const {return getName() == getDefaultName() ? true : false;};


	//Set Methods

        //! Set whether or not the city can produce new production lines.
        void setBuildProduction(bool build){d_build_production = build;}

        //! Set the gold the city produces each turn.
        void setGold(guint32 gold){d_gold = gold;}

        //! Set whether or not the city is destroyed.
        void setBurnt(bool burnt){d_burnt = burnt;}

        //! Sets whether the city is a capital.
        void setCapital(bool capital) {d_capital = capital;}

        //! Sets whether the city is a capital.
        void setCapitalOwner(Player *p) {d_capital_owner = p;}

        //! Set the point where the city will send the produced armies.
        /**
	 * @note This method does not check to see if the destination point
	 * can receive yet another vectored unit.
	 *
	 * @param pos  The position on the map to send the produced units to.
	 *             The position must point to another city, or a planted
	 *             standard.  Set to (-1,-1) if not vectoring.
	 */
        void setVectoring(Vector<int> pos);


	// Methods that operate on class data and modify the class.

        //! Changes the owner of the city and prepares it for takeover.
	/**
	 * @param newowner  The pointer to the Player in Playerlist who is the
	 *                  new owner of this City.
	 */
        void conquer(Player* newowner);
        
        //! Sets the production to random starting values
	/**
	 * @param produce_allies   Whether or not awardable Army units can
	 *                         be production bases in this city.
	 * @param likely  A value between 0 and 3 that represents how likely
	 *                more production bases are.
	 */
	void setRandomArmytypes(bool produce_allies, int likely);

        //! Produces one instance of the strongest Army the city can produce.
        void produceStrongestProductionBase();

        //! Produces one instance of the weakest Army the city can produce.
        void produceWeakestProductionBase();

        //! Produces a scout in the city.
        void produceWeakestQuickestArmyInArmyset();

        //! Do everything neccessary for a new turn.
	/**
	 * Checks to see if an Army unit should be produced.
	 * Checks to see if a newly produced Army unit should be sent off to 
	 * the city's vector destination.
	 */
        void nextTurn();

	//! Changes the vectoring destination, but for en-route units too.
	/**
	 * This method acts like City::setVectoring but also changes the
	 * destination of the units that this city has already vectored.
	 *
	 * @note This method does not check to see if the destination point
	 * can receive yet another vectored unit.
	 *
	 * @param dest  The new destination position on the game map to vector 
	 *              newly produced Army units to.  The position must point
	 *              to a city or a planted standard.
	 *
	 * @return This method always returns true.
	 */
	bool changeVectorDestination(Vector<int> dest);

	//! Callback that makes the army show up.
	const Army *armyArrives(Stack *& stack);

        //! Kill a percentage of the army units in the city.
        std::list<Stack*> diseaseDefenders(double percent_to_kill);

        //! Change owner of city AND the stacks in it.
        void persuadeDefenders(Player *new_owner);
	
	// Methods that operate on class data and do not modify the class.

        //! Save the city to an opened saved-game file.
        bool save(XML_Helper* helper) const;
        
	//! Returns true if the city isn't accepting too many vectored armies.
	/**
	 * Scans all of the cities vectoring to this city.  If vectoring
	 * to this city would accrue the count to 
	 * MAX_CITIES_VECTORED_TO_ONE_CITY, this method returns false.
	 *
	 * @return True if the city can have another city vectoring to it.
	 *         Otherwise false.
	 */
	bool canAcceptMoreVectoring() const;

	//! Returns true if the city can accept vectoring from a set of cities.
	/**
	 * Instead of checking to see if one unit can be vectored here like in 
	 * City::canAcceptMoreVectoring, this method checks if the city can
	 * receive multiple units (from an equal number of cities, because a
	 * city can only produce and vector one army at a time).
	 *
	 * @param number_of_cities The number of cities to check to
	 *                         see if this city can receive on top of what
	 *                         it's already receiving.
	 *
	 * @return True if the city can have this many more cities vectoring 
	 *         to it.  Otherwise false.
	 */
	bool canAcceptMoreVectoring(guint32 number_of_cities) const;

	//! Return how many armies are in the city.
	guint32 countDefenders() const;

	//! Return the stacks that are inside the city walls.
	std::vector<Stack *> getDefenders() const;

	// Static Methods
	
	//! Get the default name of any city.
	static Glib::ustring getDefaultName() {return _(DEFAULT_CITY_NAME);};

    private:

	//! Callback for loading city objects from a saved-game file.
	bool load(Glib::ustring tag, XML_Helper *helper);

        //! Produces the currently active Army production base.
        Army * produceArmy(Stack *& stack);

	//! Makes an Army production base be a little different than expected.
	/**
	 * This method is responsible for making Army production bases be
	 * a little bit different than the Army prototypes they derive from.
	 * This method will take a prototype (e.g. scouts), and maybe give it
	 * a strength of 2 (rather than 1), or a time of 2 (rather than 1).
	 * It also works the other way.  Elephants can be altered to take 5 
	 * turns instead of 4 turns, or have their strength decreased to 7.
	 */
	void randomlyImproveOrDegradeArmy(ArmyProdBase *army);

	//! Sort the Army production bases that this city produces by strength.
	/**
	 * @note Pnly use this prior to the start of game.
	 */
	void sortProduction();

        // DATA

	//! The City gives the Player this much gold per turn.
        guint32 d_gold;

	//! The defense level of the city.
	/**
	 * This value is not taken into consideration for battles.
	 * It is just for show.
	 */
        int d_defense_level;
        
	//! Whether or not the city is destroyed or not.
	/**
	 * When City objects are razed they become uninhabitable and also 
	 * unconquerable.  They can still be used as a jumping off point
	 * into water, but they will not produce any more Army units or
	 * provide income to any players.
	 */
        bool d_burnt;

	//! Whether or not the city is vectoring units.
	/**
	 * Vectoring involves sending units to a destination other than this
	 * city.  The destination can be any other city also owned by the
	 * owner of this city, or to the planted standard belonging to the
	 * owner of this city.  It always takes two turns to get to the
	 * vectoring destination.  See VectoredUnitlist for more information.
	 *
	 * If this value is True, then the newly produced Army units are
	 * vectored to a destination determined by City::d_vector.
	 */
        bool d_vectoring;

	//! Where to send newly produced Army units to.
	/**
	 * A position on the game map to send the Army units that this City
	 * produces.  When vectoring is disabled, this value is (-1, -1).
	 * The position on the map must coincide with a City owned by the
	 * owner of this city, or the planted standard of the owner of this
	 * city.
	 */
        Vector<int> d_vector;

	//! Whether or not this is a capital city.
	/**
	 * Capital cities do not have a purpose other than bragging rights
	 * among players during gameplay.  Every Player starts with exactly
	 * one capital city.
	 *
	 * Conquering a capital city does not change the original owner of
	 * the capital city.
	 *
	 * If this value is True, this city is a capital city of the
	 * City::d_capital_owner player.  When false, it is not a capital city.
	 */
        bool d_capital;

	//! The original owner of this capital city.
	Player *d_capital_owner;

        //! Whether or not we're allowed to build new production lines.
        /**
         * This refers to the "buy" button in the city window.
         * If we're not allowed to build new production lines in the city,
         * the buy button will be disabled.
         *
         */
        bool d_build_production;
};

bool armyCompareStrength (const ArmyProdBase *lhs, const ArmyProdBase *rhs);
#endif // CITY_H
