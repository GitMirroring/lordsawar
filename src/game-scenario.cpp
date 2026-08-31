// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2010, 2011, 2014, 2015, 2017, 2020,
// 2021 Ben Asselstine
// Copyright (C) 2007, 2008 Ole Laursen
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

#include <config.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <errno.h>
#include <sigc++/functors/mem_fun.h>
#include <string.h>

#include "ucompose.hpp"
#include "GameScenario.h"
#include "MapGenerator.h"
#include "playerlist.h"
#include "FogMap.h"
#include "citylist.h"
#include "ruinlist.h"
#include "SightMap.h"
#include "rewardlist.h"
#include "templelist.h"
#include "bridgelist.h"
#include "portlist.h"
#include "roadlist.h"
#include "stonelist.h"
#include "signpostlist.h"
#include "city.h"
#include "ruin.h"
#include "File.h"
#include "armysetlist.h"
#include "tilesetlist.h"
#include "citysetlist.h"
#include "shieldsetlist.h"
#include "stacklist.h"
#include "stack.h"
#include "GameMap.h"
#include "player.h"
#include "Configuration.h"
#include "real_player.h"
#include "ai_dummy.h"
#include "AI_Diplomacy.h"
#include "AI_Analysis.h"
#include "ai_fast.h"
#include "counter.h"
#include "army.h"
#include "QuestsManager.h"
#include "Itemlist.h"
#include "vectoredunitlist.h"
#include "history.h"
#include "xmlhelper.h"
#include "tarhelper.h"
#include "stacktile.h"
#include "file-compat.h"
#include "Item.h"
#include "rnd.h"
#include "game-actionlist.h"
#include "ScenarioMedia.h"
#include "herotemplates.h"
#include "heroproto.h"
#include "character.h"
#include "keeper.h"

Glib::ustring GameScenario::d_tag = "scenario";
Glib::ustring GameScenario::d_top_tag = PACKAGE;

sigc::signal<void> GameScenario::load_tick;
sigc::signal<void> GameScenario::load_finish;

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

GameScenario::GameScenario(Glib::ustring name,Glib::ustring comment,
			   GameScenario::PlayMode playmode)
 : TarFile("", name, MAP_EXT), d_name(name),d_comment(comment), d_copyright(""),
    d_license(""), d_playmode(playmode), inhibit_autosave_removal(false),
    loaded_game_filename(""), d_unique (true)
{
    Armysetlist::getInstance();
    Tilesetlist::getInstance();
    Shieldsetlist::getInstance();

    if (fl_counter == 0)
      fl_counter = new FL_Counter();

    setNewRandomId();
}

//savegame has an absolute path
GameScenario::GameScenario(Glib::ustring savegame, bool& broken)
 : TarFile (File::get_dirname (savegame), File::get_basename (savegame, false),
            File::get_extension (savegame)),
    d_playmode(GameScenario::HOTSEAT), inhibit_autosave_removal(false),
    loaded_game_filename(""), d_unique (true)
{
  Tar_Helper t(savegame, std::ios::in, broken);
  load_tick.emit ();
  if (broken == false)
    {
      loaded_game_filename = savegame;
      loadArmysets(&t);
      load_tick.emit ();
      loadTilesets(&t);
      load_tick.emit ();
      loadCitysets(&t);
      load_tick.emit ();
      loadShieldsets(&t);
      load_tick.emit ();
      std::list<Glib::ustring> ext;
      ext.push_back(MAP_EXT);
      ext.push_back(SAVE_EXT);
      Glib::ustring filename = t.getFirstFile(ext, broken);
      XML_Helper helper(filename, std::ios::in);
      broken = loadWithHelper(helper);
      load_tick.emit ();
      ScenarioMedia::getInstance()->instantiateImages(t, broken);
      load_tick.emit ();
      ScenarioMedia::getInstance()->copySounds(t, broken);
      load_tick.emit ();
      helper.close();
      File::erase(filename);
      t.Close();
      if (broken)
        cleanup();
      load_finish.emit();
    }
  else
    {
      t.Close();
      cleanup();
    }
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
  std::list<Glib::ustring> armysets = t->getFilenames(Armyset::file_extension);
  for (std::list<Glib::ustring>::iterator it = armysets.begin();
       it != armysets.end(); ++it)
    {
      guint32 id = Armysetlist::getInstance()->import(t, *it, broken);
      if (!broken)
        Armysetlist::getInstance()->get(id)->instantiateImages(true, broken);
    }
  return !broken;
}

bool GameScenario::loadTilesets(Tar_Helper *t)
{
  bool broken = false;
  std::list<Glib::ustring> tilesets = t->getFilenames(Tileset::file_extension);
  for (auto it: tilesets)
    {
      guint32 id = Tilesetlist::getInstance()->import(t, it, broken);
      if (!broken)
        Tilesetlist::getInstance()->get(id)->instantiateImages(true, broken);
    }
  return !broken;
}

bool GameScenario::loadCitysets(Tar_Helper *t)
{
  bool broken = false;
  std::list<Glib::ustring> citysets = t->getFilenames(Cityset::file_extension);
  for (auto it: citysets)
    {
      guint32 id = Citysetlist::getInstance()->import(t, it, broken);
      if (!broken)
        Citysetlist::getInstance()->get(id)->instantiateImages(true, broken);
    }
  return !broken;
}

bool GameScenario::loadShieldsets(Tar_Helper *t)
{
  bool broken = false;
  std::list<Glib::ustring> shieldsets =
    t->getFilenames(Shieldset::file_extension);
  for (auto it: shieldsets)
    {
      guint32 id = Shieldsetlist::getInstance()->import(t, it, broken);
      if (!broken)
        Shieldsetlist::getInstance()->get(id)->instantiateImages(true, broken);
    }
  return !broken;
}

void GameScenario::quickStartEvenlyDivided()
{
  Playerlist *plist = Playerlist::getInstance();
  Vector <int> pos;
  // no neutral cities
  // divvy up the neutral cities among other non-neutral players
  int cities_left = Citylist::getInstance()->size() - plist->size() + 1;
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
	  Player *p = plist->getPlayer(i);
	  if (!p)
	    continue;
	  if (p == plist->getNeutral())
	    continue;
	  pos = Citylist::getInstance()->getCapitalCity(p)->getPos();
	  City *c = Citylist::getInstance()->getNearestNeutralCity(pos);
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

  Playerlist *plist = Playerlist::getInstance();
  Vector <int> pos;

  unsigned int citycount = Citylist::getInstance()->size() * head_start_factor;
  if (citycount == 0)
    citycount = 1;
  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
      for (unsigned int j = 0; j < citycount; j++)
	{
	  Player *p = plist->getPlayer(i);
	  if (!p)
	    continue;
	  if (p == plist->getNeutral())
	    continue;
	  if (p->getType() == Player::HUMAN)
	    continue;
	  pos = Citylist::getInstance()->getCapitalCity(p)->getPos();
	  City *c = Citylist::getInstance()->getNearestNeutralCity(pos);
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
  for (auto it: *Playerlist::getInstance())
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
  for (Playerlist::iterator it = Playerlist::getInstance()->begin();
       it != Playerlist::getInstance()->end(); ++it)
    {
      if ((*it) == Playerlist::getInstance()->getNeutral())
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
  Rewardlist::getInstance()->push_back(reward);
  reward = new Reward_Map(Vector<int>(step.x * 1, 0), 
			  _("Northern map"), step.x, step.y);
  Rewardlist::getInstance()->push_back(reward);
  reward = new Reward_Map(Vector<int>(step.x * 2, 0), 
			  _("Northeastern map"), step.x, step.y);
  Rewardlist::getInstance()->push_back(reward);
  reward = new Reward_Map(Vector<int>(step.x * 0, step.y * 1), 
			  _("Western map"), step.x, step.y);
  Rewardlist::getInstance()->push_back(reward);
  reward = new Reward_Map(Vector<int>(step.x * 1, step.y * 1), 
			  _("Central map"), step.x, step.y);
  Rewardlist::getInstance()->push_back(reward);
  reward = new Reward_Map(Vector<int>(step.x * 2, step.y * 1), 
			  _("Eastern map"), step.x, step.y);
  Rewardlist::getInstance()->push_back(reward);
  reward = new Reward_Map(Vector<int>(step.x * 0, step.y * 2), 
			  _("Southwestern map"), step.x, step.y);
  Rewardlist::getInstance()->push_back(reward);
  reward = new Reward_Map(Vector<int>(step.x * 1, step.y * 2), 
			  _("Southern map"), step.x, step.y);
  Rewardlist::getInstance()->push_back(reward);
  reward = new Reward_Map(Vector<int>(step.x * 2, step.y * 2), 
			  _("Southeastern map"), step.x, step.y);
  Rewardlist::getInstance()->push_back(reward);
  return true;
}

bool GameScenario::setupRuinOccupants()
{
  debug("GameScenario::setupRuinOccupants")
    for (Ruinlist::iterator it = Ruinlist::getInstance()->begin();
	 it != Ruinlist::getInstance()->end(); ++it)
      {
        if ((*it)->getOccupant () == NULL)
          {
            const ArmyProto *a = Keeper::randomRuinDefender ();
            Keeper *keeper = new Keeper (a, (*it)->getPos ());
            (*it)->setOccupant (keeper);
          }
      }
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
    for (auto i : *Ruinlist::getInstance ())
      if (i->isHidden ())
        num_hidden++;
    guint32 num_not_hidden = Ruinlist::getInstance ()->size () - num_hidden;
    // first, mark some ruins as hidden for rewards
    // only til we have as many hidden ruins as we have non-hidden ruins
    for (Ruinlist::iterator it = Ruinlist::getInstance()->begin();
         it != Ruinlist::getInstance()->end(); ++it)
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
  for (Ruinlist::iterator it = Ruinlist::getInstance()->begin();
       it != Ruinlist::getInstance()->end(); ++it)
    {
      if ((*it)->isHidden() == true)
        {
          //add it to the reward list
          Reward_Ruin *newReward = new Reward_Ruin((*it)); //make a reward
          newReward->setName(newReward->getDescription());
          Rewardlist::getInstance()->push_back(newReward); //add it
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
  for (auto iter : *Itemlist::getInstance())
    {
      const ItemProto* templateItem = iter.second;
      Item *newItem = new Item(*templateItem, count); //instantiate it
      Reward_Item *newReward = new Reward_Item(newItem); //make a reward
      delete newItem;
      newReward->setName(newReward->getDescription());
      Rewardlist::getInstance()->push_back(newReward); //add it
      count++;
    }

  return true;
}

bool GameScenario::setupRewards(bool hidden_map, int difficulty)
{
  if (Rewardlist::getInstance()->size() != 0)
    return true;
  setupItemRewards();
  setupRuinOccupants ();
  setupRuinRewards(difficulty);
  if (hidden_map)
    setupMapRewards();
  return true;
}

bool GameScenario::setupCities(GameParameters::QuickStartPolicy quick_start,
                               GameParameters::BuildProductionMode build)
{
  debug("GameScenario::setupCities")

  for (Playerlist::iterator it = Playerlist::getInstance()->begin();
       it != Playerlist::getInstance()->end(); ++it)
    {
      if ((*it) == Playerlist::getInstance()->getNeutral())
	continue;
      City *city = Citylist::getInstance()->getCapitalCity(*it);
      if (city)
	{
	  city->deFog(city->getOwner());
          (*it)->conquerCity(city, NULL);
	}
    }

  if (quick_start == GameParameters::EVENLY_DIVIDED)
    quickStartEvenlyDivided();
  else if (quick_start == GameParameters::AI_HEAD_START)
    quickStartAIHeadStart();

  for (Citylist::iterator it = Citylist::getInstance()->begin();
       it != Citylist::getInstance()->end(); ++it)
    {
      if ((*it)->isBurnt())
        continue;
      if ((*it)->getOwner() == Playerlist::getInstance()->getNeutral())
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
  for (Citylist::iterator it = Citylist::getInstance()->begin();
       it != Citylist::getInstance()->end(); ++it)
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
      for (Citylist::iterator it = Citylist::getInstance()->begin();
           it != Citylist::getInstance()->end(); ++it)
        {
          if ((*it)->isBurnt())
            continue;
          (*it)->setBuildProduction(true);
        }
      break;
    case GameParameters::BUILD_PRODUCTION_USUALLY:
        {
          //usually means 66% have their build production turned on.
          int target = (double)Citylist::getInstance()->size() * 0.33;
          int to_turn_off = cities.size() - target;
          if (to_turn_off > 0)
            {
              for (Citylist::iterator it = Citylist::getInstance()->begin();
                   it != Citylist::getInstance()->end(); ++it)
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
          int target = (double)Citylist::getInstance()->size() * 0.90;
          int to_turn_off = target - cities.size ();
          if (to_turn_off > 0)
            {
              for (Citylist::iterator it = Citylist::getInstance()->begin();
                   it != Citylist::getInstance()->end(); ++it)
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
      for (Citylist::iterator it = Citylist::getInstance()->begin();
           it != Citylist::getInstance()->end(); ++it)
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
  for (auto pit: *Playerlist::getInstance())
    {
      if (Playerlist::getInstance()->getNeutral() == pit)
        continue;
      for (auto it: *Playerlist::getInstance())
        {
          if (Playerlist::getInstance()->getNeutral() == it)
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
      Playerlist::getInstance()->calculateDiplomaticRankings();
}

bool GameScenario::loadWithHelper(XML_Helper& helper)
{
  Armysetlist::getInstance();
  Tilesetlist::getInstance();
  Shieldsetlist::getInstance();

  bool broken = false;

  helper.registerTag(d_top_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(Itemlist::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(Playerlist::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(GameMap::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(Citylist::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(Templelist::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(Ruinlist::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(Rewardlist::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(Signpostlist::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(Roadlist::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(Stonelist::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(FL_Counter::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(QuestsManager::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(Bridgelist::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(Portlist::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(VectoredUnitlist::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(GameActionlist::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(ScenarioMedia::d_tag, sigc::mem_fun(this, &GameScenario::load));
  helper.registerTag(HeroTemplates::d_tag, sigc::mem_fun(this, &GameScenario::load));

  if (!helper.parseXML())
    broken = true;

  if (!broken)
    {
      GameMap::getInstance()->updateStackPositions();
      GameMap::getInstance()->calculateBlockedAvenues();
      HeroTemplates::getInstance ()->populateHeroProtos ();
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
  for (auto it: *Playerlist::getInstance())
    {
      guint32 armyset = it->getArmyset();
      if (std::find(armysets.begin(), armysets.end(), armyset) == armysets.end())
	armysets.push_back(armyset);
    }

  for (auto it: armysets)
    {
      Armyset *as = Armysetlist::getInstance()->get(it);
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

std::vector<Glib::ustring> GameScenario::getSetFiles () const
{
  std::vector<Glib::ustring> sets;
  sets.push_back (GameMap::getTileset()->getConfigurationFile ());
  sets.push_back (GameMap::getCityset()->getConfigurationFile ());
  sets.push_back (GameMap::getShieldset()->getConfigurationFile ());

  for (auto a : GameMap::getArmysets ())
    sets.push_back (a->getConfigurationFile ());
  return sets;
}

bool GameScenario::saveWithHelper(XML_Helper &helper) const
{
  bool retval = true;

  //start writing
  retval &= helper.begin(LORDSAWAR_SAVEGAME_VERSION);
  retval &= helper.openTag(d_top_tag);

  //if retval is still true it propably doesn't change throughout the rest
  //now save the single object's data
  retval &= fl_counter->save(&helper);
  retval &= Itemlist::getInstance()->save(&helper);
  retval &= Playerlist::getInstance()->save(&helper);
  retval &= GameMap::getInstance()->save(&helper);
  retval &= Citylist::getInstance()->save(&helper);
  retval &= Templelist::getInstance()->save(&helper);
  retval &= Ruinlist::getInstance()->save(&helper);
  retval &= Rewardlist::getInstance()->save(&helper);
  retval &= Signpostlist::getInstance()->save(&helper);
  retval &= Roadlist::getInstance()->save(&helper);
  retval &= Stonelist::getInstance()->save(&helper);
  retval &= Portlist::getInstance()->save(&helper);
  retval &= Bridgelist::getInstance()->save(&helper);
  retval &= QuestsManager::getInstance()->save(&helper);
  retval &= VectoredUnitlist::getInstance()->save(&helper);
  retval &= GameActionlist::getInstance()->save(&helper);
  if (HeroTemplates::getInstance()->isDefault () == false)
    retval &= HeroTemplates::getInstance()->save(&helper);

  //save the private GameScenario data last due to dependencies
  retval &= helper.openTag(GameScenario::d_tag);
  retval &= helper.saveData("id", d_id);
  retval &= helper.saveData("name", d_name);
  retval &= helper.saveData("comment", d_comment);
  retval &= helper.saveData("copyright", d_copyright);
  retval &= helper.saveData("license", d_license);
  retval &= helper.saveData("turn", s_round);
  retval &= helper.saveData("view_enemies", s_see_opponents_stacks);
  retval &= helper.saveData("view_production", s_see_opponents_production);
  Glib::ustring quest_policy_str = Configuration::questPolicyToString(GameParameters::QuestPolicy(s_play_with_quests));
  retval &= helper.saveData("quests", quest_policy_str);
  retval &= helper.saveData("hidden_map", s_hidden_map);
  retval &= helper.saveData("diplomacy", s_diplomacy);
  retval &= helper.saveData("cusp_of_war", s_cusp_of_war);
  Glib::ustring neutral_cities_str = Configuration::neutralCitiesToString(GameParameters::NeutralCities(s_neutral_cities));
  retval &= helper.saveData("neutral_cities", neutral_cities_str);
  Glib::ustring razing_cities_str = Configuration::razingCitiesToString(GameParameters::RazingCities(s_razing_cities));
  retval &= helper.saveData("razing_cities", razing_cities_str);
  Glib::ustring vectoring_mode_str = Configuration::vectoringModeToString(GameParameters::VectoringMode(s_vectoring_mode));
  retval &= helper.saveData("vectoring_mode", vectoring_mode_str);
  Glib::ustring build_prod_mode_str = Configuration::buildProductionModeToString(GameParameters::BuildProductionMode(s_build_production_mode));
  retval &= helper.saveData("build_production_mode", build_prod_mode_str);
  Glib::ustring sacking_mode_str = Configuration::sackingModeToString(GameParameters::SackingMode(s_sacking_mode));
  retval &= helper.saveData("sacking_mode", sacking_mode_str);
  retval &= helper.saveData("intense_combat", s_intense_combat);
  retval &= helper.saveData("military_advisor", s_military_advisor);
  retval &= helper.saveData("random_turns", s_random_turns);
  retval &= helper.saveData("surrender_already_offered", 
			    s_surrender_already_offered);
  Glib::ustring playmode_str = playModeToString(GameScenario::PlayMode(d_playmode));
  retval &= helper.saveData("playmode", playmode_str);

  retval &= helper.closeTag();

  retval &= ScenarioMedia::getInstance()->save(&helper);

  retval &= helper.closeTag();

  return retval;
}

bool GameScenario::load(Glib::ustring tag, XML_Helper* helper)
{
  if (tag == d_top_tag)
    {
      if (helper->getVersion() != LORDSAWAR_SAVEGAME_VERSION)
	{
          std::cerr << String::ucompose(_("saved game file has wrong version.  Expecting %1 but got %2."), LORDSAWAR_SAVEGAME_VERSION, helper->getVersion()) << std::endl;
	  return false;
	}
      return true;
    }
  if (tag == GameScenario::d_tag)
    {
      debug("loading scenario")

      helper->getData(d_id, "id");
      helper->getData(d_name, "name");
      helper->getData(d_comment, "comment");
      helper->getData(d_copyright, "copyright");
      helper->getData(d_license, "license");
      helper->getData(s_round, "turn");
      helper->getData(s_see_opponents_stacks, "view_enemies");
      helper->getData(s_see_opponents_production, "view_production");
      Glib::ustring quest_policy_str;
      helper->getData(quest_policy_str, "quests");
      s_play_with_quests = Configuration::questPolicyFromString(quest_policy_str);
      helper->getData(s_hidden_map, "hidden_map");
      helper->getData(s_diplomacy, "diplomacy");
      helper->getData(s_cusp_of_war, "cusp_of_war");
      Glib::ustring neutral_cities_str;
      helper->getData(neutral_cities_str, "neutral_cities");
      s_neutral_cities = Configuration::neutralCitiesFromString(neutral_cities_str);
      Glib::ustring razing_cities_str;
      helper->getData(razing_cities_str, "razing_cities");
      s_razing_cities = Configuration::razingCitiesFromString(razing_cities_str);
      Glib::ustring vectoring_mode_str;
      helper->getData(vectoring_mode_str, "vectoring_mode");
      s_vectoring_mode = Configuration::vectoringModeFromString(vectoring_mode_str);
      Glib::ustring build_prod_mode_str;
      helper->getData(build_prod_mode_str, "build_production_mode");
      s_build_production_mode = Configuration::buildProductionModeFromString(build_prod_mode_str);
      Glib::ustring sacking_mode_str;
      helper->getData(sacking_mode_str, "sacking_mode");
      s_sacking_mode = Configuration::sackingModeFromString(sacking_mode_str);
      helper->getData(s_intense_combat, "intense_combat");
      helper->getData(s_military_advisor, "military_advisor");
      helper->getData(s_random_turns, "random_turns");
      helper->getData(s_surrender_already_offered, 
		      "surrender_already_offered");
      Glib::ustring playmode_str;
      helper->getData(playmode_str, "playmode");
      d_playmode = GameScenario::playModeFromString(playmode_str);

      return true;
    }
  
  if (tag == FL_Counter::d_tag)
    {
      debug("loading counter")
	fl_counter = new FL_Counter(helper);
      return true;
    }

  if (tag == Itemlist::d_tag)
    {
      debug("loading items");
      Itemlist::getInstance(helper);
      return true;
    }

  if (tag == Playerlist::d_tag)
    {
      debug("loading players");
      Playerlist::getInstance(helper);
      return true;
    }

  if (tag == GameMap::d_tag)
    {
      debug("loading map")
	GameMap::getInstance(helper);
      return true;
    }

  if (tag == Citylist::d_tag)
    {
      debug("loading cities")

	Citylist::getInstance(helper);
      return true;
    }

  if (tag == Templelist::d_tag)
    {
      debug("loading temples")
	Templelist::getInstance(helper);
      return true;
    }

  if (tag == Ruinlist::d_tag)
    {
      debug("loading ruins")
	Ruinlist::getInstance(helper);
      return true;
    }

  if (tag == Rewardlist::d_tag)
    {
      debug("loading rewards")
	Rewardlist::getInstance(helper);
      return true;
    }

  if (tag == Signpostlist::d_tag)
    {
      debug("loading signposts")
	Signpostlist::getInstance(helper);
      return true;
    }

  if (tag == Roadlist::d_tag)
    {
      debug("loading roads")
	Roadlist::getInstance(helper);
      return true;
    }

  if (tag == Stonelist::d_tag)
    {
      debug("loading stones")
	Stonelist::getInstance(helper);
      return true;
    }

  if (tag == QuestsManager::d_tag)
    {
      debug("loading quests")
	QuestsManager::getInstance(helper);
      return true;
    }

  if (tag == VectoredUnitlist::d_tag)
    {
      debug("loading vectored units")
	VectoredUnitlist::getInstance(helper);
      return true;
    }

  if (tag == Portlist::d_tag)
    {
      debug("loading ports")
	Portlist::getInstance(helper);
      return true;
    }

  if (tag == Bridgelist::d_tag)
    {
      debug("loading bridges")
	Bridgelist::getInstance(helper);
      return true;
    }

  if (tag == GameActionlist::d_tag)
    {
      GameActionlist::getInstance(helper);
      return true;
    }

  if (tag == ScenarioMedia::d_tag)
    {
      ScenarioMedia::getInstance(helper);
      return true;
    }

  if (tag == HeroTemplates::d_tag)
    {
      HeroTemplates::getInstance(helper);
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
  guint32 num = Playerlist::getInstance()->countPlayersAlive();
  if (num < 2)
    errors.push_back(_("There must be at least 2 players in the scenario."));

  num = Citylist::getInstance()->countCities();
  if (num < 2)
    errors.push_back(_("There must be at least 2 cities in the scenario."));

  Glib::ustring match = _("Untitled");
  if (getName ().find (match) == 0 ||
      String::utrim (getName ()).empty () == true)
    errors.push_back(_("The scenario does not have a name."));

  for (auto it: *Playerlist::getInstance())
    {
      if (it == Playerlist::getInstance()->getNeutral())
        {
          if (it->getHeroes().size ())
            errors.push_back(_("Neutrals have one or more heroes."));
          continue;
        }
      if (it->isDead() == true)
	continue;
      if (Citylist::getInstance()->getCapitalCity(it) == NULL ||
          Citylist::getInstance()->getCapitalCity(it)->isBurnt() == true)
	{
	  s = String::ucompose
	    (_("The player called `%1' lacks a capital city."), 
	     it->getName().c_str());
	  errors.push_back(s);
	  break;
	}
      std::vector<Character*> heroes =
        HeroTemplates::getInstance()->getHeroes (it->getId());
      Itemlist *il = Itemlist::getInstance ();
      for (auto h : heroes)
        {
          for (auto item : h->item_ids)
            {
              if (il->find (item) == il->end ())
                {
                  s = String::ucompose
                    (_("The hero type called `%1' belonging to '%2' has bad starting items."),
                     h->name, Playerlist::getInstance ()->getPlayer (h->owner)->getName ());
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
          HeroTemplates *templates = HeroTemplates::getInstance ();
          Character *c = templates->getCharacterById (h->getHeroTypeId ());
          if (c->owner == Playerlist::getInstance ()->getNeutral ()->getId ())
            continue;
          if (!c || c->owner != it->getId ())
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
  for (auto it: *Citylist::getInstance())
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
  for (auto it: *Ruinlist::getInstance())
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

  for (auto it: *Ruinlist::getInstance())
    {
      if (it->getOccupant () && it->getOccupant ()->getName () == "" &&
          it->getOccupant()->getStack ())
        {
          s = String::ucompose("%1 has an unnamed keeper", it->getName ());
          errors.push_back(s);
        }
    }

  std::vector<Vector<int> >unnamed_temple_pos;
  for (auto it: *Templelist::getInstance())
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
  for (auto it: *Playerlist::getInstance()->getNeutral()->getStacklist())
    {
      if (Citylist::getInstance()->getObjectAt(it->getPos()) == NULL)
	count++;
    }
  if (count > 0)
    {
      s = String::ucompose(ngettext("There is %1 neutral stack not in a city", "There are %1 neutral stacks not in cities", count), count);
      warnings.push_back(s);
    }

      
  GameMap::getInstance()->calculateBlockedAvenues();
  if (GameMap::getInstance()->checkCityAccessibility() == false)
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
  
  for (auto it: *Itemlist::getInstance())
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

void GameScenario::initialize(GameParameters g)
{
  Playerlist::getInstance()->clearAllActions();
  setupFog(g.hidden_map);
  setupCities(g.quick_start, g.build_production_mode);
  setupStacks(g.hidden_map);
  setupRewards(g.hidden_map, g.difficulty);
  setupDiplomacy(g.diplomacy);
  if (s_random_turns)
    Playerlist::getInstance()->randomizeOrder();
  nextRound();
  if (d_playmode == GameScenario::NETWORKED)
    {
      GameMap::getInstance()->clearStackPositions();
      Playerlist::getInstance()->turnHumansIntoNetworkPlayers();
    }
  else
    autoSave();
  GameMap::getInstance()->updateStackPositions();

  if (d_name == "AutoGenerated")
    {
      if (GameMap::getInstance()->checkCityAccessibility() == false)
	exit (0);
    }
}

//! Grabs the game option information out of a scenario file.
class ParamLoader
{
public:
    ParamLoader(Glib::ustring filename, bool &broken) {
      Tar_Helper t(filename, std::ios::in, broken);
      if (broken)
        return;
      std::list<Glib::ustring> ext;
      ext.push_back(MAP_EXT);
      ext.push_back(SAVE_EXT);
      Glib::ustring tmpfile = t.getFirstFile(ext, broken);
      XML_Helper helper(tmpfile, std::ios::in);
      helper.registerTag(GameMap::d_tag, 
			 sigc::mem_fun(this, &ParamLoader::loadParam));
      helper.registerTag(GameScenario::d_tag, 
			 sigc::mem_fun(this, &ParamLoader::loadParam));
      helper.registerTag(Playerlist::d_tag, 
			 sigc::mem_fun(this, &ParamLoader::loadParam));
      helper.registerTag(Player::d_tag, 
			 sigc::mem_fun(this, &ParamLoader::loadParam));
      bool retval = helper.parseXML();
      helper.close();
      File::erase(tmpfile);
      if (broken == false)
	broken = !retval;
    }
    bool loadParam(Glib::ustring tag, XML_Helper* helper)
      {
	if (tag == Playerlist::d_tag)
	  {
	    helper->getData(d_neutral, "neutral");
	    return true;
	  }
	if (tag == Player::d_tag)
	  {
	    int type;
	    int id;
	    Glib::ustring name;
	    GameParameters::Player p;
	    helper->getData(id, "id");
	    p.id = id;
	    helper->getData(type, "type");
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
	    helper->getData(name, "name");
	    p.name = name;
	    if (p.id != d_neutral) //is not neutral
	      game_params.players.push_back(p);
	    else
	      {
		int armyset_id;
		helper->getData(armyset_id, "armyset");
		Armyset *armyset = Armysetlist::getInstance()->get(armyset_id);
		game_params.army_theme = armyset->getBaseName();
	      }

	    return true;
	  }
	if (tag == GameMap::d_tag)
	  {
	    helper->getData(game_params.shield_theme, "shieldset");
	    helper->getData(game_params.tile_theme, "tileset");
	    helper->getData(game_params.city_theme, "cityset");
	    return true;
	  }
	if (tag == GameScenario::d_tag)
	  {
	    helper->getData(game_params.name, "name");
	    helper->getData(game_params.see_opponents_stacks, 
			    "view_enemies");
	    helper->getData(game_params.see_opponents_production, 
			    "view_production");
	    Glib::ustring quest_policy_str;
	    helper->getData(quest_policy_str, "quests");
	    game_params.play_with_quests = 
	      Configuration::questPolicyFromString(quest_policy_str);
	    helper->getData(game_params.hidden_map, "hidden_map");
	    helper->getData(game_params.diplomacy, "diplomacy");
	    helper->getData(game_params.cusp_of_war, "cusp_of_war");
	    Glib::ustring neutral_cities_str;
	    helper->getData(neutral_cities_str, "neutral_cities");
	    game_params.neutral_cities = 
	      Configuration::neutralCitiesFromString(neutral_cities_str);
	    Glib::ustring razing_cities_str;
	    helper->getData(razing_cities_str, "razing_cities");
	    game_params.razing_cities = 
	      Configuration::razingCitiesFromString(razing_cities_str);
	    Glib::ustring vectoring_mode_str;
	    helper->getData(vectoring_mode_str, "vectoring_mode");
	    game_params.vectoring_mode = 
	      Configuration::vectoringModeFromString(vectoring_mode_str);
	    helper->getData(game_params.intense_combat, 
			    "intense_combat");
	    helper->getData(game_params.military_advisor, 
			    "military_advisor");
	    helper->getData(game_params.random_turns, "random_turns");
	    return true;
	  }
	return false;
      };
    GameParameters game_params;
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
      std::list<Glib::ustring> ext;
      ext.push_back(MAP_EXT);
      ext.push_back(SAVE_EXT);
      Glib::ustring tmpfile = t.getFirstFile(ext, broken);
      if (tmpfile == "")
        {
          broken = true;
          return;
        }
      XML_Helper helper(tmpfile, std::ios::in);
      helper.registerTag(GameScenario::d_tag, 
			 sigc::mem_fun(this, &PlayModeLoader::loadParam));
      bool retval = helper.parseXML();
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
	    helper->getData(playmode_str, "playmode");
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
        std::list<Glib::ustring> ext;
        ext.push_back(MAP_EXT);
        ext.push_back(SAVE_EXT);
        Glib::ustring tmpfile = tar.getFirstFile(ext, broken);
        XML_Helper helper(tmpfile, std::ios::in);
        helper.registerTag(GameScenario::d_tag, 
                           sigc::mem_fun(this, &DetailsLoader::loadDetails));
        helper.registerTag(Player::d_tag, 
                           sigc::mem_fun(this, &DetailsLoader::loadDetails));
        helper.registerTag(City::d_tag, 
                           sigc::mem_fun(this, &DetailsLoader::loadDetails));
        bool retval = helper.parseXML();
        helper.close();
        File::erase(tmpfile);
        if (!broken)
          broken = !retval;
      }

    bool loadDetails(Glib::ustring tag, XML_Helper* helper)
      {
	if (tag == GameScenario::d_tag)
	  {
	    helper->getData(name, "name");
	    helper->getData(comment, "comment");
	    helper->getData(id, "id");
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
  if (fl_counter)
    {
      delete fl_counter;
      fl_counter = 0;
    }
  GameScenarioOptions::s_round = 0;
}

bool GameScenario::upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version)
{
  return FileCompat::getInstance()->upgrade(filename, old_version, new_version,
                                            FileCompat::GAMESCENARIO, 
                                            d_top_tag);
}

void GameScenario::support_backward_compatibility()
{
  FileCompat::getInstance()->support_type (FileCompat::GAMESCENARIO, MAP_EXT, 
                                           d_top_tag, true);
  FileCompat::getInstance()->support_type (FileCompat::GAMESCENARIO, SAVE_EXT, 
                                           d_top_tag, true);
  FileCompat::getInstance()->support_version
    (FileCompat::GAMESCENARIO, "0.2.0", "0.2.1",
     sigc::ptr_fun(&GameScenario::upgrade));
  FileCompat::getInstance()->support_version
    (FileCompat::GAMESCENARIO, "0.2.1", "0.3.2",
     sigc::ptr_fun(&GameScenario::upgrade));
  FileCompat::getInstance()->support_version
    (FileCompat::GAMESCENARIO, "0.3.2", "0.3.3",
     sigc::ptr_fun(&GameScenario::upgrade));
}
