// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2004 John Farrell
// Copyright (C) 2005 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2014, 2015, 2020,
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

#include <sigc++/functors/mem_fun.h>

#include "citylist.h"
#include "city.h"
#include "playerlist.h"
#include <limits.h>
#include "xmlhelper.h"
#include "hero.h"
#include "stack.h"
#include "armyprodbase.h"
#include "GameMap.h"
#include "cityset.h"
#include "citysetlist.h"
#include "PathCalculator.h"
#include "rnd.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Glib::ustring Citylist::d_tag = "citylist";

Citylist* Citylist::s_instance = 0;

Citylist* Citylist::getInstance()
{
    if (s_instance == 0)
        s_instance = new Citylist();

    return s_instance;
}

Citylist* Citylist::getInstance(XML_Helper* helper)
{
    if (s_instance)
        deleteInstance();

    s_instance = new Citylist(helper);
    return s_instance;
}

void Citylist::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

Citylist::Citylist()
{
}

Citylist::~Citylist()
{
  for (iterator it = begin(); it != end(); ++it)
    delete *it;
  d_object.clear();
  d_id.clear();
}

Citylist::Citylist (const Citylist &c, bool sync_ids)
 : LocationList<City*> (), sigc::trackable (c)
{
  for (auto city : c)
    add (new City (*city, sync_ids));
}

Citylist::Citylist(XML_Helper* helper)
{
    // simply ask the helper to inform us when a city tag is opened
    helper->registerTag(City::d_tag, sigc::mem_fun(this, &Citylist::load));
}

int Citylist::countCities() const
{
    int cities = 0;
    
    for (const_iterator it = begin(); it != end(); ++it)
    {
        if ((*it)->isBurnt())
          continue;
        cities++;
    }
    
    return cities;
}

int Citylist::countCities(Player* player) const
{
    int cities = 0;
    
    for (const_iterator it = begin(); it != end(); ++it)
    {
        if ((*it)->isBurnt())
          continue;
        if ((*it)->getOwner() == player) cities++;
    }
    
    return cities;
}

void Citylist::collectTaxes(Player* p) const
{
  // Collect the taxes
  for (const_iterator it = begin(); it != end(); ++it)
    if ((*it)->getOwner() == p && (*it)->isBurnt() == false)
      p->addGold((*it)->getGold());

}

//calculate the amount of money new armies will cost in the upcoming turn.
guint32 Citylist::calculateUpcomingUpkeep(Player *p) const
{
  guint32 total = 0;
  for (const_iterator it = begin(); it != end(); ++it)
    if ((*it)->getOwner() == p && (*it)->isBurnt() == false)
      {
	if ((*it)->getDuration() == 1)
	  {
	    int slot =(*it)->getActiveProductionSlot();
	    if (slot == -1)
	      continue;
	    const ArmyProdBase *a = (*it)->getProductionBase(slot);
	    total += a->getUpkeep();
	    total += a->getProductionCost();
	  }
      }
  return total;
}

void Citylist::nextTurn(Player* p)
{
    debug("next_turn(" <<p->getName() <<")");

    // Because players are nextTurn'd before cities, the income, treasury, 
    // and upkeep are calculated already for the upcoming round.

    //we've already collected taxes this round, so hopefully our
    //treasury has enough money to pay our city upkeep.

    guint32 cost_of_new_armies = calculateUpcomingUpkeep(p);
    if (p->getGold() < (int)cost_of_new_armies)
      {
	int diff = cost_of_new_armies - p->getGold();
	//then we have to turn off enough production to make up for diff
	//gold pieces.
	for (iterator it = begin(); it != end(); ++it)
	  {
	    if ((*it)->isBurnt() == true)
	      continue;
	    if ((*it)->getOwner() != p)
	      continue;
	    int slot =(*it)->getActiveProductionSlot();
	    if (slot == -1)
	      continue;
	    const ArmyProdBase *a = (*it)->getProductionBase(slot);
	    diff -= a->getUpkeep();
    
	    p->cityTooPoorToProduce((*it), slot);
	    if (diff < 0)
	      break;
	  }
      }

    // This iteration adds the city production to the player    
    for (iterator it = begin(); it != end(); ++it)
      {
        if ((*it)->getOwner() == p)
            (*it)->nextTurn();
      }

}

static bool isFogged(void *object)
{
  return ((City*)object)->isVisible(Playerlist::getViewingplayer()) == false;
}

static bool isBurnt(void *object)
{
  return ((City*)object)->isBurnt();
}

static bool isNotOwnedByNeutral(void *object)
{
  return ((City*)object)->getOwner() != Playerlist::getInstance()->getNeutral();
}

static bool isNotOwnedByActivePlayer(void *object)
{
  return ((City*)object)->getOwner() != Playerlist::getActiveplayer();
}

static bool isOwnedByActivePlayer(void *object)
{
  return ((City*)object)->getOwner() == Playerlist::getActiveplayer();
}

static bool isNotOwnedByEnemy(void *object)
{
  Player *p = Playerlist::getActiveplayer();
  if (!p)
    return false;
  City *city = ((City*)object);
  if (city->getOwner() != p &&
	    p->getDiplomaticState(city->getOwner()) == Player::AT_WAR)
    return false;
  return true;
}

static bool canNotAcceptMoreVectoring(void *object)
{
  City *c = ((City*)object);
  guint32 num = Citylist::getInstance()->countCitiesVectoringTo(c);
  if (num < MAX_CITIES_VECTORED_TO_ONE_CITY)
    return false;
  return true;
}

City* Citylist::getNearestEnemyCity(const Vector<int>& pos) const
{
  std::list<bool (*)(void *)> filters;
  filters.push_back(isBurnt);
  filters.push_back(isNotOwnedByEnemy);
  return getNearestObject(pos, &filters);
}

City* Citylist::getClosestEnemyCity(const Stack *stack) const
{
  std::list<bool (*)(void *)> filters;
  filters.push_back(isBurnt);
  filters.push_back(isNotOwnedByEnemy);
  return getClosestObject(stack, &filters);
}

City* Citylist::getClosestForeignCity(const Stack *stack) const
{
  std::list<bool (*)(void *)> filters;
  filters.push_back(isBurnt);
  filters.push_back(isOwnedByActivePlayer);
  return getClosestObject(stack, &filters);
}

City* Citylist::getNearestForeignCity(const Vector<int>& pos) const
{
  std::list<bool (*)(void *)> filters;
  filters.push_back(isBurnt);
  filters.push_back(isOwnedByActivePlayer);
  return getNearestObject(pos, &filters);
}

City* Citylist::getNearestCity(const Vector<int>& pos, int dist) const
{
  City *c = getNearestCity(pos);
  if (!c)
    return c;
  if (c->getPos().x <= pos.x + dist && c->getPos().x >= pos.x - dist &&
      c->getPos().y <= pos.y + dist && c->getPos().y >= pos.y - dist)
    return c;
  return NULL;
}

City* Citylist::getNearestFriendlyCity(const Vector<int>& pos, int dist) const 
{
  City *c = getNearestFriendlyCity(pos);
  if (!c)
    return c;
  if (c->getPos().x <= pos.x + dist && c->getPos().x >= pos.x - dist &&
      c->getPos().y <= pos.y + dist && c->getPos().y >= pos.y - dist)
    return c;
  return NULL;
}

City* Citylist::getNearestFriendlyCity(const Vector<int>& pos) const
{
    Player* p = Playerlist::getInstance()->getActiveplayer();
    return getNearestCity (pos, p);
}

City* Citylist::getNearestCity(const Vector<int>& pos, Player *player) const
{
    int diff = -1;
    const_iterator diffit;
    
    for (const_iterator it = begin(); it != end(); ++it)
    {
        if ((*it)->isBurnt())
            continue;

        if ((*it)->getOwner() == player)
        {
            Vector<int> p = (*it)->getPos();
            int delta = abs(p.x - pos.x);
            if (delta < abs(p.y - pos.y))
                delta = abs(p.y - pos.y);
            
            if ((diff > delta) || (diff == -1))
            {
                diff = delta;
                diffit = it;
            }
        }
    }
    
    if (diff == -1) return 0;
    return (*diffit);
}

City* Citylist::getClosestCity(const Stack *stack, Player *p) const
{
    int diff = -1;
    const_iterator diffit;
    PathCalculator pc(stack, true, 0, 0);
    
    for (const_iterator it = begin(); it != end(); ++it)
    {
        if ((*it)->isBurnt())
            continue;

        if ((*it)->getOwner() == p)
        {
            int delta = pc.calculate((*it)->getPos());
            if (delta <= 0)
              continue;
            
            if ((diff > delta) || (diff == -1))
            {
                diff = delta;
                diffit = it;
            }
        }
    }
    
    if (diff == -1) return 0;
    return (*diffit);
}

City* Citylist::getClosestFriendlyCity(const Stack *stack) const
{
    Player* p = Playerlist::getInstance()->getActiveplayer();
    return getClosestCity (stack, p);
}
City* Citylist::getClosestCity(const Stack *stack) const
{
  std::list<bool (*)(void *)> filters;
  filters.push_back(isBurnt);
  return getClosestObject(stack, &filters);
}

City* Citylist::getNearestCity(const Vector<int>& pos) const
{
  std::list<bool (*)(void *)> filters;
  filters.push_back(isBurnt);
  return getNearestObject(pos, &filters);
}

City* Citylist::getNearestVisibleCity(const Vector<int>& pos) const
{
  std::list<bool (*)(void *)> filters;
  filters.push_back(isBurnt);
  filters.push_back(isFogged);
  return getNearestObject(pos, &filters);
}

City* Citylist::getNearestVisibleCity(const Vector<int>& pos, int dist) const
{
  City *c = getNearestVisibleCity(pos);
  if (!c)
    return c;
  if (c->getPos().x <= pos.x + dist && c->getPos().x >= pos.x - dist &&
      c->getPos().y <= pos.y + dist && c->getPos().y >= pos.y - dist)
    return c;
  return NULL;
}

City* Citylist::getNearestVisibleFriendlyCity(const Vector<int>& pos, int dist) const
{
  City *c = getNearestFriendlyCity(pos);
  if (!c)
    return c;
  if (c->getPos().x <= pos.x + dist && c->getPos().x >= pos.x - dist &&
      c->getPos().y <= pos.y + dist && c->getPos().y >= pos.y - dist)
    return c;
  return NULL;
}


City* Citylist::getNearestVisibleFriendlyCity(const Vector<int>& pos) const
{
  std::list<bool (*)(void *)> filters;
  filters.push_back(isBurnt);
  filters.push_back(isFogged);
  filters.push_back(isNotOwnedByActivePlayer);
  return getNearestObject(pos, &filters);
}

City* Citylist::getNearestNeutralCity(const Vector<int>& pos) const
{
  std::list<bool (*)(void *)> filters;
  filters.push_back(isBurnt);
  filters.push_back(isNotOwnedByNeutral);
  return getNearestObject(pos, &filters);
}

City* Citylist::getNearestFriendlyVectorableCity(const Vector<int>& pos) const
{
  std::list<bool (*)(void *)> filters;
  filters.push_back(isBurnt);
  filters.push_back(isNotOwnedByActivePlayer);
  filters.push_back(canNotAcceptMoreVectoring);
  return getNearestObject(pos, &filters);
}

bool Citylist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(Citylist::d_tag);

    for (const_iterator it = begin(); it != end(); ++it)
        (*it)->save(helper);
    
    retval &= helper->closeTag();

    return retval;
}

bool Citylist::load(Glib::ustring tag, XML_Helper* helper)
{
    if (tag == City::d_tag)
      {
        Cityset *cs = GameMap::getCityset();
	City *c = new City(helper, cs->getCityTileWidth());
	add(c);
	return true;
      }
    return false;
}

void Citylist::changeOwnership(Player *old_owner, Player *new_owner)
{
  for (iterator it = begin(); it != end(); ++it)
    if ((*it)->getOwner() == old_owner)
      {
        stopVectoringTo(*it);
        (*it)->setVectoring(Vector<int>(-1,-1));
	(*it)->setOwner(new_owner);
	if ((*it)->isCapital())
	  if ((*it)->getCapitalOwner() == old_owner)
	    (*it)->setCapitalOwner(new_owner);
        if (new_owner == Playerlist::getInstance()->getNeutral())
          (*it)->setActiveProductionSlot(-1); //hmm, what about neutral policy.
      }
}

void Citylist::stopVectoringTo(City *c)
{
  for (iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->isBurnt() == true)
	continue;
      if ((*it)->getVectoring() == Vector<int>(-1,-1))
	continue;
      if (c->contains((*it)->getVectoring()))
	(*it)->setVectoring(Vector<int>(-1,-1));
    }
  return;
}

bool Citylist::isVectoringTarget(City *target) const
{
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getOwner() != target->getOwner())
	continue;
      if (target->contains((*it)->getVectoring()))
	return true;
    }
  return false;
}

std::list<City*> Citylist::getCitiesVectoringTo(Vector<int> target) const
{
  std::list<City*> cities;
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if (target == (*it)->getVectoring())
	cities.push_back((*it));
    }
  return cities;
}

std::list<City*> Citylist::getCitiesVectoringTo(City *target) const
{
  std::list<City*> cities;
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getOwner() != target->getOwner())
	continue;
      if (target->contains((*it)->getVectoring()))
	cities.push_back((*it));
    }
  return cities;
}

City* Citylist::getNearestCityPast(const Vector<int>& pos, int dist) const
{
  std::list<bool (*)(void *)> filters;
  filters.push_back(isBurnt);
  return getNearestObjectAfter(pos, dist, &filters);
}

guint32 Citylist::countCitiesVectoringTo(const City *dest) const
{
  guint32 count = 0;
  for (const_iterator it = begin(); it != end(); ++it)
    {
      City *c = *it;
      if (c->getOwner() != dest->getOwner())
	continue;
      if (c->getVectoring() == dest->getPos())
	count++;
    }

  return count;
}

std::list<City*> Citylist::getNearestFriendlyCities(Player *player, Vector<int> pos) const
{
  std::list<City*> cities;
  for (const_iterator it = begin(); it != end(); ++it)
    {
      City *c = *it;
      if (c->getOwner() != player)
        continue;
      if (c->isBurnt() == true)
        continue;
      cities.push_back(c);
    }
  if (cities.size() == 0)
    return cities;
  std::list<int> distances;

  if (pos == Vector<int>(-1,-1))
    pos = player->getFirstCity()->getPos();

  for (std::list<City*>::iterator it = cities.begin(); it != cities.end(); ++it)
    distances.push_back(dist((*it)->getNearestPos(pos), pos));

  bool sorted = false;

  while (!sorted)
    {
      sorted = true;

      // setup
      std::list<int>::iterator dit = distances.begin();
      std::list<int>::iterator dnextit = distances.begin();
      ++dnextit;

      std::list<City*>::iterator it = cities.begin();
      std::list<City*>::iterator nextit = it;
      ++nextit;

      for (; nextit != cities.end(); ++it, ++nextit, ++dit, ++dnextit)
        if ((*dit) > (*dnextit))
          {
            // exchange the items in both lists
            sorted = false;

            City* tmp = (*nextit);
            cities.erase(nextit);
            nextit = it;
            it = cities.insert(nextit, tmp);

            int val = (*dnextit);
            distances.erase(dnextit);
            dnextit = dit;
            dit = distances.insert(dnextit, val);
          }
    }
  return cities;
}

City *Citylist::getCapitalCity(Player *player) const
{
  for (const_iterator it = begin(); it != end(); ++it)
    {
      City *c = *it;
      if (c->isCapital() && c->getCapitalOwner() &&
          c->getCapitalOwner()->getId() == player->getId())
        return c;
    }
  return NULL;
}

City *Citylist::getRandomCityForHero(Player *player) const
{
  std::vector<City*> cities;
  for (const_iterator it = begin(); it != end(); ++it)
    if (!(*it)->isBurnt() && (*it)->getOwner() == player)
      cities.push_back((*it));
  if (cities.empty())
    return NULL;
  return cities[Rnd::rand() % cities.size()];
}

guint32 Citylist::countUnamedCities () const
{
  guint32 count = 0;
  for (const_iterator it = begin(); it != end(); ++it)
    if ((*it)->getName () == DEFAULT_CITY_NAME)
      count++;
  return count;
}

void Citylist::reset (Citylist *c)
{
  delete s_instance;
  s_instance = c;
}
// End of file
