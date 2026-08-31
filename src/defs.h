//  Copyright (C) 2001, 2002, 2003 Michael Bartl
//  Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
//  Copyright (C) 2003, 2004, 2005, 2006 Andrea Paternesi
//  Copyright (C) 2004 David Sterba
//  Copyright (C) 2005 Bryan Duff
//  Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2014, 2015, 2016, 2020,
//  2021, 2026 Ben Asselstine
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
//  Foundation, Inc., 31 Milk Street #960789, Boston, MA 02196, USA.

//This file contains the various macros used within lordsawar.

#pragma once
#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <gtkmm.h>
#include <glibmm.h>
#include <libintl.h>

#define LW_APP_ID "com.gitlab.benhorst.lw"
#define RESOURCE "/com/gitlab/benhorst/lw/"
#define LORDSAWAR_SAVEGAME_VERSION "0.4.0"
#define LORDSAWAR_TILESET_VERSION "0.4.0"
#define LORDSAWAR_ARMYSET_VERSION "0.4.0"
#define LORDSAWAR_CITYSET_VERSION "0.2.1"
#define LORDSAWAR_SHIELDSET_VERSION "0.4.0"
#define LORDSAWAR_CONFIG_VERSION "0.4.0"
#define LORDSAWAR_ITEMS_VERSION "0.2.1"
#define LORDSAWAR_RECENTLY_PLAYED_VERSION "0.2.1"
#define LORDSAWAR_RECENTLY_EDITED_VERSION "0.2.1"
#define LORDSAWAR_PROFILES_VERSION "0.4.0"
#define LORDSAWAR_RECENTLY_HOSTED_VERSION "0.3.0"
#define LORDSAWAR_PBM_TURN_VERSION "0.3.0"
#define _(string) Glib::locale_to_utf8(Glib::ustring(gettext(string))) // Macro for the gettext
#define N_(string) string


//-----------------------------------------------------------------------------
//some standard timers. They can easier be changed here than somewhere deep
//within the code, and sometimes you have to tweak them a little bit.
const unsigned int TIMER_BIGMAP_SELECTOR = 150; //milliseconds
//const unsigned int TIMER_SMALLMAP_REFRESH = 8000; //microseconds
const unsigned int TIMER_SMALLMAP_REFRESH = 8;
const unsigned int TIMER_BIGMAP_EXPLOSION_DELAY = 1500; //milliseconds
const unsigned int TIMER_BIGMAP_ABBREVIATED_FIGHT_DELAY = 1500; //miliseconds
const unsigned int CITY_LEVELS = 4;
const unsigned int MAX_PLAYERS = 8;
const unsigned int TEMPLE_TYPES = 2;
const unsigned int RUIN_TYPES = 3;
const unsigned int DIPLOMACY_TYPES = 3;
const unsigned int ROAD_TYPES = 15;
const unsigned int STONE_TYPES = 89;
const unsigned int FOG_TYPES = 15;
const unsigned int BRIDGE_TYPES = 4;
const unsigned int CURSOR_TYPES = 14;
const unsigned int DEFAULT_TILESTYLE_TYPES = 18;
const unsigned int MAX_CITIES_VECTORED_TO_ONE_CITY = 4;
const unsigned int MAX_TURNS_FOR_VECTORING = 2;
const unsigned int MAX_BOAT_MOVES = 18;
const unsigned int CUSP_OF_WAR_ROUND = 9;
const unsigned int DIPLOMACY_STARTING_SCORE = 8;
const unsigned int DIPLOMACY_MAX_SCORE = 15;
const unsigned int DIPLOMACY_MIN_SCORE = 0;
const unsigned int MAX_STACK_SIZE = 8;
const unsigned int FLAG_TYPES = MAX_STACK_SIZE;
const unsigned int MAX_ARMIES_ON_A_SINGLE_TILE = 8;
const unsigned int MAX_PRODUCTION_SLOTS_IN_A_CITY = 4;
const unsigned int MAX_ARMIES_PRODUCED_IN_NEUTRAL_CITY = 5;

const unsigned int MAP_SIZE_TINY_WIDTH = 50;
const unsigned int MAP_SIZE_TINY_HEIGHT = 75;
const unsigned int MAP_SIZE_SMALL_WIDTH = 70;
const unsigned int MAP_SIZE_SMALL_HEIGHT = 105;
const unsigned int MAP_SIZE_NORMAL_WIDTH = 112;
const unsigned int MAP_SIZE_NORMAL_HEIGHT = 156;

const unsigned int PRODUCTION_SHIELD_TYPES = 8;
const unsigned int MEDAL_TYPES = 3;
const unsigned int NUM_WAYPOINTS = 2;
const unsigned int NUM_GAME_BUTTON_IMAGES = 12;
const unsigned int NUM_ARROW_IMAGES = 8;

const int MAX_GOLD_TO_CARRY_OVER_TO_NEXT_SCENARIO = 5000;
const unsigned int MAX_ARMY_STRENGTH = 9;
const unsigned int MAX_BOAT_STRENGTH = 4;
const unsigned int BATTLE_DICE_SIDES_INTENSE = 24;
const unsigned int BATTLE_DICE_SIDES_NORMAL = 20;

const unsigned short LORDSAWAR_PORT = 14998;
const unsigned short LORDSAWAR_GAMELIST_PORT = 18998;
const unsigned short LORDSAWAR_GAMEHOST_PORT = 22998;
const unsigned int MINIMUM_CACHE_SIZE = (1 << 21);
#define HUMAN_PLAYER_TYPE _("Human")
#define EASY_PLAYER_TYPE _("Easy")
#define HARD_PLAYER_TYPE _("Hard")
#define NO_PLAYER_TYPE _("Off")
#define NETWORKED_PLAYER_TYPE _("Network")

const Glib::ustring ARMYSETDIR = "armies";
const Glib::ustring TILESETDIR = "terrain";
const Glib::ustring CITYSETDIR = "buildings";
const Glib::ustring SHIELDSETDIR = "shields";
const Glib::ustring MAPDIR = "maps";
const Glib::ustring ARMYSET_EXT = ".lwa";
const Glib::ustring TILESET_EXT = ".lwt";
const Glib::ustring CITYSET_EXT = ".lwc";
const Glib::ustring SHIELDSET_EXT = ".lws";
const Glib::ustring MAP_EXT = ".map";
const Glib::ustring SAVE_EXT = ".sav";
const Glib::ustring PBM_EXT = ".trn";
const Glib::ustring RECENTLY_PLAYED_LIST = "recently-played.xml";
const Glib::ustring RECENTLY_EDITED_LIST = "recently-edited.xml";
const Glib::ustring PROFILE_LIST = "profiles.xml";
const Glib::ustring RECENTLY_ADVERTISED_LIST = "recently-advertised.xml";
const Glib::ustring RECENTLY_HOSTED_LIST = "recently-hosted.xml";

const unsigned int MIN_LOOTED_GOLD = 10;

const float SIGNPOST_FREQUENCY = 0.0030;

const Glib::ustring YELLOW_COLOR = "#FCFCECEC2020";
const Glib::ustring ORANGE_COLOR = "#FCFCA0A00000";
const Glib::ustring WHITE_COLOR = "#FFFFFFFFFFFF";
const Glib::ustring BLACK_COLOR = "#000000000000";
const Glib::ustring GREY_1_COLOR = "#292929292929";
const Glib::ustring GREY_2_COLOR = "#393f3f393f3f";
const Glib::ustring GREY_3_COLOR = "#515151515151";
const Glib::ustring GREY_4_COLOR = "#929292929292";
const Gdk::RGBA SEND_VECTORED_UNIT_LINE_COLOR(YELLOW_COLOR);
const Gdk::RGBA RECEIVE_VECTORED_UNIT_LINE_COLOR(ORANGE_COLOR);
const Gdk::RGBA SELECTOR_BOX_COLOR(WHITE_COLOR);
const Gdk::RGBA QUEST_LINE_COLOR(ORANGE_COLOR);
const Gdk::RGBA QUESTMAP_TARGET_BOX_COLOR(ORANGE_COLOR);
const Gdk::RGBA SAGEMAP_TARGET_BOX_COLOR(ORANGE_COLOR);
const Gdk::RGBA SAGE_LINE_COLOR(ORANGE_COLOR);
const Gdk::RGBA ROAD_PLANNER_TARGET_BOX_COLOR(ORANGE_COLOR);
const Gdk::RGBA GRID_BOX_COLOR(BLACK_COLOR);
const Gdk::RGBA FOG_COLOR(BLACK_COLOR);
const Gdk::RGBA VECTORMAP_ACTIVE_BOX_COLOR(WHITE_COLOR);
const Gdk::RGBA SELECTED_CITY_BOX_COLOR(WHITE_COLOR);
const Gdk::RGBA BEVELED_CIRCLE_DARK(GREY_1_COLOR);
const Gdk::RGBA BEVELED_CIRCLE_LIGHT(GREY_2_COLOR);
const Gdk::RGBA DARK_MODE_BEVELED_CIRCLE_DARK(GREY_3_COLOR);
const Gdk::RGBA DARK_MODE_BEVELED_CIRCLE_LIGHT(GREY_4_COLOR);
const Gdk::RGBA ACTIVE_RUIN_BOX(YELLOW_COLOR);

#ifdef GDK_WINDOWING_WIN32
const int SPEED_DELAY = 0;
#else
const int SPEED_DELAY = 300;
#endif
const double ZOOM_STEP = 0.1;

#define DEFAULT_CONFIG_FILENAME "lordsawarrc"

//1 in x chance of standing stone being on a road tile for random map.
const unsigned int ROAD_STONE_CHANCE = 150;

#define LW_BUTTON_SIZE 44
//do dialog pics too

//for mingw:
#ifndef M_PI
 # define M_PI 3.14159265358979323846 /* pi */
#endif
#endif // DEFINITIONS_H

