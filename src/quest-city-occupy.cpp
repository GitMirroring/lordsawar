//  Copyright (C) 2007, 2008, 2014, 2015, 2020, 2021 Ben Asselstine
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
#include "QCityOccupy.h"
#include "QuestsManager.h"
#include "citylist.h"
#include "playerlist.h"
#include "stack.h"
#include "xmlhelper.h"
#include "hero.h"
#include "rnd.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

QuestCityOccupy::QuestCityOccupy (QuestsManager& mgr, guint32 hero) 
 : Quest(mgr, hero, Quest::CITYOCCUPY),
    d_city (chooseToOccupy(getHero()->getOwner())->getId ())
{
  d_targets.push_back (getCity ()->getPos ());
  debug("city_id = " << d_city);
  initDescription();
}

QuestCityOccupy::QuestCityOccupy (const QuestCityOccupy &q)
 : Quest (q), sigc::trackable (q), d_city (q.d_city)
{
}

QuestCityOccupy::QuestCityOccupy (QuestsManager& q_mgr, XML_Helper* helper) 
 : Quest(q_mgr, helper)
{
  helper->getData(d_city, "city");
  d_targets.push_back(getCity()->getPos());
  initDescription();
}

QuestCityOccupy::QuestCityOccupy (QuestsManager& mgr, guint32 hero, guint32 target) 
 : Quest(mgr, hero, Quest::CITYOCCUPY), d_city (target)
{
  d_targets.push_back(getCity()->getPos());
  initDescription();
}

bool QuestCityOccupy::isFeasible(guint32 heroId)
{
  if (QuestCityOccupy::chooseToOccupy(getHeroById(heroId)->getOwner()))
    return true;

  return false;
}

bool QuestCityOccupy::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag(Quest::d_tag);
  retval &= Quest::save(helper);
  retval &= helper->saveData("city", d_city);
  retval &= helper->closeTag();

  return retval;
}

Glib::ustring QuestCityOccupy::getProgress() const
{
  return _("You aren't afraid of doing it, are you?");
}

void QuestCityOccupy::getSuccessMsg(std::queue<Glib::ustring>& msgs) const
{
  msgs.push(_("The priests thank you for occupying this evil place."));
}

void QuestCityOccupy::getExpiredMsg(std::queue<Glib::ustring>& msgs) const
{
  const City* c = getCity();

  msgs.push(String::ucompose
            (_("The occupation of city \"%1\" could not be accomplished."), 
             c->getName()));
}

City* QuestCityOccupy::getCity() const
{
  for (auto it: *Citylist::getInstance())
    if (it->getId() == d_city)
      return (it);

  return NULL;
}

void QuestCityOccupy::initDescription()
{
  const City* c = getCity();
  d_description = String::ucompose
    (_("You must take over the city \"%1\" and occupy it."), c->getName());
}

City * QuestCityOccupy::chooseToOccupy(Player *p)
{
  std::vector<City*> cities;

  // Collect all cities
  for (auto i: *Citylist::getInstance())
    if (!i->isBurnt() && i->getOwner() != p &&
        i->getOwner() != Playerlist::getInstance()->getNeutral())
      cities.push_back(i);

  // Find a suitable city for us to occupy
  if (cities.empty())
    return 0;

  return cities[Rnd::rand() % cities.size()];
}
	 
void QuestCityOccupy::armyDied(Army *a, bool heroIsCulprit)
{
  (void) a;
  (void) heroIsCulprit;
  //this quest does nothing when an army dies
}

void QuestCityOccupy::cityAction(City *c, CityDefeatedAction action, 
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
  //did our hero occupy the city? success.
  //did our hero do something else with the city?  expire.
  //did another of our stacks take the city?  expire.
  //did another player take the city? do nothing
  switch (action)
    {
    case CITY_DEFEATED_OCCUPY: //somebody occupied
      if (heroIsCulprit) //quest hero did
	d_q_mgr.questCompleted(d_hero);
      else if (c->getOwner() == getHero()->getOwner()) //our stack did
	d_q_mgr.questExpired(d_hero);
      break;
    case CITY_DEFEATED_RAZE: //somebody razed
      if (heroIsCulprit) // quest hero
	d_q_mgr.questExpired(d_hero);
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
      else // their stack did
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
