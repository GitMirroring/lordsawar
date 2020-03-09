//  Copyright (C) 2002, 2003 Michael Bartl
//  Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
//  Copyright (C) 2004, 2005 Andrea Paternesi
//  Copyright (C) 2005 Josef Spillner
//  Copyright (C) 2006, 2010, 2011, 2014, 2015, 2017, 2020 Ben Asselstine
//  Copyright (C) 2007 Ole Laursen
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
#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <gtkmm.h>
#include <sigc++/trackable.h>

class XML_Helper;

#include "game-parameters.h"

// helper for making sure we got the initial configuration stuff up running
void initialize_configuration();

/** \brief The class which holds all configuration options.  e.g. ~/.lordsawarrc
  * 
  * Basically, this class is more of a namespace than a real class. It
  * provides all global information about directories, settings etc.
  */

// TODO: do we really want all this static stuff or rather do a singleton or such?

class Configuration : public sigc::trackable
{
    public:

        static Glib::ustring d_tag;

        // CREATORS
        Configuration();
        ~Configuration() {};

        /** \brief Load a configuration file
          * 
          * @param fileName     the full name of the config file
          *
          * This class loads an xml-style config file and sets the settings
          * appropriately.
          */
        bool loadConfigurationFile(Glib::ustring fileName = s_configuration_file_path);

        /** \brief Save the configuration file
          * 
          * @param fileName     the full name of the config file
          *
          * This class saves the current config to an xml-style config file.
          */
        static bool saveConfigurationFile(Glib::ustring filename = s_configuration_file_path);

        static Glib::ustring s_configuration_file_path;
	
        // as the name implies
        static bool s_showNextPlayer;
        static int s_displaySpeedDelay;
        static int s_displayFightRoundDelayFast;
        static int s_displayFightRoundDelaySlow;
        static bool s_displayCommentator;
        
        //the paths
        static Glib::ustring s_dataPath;
        static Glib::ustring s_savePath;

        // Language setting
        static Glib::ustring s_lang;

        //the maximum size of the graphics cache
        static guint32 s_cacheSize;

        //zip and obfuscate save files
        static bool s_zipfiles;

	// when to save autosave files
	// 0 = never, 1 = once a round overwrting, 
	// 2 = once a round not-overwriting
	static int s_autosave_policy;

        // music settings; the cache size is given in pieces instead of memory
        static bool s_musicenable;
        static guint32 s_musicvolume;
        static guint32 s_musiccache;

        // the hostname of the game-list server
        static Glib::ustring s_gamelist_server_hostname;
        static guint32 s_gamelist_server_port;

        // the hostname of the game-host server
        static Glib::ustring s_gamehost_server_hostname;
        static guint32 s_gamehost_server_port;

	// various default game settings
        static bool s_see_opponents_stacks;
        static bool s_see_opponents_production;
        static GameParameters::QuestPolicy s_play_with_quests;
        static GameParameters::VectoringMode s_vectoring_mode;
        static GameParameters::BuildProductionMode s_build_production_mode;
        static GameParameters::SackingMode s_sacking_mode;
        static bool s_hidden_map;
        static bool s_diplomacy;
        static GameParameters::NeutralCities s_neutral_cities;
        static GameParameters::RazingCities s_razing_cities;
        static bool s_intense_combat;
        static bool s_military_advisor;
        static bool s_random_turns;
        static GameParameters::QuickStartPolicy s_quick_start;
        static bool s_cusp_of_war;
        static bool s_decorated;
        static bool s_remember_recent_games;
        static bool s_remember_recently_edited_files;
	static guint32 s_double_click_threshold;
        static guint32 s_font_size_override;

	static GameParameters::NeutralCities neutralCitiesFromString(const Glib::ustring str);
	static Glib::ustring neutralCitiesToString(const GameParameters::NeutralCities neutrals);
	static GameParameters::RazingCities razingCitiesFromString(const Glib::ustring str);
	static Glib::ustring razingCitiesToString(const GameParameters::RazingCities razing);
        enum SavingPolicy {
	  NO_SAVING = 0,
	  WRITE_UNNUMBERED_AUTOSAVE_FILE = 1,
	  WRITE_NUMBERED_AUTOSAVE_FILE = 2
	};
	static Configuration::SavingPolicy savingPolicyFromString(const Glib::ustring str);
	static Glib::ustring savingPolicyToString(const Configuration::SavingPolicy policy);
	static GameParameters::QuickStartPolicy quickStartPolicyFromString(const Glib::ustring str);
        static Glib::ustring quickStartPolicyToString(const GameParameters::QuickStartPolicy policy);
        static GameParameters::QuestPolicy questPolicyFromString(Glib::ustring str);
        static Glib::ustring questPolicyToString(const GameParameters::QuestPolicy quest);
        static Glib::ustring vectoringModeToString(const GameParameters::VectoringMode vectoring);
        static GameParameters::VectoringMode vectoringModeFromString(Glib::ustring str);
        static Glib::ustring buildProductionModeToString(const GameParameters::BuildProductionMode mode);
        static GameParameters::BuildProductionMode buildProductionModeFromString(Glib::ustring str);
        static Glib::ustring sackingModeToString(const GameParameters::SackingMode mode);
        static GameParameters::SackingMode sackingModeFromString(Glib::ustring str);

        static bool upgrade(Glib::ustring filename, Glib::ustring old_version,
                            Glib::ustring new_version);
        static void support_backward_compatibility();
    private:
        /** \brief The callback for the XML_Helper
          * 
          * See the XML_Helper documentation for an explanation what the
          * callback is good for.
          */
        bool parseConfiguration(XML_Helper* helper);

        static Glib::ustring s_filename;
};

#endif // CONFIGURATION_H
