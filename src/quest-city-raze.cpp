//  Copyright (C) 2007, 2008, 2014, 2015, 2017, 2021 Ben Asselstine
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
#include <assert.h>
#include <sigc++/functors/mem_fun.h>
#include "ucompose.hpp"
#include "army.h"
#include "city.h"
#include "QCityRaze.h"
#include "QuestsManager.h"
#include "citylist.h"
#include "playerlist.h"
#include "stack.h"
#include "xmlhelper.h"
#include "hero.h"
#include "rnd.h"
#include "GameScenarioOptions.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

QuestCityRaze::QuestCityRaze (QuestsManager& mgr, guint32 hero) 
  : Quest(mgr, hero, Quest::CITYRAZE),
    d_city (chooseToRaze (getHero ()->getOwner ())->getId ())
{
  d_targets.push_back(getCity ()->getPos());
  debug("city_id = " << d_city);
  initDescription();
}

QuestCityRaze::QuestCityRaze (const QuestCityRaze &q)
 : Quest (q), sigc::trackable (q), d_city (q.d_city)
{
}

QuestCityRaze::QuestCityRaze (QuestsManager& q_mgr, XML_Helper* helper) 
  : Quest(q_mgr, helper)
{
  helper->getData(d_city, "city");
  d_targets.push_back(getCity()->getPos());
  initDescription();
}

QuestCityRaze::QuestCityRaze (QuestsManager& mgr, guint32 hero, guint32 target) 
  : Quest(mgr, hero, Quest::CITYRAZE), d_city (target)
{
  d_targets.push_back(getCity()->getPos());
  initDescription();
}

bool QuestCityRaze::isFeasible(guint32 heroId)
{
  if (QuestCityRaze::chooseToRaze(getHeroById(heroId)->getOwner()))
    return true;

  return false;
}

bool QuestCityRaze::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag(Quest::d_tag);
  retval &= Quest::save(helper);
  retval &= helper->saveData("city", d_city);
  retval &= helper->closeTag();

  return retval;
}

Glib::ustring QuestCityRaze::getProgress() const
{
  return _("You aren't afraid of doing it, are you?");
}

void QuestCityRaze::getSuccessMsg(std::queue<Glib::ustring>& msgs) const
{
  msgs.push(_("The priests thank you for razing this evil place."));
}

void QuestCityRaze::getExpiredMsg(std::queue<Glib::ustring>& msgs) const
{
  const City* c = getCity();
  msgs.push(String::ucompose
	    (_("The razing of city \"%1\" could not be accomplished."), 
	     c->getName()));
}

City* QuestCityRaze::getCity() const
{
  for (auto it: *Citylist::getInstance())
    if (it->getId() == d_city)
      return it;

  return NULL;
}

void QuestCityRaze::initDescription()
{
  const City* c = getCity();

  d_description = String::ucompose 
    ( _("You must conquer the city \"%1\" and burn it to the ground."),
      c->getName());
}

City* QuestCityRaze::chooseToRaze(Player *p)
{
  if (GameScenarioOptions::s_razing_cities == GameParameters::NEVER)
    return NULL;
  std::vector<City*> cities;

  // Collect all cities
  for (auto it: *Citylist::getInstance())
    if (!it->isBurnt() && it->getOwner() != p && 
        it->getOwner() != Playerlist::getInstance()->getNeutral())
      cities.push_back(it);

  // Find a suitable city for us to raze 
  if (cities.empty())
    return 0;

  return cities[Rnd::rand() % cities.size()];
}

void QuestCityRaze::armyDied(Army *a, bool heroIsCulprit)
{
  (void) a;
  (void) heroIsCulprit;
  //this quest does nothing when an army dies
}

void QuestCityRaze::cityAction(City *c, CityDefeatedAction action, 
			       bool heroIsCulprit, int gold)
{
  (void) gold;
  if (isPendingDeletion())
    return;
  Hero *h = getHero();
  if (!h || h->getHP() <= 0)
    {
      deactivate();
      return;
    }
  if (!c)
    return;
  if (c->getId() != d_city)
    return;
  //did our hero raze the city? success.
  //did our hero do something else with the city?  expire.
  //did another of our stacks take the city?  expire.
  //did another player take the city? do nothing
  switch (action)
    {
    case CITY_DEFEATED_OCCUPY: //somebody occupied
      if (heroIsCulprit) //quest hero did
	d_q_mgr.questExpired(d_hero);
      else if (c->getOwner() == getHero()->getOwner()) //our stack did
	d_q_mgr.questExpired(d_hero);
      break;
    case CITY_DEFEATED_RAZE: //somebody razed
      if (heroIsCulprit) // quest hero
	d_q_mgr.questCompleted(d_hero);
      else if (c->getOwner() == getHero()->getOwner()) // our stack razed
	d_q_mgr.questExpired(d_hero);
      else // their stack did
	d_q_mgr.questExpired(d_hero);
      break;
    case CITY_DEFEATED_SACK: //somebody sacked
      if (heroIsCulprit) // quest hero did
	d_q_mgr.questExpired(d_hero);
      else if (c->getOwner() == getHero()->getOwner()) // our stack did
	d_q_mgr.questExpired(d_hero);
      break;
    case CITY_DEFEATED_PILLAGE: //somebody pillaged
      if (heroIsCulprit) // quest hero did
	d_q_mgr.questExpired(d_hero);
      else if (c->getOwner() == getHero()->getOwner()) // our stack did
	d_q_mgr.questExpired(d_hero);
      break;
    }
}
