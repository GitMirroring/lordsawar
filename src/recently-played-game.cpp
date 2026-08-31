//  Copyright (C) 2008, 2011, 2014, 2026 Ben Asselstine
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

//#include <iostream>
#include <fstream>
#include <sstream>
#include "recently-played-game.h"
#include "player-list.h"
#include "city-list.h"
#include "xml-helper.h"
#include "profile.h"

Glib::ustring RecentlyPlayedGame::d_tag = "recentlyplayedgame";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

RecentlyPlayedGame::RecentlyPlayedGame(GameScenario *game_scenario, Profile *p)
 : d_id (game_scenario->getId()), d_last_played(Glib::DateTime()),
    d_round (game_scenario->getRound()),
    d_number_of_cities (Citylist::instance()->size()),
    d_number_of_players (Playerlist::instance()->size() - 1),
    d_playmode (GameScenario::PlayMode(game_scenario->getPlayMode())),
    d_name (game_scenario->getName()), d_profile_id (p->getId())
{
  d_last_played = Glib::DateTime::create_now_local ();
}

RecentlyPlayedGame::RecentlyPlayedGame(XML_Helper* helper)
{
  helper->get(d_id, "id");
  Glib::ustring s;
  helper->get(s, "last_played_on");
  if (s == "")
    d_last_played = Glib::DateTime::create_now_local ();
  else
    d_last_played = Glib::DateTime::create_from_iso8601(s);
  helper->get(d_round, "round");
  helper->get(d_number_of_cities, "number_of_cities");
  helper->get(d_number_of_players, "number_of_players");
  Glib::ustring playmode_str;
  helper->get(playmode_str, "playmode");
  d_playmode = GameScenario::playModeFromString(playmode_str);
  helper->get(d_name, "name");
  helper->get(d_profile_id, "profile_id");
}
        
RecentlyPlayedGame::RecentlyPlayedGame(Glib::ustring id, Glib::ustring profile_id, 
                                       guint32 round, guint32 num_cities, 
                                       guint32 num_players, 
                                       GameScenario::PlayMode mode, 
                                       Glib::ustring name)
: d_id(id), d_last_played(Glib::DateTime::create_now_local ()), d_round(round), 
    d_number_of_cities(num_cities), d_number_of_players(num_players),
    d_playmode(mode), d_name(name), d_profile_id(profile_id)
{
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
  retval &= helper->save("id", d_id);
  Glib::ustring s = d_last_played.format_iso8601();
  retval &= helper->save("last_played_on", s);
  retval &= helper->save("round", d_round);
  retval &= helper->save("number_of_cities", d_number_of_cities);
  retval &= helper->save("number_of_players", d_number_of_players);
  Glib::ustring playmode_str = GameScenario::playModeToString(d_playmode);
  retval &= helper->save("playmode", playmode_str);
  retval &= helper->save("name", d_name);
  retval &= helper->save("profile_id", d_profile_id);
  retval &= doSave(helper);
  return retval;
}

RecentlyPlayedGame* RecentlyPlayedGame::handle_load(XML_Helper *helper)
{
  Glib::ustring mode_str;
  helper->get(mode_str, "playmode");
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
  retval &= helper->open_tag(RecentlyPlayedGame::d_tag);
  retval &= saveContents(helper);
  retval &= helper->close_tag();
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
  helper->get(d_filename, "filename");
}

RecentlyPlayedHotseatGame::~RecentlyPlayedHotseatGame()
{
}

bool RecentlyPlayedHotseatGame::doSave(XML_Helper *helper) const
{
  bool retval = true;
  retval &= helper->save("filename", d_filename);
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
  helper->get(d_host, "host");
  helper->get(d_port, "port");
}

RecentlyPlayedNetworkedGame::~RecentlyPlayedNetworkedGame()
{
}

bool RecentlyPlayedNetworkedGame::doSave(XML_Helper *helper) const
{
  bool retval = true;
  retval &= helper->save("host", d_host);
  retval &= helper->save("port", d_port);
  return retval;
}

bool RecentlyPlayedNetworkedGame::fillData(Glib::ustring host, guint32 port)
{
  d_host = host;
  d_port = port;
  return true;
}

