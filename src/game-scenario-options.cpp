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

#include <iostream>
#include <fstream>
#include <errno.h>
#include <sigc++/functors/mem_fun.h>

#include "GameScenarioOptions.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

bool GameScenarioOptions::s_see_opponents_stacks = false;
bool GameScenarioOptions::s_see_opponents_production = false;
GameParameters::QuestPolicy GameScenarioOptions::s_play_with_quests = GameParameters::ONE_QUEST_PER_PLAYER;
bool GameScenarioOptions::s_hidden_map = false;
bool GameScenarioOptions::s_diplomacy = false;
bool GameScenarioOptions::s_cusp_of_war = false;
GameParameters::NeutralCities GameScenarioOptions::s_neutral_cities = GameParameters::AVERAGE;
GameParameters::RazingCities GameScenarioOptions::s_razing_cities = GameParameters::ALWAYS;
bool GameScenarioOptions::s_intense_combat = false;
bool GameScenarioOptions::s_military_advisor = false;
bool GameScenarioOptions::s_random_turns = false;
bool GameScenarioOptions::s_surrender_already_offered = false;
unsigned int GameScenarioOptions::s_round = 0;
GameParameters::VectoringMode GameScenarioOptions::s_vectoring_mode = GameParameters::VECTORING_ALWAYS_TWO_TURNS;
GameParameters::BuildProductionMode GameScenarioOptions::s_build_production_mode = GameParameters::BUILD_PRODUCTION_ALWAYS;
GameParameters::SackingMode GameScenarioOptions::s_sacking_mode = GameParameters::SACKING_ALWAYS;

GameScenarioOptions::GameScenarioOptions()
{
}

int GameScenarioOptions::calculate_difficulty_rating(const GameParameters &g)
{
  float total_difficulty = 0;
  int max_player_difficulty = 73;
  int players_on = 0;
  for (std::vector<GameParameters::Player>::const_iterator it = g.players.begin();
       it != g.players.end(); ++it)
    {
      if ((*it).type != GameParameters::Player::OFF)
	players_on++;

    }
  int players_off = g.players.size() - players_on;

  //find out how much each player is worth
  float player_difficulty;
  player_difficulty = (float) max_player_difficulty / (float) players_off;
  if (players_on != 0)
    player_difficulty = (float)max_player_difficulty / (float)players_on;

  //go through all players, adding up difficulty points for each
  for (std::vector<GameParameters::Player>::const_iterator i = g.players.begin();
       i != g.players.end(); ++i)
    {
      if ((*i).type == GameParameters::Player::HUMAN || 
	  ((*i).type == GameParameters::Player::HARD))
	total_difficulty += player_difficulty;
      else if ((*i).type == GameParameters::Player::EASY)
	total_difficulty += player_difficulty;
      //FIXME: when the hard player gets better, switch this.
      //total_difficulty += (player_difficulty * 0.325);
      //else if ((*i).type == GameParameters::Player::EASIER)
      //total_difficulty += (player_difficulty * 0.655);
    }
  if (g.diplomacy)
    total_difficulty += (float) 3.0;
  if (g.hidden_map)
    total_difficulty += (float) 3.0;
  if (g.play_with_quests)
    total_difficulty += (float) 3.0;
  if (g.see_opponents_production == false)
    total_difficulty += (float) 2.0;
  if (g.see_opponents_stacks == false)
    total_difficulty += (float) 2.0;
  if (g.neutral_cities == GameParameters::STRONG)
    total_difficulty += (float) 3.0;
  else if (g.neutral_cities == GameParameters::ACTIVE)
    total_difficulty += (float) 5.0;
  else if (g.neutral_cities == GameParameters::DEFENSIVE)
    total_difficulty += (float) 6.0;
  if (g.razing_cities == GameParameters::ON_CAPTURE)
    total_difficulty += (float) 3.0;
  else if (g.razing_cities == GameParameters::NEVER)
    total_difficulty += (float) 6.0;
  if (g.cusp_of_war == true)
    total_difficulty += (float) 2.0;
  return (int) total_difficulty;
}

