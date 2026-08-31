//  Copyright (C) 2007, 2008, 2009, 2014, 2015 Ben Asselstine
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
#include "AI_Diplomacy.h"
#include "player.h"
#include "playerlist.h"
#include "citylist.h"
#include "history.h"
#include "game.h"
#include "GameScenarioOptions.h"
#include "city.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::flush<<std::endl;}
#define debug(x)

AI_Diplomacy* AI_Diplomacy::instance = 0;

AI_Diplomacy::AI_Diplomacy(Player *owner)
:d_owner(owner)
{
  instance = this;
}

AI_Diplomacy::~AI_Diplomacy()
{
  instance = 0;
}

void AI_Diplomacy::considerCuspOfWar()
{
  if (GameScenarioOptions::s_cusp_of_war &&
      GameScenarioOptions::s_round == CUSP_OF_WAR_ROUND)
  {
    for (auto other: *Playerlist::getInstance())
    {
      if (other->getType() == Player::HUMAN && !other->isDead() &&
          d_owner->getDiplomaticState(other) != Player::AT_WAR)
      {
        d_owner->proposeDiplomacy (Player::PROPOSE_WAR, other);
        other->proposeDiplomacy (Player::PROPOSE_WAR, d_owner);
        d_owner->declareDiplomacy (Player::AT_WAR, other, false);
      }
    }
  }
}

void AI_Diplomacy::makeFriendsAndEnemies()
{
  // Declare war with enemies, make peace with friends
  // according to their diplomatic scores
  for (auto it: *Playerlist::getInstance())
    {
      if (Playerlist::getInstance()->getNeutral() == it)
	continue;
      if (it->isDead())
	continue;
      if (it == d_owner)
	continue;
      if (d_owner->getDiplomaticState(it) != Player::AT_WAR)
	{
	  if (d_owner->getDiplomaticScore (it) < DIPLOMACY_MIN_SCORE + 2)
	    d_owner->proposeDiplomacy (Player::PROPOSE_WAR , it);
	}
      else if (d_owner->getDiplomaticState(it) != Player::AT_PEACE)
	{
	  if (d_owner->getDiplomaticScore (it) > DIPLOMACY_MAX_SCORE - 2)
	    d_owner->proposeDiplomacy (Player::PROPOSE_PEACE, it);
	}
    }
}

void AI_Diplomacy::makeRequiredEnemies()
{
  for (std::list<Player *>::iterator it = new_enemies.begin();
       it != new_enemies.end(); ++it)
    d_owner->proposeDiplomacy (Player::PROPOSE_WAR , *it);
}

void AI_Diplomacy::neutralsDwindlingNeedFirstEnemy()
{
  // find a close player if neutral cities are getting low
  int target_level = (int)((float)Citylist::getInstance()->size() * (float) 0.06);
  target_level++;
  bool at_war = false;
  guint32 neutral_cities = 
    Citylist::getInstance()->countCities(Playerlist::getInstance()->getNeutral());
  if (neutral_cities && (int)neutral_cities > target_level)
    {
      //Pick a new opponent if we don't already have one.
      for (auto it: *Playerlist::getInstance())
	{
	  if (Playerlist::getInstance()->getNeutral() == it)
	    continue;
	  if (it->isDead())
	    continue;
	  if (it == d_owner)
	    continue;
	  if (d_owner->getDiplomaticState(it) != Player::AT_WAR)
	    at_war = true;
	}
      if (at_war == false)
	{
	  // not at war?  great.  let's pick a player to attack.
	  City *first = d_owner->getFirstCity();
	  if (first)
	    {
	      City *c =
                Citylist::getInstance()->getNearestForeignCity(first->getPos());
	      if (c)
		d_owner->proposeDiplomacy(Player::PROPOSE_WAR, c->getOwner());
	    }
	}
    }
}

void AI_Diplomacy::gangUpOnTheBully()
{
  // declare war with the strong player.
  // declare peace with every other.
  int target_level = (int)((float)Citylist::getInstance()->size() * (float)0.35);
  if (Playerlist::getInstance()->countPlayersAlive() <  MAX_PLAYERS / 2)
    return;
  for (auto it: *Playerlist::getInstance())
    {
      if (Playerlist::getInstance()->getNeutral() == it)
	continue;
      if (it->isDead())
	continue;
      if (it == d_owner)
	continue;
      if (Citylist::getInstance()->countCities(it) > target_level && 
          Playerlist::getInstance()->countPlayersAlive() > 4)
	{
	  if (d_owner->getDiplomaticState(it) != Player::AT_WAR)
	    d_owner->proposeDiplomacy(Player::PROPOSE_WAR, it);
          for (auto pit: *Playerlist::getInstance())
	    {
	      if (Playerlist::getInstance()->getNeutral() == pit)
		continue;
	      if (pit->isDead())
		continue;
	      if (pit == d_owner)
		continue;
	      if (pit == it)
		continue;
	      if (pit->getType() == Player::HUMAN)
		continue;
	      if (d_owner->getDiplomaticState(pit) != Player::AT_PEACE)
		d_owner->declareDiplomacy(Player::AT_PEACE, pit, false);
	    }
	}
    }
}

void AI_Diplomacy::makeProposals()
{
  makeFriendsAndEnemies();
  makeRequiredEnemies();
  neutralsDwindlingNeedFirstEnemy();
  gangUpOnTheBully();
}

void AI_Diplomacy::needNewEnemy(Player *player)
{
  new_enemies.push_back(player);
}
// End of file
