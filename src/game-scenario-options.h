// Copyright (C) 2008, 2014, 2017 Ben Asselstine
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
#ifndef GAME_SCENARIO_OPTIONS_H
#define GAME_SCENARIO_OPTIONS_H

#include <list>
#include <sigc++/trackable.h>
#include "game-parameters.h"

//! A class to hold several scenario options.

class GameScenarioOptions: public sigc::trackable
{
    public:

        GameScenarioOptions();
        virtual ~GameScenarioOptions() {};


        static bool s_see_opponents_stacks;
        static bool s_see_opponents_production;
        static GameParameters::QuestPolicy s_play_with_quests;
        static bool s_hidden_map;
        static bool s_diplomacy;
        static bool s_cusp_of_war;
        static GameParameters::NeutralCities s_neutral_cities;
        static GameParameters::RazingCities s_razing_cities;
        static bool s_intense_combat;
        static bool s_military_advisor;
        static bool s_random_turns;
	static bool s_surrender_already_offered;
	static int s_difficulty;
        static GameParameters::VectoringMode s_vectoring_mode;
        static GameParameters::BuildProductionMode s_build_production_mode;
        static GameParameters::SackingMode s_sacking_mode;

        static unsigned int s_round;

	static int calculate_difficulty_rating(const GameParameters &g);

    private:
};

#endif // GAME_SCENARIO_OPTIONS_H

// End of file
