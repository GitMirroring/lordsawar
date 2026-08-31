//  Copyright (C) 2002, 2003 Michael Bartl
//  Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
//  Copyright (C) 2004, 2005, 2006 Andrea Paternesi
//  Copyright (C) 2005 Josef Spillner
//  Copyright (C) 2006, 2007, 2008, 2011, 2014, 2015, 2017, 2020 Ben Asselstine
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

#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <sigc++/functors/mem_fun.h>

#include "Configuration.h"

#include "xmlhelper.h"
#include "defs.h"
#include "File.h"
#include "file-compat.h"
#include "ucompose.hpp"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

// define static variables

Glib::ustring Configuration::d_tag = "lordsawarrc";
Glib::ustring Configuration::s_configuration_file_path;
Glib::ustring Configuration::s_dataPath = LORDSAWAR_DATADIR;
Glib::ustring Configuration::s_savePath;
Glib::ustring Configuration::s_lang = "";
int Configuration::s_displaySpeedDelay = SPEED_DELAY;
int Configuration::s_displayFightRoundDelayFast = 250;
int Configuration::s_displayFightRoundDelaySlow = 500;
bool Configuration::s_displayCommentator = true;
guint32 Configuration::s_cacheSize = MINIMUM_CACHE_SIZE;
bool Configuration::s_zipfiles = false;
int Configuration::s_autosave_policy = 1;
bool Configuration::s_musicenable = false;
guint32 Configuration::s_musicvolume = 64;
guint32 Configuration::s_musiccache = 10000000;
Glib::ustring Configuration::s_filename = "";
bool Configuration::s_see_opponents_stacks = false;
bool Configuration::s_see_opponents_production = false;
GameParameters::QuestPolicy Configuration::s_play_with_quests = GameParameters::ONE_QUEST_PER_PLAYER;
GameParameters::VectoringMode Configuration::s_vectoring_mode = GameParameters::VECTORING_ALWAYS_TWO_TURNS;
GameParameters::BuildProductionMode Configuration::s_build_production_mode = GameParameters::BUILD_PRODUCTION_ALWAYS;
GameParameters::SackingMode Configuration::s_sacking_mode = GameParameters::SACKING_ALWAYS;
bool Configuration::s_hidden_map = false;
bool Configuration::s_diplomacy = false;
GameParameters::NeutralCities Configuration::s_neutral_cities = GameParameters::AVERAGE;
GameParameters::RazingCities Configuration::s_razing_cities = GameParameters::ALWAYS;
bool Configuration::s_intense_combat = false;
bool Configuration::s_military_advisor = false;
bool Configuration::s_random_turns = false;
GameParameters::QuickStartPolicy Configuration::s_quick_start = GameParameters::NO_QUICK_START;
bool Configuration::s_cusp_of_war = false;
bool Configuration::s_decorated = false;
bool Configuration::s_remember_recent_games = true;
bool Configuration::s_remember_recently_edited_files = true;
guint32 Configuration::s_double_click_threshold = 400; //milliseconds
Glib::ustring Configuration::s_gamelist_server_hostname = "";//lordsawar.com";
guint32 Configuration::s_gamelist_server_port = LORDSAWAR_GAMELIST_PORT;
Glib::ustring Configuration::s_gamehost_server_hostname = "";//lordsawar.com";
guint32 Configuration::s_gamehost_server_port = LORDSAWAR_GAMEHOST_PORT;
guint32 Configuration::s_font_size_override = 0;

Configuration::Configuration()
{
  File::create_dir (File::getConfigDir ());
  File::create_dir (File::getUserDataDir ());
  File::create_dir (File::getCacheDir ());
  if (s_configuration_file_path == "")
    s_configuration_file_path = File::getConfigFile (DEFAULT_CONFIG_FILENAME);
  if (s_savePath == "")
    s_savePath = File::add_slash_if_necessary (File::getUserDataDir ());

  char *s = setlocale(LC_ALL, "");
  if (s)
    Configuration::s_lang = s;
}

// check if file exists and parse it

bool Configuration::loadConfigurationFile(Glib::ustring fileName)
{
    debug("loadConfiguration()");
    s_filename=fileName;
     
    std::ifstream in(fileName.c_str());
    if (in)
    {
        //cout << _("Found configuration file: ") << fileName << endl;

        //parse the file
        XML_Helper helper(fileName.c_str(), std::ios::in);
        helper.registerTag(d_tag,
	    sigc::hide<0>(sigc::mem_fun(*this, &Configuration::parseConfiguration)));
    
        bool ret = helper.parseXML();
        helper.close();
        if (ret == false)
          std::cerr << String::ucompose(_("Okay, we're throwing your config file %1 away"), fileName) << std::endl;
        return ret;
    }
    else return false;
}

bool Configuration::saveConfigurationFile(Glib::ustring filename)
{
    bool retval = true;

    XML_Helper helper(filename, std::ios::out);

    //start writing
    retval &= helper.begin(LORDSAWAR_CONFIG_VERSION);
    retval &= helper.openTag(d_tag);
    
    //save the values 
    retval &= helper.saveData("datapath",s_dataPath);
    retval &= helper.saveData("savepath", s_savePath);
    retval &= helper.saveData("lang", s_lang);
    retval &= helper.saveData("cachesize", s_cacheSize);
    retval &= helper.saveData("zipfiles", s_zipfiles);
    Glib::ustring autosave_policy_str = savingPolicyToString(SavingPolicy(s_autosave_policy));
    retval &= helper.saveData("autosave_policy", autosave_policy_str);
    retval &= helper.saveData("speeddelay", s_displaySpeedDelay);
    retval &= helper.saveData("fightrounddelayfast", s_displayFightRoundDelayFast);
    retval &= helper.saveData("fightrounddelayslow", s_displayFightRoundDelaySlow);
    retval &= helper.saveData("commentator", s_displayCommentator);
    retval &= helper.saveData("musicenable", s_musicenable);
    retval &= helper.saveData("musicvolume", s_musicvolume);
    retval &= helper.saveData("musiccache", s_musiccache);
    retval &= helper.saveData("view_enemies", s_see_opponents_stacks);
    retval &= helper.saveData("view_production", s_see_opponents_production);
    Glib::ustring quest_policy_str = questPolicyToString(GameParameters::QuestPolicy(s_play_with_quests));
    retval &= helper.saveData("quests", quest_policy_str);
    Glib::ustring vectoring_mode_str = vectoringModeToString(GameParameters::VectoringMode(s_vectoring_mode));
    retval &= helper.saveData("vectoring_mode", vectoring_mode_str);
    Glib::ustring build_prod_mode_str = buildProductionModeToString(GameParameters::BuildProductionMode(s_build_production_mode));
    retval &= helper.saveData("build_production_mode", build_prod_mode_str);
    Glib::ustring sack_mode_str = sackingModeToString(GameParameters::SackingMode(s_sacking_mode));
    retval &= helper.saveData("sacking_mode", sack_mode_str);
    retval &= helper.saveData("hidden_map", s_hidden_map);
    retval &= helper.saveData("diplomacy", s_diplomacy);
    Glib::ustring neutral_cities_str = neutralCitiesToString(GameParameters::NeutralCities(s_neutral_cities));
    retval &= helper.saveData("neutral_cities", neutral_cities_str);
    Glib::ustring razing_cities_str = razingCitiesToString(GameParameters::RazingCities(s_razing_cities));
    retval &= helper.saveData("razing_cities", razing_cities_str);
    retval &= helper.saveData("intense_combat", s_intense_combat);
    retval &= helper.saveData("military_advisor", s_military_advisor);
    retval &= helper.saveData("random_turns", s_random_turns);
    Glib::ustring quick_start_str = quickStartPolicyToString(GameParameters::QuickStartPolicy(s_quick_start));
    retval &= helper.saveData("quick_start", quick_start_str);
    retval &= helper.saveData("cusp_of_war", s_cusp_of_war);
    retval &= helper.saveData("decorated", s_decorated);
    retval &= helper.saveData("remember_recent_games", s_remember_recent_games);
    retval &= helper.saveData("remember_recently_edited_files", s_remember_recently_edited_files);
    retval &= helper.saveData("double_click_threshold", 
			      s_double_click_threshold);
    retval &= helper.saveData("gamelist_server_hostname", 
			      s_gamelist_server_hostname);
    retval &= helper.saveData("gamelist_server_port", 
			      s_gamelist_server_port);
    retval &= helper.saveData("gamehost_server_hostname", 
			      s_gamehost_server_hostname);
    retval &= helper.saveData("gamehost_server_port", 
			      s_gamehost_server_port);
    retval &= helper.saveData("font_size_override",
			      s_font_size_override);
    retval &= helper.closeTag();
    
    if (!retval)
    {
        std::cerr << "Configuration: Something went wrong while saving.\n";
        return false;
    }
    
    helper.close();

    return true;
}

// parse the configuration file and set the variables

bool Configuration::parseConfiguration(XML_Helper* helper)
{
    debug("parseConfiguration()");
    
    Glib::ustring temp;
    bool retval, zipping;
    
    if (helper->getVersion() != LORDSAWAR_CONFIG_VERSION)
    {
      std::cerr << String::ucompose(_("Configuration file has wrong version.  Expected %1, but got %2"), LORDSAWAR_CONFIG_VERSION, helper->getVersion()) << std::endl;
            Glib::ustring orig = s_filename;
            Glib::ustring dest = s_filename+".OLD";
            std::cerr << String::ucompose(_("backing up config file `%1' to `%2'."), orig, dest) << std::endl;

            std::ofstream ofs(dest.c_str());
            std::ifstream ifs(orig.c_str());
	    ofs << ifs.rdbuf();
	    ofs.close();
            return false;
    }
   
    //get the paths
    retval = helper->getData(temp, "datapath");
    if (retval)
      s_dataPath = temp;
        
    retval = helper->getData(temp, "savepath");
    if (retval)
      s_savePath = temp;

    if (helper->getData(temp, "lang"))
        s_lang = temp;
    
    //parse cache size
    retval = helper->getData(temp, "cachesize");
    if (retval)
        s_cacheSize = atoi(temp.c_str());

    //parse if savefiles should be zipped
    retval = helper->getData(zipping, "zipfiles");
    if (retval)
        s_zipfiles = zipping;

    //parse when and how to save autosave files
    Glib::ustring autosave_policy_str;
    helper->getData(autosave_policy_str, "autosave_policy");
    s_autosave_policy = savingPolicyFromString(autosave_policy_str);

    //parse the speed delays
    helper->getData(s_displaySpeedDelay, "speeddelay");
    helper->getData(s_displayFightRoundDelayFast, "fightrounddelayfast");
    helper->getData(s_displayFightRoundDelaySlow, "fightrounddelayslow");

    //parse whether or not the commentator should be shown
    helper->getData(s_displayCommentator, "commentator");

    // parse musicsettings
    helper->getData(s_musicenable, "musicenable");
    helper->getData(s_musicvolume, "musicvolume");
    helper->getData(s_musiccache, "musiccache");
    
    helper->getData(s_see_opponents_stacks, "view_enemies");
    helper->getData(s_see_opponents_production, "view_production");
    Glib::ustring quest_policy_str;
    helper->getData(quest_policy_str, "quests");
    s_play_with_quests = questPolicyFromString(quest_policy_str);
    Glib::ustring vectoring_mode_str;
    helper->getData(vectoring_mode_str, "vectoring_mode");
    s_vectoring_mode = vectoringModeFromString(vectoring_mode_str);
    Glib::ustring build_prod_mode_str;
    helper->getData(build_prod_mode_str, "build_production_mode");
    s_build_production_mode =
      buildProductionModeFromString(build_prod_mode_str);
    Glib::ustring sack_mode_str;
    helper->getData(sack_mode_str, "sacking_mode");
    s_sacking_mode = sackingModeFromString(sack_mode_str);
    helper->getData(s_hidden_map, "hidden_map");
    helper->getData(s_diplomacy, "diplomacy");
    Glib::ustring neutral_cities_str;
    helper->getData(neutral_cities_str, "neutral_cities");
    s_neutral_cities = neutralCitiesFromString(neutral_cities_str);
    Glib::ustring razing_cities_str;
    helper->getData(razing_cities_str, "razing_cities");
    s_razing_cities = razingCitiesFromString(razing_cities_str);
    helper->getData(s_intense_combat, "intense_combat");
    helper->getData(s_military_advisor, "military_advisor");
    helper->getData(s_random_turns, "random_turns");
    Glib::ustring quick_start_str;
    helper->getData(quick_start_str, "quick_start");
    s_quick_start = quickStartPolicyFromString(quick_start_str);
    helper->getData(s_cusp_of_war, "cusp_of_war");
    helper->getData(s_decorated, "decorated");
    s_decorated = false;
    helper->getData(s_remember_recent_games, "remember_recent_games");
    helper->getData(s_remember_recently_edited_files, "remember_recently_edited_files");
    helper->getData(s_double_click_threshold, "double_click_threshold");
    helper->getData(s_gamelist_server_hostname, "gamelist_server_hostname");
    helper->getData(s_gamelist_server_port, "gamelist_server_port");
    helper->getData(s_gamehost_server_hostname, "gamehost_server_hostname");
    helper->getData(s_gamehost_server_port, "gamehost_server_port");
    helper->getData(s_font_size_override, "font_size_override");
    return true;
}


void initialize_configuration()
{
  bool try_upgrade = true;
  if (try_upgrade)
    {
      std::string cfgfile = File::getConfigFile (DEFAULT_CONFIG_FILENAME);
      FileCompat::support_backward_compatibility_for_common_files();
      Glib::ustring tmpfile = File::get_tmp_file(".tmp");
      File::copy(cfgfile, tmpfile);
      bool same_version = false;
      bool upgraded = FileCompat::getInstance()->upgrade(tmpfile, 
                                                         same_version);
      if (upgraded)
        File::copy(tmpfile, cfgfile);
      File::erase(tmpfile);
    }
  Configuration conf;

  bool foundconf = conf.loadConfigurationFile();
  if (!foundconf)
    {
      bool saveconf = conf.saveConfigurationFile();
      if (!saveconf)
        {
          std::cerr << String::ucompose(_("Error!  couldn't save configuration file `%1'.  Exiting."), Configuration::s_configuration_file_path) << std::endl;
          exit(-1);
        }
      else
        std::cerr << String::ucompose(_("Created default configuration file `%1'."), Configuration::s_configuration_file_path) << std::endl;
    }

  //Check if the save game directory exists. If not, try to create it.

  if (File::create_dir(Configuration::s_savePath) == false)
    {
      std::cerr << String::ucompose("Error!  Couldn't create saved game directory `%1'.  Exiting.", Configuration::s_savePath) << std::endl;
      exit(-1);
    }
  //Check if the personal armyset directory exists. If not, try to create it.
  if (File::create_dir(File::getSetDir(ARMYSET_EXT, false)) == false)
    {
      std::cerr << String::ucompose(_("Error!  Couldn't create armyset directory `%1'.  Exiting."), File::getSetDir(ARMYSET_EXT, false)) << std::endl;
      exit(-1);
    }
  //Check if the personal tileset directory exists. If not, try to create it.
  if (File::create_dir(File::getSetDir(TILESET_EXT, false)) == false)
    {
      std::cerr << String::ucompose(_("Error!  Couldn't create tileset directory `%1'.  Exiting."), File::getSetDir(TILESET_EXT, false)) << std::endl;
      exit(-1);
    }

  //Check if the personal maps directory exists. If not, try to create it.
  if (File::create_dir(File::getUserMapDir()) == false)
    {
      std::cerr << String::ucompose(_("Error!  Couldn't create map directory `%1'.  Exiting."), File::getUserMapDir()) << std::endl;
      exit(-1);
    }

  //Check if the personal shieldset directory exists. If not, try to make it.
  if (File::create_dir(File::getSetDir(SHIELDSET_EXT, false)) == false)
    {
      std::cerr << String::ucompose(_("Error!  Couldn't create shieldset directory `%1'.  Exiting."), File::getSetDir(SHIELDSET_EXT, false)) << std::endl;
      exit(-1);
    }

  //Check if the personal cityset directory exists. If not, try to make it.
  if (File::create_dir(File::getSetDir(CITYSET_EXT, false)) == false)
    {
      std::cerr << String::ucompose(_("Error!  Couldn't create cityset directory `%1'.  Exiting."), File::getSetDir(CITYSET_EXT, false)) << std::endl;
      exit(-1);
    }
}

Glib::ustring Configuration::neutralCitiesToString(const GameParameters::NeutralCities neutrals)
{
  switch (neutrals)
    {
      case GameParameters::AVERAGE:
	return "GameParameters::AVERAGE";
      case GameParameters::STRONG:
	return "GameParameters::STRONG";
      case GameParameters::ACTIVE:
	return "GameParameters::ACTIVE";
      case GameParameters::DEFENSIVE:
	return "GameParameters::DEFENSIVE";
    }
  return "GameParameters::AVERAGE";
}

GameParameters::NeutralCities Configuration::neutralCitiesFromString(Glib::ustring str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return GameParameters::NeutralCities(atoi(str.c_str()));
  if (str == "GameParameters::AVERAGE")
    return GameParameters::AVERAGE;
  else if (str == "GameParameters::STRONG")
    return GameParameters::STRONG;
  else if (str == "GameParameters::ACTIVE")
    return GameParameters::ACTIVE;
  else if (str == "GameParameters::DEFENSIVE")
    return GameParameters::DEFENSIVE;
    
  return GameParameters::AVERAGE;
}

Glib::ustring Configuration::razingCitiesToString(const GameParameters::RazingCities razing)
{
  switch (razing)
    {
      case GameParameters::NEVER:
	return "GameParameters::NEVER";
      case GameParameters::ON_CAPTURE:
	return "GameParameters::ON_CAPTURE";
      case GameParameters::ALWAYS:
	return "GameParameters::ALWAYS";
    }
  return "GameParameters::ALWAYS";
}

GameParameters::RazingCities Configuration::razingCitiesFromString(Glib::ustring str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return GameParameters::RazingCities(atoi(str.c_str()));
  if (str == "GameParameters::NEVER")
    return GameParameters::NEVER;
  else if (str == "GameParameters::ON_CAPTURE")
    return GameParameters::ON_CAPTURE;
  else if (str == "GameParameters::ALWAYS")
    return GameParameters::ALWAYS;
    
  return GameParameters::ALWAYS;
}

Glib::ustring Configuration::savingPolicyToString(const Configuration::SavingPolicy policy)
{
  switch (policy)
    {
    case Configuration::NO_SAVING:
      return "Configuration::NO_SAVING";
    case Configuration::WRITE_UNNUMBERED_AUTOSAVE_FILE:
      return "Configuration::WRITE_UNNUMBERED_AUTOSAVE_FILE";
    case Configuration::WRITE_NUMBERED_AUTOSAVE_FILE:
      return "Configuration::WRITE_NUMBERED_AUTOSAVE_FILE";
    }
  return "Configuration::NO_SAVING";
}

Configuration::SavingPolicy Configuration::savingPolicyFromString(Glib::ustring str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return Configuration::SavingPolicy(atoi(str.c_str()));
  if (str == "Configuration::NO_SAVING")
    return Configuration::NO_SAVING;
  else if (str == "Configuration::WRITE_UNNUMBERED_AUTOSAVE_FILE")
    return Configuration::WRITE_UNNUMBERED_AUTOSAVE_FILE;
  else if (str == "Configuration::WRITE_NUMBERED_AUTOSAVE_FILE")
    return Configuration::WRITE_NUMBERED_AUTOSAVE_FILE;
    
  return Configuration::WRITE_NUMBERED_AUTOSAVE_FILE;
}

Glib::ustring Configuration::quickStartPolicyToString(const GameParameters::QuickStartPolicy policy)
{
  switch (policy)
    {
    case GameParameters::NO_QUICK_START:
      return "GameParameters::NO_QUICK_START";
    case GameParameters::EVENLY_DIVIDED:
      return "GameParameters::EVENLY_DIVIDED";
    case GameParameters::AI_HEAD_START:
      return "GameParameters::AI_HEAD_START";
    }
  return "GameParameters::NO_QUICK_START";
}

GameParameters::QuickStartPolicy Configuration::quickStartPolicyFromString(Glib::ustring str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return GameParameters::QuickStartPolicy(atoi(str.c_str()));
  if (str == "GameParameters::NO_QUICK_START")
    return GameParameters::NO_QUICK_START;
  else if (str == "GameParameters::EVENLY_DIVIDED")
    return GameParameters::EVENLY_DIVIDED;
  else if (str == "GameParameters::AI_HEAD_START")
    return GameParameters::AI_HEAD_START;
    
  return GameParameters::NO_QUICK_START;
}

Glib::ustring Configuration::questPolicyToString(const GameParameters::QuestPolicy quest)
{
  switch (quest)
    {
      case GameParameters::NO_QUESTING:
	return "GameParameters::NO_QUESTING";
      case GameParameters::ONE_QUEST_PER_PLAYER:
	return "GameParameters::ONE_QUEST_PER_PLAYER";
      case GameParameters::ONE_QUEST_PER_HERO:
	return "GameParameters::ONE_QUEST_PER_HERO";
    }
  return "GameParameters::NO_QUESTING";
}

GameParameters::QuestPolicy Configuration::questPolicyFromString(Glib::ustring str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return GameParameters::QuestPolicy(atoi(str.c_str()));
  if (str == "GameParameters::NO_QUESTING")
    return GameParameters::NO_QUESTING;
  else if (str == "GameParameters::ONE_QUEST_PER_PLAYER")
    return GameParameters::ONE_QUEST_PER_PLAYER;
  else if (str == "GameParameters::ONE_QUEST_PER_HERO")
    return GameParameters::ONE_QUEST_PER_HERO;
    
  return GameParameters::NO_QUESTING;
}

Glib::ustring Configuration::vectoringModeToString(const GameParameters::VectoringMode vectoring)
{
  switch (vectoring)
    {
      case GameParameters::VECTORING_ALWAYS_TWO_TURNS:
	return "GameParameters::VECTORING_ALWAYS_TWO_TURNS";
      case GameParameters::VECTORING_VARIABLE_TURNS:
	return "GameParameters::VECTORING_VARIABLE_TURNS";
    }
  return "GameParameters::VECTORING_ALWAYS_TWO_TURNS";
}

GameParameters::VectoringMode Configuration::vectoringModeFromString(Glib::ustring str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return GameParameters::VectoringMode(atoi(str.c_str()));
  if (str == "GameParameters::VECTORING_ALWAYS_TWO_TURNS")
    return GameParameters::VECTORING_ALWAYS_TWO_TURNS;
  else if (str == "GameParameters::VECTORING_VARIABLE_TURNS")
    return GameParameters::VECTORING_VARIABLE_TURNS;

  return GameParameters::VECTORING_ALWAYS_TWO_TURNS;
}

Glib::ustring Configuration::buildProductionModeToString(const GameParameters::BuildProductionMode mode)
{
  switch (mode)
    {
      case GameParameters::BUILD_PRODUCTION_ALWAYS:
	return "GameParameters::BUILD_PRODUCTION_ALWAYS";
      case GameParameters::BUILD_PRODUCTION_USUALLY:
	return "GameParameters::BUILD_PRODUCTION_USUALLY";
      case GameParameters::BUILD_PRODUCTION_SELDOM:
	return "GameParameters::BUILD_PRODUCTION_SELDOM";
      case GameParameters::BUILD_PRODUCTION_NEVER:
	return "GameParameters::BUILD_PRODUCTION_NEVER";
    }
  return "GameParameters::BUILD_PRODUCTION_ALWAYS";
}

GameParameters::BuildProductionMode Configuration::buildProductionModeFromString(Glib::ustring str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return GameParameters::BuildProductionMode(atoi(str.c_str()));
  if (str == "GameParameters::BUILD_PRODUCTION_ALWAYS")
    return GameParameters::BUILD_PRODUCTION_ALWAYS;
  else if (str == "GameParameters::BUILD_PRODUCTION_USUALLY")
    return GameParameters::BUILD_PRODUCTION_USUALLY;
  else if (str == "GameParameters::BUILD_PRODUCTION_SELDOM")
    return GameParameters::BUILD_PRODUCTION_SELDOM;
  else if (str == "GameParameters::BUILD_PRODUCTION_NEVER")
    return GameParameters::BUILD_PRODUCTION_NEVER;

  return GameParameters::BUILD_PRODUCTION_ALWAYS;
}

Glib::ustring Configuration::sackingModeToString(const GameParameters::SackingMode sack)
{
  switch (sack)
    {
      case GameParameters::SACKING_ALWAYS:
	return "GameParameters::SACKING_ALWAYS";
      case GameParameters::SACKING_ON_CAPTURE:
	return "GameParameters::SACKING_ON_CAPTURE";
      case GameParameters::SACKING_ON_QUEST:
	return "GameParameters::SACKING_ON_QUEST";
      case GameParameters::SACKING_NEVER:
	return "GameParameters::SACKING_NEVER";
    }
  return "GameParameters::SACKING_ALWAYS";
}

GameParameters::SackingMode Configuration::sackingModeFromString(Glib::ustring str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return GameParameters::SackingMode(atoi(str.c_str()));
  if (str == "GameParameters::SACKING_ALWAYS")
    return GameParameters::SACKING_ALWAYS;
  else if (str == "GameParameters::SACKING_ON_CAPTURE")
    return GameParameters::SACKING_ON_CAPTURE;
  else if (str == "GameParameters::SACKING_ON_QUEST")
    return GameParameters::SACKING_ON_QUEST;
  else if (str == "GameParameters::SACKING_NEVER")
    return GameParameters::SACKING_NEVER;

  return GameParameters::SACKING_ALWAYS;
}

bool Configuration::upgrade(Glib::ustring filename, Glib::ustring old_version,
                            Glib::ustring new_version)
{
  return FileCompat::getInstance()->upgrade(filename, old_version, new_version,
                                            FileCompat::CONFIGURATION, 
                                            d_tag);
}

void Configuration::support_backward_compatibility()
{
  FileCompat::getInstance()->support_type (FileCompat::CONFIGURATION, "rc",
                                           d_tag, false);
  FileCompat::getInstance()->support_version
    (FileCompat::CONFIGURATION, "0.2.1", LORDSAWAR_CONFIG_VERSION,
     sigc::ptr_fun(&Configuration::upgrade));
}
