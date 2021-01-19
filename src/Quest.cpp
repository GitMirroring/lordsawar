// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004 Andrea Paternesi
// Copyright (C) 2007, 2008, 2009, 2014, 2021 Ben Asselstine
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
#include "army.h"
#include "xmlhelper.h"
#include "stack.h"
#include "Quest.h"
#include "QuestsManager.h"
#include "hero.h"
#include "playerlist.h"
#include "stacklist.h"
#include "history.h"
#include "QCityOccupy.h"
#include "QCityRaze.h"
#include "QCitySack.h"
#include "QEnemyArmies.h"
#include "QEnemyArmytype.h"
#include "QKillHero.h"
#include "QPillageGold.h"

Glib::ustring Quest::d_tag = "quest";
#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
//#define debug(x)


Quest::Quest(QuestsManager& q_mgr, guint32 hero, Type type)
    :d_q_mgr(q_mgr), d_hero(hero), d_type(type), d_pending(false)
{
  Hero *h = getHeroById(hero);
  if (h)
    {
      setOwnerId(h->getOwner()->getId());
      d_hero_name = h->getName();
    }

}

Quest::Quest (const Quest &q)
 : OwnerId (q), d_q_mgr (q.d_q_mgr), d_description (q.d_description),
    d_hero (q.d_hero), d_type (q.d_type), d_pending (q.d_pending),
    d_hero_name (q.d_hero_name), d_targets (std::list<Vector<int>> (d_targets))
{
}

Quest::Quest(QuestsManager& q_mgr, XML_Helper* helper)
    :OwnerId(helper), d_q_mgr(q_mgr)
{
    Glib::ustring s;
    helper->getData(s, "type");
    d_type = questTypeFromString(s);
    helper->getData(d_hero, "hero");
    helper->getData(d_hero_name, "hero_name");
    helper->getData(d_pending, "pending_deletion");
}

Hero* Quest::getHeroById(guint32 hero, Stack** stack)
{
  for (auto pit: *Playerlist::getInstance())
    for (auto it: *pit->getStacklist())
      for (auto sit: *it)
        {
          if ( (sit->isHero()) && (sit->getId() == hero))
            {
              if (stack)
                *stack = it;
              // isHero is TRUE, so dynamic_cast should succeed
              return dynamic_cast<Hero*>(sit);
            }
        }
  return NULL;
}

bool Quest::save(XML_Helper* helper) const
{
    bool retval = true;

    Glib::ustring s;
    s = questTypeToString(Quest::Type(d_type));
    retval &= helper->saveData("type", s);
    retval &= helper->saveData("hero", d_hero);
    retval &= helper->saveData("hero_name", d_hero_name);
    retval &= helper->saveData("pending_deletion", d_pending);
    retval &= OwnerId::save(helper);

    return retval;
}

Glib::ustring Quest::getHeroNameForDeadHero() const
{
  return getHeroNameForDeadHero(d_hero);
}

Glib::ustring Quest::getHeroNameForDeadHero(guint32 id)
{
  std::list<History *>events;
  events = Playerlist::getInstance()->getHistoryForHeroId(id);
  if (events.size() == 0)
    return "";
  History *history = events.front();
  History_HeroEmerges *event = dynamic_cast<History_HeroEmerges*>(history);
  return event->getHeroName();
}

Glib::ustring Quest::questTypeToString(const Quest::Type type)
{
  switch (type)
    {
    case Quest::KILLHERO: return "Quest::KILLHERO";
    case Quest::KILLARMIES: return "Quest::KILLARMIES";
    case Quest::CITYSACK: return "Quest::CITYSACK";
    case Quest::CITYRAZE: return "Quest::CITYRAZE";
    case Quest::CITYOCCUPY: return "Quest::CITYOCCUPY";
    case Quest::KILLARMYTYPE: return "Quest::KILLARMYTYPE";
    case Quest::PILLAGEGOLD: return "Quest::PILLAGEGOLD";
    }
  return "Quest::KILLHERO";
}

Quest::Type Quest::questTypeFromString(Glib::ustring str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return Quest::Type(atoi(str.c_str()));
  if (str == "Quest::KILLHERO") return Quest::KILLHERO;
  else if (str == "Quest::KILLARMIES") return Quest::KILLARMIES;
  else if (str == "Quest::CITYSACK") return Quest::CITYSACK;
  else if (str == "Quest::CITYRAZE") return Quest::CITYRAZE;
  else if (str == "Quest::CITYOCCUPY") return Quest::CITYOCCUPY;
  else if (str == "Quest::KILLARMYTYPE") return Quest::KILLARMYTYPE;
  else if (str == "Quest::PILLAGEGOLD") return Quest::PILLAGEGOLD;
    
  return Quest::KILLHERO;
}

Quest* Quest::copy(const Quest* q)
{
  switch(q->getType())
    {
    case KILLHERO: 
      return (new QuestKillHero (*dynamic_cast<const QuestKillHero*>(q)));
    case KILLARMIES:
      return (new QuestEnemyArmies (*dynamic_cast<const QuestEnemyArmies*>(q)));
    case CITYSACK: 
      return (new QuestCitySack (*dynamic_cast<const QuestCitySack*>(q)));
    case CITYRAZE: 
      return (new QuestCityRaze (*dynamic_cast<const QuestCityRaze*>(q)));
    case CITYOCCUPY:
      return (new QuestCityOccupy (*dynamic_cast<const QuestCityOccupy*>(q)));
    case KILLARMYTYPE: 
      return (new QuestEnemyArmytype
              (*dynamic_cast<const QuestEnemyArmytype*>(q)));
    case PILLAGEGOLD:
      return (new QuestPillageGold (*dynamic_cast<const QuestPillageGold*>(q)));
    }
  return 0;
}
        
void Quest::setQuestsManager (QuestsManager &q_mgr)
{
  d_q_mgr = q_mgr;
}
