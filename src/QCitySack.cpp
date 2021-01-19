//  Copyright (C) 2007, 2008, 2014, 2015, 2021 Ben Asselstine
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

#include "city.h"
#include "army.h"
#include "QCitySack.h"
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

QuestCitySack::QuestCitySack (QuestsManager& mgr, guint32 hero) 
  : Quest(mgr, hero, Quest::CITYSACK),
    d_city (chooseToSack (getHero()->getOwner())->getId ())
{
  d_targets.push_back(getCity ()->getPos());
  debug("city_id = " << d_city);
  initDescription();
}

QuestCitySack::QuestCitySack (const QuestCitySack &q)
 : Quest (q), sigc::trackable (q), d_city (q.d_city)
{
}

QuestCitySack::QuestCitySack (QuestsManager& q_mgr, XML_Helper* helper) 
  : Quest(q_mgr, helper)
{
  // let us stay in touch with the world...
  helper->getData(d_city, "city");
  d_targets.push_back(getCity()->getPos());
  initDescription();
}

QuestCitySack::QuestCitySack (QuestsManager& mgr, guint32 hero, guint32 target) 
  : Quest(mgr, hero, Quest::CITYSACK), d_city (target)
{
  d_targets.push_back(getCity()->getPos());
  initDescription();
}

bool QuestCitySack::isFeasible(guint32 heroId)
{
  if (QuestCitySack::chooseToSack(getHeroById(heroId)->getOwner()))
    return true;
  return false;
}

bool QuestCitySack::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag(Quest::d_tag);
  retval &= Quest::save(helper);
  retval &= helper->saveData("city", d_city);
  retval &= helper->closeTag();

  return retval;
}

Glib::ustring QuestCitySack::getProgress() const
{
  return _("You aren't afraid of doing it, are you?");
}

void QuestCitySack::getSuccessMsg(std::queue<Glib::ustring>& msgs) const
{
  msgs.push(_("The priests thank you for sacking this evil place."));
}

void QuestCitySack::getExpiredMsg(std::queue<Glib::ustring>& msgs) const
{
  const City* c = getCity();
  msgs.push(String::ucompose(_("The sacking of \"%1\" could not be accomplished."), 
			     c->getName()));
}

City* QuestCitySack::getCity() const
{
  for (auto it: *Citylist::getInstance())
    if (it->getId() == d_city)
      return it;

  return 0;
}

void QuestCitySack::initDescription()
{
  const City* c = getCity();
  d_description = String::ucompose (_("You must take over and sack the city of \"%1\"."), c->getName());
}

City * QuestCitySack::chooseToSack(Player *p)
{
  if (GameScenarioOptions::s_sacking_mode == GameParameters::SACKING_NEVER ||
      GameScenarioOptions::s_sacking_mode == GameParameters::SACKING_ON_CAPTURE)
    return NULL;

  std::vector<City*> cities;

  // Collect all cities
  for (auto it: *Citylist::getInstance())
    if (!it->isBurnt() && it->getOwner() != p && it->getNoOfProductionBases() > 1 &&
	it->getOwner() != Playerlist::getInstance()->getNeutral())
      cities.push_back(it);

  // Find a suitable city for us to sack
  if (cities.empty())
    return 0;

  return cities[Rnd::rand() % cities.size()];
}

void QuestCitySack::armyDied(Army *a, bool heroIsCulprit)
{
  (void) a;
  (void) heroIsCulprit;
  //this quest does nothing when an army dies
}

void QuestCitySack::cityAction(City *c, CityDefeatedAction action, 
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
  //did our hero sack the city? success.
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
	d_q_mgr.questExpired(d_hero);
      else if (c->getOwner() == getHero()->getOwner()) // our stack razed
	d_q_mgr.questExpired(d_hero);
      else // their stack did
	d_q_mgr.questExpired(d_hero);
      break;
    case CITY_DEFEATED_SACK: //somebody sacked
      if (heroIsCulprit) // quest hero did
	d_q_mgr.questCompleted(d_hero);
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
