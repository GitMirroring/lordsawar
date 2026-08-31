// Copyright (C) 2004 John Farrell
// Copyright (C) 2004, 2005, 2006, 2007 Ulf Lorenz
// Copyright (C) 2008, 2009, 2010, 2014, 2015, 2017 Ben Asselstine
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
#include <assert.h>
#include "AI_Analysis.h"
#include "AI_Allocation.h"
#include "player.h"
#include "playerlist.h"
#include "citylist.h"
#include "stacklist.h"
#include "stack.h"
#include "city.h"
#include "Threat.h"
#include "MoveResult.h"
#include "ruin.h"
#include "path.h"
#include "ruinlist.h"
#include "GameMap.h"
#include "GameScenarioOptions.h"
#include "Threatlist.h"
#include "PathCalculator.h"
#include "stacktile.h"
#include "stackreflist.h"
#include "armyproto.h"
#include "QuestsManager.h"
#include "Quest.h"
#include "QKillHero.h"
#include "QEnemyArmies.h"
#include "QEnemyArmytype.h"
#include "rnd.h"
#include "reward.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::flush<<std::endl;}
#define debug(x)

AI_Allocation* AI_Allocation::s_instance = 0;


AI_Allocation::AI_Allocation(AI_Analysis *analysis, const Threatlist *threats, Player *owner)
 : d_owner(owner), d_analysis(analysis), d_stacks (NULL), d_threats(threats)
{
    s_instance = this;
}

AI_Allocation::~AI_Allocation()
{
    s_instance = 0;
}

StackReflist::iterator AI_Allocation::eraseStack(StackReflist::iterator it)
{
  setParked(*it, true);
  return d_stacks->eraseStack(it);
}

void AI_Allocation::deleteStack(Stack* s)
{
  //this method deletes it from our list of stacks to consider.
  //it doesn't really delete the stack from the game.
    // we need to remove collaterally eradicated stacks before they make
    // trouble
    if (s_instance)
      {
        s_instance->setParked(s, true);
        s_instance->d_stacks->removeStack(s->getId());
      }
}

int AI_Allocation::allocateStackToCapacityBuilding(Threat *threat, City *first_city, bool take_neutrals)
{
  bool moved = false;
  Vector<int> pos;
  if (first_city)
    pos = threat->getClosestPoint(first_city->getPos());
  else if (d_owner->getStacklist()->size() > 0)
    pos = threat->getClosestPoint(d_owner->getStacklist()->front()->getPos());
  else
    return moved;
  City *c =GameMap::getEnemyCity(pos);
  if (!c)
    return moved;
  if (c->isBurnt() == true)
    return moved;
  if (c->getOwner() == Playerlist::getInstance()->getNeutral() && take_neutrals == false)
    return moved;
  Stack *attacker = findClosestStackToEnemyCity(c, take_neutrals);
  if (!attacker)
    return moved;
  Vector<int> dest = threat->getClosestPoint(attacker->getPos());
  bool killed = false;
  //only take what we need.
  std::list<guint32> armies = attacker->determineStrongArmies(3.0);
  if (armies.size() > 0 && armies.size() != attacker->size() &&
      !attacker->fliesWithItemAndNonFlyersOverWaterOrMountains())
    {
      Stack *stack = d_owner->stackSplitArmies(attacker, armies);
      moved = moveStack(stack, dest, killed);
      if (!killed)
        {
          if (stack->hasPath() == false)
            d_stacks->addStack(stack);
        }
    }
  else
    {
      moved = moveStack(attacker, dest, killed);
      if (!killed)
        {
          if (attacker->hasPath() == true)
            deleteStack(attacker);
        }
    }
  return moved;
}

int AI_Allocation::allocateStacksToCapacityBuilding(City *first_city,
                                                    bool take_neutrals)
{
  int count = 0;

  for (Threatlist::const_iterator it = d_threats->begin(); 
       it != d_threats->end(); ++it)
    {
      Threat *t = *it;
      if (d_stacks->size() == 0)
        break;
      if (t->isCity() && t->getStrength() <= 0.5003)
        {
          if (allocateStackToCapacityBuilding(*it, first_city, take_neutrals))
            {
              count++;
            }
        }
    }
  return count;
}

bool AI_Allocation::continueQuest(Quest *quest, Stack *stack)
{
  bool quest_completed = false;
  bool stack_died = false;
  bool moved = d_owner->AI_maybeContinueQuest(stack, quest, 
                                              quest_completed, stack_died);
  if (!stack_died)
    {
      groupStacks(stack);
      if (!quest_completed)
        deleteStack(stack);
    }
  return moved;
}

int AI_Allocation::continueQuests()
{
  int count = 0;
  if (GameScenarioOptions::s_play_with_quests == GameParameters::NO_QUESTING)
    return count;

  auto quests = QuestsManager::getInstance()->getPlayerQuests(d_owner);
  for (std::vector<Quest*>::iterator i = quests.begin(); i != quests.end(); ++i)
    {
      Quest *quest = *i;
      if (quest == NULL)
        continue;
      if (quest->isPendingDeletion())
        continue;
      Stack *s = d_owner->getStacklist()->getArmyStackById(quest->getHeroId());
      if (s)
        {
          bool moved = continueQuest(quest, s);
          if (moved)
            count++;
        }
    }
  return count;
}

int AI_Allocation::continueAttacks()
{
  int count = 0;
  for (StackReflist::iterator i = d_stacks->begin(); i != d_stacks->end(); ++i)
    {
      Stack *s = *i;
      Vector<int> pos = s->getLastPointInPath();
      City *city = NULL;
      if (pos != Vector<int>(-1,-1))
        city = GameMap::getCity(pos);

      if (s->getParked() == false && s->isOnCity() == false &&
          s->hasPath() == true && city != NULL && city->getOwner() != d_owner
          && city->isBurnt() == false)
        {
          bool killed = false;
          bool moved = moveStack(s, killed);
          if (moved)
            count++;
          if (!killed)
            {
              if (s->hasPath() == true)
                {
                  i = eraseStack(i);
                }
              else
                {
                  if (s->isOnCity() == true)
                    shuffleStacksWithinCity(GameMap::getCity(s), s,
                                            Vector<int>(0,0));
                  i = eraseStack(i);
                }
            }
          else
            i = d_stacks->begin ();
            /*
             * we are iterating over d_stacks and when our stack dies
             * it gets deleted via deleteStack, and it messes up our
             * iterator.
             */
        }
      else if (s->getParked() == false && s->isOnCity() == false &&
               s->hasPath() == true && city != NULL && 
               city->getOwner() == d_owner && city->isBurnt() == false)
        {
          guint32 turns_ago = 0;
          if (d_owner->conqueredCity(city, turns_ago))
            {
              if (turns_ago <= 1)
                {
                  //hey, the city we were moving to was taken over by us.
                  //pick another enemy city.
                  //just clear the path so that another routine will move us.
                  s->clearPath();
                }
            }
        }

    }
  return count;
}

int AI_Allocation::attackNearbyEnemies()
{
  bool moved;
  int count = 0;
  for (auto city: *Citylist::getInstance())
    {
      if (d_owner->abortRequested())
        return count;
      if (d_stacks->size() == 0)
        break;
      if (city->getOwner() == d_owner || city->isBurnt() == true)
        continue;
      std::list<Vector<int> > p = GameMap::getNearbyPoints(city->getPos(), 2);
      for (std::list<Vector<int> >::iterator j = p.begin(); j != p.end(); ++j)
        {
          Stack *s = GameMap::getFriendlyStack(*j);
          if (!s)
            continue;
          if (s->getParked() == false && s->getMoves() >= 4)
            {
              bool killed = false;
              moved = moveStack(s, city->getNearestPos(s->getPos()), killed);
              if (!killed)
                {
                  if (s->hasPath() == true)
                    deleteStack(s);
                  else if (s->isOnCity() == true)
                    {
                      deleteStack(s);
                      shuffleStacksWithinCity(GameMap::getCity(s), s, 
                                              Vector<int>(0,0));
                    }
                }
              else
                break;
              if (moved)
                count++;
            }
          //break;
        }
      sbusy.emit();
    }
  //return count;
  //attack nearby stacks in the field.
  Stacklist *sl = d_owner->getStacklist();
  std::list<Vector<int> > pos = sl->getPositions();
  for (std::list<Vector<int> >::iterator i = pos.begin(); i != pos.end(); ++i)
    {
      if (d_owner->abortRequested())
        return count;
      Stack *s = GameMap::getFriendlyStack(*i);
      if (!s)
        continue;
      if (d_stacks->size() == 0)
        break;
      if (s->isOnCity() == true)
        continue;
      if (s->getParked() == true)
        continue;
      std::list<Vector<int> > p = GameMap::getNearbyPoints(*i, 2);
      for (std::list<Vector<int> >::iterator j = p.begin(); j != p.end(); ++j)
        {
          Stack *enemy = GameMap::getEnemyStack(*j);
          if (!enemy)
            continue;
          bool killed = false;
          if (enemy->isOnCity() == true)
            continue;
          if (s->size() < enemy->size())
            continue;
          if (s->hasShip() != enemy->hasShip())
            continue;
          moved = moveStack(s, enemy->getPos(), killed);
          if (moved)
            count++;
          if (!killed)
            {
              if (s->hasPath() == true)
                deleteStack(s);
            }
          else
            break;
        }
      sbusy.emit();
    }
  //don't leave heroes sitting around in cities.
  pos = sl->getPositions();
  for (std::list<Vector<int> >::iterator i = pos.begin(); i != pos.end(); ++i)
    {
      if (d_owner->abortRequested())
        return count;
      Stack *s = GameMap::getFriendlyStack(*i);
      if (!s)
        continue;
      if (s->getParked() == true)
        continue;
      if (s->hasHero() == false)
        continue;
      if (s->getMoves() < s->getMaxMoves())
        continue;
      City *target = Citylist::getInstance()->getClosestEnemyCity(s);
      if (target)
        {
          bool killed = false;
          moved = moveStack(s, target->getNearestPos(s->getPos()), killed);
          if (moved)
            count++;
          if (!killed)
            {
              if (s->hasPath() == true)
                deleteStack(s);
            }
        }
      sbusy.emit();
    }
   //fixme: this should probably be commented out in favour of emptyOutCities
  //don't leave stacks of eight lying around in cities.
  pos = sl->getPositions();
  for (std::list<Vector<int> >::iterator i = pos.begin(); i != pos.end(); ++i)
    {
      if (d_owner->abortRequested())
        return count;
      Stack *s = GameMap::getFriendlyStack(*i);
      if (!s)
        continue;
      if (s->getParked() == true)
        continue;
      if (s->size() != MAX_STACK_SIZE)
        continue;
      if (s->getMoves() < s->getMaxMoves())
        continue;
      City *city = GameMap::getCity(*i);
      if (!city)
        continue;
      if (city->isBurnt() == false && 
          city->getDefenders().size() > MAX_STACK_SIZE * 2)
        continue;
      City *target = Citylist::getInstance()->getClosestEnemyCity(s);
      if (target)
        {
          bool killed = false;
          moved = moveStack(s, target->getNearestPos(s->getPos()), killed);
          if (moved)
            count++;
          if (!killed)
            {
              if (s->hasPath() == true)
                deleteStack(s);
            }
        }
      sbusy.emit();
    }
  return count;
}

bool AI_Allocation::emptyOutCities()
{
  //everybody out on the dancefloor.
  for (auto c: *Citylist::getInstance())
    {
      if (d_owner->abortRequested())
        return false;
      if (c->getOwner() != d_owner || c->isBurnt() == true)
        continue;
      bool bail = false;
      guint32 num_defenders = c->countDefenders();
      for (guint i = 0; i < c->getSize(); i++)
        {
          for (guint j = 0; j < c->getSize(); j++)
            {
              Stack *s = GameMap::getStack(c->getPos() + Vector<int>(i,j));
              if (!s)
                continue;
              if ((s->getMoves() > 3 && s->size() >= 4 &&
                   (num_defenders - s->size()) >= 3) || (Rnd::rand() % 10) == 0)
                {
                  City *target = 
                    Citylist::getInstance()->getNearestEnemyCity(s->getPos());
                  if (target)
                    {
                      bool killed = false;
                      if (d_stacks->contains(s->getId()) == false)
                        d_stacks->addStack(s);
                      moveStack(s, target->getNearestPos(s->getPos()), killed);
                    }
                  else
                    return false;
                  bail = true;
                  break;
                }
            }
          if (d_owner->getGold() < 20)
            continue;
          if (bail)
            break;
        }
      sbusy.emit();
    }
  return true;
}

int AI_Allocation::visitTemples(bool get_quests)
{
  int count = 0;
  Stacklist *sl = d_owner->getStacklist();
  std::list<Vector<int> > pos = sl->getPositions();
  for (std::list<Vector<int> >::iterator i = pos.begin(); i != pos.end(); ++i)
    {
      Stack *s = GameMap::getFriendlyStack(*i);
      if (!s)
        continue;
      if (s->hasHero() && get_quests)
        {
          //debug("Player " << d_owner->getName() << " moving hero-laden stack " << s->getId() << " towards a temple");
          bool moved;
          bool killed = false;
          bool got_quest = false;
          moved = d_owner->AI_maybeVisitTempleForQuest(s, s->getMaxMoves(), 
                                                       got_quest, killed);
          if (moved)
            {
              count++;
              if (!killed)
                {
                  groupStacks(s);
                  deleteStack(s);
                }
            }
        }
      else
        {
          bool moved;
          bool killed = false;
          bool blessed = false;
          //debug("Player " << d_owner->getName() << " moving stack " << s->getId() << " at " <<s->getPos().x << "," << s->getPos().y << " towards a temple");
          moved = d_owner->AI_maybeVisitTempleForBlessing(s, s->getMoves(), 
                                                          50.0, blessed, 
                                                          killed);
          //debug("moved is " << moved << ", killed is " << killed << ", blessed is " << blessed);
          if (moved)
            {
              count++;
              if (!killed)
                {
                  groupStacks(s);
                  deleteStack(s);
                }
            }
        }
    }
  return count;
}

int AI_Allocation::visitRuins()
{
  int count = 0;
  Stacklist *sl = d_owner->getStacklist();
  std::list<Vector<int> > pos = sl->getPositions();
  for (std::list<Vector<int> >::iterator i = pos.begin(); i != pos.end(); ++i)
    {
      Stack *s = GameMap::getFriendlyStack(*i);
      if (!s)
        continue;
      if (s->hasHero())
        {
          bool moved;
          bool killed = false;
          bool visited_ruin = false;
          moved = d_owner->AI_maybeVisitRuin(s, s->getMoves(), 
                                             visited_ruin, killed);
          if (moved)
            {
              count++;
              if (!killed)
                {
                  groupStacks(s);
                  deleteStack(s);
                }
            }
        }
    }
  return count;
}

int AI_Allocation::pickupItems()
{
  int count = 0;
  //loop over all heroes
  std::list<Vector<int> > pos = d_owner->getStacklist()->getPositions();
  for (std::list<Vector<int> >::iterator i = pos.begin(); i != pos.end(); ++i)
    {
      Stack *s = GameMap::getFriendlyStack(*i);
      if (!s)
        continue;
      if (s->hasHero() && s->getParked() == false)
        {
          bool picked_up = false;
          bool killed = false;
          bool moved = d_owner->AI_maybePickUpItems(s, 8, picked_up, killed);
          if (moved)
            count++;
          if (!killed && moved)
            {
              groupStacks(s);
              deleteStack(s);
            }
        }
    }
  return count;
}

int AI_Allocation::move(City *first_city, bool take_neutrals)
{
  int attack_moved = 0, defensive_moved = 0, capacity_moved = 0, offensive_moved = 0, default_moved= 0;
  int temple_alloc = 0, ruin_alloc = 0, pickup_alloc = 0, attack_alloc = 0, quest_alloc = 0, immediate_alloc = 0, defensive_alloc = 0, capacity_alloc = 0, offensive_alloc = 0, default_alloc= 0;
  int moved;
  (void) attack_moved;
  (void) defensive_moved;
  (void) capacity_moved;
  (void) offensive_moved;
  (void) default_moved;
  // move stacks
  d_stacks = new StackReflist(d_owner->getStacklist(), true);

  debug("Player " << d_owner->getName() << " starts with " << d_stacks->size() << " stacks to do something with");

  int count = 0;

  if (d_owner->abortRequested())
    return count;

  // go on a quest
  quest_alloc = d_stacks->size();
  moved = continueQuests();
  quest_alloc -= d_stacks->size();
  debug("Player " << d_owner->getName() << " still has " << d_stacks->size() << " stacks after allocating stacks to fulfilling quests");
  debug("Player " << d_owner->getName() << " moved " << moved << " stacks in quest mode.");

  if (d_owner->abortRequested())
    return count;

  //move stacks to temples for blessing, or ones with heroes for a quest.
  temple_alloc = d_stacks->size();
  moved = visitTemples(GameScenarioOptions::s_play_with_quests != GameParameters::NO_QUESTING);
  temple_alloc -= d_stacks->size();
  debug("Player " << d_owner->getName() << " still has " << d_stacks->size() << " stacks after allocating stacks to visiting temples");
  debug("Player " << d_owner->getName() << " moved " << moved << " stacks in temple-visiting mode.");

  if (d_owner->abortRequested())
    return count;
  //move hero stacks to ruins for searching.
  ruin_alloc = d_stacks->size();
  moved = visitRuins();
  ruin_alloc -= d_stacks->size();
  debug("Player " << d_owner->getName() << " still has " << d_stacks->size() << " stacks after allocating stacks to visiting ruins");
  debug("Player " << d_owner->getName() << " moved " << moved << " stacks in ruin-visiting mode.");

  if (d_owner->abortRequested())
    return count;
  //if we're near a bag of stuff, go pick it up.
  pickup_alloc = d_stacks->size();
  moved = pickupItems();
  pickup_alloc -= d_stacks->size();
  debug("Player " << d_owner->getName() << " still has " << d_stacks->size() << " stacks after allocating stacks to picking up items");
  debug("Player " << d_owner->getName() << " moved " << moved << " stacks in pickup-items mode.");

  if (d_owner->abortRequested())
    return count;
  // if a stack has a path for an enemy city and is outside of a city, then keep going.
  attack_alloc = d_stacks->size();
  moved = continueAttacks();
  attack_moved = moved;
  attack_alloc -= d_stacks->size();
  debug("Player " << d_owner->getName() << " still has " << d_stacks->size() << " stacks after allocating stacks to continuing attacks");
  debug("Player " << d_owner->getName() << " moved " << moved << " stacks in continuing-attacks mode.");

  if (d_owner->abortRequested())
    return count;
  // if a stack is 2 tiles away from another enemy city, then attack it.
  immediate_alloc = d_stacks->size();
  moved = attackNearbyEnemies();
  immediate_alloc -= d_stacks->size();
  debug("Player " << d_owner->getName() << " still has " << d_stacks->size() << " stacks after allocating stacks to attacking nearby stacks");
  debug("Player " << d_owner->getName() << " moved " << moved << " stacks in attack-nearby-stacks mode.");

  if (d_owner->abortRequested())
    return count;
  //if (take_neutrals)
    {
      capacity_alloc = d_stacks->size();
      moved = allocateStacksToCapacityBuilding(first_city, take_neutrals);
      capacity_moved = moved;
      capacity_alloc -= d_stacks->size();
      debug("Player " << d_owner->getName() << " still has " << d_stacks->size() << " stacks after allocating stacks to capacity building");
      debug("Player " << d_owner->getName() << " moved " << moved << " stacks in capacity building mode.");
      count+=moved;
    }

  if (d_owner->abortRequested())
    return count;
  defensive_alloc = d_stacks->size();
  moved = allocateDefensiveStacks(Citylist::getInstance());
  defensive_moved = moved;
  defensive_alloc -= d_stacks->size();
  count +=moved;
  debug("Player " << d_owner->getName() << " has " << d_stacks->size() << " stacks after assigning defenders");
  debug("Player " << d_owner->getName() << " moved " << count << " stacks in defensive mode.");

  if (d_stacks->size() == 0)
    {
      delete d_stacks;
      return count;
    }

  if (d_owner->abortRequested())
    return count;
  offensive_alloc = d_stacks->size();
  moved = allocateStacksToThreats();
  offensive_moved = moved;
  offensive_alloc -= d_stacks->size();
  count+= moved;
  debug("Player " << d_owner->getName() << " still has " << d_stacks->size() << " stacks after allocating stacks to threats")
    debug("Player " << d_owner->getName() << " moved " << moved << " stacks in offensive mode.");

  if (d_stacks->size() == 0)
    {
      delete d_stacks;
      return count;
    }
      
  if (d_owner->abortRequested())
    return count;
  default_alloc = d_stacks->size();
  moved = defaultStackMovements();
  default_moved = moved;
  default_alloc -= d_stacks->size();
  debug("Player " << d_owner->getName() << " moved " << moved << " stacks in Default stack movements.");
  count+= moved;

  if (d_owner->abortRequested())
    return count;
  //empty out the cities damnit.
  emptyOutCities();
  debug("Player " << d_owner->getName() << " moved totals: " << attack_moved << "," << capacity_moved <<"," <<defensive_moved<<"," << offensive_moved <<"," << default_moved <<".");
  debug("Player " << d_owner->getName() << " alloc totals: " << attack_alloc << "," << capacity_alloc <<"," <<defensive_alloc<<"," << offensive_alloc <<"," << default_alloc <<" (" << total <<").");
  delete d_stacks;

  //if (IIId_owner->getId() == 0)
    //exit(0);
  return count;
}

int AI_Allocation::allocateDefensiveStacksToCity(City *city)
{
  int count = 0;
  float cityDanger = d_analysis->getCityDanger(city);
  //if city is not endangered, we keep a skeleton crew.
  //if a city has 10 strength in it, it's probably pretty safe.
  if (cityDanger < 3.0) 
    cityDanger = 3.0;
  else if (cityDanger > 10.0)
    cityDanger = 10.0;

  // Look how many defenders the city already has
  float totalDefenderStrength = 0.0;
  for (guint32 i = 0; i < city->getSize(); i++)
    for (guint32 j = 0; j < city->getSize(); j++)
      {
        Stack *d = GameMap::getFriendlyStack(city->getPos() + Vector<int>(i,j));
        if (d && d->getParked() == false)
          shuffleStacksWithinCity(city, d, Vector<int>(0,0));
      }
  std::vector<Stack*> defenders = city->getDefenders();
  for (std::vector<Stack*>::iterator it = defenders.begin(); it != defenders.end(); ++it)
    {
      Stack *defender = *it;
      if (defender->getParked() == true)
        continue;
      if (defender->fliesWithItemAndNonFlyersOverWaterOrMountains())
        continue;
      //shuffleStacksWithinCity(city, defender, Vector<int>(0,0));
      float stackStrength = d_analysis->assessStackStrength(defender);
      debug("Player " << d_owner->getName() << " assigns some or all of stack " << defender->getId() << " with strength " << stackStrength
            << " to " << city->getName() << " because its danger is " << cityDanger)

        totalDefenderStrength += stackStrength;
      if (totalDefenderStrength > cityDanger)
        {
          float diff = totalDefenderStrength - cityDanger;
          //we need to excise DIFF points from this stack.
          std::list<guint32> armies = (*it)->determineWeakArmies(diff);
          //split off some armies in this stack.
          if (armies.size() > 0 && armies.size() != defender->size())
            {
              Vector<int> dest = getFreeOtherSpotInCity(city, defender);
              if (dest != Vector<int>(-1,-1))
                {
                  Stack *stack = d_owner->stackSplitArmies(defender, 
                                                           armies);
                  if (stack->getMoves() > 0)
                    {
                      if (shuffleStack(stack, dest, false))
                        {
                          count++;
                          if (stack->getParked() == false)
                            d_stacks->addStack(stack);
                        }
                    }
                  else
                    groupStacks(defender);
                }
            }

          deleteStack(defender);
          break;
        }
      else
        {
          deleteStack(defender);
        }
      // if we get here, we have assigned defenders but not enough to
      // counter the danger that the city is in
    }

  // allocate nearby stacks to come back to the city,
  // because we don't have enough defence
  // this is disabled for now.
  while (totalDefenderStrength < cityDanger )
    {
      break;
      Stack *s = findClosestStackToCity(city);
      if (!s) 
        {
          debug("City " << city->getName() << " is endangered but no stacks are close enough to go defend it (or no more space available in city).")
            break;
        }
      debug("Stack " << s->getId() << " at " << s->getPos().x << "," << s->getPos().y << " should return to " << city->getName() << " to defend")
        Vector<int> dest = getFreeSpotInCity(city, s->size());
      if (dest == Vector<int>(-1,-1))
        break;
      if (s->fliesWithItemAndNonFlyersOverWaterOrMountains())
        break;
      float stackStrength = d_analysis->assessStackStrength(s);
      totalDefenderStrength += stackStrength;

      if (totalDefenderStrength > cityDanger)
        {
          float diff = totalDefenderStrength - cityDanger;
          //we need to excise DIFF points from this stack.
          std::list<guint32> armies = s->determineWeakArmies(diff);
          //split off some armies in this stack.
          if (armies.size() > 0 && armies.size() != s->size())
            {
              Stack *stack = d_owner->stackSplitArmies(s, armies);
              d_stacks->addStack(stack);
            }

        }
      deleteStack(s);

      bool killed = false;
      if (moveStack(s, dest, killed))
        {
          count++;
        }
    }

  return count;
}

// for each city, if it is in danger at all, allocate a stack to be its
// defender. These stacks sit in the NW corner and don't move out of the city.
int AI_Allocation::allocateDefensiveStacks(Citylist *cities)
{
  //we need to split the stacks and add the newly split ones to d_stacks.
  int count = 0;
  for (Citylist::iterator it = cities->begin(); it != cities->end(); ++it)
    {
      City *city = (*it);
      if (!city->isFriend(d_owner) || city->isBurnt())
	continue;
      count += allocateDefensiveStacksToCity(city);
      if (d_owner->abortRequested())
        return count;

    }
  return count;
}

int AI_Allocation::allocateStacksToThreat(Threat *threat)
{
  int count = 0;
  float threat_danger = threat->getDanger() * 1.000;
  if (threat_danger > 32.0) //e.g. 32 light infantry in a city.
    threat_danger = 32.0;
  City *city = GameMap::getCity(threat->getClosestPoint(Vector<int>(0,0)));
  while (true)
    {
      if (city && city->getOwner() == d_owner)
        break;
      guint32 num_city_defenders = 0;
      Stack *attacker = findBestAttackerFor(threat, num_city_defenders);
      //if (attacker && attacker->getId() == 4207)
        //{
          //printf("4207 was chosen for threat: `%s'\n", threat->toString().c_str());
        //}
      // if there is nobody to attack the threat, go onto the next one
      if (!attacker) 
        break;
      float score = d_analysis->assessStackStrength(attacker);
      bool killed = false;
      Vector<int> dest = threat->getClosestPoint(attacker->getPos());
      //if (attacker->getId() == 4207)
        //{
          //printf("dest is %d,%d\n", dest.x, dest.y);
          //exit(0);
        //}
      if (num_city_defenders == 0 || num_city_defenders - attacker->size() > 3)
        {

	debug("Player " << d_owner->getName() << " thinking about attacking threat at (" << dest.x <<"," << dest.y << ") with stack " << attacker->getId() <<" at ("<<attacker->getPos().x<<","<<attacker->getPos().y<<")");
        deleteStack(attacker);
          if (moveStack(attacker, dest, killed))
            {
              count++;
              if (!killed)
                {
                  if (attacker->isOnCity())
                    {
                      shuffleStacksWithinCity (GameMap::getCity(attacker), 
                                               attacker, Vector<int>(0,0));
                      //setParked(attacker, true);

                    }
                }
            }
        }
      else
        {
          std::list<guint32> armies = attacker->determineStrongArmies(3.0);
          if (armies.size() > 0 && armies.size() != attacker->size())
            {
              Stack *stack = d_owner->stackSplitArmies(attacker, armies);
              score = d_analysis->assessStackStrength(stack);
              debug("Player " << d_owner->getName() << " thinking about attacking threat at (" << dest.x <<"," << dest.y << ") with split stack " << stack->getId() <<" at ("<<stack->getPos().x<<","<<stack->getPos().y<<")");
              bool moved = moveStack(stack, dest, killed);
              if (!killed)
                {
                  if (stack->hasPath() == false)
                    d_stacks->addStack(stack);
                }
              if (moved)
                count++;
            }
        }

      threat_danger -= score;
      // if the threat has been removed, go onto the next one
      if (threat->getStrength() == 0.0)
        break;
      if (threat_danger <= 0)
        break;
    }
  return count;
}

int AI_Allocation::allocateStacksToThreats()
{
  int count = 0;

  for (Threatlist::const_iterator it = d_threats->begin(); 
       it != d_threats->end(); ++it)
    {
      if ((*it)->isCity())
        count += allocateStacksToThreat(*it);

      if (d_stacks->size() == 0)
        break;
      if (d_owner->abortRequested())
        return count;

    }
  return count;
}

Vector<int> AI_Allocation::getFreeOtherSpotInCity(City *city, Stack *stack)
{
  guint size = 0;
  Vector<int> best = Vector<int>(-1,-1);
  assert (city->contains(stack->getPos()) == true);
  for (unsigned int i = 0; i < city->getSize(); i++)
    for (unsigned int j = 0; j < city->getSize(); j++)
      {
	Vector<int> pos = city->getPos() + Vector<int>(i,j);
        if (pos == stack->getPos())
          continue;
	if (GameMap::canAddArmies(pos, stack->size()) == false)
	  continue;
        std::vector<Stack*> f = GameMap::getFriendlyStacks(pos);
        if (f.size() > 0)
          {
            for (std::vector<Stack*>::iterator k = f.begin(); k != f.end(); ++k)
              {
                if ((*k)->size() > size)
                  {
                    size = (*k)->size();
                    best = pos;
                  }
              }
          }
        else
          {
            if (size == 0)
              best = pos;
          }
      }
  return best;
}
Vector<int> AI_Allocation::getFreeSpotInCity(City *city, int stackSize)
{
  for (unsigned int i = 0; i < city->getSize(); i++)
    for (unsigned int j = 0; j < city->getSize(); j++)
      {
	Vector<int> pos = city->getPos() + Vector<int>(i,j);
	if (GameMap::canAddArmies(pos, stackSize) == false)
	  continue;
	return pos;
      }
  //there's no room in the inn.
  return Vector<int>(-1,-1);
}

Stack *AI_Allocation::findClosestStackToEnemyCity(City *city, bool try_harder)
{
  Stack *best = 0;
  int lowest_mp = -1;
  for (StackReflist::iterator it = d_stacks->begin(); it != d_stacks->end(); ++it)
    {
      Stack* s = *it;
      if (s->getParked() == true)
        continue;

      int tiles = dist(city->getPos(), s->getPos());
      if (tiles > 51)
	continue;
      int moves = (tiles + 6) / 7;

      if (try_harder == false && s->isOnCity())
        {
          City *source_city = GameMap::getCity(s);
          if (source_city)
            {
              if (d_analysis->getNumberOfDefendersInCity(source_city) <= 
                  (3 + 4))
                continue;
            }
        }

      if (moves < lowest_mp || lowest_mp == -1)
	{
	  best = s;
	  lowest_mp = moves;
	}
    }
  return best;
}

Stack *AI_Allocation::findClosestStackToCity(City *city)
{
  Stack *best = 0;
  int lowest_mp = -1;
  for (StackReflist::iterator it = d_stacks->begin(); it != d_stacks->end(); ++it)
    {
      Stack* s = *it;
      if (s->getParked() == true)
        continue;
      //don't consider the stack if it's already in the city
      Vector<int> spos = s->getPos();
      if (city->contains(spos))
	continue;
      //don't consider the city if we can't fit this stack anywhere inside it.
      Vector<int> dest = getFreeSpotInCity(city, s->size());
      if (dest == Vector<int>(-1, -1))
	continue;
      //don't consider the stack if it's in an endangered city
      City *source_city = GameMap::getCity(s);
      if (source_city)
	{
          if (d_analysis->getNumberOfDefendersInCity(source_city) <= 3)
            continue;
	}

      int tiles = dist(city->getPos(), s->getPos());
      int moves = (tiles + 6) / 7;
      if (moves < lowest_mp || lowest_mp == -1)
	{
	  best = s;
	  lowest_mp = moves;
	}
    }
  return best;
}

Stack *AI_Allocation::findBestAttackerFor(Threat *threat, guint32 &city_defenders)
{
  Stack *best = NULL;
  float best_score = -1.0;
  for (StackReflist::iterator it = d_stacks->begin(); it != d_stacks->end(); ++it)
    {
      Stack* s = *it;
      if (s->getParked() == true)
        continue;
      Vector<int> closestPoint = threat->getClosestPoint(s->getPos());
      // threat has been destroyed anyway
      if (closestPoint.x == -1)
	return 0;
      Vector<int> spos = s->getPos();

      int distToThreat = dist(closestPoint, spos);
      if (distToThreat > 27)
	continue;
      else if (distToThreat == 0)
        continue;

      //don't consider the stack if it's in an endangered city
      City *source_city = GameMap::getCity(s);
      guint32 num_source_city_defenders = 0;
      if (source_city)
	{
          num_source_city_defenders = 
            d_analysis->getNumberOfDefendersInCity(source_city);
          if (num_source_city_defenders <= (3 + 4))
	    continue;
	}

      int score = d_analysis->assessStackStrength(s);
      if (score > best_score || best_score == -1.0)
	{
	  best = s;
	  best_score = score;
          city_defenders = num_source_city_defenders;
	}
    }
  return best;
}

int AI_Allocation::defaultStackMovements()
{
  int count = 0;
  debug("Default movement for " <<d_stacks->size() <<" stacks");

  while (d_stacks->size() > 0)
    {
      if (d_owner->abortRequested())
        return count;
      Stack* s = d_stacks->front();
      debug("Player " << d_owner->getName() << " thinking about default movements for stack " << s->getId() <<" at ("<<s->getPos().x<<","<<s->getPos().y<<")");
      deleteStack(s);
      bool leave = false;

      City *source_city = GameMap::getCity(s);
      if (source_city)
        {
          if (s->isFull() &&
              source_city->countDefenders() - s->isFull() > 3)
            leave = true;
        }
      else
        leave = (Rnd::rand () % 100) > 98 ? false : true;

      if (leave == true)
        {
          bool moved = false;
          City* enemyCity = 
            Citylist::getInstance()->getNearestEnemyCity(s->getPos());
          if (enemyCity)
            {
              int mp = s->getPath()->calculate(s, enemyCity->getNearestPos(s->getPos()));
              debug("Player " << d_owner->getName() << " attacking " <<enemyCity->getName() << " that is " << mp << " movement points away");
              if (mp > 0)
                {
                  bool killed = false;
                  moved = moveStack(s, killed);
                  if (!killed)
                    {
                      if (s->isOnCity())
                        shuffleStacksWithinCity (GameMap::getCity(s), s, 
                                                 Vector<int>(0,0));
                      //setParked(s, true);
                    }
                  if (moved)
                    count++;
                }
            }
          else
            {
              enemyCity = 
                Citylist::getInstance()->getNearestForeignCity(s->getPos());
              if (enemyCity)
                {
                  s->getOwner()->proposeDiplomacy(Player::PROPOSE_WAR,
                                                  enemyCity->getOwner());
                  debug("Player " << d_owner->getName() << " attacking " <<enemyCity->getName())
                    int mp = s->getPath()->calculate(s, enemyCity->getNearestPos(s->getPos()));
                  if (mp > 0)
                    {
                      bool killed = false;
                      moved = moveStack(s, killed);
                      if (!killed)
                        {
                          if (s->isOnCity())
                            shuffleStacksWithinCity (GameMap::getCity(s), 
                                                     s, Vector<int>(0,0));
                          //setParked(s, true);
                        }
                      if (moved)
                        count++;
                    }
                }
            }

          if (!moved)
            {
              // for some reason (islands are one bet), we could not attack the
              // enemy city. Let's iterator through all cities and attack the first
              // one we can lay our hands on.
              debug("Mmmh, did not work.")
            }
        }
      else
        {
          bool moved;
          if (!source_city)
            moved = stackReinforce(s);
          else
            {
              City *c = source_city;
              debug("stack " << s->getId() <<" at ("<<s->getPos().x<<","<<s->getPos().y<<") shuffling in city " << c->getName());
              moved = shuffleStacksWithinCity (c, s, Vector<int>(0,0));
              //setParked(s, true);  valgrind doesn't like this.
            }

          if (moved)
            count++;
        }
      sbusy.emit();
    }
  return count;
}

bool AI_Allocation::stackReinforce(Stack *s)
{
  float mostNeeded = -1000.0;
  City *cityNeeds = NULL;
  Vector<int> target_tile = Vector<int>(-1,-1);
  for (auto city: *Citylist::getInstance())
    {
      if (city->getOwner() != d_owner)
        continue;
      if (city->isBurnt() == true)
        continue;
      int distToCity = dist(s->getPos(), city->getPos());

      //if the city already contains the given stack, then disregard it
      //hopefully it will be shuffled later
      if (city->contains(s->getPos()))
	return false;

      if (city->countDefenders () != 0)
        continue;

      //disregard if the city is too far away
      int movesToCity = (distToCity + 6) / 7;
      if (movesToCity > 2) continue;
      
      //disregard if the city can't hold our stack
      Vector<int> dest = getFreeSpotInCity(city, s->size());
      if (dest == Vector<int>(-1,-1))
	continue;

      //pick the city that needs us the most and is closer
      /*
       * movesToCity is a category 0, 1, or 2 .
       */
      float need = d_analysis->reinforcementsNeeded(city) *
        ((3 - movesToCity) * 7);
      if (need > mostNeeded)
	{
	  cityNeeds = city;
	  mostNeeded = need;
          target_tile = dest;
	}
    }

  if (cityNeeds) {
    debug("stack is sent to reinforce " << cityNeeds->getName() <<" if possible")
    // don't forget to send the stack to a free field within the city
    if (target_tile != Vector<int>(-1,-1))
      {
        guint32 m, t, l;
        PathCalculator pc (s);
        Path *p = pc.calculateToCity (cityNeeds, m, t, l);
        delete p;
        d_analysis->reinforce(cityNeeds, s, m);
        bool killed = false;
        bool moved = moveStack(s, target_tile, killed);
        return moved;
      }
  }

  return 0;
}

void AI_Allocation::searchRuin(Stack *stack, Ruin *ruin)
{
  bool stack_died = false;
  Reward *reward = d_owner->stackSearchRuin(stack, ruin, stack_died);
  if (reward && ruin->isSearched() == true && stack_died == false)
    {
      StackReflist *stacks = new StackReflist();
      d_owner->giveReward(stack, reward, stacks, false);
      delete reward;
      delete stacks;
    }
  // what to do if the ruin search fails?
}

bool AI_Allocation::shuffleStacksWithinCity(City *city, Stack *stack,
                                            Vector<int> diff)
{
  if (!city)
    return false;
  if (city->isBurnt() == true)
    {
      groupStacks(stack);
      return false;
    }
  Vector<int> target = city->getPos() + diff;
  if (stack->getPos() == target)
    {
      debug("stack " << stack->getId() <<" at ("<<stack->getPos().x<<","<<stack->getPos().y<<") already in preferred position.");
      std::vector<Stack*> f = GameMap::getFriendlyStacks(target);
      if (f.size() > 1)
        {
          groupStacks(stack);
          return false;
        }
      // already in the preferred position
      return false;
    }

  std::vector<Stack*> f = GameMap::getFriendlyStacks(target);
  if (f.size() > 1)
    {
      printf("i am stack %d at %d,%d\n", stack->getId(), stack->getPos().x, stack->getPos().y);
      printf("crap.  there are %lu stacks at %d,%d\n", f.size(), target.x, target.y);
      for (std::vector<Stack*>::iterator it = f.begin(); it != f.end(); ++it)
        {
          Stack *n = *it;
          if (n)
            printf("\tstack is %d\n", n->getId());
          else
            printf("\tstack is null\n");
        }
    }
  assert (f.size() <= 1);
  Stack *join = NULL;
  if (f.size() == 1)
    join = f.front();
  if (!join)
    {
      debug("no stack to land on.  just moving there.");
      bool moved = shuffleStack(stack, target, false);
      setParked(stack, true);
      return moved;
    }
  else if (GameMap::canJoin(stack, target))
    {
      debug("hey there's a stack to land on at (" <<target.x <<"," <<target.y<<").  moving there.");
      bool moved = shuffleStack(stack, target, false);
      setParked(stack, true);
      return moved;
    }
  else if (join->isFull())
    {
      debug("recursing now");
      //recurse, but prefer a different tile.
      if (diff == Vector<int>(0,0))
	diff = Vector<int>(0,1);
      else if (diff == Vector<int>(0,1))
	diff = Vector<int>(1,0);
      else if (diff == Vector<int>(1,0))
	diff = Vector<int>(1,1);
      else if (diff == Vector<int>(1,1))
	return false;
      return shuffleStacksWithinCity(city, stack, diff);
    }
  else
    {
      debug("alright, we're going to move what we can");
      bool moved = shuffleStack(stack, target, true);
      return moved;
    }
  return false;
}

bool AI_Allocation::shuffleStack(Stack *stack, Vector<int> dest, bool split_if_necessary)
{
  Stack *s = stack;
  assert (s != NULL);
  d_owner->getStacklist()->setActivestack(s);
  Path *p = new Path();
  p->push_back(dest);
  s->setPath(*p);
  delete p;
  if (s->enoughMoves())
    s->getPath()->setMovesExhaustedAtPoint(1);
  else
    s->getPath()->setMovesExhaustedAtPoint(0);
  bool moved;

  if (split_if_necessary)
    {
      //the new stack is left behind, and the current stack goes forward.
      Stack *new_stack = NULL;
      moved = d_owner->stackSplitAndMove(s, new_stack);
      if (new_stack)
        {
          if (moved)
            {
              groupStacks(s);
              setParked(s, true);
            }
          groupStacks(new_stack);
          setParked(new_stack, true);
          return moved;
        }
    }
  else
    moved = d_owner->stackMove(s);
      
  groupStacks(s);

  debug("shuffleStack on stack id " << s->getId() <<" has moved from " <<
        src.x << "," << src.y <<" to "
        << s->getPos().x << "," << s->getPos().y << ".");
  return moved;
}

bool AI_Allocation::moveStack(Stack *stack, bool &stack_died)
{
  Stack *s = stack;
  assert (s != NULL);
  d_owner->getStacklist()->setActivestack(s);
  bool moved;

  //printf("going in, size of path for stack: %d\n", s->getPath()->size());
  MoveResult *moveResult = d_owner->stackMove(s, Vector<int>(-1,-1));
  //printf("fight result is %d\n", moveResult->getFightResult());
  //printf("took steps? %d\n", moveResult->getStepCount());
  //printf("size of path for stack: %d\n", s->getPath()->size());
  //printf("reached end of path? %d\n", moveResult->getReachedEndOfPath());
  //printf("out of moves? %d\n", moveResult->getOutOfMoves());
  //printf("too large stack in the way? %d\n", moveResult->getTooLargeStackInTheWay());
  moved = moveResult->didSomething();
  delete moveResult;
  if (d_owner->getActivestack() == NULL)
    {
      debug("stack id " << stack_id << " died")
      stack_died = true;
    }
  else
    {
      groupStacks(s);
      stack_died = false;
      debug("Player " << d_owner->getName() << " moveStack on stack id " << s->getId() <<" has moved from " <<
            src.x << "," << src.y <<" to "
            << s->getPos().x << "," << s->getPos().y << ".  moved is " << moved << ".  moves left is " << s->getMoves() <<".");
    }
  return moved;
}

bool AI_Allocation::moveStack(Stack *stack, Vector<int> dest, bool &stack_died)
{
  Stack *s = stack;
  assert (s != NULL);
  int mp = s->getPath()->calculate(s, dest);
  if (mp <= 0)
    return false;
  return moveStack(s, stack_died);
}

void AI_Allocation::setParked(Stack *stack, bool force_park)
{
  if (!stack)
    return;
  if (force_park == false)
    {
      if (stack->hasPath() > 0 && stack->enoughMoves())
        d_owner->stackPark(stack);
      else if (stack->canMove() == false)
        d_owner->stackPark(stack);
    }
  else
    d_owner->stackPark(stack);
}

bool AI_Allocation::groupStacks(Stack *stack)
{
  Stack *s = stack;
  debug("groupStacks on stack id " << stack->getId() << " at pos (" << s->getPos().x <<"," <<s->getPos().y<<")");
  //which friendly stacks are on that tile that aren't us?
  std::vector<Stack*> stks = GameMap::getFriendlyStacks(s->getPos());
  if (stks.size() <= 1)
    {
      if (stks.front()->getId() != stack->getId())
        {
          printf("whoops\n");
          printf("expected stack id %d, but got %d\n", stack->getId(), stks.front()->getId());
          assert(0);
        }
      assert (stks.front()->getId() == stack->getId());
      setParked(s);
      return false;
    }
  GameMap::groupStacks(s);
  setParked(s);
  return true;
}

// End of file
