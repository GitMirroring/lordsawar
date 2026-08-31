// Copyright (C) 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2006, 2007, 2008, 2009, 2012, 2014, 2015, 2020,
// 2021 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
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
#ifndef CREATE_SCENARIO_H
#define CREATE_SCENARIO_H

#include <vector>
#include <list>
#include <gtkmm.h>
#include <sigc++/signal.h>
#include "CreateScenarioRandomize.h"
#include "vector.h"

class GameParameters;
class MapGenerator;
class GameScenario;
class Player;
class City;

/** \brief Creates and dumps (i.e. saves) a scenario.
  * 
  * The purpose of this class is to create a new scenario which can then be
  * "loaded" by the game scenario class. The advantage is that the scenario
  * creation can be concentrated in one specific class instead of being scattered
  * over different classes and all objects (city, player, stack...) only need
  * one "loading" constructor. The disavantage is that all significant changes
  * e.g. to the player class have to be reflected here, too.
  */

class CreateScenario : public CreateScenarioRandomize
{
    public:
    
        /** The Constructor
          * 
          * @param width    the width of the map in the new scenario
          * @param height   the heightof the map in the new scenario
          */
        CreateScenario(int width = 112, int height = 156);
        ~CreateScenario();

        // setters

        //! Set the terrain distribution; differences to 100% are grass; sum may exceed 100%
        void setPercentages(int pgrass, int pwater, int pforest, int pswamp,
                            int phills, int pmountains);

        //! Set the tileset of the map
        void setMapTiles(Glib::ustring tilesname);

	//! Set the shieldset for the map
	void setShieldset(Glib::ustring shieldsname);

	//! Set the cityset for the map
	void setCityset(Glib::ustring citysetname);

        //! Set the number of cities on the map
        void setNoCities(int number);

        //! Set the number of ruins on the map
        void setNoRuins(int number);

        //! Set the number of signposts on the map
        void setNoSignposts(int number);

        //! Set the number of standing stones on the map
        void setNoStones(int number);

        //! Set the number of temples
        void setNoTemples(int number);

        //! Set the width of the map
        void setWidth(int width);

        //! Set the height of the map
        void setHeight(int height);

        /** Add a player to the scenario
          * 
          * @param name     the name of the player
          * @param armyset  the name of the player's armyset
          * @param colors   the colors of the player
          * @param type     the type of the player (see class player for more info)
          * @return a pointer to the created player
          */
        Player* addPlayer(Glib::ustring name, guint32 armyset,
                          std::vector<Gdk::RGBA> colors, int type);

        /** Almost the same as addPlayer, but performs some additional checks
          * 
          * @param name     the name of the player
          * @param armyset  the name of the player's armyset
          * @param colors   the colors of the player
          * @param type     the type of the player (see class player for more info)
          * @return false if a neutral player already exists, true otherwise
          */
        bool addNeutral(Glib::ustring name, guint32 armyset,
                        std::vector<Gdk::RGBA> colors, int type);

        /** Creates a map
          * 
          * Calling this function will initiate the creation process itself. The
          * result is a saved map with several distributed players. The created
          * map can be either saved or used further (all lists etc. have already
          * been filled)
          */
        bool create(const GameParameters &g);

        /** Dumps (Saves) the map
          * 
          * @param filename     the full name of the save file
          * 
          * This will do the same as GameScenario::save() does (in fact it calls
          * GameScenario::save)
          */
        bool dump(Glib::ustring filename) const;

	MapGenerator *getGenerator() const {return d_generator;};
	static int calculateRoadType (Vector<int> t);
	static int calculateBridgeType (Vector<int> t);
        static int calculateStoneType (Vector<int> t);
        
        //! Emitted when the generator generates something
	sigc::signal<void> progress;

        static int calculateNumberOfSignposts(int width, int height, int grass);

        //! Determine the types for roads/bridges and standing stones.
        static void updateRoadsBridgesAndStones();

    private:
        //! Creates the map and distributes cities, temples and ruins
        bool createMap();

	void createCapitalCity(Player *player, City *city);
	bool tooNearToOtherCapitalCities(City *c, std::list<City*> capitals, guint32 distance);

        //! Distributes the players over the map
        bool distributePlayers();

        //! Setup city names and production
        bool setupCities(bool cities_can_produce_allies, 
			 int number_of_armies_factor);

        //! Setup temple names
        bool setupTemples();
        
        //! Set up ruin names and keepers
	bool setupRuins(bool strongholds_invisible, int sage_factor, 
			int no_guardian_factor, int stronghold_factor);


        //! Set up standing stones along roads and in the field
        static bool setupStandingStones(std::vector<Vector<int> > road_stones);

	//! Set up the standard set of items
	bool setupItems();

        //! Set up signposts
	//! @param ratio - how many signposts reference nearby cities vs
	//                 signposts that don't.
        bool setupSignposts(int ratio);

        //! Do some setup concerning the players (give them money)
	//! If we're playing with diplomacy then we start out at peace,
	//! and if we're not playing with diplomacy we start out at war.
        bool setupPlayers(bool random_turns, int base_gold);

	static bool setupRoads();
	static bool setupBridges();

	void quickStart();

	//! Given the difficulty, get some characteristics of ruins
	void getRuinDifficulty (int difficulty, int *sage_factor, 
				int *no_guardian_factor, 
				int *stronghold_factor);

	//! Given the difficulty, and whether we're doing a hidden map,
	//see how many signposts should point to cities vs how many
	//do not.
	void getSignpostDifficulty (int difficulty, bool hidden_map, 
				    int *signpost_ratio);

	//! Given the difficulty, see how many army units every city
	//produces by default.  it is thought that more is easier because
	//players do not have to pay for more armies.
	void getCityDifficulty (int difficulty, int *number_of_armies_factor);

	void on_progress();

        //data
        //for map creation
        GameScenario* d_scenario;
        MapGenerator* d_generator;
        Glib::ustring d_tilesname;
        Glib::ustring d_shieldsname;
        Glib::ustring d_citysetname;
        int d_width;
        int d_height;
};

#endif  //CREATE_SCENARIO_H
