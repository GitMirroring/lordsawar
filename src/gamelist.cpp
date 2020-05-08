// Copyright (C) 2011, 2014 Ben Asselstine
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

#include "xmlhelper.h"
#include "gamelist.h"
#include "hosted-game.h"
#include <limits.h>
#include <fstream>
#include <iostream>
#include "Configuration.h"
#include "defs.h"
#include "profile.h"
#include "profilelist.h"
#include "file-compat.h"
#include "advertised-game.h"
#include "recently-played-game-list.h"
#include "recently-played-game.h"
#include "File.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Glib::ustring Gamelist::d_tag = "gamelist";

Gamelist* Gamelist::s_instance = 0;

Gamelist* Gamelist::getInstance()
{
  if (s_instance == 0)
    s_instance = new Gamelist();

  return s_instance;
}

bool Gamelist::saveToFile(Glib::ustring filename) const
{
  bool retval = true;
  XML_Helper helper(filename, std::ios::out);
  retval &= save(&helper);
  helper.close();
  return retval;
}

bool Gamelist::loadFromFile(Glib::ustring filename)
{
  remove_all();
  std::ifstream in(filename.c_str());
  if (in)
    {
      XML_Helper helper(filename.c_str(), std::ios::in);
      helper.registerTag(HostedGame::d_tag, sigc::mem_fun(this, &Gamelist::load_tag));
      bool retval = helper.parseXML();
      helper.close();
      if (retval == false)
	File::erase(filename);
      return retval;
    }
  return true;
}

Gamelist* Gamelist::getInstance(XML_Helper* helper)
{
  if (s_instance)
    deleteInstance();

  s_instance = new Gamelist(helper);
  return s_instance;
}

void Gamelist::deleteInstance()
{
  if (s_instance)
    delete s_instance;

  s_instance = 0;
}

Gamelist::Gamelist()
{
}

Gamelist::Gamelist(XML_Helper* helper)
{
  helper->registerTag(HostedGame::d_tag, sigc::mem_fun(this, &Gamelist::load_tag));
}

void Gamelist::remove_all()
{
  for (Gamelist::iterator it = begin(); it != end(); ++it)
    delete *it;
  clear();
}

Gamelist::~Gamelist()
{
  remove_all();
}

bool Gamelist::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->begin(LORDSAWAR_RECENTLY_HOSTED_VERSION);
  retval &= helper->openTag(Gamelist::d_tag);

  for (const_iterator it = begin(); it != end(); ++it)
    (*it)->save(helper);

  retval &= helper->closeTag();

  return retval;
}

bool Gamelist::load_tag(Glib::ustring tag, XML_Helper* helper)
{
  if (helper->getVersion() != LORDSAWAR_RECENTLY_HOSTED_VERSION)
    {
      return false;
    }
  if (tag == HostedGame::d_tag)
    {
      HostedGame *g = new HostedGame(helper);
      push_back(g);
      return true;
    }
  return false;
}

void Gamelist::addEntry(AdvertisedGame *advertised_game)
{
  HostedGame *g = NULL;
  g = new HostedGame(advertised_game);
  if (g)
    push_back(g);
  sort(orderByTime);
}

bool Gamelist::orderByTime(HostedGame*rhs, HostedGame *lhs)
{
  if (rhs->getAdvertisedGame()->getTimeOfLastPlay().as_double() > lhs->getAdvertisedGame()->getTimeOfLastPlay().as_double())
    return true;
  else
    return false;
}

void Gamelist::pruneGames()
{
  sort(orderByTime);
  pruneOldGames(TEN_DAYS_OLD);
  pruneUnresponsiveGames();
  pruneTooManyGames(100);
}

void Gamelist::pruneTooManyGames(int too_many)
{
  int count = 0;
  for (Gamelist::iterator it = begin(); it != end();)
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

void Gamelist::pruneOldGames(int stale)
{
  Glib::TimeVal now;
  now.assign_current_time();
  for (Gamelist::iterator it = begin(); it != end();)
    {
      if ((*it)->getAdvertisedGame()->getTimeOfLastPlay().as_double() + stale < now.as_double())
	{
	  delete *it;
	  it = erase (it);
	  continue;
	}
      ++it;
    }
}

void Gamelist::updateEntry(Glib::ustring scenario_id, guint32 round)
{
  for (Gamelist::iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getAdvertisedGame()->getId() == scenario_id)
	{
          Glib::TimeVal now;
          now.assign_current_time();
	  (*it)->getAdvertisedGame()->setTimeOfLastPlay(now);
	  (*it)->getAdvertisedGame()->setRound(round);
	}
    }
}
	
bool Gamelist::load()
{
  return loadFromFile(File::getSaveFile(RECENTLY_HOSTED_LIST));
}

bool Gamelist::save() const
{
  return saveToFile(File::getSaveFile(RECENTLY_HOSTED_LIST));
}

RecentlyPlayedGameList* Gamelist::getList(bool scrub_profile_id) const
{
  RecentlyPlayedGameList *l = new RecentlyPlayedGameList();
  for (Gamelist::const_iterator i = begin(); i != end(); ++i)
    {
      if ((*i)->getUnresponsive())
        continue;
      RecentlyPlayedNetworkedGame *g = 
        new RecentlyPlayedNetworkedGame(*(*i)->getAdvertisedGame());
      if (scrub_profile_id)
        g->clearProfileId();
      l->push_back (g);
    }
  l->pruneGames(100);
  return l;
}
  
HostedGame *Gamelist::findGameByScenarioId(Glib::ustring scenario_id) const
{
  for (Gamelist::const_iterator i = begin(); i != end(); ++i)
    {
      if ((*i)->getAdvertisedGame()->getId() == scenario_id)
        return *i;
    }
  return NULL;
}

bool Gamelist::add(HostedGame *g)
{
  if (size() >= (guint32) MAX_NUMBER_OF_ADVERTISED_GAMES && 
      MAX_NUMBER_OF_ADVERTISED_GAMES != -1)
    return false;
  push_back(g);
  return true;
}

void Gamelist::pingGames()
{
  double stale = (double) FIVE_MINUTES_OLD;
  Glib::TimeVal now;
  now.assign_current_time();
  for (iterator i = begin(); i != end(); ++i)
    {
      AdvertisedGame *a = (*i)->getAdvertisedGame();
      if (a->getGameLastPingedOn().as_double() + stale < now.as_double())
        {
          (*i)->cannot_ping_game.connect
            (sigc::mem_fun(*this, &Gamelist::on_could_not_ping_game));
          (*i)->ping();
        }
    }
}

void Gamelist::pruneUnresponsiveGames()
{
  for (iterator i = begin(); i != end(); ++i)
    {
      if ((*i)->getUnresponsive())
        {
          delete *i;
          i = erase (i);
        }
    }
}

void Gamelist::on_could_not_ping_game(HostedGame *game)
{
  game->setUnresponsive(true);
}

bool Gamelist::upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version)
{
  return FileCompat::getInstance()->upgrade(filename, old_version, new_version,
                                            FileCompat::GAMELIST, 
                                            d_tag);
}

void Gamelist::support_backward_compatibility()
{
  FileCompat::getInstance()->support_type
    (FileCompat::GAMELIST, 
     File::get_extension(File::getUserRecentlyHostedGamesDescription()), d_tag, 
     false);
  FileCompat::getInstance()->support_type
    (FileCompat::GAMELIST, 
     File::get_extension(File::getUserRecentlyAdvertisedGamesDescription()), 
     d_tag, false);
  FileCompat::getInstance()->support_version
    (FileCompat::GAMELIST, "0.2.0", LORDSAWAR_RECENTLY_HOSTED_VERSION,
     sigc::ptr_fun(&Gamelist::upgrade));
}
// End of file
