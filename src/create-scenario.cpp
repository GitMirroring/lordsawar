// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2004 John Farrell
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2012, 2014, 2015, 2017, 2020,
// 2021 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
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

#include <iostream>
#include <sstream>
#include <sigc++/functors/mem_fun.h>

#include "CreateScenario.h"
#include "GameScenario.h"
#include "army.h"
#include "GameMap.h"
#include "counter.h"
#include "player.h"
#include "playerlist.h"
#include "stacklist.h"
#include "citylist.h"
#include "city.h"
#include "ruinlist.h"
#include "ruin.h"
#include "SightMap.h"
#include "rewardlist.h"
#include "Itemlist.h"
#include "templelist.h"
#include "temple.h"
#include "signpostlist.h"
#include "signpost.h"
#include "portlist.h"
#include "port.h"
#include "bridgelist.h"
#include "bridge.h"
#include "roadlist.h"
#include "road.h"
#include "stonelist.h"
#include "stone.h"
#include "armysetlist.h"
#include "citysetlist.h"
#include "tilesetlist.h"
#include "shieldsetlist.h"
#include "real_player.h"
#include "AI_Analysis.h"
#include "AI_Diplomacy.h"
#include "ai_fast.h"
#include "ai_smart.h"
#include "ai_dummy.h"
#include "File.h"
#include "MapGenerator.h"
#include "QuestsManager.h"
#include "Configuration.h"
#include "FogMap.h"
#include "history.h"
#include "game-parameters.h"
#include "rnd.h"
#include "keeper.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

CreateScenario::CreateScenario(int width, int height)
    :d_scenario(0), d_generator(0)
{
    debug("CreateScenario::CreateScenario")
   
    //make sure that objects are deleted
    GameMap::deleteInstance();
    Playerlist::deleteInstance();
    Templelist::deleteInstance();
    Ruinlist::deleteInstance();
    Rewardlist::deleteInstance();
    Portlist::deleteInstance();
    Bridgelist::deleteInstance();
    Citylist::deleteInstance();
    Itemlist::deleteInstance();

    QuestsManager::deleteInstance();

    fl_counter = new FL_Counter();

    setWidth(width);
    setHeight(height);

    d_generator = new MapGenerator();
    d_generator->progress.connect
      (sigc::hide(sigc::mem_fun(*this, &CreateScenario::on_progress)));
}

CreateScenario::~CreateScenario()
{
    debug("CreateScenario::~CreateScenario")

    if (d_generator)
        delete d_generator;

    if (d_scenario)
        delete d_scenario;
    cleanup();
}

void CreateScenario::on_progress()
{
  progress.emit();
}

void CreateScenario::setPercentages(int pgrass, int pwater, int pforest,
                                    int pswamp, int phills, int pmountains)
{
    debug("CreateScenario::setPercentages")

    //handle input with !=100% sum
    int sum = pgrass + pwater + pforest + pswamp +phills + pmountains;

    if (sum != 100)
    {
        double factor = 100 / static_cast<double>(sum);  

        pwater = static_cast<int>(pwater * factor);
        pforest = static_cast<int>(pforest * factor);
        pswamp = static_cast<int>(pswamp * factor);
        phills = static_cast<int>(phills * factor);
        pmountains = static_cast<int>(pmountains * factor);
    }

    //the multiplication doesn't round up, so the figures should be OK now, the
    //missing percentage is implicitely added to the grass part.
    d_generator->setPercentages(pwater, pforest, pswamp, phills, pmountains);
}

void CreateScenario::setMapTiles(Glib::ustring tilesname)
{
    debug("CreateScenario::setMapTiles")
    d_tilesname = tilesname;
    GameMap::getInstance()->setTileset(tilesname);
}

void CreateScenario::setShieldset(Glib::ustring shieldset)
{
    debug("CreateScenario::setShieldset")
    d_shieldsname = shieldset;
    GameMap::getInstance()->setShieldset(shieldset);
}

void CreateScenario::setCityset(Glib::ustring citysetname)
{
    debug("CreateScenario::setCityset")
    d_citysetname = citysetname;
    Cityset *cs = Citysetlist::getInstance()->get(citysetname);
    d_generator->setCityset(cs);
    GameMap::getInstance()->setCityset(citysetname);
}

void CreateScenario::setNoCities(int nocities)
{
    debug("CreateScenario::setNoCities")

    d_generator->setNoCities(nocities);
}

void CreateScenario::setNoRuins(int noruins)
{
    debug("CreateScenario::setNoRuins")

    d_generator->setNoRuins(noruins);
}

void CreateScenario::setNoSignposts (int nosignposts)
{
    debug("CreateScenario::setNoSignposts")

    d_generator->setNoSignposts(nosignposts);
}

void CreateScenario::setNoStones (int nostones)
{
    debug("CreateScenario::setNoStones")

    d_generator->setNoStones(nostones);
}

void CreateScenario::setNoTemples(int notemples)
{
    debug("CreateScenario::setNoTemples")

    d_generator->setNoTemples(notemples);
}

void CreateScenario::setWidth(int width)
{
    debug("CreateScenario::setWidth")

    if (width < 0)
    {
        std::cerr << "CreateScenario:: wrong width given\n";
        return;
    }

    d_width = width;

    //IMPORTANT!!
    GameMap::setWidth(width);
}
void CreateScenario::setHeight(int height)
{
    debug("CreateScenario::setHeight")

    if (height < 0)
    {
        std::cerr << "CreateScenario:: wrong height given\n";
        return;
    }

    d_height = height;

    //IMPORTANT!!
    GameMap::setHeight(height);
}

Player* CreateScenario::addPlayer(Glib::ustring name, guint32 armyset,
                                std::vector<Gdk::RGBA> colors, int type)
{
    debug("CreateScenario::addPlayer")

    Player* p = Player::create(name, armyset, colors, d_width, d_height,
			       Player::Type(type));
    Playerlist::getInstance()->add(p);

    return p;
}

bool CreateScenario::addNeutral(Glib::ustring name, guint32 armyset,
                                std::vector<Gdk::RGBA> colors, int type)
{
    // for consistency, we only allow exactly one neutral player
    if (Playerlist::getInstance()->getNeutral() != 0)
        return false;

    Player* p = addPlayer(name, armyset, colors, Player::Type(type));
    Playerlist::getInstance()->setNeutral(p);
    return true;
}

bool CreateScenario::create(const GameParameters &g)
{
  debug("CreateScenario::create");

  d_scenario = new GameScenario("AutoGenerated", "AutoGenerated");

  GameScenario::s_see_opponents_stacks = g.see_opponents_stacks;
  GameScenario::s_see_opponents_production = g.see_opponents_production;
  GameScenario::s_play_with_quests = g.play_with_quests;
  GameScenario::s_vectoring_mode = g.vectoring_mode;
  GameScenario::s_build_production_mode = g.build_production_mode;
  GameScenario::s_sacking_mode = g.sacking_mode;
  GameScenario::s_hidden_map = g.hidden_map;
  GameScenario::s_diplomacy = g.diplomacy;
  GameScenario::s_cusp_of_war = g.cusp_of_war;
  GameScenario::s_neutral_cities = g.neutral_cities;
  GameScenario::s_razing_cities = g.razing_cities;
  GameScenario::s_military_advisor= g.military_advisor;
  GameScenario::s_random_turns = g.random_turns;
  GameScenario::s_intense_combat = g.intense_combat;

  if (!createMap())
    return false;

  // fog it up
  if (GameScenario::s_hidden_map)
    {
      for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
           pit != Playerlist::getInstance()->end(); ++pit)
        (*pit)->getFogMap()->fill(FogMap::CLOSED);
    }

  if (!setupTemples())
    return false;

  int sage_factor;
  int no_guardian_factor;
  int stronghold_factor;

  getRuinDifficulty (g.difficulty, &sage_factor, &no_guardian_factor,
                     &stronghold_factor);
  if (!setupRuins(GameScenario::s_play_with_quests != GameParameters::NO_QUESTING, 20, 10, 6))
    return false;

  int base_gold;
  getBaseGold (g.difficulty, &base_gold);
  if (!setupPlayers(g.random_turns, base_gold))
    return false;

  if (!setupItems())
    return false;

  if (!setupRoads())
    return false;

  if (!setupBridges())
    return false;

  if (!setupStandingStones(d_generator->getRoadStones ()))
    return false;

  if (!distributePlayers())
    return false;

  int number_of_armies_factor;
  getCityDifficulty(g.difficulty, &number_of_armies_factor);
  if (!setupCities(g.cities_can_produce_allies, number_of_armies_factor))
    return false;

  int signpost_ratio;
  getSignpostDifficulty (g.difficulty, g.hidden_map, &signpost_ratio);
  if (!setupSignposts(signpost_ratio))
    return false;

  return true;
}

bool CreateScenario::dump(Glib::ustring filename) const
{
    debug("CreateScenario::dump")

    if (d_scenario)
        return d_scenario->dump(filename, "map");

    return false;
}

bool CreateScenario::createMap()
{
    debug("CreateScenario::createMap")

    const Maptile::Building* map;
    
    Rewardlist::getInstance();

    //have the generator make the map...
    d_generator->makeMap(d_width, d_height, true);
    
    //...fill the terrain...
    GameMap::getInstance(d_tilesname, d_shieldsname,
			 d_citysetname)->fill(d_generator);

    //...and create cities, temples, ruins ,signposts
    map = d_generator->getBuildings(d_width, d_height);
    Cityset *cityset = Citysetlist::getInstance()->get(d_citysetname);
    
    for (int y = 0; y < d_height; y++)
        for (int x = 0; x < d_width; x++)
        {
            switch (map[y*d_width + x])
            {
                case Maptile::SIGNPOST:
                    Signpostlist::getInstance()->add(new Signpost(Vector<int>(x,y)));
                    break;
                case Maptile::TEMPLE:
                    Templelist::getInstance()->add
		      (new Temple(Vector<int>(x,y), 
				  cityset->getTempleTileWidth(),
				  popRandomTempleName()));
                    break;
                case Maptile::RUIN:
		    Ruinlist::getInstance()->add
		      (new Ruin(Vector<int>(x,y), 
				cityset->getRuinTileWidth(),
				popRandomRuinName()));
		    break;
                case Maptile::CITY:
                    Citylist::getInstance()->add
		      (new City(Vector<int>(x,y), cityset->getCityTileWidth()));
                    break;
                case Maptile::ROAD:
                    Roadlist::getInstance()->add(new Road(Vector<int>(x,y)));
                    break;
                case Maptile::PORT:
                    Portlist::getInstance()->add(new Port(Vector<int>(x,y)));
                    break;
                case Maptile::BRIDGE:
                    Bridgelist::getInstance()->add(new Bridge(Vector<int>(x,y)));
                    break;
                case Maptile::STONE:
                    Stonelist::getInstance()->add(new Stone(Vector<int>(x,y)));
                    break;
                case Maptile::NONE:
		    break;
            }
        }
    //the other details such as giving names are done later
    
    return true;
}

void CreateScenario::createCapitalCity(Player *player, City *city)
{
  // distribute capitals for the players
  city->conquer(player);
  city->setCapitalOwner(player);
  city->setCapital(true);

  player->conquerCity(city, NULL);
}

bool CreateScenario::tooNearToOtherCapitalCities(City *c, std::list<City*> capitals, guint32 distance)
{
  for (std::list<City*>::iterator it = capitals.begin(); it != capitals.end(); 
       ++it)
    {
      int d = dist(c->getPos(), (*it)->getPos());
      if ((guint32) d < distance)
	return true;
    }
  return false;
}

bool CreateScenario::distributePlayers()
{
  debug("CreateScenario::distributePlayers")

  //okay, everyone starts out as neutral.
  for (auto c: *Citylist::getInstance())
    if (c->isBurnt() == false)
      c->setOwner(Playerlist::getInstance()->getNeutral());

  std::list<City*> capitals;
  //now pick some equidistant cities for capitals, that aren't too close.
  for (auto pit: *Playerlist::getInstance())
    {
      int tries = 0;
      if (pit == Playerlist::getInstance()->getNeutral())
        continue;
      while (1)
        {
          Vector<int> pos = 
            Vector<int>(Rnd::rand() % d_width, Rnd::rand() % d_height);
          City *city = Citylist::getInstance()->getNearestCity(pos);
          if (city->isBurnt() == false && city->isCapital() == false)
            {
              if (tooNearToOtherCapitalCities(city, capitals, 30) == false || 
                  tries > 50)
                {
                  createCapitalCity(pit, city);
                  capitals.push_back(city);
                  break;
                }
              else
                tries++;
            }
          else
            tries++;
          if (tries > 100)
            break;
        }
    }

  return true;
}

bool CreateScenario::setupCities(bool cities_can_produce_allies,
				 int number_of_armies_factor)
{
  debug("CreateScenario::setupCities")

  for (auto c: *Citylist::getInstance())
    {
      //1. set a reasonable cityname
      c->setName(popRandomCityName());

      //2. distribute the income a bit (TBD)

      //3. set the city production
      c->setRandomArmytypes(cities_can_produce_allies, 
                            number_of_armies_factor);

      c->setGold(getRandomCityIncome(c->isCapital()));
    }

  return true;
}

bool CreateScenario::setupRoads()
{
  for (auto it: *Roadlist::getInstance())
    it->setType(calculateRoadType(it->getPos()));
  return true;
}

bool CreateScenario::setupBridges()
{
  for (auto it: *Bridgelist::getInstance())
    it->setType(Bridgelist::getInstance()->calculateType(it->getPos()));
  return true;
}

bool CreateScenario::setupTemples()
{
  for (auto it: *Templelist::getInstance())
    {
      // set a random temple type
      int type = Rnd::rand () % TEMPLE_TYPES;
      it->setType(type);
    }
  return true;
}

bool CreateScenario::setupRuins(bool strongholds_invisible, int sage_factor,
				int no_guardian_factor, int stronghold_factor)
{
    debug("CreateScenario::setupRuins")

    //The aim of this function is to put a strong stack as sentinel in all
    //ruins.

    for (Ruinlist::iterator it = Ruinlist::getInstance()->begin();
        it != Ruinlist::getInstance()->end(); ++it)
    {
        // set a random ruin type
        if (Rnd::rand() % stronghold_factor == 0) //one in six ruins is a stronghold
          {
            (*it)->setType(Ruin::STRONGHOLD);
            if (strongholds_invisible == true)
              {
                (*it)->setHidden(true);
                (*it)->setOwner(NULL);
              }
          }
        else
          (*it)->setType(Ruin::RUIN);

        //one in twenty ruins is a sage
        if (Rnd::rand() % sage_factor == 0 &&
           ((*it)->getType() == Ruin::RUIN ||
            (*it)->getType() == Ruin::STRONGHOLD))

          {
            (*it)->setSage (true);
            continue;
          }

        // and set a guardian / occupant / keeper / lone defender, etc
        // but one in ten ruins doesn't have a guardian
        if (Rnd::rand() % no_guardian_factor == 0 &&
            (*it)->getType() == Ruin::RUIN) 
          (*it)->setOccupant (new Keeper (NULL, (*it)->getPos ()));
        else
          (*it)->setOccupant (getRandomRuinKeeper ((*it)->getPos ()));
    }

    return true;
}

bool CreateScenario::setupSignposts(int ratio)
{
  int randno;
  int dynamicPercent = static_cast<int>(1.0 / ratio * 100);
  debug("CreateScenario::setupSignposts")

  for (auto it: *Signpostlist::getInstance())
    {
      if (randomSignpostsEmpty())
        randno = dynamicPercent;
      else
        randno = Rnd::rand() % 100;
      if (randno < dynamicPercent) // set up a signpost from the list of signposts
        it->setName(popRandomSignpost());
      else
        it->setName(getDynamicSignpost(it));
    }

  return true;
}

bool CreateScenario::setupStandingStones(std::vector<Vector<int> > road_stones)
{
  for (auto s : *Stonelist::getInstance ())
    s->setType (CreateScenario::calculateStoneType(s->getPos ()));

  for (auto pos : road_stones)
    {
      Road *r = Roadlist::getInstance()->getObjectAt (pos);
      if (r)
        {
          int type = CreateScenario::calculateStoneType(r->getPos ());
          Stonelist::getInstance ()->add (new Stone (pos, type));
        }
    }
  return true;
}


bool CreateScenario::setupPlayers(bool random_turns, 
				  int base_gold)
{
  debug("CreateScenario::setupPlayers");
  for (auto pit: *Playerlist::getInstance())
    pit->setGold(adjustBaseGold(base_gold));


  if (random_turns)
    Playerlist::getInstance()->randomizeOrder();
  return true;
}

bool CreateScenario::setupItems()
{
  Itemlist::createStandardInstance();
  return true;
}

void CreateScenario::getRuinDifficulty (int difficulty, int *sage_factor, 
					int *no_guardian_factor, 
					int *stronghold_factor)
{
  if (difficulty < 50)
    {
      *sage_factor = 3;
      *no_guardian_factor = 5;
      *stronghold_factor = 12;
    }
  else if (difficulty < 60)
    {
      *sage_factor = 9;
      *no_guardian_factor = 6;
      *stronghold_factor = 10;
    }
  else if (difficulty < 70)
    {
      *sage_factor = 14;
      *no_guardian_factor = 8;
      *stronghold_factor = 9;
    }
  else if (difficulty < 80)
    {
      *sage_factor = 20;
      *no_guardian_factor = 10;
      *stronghold_factor = 6;
    }
  else if (difficulty < 90)
    {
      *sage_factor = 22;
      *no_guardian_factor = 12;
      *stronghold_factor = 4;
    }
  else 
    {
      *sage_factor = 24;
      *no_guardian_factor = 15;
      *stronghold_factor = 3;
    }
}

void CreateScenario::getSignpostDifficulty (int difficulty, bool hidden_map, 
					    int *signpost_ratio)
{
  //the idea here is that we're on a hidden map, and if it's harder
  //difficulty, then we don't get as many signs directing us to cities.
  if (hidden_map)
    {
      if (difficulty < 60)
	*signpost_ratio = 2; //50% of signs point to cities
      else if (difficulty < 70)
	*signpost_ratio = 3; //33% of signs point to cities
      else if (difficulty < 80)
	*signpost_ratio = 6; //16% of signs point to cities
      else if (difficulty < 90)
	*signpost_ratio = 9; //11% of signs point to cities
      else
	*signpost_ratio = 15; //6% of signs point to cities
    }
  else
    *signpost_ratio = 6;
}

void CreateScenario::getCityDifficulty(int difficulty, 
				       int *number_of_armies_factor)
{
  if (difficulty < 50)
    *number_of_armies_factor = 3;
  else if (difficulty < 60)
    *number_of_armies_factor = 2;
  else if (difficulty < 70)
    *number_of_armies_factor = 1;
  else 
    *number_of_armies_factor = 0;
}

int CreateScenario::calculateStoneType (Vector<int> t)
{
  Road *r = Roadlist::getInstance()->getObjectAt(t);
  if (r)
    return Stone::getRandomType(Road::Type(r->getType()));
  else
    return Stone::getRandomType();
}

int CreateScenario::calculateBridgeType (Vector<int> t)
{
  return Bridgelist::getInstance()->calculateType(t);
}

int CreateScenario::calculateRoadType (Vector<int> t)
{
    // examine neighbour tiles to discover whether there's a road or
    // bridge on them
    bool u = false; //up
    bool b = false; //bottom
    bool l = false; //left
    bool r = false; //right

    if (t.y > 0)
      u = Roadlist::getInstance()->getObjectAt(t + Vector<int>(0, -1));
    if (t.y < GameMap::getHeight() - 1)
      b = Roadlist::getInstance()->getObjectAt(t + Vector<int>(0, 1));
    if (t.x > 0)
      l = Roadlist::getInstance()->getObjectAt(t + Vector<int>(-1, 0));
    if (t.x < GameMap::getWidth() - 1)
      r = Roadlist::getInstance()->getObjectAt(t + Vector<int>(1, 0));

    if (!u && t.y > 0)
      u = Bridgelist::getInstance()->getObjectAt(t + Vector<int>(0, -1));
    if (!b && t.y < GameMap::getHeight() - 1)
      b = Bridgelist::getInstance()->getObjectAt(t + Vector<int>(0, 1));
    if (!l && t.x > 0)
      l = Bridgelist::getInstance()->getObjectAt(t + Vector<int>(-1, 0));
    if (!r && t.x < GameMap::getWidth() - 1)
      r = Bridgelist::getInstance()->getObjectAt(t + Vector<int>(1, 0));

    // then translate this to the type
    int type = 2; 
    //show road type 2 when no other road tiles are around
    if (!u && !b && !l && !r)
	type = 2;
    else if (u && b && l && r)
	type = 2;
    else if (!u && b && l && r)
	type = 9;
    else if (u && !b && l && r)
	type = 8;
    else if (u && b && !l && r)
	type = 7;
    else if (u && b && l && !r)
	type = 10;
    else if (u && b && !l && !r)
	type = 1;
    else if (!u && !b && l && r)
	type = 0;
    else if (u && !b && l && !r)
	type = 3;
    else if (u && !b && !l && r)
	type = 4;
    else if (!u && b && l && !r)
	type = 6;
    else if (!u && b && !l && r)
	type = 5;
    else if (u && !b && !l && !r)
	type = Road::CONNECTS_NORTH;
    else if (!u && b && !l && !r)
	type = Road::CONNECTS_SOUTH;
    else if (!u && !b && l && !r)
	type = Road::CONNECTS_WEST;
    else if (!u && !b && !l && r)
	type = Road::CONNECTS_EAST;
    return type;
}

int CreateScenario::calculateNumberOfSignposts(int width, int height, int grass)
{
  int area = width * height;
  return int(area * (grass / 100.0) * SIGNPOST_FREQUENCY);
}

void CreateScenario::updateRoadsBridgesAndStones()
{
  for (auto i : *Roadlist::getInstance ())
    i->setType(CreateScenario::calculateRoadType(i->getPos()));
  for (auto i : *Bridgelist::getInstance ())
    i->setType(CreateScenario::calculateBridgeType(i->getPos()));
  for (auto i : *Stonelist::getInstance ())
    i->setType(CreateScenario::calculateStoneType(i->getPos()));
}

