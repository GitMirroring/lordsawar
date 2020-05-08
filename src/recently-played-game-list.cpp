// Copyright (C) 2008, 2011, 2014 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>

#include "recently-played-game-list.h"
#include "recently-played-game.h"
#include <limits.h>
#include <fstream>
#include <iostream>
#include "xmlhelper.h"
#include "Configuration.h"
#include "defs.h"
#include "profile.h"
#include "profilelist.h"
#include "file-compat.h"
#include "File.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Glib::ustring RecentlyPlayedGameList::d_tag = "recentlyplayedgamelist";

RecentlyPlayedGameList* RecentlyPlayedGameList::s_instance = 0;

RecentlyPlayedGameList* RecentlyPlayedGameList::getInstance()
{
  if (s_instance == 0)
    s_instance = new RecentlyPlayedGameList();

  return s_instance;
}

bool RecentlyPlayedGameList::saveToFile(Glib::ustring filename) const
{
  bool retval = true;
  XML_Helper helper(filename, std::ios::out);
  retval &= save(&helper);
  helper.close();
  return retval;
}

bool RecentlyPlayedGameList::loadFromFile(Glib::ustring filename)
{
  remove_all();
  std::ifstream in(filename.c_str());
  if (in)
    {
      XML_Helper helper(filename.c_str(), std::ios::in);
      helper.registerTag(RecentlyPlayedGame::d_tag, sigc::mem_fun(this, &RecentlyPlayedGameList::load_tag));
      bool retval = helper.parseXML();
      helper.close();
      if (retval == false)
	File::erase(filename);
      return retval;
    }
  return true;
}

RecentlyPlayedGameList* RecentlyPlayedGameList::getInstance(XML_Helper* helper)
{
  if (s_instance)
    deleteInstance();

  s_instance = new RecentlyPlayedGameList(helper);
  return s_instance;
}

void RecentlyPlayedGameList::deleteInstance()
{
  if (s_instance)
    delete s_instance;

  s_instance = 0;
}

RecentlyPlayedGameList::RecentlyPlayedGameList()
{
}

RecentlyPlayedGameList::RecentlyPlayedGameList(XML_Helper* helper)
{
  helper->registerTag(RecentlyPlayedGame::d_tag, sigc::mem_fun(this, &RecentlyPlayedGameList::load_tag));
}

RecentlyPlayedGameList::~RecentlyPlayedGameList()
{
  remove_all();
}

void RecentlyPlayedGameList::remove_all()
{
  for (RecentlyPlayedGameList::iterator it = begin(); it != end(); ++it)
    delete *it;
  clear();
}

bool RecentlyPlayedGameList::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->begin(LORDSAWAR_RECENTLY_PLAYED_VERSION);
  retval &= helper->openTag(RecentlyPlayedGameList::d_tag);

  for (const_iterator it = begin(); it != end(); ++it)
    (*it)->save(helper);

  retval &= helper->closeTag();

  return retval;
}

bool RecentlyPlayedGameList::load_tag(Glib::ustring tag, XML_Helper* helper)
{
  if (helper->getVersion() != LORDSAWAR_RECENTLY_PLAYED_VERSION)
    {
      return false;
    }
  if (tag == RecentlyPlayedGame::d_tag)
    {
      RecentlyPlayedGame *g = RecentlyPlayedGame::handle_load(helper);
      push_back(g);
      return true;
    }
  return false;
}

void RecentlyPlayedGameList::addNetworkedEntry(GameScenario *game_scenario, Profile *p, Glib::ustring host, guint32 port)
{
  if (Configuration::s_remember_recent_games == false)
    return;
  RecentlyPlayedNetworkedGame *g = NULL;
  switch (GameScenario::PlayMode(game_scenario->getPlayMode()))
    {
      case GameScenario::NETWORKED:
	g = new RecentlyPlayedNetworkedGame(game_scenario, p);
	g->fillData(host, port);
	break;
      default:
	break;
    }
  if (g)
    push_back(g);
  sort(orderByTime);
}

void RecentlyPlayedGameList::addEntry(GameScenario *game_scenario, Profile *p, 
                                      Glib::ustring filename)
{
  if (Configuration::s_remember_recent_games == false)
    return;
  switch (GameScenario::PlayMode(game_scenario->getPlayMode()))
    {
      case GameScenario::HOTSEAT:
	  {
	    RecentlyPlayedHotseatGame *g = NULL;
	    g = new RecentlyPlayedHotseatGame(game_scenario, p);
	    g->fillData(filename);
	    push_back(g);
	    break;
	  }
      default:
	break;
    }
}

bool RecentlyPlayedGameList::orderByTime(RecentlyPlayedGame*rhs, RecentlyPlayedGame *lhs)
{
  if (rhs->getTimeOfLastPlay().as_double() > lhs->getTimeOfLastPlay().as_double())
    return true;
  else
    return false;
}

void RecentlyPlayedGameList::pruneGames(int max_number_of_games)
{
  sort(orderByTime);
  pruneGamesBelongingToRemovedProfiles();
  pruneSameNamedAndSameHostGames();
  pruneOldGames(TWO_WEEKS_OLD);
  pruneTooManyGames(max_number_of_games);
}

void RecentlyPlayedGameList::pruneGamesBelongingToRemovedProfiles()
{
  for (RecentlyPlayedGameList::iterator it = begin(); it != end(); ++it)
    {
      Profile *p = 
        Profilelist::getInstance()->findProfileById((*it)->getProfileId());
      if (!p)
        {
          delete *it;
          it = erase (it);
        }
    }
}

void RecentlyPlayedGameList::pruneTooManyGames(int too_many)
{
  int count = 0;
  for (RecentlyPlayedGameList::iterator it = begin(); it != end();)
    {
      count++;
      if (count > too_many)
	{
	  delete *it;
	  it = erase (it);
	  continue;
	}
      ++it;
    }
}

void RecentlyPlayedGameList::pruneSameNamedAndSameHostGames()
{
  for (RecentlyPlayedGameList::iterator it = begin(); it != end();)
    {
      int count = 0;
      for (RecentlyPlayedGameList::iterator rit = begin(); rit != end(); ++rit)
        {
          if ((*it)->getPlayMode() == GameScenario::NETWORKED &&
              (*rit)->getPlayMode() == GameScenario::NETWORKED)
            {
              RecentlyPlayedNetworkedGame *i = 
                dynamic_cast<RecentlyPlayedNetworkedGame*>((*it));
              RecentlyPlayedNetworkedGame *j = 
                dynamic_cast<RecentlyPlayedNetworkedGame*>((*rit));
              if (j->getHost() == i->getHost() &&
                  j->getPort() == i->getPort() && count > 0)
                {
                  count++;
                  break;
                }
              else if (j->getHost() == i->getHost() &&
                       j->getPort() == i->getPort() && count == 0)
                count++;
            }
        }
      if (count > 1)
        {
          delete *it;
          it = erase (it);
          continue;
        }
      ++it;
    }
}

void RecentlyPlayedGameList::pruneOldGames(int stale)
{
  Glib::TimeVal now;
  now.assign_current_time();
  for (RecentlyPlayedGameList::iterator it = begin(); it != end();)
    {
      if ((*it)->getTimeOfLastPlay().as_double() + stale < now.as_double())
	{
	  delete *it;
	  it = erase (it);
	  continue;
	}
      ++it;
    }
}

void RecentlyPlayedGameList::updateEntry(GameScenario *game_scenario)
{
  for (RecentlyPlayedGameList::iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getId() == game_scenario->getId())
	{
          Glib::TimeVal now;
          now.assign_current_time();
	  (*it)->setTimeOfLastPlay(now);
	  (*it)->setRound(game_scenario->getRound());
	}
    }
}
	
void RecentlyPlayedGameList::removeAllNetworkedGames()
{
  for (RecentlyPlayedGameList::iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getPlayMode() == GameScenario::NETWORKED)
	{
	  erase (it);
	  delete *it;
	  it = begin();
	  continue;
	}
    }
}

bool RecentlyPlayedGameList::load()
{
  return loadFromFile(File::getSaveFile(RECENTLY_PLAYED_LIST));
}

bool RecentlyPlayedGameList::save() const
{
  return saveToFile(File::getSaveFile(RECENTLY_PLAYED_LIST));
}

bool RecentlyPlayedGameList::upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version)
{
  return FileCompat::getInstance()->upgrade(filename, old_version, new_version,
                                            FileCompat::RECENTLYPLAYEDGAMELIST, 
                                            d_tag);
}

void RecentlyPlayedGameList::support_backward_compatibility()
{
  FileCompat::getInstance()->support_type
    (FileCompat::RECENTLYPLAYEDGAMELIST, 
     File::get_extension(File::getUserRecentlyPlayedGamesDescription()), d_tag, 
     false);
  FileCompat::getInstance()->support_version
    (FileCompat::RECENTLYPLAYEDGAMELIST, "0.2.0", 
     LORDSAWAR_RECENTLY_PLAYED_VERSION,
     sigc::ptr_fun(&RecentlyPlayedGameList::upgrade));
}
// End of file
