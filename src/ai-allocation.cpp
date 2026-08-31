//  Copyright (C) 2004 John Farrell
//  Copyright (C) 2004, 2005, 2006, 2007 Ulf Lorenz
//  Copyright (C) 2008, 2009, 2010, 2014, 2015, 2017, 2026 Ben Asselstine
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
#include <assert.h>
#include "ai-analysis.h"
#include "ai-allocation.h"
#include "player.h"
#include "player-list.h"
#include "city-list.h"
#include "stack-list.h"
#include "stack.h"
#include "city.h"
#include "threat.h"
#include "move-result.h"
#include "ruin.h"
#include "path.h"
#include "ruin-list.h"
#include "game-map.h"
#include "game-scenario-options.h"
#include "threat-list.h"
#include "path-calculator.h"
#include "stack-tile.h"
#include "stack-ref-list.h"
#include "army-proto.h"
#include "quest-manager.h"
#include "quest.h"
#include "quest-kill-hero.h"
#include "quest-enemy-armies.h"
#include "quest-enemy-army-type.h"
#include "rnd.h"
#include "reward.h"
#include "boon.h"

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

void AI_Allocation::allocateStacksToCapacityBuilding (sigc::slot<void()> after)
{
  if (m_did_something)
    return after ();
  if (m_did_something)
    {
      after ();
      return;
    }
  std::list<Threat*> threatlist = *d_threats;

  m_allocateStacksToCapacityBuilding =
    [this, after] (std::list<Threat*> threats) mutable
      {
        Threat *t = threats.front ();

        Stack *s = NULL;
        Vector<int> target = Vector<int>(-1,-1);
        if (t->isCity () && t->getStrength () <= 0.5003 &&
            d_owner->getStacklist ()->empty () == false)
          {
            Stack *first_stack = d_owner->getStacklist ()->front ();
            target = t->getClosestPoint (first_stack->getPos ());
            if (m_first_city)
              target = t->getClosestPoint (m_first_city->getPos ());

            City *c = GameMap::getEnemyCity (target);
            if (c)
              {
                Stack *attacker =
                  findClosestStackToEnemyCity (c, m_take_neutrals);
                s = attacker;
                if (c->isBurnt () == true ||
                    (c->getOwner () == Playerlist::getNeutral () &&
                     !m_take_neutrals) ||
                    !attacker)
                  target = Vector<int>(-1,-1);
              }
          }

        if (s && target != Vector<int>(-1,-1)) //yes
          {
            auto armies = s->determineStrongArmies (3.0);
            if (armies.size () > 0 && armies.size () != s->size () &&
                !s->fliesWithItemAndNonFlyersOverWaterOrMountains ())
              s = d_owner->stackSplitArmies (s, armies);

            s->calculatePath (target);
            moveStack
              (s, "allocateStacksToCapacityBuilding",
               [this, after, threats, s] (bool moved, bool fought, bool) mutable
               {
                 if (moved || fought)
                   m_did_something = true;

                 if (m_did_something)
                   after ();
                 else
                   {
                     //do the next if there is one, or end
                     threats.erase (threats.begin ());
                     if (threats.empty () == false)
                       m_allocateStacksToCapacityBuilding (std::move (threats));
                     else
                       after ();
                   }
               });
          }
        else
          {
            //do the next if there is one, or end
            threats.erase (threats.begin ());
            if (threats.empty () == false)
              m_allocateStacksToCapacityBuilding (std::move (threats));
            else
              after ();
          }
      };
  if (threatlist.empty () == false)
    m_allocateStacksToCapacityBuilding (threatlist);
  else
    after ();

  return;
}

bool AI_Allocation::should_do_boon (const Boon *b) const
{
  if (b->is_city () && b->get_turns_away () < 1) 
    return true;
  else if (b->is_temple () && b->get_turns_away () < 2.5)
    return true;
  else if (b->is_ruin () && b->get_turns_away () < 3.5)
    return true;
  else if (b->is_backpack () && b->get_turns_away () < 4.5) 
    return true;
  else if (b->is_quest () && b->get_turns_away () < 10) 
    return true;

  return false;
}

void AI_Allocation::doBoons (sigc::slot<void ()> after)
{
  if (m_did_something)
    return after ();

  auto boons = d_analysis->getBoonsInOrder ();
  auto it =
    std::find_if
    (boons.begin (), boons.end (),
     [this](const std::pair<Boon,Stack*> b)
     {
       return should_do_boon (&b.first);
     });

  if (it == boons.end ())
    after ();
  else
    {
      Boon b = (*it).first;
      Stack *s = (*it).second;
      Vector<int> dest = b.get_destination (s);
      if (s->calculatePath (dest) || s->getPos () == dest)
        {
          moveStack
            (s, "doBoons " + b.to_string (),
             [this, s, b, dest, after] (bool moved, bool fought, bool died)
             {
               if (!died)
                 {
                   if (s->getPos () == dest)
                     {
                       if (moved)
                         m_did_something = true;
                       if (b.is_city ())
                         {
                           // nothing to do.
                           after ();
                         }
                       else if (b.is_temple ())
                         {
                           d_owner->svisitingTemple.emit
                             (s,
                              [this, after, moved] (bool quest, int blessed) mutable
                              {
                                if (moved || quest || blessed)
                                  m_did_something = true;
                                after ();
                                return;
                              });
                         }
                       else if (b.is_ruin ())
                         {
                           d_owner->ssearchingRuin.emit
                             (s,
                              [this, after] (bool /*died */) mutable
                              {
                                m_did_something = true;
                                after ();
                                return;
                              });
                         }
                       else if (b.is_backpack ())
                         {
                           Hero *hero =
                             static_cast<Hero*>(s->getFirstHero ());
                           if (hero)
                             {
                               if (d_owner->heroPickupAllItems (hero,
                                                                s->getPos ()))
                                 m_did_something = true;
                               after ();
                             }
                           else
                             after ();
                         }
                       else if (b.is_quest ())
                         {
                           // nothing to do.
                           // we complete a quest by killing a stack or a
                           // city.  all that happens in stackMove.
                           after ();
                         }
                     }
                   else
                     after ();
                 }
               else
                 {
                   if (moved || fought)
                     m_did_something = true;
                   after ();
                 }
             });
        }
      else
        after ();
    }
}

void AI_Allocation::continueAttacks (sigc::slot<void()> after)
{
  if (m_did_something)
    return after ();
  std::list<Stack*> stacklist = *d_stacks;
  m_continueAttacks =
    [this, after] (std::list<Stack*> stacks) mutable
      {
        guint32 turns_ago = 0;
        Stack *s = stacks.front ();

        Vector<int> pos = s->getLastPointInPath ();
        City *city = NULL;
        if (pos != Vector<int> (-1,-1))
          city = GameMap::getCity (pos);

        bool already_conquered = d_owner->conqueredCity (city, turns_ago);

        if (s->getParked () == false && s->isOnCity () == false &&
            s->hasPath () == true && city != NULL && city->getOwner () != d_owner &&
            city->isBurnt () == false)
          {
            moveStack
              (s, "continueAttacks",
               [this, after, stacks, s] (bool moved, bool fought, bool) mutable
               {
                 if (moved || fought)
                   m_did_something = true;

                 if (m_did_something)
                   after ();
                 else
                   {
                     //do the next if there is one, or end
                     stacks.erase (stacks.begin ());
                     if (stacks.empty () == false)
                       m_continueAttacks (std::move (stacks));
                     else
                       after ();
                   }
               });
          }
        else if (s->getParked () == false && s->isOnCity () == false &&
                 s->hasPath () == true && city != NULL &&
                 city->getOwner () == d_owner && city->isBurnt () == false &&
                 already_conquered && turns_ago <= 1)
          {
            //not sure why we're checking already-conquered here
            //bc hey it's owned by us, what else do we need to know

            //hey, the city we were moving to was taken over by us.
            //pick another enemy city.
            //just clear the path so that another routine will move us.
            s->clearPath ();
            m_did_something = true;

            //do the next if there is one, or end
            stacks.erase (stacks.begin ());
            if (stacks.empty () == false)
              m_continueAttacks (std::move (stacks));
            else
              after ();
          }
        else
          {
            //do the next if there is one, or end
            stacks.erase (stacks.begin ());
            if (stacks.empty () == false)
              m_continueAttacks (std::move (stacks));
            else
              after ();
          }

      };
  if (stacklist.empty () == false)
    m_continueAttacks (stacklist);
  else
    after ();

  return;
}

void AI_Allocation::attackNearbyEnemyCities (sigc::slot<void()> after)
{
  if (m_did_something)
    return after ();
  std::list<Stack *> stacklist = *d_stacks;
  m_attackNearbyEnemyCities =
    [this, after] (std::list<Stack*> stacks) mutable
      {
        Stack *s = stacks.front ();

        //is the stack next to an unrazed enemy city
        Vector<int> target = Vector<int>(-1,-1);
        for (auto j : GameMap::getNearbyPoints (s->getPos (), 1))
          {
            City *city = GameMap::getCity (j);
            if (!city)
              continue;
            if (city->getOwner () == d_owner || city->isBurnt ())
              continue;
            target = j;
            break;
          }

        if (target != Vector<int>(-1,-1)) //yes
          {
            s->calculatePath (target);
            moveStack
              (s, "attackNearbyEnemyCities",
               [this, after, stacks, s] (bool moved, bool fought, bool) mutable
               {
                 if (moved || fought)
                   m_did_something = true;

                 if (m_did_something)
                   after ();
                 else
                   {
                     //do the next if there is one, or end
                     stacks.erase (stacks.begin ());
                     if (stacks.empty () == false)
                       m_attackNearbyEnemyCities (std::move (stacks));
                     else
                       after ();
                   }
               });
          }
        else
          {
            //do the next if there is one, or end
            stacks.erase (stacks.begin ());
            if (stacks.empty () == false)
              m_attackNearbyEnemyCities (std::move (stacks));
            else
              after ();
          }
      };
  if (stacklist.empty () == false)
    m_attackNearbyEnemyCities (stacklist);
  else
    after ();
}

void AI_Allocation::attackNearbyEnemiesInField (sigc::slot<void()> after)
{
  if (m_did_something)
    return after ();
  std::list<Stack*> stacklist = *d_stacks;
  m_attackNearbyEnemiesInField =
    [this, after] (std::list<Stack*> stacks) mutable
      {
        Stack *s = stacks.front ();

        //is the stack next to an enemy stack
        Vector<int> target = Vector<int>(-1,-1);
        for (auto j : GameMap::getNearbyPoints (s->getPos (), 1))
          {
            Stack *enemy = GameMap::getEnemyStack (j);
            if (enemy && !s->isOnCity () && s->getParked () == false &&
                enemy->isOnCity () == false && s->size () >= enemy->size () &&
                s->hasShip () == enemy->hasShip ())
              {
                target = j;
                break;
              }
          }

        if (target != Vector<int>(-1,-1)) //yes
          {
            s->calculatePath (target);
            moveStack
              (s, "attackNearbyEnemiesInField",
               [this, after, stacks, s] (bool moved, bool fought, bool) mutable
               {
                 if (moved || fought)
                   m_did_something = true;

                 if (m_did_something)
                   after ();
                 else
                   {
                     //do the next if there is one, or end
                     stacks.erase (stacks.begin ());
                     if (stacks.empty () == false)
                       m_attackNearbyEnemiesInField (std::move (stacks));
                     else
                       after ();
                   }
               });
          }
        else
          {
            //do the next if there is one, or end
            stacks.erase (stacks.begin ());
            if (stacks.empty () == false)
              m_attackNearbyEnemiesInField (std::move (stacks));
            else
              after ();
          }

      };
  if (stacklist.empty () == false)
    m_attackNearbyEnemiesInField (stacklist);
  else
    after ();
}

void AI_Allocation::heroesLeaveCities (sigc::slot<void()> after)
{
  if (m_did_something)
    return after ();
  std::list<Stack*> stacklist = *d_stacks;
  m_heroesLeaveCities =
    [this, after] (std::list<Stack*> stacks) mutable
      {
        Stack *s = stacks.front ();

        City *target = NULL;
        if (s->getParked () == false && s->hasHero () == true &&
            s->getMoves () >= s->getMaxMoves ())
          target = Citylist::instance ()->getClosestEnemyCity (s);

        if (target)
          {
            s->calculatePath (target);
            moveStack
              (s, "heroesLeaveCities",
               [this, after, stacks, s] (bool moved, bool fought, bool) mutable
               {
                 if (moved || fought)
                   m_did_something = true;

                 if (m_did_something)
                   after ();
                 else
                   {
                     //do the next if there is one, or end
                     stacks.erase (stacks.begin ());
                     if (stacks.empty () == false)
                       m_heroesLeaveCities (std::move (stacks));
                     else
                       after ();
                   }
               });
          }
        else
          {
            //do the next if there is one, or end
            stacks.erase (stacks.begin ());
            if (stacks.empty () == false)
              m_heroesLeaveCities (std::move (stacks));
            else
              after ();
          }

      };
  if (stacklist.empty () == false)
    m_heroesLeaveCities (stacklist);
  else
    after ();
}

void AI_Allocation::fullStacksLeaveCities (sigc::slot<void()> after)
{
  if (m_did_something)
    return after ();
  std::list<Stack*> stacklist = *d_stacks;
  m_fullStacksLeaveCities =
    [this, after] (std::list<Stack *> stacks) mutable
      {
        Stack *s = stacks.front ();

        City *target = NULL;
        City *city = GameMap::getCity (s->getPos ());
        if (s->getParked () == false &&
            s->size () == MAX_STACK_SIZE &&
            s->getMoves () >= s->getMaxMoves () &&
            s->isOnCity () && city->isBurnt () == false &&
            city->getDefenders ().size () > MAX_STACK_SIZE * 2)
          target = Citylist::instance()->getClosestEnemyCity (s);

        if (target)
          {
            s->calculatePath (target);
            moveStack
              (s, "fullStacksLeaveCities",
               [this, after, stacks, s] (bool moved, bool fought, bool) mutable
               {
                 if (moved || fought)
                   m_did_something = true;

                 if (m_did_something)
                   after ();
                 else
                   {
                     //do the next if there is one, or end
                     stacks.erase (stacks.begin ());
                     if (stacks.empty () == false)
                       m_fullStacksLeaveCities (std::move (stacks));
                     else
                       after ();
                   }
               });
          }
        else
          {
            //do the next if there is one, or end
            stacks.erase (stacks.begin ());
            if (stacks.empty () == false)
              m_fullStacksLeaveCities (std::move (stacks));
            else
              after ();
          }

      };
  if (stacklist.empty () == false)
    m_fullStacksLeaveCities (stacklist);
  else
    after ();
}

void AI_Allocation::emptyOutCities (sigc::slot<void()> after)
{
  if (m_did_something)
    return after ();
  std::list<City*> citylist = *Citylist::instance ();

  m_emptyOutCities =
    [this, after] (std::list<City *> cities) mutable
      {
        City *c = cities.front ();

        Vector<int> target = Vector<int>(-1,-1);
        Stack *s = NULL;
        int num_defenders = c->countDefenders ();
        for (auto def : c->getDefenders ())
          {
            if ((def->getMoves () > 3 && def->size () >= 4 &&
                 (num_defenders - def->size ()) >= 3) ||
                (Rnd::rand () % 10) == 0)
              {
                s = def;
                break;
              }
          }
        if (c->getOwner () == d_owner && c->isBurnt () == false && s)
          {
            City *target_city =
              Citylist::instance ()->getNearestEnemyCity (s->getPos ());
            if (target_city)
              target = target_city->getNearestPos (s->getPos ());
          }

        if (target != Vector<int>(-1,-1)) //yes
          {
            s->calculatePath (target);
            moveStack
              (s, "emptyOutCities",
               [this, after, cities, s] (bool moved, bool fought, bool) mutable
               {
                 if (moved || fought)
                   m_did_something = true;

                 //do the next if there is one, or end
                 cities.erase (cities.begin ());
                 if (cities.empty () == false)
                   m_emptyOutCities (std::move (cities));
                 else
                   after ();
               });
          }
        else
          {
            //do the next if there is one, or end
            cities.erase (cities.begin ());
            if (cities.empty () == false)
              m_emptyOutCities (std::move (cities));
            else
              after ();
          }

      };
  m_emptyOutCities (citylist);
  return;
}

void AI_Allocation::move (City *first_city, bool take_neutrals,
                          sigc::slot<void(bool)> after)
{
  m_take_neutrals = take_neutrals;
  m_first_city = first_city;
  // move stacks
  d_stacks = new StackReflist (d_owner->getStacklist (), true);

  m_did_something = false;
  //the idea here is that we try to do one thing, move one stack or park it or
  //whatever.
  //for example if we continue a quest, we short circuit the rest of the steps
  //we depend on the caller to keep calling move until we can't do anything.

  debugg = false;
  if (debugg)
    printf ("doBoons\n");
  doBoons
    ([this, after]() mutable
     {
       if (debugg)
         printf ("continueAttacks\n");
       continueAttacks
         ([this, after] () mutable
          {
            if (debugg)
              printf ("attackNearbyEnemyCities\n");
            attackNearbyEnemyCities
              ([this, after] () mutable
               {
                 if (debugg)
                   printf ("attackNearbyEnemyInField\n");
                 attackNearbyEnemiesInField
                   ([this, after] () mutable
                    {
                      if (debugg)
                        printf ("heroesLeaveCities\n");
                      heroesLeaveCities
                        ([this, after] () mutable
                         {
                           if (debugg)
                             printf ("fullStacksLeaveCities\n");
                           fullStacksLeaveCities
                             ([this, after] () mutable
                              {
                                if (debugg)
                                  printf ("allocateStacksToCapacityBuilding\n");
                                allocateStacksToCapacityBuilding
                                  ([this, after] () mutable
                                   {
                                     if (debugg)
                                       printf ("allocateDefensiveStacks\n");
                                     allocateDefensiveStacks
                                       ([this, after] () mutable
                                        {
                                          if (debugg)
                                            printf ("allocateStacksToThreats\n");
                                          allocateStacksToThreats
                                            ([this, after] () mutable
                                             {
                                               if (debugg)
                                                 printf ("defaultStackMovements\n");
                                               defaultStackMovements
                                                 ([this, after] () mutable
                                                  {
                                                    if (debugg)
                                                      printf ("emptyOutCities\n");
                                                    emptyOutCities
                                                      ([this, after] () mutable
                                                       {
                                                         if (debugg)
                                                           printf ("all_done\n");
                                                         delete d_stacks;
                                                         after (m_did_something);
                                                       });
                                                  });
                                             });
                                        });
                                   });
                              });
                         });
                    });
               });
          });
     });
  return;
}

float AI_Allocation::get_city_defender_strength (City *c)
{
  float defender_strength = 0.0;
  for (auto defender : c->getDefenders ())
    defender_strength += d_analysis->assessStackStrength (defender);
  return defender_strength;
}

Stack *AI_Allocation::findClosestStackToCity2(City *city, float strength)
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

      //don't consider it if it can't maybe save us
      if (strength * 0.80 < d_analysis->assessStackStrength (s))
        continue;

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

void AI_Allocation::allocateDefensiveStacks(sigc::slot<void()> after)
{
  if (m_did_something)
    return after ();
  std::list<City*> citylist = *Citylist::instance ();
  m_allocateDefensiveStacks =
    [this, after] (std::list<City *> cities) mutable
      {
        City *c = cities.front ();

        float city_danger = d_analysis->getCityDanger (c);
        //if city is not endangered, we keep a skeleton crew.
        //if a city has 10 strength in it, it's probably pretty safe.
        if (city_danger < 3.0)
          city_danger = 3.0;
        else if (city_danger > 10.0)
          city_danger = 10.0;
        Stack *s = NULL;
        Vector<int> target = Vector<int>(-1,-1);
        auto city_defender_strength = get_city_defender_strength (c);
        if (c->isFriend (d_owner) && c->isBurnt () == false &&
           city_danger > city_defender_strength)
          {
            s = findClosestStackToCity2 (c,
                                         city_danger - city_defender_strength);
            if (s)
              target = c->getPos ();
          }

        if (target != Vector<int>(-1,-1)) //yes
          {
            s->calculatePath (target);
            moveStack
              (s, "allocateDefensiveStacks",
               [this, after, cities, s] (bool moved, bool fought, bool) mutable
               {
                 if (moved || fought)
                   m_did_something = true;

                 if (m_did_something)
                   after ();
                 else
                   {
                     //do the next if there is one, or end
                     cities.erase (cities.begin ());
                     if (cities.empty () == false)
                       m_allocateDefensiveStacks (std::move (cities));
                     else
                       after ();
                   }
               });
          }
        else
          {
            //do the next if there is one, or end
            cities.erase (cities.begin ());
            if (cities.empty () == false)
              m_allocateDefensiveStacks (std::move (cities));
            else
              after ();
          }

      };
  m_allocateDefensiveStacks (citylist);
  return;
}

void AI_Allocation::allocateStacksToThreats (sigc::slot<void()> after)
{
  if (m_did_something)
    return after ();
  std::list<Threat*> threatlist = *d_threats;

  m_allocateStacksToThreats =
    [this, after] (std::list<Threat*> threats) mutable
      {
        Threat *t = threats.front ();
        float threat_danger = t->getDanger () * 1.000;
        if (threat_danger > 32.0) //e.g. 32 light infantry in a city.
          threat_danger = 32.0;

        guint32 num_city_defenders = 0;
        Stack *s = findBestAttackerFor (t, num_city_defenders);

        Vector<int> target = Vector<int>(-1,-1);
        if (s)
          {
            City *c = GameMap::getCity (t->getClosestPoint (Vector<int> (0,0)));
            if (c && c->getOwner () == d_owner)
              ;// it's us? do we really need to check this
            else if (num_city_defenders == 0 || num_city_defenders - s->size () > 3)
              target = t->getClosestPoint (s->getPos ());
          }

        if (target != Vector<int>(-1,-1)) //yes
          {
            s->calculatePath (target);
            moveStack
              (s, "allocateStacksToThreats",
               [this, after, threats, s] (bool moved, bool fought, bool) mutable
               {
                 if (moved || fought)
                   m_did_something = true;

                 if (m_did_something)
                   after ();
                 else
                   {
                     //do the next if there is one, or end
                     threats.erase (threats.begin ());
                     if (threats.empty () == false)
                       m_allocateStacksToThreats (std::move (threats));
                     else
                       after ();
                   }
               });
          }
        else
          {
            //do the next if there is one, or end
            threats.erase (threats.begin ());
            if (threats.empty () == false)
              m_allocateStacksToThreats (std::move (threats));
            else
              after ();
          }

      };
  if (threatlist.empty () == false)
    m_allocateStacksToThreats (threatlist);
  else
    after ();
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

void AI_Allocation::defaultStackMovements (sigc::slot<void()> after)
{
  if (m_did_something)
    return after ();
  std::list<Stack*> stacklist = *d_stacks;
  m_defaultStackMovements =
    [this, after] (std::list<Stack*> stacks) mutable
      {
        Stack *s = stacks.front ();

        bool leave = false;
        bool go_home = false;

        City *source_city = GameMap::getCity (s);
        if (source_city)
          {
            if (s->isFull () &&
                source_city->countDefenders () - s->isFull () > 3)
              leave = true;
          }
        else
          {
            if (s->getPath ()->empty () == true && s->getParked () == false)
              {
                go_home = s->getId() % 2 == 0;
                leave = true;
              }
            else
              leave = (Rnd::rand () % 100) > 98 ? false : true;
          }

        Vector<int> target = Vector<int>(-1,-1);
        if (leave == true)
          {
            City *c;
            if (go_home)
              c = Citylist::instance ()->getNearestFriendlyCity (s->getPos ());
            else
              {
                c = Citylist::instance ()->getNearestEnemyCity (s->getPos ());
                if (!c)
                  c = Citylist::instance ()->getNearestForeignCity (s->getPos ());
              }
            if (c)
              target = c->getNearestPos (s->getPos ());
            if (target != Vector<int>(-1,-1))
              {
                s->calculatePath (target);
                moveStack
                  (s, "defaultStackMovements",
                   [this, after, stacks, s] (bool moved, bool fought, bool) mutable
                   {
                     if (moved || fought)
                       m_did_something = true;

                     if (m_did_something)
                       after ();
                     else
                       {
                         //do the next if there is one, or end
                         stacks.erase (stacks.begin ());
                         if (stacks.empty () == false)
                           m_defaultStackMovements (std::move (stacks));
                         else
                           after ();
                       }
                   });
              }
            else
              {
                //do the next if there is one, or end
                stacks.erase (stacks.begin ());
                if (stacks.empty () == false)
                  m_defaultStackMovements (std::move (stacks));
                else
                  after ();
              }
          }
        else
          {
            //do the next if there is one, or end
            stacks.erase (stacks.begin ());
            if (stacks.empty () == false)
              m_defaultStackMovements (std::move (stacks));
            else
              after ();
          }

      };
  if (stacklist.empty () == false)
    m_defaultStackMovements (stacklist);
  else
    after ();

  return ;
}

/*
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
*/

void AI_Allocation::moveStack (Stack *s, Glib::ustring reason, MoveStackCallback after)
{
  if (debugg)
    printf ("moving %d from %d,%d to %d,%d for %s\n", s->getId (), s->getPos ().x, s->getPos ().y, s->getPath ()->back ().x, s->getPath ()->back ().y, reason.c_str ());
  d_stacks->remove (s);
  d_owner->stackSelect (s);

  if (s->getPath ()->empty ())
    {
      after (false, false, false);
    }
  else
    {
      if (!s->enoughMoves ()) //we don't have enough to go one step along the path
        {
          after (false, false, false);
          return;
        }
      if (s->getPos () == s->getPath ()->back ()) //we're already there
        {
          after (false, false, false);
          return;
        }

      //auto pos = s->getPath ()->back ();
      //printf ("moving %d from %d,%d to %d,%d for %s\n", s->getId (), s->getPos ().x, s->getPos ().y, pos.x, pos.y, reason.c_str ());
      (void) reason;
      d_owner->stackMove
        (s,
         [this, s, after](MoveResult *res)
         {
           bool moved = res->getStepCount () > 0;
           bool fought = res->fought ();
           bool died = res->is_alive () == false;
           bool out_of_moves = res->getOutOfMoves ();
           delete res;
           if (out_of_moves && !died)
             s->setParked (true);
           if (!died)
             groupStacks (s);
           after (moved, fought, died);
         });
    }
  return;
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
  return;
 }

