//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2011, 2014, 2015, 2020 Ben Asselstine
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

#include <iostream>
#include <sstream>
#include <vector>
#include <glibmm.h>
#include "defs.h"
#include "ucompose.hpp"
#include "gui/main.h"

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
  std::string dump ()
    {
      std::stringstream out;

      out << "This map was made with the following parameters:" << std::endl;
      out << String::ucompose ("random seed: %1",
                               Main::instance().random_number_seed) << std::endl;
      out << String::ucompose ("%1 players", players.size ()) << std::endl;
      for (guint32 i = 0; i < players.size (); i++)
        {
          out << String::ucompose ("  player %1: type='", i);

          switch (players[i].type)
            {
            case GameParameters::Player::HUMAN:
              out << "HUMAN"; break;
            case GameParameters::Player::EASY:
              out << "EASY"; break;
            case GameParameters::Player::HARD:
              out << "HARD"; break;
            case GameParameters::Player::OFF:
              out << "OFF"; break;
            case GameParameters::Player::NETWORKED:
              out << "NETWORKED"; break;
            }
          out << String::ucompose ("', name='%1', id=%2",
                                   players[i].name, players[i].id) << std::endl;
        }
      out << String::ucompose ("map size: width=%1, height=%2",
                               map.width, map.height) << std::endl;
      out << "map terrain:" << std::endl;
      out << String::ucompose
        ("  grass=%1, water=%2, swamp=%3, forest=%4, hills=%5, mountains=%6",
         map.grass, map.water, map.swamp, map.forest, map.hills,
         map.mountains) << std::endl;
      out << String::ucompose
        ("map features: cities=%1, ruins=%2, temples=%3, signposts=%4",
         map.cities, map.ruins, map.temples, map.signposts) << std::endl;
      out << String::ucompose ("map path: '%1'", map_path) << std::endl;
      out << String::ucompose ("tile theme: '%1'", tile_theme) << std::endl;
      out << String::ucompose ("army theme: '%1'", army_theme) << std::endl;
      out <<
        String::ucompose ("shield theme: '%1'", shield_theme) << std::endl;
      out << String::ucompose ("city theme: '%1'", city_theme) << std::endl;
      out <<String::ucompose ("see opponents stacks: %1",
                              see_opponents_stacks) << std::endl;
      out << String::ucompose ("see opponents production: %1",
                               see_opponents_stacks) << std::endl;
      out << "quest policy: ";
      switch (play_with_quests)
        {
        case GameParameters::QuestPolicy::NO_QUESTING:
          out << "NO_QUESTING"; break;
        case GameParameters::QuestPolicy::ONE_QUEST_PER_PLAYER:
          out << "ONE_QUEST_PER_PLAYER"; break;
        case GameParameters::QuestPolicy::ONE_QUEST_PER_HERO:
          out << "ONE_QUEST_PER_HERO"; break;
        }
      out << std::endl;

      out << "quick start: ";
      switch (quick_start)
        {
        case GameParameters::QuickStartPolicy::NO_QUICK_START:
          out << "NO_QUICK_START"; break;
        case GameParameters::QuickStartPolicy::EVENLY_DIVIDED:
          out << "EVENLY_DIVIDED"; break;
        case GameParameters::QuickStartPolicy::AI_HEAD_START:
          out << "AI_HEAD_START"; break;
        }
      out << std::endl;

      out << String::ucompose ("hidden map: %1", hidden_map) << std::endl;
      out << String::ucompose ("diplomacy: %1", diplomacy) << std::endl;

      out << "neutral cities: ";
      switch (neutral_cities)
        {
        case GameParameters::NeutralCities::AVERAGE:
          out << "AVERAGE"; break;
        case GameParameters::NeutralCities::STRONG:
          out << "STRONG"; break;
        case GameParameters::NeutralCities::ACTIVE:
          out << "ACTIVE"; break;
        case GameParameters::NeutralCities::DEFENSIVE:
          out << "DEFENSIVE"; break;
        }
      out << std::endl;

      out << "razing cities: ";
      switch (razing_cities)
        {
        case GameParameters::RazingCities::NEVER:
          out << "NEVER"; break;
        case GameParameters::RazingCities::ON_CAPTURE:
          out << "ON_CAPTURE"; break;
        case GameParameters::RazingCities::ALWAYS:
          out << "ALWAYS"; break;
        }
      out << std::endl;

      out << "vectoring mode: ";
      switch (vectoring_mode)
        {
        case GameParameters::VectoringMode::VECTORING_ALWAYS_TWO_TURNS:
          out << "VECTORING_ALWAYS_TWO_TURNS"; break;
        case GameParameters::VectoringMode::VECTORING_VARIABLE_TURNS:
          out << "VECTORING_VARIABLE_TURNS"; break;
        }
      out << std::endl;

      out << "build production mode: ";
      switch (build_production_mode)
        {
        case GameParameters::BuildProductionMode::BUILD_PRODUCTION_ALWAYS:
          out << "BUILD_PRODUCTION_ALWAYS"; break;
        case GameParameters::BuildProductionMode::BUILD_PRODUCTION_USUALLY:
          out << "BUILD_PRODUCTION_USUALLY"; break;
        case GameParameters::BuildProductionMode::BUILD_PRODUCTION_SELDOM:
          out << "BUILD_PRODUCTION_SELDOM"; break;
        case GameParameters::BuildProductionMode::BUILD_PRODUCTION_NEVER:
          out << "BUILD_PRODUCTION_NEVER"; break;
        }
      out << std::endl;

      out << "sacking mode: ";
      switch (sacking_mode)
        {
        case GameParameters::SackingMode::SACKING_ALWAYS:
          out << "SACKING_ALWAYS"; break;
        case GameParameters::SackingMode::SACKING_ON_CAPTURE:
          out << "SACKING_ON_CAPTURE"; break;
        case GameParameters::SackingMode::SACKING_ON_QUEST:
          out << "SACKING_ON_QUEST"; break;
        case GameParameters::SackingMode::SACKING_NEVER:
          out << "SACKING_NEVER"; break;
        }
      out << std::endl;

      out << String::ucompose ("cusp of war: %1", cusp_of_war) << std::endl;
      out << String::ucompose ("intense combat: %1", intense_combat) << std::endl;
      out << String::ucompose ("military advisor: %1",
                               military_advisor) << std::endl;
      out << String::ucompose ("random turns: %1", random_turns) << std::endl;
      out << String::ucompose ("cities can produce allies: %1",
                                     cities_can_produce_allies) << std::endl;
      out << String::ucompose ("difficulty : %1", difficulty) << std::endl;
      out << String::ucompose ("name: '%1'", name) << std::endl;
      return out.str ();
    }
};

#endif
