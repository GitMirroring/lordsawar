//  Copyright (C) 2011, 2014, 2026 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>

#include "xml-helper.h"
#include "game-list.h"
#include "hosted-game.h"
#include <limits.h>
#include <fstream>
#include <iostream>
#include "configuration.h"
#include "defs.h"
#include "profile.h"
#include "profile-list.h"
#include "file-compat.h"
#include "advertised-game.h"
#include "recently-played-game-list.h"
#include "recently-played-game.h"
#include "file.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Glib::ustring Gamelist::d_tag = "gamelist";

Gamelist* Gamelist::s_instance = 0;

Gamelist* Gamelist::instance()
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
      helper.register_tag(HostedGame::d_tag, sigc::mem_fun(*this, &Gamelist::load_tag));
      bool retval = helper.parse_XML();
      helper.close();
      if (retval == false)
	File::erase(filename);
      return retval;
    }
  return true;
}

Gamelist* Gamelist::instance(XML_Helper* helper)
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
  helper->register_tag(HostedGame::d_tag, sigc::mem_fun(*this, &Gamelist::load_tag));
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
  retval &= helper->open_tag(Gamelist::d_tag);

  for (const_iterator it = begin(); it != end(); ++it)
    (*it)->save(helper);

  retval &= helper->close_tag();

  return retval;
}

bool Gamelist::load_tag(Glib::ustring tag, XML_Helper* helper)
{
  if (helper->get_version() != LORDSAWAR_RECENTLY_HOSTED_VERSION)
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
  if (rhs->getAdvertisedGame()->getTimeOfLastPlay().to_unix () > lhs->getAdvertisedGame()->getTimeOfLastPlay().to_unix())
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
  Glib::DateTime now = Glib::DateTime::create_now_local();
  for (Gamelist::iterator it = begin(); it != end();)
    {
      if ((*it)->getAdvertisedGame()->getTimeOfLastPlay().to_unix() + stale < now.to_unix())
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
          Glib::DateTime now = Glib::DateTime::create_now_local();
	  (*it)->getAdvertisedGame()->setTimeOfLastPlay(now);
	  (*it)->getAdvertisedGame()->setRound(round);
	}
    }
}
	
bool Gamelist::load()
{
  return loadFromFile(File::getUserRecentlyHostedGamesDescription());
}

bool Gamelist::save() const
{
  return saveToFile(File::getUserRecentlyHostedGamesDescription());
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
  Glib::DateTime now = Glib::DateTime::create_now_local();
  for (iterator i = begin(); i != end(); ++i)
    {
      AdvertisedGame *a = (*i)->getAdvertisedGame();
      if (a->getGameLastPingedOn().to_unix() + stale < now.to_unix())
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
  return FileCompat::instance()->upgrade(filename, old_version, new_version,
                                            FileCompat::GAMELIST, 
                                            d_tag);
}

void Gamelist::support_backward_compatibility()
{
  FileCompat::instance()->support_type
    (FileCompat::GAMELIST, 
     File::get_extension(File::getUserRecentlyHostedGamesDescription()), d_tag, 
     false);
  FileCompat::instance()->support_type
    (FileCompat::GAMELIST, 
     File::get_extension(File::getUserRecentlyAdvertisedGamesDescription()), 
     d_tag, false);
  FileCompat::instance()->support_version
    (FileCompat::GAMELIST, "0.2.0", LORDSAWAR_RECENTLY_HOSTED_VERSION,
     sigc::ptr_fun(&Gamelist::upgrade));
}
