//  Copyright (C) 2007, 2008, 2009, 2014, 2015, 2021, 2026 Ben Asselstine
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

#include <sstream>
#include <sigc++/functors/mem_fun.h>
#include "ucompose.hpp"

#include "xml-helper.h"
#include "stack.h"
#include "quest-enemy-army-type.h"
#include "quest-manager.h"
#include "player-list.h"
#include "stack-list.h"
#include "army-set-list.h"
#include "game-map.h"
#include "player.h"
#include "army-proto.h"
#include "hero.h"
#include "rnd.h"

//go get an existing army type,
//with the stipluation that player P's armies are not taken into consideration
int getVictimArmytype(Player *p, std::list<Vector<int> >&targets)
{
  std::vector<Army*> specials;
  Stacklist::const_iterator sit;
  Stack::iterator it;
  Stacklist *sl;
  for (auto pit: *Playerlist::instance())
    {
      if (pit == p)
	continue;
      sl = pit->getStacklist();
      for (sit = sl->begin(); sit != sl->end(); ++sit)
	{
	  //is this stack not in a city?  no?  it's a target.
	  if (GameMap::getCity((*sit)->getPos()) == NULL)
	    targets.push_back((*sit)->getPos());
	  for (it = (*sit)->begin(); it != (*sit)->end(); ++it)
	    {
	      if ((*it)->getAwardable())
                specials.push_back((*it));
	    }
	}
    }
  if (specials.size() == 0)
    return -1;
  else
    return specials[Rnd::rand() % specials.size()]->getTypeId();
}

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)
QuestEnemyArmytype::QuestEnemyArmytype(guint32 hero)
  : Quest(hero, Quest::KILLARMYTYPE),
  d_type_to_kill (getVictimArmytype (getHero ()->getOwner (), d_targets))
{
  initDescription();
}

QuestEnemyArmytype::QuestEnemyArmytype (const QuestEnemyArmytype &q)
 : Quest (q), sigc::trackable (q), d_type_to_kill (q.d_type_to_kill)
{
}

QuestEnemyArmytype::QuestEnemyArmytype(XML_Helper* helper) 
  : Quest(helper)
{
  helper->get(d_type_to_kill, "type_to_kill");

  initDescription();
}

QuestEnemyArmytype::QuestEnemyArmytype(guint32 hero,
				       guint32 type_to_kill)
  : Quest(hero, Quest::KILLARMYTYPE), d_type_to_kill (type_to_kill)
{
  initDescription();
}

bool QuestEnemyArmytype::save(XML_Helper *helper) const
{
  bool retval = true;

  retval &= helper->open_tag(Quest::d_tag);
  retval &= Quest::save(helper);
  retval &= helper->save("type_to_kill", d_type_to_kill);
  retval &= helper->close_tag();

  return retval;
}

Glib::ustring QuestEnemyArmytype::getProgress() const
{
  guint32 set = Playerlist::instance()->getActiveplayer()->getArmyset();
  const ArmyProto *a = Armysetlist::instance()->getArmy(set, d_type_to_kill);
  return String::ucompose(
			  _("You have not killed a unit of enemy %1 yet."), a->getName());
}

void QuestEnemyArmytype::getSuccessMsg(std::queue<Glib::ustring>& msgs) const
{
  guint32 set = Playerlist::instance()->getActiveplayer()->getArmyset();
  const ArmyProto *a = Armysetlist::instance()->getArmy(set, d_type_to_kill);
  msgs.push(String::ucompose(_("You have killed a unit of enemy %1."), a->getName()));
}

void QuestEnemyArmytype::getExpiredMsg(std::queue<Glib::ustring>& msgs) const
{
  (void) msgs;
  // This quest should never expire, so this is just a dummy function
}

void QuestEnemyArmytype::initDescription()
{
  guint32 set = Playerlist::instance()->getActiveplayer()->getArmyset();
  const ArmyProto *a = Armysetlist::instance()->getArmy(set, d_type_to_kill);
  d_description = String::ucompose(_("You must destroy a unit of enemy %1."), 
				   a->getName());
}

bool QuestEnemyArmytype::isFeasible(guint32 heroId)
{
  std::list< Vector<int> >targets;
  int type = getVictimArmytype(getHeroById(heroId)->getOwner(), targets);
  if (type >= 0)
    return true;
  return false;
}

void QuestEnemyArmytype::armyDied(Army *a, bool heroIsCulprit)
{
  //was it the army type we were after?

  debug("QuestEnemyArmytype: armyDied - pending = " << (int)d_pending);

  if (isPendingDeletion())
    return;
  Hero *h = getHero();
  if (!h || h->getHP() <= 0)
    {
      deactivate();
      return;
    }

  if (a->getTypeId() == d_type_to_kill)
    {
      if (heroIsCulprit)
	{
	  debug("CONGRATULATIONS: QUEST 'KILL ENEMY ARMYTYPE' IS COMPLETED!");
          QuestsManager::instance()->questCompleted(d_hero);
	}
    }
}

void QuestEnemyArmytype::cityAction(City *c, CityDefeatedChoice action, 
				    bool heroIsCulprit, int gold)
{
  (void) c;
  (void) action;
  (void) heroIsCulprit;
  (void) gold;
}
