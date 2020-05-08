//  Copyright (C) 2008, 2011, 2014 Ben Asselstine
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

//#include <iostream>
#include <fstream>
#include <sstream>
#include "recently-played-game.h"
#include "playerlist.h"
#include "citylist.h"
#include "xmlhelper.h"
#include "profile.h"

Glib::ustring RecentlyPlayedGame::d_tag = "recentlyplayedgame";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

RecentlyPlayedGame::RecentlyPlayedGame(GameScenario *game_scenario, Profile *p)
 : d_id (game_scenario->getId()), d_last_played(Glib::TimeVal()),
    d_round (game_scenario->getRound()),
    d_number_of_cities (Citylist::getInstance()->size()),
    d_number_of_players (Playerlist::getInstance()->size() - 1),
    d_playmode (GameScenario::PlayMode(game_scenario->getPlayMode())),
    d_name (game_scenario->getName()), d_profile_id (p->getId())
{
  d_last_played.assign_current_time();
}

RecentlyPlayedGame::RecentlyPlayedGame(XML_Helper* helper)
{
  helper->getData(d_id, "id");
  Glib::ustring s;
  helper->getData(s, "last_played_on");
  d_last_played.assign_from_iso8601(s);
  helper->getData(d_round, "round");
  helper->getData(d_number_of_cities, "number_of_cities");
  helper->getData(d_number_of_players, "number_of_players");
  Glib::ustring playmode_str;
  helper->getData(playmode_str, "playmode");
  d_playmode = GameScenario::playModeFromString(playmode_str);
  helper->getData(d_name, "name");
  helper->getData(d_profile_id, "profile_id");
}
        
RecentlyPlayedGame::RecentlyPlayedGame(Glib::ustring id, Glib::ustring profile_id, 
                                       guint32 round, guint32 num_cities, 
                                       guint32 num_players, 
                                       GameScenario::PlayMode mode, 
                                       Glib::ustring name)
: d_id(id), d_last_played(Glib::TimeVal()), d_round(round), 
    d_number_of_cities(num_cities), d_number_of_players(num_players),
    d_playmode(mode), d_name(name), d_profile_id(profile_id)
{
  d_last_played.assign_current_time();
}

RecentlyPlayedGame::RecentlyPlayedGame(const RecentlyPlayedGame &orig)
: d_id(orig.d_id), d_last_played(orig.d_last_played), d_round(orig.d_round), 
    d_number_of_cities(orig.d_number_of_cities), 
    d_number_of_players(orig.d_number_of_players), d_playmode(orig.d_playmode),
    d_name(orig.d_name), d_profile_id(orig.d_profile_id)
{
}

bool RecentlyPlayedGame::saveContents(XML_Helper *helper) const
{
  bool retval = true;
  retval &= helper->saveData("id", d_id);
  Glib::ustring s = d_last_played.as_iso8601();
  retval &= helper->saveData("last_played_on", s);
  retval &= helper->saveData("round", d_round);
  retval &= helper->saveData("number_of_cities", d_number_of_cities);
  retval &= helper->saveData("number_of_players", d_number_of_players);
  Glib::ustring playmode_str = GameScenario::playModeToString(d_playmode);
  retval &= helper->saveData("playmode", playmode_str);
  retval &= helper->saveData("name", d_name);
  retval &= helper->saveData("profile_id", d_profile_id);
  retval &= doSave(helper);
  return retval;
}

RecentlyPlayedGame* RecentlyPlayedGame::handle_load(XML_Helper *helper)
{
  Glib::ustring mode_str;
  helper->getData(mode_str, "playmode");
  GameScenario::PlayMode mode = GameScenario::playModeFromString(mode_str);
  switch (mode)
    {
    case GameScenario::HOTSEAT:
      return new RecentlyPlayedHotseatGame(helper);
    case GameScenario::NETWORKED:
      return new RecentlyPlayedNetworkedGame(helper);
    }
  return NULL;
}

bool RecentlyPlayedGame::save(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->openTag(RecentlyPlayedGame::d_tag);
  retval &= saveContents(helper);
  retval &= helper->closeTag();
  return retval;
}

//-----------------------------------------------------------------------------
//RecentlyPlayedHotseatGame

RecentlyPlayedHotseatGame::RecentlyPlayedHotseatGame(GameScenario *scen,
                                                     Profile *p)
	:RecentlyPlayedGame(scen, p), d_filename("")
{
}
	
RecentlyPlayedHotseatGame::RecentlyPlayedHotseatGame(const RecentlyPlayedHotseatGame &orig)
: RecentlyPlayedGame(orig), d_filename(orig.d_filename)
{
}

RecentlyPlayedHotseatGame::RecentlyPlayedHotseatGame(XML_Helper *helper)
	:RecentlyPlayedGame(helper)
{
  helper->getData(d_filename, "filename");
}

RecentlyPlayedHotseatGame::~RecentlyPlayedHotseatGame()
{
}

bool RecentlyPlayedHotseatGame::doSave(XML_Helper *helper) const
{
  bool retval = true;
  retval &= helper->saveData("filename", d_filename);
  return retval;
}

bool RecentlyPlayedHotseatGame::fillData(Glib::ustring filename)
{
  d_filename = filename;
  return true;
}

//-----------------------------------------------------------------------------
//RecentlyPlayedNetworkedGame

RecentlyPlayedNetworkedGame::RecentlyPlayedNetworkedGame(GameScenario *scen,
                                                         Profile *p)
	:RecentlyPlayedGame(scen, p), d_host(""), d_port(LORDSAWAR_PORT)
{
}

RecentlyPlayedNetworkedGame::RecentlyPlayedNetworkedGame
                              (Glib::ustring id, Glib::ustring profile_id, 
                               guint32 round, guint32 num_cities, 
                               guint32 num_players, 
                               GameScenario::PlayMode mode, 
                               Glib::ustring name, Glib::ustring host, guint32 port)
  : RecentlyPlayedGame(id, profile_id, round, num_cities, num_players, mode, 
                       name), d_host(host), d_port(port)
{
}

RecentlyPlayedNetworkedGame::RecentlyPlayedNetworkedGame(const RecentlyPlayedNetworkedGame &orig)
  : RecentlyPlayedGame(orig), d_host(orig.d_host), d_port(orig.d_port)
{
}

RecentlyPlayedNetworkedGame::RecentlyPlayedNetworkedGame(XML_Helper *helper)
	:RecentlyPlayedGame(helper)
{
  helper->getData(d_host, "host");
  helper->getData(d_port, "port");
}

RecentlyPlayedNetworkedGame::~RecentlyPlayedNetworkedGame()
{
}

bool RecentlyPlayedNetworkedGame::doSave(XML_Helper *helper) const
{
  bool retval = true;
  retval &= helper->saveData("host", d_host);
  retval &= helper->saveData("port", d_port);
  return retval;
}

bool RecentlyPlayedNetworkedGame::fillData(Glib::ustring host, guint32 port)
{
  d_host = host;
  d_port = port;
  return true;
}

