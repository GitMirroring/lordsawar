//  Copyright (C) 2007, 2008, 2009, 2014, 2015, 2021 Ben Asselstine
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

#include <sstream>
#include <sigc++/functors/mem_fun.h>
#include "ucompose.hpp"

#include "army.h"
#include "QPillageGold.h"
#include "QuestsManager.h"
#include "playerlist.h"
#include "city.h"
#include "xmlhelper.h"
#include "hero.h"
#include "rnd.h"
#include "GameScenarioOptions.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

QuestPillageGold::QuestPillageGold(QuestsManager& q_mgr, guint32 hero)
  : Quest(q_mgr, hero, Quest::PILLAGEGOLD), 
    d_to_pillage (850 + (Rnd::rand() % 630)), d_pillaged(0)
{
  initDescription();
}

QuestPillageGold::QuestPillageGold(QuestsManager& q_mgr, XML_Helper* helper) 
  : Quest(q_mgr, helper)
{
  helper->getData(d_to_pillage, "to_pillage");
  helper->getData(d_pillaged, "pillaged");

  initDescription();
}

QuestPillageGold::QuestPillageGold (const QuestPillageGold &q)
 : Quest (q), sigc::trackable (q), d_to_pillage (q.d_to_pillage),
    d_pillaged (q.d_pillaged)
{
}

QuestPillageGold::QuestPillageGold(QuestsManager& q_mgr, guint32 hero, guint32 gold)
  : Quest(q_mgr, hero, Quest::PILLAGEGOLD), d_to_pillage (gold), d_pillaged(0)
{
  initDescription();
}

bool QuestPillageGold::save(XML_Helper *helper) const
{
  bool retval = true;

  retval &= helper->openTag(Quest::d_tag);
  retval &= Quest::save(helper);
  retval &= helper->saveData("to_pillage", d_to_pillage);
  retval &= helper->saveData("pillaged",  d_pillaged);
  retval &= helper->closeTag();

  return retval;
}

Glib::ustring QuestPillageGold::getProgress() const
{
  return String::ucompose(_("You have already stolen %1 gold pieces."), d_pillaged);
}

void QuestPillageGold::getSuccessMsg(std::queue<Glib::ustring>& msgs) const
{
  msgs.push(String::ucompose(_("You have managed to sack and pillage %1 gold."), d_pillaged));
  msgs.push(_("Well done!"));
}

void QuestPillageGold::getExpiredMsg(std::queue<Glib::ustring>& msgs) const
{
  (void) msgs;
    // This quest should never expire, so this is just a dummy function
}

void QuestPillageGold::initDescription()
{
  d_description = String::ucompose(_("You shall sack and pillage %1 gold from thy mighty foes."), d_to_pillage);
}
	
void QuestPillageGold::armyDied(Army *a, bool heroIsCulprit)
{
  (void) a;
  (void) heroIsCulprit;
}

void QuestPillageGold::cityAction(City *c, CityDefeatedAction action, 
				  bool heroIsCulprit, int gold)
{
  (void) c;
  (void) gold;
  if (isPendingDeletion())
    return;
  Hero *h = getHero();
  if (!h || h->getHP() <= 0)
    {
      deactivate();
      return;
    }
  if (action == CITY_DEFEATED_SACK || action == CITY_DEFEATED_PILLAGE)
    {
      if (heroIsCulprit)
	{
	  d_pillaged += gold;
	  if (d_pillaged > d_to_pillage)
	    {
	      d_pillaged = d_to_pillage;
	      d_q_mgr.questCompleted(d_hero);
	    }
	}
    }
}

bool QuestPillageGold::isFeasible(guint32 heroId)
{
  if (GameScenarioOptions::s_sacking_mode == GameParameters::SACKING_NEVER ||
      GameScenarioOptions::s_sacking_mode == GameParameters::SACKING_ON_CAPTURE)
    return false;
  if (heroId)
    return true;
  return false;
}

