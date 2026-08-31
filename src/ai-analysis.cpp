//  Copyright (C) 2004 John Farrell
//  Copyright (C) 2004, 2005, 2006 Ulf Lorenz
//  Copyright (C) 2006 Andrea Paternesi
//  Copyright (C) 2009, 2014, 2026 Ben Asselstine
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

#include <iostream>
#include "ai-analysis.h"
#include "city-list.h"
#include "threat-list.h"
#include "threat.h"
#include "player-list.h"
#include "stack-ref-list.h"
#include "stack-list.h"
#include "ruin-list.h"
#include "army.h"
#include "city.h"
#include "ai-city-info.h"
#include "army-set-list.h"
#include "boon-list.h"
#include "boon.h"
#include "quest-manager.h"
#include "quest.h"
#include "temple-list.h"
#include "game-map.h"
#include "boon.h"
#include "path.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::flush<<std::endl;}
#define debug(x)

//this instance is just needed in case one of the observed stacks dies during
//the analysis (and the following actions).
AI_Analysis* AI_Analysis::instance = 0;

AI_Analysis::AI_Analysis(Player *owner)
    :d_owner(owner)
{
    d_stacks = 0;
    d_threats = new Threatlist();
    d_boons = new Boonlist();
    d_stacks = new StackReflist(owner->getStacklist());

    examineCities();
    examineRuins();
    examineTemples();
    examineQuests();
    examineBags();
    examineStacks();
    calculateDanger();

    instance = this;
}

AI_Analysis::~AI_Analysis()
{
    instance = 0;

    delete d_threats;
    delete d_boons;
    delete d_stacks;

    while (!d_cityInfo.empty())
    {
        delete (*d_cityInfo.begin()).second;
        d_cityInfo.erase(d_cityInfo.begin());
    }
}

void AI_Analysis::deleteStack(guint32 id)
{
  if (instance)
    {
        instance->d_threats->deleteStack(id);
        instance->d_stacks->removeStack(id);
    }
}

void AI_Analysis::deleteStack(Stack* s)
{
    if (instance)
    {
        debug("delete stack from ai_analysis")
        instance->d_threats->deleteStack(s->getId());
        instance->d_stacks->removeStack(s->getId());
        debug("stack " << s << " died")
    }
}

float AI_Analysis::assessArmyStrength(const Army *army)
{
  return (float)army->getStat(Army::STRENGTH);
}

float AI_Analysis::assessStackStrength(const Stack *stack)
{
    if (!instance)
        return stack->size() * 5.0;

    if (stack->getOwner() == instance->d_owner)
    {
        // our stack, so we can look inside it
        float total = 0.0;
        for (Stack::const_iterator it = stack->begin(); it != stack->end(); ++it)
          total += assessArmyStrength(*it);

        return total;
    }
    else
      {
        // enemy stack, no cheating!
        // if we were smarter, we would remember all stacks we had seen before and return a better number here.
        // We don't assume a too high average strength
        guint32 as = stack->getOwner()->getArmyset();
        guint32 type_id = stack->getStrongestArmy()->getTypeId();
        ArmyProto *strongest = Armysetlist::instance()->getArmy(as, type_id);
        //if the strongest army has a strength of 4 or less,
        //we assume that all army units in the stack have the same strength.
        if (strongest->getStrength() < 5)
          return stack->size() * strongest->getStrength();
        //otherwise we round everything down to an average of 5 strength.
        return stack->size() * 5.0;
      }
}

const Threatlist* AI_Analysis::getThreatsInOrder()
{
    d_threats->sortByValue();
    return d_threats;
}

const Threatlist* AI_Analysis::getThreatsInOrder(Vector<int> pos)
{
    d_threats->sortByDistance(pos);
    return d_threats;
}

std::list<Boon> AI_Analysis::getBoonsInOrder (Stack *s)
{
  return d_boons->get_best_boons (s);
}

void AI_Analysis::getCityWorstDangers(float dangers[3])
{
  // i wanto to have a result array with the first worst dangers
  for (int i = 0; i < 3; i++)
    {
      float tmp=0.0;

      for (std::map<guint32, AICityInfo *>::iterator it = d_cityInfo.begin();
           it!=d_cityInfo.end(); ++it)
        {
          tmp=(*it->second).getDanger();

          if (dangers[i] < tmp)
            {
              if (i>0) // If The iteration is not the first we do want to avoid to store
                       // already stored worst dangers
                {
                  if(tmp < dangers[i-1])
                    dangers[i]=tmp;
                }
              else
                {
                  dangers[i]=tmp; // The first iteration we get the real worst Danger
                }
            }
        }
    }

  return;
}

int AI_Analysis::getNumberOfDefendersInCity(City *city)
{
  AICityMap::iterator it = d_cityInfo.find(city->getId());
  if (it == d_cityInfo.end())
    return 0;

  return (*it).second->getDefenderCount();
}

float AI_Analysis::getCityDanger(City *city)
{
  AICityMap::iterator it = d_cityInfo.find(city->getId());
  // city does not exist in the map
  if (it == d_cityInfo.end())
    return 0.0;

  debug("Threats to " << city->getName() << " are " << d_cityInfo[city->getId()]->getThreats()->toString())
    return (*it).second->getDanger();
}

void AI_Analysis::reinforce(City *city, Stack *stack, int movesToArrive)
{
  AICityMap::iterator it = d_cityInfo.find(city->getId()) ;
  if (it == d_cityInfo.end())
    return;

  (*it).second->addReinforcements(assessStackStrength(stack) / (float) movesToArrive);
}

float AI_Analysis::reinforcementsNeeded(City *city)
{
  AICityMap::iterator it = d_cityInfo.find(city->getId());
  if (it == d_cityInfo.end())
    return -1000.0;

  return (*it).second->getDanger() - (*it).second->getReinforcements();
}

void AI_Analysis::examineCities ()
{
  for (auto city: *Citylist::instance ())
    {
      if (!city->isFriend (d_owner) && !city->isBurnt ())
        d_threats->push_back (new Threat (city));
      if (!city->isFriend (d_owner) && !city->isBurnt () &&
          city->countDefenders () == 0)
        d_boons->add (city);
    }
}

void AI_Analysis::examineStacks()
{
  // add all enemy stacks to the list of threats
  for (auto player: *Playerlist::instance())
    {
      if (player == d_owner)
        continue;

      Stacklist *sl = player->getStacklist();
      for (Stacklist::iterator sit = sl->begin(); sit != sl->end(); ++sit)
        d_threats->addStack(*sit);
    }
}

void AI_Analysis::examineRuins ()
{
  for (auto ruin : *Ruinlist::instance ())
    {
      if (ruin->isSearched())
        continue;
      if (ruin->isHidden () && ruin->getOwner () != d_owner)
        continue;
      d_boons->add (ruin);
    }
}

void AI_Analysis::examineTemples ()
{
  for (auto temple : *Templelist::instance ())
    d_boons->add (temple);
}

void AI_Analysis::examineBags ()
{
  for (auto backpack: GameMap::instance ()->getBackpacks ())
    d_boons->add (backpack);
}

void AI_Analysis::examineQuests ()
{
  for (auto quest: QuestsManager::instance ()->getPlayerQuests (d_owner))
    {
      if (!quest->isPendingDeletion ())
        d_boons->add (quest);
    }
}

void AI_Analysis::calculateDanger()
{
  for (auto city: *Citylist::instance())
    {
      if (city->isFriend(d_owner))
        {
          AICityInfo *info = new AICityInfo(city);
          d_threats->findThreats(info);
          d_cityInfo[city->getId()] = info;
        }
    }
}

void AI_Analysis::changeOwnership (Player * old_player, Player * new_player)
{
  if (instance)
    instance->d_threats->changeOwnership(old_player, new_player);
}

void AI_Analysis::deleteBoon (Boon *b)
{
  auto it = std::find (d_boons->begin (), d_boons->end (), b);

  if (it != d_boons->end ())
    {
      delete *it;
      d_boons->erase (it);
    }
}

Boon* AI_Analysis::getBoonAlongTheWay (Boon b, Stack *s)
{
  auto path = new Path ();
  path->calculate (s, b.get_destination (s));
  auto points = path->getNearbyPoints (2);
  delete path;
  auto boon = d_boons->getClosestBoon (s, points);
  if (!boon)
    return NULL;
  if (b.get_destination (s) == boon->get_destination (s))
    return NULL;

  return boon;
}

std::list<std::pair<Boon,Stack*>> AI_Analysis::getBoonsInOrder ()
{
  std::list<std::pair<Boon,Stack*>> cities;
  std::list<std::pair<Boon,Stack*>> temples;
  std::list<std::pair<Boon,Stack*>> ruins;
  std::list<std::pair<Boon,Stack*>> backpacks;
  std::list<std::pair<Boon,Stack *>> quests;
  for (auto s : *d_owner->getStacklist ())
    {
      if (s->hasHero () && s->canMove ())
        {
          d_boons->calculate (s);
          auto cboons = d_boons->get_best_city_boons ();
          if (cboons.empty () == false)
            cities.push_back (std::pair<Boon,Stack*>(cboons.front (), s));
          auto tboons = d_boons->get_best_temple_boons ();
          if (tboons.empty () == false)
            temples.push_back (std::pair<Boon,Stack*>(tboons.front (), s));
          auto rboons = d_boons->get_best_ruin_boons ();
          if (rboons.empty () == false)
            ruins.push_back (std::pair<Boon,Stack*>(rboons.front (), s));
          auto bboons = d_boons->get_best_backpack_boons ();
          if (bboons.empty () == false)
            backpacks.push_back (std::pair<Boon,Stack*>(bboons.front (), s));
          auto qboons = d_boons->get_best_quest_boons ();
          if (qboons.empty () == false)
            quests.push_back (std::pair<Boon,Stack*>(qboons.front (), s));
        }
    }

  auto cmp =
    sigc::slot<bool(const std::pair<Boon, Stack*>&,
                    const std::pair<Boon, Stack*>&)>
    ([](const auto& a, const auto& b)
     {
       return a.first.get_value () > b.first.get_value ();
     });
  cities.sort (cmp);
  ruins.sort (cmp);
  backpacks.sort (cmp);
  quests.sort (cmp);
  temples.sort (cmp);

  std::list<std::pair<Boon,Stack*>> boons;
  if (temples.empty () == false && temples.front ().first.get_value () > 0)
    boons.push_back (temples.front ());

  temples.clear ();
  //now we do heroless stacks that want to go to a temple
  for (auto s : GameMap::instance ()->getUnblessedStacksNearTemples (7))
    {
      if (s->hasHero () && s->canMove ())
        continue;
      d_boons->calculate (s);
      auto templeboons = d_boons->get_best_temple_boons ();
      if (templeboons.empty () == false)
        temples.push_back (std::pair<Boon,Stack*>(templeboons.front (), s));
    }
  temples.sort (cmp);

  // take two unblessed stacks to the temple
  if (temples.empty () == false && temples.front ().first.get_value () > 0)
    {
      boons.push_back (temples.front ());
      temples.erase (temples.begin ());
      if (temples.empty () == false && temples.front ().first.get_value () > 0)
        boons.push_back (temples.front ());
    }

  if (cities.empty () == false && cities.front ().first.get_value () > 0)
    boons.push_back (cities.front ());
  if (ruins.empty () == false && ruins.front ().first.get_value () > 0)
    boons.push_back (ruins.front ());
  if (backpacks.empty () == false && backpacks.front ().first.get_value () > 0)
    boons.push_back (backpacks.front ());
  if (quests.empty () == false && quests.front ().first.get_value () > 0)
    boons.push_back (quests.front ());

  boons.sort (cmp);
      
  std::vector<Stack*> seen;

  // remove duplicate entries with same stack
  boons.erase
    (std::remove_if
     (boons.begin (),
      boons.end (),
      [&seen](const auto& item)
      {
        Stack* stack = item.second;

        if (std::find (seen.begin (), seen.end (), stack) != seen.end ())
          return true;

        seen.push_back (stack);
        return false;
      }),
     boons.end ());

  //great we have our boons, but are any others on the way?
  //we want to stop the case of blowing right past a temple to get to a ruin
  for (auto b : boons)
    {
      Stack *s = b.second;
      Boon *better_boon = getBoonAlongTheWay (b.first, s);
      if (better_boon)
        {
          auto it = std::find_if
            (boons.begin (), boons.end (),
             [s](std::pair<Boon,Stack*> rec)
             {
               return rec.second == s;
             });
          if (it != boons.end ())
            *it = std::pair<Boon,Stack*>(*better_boon, s);
        }
    }
  return boons;
}

