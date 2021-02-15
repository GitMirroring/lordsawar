// Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2004, 2006 Andrea Paternesi
// Copyright (C) 2004 John Farrell
// Copyright (C) 2006, 2007, 2008, 2009, 2014, 2015, 2017, 2021 Ben Asselstine
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

#include <fstream>
#include <vector>

#include "AI_Diplomacy.h"
#include "AI_Analysis.h"
#include "ai_fast.h"

#include "playerlist.h"
#include "armysetlist.h"
#include "stacklist.h"
#include "citylist.h"
#include "city.h"
#include "templelist.h"
#include "ruinlist.h"
#include "path.h"
#include "GameMap.h"
#include "Threatlist.h"
#include "action.h"
#include "xmlhelper.h"
#include "stack.h"
#include "GameScenarioOptions.h"
#include "hero.h"
#include "vectoredunitlist.h"
#include "PathCalculator.h"
#include "stacktile.h"
#include "armyprodbase.h"
#include "QuestsManager.h"
#include "Quest.h"
#include "SightMap.h"
#include "Sage.h"
#include "rnd.h"
#include "GameScenarioOptions.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::flush<<std::endl;}
#define debug(x)

AI_Fast::AI_Fast(Glib::ustring name, guint32 armyset,
                 std::vector<Gdk::RGBA> colors, int width, int height,
                 int player_no)
    :RealPlayer(name, armyset, colors, width, height, Player::AI_FAST, player_no), d_join(true),
    d_maniac(false), d_analysis(0), d_diplomacy(0)
{
}

AI_Fast::AI_Fast(const Player& player, bool sync_ids)
    :RealPlayer(player, sync_ids), d_join(true), d_maniac(false),
    d_analysis(0), d_diplomacy(0)
{
    d_type = AI_FAST;
}

AI_Fast::AI_Fast(XML_Helper* helper)
    :RealPlayer(helper), d_analysis(0), d_diplomacy(0)
{
    helper->getData(d_join, "join");
    helper->getData(d_maniac, "maniac");
}

AI_Fast::~AI_Fast()
{
    if (d_analysis)
        delete d_analysis;
    if (d_diplomacy)
        delete d_diplomacy;
}

bool AI_Fast::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(Player::d_tag);
    retval &= helper->saveData("join", d_join);
    retval &= helper->saveData("maniac", d_maniac);
    retval &= Player::save(helper);
    retval &= helper->closeTag();

    return retval;
}

void AI_Fast::abortTurn()
{
  abort_requested = true;
  if (surrendered)
    aborted_turn.emit();
  else if (Playerlist::getInstance()->countPlayersAlive() == 1)
    aborted_turn.emit();
}

bool AI_Fast::startTurn()
{
    sbusy.emit();
    if (getStacklist()->getHeroes().size() == 0 &&
        Citylist::getInstance()->countCities(this) == 1)
      {
        City *first = getFirstCity();
        if (first)
          AI_maybeBuyScout(first);
        else
          {
            fprintf(stderr,"%s : we have 1 city but no first city?  impossible\n", getName().c_str());
            for (auto c: *Citylist::getInstance())
              {
                if (c->getOwner() == this)
                  {
                    fprintf(stderr, "Our city is %d\n", c->getId());
                  }
              }
            exit (0);
          }
      }

    debug(getName() << ": AI_Fast::start_turn")
    debug("being in " <<(d_maniac?"maniac":"normal") <<" mode")
    debug((d_join?"":"not ") <<"joining armies")

    d_analysis = new AI_Analysis(this);
    d_diplomacy = new AI_Diplomacy(this);

    d_diplomacy->considerCuspOfWar();
    
    d_maniac = false;
    float ratio = 2.0;
    if (getUpkeep() > getIncome() * ratio)
      d_maniac = true;

    //setup production
    debug("examining cities");
    for (auto c: *Citylist::getInstance())
      {
        if (c->getOwner() != this || c->isBurnt())
          continue;
        if (c->getActiveProductionSlot() == -1)
          setBestProduction(c);
      }

    //setup vectoring
    debug("setting up vectoring");
    if (!d_maniac)
	AI_setupVectoring(18, 3, 30);

    debug("trying to complete quests");
    //try to complete our quests
    std::vector<Quest*> q = QuestsManager::getInstance()->getPlayerQuests(this);
    for (std::vector<Quest*>::iterator it = q.begin(); it != q.end(); ++it)
      {
        Quest *quest = *it;
        if (quest->isPendingDeletion())
          continue;
        Stack *s = getStacklist()->getArmyStackById(quest->getHeroId());
        if (!s)
          continue;
        bool stack_died = false;
        bool quest_completed = false;
        bool stack_moved = AI_maybeContinueQuest(s, quest, quest_completed, 
                                                 stack_died);

        if (stack_moved == true && stack_died == false)
          GameMap::groupStacks(s);
      }

    while (computerTurn() == true)
      {
	bool found = false;
    
	//are there any stacks with paths that can move?
	for (Stacklist::reverse_iterator it = d_stacklist->rbegin(); it != d_stacklist->rend(); ++it)
	  {
	    Stack *s = (*it);
	    if (s->getPath()->size() > 0 && s->enoughMoves())
	      {
		int mp = s->getPath()->calculate(s, s->getLastPointInPath());
		if (mp <= 0)
		  continue;
		debug ("AI_FAST stack " << s->getId() << " can still potentially move");
		debug ("moving from (" << s->getPos().x << "," << s->getPos().y
		       << ") to (" <<s->getFirstPointInPath().x << "," <<
		       s->getFirstPointInPath().y << ") with " << s->getMoves() <<" left");

	    
		found = true;
	      }
	    //are there any stacks without paths that still have some moves?
	    else if (s->getPath()->size() == 0 && s->getMoves() > 1)
	      found = true;
	  }
	sbusy.emit();
	if (!found)
	  break;
	if (found)
	  found = false;
	if (abort_requested)
	  break;
      }
    parkAllStacks();
    sbusy.emit();
    Glib::usleep (50000);
    while (g_main_context_iteration(NULL, FALSE)); //doEvents
    Glib::usleep (50000);

    delete d_analysis;
    d_analysis = 0;

    d_stacklist->setActivestack(0);

    // Declare war with enemies, make peace with friends
    if (GameScenarioOptions::s_diplomacy)
      d_diplomacy->makeProposals();

    if (abort_requested)
      aborted_turn.emit();
    return !(Playerlist::getInstance()->getNoOfPlayers() <= 1);
}

int AI_Fast::scoreArmyType(const ArmyProdBase *a)
{
  int max_strength = a->getStrength();

  int production;
  if (a->getProduction() == 1)
    production = 6;
  else if (a->getProduction() == 2)
    production = 2;
  else
    production = 1;

  int upkeep = 0;
  if (a->getUpkeep() < 5)
    upkeep = 6;
  else if (a->getUpkeep() < 10)
    upkeep = 2;
  else 
    upkeep = 1;

  int newcost = 0;
  if (a->getProductionCost() < 5)
    newcost = 6;
  else if (a->getProductionCost() < 10)
    newcost = 2;
  else
    newcost = 1;

  //we prefer armies that move farther
  int move_bonus = 0;
  if (a->getMaxMoves() >  10)
    move_bonus += 2;
  if (a->getMaxMoves() >=  20)
    move_bonus += 4;

  return max_strength + move_bonus + production + upkeep + newcost;
}

int AI_Fast::setBestProduction(City *c)
{
  int select = -1;
  int score = -1;

  // we try to determine the most attractive basic production
  for (guint32 i = 0; i < c->getMaxNoOfProductionBases(); i++)
    {
      if (c->getArmytype(i) == -1)    // no production in this slot
        continue;

      const ArmyProdBase *proto = c->getProductionBase(i);
      if (scoreArmyType(proto) > score)
        {
          select = i;
          score = scoreArmyType(proto);
        }
    }


  if (select != c->getActiveProductionSlot())
    {
      cityChangeProduction(c, select);
      debug(getName() << " Set production to slot " << select << " in " << c->getName())
    }

  return c->getActiveProductionSlot();
}

void AI_Fast::invadeCity(City* c)
{
  CityDefeatedAction action = CITY_DEFEATED_OCCUPY;
  bool quest_preference = AI_invadeCityQuestPreference(c, action);
  debug("Invaded city " <<c->getName());

  if (quest_preference == false)
    {
      if (getIncome() < getUpkeep())
        action = CITY_DEFEATED_OCCUPY;
      else if (d_maniac &&
               GameScenarioOptions::s_razing_cities != GameParameters::NEVER)
        action = CITY_DEFEATED_RAZE;
      else
        action = CITY_DEFEATED_OCCUPY;
    }
  int gold = 0;
  int pillaged_army_type = -1;
  std::list<guint32> sacked_army_types;
  switch (action)
    {
    case CITY_DEFEATED_OCCUPY:
      cityOccupy(c);
      setBestProduction(c);
      break;
    case CITY_DEFEATED_PILLAGE:
      cityPillage(c, gold, &pillaged_army_type);
      AI_maybeBuyScout(c);
      setBestProduction(c);
      break;
    case CITY_DEFEATED_RAZE:
      cityRaze(c);
      break;
    case CITY_DEFEATED_SACK:
      citySack(c, gold, &sacked_army_types);
      AI_maybeBuyScout(c);
      setBestProduction(c);
      break;
    }
}

void AI_Fast::heroGainsLevel(Hero * a)
{
    debug("Army raised a level, id = " <<a->getId())
    
    //advancing a level
    // increase the strength attack (uninnovative, but enough here)
    Army::Stat stat = Army::STRENGTH;
    doHeroGainsLevel(a, stat);
    addAction(new Action_Level(a, stat));
}

Stack *AI_Fast::findNearOwnStackToJoin(Stack *src, int max_distance)
{
  int min_mp = -1;
  std::vector<Stack*> stks =
    GameMap::getNearbyFriendlyStacks(src->getPos(), max_distance);
  if (stks.size() <= 1)
    return NULL;
  PathCalculator pc(src);
  Stack* target = NULL;
  for (auto dest : stks)
    {
      //is this us?
      if (src == dest)
        continue;

      //does the destination have few enough army units to join?
      if (GameMap::canJoin(src, dest) == false)
        continue;

      //is this a stack that is co-located?
      if (src->getPos() == dest->getPos())
        return dest;

      //can we actually get there?
      int mp = pc.calculate(dest->getPos());
      if (mp <= 0)
        continue;

      if (mp < min_mp || min_mp == -1)
        {
          target = dest;
          min_mp = mp;
        }
    }
  return target;
}

bool AI_Fast::computerTurn()
{
  bool stack_moved = false;
    // we have configurable behaviour in two ways:
    // 1. join => merges close stacks
    // 2. maniac => attack at any costs, raze everything in the path
    //
    // So the basic algorithm is like
    // for all armies
    //      if !maniac, and close to a temple, visit it
    //      if (army_to_join close && d_join)
    //          join armies
    //      if (!maniac && army_damaged)
    //          resupply
    //      if (!maniac)
    //          find next enemy city
    //      else
    //          find next enemy unit with preference to cities
    //      attack
    //
    // return true if any stack moved

    // we are using reversed order because new stacks come behind old stacks
    // and we want the freshly created stacks join the veterans and not the other
    // way round.
 
  std::list<Vector<int> > points = d_stacklist->getPositions();
  for (auto it: points)
    {
      Stack *s = GameMap::getFriendlyStack(it);
      if (!s)
        continue;

      d_stacklist->setActivestack(s);

      //move stacks to enemy cities.
      if (s->hasPath() == true && s->getParked() == false)
        {
          Vector<int> pos = s->getLastPointInPath();
          City *enemy = GameMap::getEnemyCity(pos);
          if (enemy)
            {
              if (enemy->isBurnt() == false)
                {
                  stack_moved |= stackMove(s);
                  if (d_stacklist->getActivestack() == NULL)
                    return true;
                  if (stack_moved)
                    continue;
                }
            }
        }

      //go to a temple or ruin
      if (!d_maniac)
        {
          bool stack_died = false;
          bool blessed = false;
          if (s->hasHero() == false)
            {
              stack_moved = AI_maybeVisitTempleForBlessing
                (s, s->getMoves(), 50.0, blessed, stack_died);
              if (stack_died)
                return true;
              s = d_stacklist->getActivestack();
              if (stack_moved)
                {
                  GameMap::groupStacks(s);
                  s->clearPath();
                  continue;
                }
            }
          else if (s->hasHero() == true)
            {
              bool got_quest = false;
              stack_moved = AI_maybeVisitTempleForQuest(s, s->getMoves(), 
                                                        got_quest, 
                                                        stack_died);
              if (stack_died)
                return true;
              if (!stack_moved)
                {
                  bool ruin_visited = false;
                  stack_moved = AI_maybeVisitRuin (s, s->getMoves(), 
                                                   ruin_visited, stack_died);
                  if (stack_died)
                    return true;
                }
              s = d_stacklist->getActivestack();
              if (stack_moved)
                {
                  GameMap::groupStacks(s);
                  s->clearPath();
                  continue;
                }
            }
        }

      //pick up items
      if (!d_maniac)
        {
          bool stack_died = false;
          bool picked_up = false;

          stack_moved = AI_maybePickUpItems(s, s->getMoves(), picked_up, 
                                            stack_died);
          if (stack_died)
            return true;
          s = d_stacklist->getActivestack();
          if (picked_up && stack_moved)
            stack_moved = false; //do this so we move it later on
          else if (stack_moved)
            continue;
        }

      debug(">>>> What to do with stack " <<s->getId() <<" at (" <<s->getPos().x
            <<"," <<s->getPos().y <<") containing " <<s->size() << " armies ?")

        // join armies if close
        if (d_join && s->isFull() == false)
          {
            Stack* target = NULL;
            target = findNearOwnStackToJoin(s, 5);

            if (target)
              {
                debug("Joining with stack " <<target->getId() <<" at (" <<target->getPos().x <<"," <<target->getPos().y <<")")
                  s->getPath()->calculate(s, target->getPos());
                stack_moved |= stackMove(s);
                //in case we lost our stack
                if (!d_stacklist->getActivestack())
                  return true;
                if (s->getPos() == target->getPos())
                  {
                    GameMap::groupStacks(s);
                    continue;
                  }
                continue;
              }
          }

      // second step: try to resupply
      if (!d_maniac)
        {
          City *target = Citylist::getInstance()->getNearestFriendlyCity(s->getPos());
          if (s->isFull() == false && target)
            {
              debug("Restocking in " <<target->getName())
                // try to move to the north west part of the city (where the units
                // move after production), otherwise just wait and stand around

                if (target->contains(s->getPos()) == false)
                  {
                    debug("Stack is not in " << target->getName() << " yet" <<std::endl);
                    int mp = s->getPath()->calculateToCity(s, target);
                    if (mp > 0)
                      {
                        stack_moved |= stackMove(s);

                        // the stack could have joined another stack waiting there
                        if (!d_stacklist->getActivestack())
                          return true;
                        if (stack_moved)
                          {
                            GameMap::groupStacks(s);
                            s->clearPath();
                            continue;
                          }
                      }
                  }
                else if (s->getPos() != target->getPos())
                  {
                    debug("Stack is inside " << target->getName() << std::endl);
                    //if we're not in the upper right corner
                    s->getPath()->calculate(s, target->getPos());
                    //go there, and take as many as we can
                    Stack *new_stack = NULL;
                    stack_moved |= stackSplitAndMove(s, new_stack);
                    //in case we lost our stack
                    if (!d_stacklist->getActivestack())
                      return true;
                    if (stack_moved)
                      {
                        GameMap::groupStacks(s);
                        s->clearPath();
                        GameMap::groupStacks(target->getPos());
                        return true;
                      }
                  }
                else
                  {
                    //otherwise just stay put in the city
                    GameMap::groupStacks(s);
                    continue;
                  }
            }

          // third step: non-maniac players attack only enemy cities
          else
            {
              target = NULL;
              PathCalculator pc(s, true, 10, -1);
              guint32 moves1 = 0, turns1 = 0, moves2 = 0, turns2 = 0;
              guint32 left1 = 0, left2 = 0;
              Path *target1_path = NULL;
              Path *target2_path = NULL;
              City *target1;
              if (Rnd::rand() % 3 == 0)
                target1 = Citylist::getInstance()->getClosestEnemyCity(s);
              else
                target1 = Citylist::getInstance()->getNearestEnemyCity(s->getPos());
              City *target2 = Citylist::getInstance()->getNearestForeignCity(s->getPos());
              if (target1)
                target1_path = pc.calculateToCity(target1, moves1, turns1, left1);
              else
                target1_path = new Path();
              if (!target2)
                {
                  delete target1_path;
                  return false; //it's game over and we're still moving
                }
              target2_path = pc.calculateToCity(target2, moves2, turns2, left2);

              //no enemies?  then go for the nearest foreign city.
              //if diplomacy isn't on and we hit this, then it's game over
              if (!target1)
                target = target2;

              //is the enemy city far enough away that a foreign city
              //outweighs it?
              else if (target1_path->size() / 13 > target2_path->size())
                target = target2;
              else
                target = target1;
              delete target1_path;
              delete target2_path;

              if (target == target2)
                {
                  if (GameScenarioOptions::s_diplomacy == true)
                    d_diplomacy->needNewEnemy(target->getOwner());
                  // try to wait a turn until we're at war
                  if (target1)
                    target = target1;
                }

              if (!target)    // strange situation
                {
                  std::cerr << "yet another bad situation!!" << std::endl;
                  stackPark(s);
                  return true;
                }

              debug("Attacking " << target->getName() << " (" << 
                    target->getPos().x <<","<< target->getPos().y << ")")
                int moves = s->getPath()->calculateToCity(s, target);
              debug("Moves to enemy city: " << moves);

              if (moves >= 1)
                {
                  stack_moved |= stackMove(s);
                  s = d_stacklist->getActivestack();
                  if (!d_stacklist->getActivestack())
                    return true;
                  //if we didn't get there
                  if (target->getOwner() != s->getOwner())
                    {
                      //and the target city is empty
                      if (target->countDefenders() == 0)
                        {
                          //attack it if we can reach it.
                          Stack *new_stack = NULL;
                          int moved = stackSplitAndMove(s, new_stack);
                          stack_moved |=  moved;
                          if (moved)
                            {
                              //either s or new_stack could be dead.
                              if (d_stacklist->getActivestack() != NULL)
                                GameMap::groupStacks(s);
                              GameMap::groupStacks(target->getPos());
                              return true;
                            }
                        }
                    }
                }
              else
                {
                  // an enemy city is completely surrouned by other stacks, or the way is blocked by a signle enemy stack
                  //let us presume this is temporary and just leave the stack here
                  //for some reason we can't set parked on this thing
                  //and have it realize it, after we return true.
                  //why is that?
                  printf("crap, it happened with a stack at %d,%d\n", s->getPos().x, s->getPos().y);
                  printf("moves is %d\n", moves);
                  printf("Destination was %d,%d (%s)\n", target->getPos().x, target->getPos().y, target->getName().c_str());
                  stackDisband(s);
                  return true;
                }

              // a stack has died ->restart
              if (!d_stacklist->getActivestack())
                return true;

              continue;
            }
        }


      // fourth step: maniac players attack everything that is close if they can
      // reach it or cities otherwise.
      if (d_maniac)
        {
          const Threatlist* threats = d_analysis->getThreatsInOrder(s->getPos());
          const Threat* target = 0;

          // prefer weak forces (take strong if neccessary) and stop after 10
          // stacks
          int i = 0;
          for (Threatlist::const_iterator tit = threats->begin();
               tit != threats->end() && i < 10; ++tit, i++)
            {
              // in a first step, we only look at enemy stacks
              if ((*tit)->isCity() || (*tit)->isRuin())
                continue;

              // ignore stacks out of reach
              Vector<int> threatpos = (*tit)->getClosestPoint(s->getPos());
              if (threatpos == Vector<int>(-1, -1))
                continue;

              guint32 mp = s->getPath()->calculate(s, threatpos);
              if ((int)mp <= 0 || mp > s->getMoves())
                continue;

              target = *tit;
              break;

            }

          // now we need to choose. If we found a target, attack it, otherwise
          // attack the closest city.
          Vector<int> pos = Vector<int>(-1,-1);
          if (target)
            {
              pos = target->getClosestPoint(s->getPos());
              debug("Maniac mode, found target at (" <<pos.x <<"," <<pos.y <<")")
            }
          else
            {
              City *enemy_city = 
                Citylist::getInstance()->getNearestForeignCity(s->getPos());
              if (enemy_city)
                {
                  pos  = enemy_city->getPos();
                  debug("Maniac, found no targets, attacking city " << enemy_city->getName() << " at (" <<pos.x <<"," <<pos.y <<")")
                }
            }

          if (pos == Vector<int>(-1,-1))
            return false;

          int mp = s->getPath()->calculate(s, pos);
          if (mp > 0)
            {
              //printf ("stack %d at %d,%d moving %d with %d moves\n",
              //s->getId(), s->getPos().x, s->getPos().y,
              //mp, s->getMoves());
              bool moved = stackMove(s);
              //printf("result of move: %d\n", moved);
              stack_moved |= moved;
              //in case we lost our stack
              if (!d_stacklist->getActivestack())
                return true;
              s = d_stacklist->getActivestack();
            }
          else
            {
              printf ("we're going the wrong way (mp is %d)!!\n", mp);
              printf ("this means we couldn't calculate a path from %d,%d to %d,%d\n", s->getPos().x, s->getPos().y, pos.x, pos.y);
              //sleep (10);
              City *friendly_city = 
                Citylist::getInstance()->getNearestFriendlyCity(s->getPos());
              if (friendly_city)
                {
                  mp = s->getPath()->calculate(s, friendly_city->getPos());
                  if (mp > 0)
                    {
                      stack_moved |= stackMove(s);
                      //in case we lost our stack
                      if (!d_stacklist->getActivestack())
                        return true;
                    }
                  else
                    stack_moved |= false;
                }
              else
                {
                  //we can't find anyplace to move to!
                  //so we stay put.
                  stack_moved |= false;
                }
            }

          if (!d_stacklist->getActivestack())
            return true;
          continue;

        }
      if (abort_requested)
        break;
    }
    return stack_moved;
}

bool AI_Fast::chooseTreachery (Stack *stack, Player *player, Vector <int> pos)
{
  (void) stack;
  (void) player;
  (void) pos;
  return true;
}

bool AI_Fast::chooseHero(HeroProto *hero, City *city, int gold)
{
  (void) hero;
  (void) city;
  (void) gold;
  return true;
}

Reward *AI_Fast::chooseReward(Ruin *ruin, Sage *sage, Stack *stack)
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

Army::Stat AI_Fast::chooseStat(Hero *hero)
{
  (void) hero;
  return Army::STRENGTH;
}

bool AI_Fast::chooseQuest(Hero *hero)
{
  (void) hero;
  return true;
}

bool AI_Fast::computerChooseVisitRuin(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) turns;
  if (stack->getPos() == dest)
    return true;
  if (moves < stack->getMoves() + 15)
    return true;
  else 
    return false;
}

bool AI_Fast::computerChoosePickupBag(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) turns;
  if (stack->getPos() == dest)
    return true;
  if (moves < stack->getMoves() + 7)
    return true;
  else 
    return false;
}

bool AI_Fast::computerChooseVisitTempleForBlessing(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) turns;
  if (stack->isOnCity() == true)
    return false;
  if (stack->getPos() == dest)
    return true;
  if (moves < stack->getMoves() + 7)
    return true;
  else 
    return false;
}

bool AI_Fast::computerChooseVisitTempleForQuest(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) turns;
  if (stack->isOnCity() == true)
    return false;
  if (stack->getPos() == dest)
    return true;
  if (moves < stack->getMoves() + 15)
    return true;
  else 
    return false;
}

bool AI_Fast::computerChooseContinueQuest(Stack *stack, Quest *quest, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) stack;
  (void) quest;
  (void) dest;
  (void) moves;
  (void) turns;
  return true;
}
// End of file
