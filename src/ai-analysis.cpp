// Copyright (C) 2004 John Farrell
// Copyright (C) 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2006 Andrea Paternesi
// Copyright (C) 2009, 2014 Ben Asselstine
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
#include "AI_Analysis.h"
#include "citylist.h"
#include "Threatlist.h"
#include "Threat.h"
#include "playerlist.h"
#include "stackreflist.h"
#include "stacklist.h"
#include "ruinlist.h"
#include "army.h"
#include "city.h"
#include "AICityInfo.h"
#include "armysetlist.h"

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
    d_stacks = new StackReflist(owner->getStacklist());

    examineCities();
    examineRuins();
    examineStacks();
    calculateDanger();

    instance = this;
}

AI_Analysis::~AI_Analysis()
{
    instance = 0;

    delete d_threats;
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
        ArmyProto *strongest = Armysetlist::getInstance()->getArmy(as, type_id);
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

void AI_Analysis::examineCities()
{
  for (auto city: *Citylist::getInstance())
    if (!city->isFriend(d_owner) && !city->isBurnt())
      d_threats->push_back(new Threat(city));
}

void AI_Analysis::examineStacks()
{
  // add all enemy stacks to the list of threats
  for (auto player: *Playerlist::getInstance())
    {
      if (player == d_owner)
        continue;

      Stacklist *sl = player->getStacklist();
      for (Stacklist::iterator sit = sl->begin(); sit != sl->end(); ++sit)
        d_threats->addStack(*sit);
    }
}

void AI_Analysis::examineRuins()
{
    // enable the searching of ruins before you start to regard them as threats
/*    Ruinlist *ruins = Ruinlist::getInstance();
    for (Ruinlist::iterator it = ruins->begin(); it != ruins->end(); ++it)
    {
        Ruin ruin = *it;
        if (!ruin->isSearched())
        {
            d_threats->push_back(new Threat(ruin));
        }
    }
*/
}

void AI_Analysis::calculateDanger()
{
  for (auto city: *Citylist::getInstance())
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

// End of file
