//  Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
//  Copyright (C) 2003 Michael Bartl
//  Copyright (C) 2004, 2006 Andrea Paternesi
//  Copyright (C) 2004 John Farrell
//  Copyright (C) 2006, 2007, 2008, 2009, 2014, 2015, 2017, 2021,
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

#include <fstream>
#include <vector>

#include "ai-diplomacy.h"
#include "ai-analysis.h"
#include "ai-fast.h"

#include "player-list.h"
#include "army-set-list.h"
#include "stack-list.h"
#include "city-list.h"
#include "city.h"
#include "temple-list.h"
#include "ruin-list.h"
#include "path.h"
#include "game-map.h"
#include "threat-list.h"
#include "action.h"
#include "xml-helper.h"
#include "stack.h"
#include "game-scenario-options.h"
#include "hero.h"
#include "vectored-unit-list.h"
#include "path-calculator.h"
#include "stack-tile.h"
#include "army-prod-base.h"
#include "quest-manager.h"
#include "quest.h"
#include "sight-map.h"
#include "sage.h"
#include "rnd.h"
#include "move-result.h"
#include "boon.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::flush<<std::endl;}
#define debug(x)

AI_Fast::AI_Fast(Glib::ustring name, guint32 armyset,
                 Shield::Color shield, int width, int height)
    :RealPlayer(name, armyset, shield, width, height, Player::AI_FAST), d_join(true),
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
    helper->get(d_join, "join");
    helper->get(d_maniac, "maniac");
}

AI_Fast::~AI_Fast()
{
    if (d_analysis)
        delete d_analysis;
}

bool AI_Fast::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->open_tag(Player::d_tag);
    retval &= helper->save("join", d_join);
    retval &= helper->save("maniac", d_maniac);
    retval &= saveContents(helper);
    retval &= helper->close_tag();

    return retval;
}

void AI_Fast::abortTurn()
{
  abort_requested = true;
  if (surrendered)
    aborted_turn.emit();
  else if (Playerlist::instance()->countPlayersAlive() == 1)
    aborted_turn.emit();
}

void AI_Fast::startTurn (sigc::slot<void(bool)> finish)
{
  sbusy.emit ();
  if (getStacklist ()->getHeroes ().size () == 0 &&
      Citylist::instance ()->countCities (this) == 1)
    {
      City *first = getFirstCity ();
      if (first)
        AI_maybeBuyScout (first);
      else
        {
          fprintf (stderr,
                   "%s : we have 1 city but no first city?  impossible\n",
                   getName ().c_str ());
          for (auto c: *Citylist::instance ())
            {
              if (c->getOwner () == this)
                {
                  fprintf (stderr, "Our city is %d\n", c->getId ());
                }
            }
          exit (0);
        }
    }

  d_analysis = new AI_Analysis (this);
  d_diplomacy = new AI_Diplomacy (this);

  d_diplomacy->considerCuspOfWar ();

  d_maniac = false;
  float ratio = 2.0;
  if (getUpkeep () > getIncome () * ratio)
    d_maniac = true;

  //setup production
  for (auto c: *Citylist::instance ())
    {
      if (c->getOwner () != this || c->isBurnt ())
        continue;
      if (c->getActiveProductionSlot () == -1)
        setBestProduction (c);
    }

  //setup vectoring
  if (!d_maniac)
    AI_setupVectoring (18, 3, 30);

  initComputerTurn ();
  computerTurn
    ([this, finish] ()
     {
       parkAllStacks ();
       sbusy.emit ();

       delete d_analysis;
       d_analysis = 0;

       stackDeselect ();

       // Declare war with enemies, make peace with friends
       if (GameScenarioOptions::s_diplomacy)
         d_diplomacy->makeProposals ();
       delete d_diplomacy;

       if (abort_requested)
         aborted_turn.emit();
       else
         finish (!(Playerlist::instance ()->getNoOfPlayers () <= 1));
     });

  return;
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
  AI_maybeBuyScout(c);
  setBestProduction(c);
}

CityDefeatedChoice AI_Fast::chooseCityDefeatedAction (City *c, Stack *s)
{
  (void) s;
  CityDefeatedChoice action = CITY_DEFEATED_OCCUPY;
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
  return action;
}

void AI_Fast::heroGainsLevel(Hero * a, Army::Stat stat)
{
    debug("Army raised a level, id = " <<a->getId())
    
    //advancing a level
    // increase the strength attack (uninnovative, but enough here)
    stat = Army::MOVES;
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
      int mp = pc.calculateMoves (dest->getPos());
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

void AI_Fast::initComputerTurn ()
{
  m_stack_points = d_stacklist->getPositions ();
  m_stack_points_iterator = m_stack_points.begin ();
  m_dirty = false;
}

Stack* AI_Fast::get_next_stack_from_list ()
{
  while (1)
    {
      if (m_stack_points_iterator == m_stack_points.end ())
        return NULL;

      Stack *s = GameMap::getFriendlyStack (*m_stack_points_iterator);

      m_stack_points_iterator++;

      if (s)
        {
          stackSelect (s);
          return s;
        }
    }

  return NULL;
}

void AI_Fast::step1 (Stack *s, sigc::slot<void()> next)
{
  if (abort_requested)
    return;
  bool moving = false;
  //move stacks to enemy cities.
  if (!m_did_something && s && s->getParked () == false &&
      GameMap::isStackDestinationEnemyCity (s))
    {
      moving = true;
      stackMove
        (s,
         [this, next] (MoveResult *res)
         {
           bool moved = res->getStepCount () > 0;
           bool fought = res->fought ();
           delete res;
           //if we moved or died we don't have to try any other steps
           if (moved || fought)
             m_did_something = true;
           next ();
         });
    }
  if (!moving)
    next ();
}

bool AI_Fast::should_do_boon (const Boon *b) const
{
  if (d_maniac)
    {
      if (b->is_city () && b->get_turns_away () < 2) 
        return true;
      else if (b->is_backpack () && b->get_turns_away () < 4) 
        return true;
      else if (b->is_quest () && b->get_turns_away () < 10) 
        return true;
    }
  else
    {
      if (b->is_city () && b->get_turns_away () < 1) 
        return true;
      else if (b->is_temple () && b->get_turns_away () < 2)
        return true;
      else if (b->is_ruin () && b->get_turns_away () < 3)
        return true;
      else if (b->is_backpack () && b->get_turns_away () < 4) 
        return true;
      else if (b->is_quest () && b->get_turns_away () < 10) 
        return true;
    }

  return false;
}

void AI_Fast::step2 (Stack *s, sigc::slot<void()> next)
{
  if (abort_requested)
    return;
  if (!m_did_something)
    {
      auto boons = d_analysis->getBoonsInOrder (s);
       auto it =
         std::find_if
         (boons.begin (), boons.end (),
          [this](const Boon b)
          {
            return should_do_boon (&b);
          });

       if (it == boons.end ())
         next ();
       else
         {
           Boon boon = (*it);
           Vector<int> dest = boon.get_destination (s);
           if (s->calculatePath (dest))
             {
               stackMove
                 (s,
                  [this, s, boon, dest, next] (MoveResult *res)
                  {
                    bool moved = res->getStepCount ();
                    bool fought = res->fought ();
                    bool died = res->is_alive () == false;
                    delete res;
                    if (moved || fought)
                      m_did_something = true;
                    if (!died)
                      {
                        if (s->getPos () == dest)
                          {
                            if (moved)
                              m_did_something = true;
                            GameMap::groupStacks (s);
                            if (boon.is_city ())
                              {
                                // nothing to do.
                                next ();
                              }
                            else if (boon.is_temple ())
                              {
                                svisitingTemple.emit
                                  (s,
                                   [this, next, moved] (bool quest, int blessed)
                                   {
                                     if (moved || quest || blessed)
                                       m_did_something = true;
                                     next ();
                                   });
                              }
                            else if (boon.is_ruin ())
                              {
                                ssearchingRuin.emit
                                  (s,
                                   [this, next] (bool /*died */)
                                   {
                                     m_did_something = true;
                                     next ();
                                   });
                              }
                            else if (boon.is_backpack ())
                              {
                                Hero *hero =
                                  static_cast<Hero*>(s->getFirstHero ());
                                if (hero)
                                  {
                                    if (heroPickupAllItems (hero, s->getPos ()))
                                      m_did_something = true;
                                    next ();
                                  }
                                else
                                  next ();
                              }
                            else if (boon.is_quest ())
                              {
                                // nothing to do.
                                // we complete a quest by killing a stack or a
                                // city.  all that happens in stackMove.
                                next ();
                              }
                          }
                        else
                          next ();
                      }
                    else //died
                      {
                        m_did_something = true;
                        next ();
                      }
                  });
             }
           else
             next ();
         }
    }
  else
    next ();
}

void AI_Fast::step3 (Stack *s, sigc::slot<void()> next)
{
  if (abort_requested)
    return;
  bool moving = false;
  // join armies if close
  if (!m_did_something && d_join && s->isFull () == false)
    {
      Stack* target = NULL;
      target = findNearOwnStackToJoin (s, 5);

      if (target)
        {
          if (s->calculatePath (target))
            {
              moving = true;
              stackMove
                (s,
                 [this, s, target, next] (MoveResult *res)
                 {
                   bool moved = res->getStepCount ();
                   bool fought = res->fought ();
                   bool died = res->is_alive () == false;
                   if (!died)
                     {
                       if (target->getPos () == s->getPos ())
                         GameMap::groupStacks(s);
                     }
                   delete res;
                   if (moved || fought)
                     m_did_something = true;
                   next ();
                 });
            }
        }
    }
  if (!moving)
    next ();
}

void AI_Fast::step4 (Stack *s, sigc::slot<void()> next)
{
  if (abort_requested)
    return;
  bool moving = false;
  // try to resupply
  if (!m_did_something && !d_maniac)
    {
      City *target =
        Citylist::instance ()->getNearestFriendlyCity (s->getPos ());
      if (s->isFull () == false && target)
        {
          // try to move to the north west part of the city (where the units
          // move after production), otherwise just wait and stand around

          if (target->contains (s->getPos ()) == false)
            {
              if (s->calculatePath (target))
                {
                  moving = true;
                  stackMove
                    (s,
                     [this, s, next] (MoveResult *res)
                     {
                       bool fought = res->fought ();
                       bool died = res->is_alive () == false;
                       if (!died)
                         {
                           GameMap::groupStacks (s);
                           s->clearPath ();
                         }
                       bool moved = res->getStepCount () > 0;
                       delete res;
                       if (moved || fought)
                         m_did_something = true;
                       next ();
                     });
                }
            }
          else if (s->getPos () != target->getPos ())
            {
              //if we're not in the upper right corner
              //go there, and take as many as we can
              if (s->calculatePath (target->getPos ()))
                {
                  moving = true;
                  stackSplitAndMove
                    (s,
                     [this, s, target, next] (MoveResult *res, Stack *new_stack)
                     {
                       bool split = new_stack != NULL;
                       bool fought = res->fought ();
                       bool died = res->is_alive () == false;
                       if (died)
                         {
                           GameMap::groupStacks (s);
                           s->clearPath ();
                           GameMap::groupStacks (target->getPos ());
                         }
                       bool moved = res->getStepCount () > 0;
                       delete res;
                       if (!died && !moved && !fought && split)
                         GameMap::groupStacks (s);
                       else if (moved || fought || split)
                         m_did_something = true;
                       next ();
                     });
                }
            }
          else
            {
              //otherwise just stay put in the city
              GameMap::groupStacks (s);
            }
        }
      else // non-maniac players attack only enemy cities
        {
          target = NULL;
          PathCalculator pc (s, true, 10, -1);
          guint32 moves1 = 0, turns1 = 0, moves2 = 0, turns2 = 0;
          guint32 left1 = 0, left2 = 0;
          Path *target1_path = NULL;
          Path *target2_path = NULL;
          City *target1, *target2;
            
          target1 = Citylist::instance ()->getClosestEnemyCity (s);

          target2 =
            Citylist::instance ()->getNearestForeignCity (s->getPos ());
          if (!target2)
            target2 =
              Citylist::instance ()->getNearestEnemyCity (s->getPos ());

          if (target1)
            target1_path = pc.calculateToCity (target1, moves1, turns1, left1);
          else
            target1_path = new Path ();

          if (target2)
            target2_path = pc.calculateToCity(target2, moves2, turns2, left2);

          //no enemies?  then go for the nearest foreign city.
          //if diplomacy isn't on and we hit this, then it's game over
          if (!target1)
            {
              target = target2;

              //end of game
              if (!target)
                {
                  next ();
                  return;
                }
            }
          //is the enemy city far enough away that a foreign city
          //outweighs it?
          else if (target1_path->size() / 13 > target2_path->size())
            target = target2;
          else
            target = target1;
          delete target1_path;
          delete target2_path;

          if (target == target2 && target)
            {
              if (GameScenarioOptions::s_diplomacy == true)
                d_diplomacy->needNewEnemy (target->getOwner ());
              // try to wait a turn until we're at war
              if (target1)
                target = target1;
            }

          if (target && s->calculatePath (target))
            {
              moving = true;
              stackMove
                (s,
                 [this, s, target, next] (MoveResult *res)
                 {
                   if (res->is_alive () == false || res->fought ())
                     m_did_something = true;
                   //if we didn't get there and the target city is empty
                   //attack it if we can reach it.
                   if (res->is_alive () &&
                       target->getOwner () != s->getOwner () && 
                       target->countDefenders () == 0)
                     {
                       stackSplitAndMove
                         (s,
                          [this, s, next] (MoveResult *res2, Stack *ns)
                          {
                            bool split = ns != NULL;
                            bool died2 = res2->is_alive () == false;
                            bool fought2 = res2->fought ();
                            bool moved2 = res2->getStepCount () > 0;
                            if (!died2)
                              GameMap::groupStacks (s);
                            delete res2;
                            if (!died2 && !moved2 && !fought2 && split)
                              GameMap::groupStacks (s);
                            else if (moved2 || fought2 || split)
                              m_did_something = true;
                            next ();
                          });
                     }
                   else
                     {
                       bool fought = res->fought ();
                       bool moved = res->getStepCount () > 0;
                       delete res;
                       if (moved || fought)
                         m_did_something = true;
                       next ();
                     }
                 });
            }
        }
    }
  if (!moving)
    next ();
}

void AI_Fast::step5 (Stack *s, sigc::slot<void()> next)
{
  if (abort_requested)
    return;
  bool moving = false;
  // maniac players attack everything that is close if they can
  // reach it or cities otherwise.
  if (!m_did_something && d_maniac)
    {
      const Threatlist* threats = d_analysis->getThreatsInOrder (s->getPos ());
      const Threat* target = NULL;

      // prefer weak forces (take strong if neccessary) and stop after 10
      // stacks
      int i = 0;
      for (auto tit = threats->begin (); tit != threats->end () && i < 10;
           ++tit, i++)
        {
          // in a first step, we only look at enemy stacks
          if ((*tit)->isCity () || (*tit)->isRuin ())
            continue;

          // ignore stacks out of reach
          Vector<int> threatpos = (*tit)->getClosestPoint (s->getPos ());
          if (threatpos == Vector<int> (-1, -1))
            continue;

          guint32 mp = s->getPath ()->calculate (s, threatpos);
          if ((int)mp <= 0 || mp > s->getMoves ())
            continue;

          target = *tit;
          break;
        }

      // now we need to choose. If we found a target, attack it, otherwise
      // attack the closest city.
      Vector<int> pos = Vector<int> (-1,-1);
      if (target)
        pos = target->getClosestPoint (s->getPos ());
      else
        {
          City *enemy_city = 
            Citylist::instance ()->getNearestForeignCity (s->getPos ());
          if (enemy_city)
              pos  = enemy_city->getPos ();
        }

      if (pos != Vector<int> (-1,-1))
        {
          if (s->calculatePath (pos))
            {
              moving = true;
              stackMove
                (s,
                 [this, next] (MoveResult *res)
                 {
                   bool fought = res->fought ();
                   bool moved = res->getStepCount () > 0;
                   delete res;
                   if (moved || fought)
                     m_did_something = true;
                   next ();
                 });
            }
          else
            {
              City *friendly_city = 
                Citylist::instance ()->getNearestFriendlyCity (s->getPos ());
              if (friendly_city)
                {
                  if (s->calculatePath (friendly_city))
                    {
                      moving = true;
                      stackMove
                        (s,
                         [this, next] (MoveResult *res)
                         {
                           bool fought = res->fought ();
                           bool moved = res->getStepCount () > 0;
                           delete res;
                           if (moved || fought)
                             m_did_something = true;
                           next ();
                         });
                    }
                }
            }
        }
    }
  if (!moving)
    next ();
}

void AI_Fast::computerTurn (sigc::slot<void()> after)
{
  debugg = false;
  Stack *s = get_next_stack_from_list ();
  if (s == NULL)
    {
      //we're thru the list. 
      //if we moved anything we do it all again.
      if (m_dirty)
        {
          initComputerTurn ();
          s = get_next_stack_from_list ();
          if (s == NULL)
            {
              after ();
              return;
            }
        }
      else
        {
          after ();
          return;
        }
    }
  if (debugg)
    printf("working with %s stack %d\n", getName ().c_str (), s->getId ());
  m_did_something = false;
  if (debugg)
    printf ("step1 %d\n", m_did_something);
  step1
    (s, [this, s, after] ()
     {
       if (debugg)
         printf ("step2 %d\n", m_did_something);
       step2
         (s, [this, s, after] ()
          {
            if (debugg)
              printf ("step3 %d\n", m_did_something);
            step3
              (s, [this, s, after] ()
               {
                 if (debugg)
                   printf ("step4 %d\n", m_did_something);
                 step4
                   (s, [this, s, after] ()
                    {
                      if (debugg)
                        printf ("step5 %d\n", m_did_something);
                      step5
                        (s, [this, s, after] ()
                         {
                           if (m_did_something)
                             m_dirty = true;
                           if (debugg)
                             printf ("all_done %d\n", m_did_something);
                           // recurse until complete
                           computerTurn (after);
                         });
                    });
               });
          });
     });
  return;
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
  //if no money then we take the first
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

bool AI_Fast::chooseVisitRuin(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) turns;
  if (stack->getPos() == dest)
    return true;
  if (moves < stack->getMoves() + 15)
    return true;
  else 
    return false;
}

bool AI_Fast::choosePickupBag(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) turns;
  if (stack->getPos() == dest)
    return true;
  if (moves < stack->getMoves() + 7)
    return true;
  else 
    return false;
}

bool AI_Fast::chooseVisitTempleForBlessing(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
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

bool AI_Fast::chooseVisitTempleForQuest(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
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

bool AI_Fast::chooseContinueQuest(Stack *stack, Quest *quest, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) stack;
  (void) quest;
  (void) dest;
  (void) moves;
  (void) turns;
  return true;
}
