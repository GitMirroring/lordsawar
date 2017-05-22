//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2011, 2014, 2015 Ben Asselstine
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
#ifndef GAME_PARAMETERS_H
#define GAME_PARAMETERS_H

#include <vector>
#include <glibmm.h>
#include "defs.h"

//! Scenario information that can be used to instantiate a new GameScenario.
class GameParameters
{
public:
    struct Player 
    {
	enum Type { HUMAN, EASY, HARD, OFF, NETWORKED };

	Type type;
	Glib::ustring name;
	guint32 id;
    };

    std::vector<Player> players;

    struct Map
    {
	int width, height;
	int grass, water, swamp, forest, hills, mountains;
	int cities, ruins, temples, signposts;
    };

    Map map;

    // path to map file to load, empty if none
    Glib::ustring map_path;
    Glib::ustring tile_theme;
    Glib::ustring army_theme;
    Glib::ustring shield_theme;
    Glib::ustring city_theme;

    bool see_opponents_stacks;
    bool see_opponents_production;
    enum QuestPolicy {
      NO_QUESTING = 0, ONE_QUEST_PER_PLAYER, ONE_QUEST_PER_HERO
    };
    QuestPolicy play_with_quests;
    enum QuickStartPolicy {
      NO_QUICK_START = 0,
      EVENLY_DIVIDED = 1,
      AI_HEAD_START = 2
    };
    QuickStartPolicy quick_start;

    bool hidden_map;
    bool diplomacy;

    enum NeutralCities {
        AVERAGE = 0, STRONG, ACTIVE, DEFENSIVE
    };
    NeutralCities neutral_cities;
    enum RazingCities {
        NEVER = 0, ON_CAPTURE, ALWAYS
    };
    RazingCities razing_cities;

    enum VectoringMode {
      VECTORING_ALWAYS_TWO_TURNS = 0,
      VECTORING_VARIABLE_TURNS = 1,
    };
    VectoringMode vectoring_mode;

    enum BuildProductionMode {
      BUILD_PRODUCTION_ALWAYS = 0,
      BUILD_PRODUCTION_USUALLY = 1,
      BUILD_PRODUCTION_SELDOM = 2,
      BUILD_PRODUCTION_NEVER = 3,
    };
    BuildProductionMode build_production_mode;

    enum SackingMode {
      SACKING_ALWAYS = 0,
      SACKING_ON_CAPTURE = 1,
      SACKING_ON_QUEST = 2,
      SACKING_NEVER = 3,
    };
    SackingMode sacking_mode;

    bool cusp_of_war;
    bool intense_combat;
    bool military_advisor;
    bool random_turns;
    bool cities_can_produce_allies;
    int difficulty;
    Glib::ustring name;
  static GameParameters::Player::Type player_type_to_player_param(guint32 type)
    {
      if (type == 0) //Player::HUMAN
        return GameParameters::Player::HUMAN;
      else if (type == 1) //Player::AI_FAST
        return GameParameters::Player::EASY;
      else if (type == 2) //Player::AI_DUMMY
        return GameParameters::Player::HUMAN; //no equiv.
      else if (type == 4) //Player::AI_SMART
        return GameParameters::Player::HARD;
      else if (type == 8)
        return GameParameters::Player::NETWORKED;
      return GameParameters::Player::OFF;
    }
  static guint32 player_param_to_player_type(guint32 param)
    {
      if (param == GameParameters::Player::HUMAN)
        return 0;
      else if (param == GameParameters::Player::EASY)
        return 1;
      else if (param == GameParameters::Player::HARD)
        return 4;
      else if (param == GameParameters::Player::NETWORKED)
        return 8;
      else if (param == GameParameters::Player::OFF)
        return 0; //no equiv.
      return 0;
    }
  static GameParameters::Player::Type player_param_string_to_player_param(Glib::ustring s)
    {
      if (s == HUMAN_PLAYER_TYPE) return GameParameters::Player::HUMAN;
      else if (s == EASY_PLAYER_TYPE) return GameParameters::Player::EASY;
      else if (s == HARD_PLAYER_TYPE) return GameParameters::Player::HARD;
      else if (s == NO_PLAYER_TYPE) return GameParameters::Player::OFF;
      else if (s == NETWORKED_PLAYER_TYPE) return GameParameters::Player::NETWORKED;
      else return GameParameters::Player::HUMAN;
    }
  static Glib::ustring player_param_to_string (guint32 type)
    {
      switch (type)
        {
        case GameParameters::Player::HUMAN: return HUMAN_PLAYER_TYPE;
        case GameParameters::Player::EASY: return EASY_PLAYER_TYPE;
        case GameParameters::Player::HARD: return HARD_PLAYER_TYPE;
        case GameParameters::Player::OFF: return NO_PLAYER_TYPE;
        case GameParameters::Player::NETWORKED: return NETWORKED_PLAYER_TYPE;
        default: return NO_PLAYER_TYPE;
        }
    }
};

#endif
