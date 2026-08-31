//  Copyright (C) 2004 John Farrell
//  Copyright (C) 2004, 2005, 2006 Ulf Lorenz
//  Copyright (C) 2004, 2005, 2006 Andrea Paternesi
//  Copyright (C) 2007, 2008, 2009, 2010, 2014, 2015, 2017, 2021,
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

#include <map>

#include "ai-smart.h"
#include "player-list.h"
#include "army-set-list.h"
#include "stack-list.h"
#include "path.h"
#include "ai-analysis.h"
#include "ai-allocation.h"
#include "ai-diplomacy.h"
#include "action.h"
#include "xml-helper.h"
#include "army-prod-base.h"
#include "army-proto.h"
#include "history.h"
#include "city-list.h"
#include "city.h"
#include "sight-map.h"
#include "sage.h"
#include "game-map.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::flush<<std::endl;}
#define debug(x)

AI_Smart::AI_Smart(Glib::ustring name, unsigned int armyset,
                   Shield::Color shield, int width, int height)
  :RealPlayer(name, armyset, shield, width, height, Player::AI_SMART),
   d_mustmakemoney(0)
{
}

AI_Smart::AI_Smart(const Player& player, bool sync_ids)
    :RealPlayer(player, sync_ids),d_mustmakemoney(0)
{
    d_type = AI_SMART;
}

AI_Smart::AI_Smart(XML_Helper* helper)
    :RealPlayer(helper),d_mustmakemoney(0)
{
}

void AI_Smart::startTurn (sigc::slot<void(bool)> finish)
{
  if (getStacklist ()->getHeroes ().size () == 0 &&
      Citylist::instance ()->countCities (this) == 1)
    AI_maybeBuyScout (getFirstCity ());

  debug("Player " << getName() << " starts a turn.");

  AI_Diplomacy diplomacy (this);

  diplomacy.considerCuspOfWar ();

  if (getGold () < 500 && getUpkeep () > getIncome ())
    d_mustmakemoney = 1;
  else
    d_mustmakemoney = 0;

  // the real stuff
  examineCities ();

  AI_setupVectoring (10, 3, 20);

  auto next = std::make_shared<std::function<void()>>();
  *next =
    [this,
    diplomacy,
    next,
    finish] () mutable
      {
        AI_Analysis *analysis = new AI_Analysis (this);
        const Threatlist *threats = analysis->getThreatsInOrder ();
        City *first_city = getFirstCity ();
        bool build_capacity = false;
        if (first_city)
          {
            Vector<int> pos = first_city->getPos ();
            City *first_neutral = Citylist::instance ()->getNearestNeutralCity (pos);
            if (first_neutral)
              {
                if (dist (pos, first_neutral->getPos()) <= 50)
                  build_capacity = true;
              }
          }
        if (getGold () < 30)
          build_capacity = true;

        AI_Allocation *allocation = new AI_Allocation (analysis, threats, this);

        allocation->move
          (first_city, build_capacity,
           [this,
           allocation,
           analysis,
           diplomacy,
           next,
           finish] (bool moved_a_stack) mutable
           {
             sbusy.emit ();
             delete analysis;
             if (!moved_a_stack)
               {
                 parkAllStacks ();
                 analysis = NULL;
                 stackDeselect ();

                 diplomacy.makeProposals ();

                 finish (!(Playerlist::instance ()->getNoOfPlayers () <= 1));
                 delete allocation;
                 return;
               }
             else
               {
                 delete allocation;
               (*next) ();
               }
           });

      };
  (*next) ();
  return;
}

void AI_Smart::abortTurn()
{
  abort_requested = true;
  if (surrendered)
    aborted_turn.emit();
  else if (Playerlist::instance()->countPlayersAlive() == 1)
    aborted_turn.emit();
}

CityDefeatedChoice AI_Smart::chooseCityDefeatedAction (City *c, Stack *s)
{
  (void) s;
  CityDefeatedChoice action = CITY_DEFEATED_OCCUPY;
  AI_invadeCityQuestPreference(c, action);
  return action;
}

void AI_Smart::invadeCity(City* c)
{
  if (c->getNoOfProductionBases() == 0)
    maybeBuyProduction(c, true);

  // Update its production
  setProduction(c);
}

void AI_Smart::heroGainsLevel(Hero * a, Army::Stat stat)
{
    doHeroGainsLevel(a, stat);
    addAction(new Action_Level(a, stat));
}

int AI_Smart::maybeBuyProduction(City *c, bool quick)
{
  if (c->getBuildProduction() == false)
    return -1;
  int armytype = -1;
  int freeslot = c->getFreeSlot();

  if (freeslot == -1)
    return -1;

  armytype = chooseArmyTypeToBuy(c, quick);
  //does this armytype beat the one we're currently producing?
  //is the one we're buying any better ?
  if (armytype == -1)
    return -1;

  bool buy = false;
  int slot = c->getActiveProductionSlot();

  ArmyProto *army = Armysetlist::instance()->getArmy(getArmyset(), armytype);
  if (slot == -1)
    buy = true;
  else if (scoreBestArmyType(army) > scoreBestArmyType(c->getProductionBase(slot)))
    buy = true;

  if (buy)
    {
      debug("armytype i want to produce " << armytype);

      bool couldbuy = cityBuyProduction(c, freeslot, armytype);

      if (armytype >= 0 && couldbuy)
        {
          debug("YES I COULD BUY! type=" << armytype) 
            return armytype;
        }
    }
  return -1;
}

int AI_Smart::setQuickProduction(City *c)
{
  int select = -1;
  int best_score = -1;

  // we try to determine the most attractive basic production
  for (guint32 i = 0; i < c->getMaxNoOfProductionBases(); i++)
    {
      if (c->getArmytype(i) == -1)    // no production in this slot
        continue;

      const ArmyProdBase *proto = c->getProductionBase(i);
      int score = scoreQuickArmyType(proto);
      if (score > best_score)
        {
          select = i;
          best_score = score;
        }
    }

  if (select != c->getActiveProductionSlot())
    {
      cityChangeProduction(c, select);
      debug(getName() << " Set production to " << select << " in " << c->getName())
    }

  return c->getActiveProductionSlot();
}

int AI_Smart::setBestProduction(City *c)
{
  int select = -1;
  int best_score = -1;

  // we try to determine the most attractive basic production
  for (guint32 i = 0; i < c->getMaxNoOfProductionBases(); i++)
    {
      if (c->getArmytype(i) == -1)    // no production in this slot
        continue;

      const ArmyProdBase *proto = c->getProductionBase(i);
      int score = scoreBestArmyType(proto);
      if (score > best_score)
        {
          select = i;
          best_score = score;
        }
    }

  if (select != c->getActiveProductionSlot())
    {
      cityChangeProduction(c, select);
      debug(getName() << " Set production to slot " << select << " in " << c->getName())
    }

  return c->getActiveProductionSlot();
}

int AI_Smart::chooseArmyTypeToBuy(City *c, bool quick)
{
  int bestScore = -1;
  int bestTypeId = -1;

  Armyset *as = Armysetlist::instance()->get(getArmyset());
  for (Armyset::iterator i = as->begin(); i != as->end(); ++i)
    {
      const ArmyProto *proto = NULL;

      proto=Armysetlist::instance()->getArmy(getArmyset(), (*i)->getId());

      if (proto->getNewProductionCost() == 0)
        continue;

      if ((int)proto->getNewProductionCost() > d_gold)
        continue;

      if (c->hasProductionBase(proto->getId())==false)
        {
          int score;
          if (quick)
            score = scoreQuickArmyType(proto);
          else
            score = scoreBestArmyType(proto);
          if (score >= bestScore)
            {
              bestTypeId = (*i)->getId();
              bestScore = score;
            }
        }
    }

  return bestTypeId;
}

int AI_Smart::scoreQuickArmyType(const ArmyProdBase *a)
{
  //go get the best 1 turn army with the highest strength
  int strength = a->getStrength();

  int production = (5 - a->getProduction()) * 10;
  return strength + production;
}

int AI_Smart::scoreQuickArmyType(const ArmyProto *a)
{
  //go get the best 1 turn army with the highest strength
  int strength = a->getStrength();

  int production = (5 - a->getProduction()) * 10;
  return strength + production;
}

int AI_Smart::scoreBestArmyType(const ArmyProto *a)
{
  int production = a->getProduction();
  if (production == 3)
    production = 4;
  //this treats armies with turns of 7 or higher unfairly
  int max_strength = 60 / production * a->getStrength();

  int city_bonus = 0;
  switch (a->getArmyBonus())
    {
    case Army::ADD1STRINCITY: city_bonus += 5; break;
    case Army::ADD2STRINCITY: city_bonus += 10; break;
    }

  int any_other_bonus = 0;
  if (a->getArmyBonus() && city_bonus == 0)
    any_other_bonus += 2;

  int move_bonus = 0;
  if (a->getMaxMoves() >  10)
    move_bonus += 2;
  if (a->getMaxMoves() >=  20)
    move_bonus += 4;

  return max_strength + city_bonus + move_bonus + any_other_bonus;
}

int AI_Smart::scoreBestArmyType(const ArmyProdBase *a)
{
  //this treats armies with turns of 7 or higher unfairly
  int max_strength = 60 / a->getProduction() * a->getStrength();

  int city_bonus = 0;
  switch (a->getArmyBonus())
    {
    case Army::ADD1STRINCITY: city_bonus += 5; break;
    case Army::ADD2STRINCITY: city_bonus += 10; break;
    }

  int any_other_bonus = 0;
  if (a->getArmyBonus() && city_bonus == 0)
    any_other_bonus += 2;

  int move_bonus = 0;
  if (a->getMaxMoves() >  10)
    move_bonus += 2;
  if (a->getMaxMoves() >=  20)
    move_bonus += 4;

  return max_strength + city_bonus + move_bonus + any_other_bonus;
}

bool AI_Smart::cityNewlyTaken(City *city, guint32 turns) const
{
  guint count = 0;
  std::list<History*> h = getHistoryForCityId(city->getId());
  for (std::list<History*>::reverse_iterator i = h.rbegin(); i != h.rend(); ++i)
    {
      if ((*i)->getType() == History::START_TURN)
        count++;
      else if ((*i)->getType() == History::CITY_WON)
        break;
    }

  if (count >= turns)
    return true;
  return false;
}

void AI_Smart::setProduction(City *city)
{
  if (city->countDefenders() < 3 || cityNewlyTaken(city) == true)
    {
      int slot = setQuickProduction(city);
      if (slot == -1)
        {
          slot = maybeBuyProduction(city, true);
          if (slot != -1)
            cityChangeProduction(city, slot);
        }
    }
  else
    {
      setBestProduction(city);
    }
}

void AI_Smart::examineCities()
{
  debug("Examinating Cities to see what we can do");
  for (auto city: *Citylist::instance())
    {
      if (city->getOwner() == this && city->isBurnt() == false)
        setProduction(city);
    }
  //do we have enough money to create all these new-fangled army units?
  int profit = getIncome() - getUpkeep();
  int total_gp_to_spend = getGold() + profit;
  //now we get to spend this amount on the city production.
  //we'll turn off the cities we can't afford.
  std::list<City*> cities = Citylist::instance()->getNearestFriendlyCities(this);
  for (std::list<City*>::iterator it = cities.begin(); it != cities.end(); ++it)
    {
      City *c = *it;
      if (total_gp_to_spend <= 0)
        cityChangeProduction(c, -1);
      else
        {
          const ArmyProdBase *prodbase = c->getActiveProductionBase();
          if (prodbase)
            {
              total_gp_to_spend -= prodbase->getProductionCost();
              if (total_gp_to_spend <= 0)
                cityChangeProduction(c, -1);
            }
        }
    }
}

bool AI_Smart::chooseTreachery (Stack *stack, Player *player, Vector <int> pos)
{
  (void) stack;
  (void) player;
  (void) pos;
  return true;
}

bool AI_Smart::chooseHero(HeroProto *hero, City *city, int gold)
{
  (void) hero;
  (void) city;
  (void) gold;
  return true;
}

Reward *AI_Smart::chooseReward(Ruin *ruin, Sage *sage, Stack *stack)
{
  (void) ruin;
  (void) stack;
  //always pick the money.
  Reward *reward = NULL;
  for (Sage::iterator it = sage->begin(); it != sage->end(); ++it)
    if ((*it)->getType() == Reward::GOLD)
      {
        reward = (*it);
        break;
      }
  if (!reward)
    reward = sage->front();
  sage->selectReward(reward);
  return reward;
}

Army::Stat AI_Smart::chooseStat(Hero *hero)
{
  if (hero && hero->getStat(Army::STRENGTH)  > 7)
    return Army::MOVES;
  return Army::STRENGTH;
}

bool AI_Smart::chooseQuest(Hero *hero)
{
  (void) hero;
  return true;
}

bool AI_Smart::chooseVisitRuin(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) turns;
  if (stack->isOnCity() == true)
    {
      if (moves <= stack->getMoves())
        return true;
      else
        return false;
    }
  //if (stack->size() == 1 && stack->getFirstHero()->getStrength() < 6)
    //return false;
  if (stack->getPos() == dest)
    return true;
  if (moves < stack->getMoves() + 17)
    return true;
  else
    return false;
}

bool AI_Smart::choosePickupBag(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) turns;
  City *enemy = GameMap::getEnemyCity(dest);
  if (enemy != NULL && enemy->isBurnt() == false)
    return false;
  if (stack->getPos() == dest)
    return true;
  City *c = GameMap::getCity(stack->getPos());
  if (c)
    {
      if (c->contains(dest) == true)
        return true;
    }
  else
    return false;
  //is this the closest hero to the bag?

  if (turns > 0)
    {
      Vector<int> diff = dest - stack->getPos();
      int dist;
      if (abs(diff.x) > abs(diff.y))
        dist = abs(diff.x);
      else
        dist = abs(diff.y);
      std::vector<Stack*> stacks = GameMap::getNearbyFriendlyStacks(dest, dist);
      for (std::vector<Stack*>::iterator it = stacks.begin(); it != stacks.end();
           ++it)
        {
          if ((*it)->hasHero() && (*it)->getId() != stack->getId())
            return false;
          else if ((*it)->getId() == stack->getId())
            break;
        }
    }

  if (moves < stack->getMoves() + 7)
    return true;
  else
    return false;
}

bool AI_Smart::chooseVisitTempleForBlessing(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) turns;
  if (stack->isOnCity() == true)
    {
      if (moves * 2 <= stack->getMoves())
        return true;
      else
        return false;
    }
  if (stack->getPos() == dest)
    return true;
  //if (stack->size() == 1)
    //return false;
  //if (stack->getMoves() != stack->getMaxMoves())
    //return false;
  //if (turns == 0)
    //return true;
  if (moves < stack->getMoves() + 7)
    return true;
  else
    return false;
  return true;
}

bool AI_Smart::chooseVisitTempleForQuest(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) turns;
  (void) dest;
  if (stack->isOnTemple () == false)
    {
      if (stack->isOnCity() == true)
        {
          if (moves <= stack->getMoves())
            return true;
          else
            return false;
        }
      if (moves < stack->getMoves() + 17)
        return true;
      else
        return false;
    }
  else
    {
      return true;
    }
}

bool AI_Smart::chooseContinueQuest(Stack *stack, Quest *quest, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) stack;
  (void) quest;
  (void) dest;
  (void) moves;
  if (turns > 4)
    return false;
  return true;
}
