//  Copyright (C) 2021 Ben Asselstine
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
#ifndef SCENARIO_H
#define SCENARIO_H

#include <gtkmm.h>


class GameScenario;
class GameMap;
class ScenarioMedia;
class FL_Counter;
class Itemlist;
class Playerlist;
class Citylist;
class Templelist;
class Ruinlist;
class Rewardlist;
class Signpostlist;
class Roadlist;
class Stonelist;
class Portlist;
class Bridgelist;
class HeroTemplates;

//! A copy of the game model
/**
 * Holds all of the objects that comprise the data model of a scenario.
 * citylists, game scenario, game map, and so on.
 *
 */
class Scenario
{
    public:

	//! Default constructor.
        Scenario (const GameScenario *g);

        //! Copy constructor.
        Scenario (const Scenario &s);

	//! Destructor.
        ~Scenario ();

        GameScenario *getGameScenario () const {return d_game_scenario;}

        static void reset (Scenario *s);
    protected:

	// DATA
	
        GameScenario *d_game_scenario;
        GameMap *d_game_map;
        ScenarioMedia *d_scenario_media;
        FL_Counter *d_fl_counter;
        Itemlist *d_itemlist;
        Playerlist *d_playerlist;
        Citylist *d_citylist;
        Templelist *d_templelist;
        Ruinlist *d_ruinlist;
        Rewardlist *d_rewardlist;
        Signpostlist *d_signpostlist;
        Roadlist *d_roadlist;
        Stonelist *d_stonelist;
        Portlist *d_portlist;
        Bridgelist *d_bridgelist;
        HeroTemplates *d_hero_templates;
        bool d_resetted;
};

#endif // SCENARIO_H
