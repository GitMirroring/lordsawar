//  Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
//  Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
//  Copyright (C) 2004, 2006 Andrea Paternesi
//  Copyright (C) 2006, 2007, 2008, 2010, 2011, 2014, 2015, 2017, 2020, 2021,
//  2026 Ben Asselstine
//  Copyright (C) 2007, 2008 Ole Laursen
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

#include <config.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <errno.h>
#include <sigc++/functors/mem_fun.h>
#include <string.h>

#include "ucompose.hpp"
#include "game-scenario.h"
#include "map-generator.h"
#include "player-list.h"
#include "fog-map.h"
#include "city-list.h"
#include "ruin-list.h"
#include "sight-map.h"
#include "reward-list.h"
#include "temple-list.h"
#include "bridge-list.h"
#include "port-list.h"
#include "road-list.h"
#include "stone-list.h"
#include "signpost-list.h"
#include "city.h"
#include "ruin.h"
#include "file.h"
#include "army-set-list.h"
#include "tile-set-list.h"
#include "city-set-list.h"
#include "shield-set-list.h"
#include "stack-list.h"
#include "stack.h"
#include "game-map.h"
#include "player.h"
#include "configuration.h"
#include "real-player.h"
#include "ai-dummy.h"
#include "ai-diplomacy.h"
#include "ai-analysis.h"
#include "ai-fast.h"
#include "counter.h"
#include "army.h"
#include "quest-manager.h"
#include "item-list.h"
#include "vectored-unit-list.h"
#include "history.h"
#include "xml-helper.h"
#include "tar-helper.h"
#include "stack-tile.h"
#include "file-compat.h"
#include "item.h"
#include "rnd.h"
#include "game-action-list.h"
#include "scenario-media.h"
#include "hero-templates.h"
#include "hero-proto.h"
#include "character.h"
#include "keeper.h"
#include "create-scenario.h"
#include "create-scenario-randomize.h"

Glib::ustring GameScenario::d_tag = "scenario";
Glib::ustring GameScenario::d_top_tag = PACKAGE;

sigc::signal<void(double)> GameScenario::load_tick;
sigc::signal<void(GameScenario *)> GameScenario::load_finished;

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

GameScenario::GameScenario(Glib::ustring name,Glib::ustring comment,
			   GameScenario::PlayMode playmode)
 : TarFile("", name, MAP_EXT), d_name(name),d_comment(comment), d_copyright(""),
    d_license(""), d_playmode(playmode), inhibit_autosave_removal(false),
    loaded_game_filename(""), d_unique (true)
{
    Armysetlist::instance();
    Tilesetlist::instance();
    Shieldsetlist::instance();

    if (id_counter == 0)
      id_counter = new ID_Counter();
    setNewRandomId();
}

//savegame has an absolute path
GameScenario::GameScenario(Glib::ustring savegame, bool& broken, Glib::ustring &err)
 : TarFile (File::get_dirname (savegame), File::get_basename (savegame, false),
            File::get_extension (savegame)),
    d_playmode(GameScenario::HOTSEAT), inhibit_autosave_removal(false),
    loaded_game_filename(""), d_unique (true)
{
  err = "";
  load_tick.emit (0.03);
  Tar_Helper t (savegame, std::ios::in, broken);
  load_tick.emit (0.06);
  if (broken == false)
    {
      bool success;
      loaded_game_filename = savegame;
      success = loadArmysets (&t);
      if (!success && err == "")
        {
          err = String::ucompose (_("Couldn't load armyset in %1"),
                                  savegame);
          broken = true;
        }
      else
        {
          load_tick.emit (0.13);
          success = loadTilesets (&t);
          if (!success && err == "")
            {
              err = String::ucompose (_("Couldn't load tileset in %1"),
                                      savegame);
              broken = true;
            }
          else
            {
              load_tick.emit (0.26);
              success = loadCitysets (&t);
              if (!success && err == "")
                {
                  err = String::ucompose (_("Couldn't load cityset in %1"),
                                          savegame);
                  broken = true;
                }
              else
                {
                  load_tick.emit (0.39);
                  success = loadShieldsets (&t);
                  if (!success && err == "")
                    {
                      err =
                        String::ucompose (_("Couldn't load shieldset in %1"),
                                          savegame);
                      broken = true;
                    }
                  else
                    {
                      load_tick.emit (0.52);
                      std::list<std::string> ext;
                      ext.push_back (MAP_EXT);
                      ext.push_back (SAVE_EXT);
                      Glib::ustring filename = t.getFirstFile (ext, broken);
                      XML_Helper helper (filename, std::ios::in);
                      broken = loadWithHelper (helper);
                      if (broken && err == "")
                        err =
                          String::ucompose (_("Couldn't parse scenario in %1"),
                                            savegame);
                      else
                        {
                          load_tick.emit (0.65);
                          ScenarioMedia::instance ()->instantiateImages
                            (t, broken);
                          if (broken && err == "")
                            err =
                              String::ucompose
                              (_("Couldn't load scenario images in %1"),
                               savegame);
                          else
                            {
                              load_tick.emit (0.78);

                              ScenarioMedia::instance ()->copySounds
                                (t, broken);
                              if (broken && err == "")
                                err =
                                  String::ucompose
                                  (_("Couldn't load scenario sounds in %1"),
                                   savegame);
                              load_tick.emit (0.91);
                            }
                        }
                      helper.close ();
                      File::erase (filename);
                    }
                }
            }
        }
    }
  else
    {
      if (File::exists (savegame) && File::is_readonly (savegame))
        err = String::ucompose (_("Couldn't open %1 for reading"), savegame);
      else
        err = String::ucompose (_("Couldn't scan archive in %1, not a valid file"), savegame);
    }
                      
  if (broken)
    cleanup();
  load_tick.emit (1.0);
  load_finished.emit (this);
  t.Close ();
}

GameScenario::GameScenario (const GameScenario &g, bool unique)
 : GameScenarioOptions (g), TarFile (g), d_name (g.d_name),
    d_comment (g.d_comment), d_copyright (g.d_copyright),
    d_license (g.d_license), d_playmode (g.d_playmode), d_id (g.d_id),
    inhibit_autosave_removal (g.inhibit_autosave_removal),
    loaded_game_filename (g.loaded_game_filename), d_unique (unique)
{
}

bool GameScenario::loadArmysets(Tar_Helper *t)
{
  bool broken = false;
  std::list<std::string> armysets = t->getFilenames(Armyset::file_extension);
  for (std::list<std::string>::iterator it = armysets.begin();
       it != armysets.end(); ++it)
    {
      Armysetlist::instance()->signal_imported().connect
        ([](int id)
         {
           bool broke;
           Armysetlist::instance()->get(id)->instantiateImages(broke);
           return;
         });
      Armysetlist::instance()->import_file (t, *it, broken);
    }
  return !broken;
}

bool GameScenario::loadTilesets(Tar_Helper *t)
{
  bool broken = false;
  std::list<std::string> tilesets = t->getFilenames(Tileset::file_extension);
  for (auto it: tilesets)
    {
      Tilesetlist::instance()->signal_imported().connect
        ([](int id)
         {
           bool broke;
           Tilesetlist::instance()->get(id)->instantiateImages(broke);
           return;
         });
      Tilesetlist::instance()->import_file (t, it, broken);
    }
  return !broken;
}

bool GameScenario::loadCitysets(Tar_Helper *t)
{
  bool broken = false;
  std::list<std::string> citysets = t->getFilenames(Cityset::file_extension);
  for (auto it: citysets)
    {
      Citysetlist::instance()->signal_imported().connect
        ([](int id)
         {
           bool broke;
           Citysetlist::instance()->get(id)->instantiateImages(broke);
           return;
         });
      Citysetlist::instance()->import_file (t, it, broken);
    }
  return !broken;
}

bool GameScenario::loadShieldsets(Tar_Helper *t)
{
  bool broken = false;
  std::list<std::string> shieldsets =
    t->getFilenames(Shieldset::file_extension);
  for (auto it: shieldsets)
    {
      Shieldsetlist::instance()->signal_imported().connect
        ([](int id)
         {
           bool broke;
           Shieldsetlist::instance()->get(id)->instantiateImages(broke);
           return;
         });
      Shieldsetlist::instance()->import_file (t, it, broken);
    }
  return !broken;
}

void GameScenario::quickStartEvenlyDivided()
{
  Playerlist *plist = Playerlist::instance();
  Vector <int> pos;
  // no neutral cities
  // divvy up the neutral cities among other non-neutral players
  int cities_left = Citylist::instance()->size() - plist->size() + 1;
  unsigned int citycount[MAX_PLAYERS];
  memset (citycount, 0, sizeof (citycount));
  Playerlist::iterator pit = plist->begin();

  while (cities_left > 0)
    {
      if (*pit != plist->getNeutral())
	{
	  citycount[(*pit)->getId()]++;
	  cities_left--;
	}

      ++pit;
      if (pit == plist->end())
	pit = plist->begin();
    }

  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
      for (unsigned int j = 0; j < citycount[i]; j++)
	{
	  Player *p = plist->get (i);
	  if (!p)
	    continue;
	  if (p == plist->getNeutral())
	    continue;
	  pos = Citylist::instance()->getCapitalCity(p)->getPos();
	  City *c = Citylist::instance()->getNearestNeutralCity(pos);
	  if (c)
	    {
	      //does the city contain any stacks yet?
	      //change their allegience to us.
	      for (unsigned int x = 0 ; x < c->getSize(); x++)
		{
		  for (unsigned int y = 0; y < c->getSize(); y++)
		    {
		      StackTile *stile = 
			GameMap::getStacks(c->getPos() + Vector<int>(x,y));
		      std::vector<Stack*> stks = stile->getStacks();
		      for (std::vector<Stack *>::iterator k = stks.begin();
			   k != stks.end(); ++k)
			Stacklist::changeOwnership(*k, p);
		    }
		}

	      //now give the city to us.
              p->conquerCity(c, NULL);
	    }
	}
    }
}

void GameScenario::quickStartAIHeadStart()
{
  float head_start_factor = 0.05;
  //each AI player gets this percent of total cities.

  Playerlist *plist = Playerlist::instance();
  Vector <int> pos;

  unsigned int citycount = Citylist::instance()->size() * head_start_factor;
  if (citycount == 0)
    citycount = 1;
  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
      for (unsigned int j = 0; j < citycount; j++)
	{
	  Player *p = plist->get (i);
	  if (!p)
	    continue;
	  if (p == plist->getNeutral())
	    continue;
	  if (p->getType() == Player::HUMAN)
	    continue;
	  pos = Citylist::instance()->getCapitalCity(p)->getPos();
	  City *c = Citylist::instance()->getNearestNeutralCity(pos);
	  if (c)
	    {
	      //does the city contain any stacks yet?
	      //change their allegience to us.
	      for (unsigned int x = 0 ; x < c->getSize(); x++)
		{
		  for (unsigned int y = 0; y < c->getSize(); y++)
		    {
		      StackTile *stile = 
			GameMap::getStacks(c->getPos() + Vector<int>(x,y));
		      std::vector<Stack*> stks = stile->getStacks();
		      for (std::vector<Stack *>::iterator k = stks.begin();
			   k != stks.end(); ++k)
			Stacklist::changeOwnership(*k, p);
		    }
		}

	      //now give the city to us.
              p->conquerCity(c, NULL);
	    }
	}
    }
}

bool GameScenario::setupFog(bool hidden_map)
{
  for (auto it: *Playerlist::instance())
    {
      if (hidden_map)
	it->getFogMap()->fill(FogMap::CLOSED);
      else
	it->getFogMap()->fill(FogMap::OPEN);
    }
  return true;
}

bool GameScenario::setupStacks(bool hidden_map)
{
  if (!hidden_map)
    return true;
  for (Playerlist::iterator it = Playerlist::instance()->begin();
       it != Playerlist::instance()->end(); ++it)
    {
      if ((*it) == Playerlist::getNeutral())
	continue;
      for (Stacklist::iterator sit = (*it)->getStacklist()->begin();
	   sit != (*it)->getStacklist()->end(); ++sit)
	(*sit)->deFog();
    }
  return true;
}

bool GameScenario::setupMapRewards()
{
  debug("GameScenario::setupMapRewards")
  //okay, let's make some maps
  //split the terrain into a 3x3 grid
  Vector<int> step = Vector<int>(GameMap::getWidth() / 3, 
				 GameMap::getHeight() / 3);
  Reward_Map *reward = new Reward_Map(Vector<int>(step.x * 0, 0), 
				      _("Northwestern map"), step.x, step.y);
  Rewardlist::instance()->push_back(reward);
  reward = new Reward_Map(Vector<int>(step.x * 1, 0), 
			  _("Northern map"), step.x, step.y);
  Rewardlist::instance()->push_back(reward);
  reward = new Reward_Map(Vector<int>(step.x * 2, 0), 
			  _("Northeastern map"), step.x, step.y);
  Rewardlist::instance()->push_back(reward);
  reward = new Reward_Map(Vector<int>(step.x * 0, step.y * 1), 
			  _("Western map"), step.x, step.y);
  Rewardlist::instance()->push_back(reward);
  reward = new Reward_Map(Vector<int>(step.x * 1, step.y * 1), 
			  _("Central map"), step.x, step.y);
  Rewardlist::instance()->push_back(reward);
  reward = new Reward_Map(Vector<int>(step.x * 2, step.y * 1), 
			  _("Eastern map"), step.x, step.y);
  Rewardlist::instance()->push_back(reward);
  reward = new Reward_Map(Vector<int>(step.x * 0, step.y * 2), 
			  _("Southwestern map"), step.x, step.y);
  Rewardlist::instance()->push_back(reward);
  reward = new Reward_Map(Vector<int>(step.x * 1, step.y * 2), 
			  _("Southern map"), step.x, step.y);
  Rewardlist::instance()->push_back(reward);
  reward = new Reward_Map(Vector<int>(step.x * 2, step.y * 2), 
			  _("Southeastern map"), step.x, step.y);
  Rewardlist::instance()->push_back(reward);
  return true;
}

bool GameScenario::setupRuinRewards(int difficulty)
{
  debug("GameScenario::setupRuinRewards")

    //the more difficult the scenario is, the more likely we are to have
    //hidden ruins
    guint32 chance = 0;
    if (difficulty <= 70)
      chance = 1;
    else if (difficulty <= 80)
      chance = 2;
    else if (difficulty <= 90)
      chance = 3;
    else
      chance = 4;

    guint32 num_hidden = 0;
    for (auto i : *Ruinlist::instance ())
      if (i->isHidden ())
        num_hidden++;
    guint32 num_not_hidden = Ruinlist::instance ()->size () - num_hidden;
    // first, mark some ruins as hidden for rewards
    // only til we have as many hidden ruins as we have non-hidden ruins
    for (Ruinlist::iterator it = Ruinlist::instance()->begin();
         it != Ruinlist::instance()->end(); ++it)
      {
        if ((*it)->isHidden () == false && Rnd::rand() % 100 < chance &&
            (*it)->hasSage() == false && (*it)->getReward() == NULL &&
            num_hidden < num_not_hidden)
          {
            (*it)->setHidden (true);
            num_hidden++;
            num_not_hidden--;
          }
      }

  // now we populate the rewards
  for (Ruinlist::iterator it = Ruinlist::instance()->begin();
       it != Ruinlist::instance()->end(); ++it)
    {
      if ((*it)->isHidden() == true)
        {
          //add it to the reward list
          Reward_Ruin *newReward = new Reward_Ruin((*it)); //make a reward
          newReward->setName(newReward->generate_name ());
          Rewardlist::instance()->push_back(newReward); //add it
        }
      else
        {
          if ((*it)->hasSage() == false && (*it)->getReward() == NULL)
            (*it)->populateWithRandomReward();
        }
    }
  return true;
}

bool GameScenario::setupItemRewards()
{
  guint32 count = 0;
  debug("GameScenario::setupItemRewards")
  for (auto iter : *Itemlist::instance())
    {
      const ItemProto* templateItem = iter.second;
      Item *newItem = new Item(*templateItem, count); //instantiate it
      Reward_Item *newReward = new Reward_Item(newItem); //make a reward
      delete newItem;
      newReward->setName(newReward->generate_name());
      Rewardlist::instance()->push_back(newReward); //add it
      count++;
    }

  return true;
}

bool GameScenario::setupRewards(bool hidden_map, int difficulty)
{
  setupItemRewards();
  setupRuinRewards(difficulty);
  if (hidden_map)
    setupMapRewards();
  return true;
}

bool GameScenario::setupCities(GameParameters::QuickStartPolicy quick_start,
                               GameParameters::BuildProductionMode build)
{
  //non-random scenarios need this fixup
  for (auto p : *Playerlist::instance ())
    {
      if (p->getFirstCity () == NULL)
        {
          for (auto c : *Citylist::instance ())
            {
              if (c->isCapital () && c->getCapitalOwner () == p)
                p->conquerCity (c, NULL);
            }
        }
    }

  for (Playerlist::iterator it = Playerlist::instance()->begin();
       it != Playerlist::instance()->end(); ++it)
    {
      if ((*it) == Playerlist::getNeutral())
	continue;
      City *city = Citylist::instance()->getCapitalCity(*it);
      if (city)
        city->deFog(city->getOwner());
    }

  if (quick_start == GameParameters::EVENLY_DIVIDED)
    quickStartEvenlyDivided();
  else if (quick_start == GameParameters::AI_HEAD_START)
    quickStartAIHeadStart();

  for (Citylist::iterator it = Citylist::instance()->begin();
       it != Citylist::instance()->end(); ++it)
    {
      if ((*it)->isBurnt())
        continue;
      if ((*it)->getOwner() == Playerlist::getNeutral())
	{
	  switch (GameScenario::s_neutral_cities)
	    {
	    case GameParameters::AVERAGE:
              (*it)->produceWeakestProductionBase();
	      break;
	    case GameParameters::STRONG:
	      (*it)->produceStrongestProductionBase();
	      break;
	    case GameParameters::ACTIVE:
	      if (Rnd::rand () % 100 >  20)
		(*it)->produceStrongestProductionBase();
	      else
		(*it)->produceWeakestProductionBase();
	      break;
	    case GameParameters::DEFENSIVE:
	      (*it)->produceWeakestQuickestArmyInArmyset();
	      (*it)->produceWeakestQuickestArmyInArmyset();
	      break;
	    }
	  (*it)->setActiveProductionSlot(-1);
	}
      else
	{
	  if ((*it)->isCapital())
	    (*it)->produceStrongestProductionBase();
	  else
	    (*it)->produceWeakestProductionBase();

	  (*it)->setActiveProductionSlot(0);
	}
    }

  //set up build production
  std::list<City*> cities;
  for (Citylist::iterator it = Citylist::instance()->begin();
       it != Citylist::instance()->end(); ++it)
    {
      if ((*it)->isBurnt())
        continue;
      if ((*it)->isCapital())
        continue;
      if ((*it)->getBuildProduction() == true)
        continue;
      cities.push_back(*it);
    }
  switch (build)
    {
    case GameParameters::BUILD_PRODUCTION_ALWAYS:
      for (Citylist::iterator it = Citylist::instance()->begin();
           it != Citylist::instance()->end(); ++it)
        {
          if ((*it)->isBurnt())
            continue;
          (*it)->setBuildProduction(true);
        }
      break;
    case GameParameters::BUILD_PRODUCTION_USUALLY:
        {
          //usually means 66% have their build production turned on.
          int target = (double)Citylist::instance()->size() * 0.33;
          int to_turn_off = cities.size() - target;
          if (to_turn_off > 0)
            {
              for (Citylist::iterator it = Citylist::instance()->begin();
                   it != Citylist::instance()->end(); ++it)
                {
                  if ((*it)->isBurnt())
                    continue;
                  if ((*it)->isCapital())
                    continue;
                  if ((*it)->getBuildProduction() == false)
                    continue;
                  (*it)->setBuildProduction (false);
                  to_turn_off--;
                  if (to_turn_off == 0)
                    break;
                }
            }
        }
      break;
    case GameParameters::BUILD_PRODUCTION_SELDOM:
        {
          //seldom means 90% have their build production turned off.
          int target = (double)Citylist::instance()->size() * 0.90;
          int to_turn_off = target - cities.size ();
          if (to_turn_off > 0)
            {
              for (Citylist::iterator it = Citylist::instance()->begin();
                   it != Citylist::instance()->end(); ++it)
                {
                  if ((*it)->isBurnt())
                    continue;
                  if ((*it)->isCapital())
                    continue;
                  if ((*it)->getBuildProduction() == false)
                    continue;
                  (*it)->setBuildProduction (false);
                  to_turn_off--;
                  if (to_turn_off == 0)
                    break;
                }
            }
        }
      break;
    case GameParameters::BUILD_PRODUCTION_NEVER:
      for (Citylist::iterator it = Citylist::instance()->begin();
           it != Citylist::instance()->end(); ++it)
        {
          if ((*it)->isBurnt())
            continue;
          (*it)->setBuildProduction(false);
        }
      break;
    }

  return true;
}

void GameScenario::setupDiplomacy(bool diplomacy)
{
  for (auto pit: *Playerlist::instance())
    {
      if (Playerlist::getNeutral() == pit)
        continue;
      for (auto it: *Playerlist::instance())
        {
          if (Playerlist::getNeutral() == it)
            continue;
          if (pit == it)
            continue;
          if (diplomacy == false)
            {
              pit->proposeDiplomacy(Player::PROPOSE_WAR, it);
              pit->declareDiplomacy(Player::AT_WAR, it, false);
            }
          else 
            {
              pit->proposeDiplomacy(Player::NO_PROPOSAL, it);
              pit->declareDiplomacy(Player::AT_PEACE, it, false);
            }
        }
    }
    if (diplomacy)
      Playerlist::instance()->calculateDiplomaticRankings();
}

bool GameScenario::loadWithHelper(XML_Helper& helper)
{
  Armysetlist::instance();
  Tilesetlist::instance();
  Shieldsetlist::instance();

  bool broken = false;

  helper.register_tag(d_top_tag, sigc::mem_fun(*this, &GameScenario::load));
  helper.register_tag(d_tag, sigc::mem_fun(*this, &GameScenario::load));
  helper.register_tag(Itemlist::d_tag, sigc::mem_fun(*this, &GameScenario::load));
  helper.register_tag(Playerlist::d_tag, sigc::mem_fun(*this, &GameScenario::load));
  helper.register_tag(GameMap::d_tag, sigc::mem_fun(*this, &GameScenario::load));
  helper.register_tag(Citylist::d_tag, sigc::mem_fun(*this, &GameScenario::load));
  helper.register_tag(Templelist::d_tag, sigc::mem_fun(*this, &GameScenario::load));
  helper.register_tag(Ruinlist::d_tag, sigc::mem_fun(*this, &GameScenario::load));
  helper.register_tag(Rewardlist::d_tag, sigc::mem_fun(*this, &GameScenario::load));
  helper.register_tag(Signpostlist::d_tag, sigc::mem_fun(*this, &GameScenario::load));
  helper.register_tag(Roadlist::d_tag, sigc::mem_fun(*this, &GameScenario::load));
  helper.register_tag(Stonelist::d_tag, sigc::mem_fun(*this, &GameScenario::load));
  helper.register_tag(ID_Counter::d_tag, sigc::mem_fun(*this, &GameScenario::load));
  helper.register_tag(QuestsManager::d_tag, sigc::mem_fun(*this, &GameScenario::load));
  helper.register_tag(Bridgelist::d_tag, sigc::mem_fun(*this, &GameScenario::load));
  helper.register_tag(Portlist::d_tag, sigc::mem_fun(*this, &GameScenario::load));
  helper.register_tag(VectoredUnitlist::d_tag, sigc::mem_fun(*this, &GameScenario::load));
  helper.register_tag(GameActionlist::d_tag, sigc::mem_fun(*this, &GameScenario::load));
  helper.register_tag(ScenarioMedia::d_tag, sigc::mem_fun(*this, &GameScenario::load));
  helper.register_tag(HeroTemplates::d_tag, sigc::mem_fun(*this, &GameScenario::load));

  if (!helper.parse_XML())
    broken = true;

  if (!broken)
    {
      GameMap::instance()->updateStackPositions();
      GameMap::instance()->calculateBlockedAvenues();
      HeroTemplates::instance ()->populateHeroProtos ();
    }

  return broken;
}

GameScenario::~GameScenario()
{
  if (d_unique)
    cleanup();
  if (Configuration::s_autosave_policy == 1 && 
      inhibit_autosave_removal == false)
    {
      Glib::ustring filename = File::getSaveFile("autosave" + SAVE_EXT);
      File::erase(filename);
    }
  clean_tmp_dir();
} 

Glib::ustring GameScenario::getName() const
{
  return d_name;
}

Glib::ustring GameScenario::getComment() const
{
  return d_comment;
}

bool GameScenario::dump(Glib::ustring filename, Glib::ustring extension) const
{
  bool retval = true;
  Glib::ustring goodfilename = File::add_ext_if_necessary(filename, extension);
  debug("saving game to " + goodfilename);

  File::erase (goodfilename);

  Glib::ustring tmpfile = File::get_tmp_file();
  XML_Helper helper(tmpfile, std::ios::out);
  retval &= saveWithHelper(helper);
  helper.close();

  if (retval == false)
    return false;

  bool broken = false;
  Tar_Helper t(goodfilename, std::ios::out, broken);
  if (broken == true)
    return false;

  t.saveFile(tmpfile, File::get_basename(goodfilename, true));
  File::erase(tmpfile);

  Cityset *cs = GameMap::getCityset();
  t.saveFile(cs->getConfigurationFile());

  Shieldset *ss = GameMap::getShieldset();
  t.saveFile(ss->getConfigurationFile());

  Tileset *ts = GameMap::getTileset();
  t.saveFile(ts->getConfigurationFile());
 
  std::list<guint32> armysets;
  for (auto it: *Playerlist::instance())
    {
      guint32 armyset = it->getArmyset();
      if (std::find(armysets.begin(), armysets.end(), armyset) == armysets.end())
	armysets.push_back(armyset);
    }

  for (auto it: armysets)
    {
      Armyset *as = Armysetlist::instance()->get(it);
      t.saveFile(as->getConfigurationFile());
    }

  return true;
}

bool GameScenario::saveGame(Glib::ustring filename, Glib::ustring extension) const
{
  bool retval = true;
  Glib::ustring goodfilename = File::add_ext_if_necessary(filename, extension);
  debug("saving game to " + goodfilename);

  Glib::ustring tmpfile = File::get_tmp_file();
  XML_Helper helper(tmpfile, std::ios::out);
  retval &= saveWithHelper(helper);
  helper.close();

  if (retval == false)
    return false;

  Glib::ustring tmptar = File::get_tmp_file() + ".tar";
  retval = saveTar(tmpfile, tmptar, goodfilename, getSetFiles ());

  return retval;
}

std::vector<std::string> GameScenario::getSetFiles () const
{
  std::vector<std::string> sets;
  sets.push_back (GameMap::getTileset()->getConfigurationFile ());
  sets.push_back (GameMap::getCityset()->getConfigurationFile ());
  sets.push_back (GameMap::getShieldset()->getConfigurationFile ());

  std::vector<Armyset*> v;
  for (auto a : GameMap::getArmysets ())
    {
      if (a)
        v.push_back (a);
    }

  std::sort (v.begin (), v.end (),
    [](Armyset* a, Armyset* b)
    {
      return a->getId () < b->getId ();
    });

  v.erase (std::unique (v.begin (), v.end (),
    [](Armyset* a, Armyset* b) 
    {
      return a->getId () == b->getId ();
    }), v.end());

  for (auto a: v)
    sets.push_back (a->getConfigurationFile ());

  return sets;
}

bool GameScenario::saveWithHelper(XML_Helper &helper) const
{
  bool retval = true;

  //start writing
  retval &= helper.begin(LORDSAWAR_SAVEGAME_VERSION);
  retval &= helper.open_tag(d_top_tag);

  //if retval is still true it propably doesn't change throughout the rest
  //now save the single object's data
  retval &= id_counter->save(&helper);
  retval &= Itemlist::instance()->save(&helper);
  retval &= Playerlist::instance()->save(&helper);
  retval &= GameMap::instance()->save(&helper);
  retval &= Citylist::instance()->save(&helper);
  retval &= Templelist::instance()->save(&helper);
  retval &= Ruinlist::instance()->save(&helper);
  retval &= Rewardlist::instance()->save(&helper);
  retval &= Signpostlist::instance()->save(&helper);
  retval &= Roadlist::instance()->save(&helper);
  retval &= Stonelist::instance()->save(&helper);
  retval &= Portlist::instance()->save(&helper);
  retval &= Bridgelist::instance()->save(&helper);
  retval &= QuestsManager::instance()->save(&helper);
  retval &= VectoredUnitlist::instance()->save(&helper);
  retval &= GameActionlist::instance()->save(&helper);
  if (HeroTemplates::instance()->isDefault () == false)
    retval &= HeroTemplates::instance()->save(&helper);

  //save the private GameScenario data last due to dependencies
  retval &= helper.open_tag(GameScenario::d_tag);
  retval &= helper.save("id", d_id);
  retval &= helper.save("name", d_name);
  retval &= helper.save("comment", d_comment);
  retval &= helper.save("copyright", d_copyright);
  retval &= helper.save("license", d_license);
  retval &= helper.save("turn", s_round);
  retval &= helper.save("view_enemies", s_see_opponents_stacks);
  retval &= helper.save("view_production", s_see_opponents_production);
  Glib::ustring quest_policy_str = Configuration::questPolicyToString(GameParameters::QuestPolicy(s_play_with_quests));
  retval &= helper.save("quests", quest_policy_str);
  retval &= helper.save("hidden_map", s_hidden_map);
  retval &= helper.save("diplomacy", s_diplomacy);
  retval &= helper.save("cusp_of_war", s_cusp_of_war);
  Glib::ustring neutral_cities_str = Configuration::neutralCitiesToString(GameParameters::NeutralCities(s_neutral_cities));
  retval &= helper.save("neutral_cities", neutral_cities_str);
  Glib::ustring razing_cities_str = Configuration::razingCitiesToString(GameParameters::RazingCities(s_razing_cities));
  retval &= helper.save("razing_cities", razing_cities_str);
  Glib::ustring vectoring_mode_str = Configuration::vectoringModeToString(GameParameters::VectoringMode(s_vectoring_mode));
  retval &= helper.save("vectoring_mode", vectoring_mode_str);
  Glib::ustring build_prod_mode_str = Configuration::buildProductionModeToString(GameParameters::BuildProductionMode(s_build_production_mode));
  retval &= helper.save("build_production_mode", build_prod_mode_str);
  Glib::ustring sacking_mode_str = Configuration::sackingModeToString(GameParameters::SackingMode(s_sacking_mode));
  retval &= helper.save("sacking_mode", sacking_mode_str);
  retval &= helper.save("intense_combat", s_intense_combat);
  retval &= helper.save("military_advisor", s_military_advisor);
  retval &= helper.save("random_turns", s_random_turns);
  retval &= helper.save("cities_can_produce_allies", s_cities_can_produce_allies);
  retval &= helper.save("surrender_already_offered", 
			    s_surrender_already_offered);
  Glib::ustring playmode_str = playModeToString(GameScenario::PlayMode(d_playmode));
  retval &= helper.save("playmode", playmode_str);

  retval &= helper.close_tag();

  retval &= ScenarioMedia::instance()->save(&helper);

  retval &= helper.close_tag();

  return retval;
}

bool GameScenario::load(Glib::ustring tag, XML_Helper* helper)
{
  if (tag == d_top_tag)
    {
      if (helper->get_version() != LORDSAWAR_SAVEGAME_VERSION)
	{
          std::cerr << String::ucompose(_("saved game file has wrong version.  Expecting %1 but got %2."), LORDSAWAR_SAVEGAME_VERSION, helper->get_version()) << std::endl;
	  return false;
	}
      return true;
    }
  if (tag == GameScenario::d_tag)
    {
      debug("loading scenario")

      helper->get(d_id, "id");
      helper->get(d_name, "name");
      helper->get(d_comment, "comment");
      helper->get(d_copyright, "copyright");
      helper->get(d_license, "license");
      helper->get(s_round, "turn");
      helper->get(s_see_opponents_stacks, "view_enemies");
      helper->get(s_see_opponents_production, "view_production");
      Glib::ustring quest_policy_str;
      helper->get(quest_policy_str, "quests");
      s_play_with_quests = Configuration::questPolicyFromString(quest_policy_str);
      helper->get(s_hidden_map, "hidden_map");
      helper->get(s_diplomacy, "diplomacy");
      helper->get(s_cusp_of_war, "cusp_of_war");
      Glib::ustring neutral_cities_str;
      helper->get(neutral_cities_str, "neutral_cities");
      s_neutral_cities = Configuration::neutralCitiesFromString(neutral_cities_str);
      Glib::ustring razing_cities_str;
      helper->get(razing_cities_str, "razing_cities");
      s_razing_cities = Configuration::razingCitiesFromString(razing_cities_str);
      Glib::ustring vectoring_mode_str;
      helper->get(vectoring_mode_str, "vectoring_mode");
      s_vectoring_mode = Configuration::vectoringModeFromString(vectoring_mode_str);
      Glib::ustring build_prod_mode_str;
      helper->get(build_prod_mode_str, "build_production_mode");
      s_build_production_mode = Configuration::buildProductionModeFromString(build_prod_mode_str);
      Glib::ustring sacking_mode_str;
      helper->get(sacking_mode_str, "sacking_mode");
      s_sacking_mode = Configuration::sackingModeFromString(sacking_mode_str);
      helper->get(s_intense_combat, "intense_combat");
      helper->get(s_military_advisor, "military_advisor");
      helper->get(s_random_turns, "random_turns");
      helper->get(s_cities_can_produce_allies, "cities_can_produce_allies");
      helper->get(s_surrender_already_offered, 
		      "surrender_already_offered");
      Glib::ustring playmode_str;
      helper->get(playmode_str, "playmode");
      d_playmode = GameScenario::playModeFromString(playmode_str);

      return true;
    }
  
  if (tag == ID_Counter::d_tag)
    {
      debug("loading counter")
	id_counter = new ID_Counter(helper);
      return true;
    }

  if (tag == Itemlist::d_tag)
    {
      debug("loading items");
      Itemlist::instance(helper);
      return true;
    }

  if (tag == Playerlist::d_tag)
    {
      debug("loading players");
      Playerlist::instance(helper);
      return true;
    }

  if (tag == GameMap::d_tag)
    {
      debug("loading map")
	GameMap::instance(helper);
      return true;
    }

  if (tag == Citylist::d_tag)
    {
      debug("loading cities")

	Citylist::instance(helper);
      return true;
    }

  if (tag == Templelist::d_tag)
    {
      debug("loading temples")
	Templelist::instance(helper);
      return true;
    }

  if (tag == Ruinlist::d_tag)
    {
      debug("loading ruins")
	Ruinlist::instance(helper);
      return true;
    }

  if (tag == Rewardlist::d_tag)
    {
      debug("loading rewards")
	Rewardlist::instance(helper);
      return true;
    }

  if (tag == Signpostlist::d_tag)
    {
      debug("loading signposts")
	Signpostlist::instance(helper);
      return true;
    }

  if (tag == Roadlist::d_tag)
    {
      debug("loading roads")
	Roadlist::instance(helper);
      return true;
    }

  if (tag == Stonelist::d_tag)
    {
      debug("loading stones")
	Stonelist::instance(helper);
      return true;
    }

  if (tag == QuestsManager::d_tag)
    {
      debug("loading quests")
	QuestsManager::instance(helper);
      return true;
    }

  if (tag == VectoredUnitlist::d_tag)
    {
      debug("loading vectored units")
	VectoredUnitlist::instance(helper);
      return true;
    }

  if (tag == Portlist::d_tag)
    {
      debug("loading ports")
	Portlist::instance(helper);
      return true;
    }

  if (tag == Bridgelist::d_tag)
    {
      debug("loading bridges")
	Bridgelist::instance(helper);
      return true;
    }

  if (tag == GameActionlist::d_tag)
    {
      GameActionlist::instance(helper);
      return true;
    }

  if (tag == ScenarioMedia::d_tag)
    {
      ScenarioMedia::instance(helper);
      return true;
    }

  if (tag == HeroTemplates::d_tag)
    {
      HeroTemplates::instance(helper);
      return true;
    }
  return false;
}

bool GameScenario::autoSave()
{
  Glib::ustring filename = "";
  if (Configuration::s_autosave_policy == 2)
    filename = String::ucompose("autosave-%1%2", Glib::ustring::format(std::setfill(L'0'), std::setw(3), s_round - 1), SAVE_EXT);
  else if (Configuration::s_autosave_policy == 1)
    filename = "autosave" + SAVE_EXT;
  else
    return true;
  // autosave to the file "autosave.sav".
  //
  // We first save  to a temporary file, then rename it.
  // This avoids screwing up the autosave if something goes wrong
  // (and we have a savefile for debugging)
  //
  // We can be somewhat assured the rename works, because we are renaming
  // from ~/.cache/lordsawar/<file> to ~/.local/share/lordsawar/<file>
  //
  Glib::ustring tmpfile = File::get_tmp_file (SAVE_EXT);
  if (!saveGame(tmpfile))
    {
      std::cerr<< "Autosave failed, see " << tmpfile << std::endl;
      return false;
    }
  //erase the old autosave file if any, and then plop our new one in place.
  File::erase(File::getSaveFile(filename));
  if (File::rename(tmpfile, File::getSaveFile(filename)) == false)
    {
      Glib::ustring errmsg = Glib::strerror(errno);
      std::cerr << String::ucompose(_("Error! can't rename the temporary file `%1' to the autosave file `%2'.  %3"), tmpfile, File::getSaveFile(filename), errmsg) << std::endl;
      return false;
    }
  return true;
}

void GameScenario::nextRound()
{
  s_round++;
  autoSave();
}

Glib::ustring GameScenario::playModeToString(const GameScenario::PlayMode mode)
{
  switch (mode)
    {
      case GameScenario::HOTSEAT: return "GameScenario::HOTSEAT";
      case GameScenario::NETWORKED: return "GameScenario::NETWORKED";
    }
  return "GameScenario::HOTSEAT";
}

GameScenario::PlayMode GameScenario::playModeFromString(const Glib::ustring str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return GameScenario::PlayMode(atoi(str.c_str()));
  if (str == "GameScenario::HOTSEAT") return GameScenario::HOTSEAT;
  else if (str == "GameScenario::NETWORKED")
    return GameScenario::NETWORKED;
  return GameScenario::HOTSEAT;
}
	
void GameScenario::setNewRandomId()
{
  d_id = generate_guid();
}
	
bool GameScenario::validate(std::list<Glib::ustring> &errors, std::list<Glib::ustring> &warnings)
{
  std::stringstream ss;
  ss << std::endl;
  Glib::ustring newline = ss.str();

  Glib::ustring s;
  guint32 num = Playerlist::instance()->countPlayersAlive();
  if (num < 2)
    errors.push_back(_("There must be at least 2 players in the scenario."));

  num = Citylist::instance()->countCities();
  if (num < 2)
    errors.push_back(_("There must be at least 2 cities in the scenario."));

  Glib::ustring match = _("Untitled");
  if (getName ().find (match) == 0 ||
      String::utrim (getName ()).empty () == true)
    errors.push_back(_("The scenario does not have a name."));

  for (auto it: *Playerlist::instance())
    {
      if (it == Playerlist::getNeutral())
        {
          if (it->getHeroes().size ())
            errors.push_back(_("Neutrals have one or more heroes."));
          continue;
        }
      if (it->isDead() == true)
	continue;
      if (Citylist::instance()->getCapitalCity(it) == NULL ||
          Citylist::instance()->getCapitalCity(it)->isBurnt() == true)
	{
	  s = String::ucompose
	    (_("The player called `%1' lacks a capital city."), 
	     it->getName().c_str());
	  errors.push_back(s);
	  break;
	}
      std::vector<Character*> heroes =
        HeroTemplates::instance()->getHeroes (it->get_shield());
      Itemlist *il = Itemlist::instance ();
      for (auto h : heroes)
        {
          for (auto item : h->get_starting_item_ids ())
            {
              if (il->find (item) == il->end ())
                {
                  s = String::ucompose
                    (_("The hero type called `%1' belonging to '%2' has bad starting items."),
                     h->get_name (), Shield::colorToFriendlyName (h->get_shield ()));
                  errors.push_back(s);
                }
            }
        }
      guint32 num_heroes = heroes.size ();
      for (auto h : heroes)
        delete h;
      if (num_heroes == 0)
        {
	  s = String::ucompose (_("The player called `%1' lacks a hero."),
                                it->getName().c_str());
	  errors.push_back(s);
          break;
        }
      for (auto h : it->getHeroes())
        {
          HeroTemplates *templates = HeroTemplates::instance ();
          Character *c = templates->getCharacterById (h->getCharacterId ());
          if (c->get_shield () == Shield::NEUTRAL)
            continue;
          if (!c || c->get_shield () != it->get_shield ())
            {
              Vector<int> pos =
                it->getStacklist ()->getArmyStackById (h->getId ())->getPos ();
              s = String::ucompose
                (_("The hero `%1' belonging to '%2' has a bad hero type (%3,%4)."),
                 h->getName(), it->getName (), pos.x, pos.y);
              errors.push_back(s);
            }
        }
    }

  std::vector<Vector<int> >unnamed_city_pos;
  for (auto it: *Citylist::instance())
    if (it->isUnnamed() == true)
      unnamed_city_pos.push_back (it->getPos ());
  if (unnamed_city_pos.size () > 0)
    {
      s = String::ucompose
        (ngettext("There is %1 unnamed city", "There are %1 unnamed cities",
                  unnamed_city_pos.size ()), unnamed_city_pos.size ());
      for (guint32 i = 0; i < unnamed_city_pos.size (); i++)
        {
          if (i > 2)
            break;
          if (unnamed_city_pos.size () == 1)
            s += String::ucompose (_(" (at %1,%2)"), unnamed_city_pos[i].x,
                                   unnamed_city_pos[i].y);
          else
            {
              s+= newline;
              s += String::ucompose (_("An unnamed city is at %1,%2"),
                                     unnamed_city_pos[i].x,
                                     unnamed_city_pos[i].y);
            }
        }
      warnings.push_back(s);
    }

  std::vector<Vector<int> >unnamed_ruin_pos;
  for (auto it: *Ruinlist::instance())
    if (it->isUnnamed() == true)
      unnamed_ruin_pos.push_back (it->getPos ());
  if (unnamed_ruin_pos.size () > 0)
    {
      s = String::ucompose(ngettext("There is %1 unnamed ruin", "There are %1 unnamed ruins", unnamed_ruin_pos.size ()), unnamed_ruin_pos.size ());
      for (guint32 i = 0; i < unnamed_ruin_pos.size (); i++)
        {
          if (i > 2)
            break;
          if (unnamed_ruin_pos.size () == 1)
            s += String::ucompose (_(" (at %1,%2)"), unnamed_ruin_pos[i].x,
                                   unnamed_ruin_pos[i].y);
          else
            {
              s+= newline;
              s += String::ucompose (_("An unnamed ruin is at %1,%2"),
                                     unnamed_ruin_pos[i].x,
                                     unnamed_ruin_pos[i].y);
            }
        }
      warnings.push_back(s);
    }

  for (auto it: *Ruinlist::instance())
    {
      if (it->getOccupant () && it->getOccupant ()->getName () == "" &&
          it->getOccupant()->getStack ())
        {
          s = String::ucompose("%1 has an unnamed keeper", it->getName ());
          errors.push_back(s);
        }
      if (it->getReward ())
        {
          auto reward = it->getReward ();
          if (reward->getType () == Reward::RUIN)
            {
              auto ruin_reward = dynamic_cast<Reward_Ruin*> (reward);
              if (ruin_reward->getRuin () == NULL)
                {
                  s = String::ucompose("The ruin reward in %1 isn't specified", it->getName ());
                  errors.push_back(s);
                }
            }

        }
    }

  std::vector<Vector<int> >unnamed_temple_pos;
  for (auto it: *Templelist::instance())
    {
      if (it->isUnnamed() == true)
        unnamed_temple_pos.push_back (it->getPos ());
    }
  if (unnamed_temple_pos.size () > 0)
    {
      s = String::ucompose(ngettext("There is %1 unnamed temple", "There are %1 unnamed temples", unnamed_temple_pos.size ()), unnamed_temple_pos.size ());
      for (guint32 i = 0; i < unnamed_temple_pos.size (); i++)
        {
          if (i > 2)
            break;
          if (unnamed_temple_pos.size () == 1)
            s += String::ucompose (_(" (at %1,%2)"), unnamed_temple_pos[i].x,
                                   unnamed_temple_pos[i].y);
          else
            {
              s+= newline;
              s += String::ucompose (_("An unnamed temple is at %1,%2"),
                                     unnamed_temple_pos[i].x,
                                     unnamed_temple_pos[i].y);
            }
        }
      warnings.push_back(s);
    }

  guint32 count = 0;
  for (auto it: *Playerlist::getNeutral()->getStacklist())
    {
      if (Citylist::instance()->getObjectAt(it->getPos()) == NULL)
	count++;
    }
  if (count > 0)
    {
      s = String::ucompose(ngettext("There is %1 neutral stack not in a city", "There are %1 neutral stacks not in cities", count), count);
      warnings.push_back(s);
    }

      
  GameMap::instance()->calculateBlockedAvenues();
  if (GameMap::instance()->checkCityAccessibility() == false)
    errors.push_back(_("Not all cities are reachable by a non-flying unit."));

  //any ports or bridges on land?
  if (GameMap::checkBuildingTerrain(Maptile::PORT, true))
    errors.push_back(_("One or more ports are on land."));
  if (GameMap::checkBuildingTerrain(Maptile::BRIDGE, true))
    errors.push_back(_("One or more bridges are on land."));
  //any cities, roads, temples, ruins, signs on water?
  if (GameMap::checkBuildingTerrain(Maptile::CITY, false))
    errors.push_back(_("One or more cities are on water."));
  if (GameMap::checkBuildingTerrain(Maptile::ROAD, false))
    errors.push_back(_("One or more roads are on water."));
  if (GameMap::checkBuildingTerrain(Maptile::RUIN, false))
    errors.push_back(_("One or more ruins are on water."));
  if (GameMap::checkBuildingTerrain(Maptile::TEMPLE, false))
    errors.push_back(_("One or more temples are on water."));
  if (GameMap::checkBuildingTerrain(Maptile::SIGNPOST, false))
    errors.push_back(_("One or more signs are on water."));
  
  for (auto it: *Itemlist::instance())
    {
      ItemProto *i = it.second;
      if (i->getBonus (ItemProto::BANISH_WORMS) &&
          i->hasArmyTypeToKill () == false)
        errors.push_back(String::ucompose (_("The item \"%1\" doesn't have an army type specified for Kill All Units Of Giant Worms"), i->getName ()));

      if (i->getBonus (ItemProto::SUMMON_MONSTER) &&
          i->hasArmyTypeToSummon () == false)
        errors.push_back(String::ucompose (_("The item \"%1\" doesn't have an army type specified for Summon Monster"), i->getName ()));

      if (i->getBonus (ItemProto::RAISE_DEFENDERS) &&
          i->hasArmyTypeToRaise () == false)
        errors.push_back(String::ucompose (_("The item \"%1\" doesn't have an army type specified for Raise Defenders In City"), i->getName ()));

      if (i->getBonus () == 0)
        warnings.push_back (String::ucompose (_("The item \"%1\" lacks a bonus"), i->getName ()));
    }

  if (errors.size() ==  0)
    return true;
  return false;
}

void GameScenario::setupRuins ()
{
  debug("GameScenario::setupRuins")
  for (auto r : *Ruinlist::instance ())
    {
      if (r->getOccupant () == NULL)
        {
          const ArmyProto *a = Keeper::randomRuinDefender ();
          Keeper *keeper = new Keeper (a, r->getPos ());
          r->setOccupant (keeper);
        }
    }
  return;
}

void GameScenario::initialize(GameParameters g)
{
  Playerlist::instance()->clearAllActions();
  setupFog(g.hidden_map);
  setupCities(g.quick_start, g.build_production_mode);
  setupRuins ();
  setupStacks(g.hidden_map);
  setupRewards(g.hidden_map, g.difficulty);
  setupDiplomacy(g.diplomacy);
  if (s_random_turns)
    Playerlist::instance()->randomizeOrder();
  if (d_playmode == GameScenario::NETWORKED)
    {
      GameMap::instance()->clearStackPositions();
      Playerlist::instance()->turnHumansIntoNetworkPlayers();
    }
  else
    autoSave();
  GameMap::instance()->updateStackPositions();

  if (d_name == "AutoGenerated")
    {
      if (GameMap::instance()->checkCityAccessibility() == false)
	exit (0);
    }
  // the neutral player starts on round 0 and being the last in the list causes
  // round 1 to start when it completes its first turn.
  //
  // this has the negative side effect of giving the neutral player a turn
  // before anyone else.  it's not usually a problem but when they're active
  // or defensive they can produce an extra army unit.
  // so we take special care in the dummy (neutral) player to avoid doing
  // anything when round is zero.
  Playerlist::instance ()->setActiveplayer (Playerlist::getNeutral ());
}

//! Grabs the game option information out of a scenario file.
class ParamLoader
{
public:
    ParamLoader(Glib::ustring filename, bool &broken) {
      Tar_Helper t(filename, std::ios::in, broken);
      if (broken)
        return;
      std::list<std::string> ext;
      ext.push_back(MAP_EXT);
      ext.push_back(SAVE_EXT);
      std::string tmpfile = t.getFirstFile(ext, broken);
      XML_Helper helper(tmpfile, std::ios::in);
      helper.register_tag(GameMap::d_tag, 
			 sigc::mem_fun(*this, &ParamLoader::loadParam));
      helper.register_tag(GameScenario::d_tag, 
			 sigc::mem_fun(*this, &ParamLoader::loadParam));
      helper.register_tag(Playerlist::d_tag, 
			 sigc::mem_fun(*this, &ParamLoader::loadParam));
      helper.register_tag(Player::d_tag, 
			 sigc::mem_fun(*this, &ParamLoader::loadParam));
      bool retval = helper.parse_XML();
      helper.close();
      File::erase(tmpfile);
      if (broken == false)
	broken = !retval;
    }
    bool loadParam(Glib::ustring tag, XML_Helper* helper)
      {
	if (tag == Playerlist::d_tag)
	  {
	    helper->get(d_neutral, "neutral");
	    return true;
	  }
	if (tag == Player::d_tag)
	  {
            int armyset_id;
	    int type;
	    int id;
	    Glib::ustring name;
	    GameParameters::Player p;
	    helper->get(id, "id");
	    p.id = id;
	    helper->get(type, "type");
	    switch (Player::Type(type))
	      {
	      case Player::HUMAN: 
		p.type = GameParameters::Player::HUMAN;
		break;
	      case Player::AI_FAST: 
		p.type = GameParameters::Player::EASY;
		break;
	      case Player::AI_DUMMY: 
		p.type = GameParameters::Player::EASY;
		break;
	      case Player::AI_SMART: 
		p.type = GameParameters::Player::HARD;
		break;
	      case Player::NETWORKED: 
		p.type = GameParameters::Player::HUMAN;
		break;
	      }
	    helper->get(name, "name");
	    p.name = name;

            helper->get(armyset_id, "armyset");
            Armyset *armyset = Armysetlist::instance()->get(armyset_id);
            game_params.army_theme[p.id] = armyset->getBaseName();
            if (p.id != d_neutral)
              game_params.players.push_back(p);

	    return true;
	  }
	if (tag == GameMap::d_tag)
	  {
	    helper->get(game_params.shield_theme, "shieldset");
	    helper->get(game_params.tile_theme, "tileset");
	    helper->get(game_params.city_theme, "cityset");
	    return true;
	  }
	if (tag == GameScenario::d_tag)
	  {
	    helper->get(game_params.name, "name");
	    helper->get(game_params.comment, "comment");
	    helper->get(game_params.see_opponents_stacks, 
			    "view_enemies");
	    helper->get(game_params.see_opponents_production, 
			    "view_production");
	    Glib::ustring quest_policy_str;
	    helper->get(quest_policy_str, "quests");
	    game_params.play_with_quests = 
	      Configuration::questPolicyFromString(quest_policy_str);
	    helper->get(game_params.hidden_map, "hidden_map");
	    helper->get(game_params.diplomacy, "diplomacy");
	    helper->get(game_params.cusp_of_war, "cusp_of_war");
	    Glib::ustring neutral_cities_str;
	    helper->get(neutral_cities_str, "neutral_cities");
	    game_params.neutral_cities = 
	      Configuration::neutralCitiesFromString(neutral_cities_str);
	    Glib::ustring razing_cities_str;
	    helper->get(razing_cities_str, "razing_cities");
	    game_params.razing_cities = 
	      Configuration::razingCitiesFromString(razing_cities_str);
	    Glib::ustring vectoring_mode_str;
	    helper->get(vectoring_mode_str, "vectoring_mode");
	    game_params.vectoring_mode = 
	      Configuration::vectoringModeFromString(vectoring_mode_str);
	    helper->get(game_params.intense_combat, 
			    "intense_combat");
	    helper->get(game_params.military_advisor, 
			    "military_advisor");
	    helper->get(game_params.random_turns, "random_turns");
	    return true;
	  }
	return false;
      };
    GameParameters game_params = {};
    guint32 d_neutral;
};

GameParameters GameScenario::loadGameParameters(Glib::ustring filename, bool &broken)
{
  ParamLoader loader(filename, broken);
  
  return loader.game_params;
}

//! Grab the type of game from a saved-game file.  Either networked or hotseat.
class PlayModeLoader
{
public:
    PlayModeLoader(Glib::ustring filename, bool &broken) {
      play_mode = GameScenario::HOTSEAT;
      Tar_Helper t(filename, std::ios::in, broken);
      if (broken)
        return;
      Glib::ustring file = File::get_basename(filename, true);
      std::list<std::string> ext;
      ext.push_back(MAP_EXT);
      ext.push_back(SAVE_EXT);
      std::string tmpfile = t.getFirstFile(ext, broken);
      if (tmpfile == "")
        {
          broken = true;
          return;
        }
      XML_Helper helper(tmpfile, std::ios::in);
      helper.register_tag(GameScenario::d_tag, 
			 sigc::mem_fun(*this, &PlayModeLoader::loadParam));
      bool retval = helper.parse_XML();
      helper.close();
      File::erase(tmpfile);
      if (broken == false)
	broken = !retval;
    }
    bool loadParam(Glib::ustring tag, XML_Helper* helper)
      {
	if (tag == GameScenario::d_tag)
	  {
	    Glib::ustring playmode_str;
	    helper->get(playmode_str, "playmode");
	    play_mode = GameScenario::playModeFromString(playmode_str);
	    return true;
	  }
	return false;
      };
    GameScenario::PlayMode play_mode;
};

GameScenario::PlayMode GameScenario::loadPlayMode(Glib::ustring filename, bool &broken)
{
  PlayModeLoader loader(filename, broken);
  if (broken)
    return HOTSEAT;
  return loader.play_mode;
}

//! Read in some basic information about a scenario from a scenario file.
class DetailsLoader
{
public:

    DetailsLoader(Glib::ustring filename, bool &broken) 
      : name(""), comment (""), player_count (0), city_count (0), id ("")
      {
        Tar_Helper tar(filename, std::ios::in, broken);
        if (broken)
          return;
        std::list<std::string> ext;
        ext.push_back(MAP_EXT);
        ext.push_back(SAVE_EXT);
        std::string tmpfile = tar.getFirstFile(ext, broken);
        XML_Helper helper(tmpfile, std::ios::in);
        helper.register_tag(GameScenario::d_tag, 
                           sigc::mem_fun(*this, &DetailsLoader::loadDetails));
        helper.register_tag(Player::d_tag, 
                           sigc::mem_fun(*this, &DetailsLoader::loadDetails));
        helper.register_tag(City::d_tag, 
                           sigc::mem_fun(*this, &DetailsLoader::loadDetails));
        bool retval = helper.parse_XML();
        helper.close();
        File::erase(tmpfile);
        if (!broken)
          broken = !retval;
      }

    bool loadDetails(Glib::ustring tag, XML_Helper* helper)
      {
	if (tag == GameScenario::d_tag)
	  {
	    helper->get(name, "name");
	    helper->get(comment, "comment");
	    helper->get(id, "id");
	    return true;
	  }
	if (tag == Player::d_tag)
	  {
	    player_count++;
	    return true;
	  }
	if (tag == City::d_tag)
	  {
	    city_count++;
	    return true;
	  }
	return false;
      };
    Tar_Helper *t;
    Glib::ustring name, comment;
    guint32 player_count, city_count;
    Glib::ustring id;
};

void GameScenario::loadDetails(Glib::ustring filename, bool &broken, guint32 &player_count, guint32 &city_count, Glib::ustring &name, Glib::ustring &comment, Glib::ustring &id)
{
  DetailsLoader loader(filename, broken);
  if (broken == false)
    {
      player_count = loader.player_count;
      city_count = loader.city_count;
      name = loader.name;
      comment = loader.comment;
      id = loader.id;
    }
  return;
}

void GameScenario::clean_tmp_dir() const
{
  if (loaded_game_filename != "")
    Tar_Helper::clean_tmp_dir(loaded_game_filename);
}

Glib::ustring GameScenario::generate_guid()
{
  char buf[40];
  //this is a very poor guid generator.
  guint32 num[11];
  num[0] = Rnd::rand ();
  num[1] = Rnd::rand () % 4096;
  num[2] = Rnd::rand () % 4096;
  num[3] = Rnd::rand () % 256;
  num[4] = Rnd::rand () % 256;
  num[5] = Rnd::rand () % 256;
  num[6] = Rnd::rand () % 256;
  num[7] = Rnd::rand () % 256;
  num[8] = Rnd::rand () % 256;
  num[9] = Rnd::rand () % 256;
  num[10] = Rnd::rand () % 256;

  snprintf (buf, sizeof (buf), "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", num[0], num[1], num[2], num[3], num[4], num[5], num[6], num[7], num[8], num[9], num[10]);

  return Glib::ustring(buf);
}

void GameScenario::cleanup()
{
  Itemlist::deleteInstance();
  Playerlist::deleteInstance();
  Citylist::deleteInstance();
  Templelist::deleteInstance();
  Ruinlist::deleteInstance();
  Rewardlist::deleteInstance();
  Signpostlist::deleteInstance();
  Portlist::deleteInstance();
  Bridgelist::deleteInstance();
  Roadlist::deleteInstance();
  Stonelist::deleteInstance();
  QuestsManager::deleteInstance();
  VectoredUnitlist::deleteInstance();
  GameMap::deleteInstance();
  GameActionlist::deleteInstance();
  ScenarioMedia::deleteInstance();
  HeroTemplates::deleteInstance ();
  if (id_counter)
    {
      delete id_counter;
      id_counter = 0;
    }
  GameScenarioOptions::s_round = 0;
}

bool GameScenario::upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version)
{
  return FileCompat::instance()->upgrade(filename, old_version, new_version,
                                            FileCompat::GAMESCENARIO, 
                                            d_top_tag);
}

void GameScenario::support_backward_compatibility()
{
  FileCompat::instance()->support_type (FileCompat::GAMESCENARIO, MAP_EXT, 
                                           d_top_tag, true);
  FileCompat::instance()->support_type (FileCompat::GAMESCENARIO, SAVE_EXT, 
                                           d_top_tag, true);
  FileCompat::instance()->support_version
    (FileCompat::GAMESCENARIO, "0.2.0", "0.2.1",
     sigc::ptr_fun(&GameScenario::upgrade));
  FileCompat::instance()->support_version
    (FileCompat::GAMESCENARIO, "0.2.1", "0.3.2",
     sigc::ptr_fun(&GameScenario::upgrade));
  FileCompat::instance()->support_version
    (FileCompat::GAMESCENARIO, "0.3.2", "0.3.3",
     sigc::ptr_fun(&GameScenario::upgrade));
  FileCompat::instance()->support_version
    (FileCompat::GAMESCENARIO, "0.3.3", "0.4.0",
     sigc::ptr_fun(&GameScenario::upgrade));
}

void GameScenario::create_and_dump (const std::string file,
                                    const GameParameters &g,
                                    sigc::slot<void(double)> *progress,
                                    sigc::slot<void()> finish)
{
  /*
   * this check here of file != "" is there so that
   * creator is cleaned up before finish () is called.
   */
  if (file != "")
    {
      CreateScenario creator (g.map.width, g.map.height);

      for (std::vector<GameParameters::Player>::const_iterator
           i = g.players.begin (), end = g.players.end ();
           i != end; ++i)
        {
          Player::Type type;
          if (i->type == GameParameters::Player::EASY)
            type = Player::AI_FAST;
          else if (i->type == GameParameters::Player::HARD)
            type = Player::AI_SMART;
          else
            type = Player::HUMAN;

          int army_id = 
            Armysetlist::instance ()->get (g.army_theme[i->id])->getId ();
          creator.addPlayer (i->name, army_id, Shield::Color (i->id), type);
        }

      int army_id = 
        Armysetlist::instance ()->get (g.army_theme[MAX_PLAYERS])->getId ();
      CreateScenarioRandomize random;

      creator.addNeutral (random.getPlayerName (Shield::NEUTRAL), army_id, 
                          Player::AI_DUMMY);

      creator.setMapTiles (g.tile_theme);
      creator.setShieldset (g.shield_theme);
      creator.setCityset (g.city_theme);
      creator.setNoCities (g.map.cities);
      creator.setNoRuins (g.map.ruins);
      creator.setNoTemples (g.map.temples);
      int num_signposts = g.map.signposts;
      if (num_signposts == -1)
        num_signposts = CreateScenario::calculateNumberOfSignposts
          (g.map.width, g.map.height, g.map.grass);
      creator.setNoSignposts (num_signposts);

      creator.setPercentages (g.map.grass, g.map.water, g.map.forest,
                              g.map.swamp, g.map.hills, g.map.mountains);

      if (progress)
        creator.progress.connect (*progress);

      creator.create (g);
      creator.dump (file);
      random.cleanup ();
    }
  finish ();
  return;
}
