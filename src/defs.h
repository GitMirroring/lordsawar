// Copyright (C) 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2003, 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2004 David Sterba
// Copyright (C) 2005 Bryan Duff
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2014, 2015, 2016,
// 2020, 2021 Ben Asselstine
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

//This file contains the various macros used within lordsawar.

#pragma once
#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <gtkmm.h>
#include <glibmm.h>
#include <libintl.h>

#define LORDSAWAR_SAVEGAME_VERSION "0.3.3"
#define LORDSAWAR_TILESET_VERSION "0.3.3"
#define LORDSAWAR_ARMYSET_VERSION "0.3.3"
#define LORDSAWAR_CITYSET_VERSION "0.2.1"
#define LORDSAWAR_SHIELDSET_VERSION "0.3.3"
#define LORDSAWAR_CONFIG_VERSION "0.2.2"
#define LORDSAWAR_ITEMS_VERSION "0.2.1"
#define LORDSAWAR_RECENTLY_PLAYED_VERSION "0.2.1"
#define LORDSAWAR_RECENTLY_EDITED_VERSION "0.2.1"
#define LORDSAWAR_PROFILES_VERSION "0.3.0"
#define LORDSAWAR_RECENTLY_HOSTED_VERSION "0.3.0"
#define LORDSAWAR_PBM_TURN_VERSION "0.3.0"
#define _(string) Glib::locale_to_utf8(Glib::ustring(gettext(string))) // Macro for the gettext
#define N_(string) string


//-----------------------------------------------------------------------------
//some standard timers. They can easier be changed here than somewhere deep
//within the code, and sometimes you have to tweak them a little bit.
const unsigned int TIMER_BIGMAP_SELECTOR = 150; //milliseconds
const unsigned int TIMER_SMALLMAP_REFRESH = 8000; //microseconds
const unsigned int TIMER_BIGMAP_EXPLOSION_DELAY = 1500000; //microseconds
const unsigned int CITY_LEVELS = 4;
const unsigned int MAX_PLAYERS = 8;
const unsigned int TEMPLE_TYPES = 2;
const unsigned int RUIN_TYPES = 3;
const unsigned int DIPLOMACY_TYPES = 3;
const unsigned int ROAD_TYPES = 15;
const unsigned int STONE_TYPES = 89;
const unsigned int FOG_TYPES = 15;
const unsigned int BRIDGE_TYPES = 4;
const unsigned int CURSOR_TYPES = 13;
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

const Glib::ustring ARMYSETDIR = "army";
const Glib::ustring TILESETDIR = "tilesets";
const Glib::ustring CITYSETDIR = "citysets";
const Glib::ustring SHIELDSETDIR = "shield";
const Glib::ustring MAPDIR = "map";
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

const unsigned int MIN_PRODUCTION_TURNS_FOR_ARMY_UNITS = 1;
const unsigned int MAX_PRODUCTION_TURNS_FOR_ARMY_UNITS = 5;
const unsigned int MIN_UPKEEP_FOR_ARMY_UNITS = 0;
const unsigned int MAX_UPKEEP_FOR_ARMY_UNITS = 20;
const unsigned int MIN_MOVES_FOR_ARMY_UNITS = 6;
const unsigned int MAX_MOVES_FOR_ARMY_UNITS = 75;
const unsigned int MIN_STRENGTH_FOR_ARMY_UNITS = 1;
const unsigned int MAX_STRENGTH_FOR_ARMY_UNITS = 9;
const unsigned int MIN_COST_FOR_ARMY_UNITS = 0;
const unsigned int MAX_COST_FOR_ARMY_UNITS = 50;
const unsigned int MIN_NEW_COST_FOR_ARMY_UNITS = 0;
const unsigned int MAX_NEW_COST_FOR_ARMY_UNITS = 20000;
const unsigned int MIN_EXP_FOR_ARMY_UNITS = 0;
const unsigned int MAX_EXP_FOR_ARMY_UNITS = 50000;
const unsigned int MIN_SIGHT_FOR_ARMY_UNITS = 0;
const unsigned int MAX_SIGHT_FOR_ARMY_UNITS = 25;
const unsigned int MIN_MOVES_FOR_TILES = 1;
const unsigned int MAX_MOVES_FOR_TILES = 12;

const float SIGNPOST_FREQUENCY = 0.0030;

const Glib::ustring YELLOW_COLOR = "#FCFCECEC2020";
const Glib::ustring ORANGE_COLOR = "#FCFCA0A00000";
const Glib::ustring WHITE_COLOR = "#FFFFFFFFFFFF";
const Glib::ustring BLACK_COLOR = "#000000000000";
//const Glib::ustring DARK_GREY_COLOR = "#515151515151";
//const Glib::ustring LIGHT_GREY_COLOR = "#929292929292";
const Glib::ustring DARK_GREY_COLOR = "#292929292929";
const Glib::ustring LIGHT_GREY_COLOR = "#393f3f393f3f";
const Gdk::RGBA SEND_VECTORED_UNIT_LINE_COLOR(YELLOW_COLOR);
const Gdk::RGBA RECEIVE_VECTORED_UNIT_LINE_COLOR(ORANGE_COLOR);
const Gdk::RGBA SELECTOR_BOX_COLOR(WHITE_COLOR);
const Gdk::RGBA QUEST_LINE_COLOR(ORANGE_COLOR);
const Gdk::RGBA QUESTMAP_TARGET_BOX_COLOR(ORANGE_COLOR);
const Gdk::RGBA ROAD_PLANNER_TARGET_BOX_COLOR(ORANGE_COLOR);
const Gdk::RGBA GRID_BOX_COLOR(BLACK_COLOR);
const Gdk::RGBA FOG_COLOR(BLACK_COLOR);
const Gdk::RGBA VECTORMAP_ACTIVE_BOX_COLOR(WHITE_COLOR);
const Gdk::RGBA SELECTED_CITY_BOX_COLOR(WHITE_COLOR);
const Gdk::RGBA BEVELED_CIRCLE_DARK(DARK_GREY_COLOR);
const Gdk::RGBA BEVELED_CIRCLE_LIGHT(LIGHT_GREY_COLOR);
const Gdk::RGBA ACTIVE_RUIN_BOX(YELLOW_COLOR);

#ifdef GDK_WINDOWING_WIN32
const int SPEED_DELAY = 0;
const double ZOOM_STEP = 0.1;
#else
const int SPEED_DELAY = 300000;
const double ZOOM_STEP = 0.1;
#endif

const Glib::ustring DEFAULT_CONFIG_FILENAME = "lordsawarrc";

//1 in x chance of standing stone being on a road tile for random map.
const unsigned int ROAD_STONE_CHANCE = 150;

//how tall an army icon appears on a dialog relative to default font height
const double DIALOG_ARMY_PIC_FONTSIZE_MULTIPLE = 4.936363634;
//scale for the small shield pictures that appear on dialogs
const double DIALOG_SMALL_SHIELD_PIC_FONTSIZE_MULTIPLE = 0.727272727;
//scale for the medium shield pictures that appear on dialogs
const double DIALOG_MEDIUM_SHIELD_PIC_FONTSIZE_MULTIPLE = 1.872727273; //was 1.2
//scale for the large shield pictures
const double DIALOG_LARGE_SHIELD_PIC_FONTSIZE_MULTIPLE = 3.272727273;
//scale for the smaller diplomacy pictures
const double DIALOG_DIPLOMACY_TYPE_0_PIC_FONTSIZE_MULTIPLE = 3.0;
//scale for the larger diplomacy pictures
const double DIALOG_DIPLOMACY_TYPE_1_PIC_FONTSIZE_MULTIPLE = 5.0;
//scale for the mouse cursor graphics
const double DIALOG_CURSOR_PIC_FONTSIZE_MULTIPLE = 2.25;
//scale for the new hero level picture
const double DIALOG_NEW_LEVEL_PIC_FONTSIZE_MULTIPLE = 23;
//scale for the tartan progress images
const double DIALOG_TARTAN_PIC_FONTSIZE_MULTIPLE = 3.73;
//scale for the movement bonus pictures
const double DIALOG_MOVE_BONUS_PIC_FONTSIZE_MULTIPLE = 2;
//scale for the large medal awarded picture
const double DIALOG_MEDAL_PIC_FONTSIZE_MULTIPLE = 19;
//scale for the cities/treasury/income/upkeep pictures on the main game screen
const double DIALOG_STATUS_PIC_FONTSIZE_MULTIPLE = 2.333;
//scale for the tiny defense icon that appears on a city info tip
const double DIALOG_DEFENSE_PIC_FONTSIZE_MULTIPLE = 1.333;
//scale for the images in the buttons on the main game screen
const double DIALOG_GAME_BUTTON_PIC_FONTSIZE_MULTIPLE = 3;
//scale for the new turn (ship) picture
const double DIALOG_NEXT_TURN_PIC_FONT_SIZE_MULTIPLE = 23;
//scale for the male/female hero picture
const double DIALOG_NEW_HERO_PIC_FONT_SIZE_MULTIPLE = 23;
//scale for city defeated picture
const double DIALOG_CONQUERED_CITY_PIC_FONT_SIZE_MULTIPLE = 23;
//scale for the game winning cheese picture
const double DIALOG_WINNING_PIC_FONT_SIZE_MULTIPLE = 41;
//scale for the ruin success/defeated picture
const double DIALOG_RUIN_PIC_FONT_SIZE_MULTIPLE = 23;
//scale for the parley accepted/refuse picture
const double DIALOG_PARLEY_PIC_FONT_SIZE_MULTIPLE = 41;
//scale for the underline beneath the shields in the turn indicator
const double TURN_INDICATOR_FONT_SIZE_MULTIPLE = 0.3;
//scale for tile graphics appearing on dialogs in the scenario builder
const double EDITOR_DIALOG_TILE_PIC_FONTSIZE_MULTIPLE = 5.3;
//scale for the warlord commentator picture
const double DIALOG_COMMENTATOR_PIC_FONT_SIZE_MULTIPLE = 41;

//do dialog pics too

//for mingw:
#ifndef M_PI
 # define M_PI 3.14159265358979323846 /* pi */
#endif
#endif // DEFINITIONS_H

