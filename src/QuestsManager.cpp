// Copyright (C) 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2007-2009, 2011, 2014, 2015, 2017, 2021 Ben Asselstine
// Copyright (C) 2007, 2008 Ole Laursen
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

#include "QuestsManager.h"

#include "Quest.h"
#include "QKillHero.h"
#include "QEnemyArmies.h"
#include "QCitySack.h"
#include "QCityRaze.h"
#include "QCityOccupy.h"
#include "QEnemyArmytype.h"
#include "QPillageGold.h"
#include "stacklist.h"
#include "SightMap.h"
#include "rewardlist.h"
#include "army.h"
#include "xmlhelper.h"
#include "history.h"
#include "stackreflist.h"
#include "hero.h"
#include "rnd.h"

Glib::ustring QuestsManager::d_tag = "questlist";

QuestsManager* QuestsManager::s_instance = NULL;

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)


QuestsManager* QuestsManager::getInstance()
{
    if (s_instance == 0)
        s_instance = new QuestsManager();
    return s_instance;
}

QuestsManager* QuestsManager::getInstance(XML_Helper* helper)
{
    if (s_instance)
        deleteInstance();

    s_instance = new QuestsManager(helper);
    return s_instance;
}

void QuestsManager::deleteInstance()
{
    debug("QuestsManager: deleteInstance")
    delete s_instance;
    s_instance = 0;
}

QuestsManager::QuestsManager()
{
    sharedInit();
}

QuestsManager::QuestsManager(const QuestsManager &q)
: sigc::trackable (q),
    d_quests (std::map<guint32,Quest*>()),
    d_inactive_quests (std::list<Quest*>()),
    d_completed_quests (std::list<Quest*>()),
    d_questsFeasible (std::vector<QFeasibilityType>(q.d_questsFeasible))
{
  for (auto a : q.d_quests)
    {
      Quest *quest = Quest::copy (a.second);
      quest->setQuestsManager (*this);
      d_quests[a.first] = quest;
    }

  for (auto b : q.d_inactive_quests)
    {
      Quest *quest = Quest::copy (b);
      quest->setQuestsManager (*this);
      d_inactive_quests.push_back (quest);
    }

  for (auto c : q.d_completed_quests)
    {
      Quest *quest = Quest::copy (c);
      quest->setQuestsManager (*this);
      d_completed_quests.push_back (quest);
    }
}

QuestsManager::QuestsManager(XML_Helper* helper)
{
    sharedInit();
    debug("QuestsManager: registerTag!");
    helper->registerTag(Quest::d_tag, sigc::mem_fun(this, &QuestsManager::load));
}

QuestsManager::~QuestsManager()
{
  d_completed_quests.unique ();
  for (auto q : d_completed_quests)
    delete q;

  for (std::map<guint32,Quest*>::iterator it = d_quests.begin();
       it != d_quests.end(); ++it)
    delete (*it).second;
  cleanup();
}

Quest* QuestsManager::createNewQuest(guint32 heroId, bool razing_possible)
{
    // don't let a hero have more than one quest
    if (d_quests.count(heroId))
        return NULL;

    int which = 0;
    while (!which)
    {
        which = 1 + Rnd::rand() % 7;
        // if this quest is not feasible - try again with another
        // quest:
        if ((*(d_questsFeasible[which-1]))(heroId) == 0)
            which = 0;
    }
    // ok - this quest can be completed

    Quest *quest = NULL;
    switch (which)
    {
        case 1:
            quest = new QuestKillHero(*this, heroId);
            break;
        case 2:
            quest = new QuestEnemyArmies(*this, heroId);
            break;
        case 3:
            quest = new QuestCitySack(*this, heroId);
            break;
        case 4:
	    if (razing_possible)
	      quest = new QuestCityRaze(*this, heroId);
	    else
	      quest = new QuestCitySack(*this, heroId);
            break;
        case 5:
            quest = new QuestCityOccupy(*this, heroId);
            break;
        case 6:
            quest = new QuestEnemyArmytype(*this, heroId);
            break;
        case 7:
            quest = new QuestPillageGold(*this, heroId);
            break;
    }

    if (quest)
    {
        d_quests[heroId] = quest;
    }

    return quest;
}

Quest* QuestsManager::createNewKillHeroQuest(guint32 heroId, guint32 targetHeroId)
{
  Quest *quest = new QuestKillHero(*this, heroId, targetHeroId);

  d_quests[heroId] = quest;

  return quest;
}

Quest* QuestsManager::createNewEnemyArmiesQuest(guint32 heroId,
						guint32 num_armies,
						guint32 victim_player_id)
{
  Quest *quest = new QuestEnemyArmies(*this, heroId, num_armies,
				      victim_player_id);

  d_quests[heroId] = quest;

  return quest;
}

Quest* QuestsManager::createNewCitySackQuest(guint32 heroId, guint32 cityId)
{
  Quest *quest = new QuestCitySack(*this, heroId, cityId);

  d_quests[heroId] = quest;

  return quest;
}

Quest* QuestsManager::createNewCityRazeQuest(guint32 heroId, guint32 cityId)
{
  Quest *quest = new QuestCityRaze(*this, heroId, cityId);

  d_quests[heroId] = quest;

  return quest;
}

Quest* QuestsManager::createNewCityOccupyQuest(guint32 heroId, guint32 cityId)
{
  Quest *quest = new QuestCityOccupy(*this, heroId, cityId);

  d_quests[heroId] = quest;

  return quest;
}

Quest* QuestsManager::createNewEnemyArmytypeQuest(guint32 heroId,
						  guint32 armyTypeId)
{
  Quest *quest = new QuestEnemyArmytype(*this, heroId, armyTypeId);

  d_quests[heroId] = quest;

  return quest;
}

Quest* QuestsManager::createNewPillageGoldQuest(guint32 heroId, guint32 amount)
{
  Quest *quest = new QuestPillageGold(*this, heroId, amount);

  d_quests[heroId] = quest;

  return quest;
}

void QuestsManager::questCompleted(guint32 heroId)
{
    Quest *quest = getHeroQuest (heroId);
    Player *p = quest->getHero()->getOwner();

    p->heroCompletesQuest(quest->getHero());
    Stack *stack = p->getStacklist()->getArmyStackById(heroId);

    Reward *reward = Reward::createRandomReward(true, true);
    StackReflist *stacks = new StackReflist();
    p->giveReward(stack, reward, stacks, true);
    quest_completed.emit(quest, reward);
    if (reward->getType() == Reward::ALLIES)
      p->addHistory(new History_HeroFindsAllies(quest->getHero()));
    else if (reward->getType() == Reward::RUIN)
      {
        Ruin *r = dynamic_cast<Reward_Ruin*>(reward)->getRuin();
        p->addHistory(new History_HeroRewardRuin(dynamic_cast<Hero*>(quest->getHero()), r));
      }
    delete reward;
    delete stacks;

    //debug("deactivate quest");

    quest->deactivate ();
    d_quests.erase(heroId);
    d_completed_quests.push_back (quest);
    //debug("quest deactivated");
}

void QuestsManager::questExpired(guint32 heroId)
{
    Quest *quest = getHeroQuest (heroId);

    if (quest == 0)
        return;

    //quest_expired.emit(quest);

    debug("deactivate quest");
    deactivateQuest(heroId);
    debug("quest deactivated");
}

std::vector<Quest*> QuestsManager::getPlayerQuests(const Player *player) const
{
  std::vector<Quest*> res;
  // loop through the player's heroes
  // for every hero check any pending quests
  const Stacklist* sl = player->getStacklist();
  std::list<Hero*> heroes = sl->getHeroes();
  for (std::list<Hero*>::iterator it = heroes.begin(); it != heroes.end(); ++it)
    {
      guint32 heroId = (*it)->getId();

      if (d_quests.count(heroId) > 0)
        {
          std::map<guint32, Quest*>::const_iterator qit = d_quests.find(heroId);
          if (qit == d_quests.end())
            continue;
          Quest *q = (*qit).second;
          if (q && q->isPendingDeletion() == true)
            continue;
          if (!q)
            continue;
          debug("heroId = " << heroId << " - has quest: " << q);
          res.push_back(q);
        }
    }
  return res;
}

Quest* QuestsManager::getHeroQuest(guint32 hero_id) const
{
  std::map<guint32, Quest*>::const_iterator qit;
  qit = d_quests.find(hero_id);
  if (qit == d_quests.end())
    return NULL;
  Quest *q = (*qit).second;
  if (q && q->isPendingDeletion() == true)
    return NULL;
  return (*qit).second;
}

bool QuestsManager::save(XML_Helper* helper) const
{
  debug("Saving quests\n");

  bool retval = true;
  retval &= helper->openTag(QuestsManager::d_tag);

  for (std::map<guint32,Quest*>::const_iterator it = d_quests.begin();
       it != d_quests.end(); ++it)
    {
      if ((*it).second == NULL)
	continue;
      retval &= ((*it).second)->save(helper);
    }
  for (std::list<Quest *>::const_iterator it = d_inactive_quests.begin();
       it != d_inactive_quests.end(); ++it)
    retval &= (*it)->save(helper);

  debug("Quests saved\n");
  retval &= helper->closeTag();
  return retval;
}

bool QuestsManager::load(Glib::ustring tag, XML_Helper* helper)
{
  debug("QuestsManager: load tag = " << tag);

  if (tag == Quest::d_tag)
    {
      guint32  questType, hero;
      Glib::ustring quest_type_str;
      helper->getData(quest_type_str, "type");
      questType = Quest::questTypeFromString(quest_type_str);
      helper->getData(hero, "hero");

      debug("quest load: type = " << questType << ", heroId = " << hero);

      Quest *quest=0;
      switch (static_cast<Quest::Type>(questType)) {
      case Quest::KILLHERO:
	quest = new QuestKillHero(*this, helper);
	break;
      case Quest::KILLARMIES:
	quest = new QuestEnemyArmies(*this, helper);
	break;
      case Quest::CITYSACK:
	quest = new QuestCitySack(*this, helper);
	break;
      case Quest::CITYRAZE:
	quest = new QuestCityRaze(*this, helper);
	break;
      case Quest::CITYOCCUPY:
	quest = new QuestCityOccupy(*this, helper);
	break;
      case Quest::KILLARMYTYPE:
	quest = new QuestEnemyArmytype(*this, helper);
	break;
      case Quest::PILLAGEGOLD:
	quest = new QuestPillageGold(*this, helper);
	break;
      }

      debug("quest created: q = " << quest);
      if (quest)
	{
	  if (quest->isPendingDeletion())
	    d_inactive_quests.push_back(quest);
	  else
	    d_quests[hero] = quest;
	}
      return true;
    }
  return false;
}

void QuestsManager::sharedInit()
{
  debug("QuestsManager constructor")

    // now prepare the vector of pointers to the
    // functions (class static members) checking feasibility
    // for every quest
    d_questsFeasible.push_back(&(QuestKillHero::isFeasible));
  d_questsFeasible.push_back(&(QuestEnemyArmies::isFeasible));
  d_questsFeasible.push_back(&(QuestCitySack::isFeasible));
  d_questsFeasible.push_back(&(QuestCityRaze::isFeasible));
  d_questsFeasible.push_back(&(QuestCityOccupy::isFeasible));
  d_questsFeasible.push_back(&(QuestEnemyArmytype::isFeasible));
  d_questsFeasible.push_back(&(QuestPillageGold::isFeasible));
}

void QuestsManager::deactivateQuest(guint32 heroId)
{
  Quest *q = getHeroQuest (heroId);
  q->deactivate();
  d_inactive_quests.push_back(q);
  // delete it from hash of active quests
  d_quests.erase(heroId);
}

void QuestsManager::cleanup()
{
  debug("QuestsManager: cleanup!");

  std::list<Quest *>::iterator it = d_inactive_quests.begin();
  while(it != d_inactive_quests.end())
    {
      Quest *q =(*it);
      it = d_inactive_quests.erase(it);
      if (q)
	delete q;
    }
}

std::vector<Quest*> QuestsManager::getActiveQuests ()
{
  std::vector<Quest*> quests;
  for (std::map<guint32,Quest*>::iterator it = d_quests.begin();
       it != d_quests.end(); ++it)
    {
      if ((*it).second == NULL)
	continue;
      if ((*it).second->isPendingDeletion() == true)
	continue;
      quests.push_back ((*it).second);
    }
  return quests;
}

void QuestsManager::armyDied(Army *a, std::vector<guint32>& culprits)
{
  //tell all quests that an army died
  //each quest takes care of what happens when an army dies
  std::vector<Quest *> quests = getActiveQuests ();
  for (auto q : quests)
    {
      //was this hero a perpetrator?
      bool heroIsCulprit = false;
      for (unsigned int i = 0; i <culprits.size(); i++)
	{
	  if (culprits[i] == q->getHeroId())
	    {
	      heroIsCulprit = true;
	      break;
	    }
	}
      q->armyDied(a, heroIsCulprit);
    }

  //is it a hero that has an outstanding quest?
  //this is what deactivates a quest upon hero death
  Quest *quest = getHeroQuest (a->getId());
  if (quest && quest->isPendingDeletion() == false)
    questExpired(a->getId());
}

void QuestsManager::cityAction(City *c, Stack *s,
			       CityDefeatedAction action, int gold)
{
  std::vector<Quest *> quests = getActiveQuests ();
  for (auto q : quests)
    {
      if (!s)
        q->cityAction(c, action, false, gold);
      else
	{
          //XXX XXX XXX why do we have to check for null here?
	  for (Stack::iterator sit = s->begin(); sit != s->end(); ++sit)
	    {
              if (q->isPendingDeletion())
		break;
	      if ((*sit)->getId() == q->getHeroId())
		q->cityAction(c, action, true, gold);
	    }
	  for (Stack::iterator sit = s->begin(); sit != s->end(); ++sit)
	    {
              if (q->isPendingDeletion())
		break;
	      if ((*sit)->getId() != q->getHeroId())
		q->cityAction(c, action, false, gold);
	    }
	}
    }
}

void QuestsManager::cityRazed(City *c, Stack *s)
{
  cityAction(c, s, CITY_DEFEATED_RAZE, 0);
  //did we raze a city we care about in another quest?
}

void QuestsManager::citySacked(City *c, Stack *s, int gold)
{
  cityAction(c, s, CITY_DEFEATED_SACK, gold);
}

void QuestsManager::cityPillaged(City *c, Stack *s, int gold)
{
  cityAction(c, s, CITY_DEFEATED_PILLAGE, gold);
}

void QuestsManager::cityOccupied(City *c, Stack *s)
{
  cityAction(c, s, CITY_DEFEATED_OCCUPY, 0);
}

void QuestsManager::nextTurn(Player *p)
{
  d_completed_quests.unique ();
  for (auto q : d_completed_quests)
    delete q;
  d_completed_quests.clear ();
  // go through our inactive list and remove quests belonging to us
  for (std::list<Quest*>::iterator it = d_inactive_quests.begin();
       it != d_inactive_quests.end(); ++it)
    {
      if ((*it)->getOwner() == p)
	{
	  Quest *q = *it;
	  quest_expired.emit(q);
	  it = d_inactive_quests.erase(it);
	  if (q)
	    delete q;
	}
    }
}

void QuestsManager::reset (QuestsManager *q)
{
  delete s_instance;
  s_instance = q;
}
