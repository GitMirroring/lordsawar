//  Copyright (C) 2007, 2008, 2011, 2014, 2015, 2026 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>

#include "army.h"
#include "history.h"
#include "hero.h"
#include "hero-proto.h"
#include "city.h"
#include "xml-helper.h"
#include "ruin.h"
#include "item.h"
#include "player.h"
#include "ucompose.hpp"

Glib::ustring History::d_tag = "history";

#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<< x << std::endl<<std::flush;}
//#define debug(x)

History::History(Type type)
:d_type(type)
{
}

History* History::handle_load(XML_Helper* helper)
{
  Glib::ustring type_str;
  helper->get(type_str, "type");
  History::Type t = historyTypeFromString(type_str);

  switch (t)
    {
    case START_TURN:
      return (new History_StartTurn(helper));
    case FOUND_SAGE:
      return (new History_FoundSage(helper));
    case GOLD_TOTAL:
      return (new History_GoldTotal(helper));
    case HERO_EMERGES:
      return (new History_HeroEmerges(helper));
    case CITY_WON:
      return (new History_CityWon(helper));
    case HERO_CITY_WON:
      return (new History_HeroCityWon(helper));
    case CITY_RAZED:
      return (new History_CityRazed(helper));
    case HERO_QUEST_STARTED:
      return (new History_HeroQuestStarted(helper));
    case HERO_QUEST_COMPLETED:
      return (new History_HeroQuestCompleted(helper));
    case HERO_KILLED_IN_CITY:
      return (new History_HeroKilledInCity(helper));
    case HERO_KILLED_IN_BATTLE:
      return (new History_HeroKilledInBattle(helper));
    case HERO_KILLED_SEARCHING:
      return (new History_HeroKilledSearching(helper));
    case SCORE:
      return (new History_Score(helper));
    case PLAYER_VANQUISHED:
      return (new History_PlayerVanquished(helper));
    case DIPLOMATIC_PEACE:
      return (new History_DiplomacyPeace(helper));
    case DIPLOMATIC_WAR:
      return (new History_DiplomacyWar(helper));
    case DIPLOMATIC_TREACHERY:
      return (new History_DiplomacyTreachery(helper));
    case HERO_FINDS_ALLIES:
      return (new History_HeroFindsAllies(helper));
    case END_TURN:
      return (new History_EndTurn(helper));
    case HERO_RUIN_EXPLORED:
      return (new History_HeroRuinExplored(helper));
    case HERO_REWARD_RUIN:
      return (new History_HeroRewardRuin(helper));
    case USE_ITEM:
      return (new History_HeroUseItem(helper));
    case HERO_QUEST_EXPIRED:
      return (new History_HeroQuestExpired(helper));
    }

  return 0;
}

History* History::copy(const History* a)
{
  switch(a->getType())
    {
    case START_TURN:
      return 
	(new History_StartTurn(*dynamic_cast<const History_StartTurn*>(a)));
    case FOUND_SAGE:
      return 
	(new History_FoundSage(*dynamic_cast<const History_FoundSage*>(a)));
    case GOLD_TOTAL:
      return 
	(new History_GoldTotal(*dynamic_cast<const History_GoldTotal*>(a)));
    case HERO_EMERGES:
      return 
	(new History_HeroEmerges(*dynamic_cast<const History_HeroEmerges*>(a)));
    case CITY_WON:
      return 
	(new History_CityWon(*dynamic_cast<const History_CityWon*>(a)));
    case HERO_CITY_WON:
      return 
	(new History_HeroCityWon(*dynamic_cast<const History_HeroCityWon*>(a)));
    case CITY_RAZED:
      return 
	(new History_CityRazed(*dynamic_cast<const History_CityRazed*>(a)));
    case HERO_QUEST_STARTED:
      return 
	(new History_HeroQuestStarted
          (*dynamic_cast<const History_HeroQuestStarted*>(a)));
    case HERO_QUEST_COMPLETED:
      return 
	(new History_HeroQuestCompleted
          (*dynamic_cast<const History_HeroQuestCompleted*>(a)));
    case HERO_KILLED_IN_CITY:
      return 
	(new History_HeroKilledInCity
          (*dynamic_cast<const History_HeroKilledInCity*>(a)));
    case HERO_KILLED_IN_BATTLE:
      return 
	(new History_HeroKilledInBattle
          (*dynamic_cast<const History_HeroKilledInBattle*>(a)));
    case HERO_KILLED_SEARCHING:
      return 
	(new History_HeroKilledSearching
          (*dynamic_cast<const History_HeroKilledSearching*>(a)));
    case SCORE:
      return 
	(new History_Score(*dynamic_cast<const History_Score*>(a)));
    case PLAYER_VANQUISHED:
      return 
	(new History_PlayerVanquished
	 (*dynamic_cast<const History_PlayerVanquished*>(a)));
    case DIPLOMATIC_PEACE:
      return 
	(new History_DiplomacyPeace
	 (*dynamic_cast<const History_DiplomacyPeace*>(a)));
    case DIPLOMATIC_WAR:
      return 
	(new History_DiplomacyWar
	 (*dynamic_cast<const History_DiplomacyWar*>(a)));
    case DIPLOMATIC_TREACHERY:
      return 
	(new History_DiplomacyTreachery
	 (*dynamic_cast<const History_DiplomacyTreachery*>(a)));
    case HERO_FINDS_ALLIES:
      return 
	(new History_HeroFindsAllies
          (*dynamic_cast<const History_HeroFindsAllies*>(a)));
    case END_TURN:
      return 
	(new History_EndTurn(*dynamic_cast<const History_EndTurn*>(a)));
    case HERO_RUIN_EXPLORED:
      return 
	(new History_HeroRuinExplored
         (*dynamic_cast<const History_HeroRuinExplored*>(a)));
    case HERO_REWARD_RUIN:
      return 
	(new History_HeroRewardRuin
         (*dynamic_cast<const History_HeroRewardRuin*>(a)));
    case USE_ITEM:
      return 
	(new History_HeroUseItem
         (*dynamic_cast<const History_HeroUseItem*>(a)));
    case HERO_QUEST_EXPIRED:
      return 
	(new History_HeroQuestExpired
          (*dynamic_cast<const History_HeroQuestExpired*>(a)));
    }

  return 0;
}

History::History (XML_Helper *helper)
{
  Glib::ustring type_str;
  helper->get(type_str, "type");
  d_type = historyTypeFromString(type_str);
}

bool History::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->open_tag(History::d_tag);
    retval &= saveContents(helper);
    retval &= helper->close_tag();

    return retval;
}

bool History::saveContents(XML_Helper* helper) const
{
    bool retval = true;

    Glib::ustring type_str = historyTypeToString(History::Type(d_type));
    retval &= helper->save("type", type_str);
    retval &= doSave(helper);

    return retval;
}

//-----------------------------------------------------------------------------
//History_StartTurn

History_StartTurn::History_StartTurn()
:History(History::START_TURN)
{
}

History_StartTurn::History_StartTurn(const History_StartTurn &history)
:History(history)
{
}

History_StartTurn::History_StartTurn(XML_Helper* helper)
:History(helper)
{
}

Glib::ustring History_StartTurn::dump() const
{
  return "Player starts a turn.\n";
}

bool History_StartTurn::doSave(XML_Helper* helper) const
{
  if (helper)
    return true;
  return false;
}

//-----------------------------------------------------------------------------
//History_FoundSage

History_FoundSage::History_FoundSage(Hero *hero)
:History(History::FOUND_SAGE), d_hero(hero->getName())
{
}

History_FoundSage::History_FoundSage(const History_FoundSage &history)
:History(history), d_hero(history.d_hero)
{
}

History_FoundSage::History_FoundSage(XML_Helper* helper)
:History(helper)
{
  helper->get(d_hero, "hero");
}

Glib::ustring History_FoundSage::dump() const
{
  return String::ucompose("Player found a sage with hero %1.\n", d_hero);
}

bool History_FoundSage::doSave(XML_Helper* helper) const
{
  return helper->save("hero", d_hero);
}

//-----------------------------------------------------------------------------
//History_GoldTotal

History_GoldTotal::History_GoldTotal(int gold)
:History(History::GOLD_TOTAL), d_gold(gold)
{
}

History_GoldTotal::History_GoldTotal(const History_GoldTotal &history)
:History(history), d_gold(history.d_gold)
{
}

History_GoldTotal::History_GoldTotal(XML_Helper* helper)
:History(helper)
{
  helper->get(d_gold, "gold");
}

Glib::ustring History_GoldTotal::dump() const
{
  return String::ucompose("Player has %1 gold pieces in total.\n", d_gold);
}

bool History_GoldTotal::doSave(XML_Helper* helper) const
{
  return helper->save("gold", d_gold);
}

//-----------------------------------------------------------------------------
//History_HeroEmerges

History_HeroEmerges::History_HeroEmerges(Hero *hero, City *city)
:History(History::HERO_EMERGES), d_hero(hero->getName()), 
    d_hero_id(hero->getId()), d_city(city->getName())
{
}

History_HeroEmerges::History_HeroEmerges(const History_HeroEmerges &h)
:History(h), d_hero(h.d_hero), d_hero_id(h.d_hero_id), d_city(h.d_city)
{
}

History_HeroEmerges::History_HeroEmerges(XML_Helper* helper)
:History(helper)
{
  helper->get(d_hero, "hero");
  helper->get(d_city, "city");
  helper->get(d_hero_id, "hero_id");
}

Glib::ustring History_HeroEmerges::dump() const
{
  return String::ucompose("Hero %1 emerges in city %2.\n", d_hero, d_city);
}

bool History_HeroEmerges::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->save("hero", d_hero);
  retval &= helper->save("city", d_city);
  retval &= helper->save("hero_id", d_hero_id);

  return retval;
}

//-----------------------------------------------------------------------------
//History_CityWon

History_CityWon::History_CityWon(City *city)
:History(History::CITY_WON), d_city(city->getId())
{
}

History_CityWon::History_CityWon(const History_CityWon &history)
:History(history), d_city(history.d_city)
{
}

History_CityWon::History_CityWon(XML_Helper* helper)
:History(helper)
{
  helper->get(d_city, "city");
}

Glib::ustring History_CityWon::dump() const
{
  return String::ucompose("City %1 has been won.\n", d_city);
}

bool History_CityWon::doSave(XML_Helper* helper) const
{
  return helper->save("city", d_city);
}

//-----------------------------------------------------------------------------
//History_HeroCityWon

History_HeroCityWon::History_HeroCityWon(City *c, Hero *h)
:History(History::HERO_CITY_WON), d_hero(h->getName()), d_city(c->getName())
{
}

History_HeroCityWon::History_HeroCityWon(const History_HeroCityWon &history)
:History(history), d_hero(history.d_hero), d_city(history.d_city)
{
}

History_HeroCityWon::History_HeroCityWon(XML_Helper* helper)
:History(helper)
{
  helper->get(d_city, "city");
  helper->get(d_hero, "hero");
}

Glib::ustring History_HeroCityWon::dump() const
{
  return String::ucompose("City %1 has been won by hero %2.\n", d_city, d_hero);
}

bool History_HeroCityWon::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->save("city", d_city);
  retval &= helper->save("hero", d_hero);

  return retval;
}

//-----------------------------------------------------------------------------
//History_CityRazed

History_CityRazed::History_CityRazed(City *c)
:History(History::CITY_RAZED), d_city(c->getId())
{
}

History_CityRazed::History_CityRazed(const History_CityRazed &history)
:History(history), d_city(history.d_city)
{
}

History_CityRazed::History_CityRazed(XML_Helper* helper)
:History(helper)
{
  helper->get(d_city, "city");
}

Glib::ustring History_CityRazed::dump() const
{
  return String::ucompose("City %1 has been razed.\n", d_city);
}

bool History_CityRazed::doSave(XML_Helper* helper) const
{
  return helper->save("city", d_city);
}

//-----------------------------------------------------------------------------
//History_HeroQuestStarted

History_HeroQuestStarted::History_HeroQuestStarted(Hero *h)
:History(History::HERO_QUEST_STARTED), d_hero(h->getName())
{
}

History_HeroQuestStarted::History_HeroQuestStarted(const History_HeroQuestStarted &history)
:History(history), d_hero(history.d_hero)
{
}

History_HeroQuestStarted::History_HeroQuestStarted(XML_Helper* helper)
:History(helper)
{
  helper->get(d_hero, "hero");
}

Glib::ustring History_HeroQuestStarted::dump() const
{
  return String::ucompose("Hero %1 gets a quest.\n", d_hero);
}

bool History_HeroQuestStarted::doSave(XML_Helper* helper) const
{
  return helper->save("hero", d_hero);
}

//-----------------------------------------------------------------------------
//History_HeroQuestCompleted

History_HeroQuestCompleted::History_HeroQuestCompleted(Hero *h)
:History(History::HERO_QUEST_COMPLETED), d_hero(h->getName())
{
}

History_HeroQuestCompleted::History_HeroQuestCompleted(const History_HeroQuestCompleted &history)
:History(history), d_hero(history.d_hero)
{
}

History_HeroQuestCompleted::History_HeroQuestCompleted(XML_Helper* helper)
:History(helper)
{
  helper->get(d_hero, "hero");
}

Glib::ustring History_HeroQuestCompleted::dump() const
{
  return String::ucompose("Hero %1 completes a quest.\n", d_hero);
}

bool History_HeroQuestCompleted::doSave(XML_Helper* helper) const
{
  return helper->save("hero", d_hero);
}

//-----------------------------------------------------------------------------
//History_HeroKilledInCity

History_HeroKilledInCity::History_HeroKilledInCity(Hero *h, City *c)
:History(History::HERO_KILLED_IN_CITY), d_hero(h->getName()), d_city(c->getName())
{
}

History_HeroKilledInCity::History_HeroKilledInCity(const History_HeroKilledInCity &history)
:History(history), d_hero(history.d_hero), d_city(history.d_city)
{
}

History_HeroKilledInCity::History_HeroKilledInCity(XML_Helper* helper)
:History(helper)
{
  helper->get(d_hero, "hero");
  helper->get(d_city, "city");
}

Glib::ustring History_HeroKilledInCity::dump() const
{
  return String::ucompose("Hero %1 died in city %2.\n", d_hero, d_city);
}

bool History_HeroKilledInCity::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->save("hero", d_hero);
  retval &= helper->save("city", d_city);

  return retval;
}

//-----------------------------------------------------------------------------
//History_HeroKilledInBattle

History_HeroKilledInBattle::History_HeroKilledInBattle(Hero *h)
:History(History::HERO_KILLED_IN_BATTLE), d_hero(h->getName())
{
}

History_HeroKilledInBattle::History_HeroKilledInBattle(const History_HeroKilledInBattle &history)
:History(history), d_hero(history.d_hero)
{
}

History_HeroKilledInBattle::History_HeroKilledInBattle(XML_Helper* helper)
:History(helper)
{
  helper->get(d_hero, "hero");
}

Glib::ustring History_HeroKilledInBattle::dump() const
{
  return String::ucompose("Hero %1 was killed in battle.\n", d_hero);
}

bool History_HeroKilledInBattle::doSave(XML_Helper* helper) const
{
  return helper->save("hero", d_hero);
}

//-----------------------------------------------------------------------------
//History_Hero KilledSearching

History_HeroKilledSearching::History_HeroKilledSearching(Hero *h)
:History(History::HERO_KILLED_SEARCHING), d_hero(h->getName())
{
}

History_HeroKilledSearching::History_HeroKilledSearching(const History_HeroKilledSearching &history)
:History(history), d_hero(history.d_hero)
{
}

History_HeroKilledSearching::History_HeroKilledSearching(XML_Helper* helper)
:History(helper)
{
  helper->get(d_hero, "hero");
}

Glib::ustring History_HeroKilledSearching::dump() const
{
  return String::ucompose("Hero %1 killed searching a ruin.\n", d_hero);
}

bool History_HeroKilledSearching::doSave(XML_Helper* helper) const
{
  return helper->save("hero", d_hero);
}

//-----------------------------------------------------------------------------
//History_Score

History_Score::History_Score(guint32 score)
:History(History::SCORE), d_score(score)
{
}

History_Score::History_Score(const History_Score &history)
:History(history), d_score(history.d_score)
{
}

History_Score::History_Score(XML_Helper* helper)
:History(helper)
{
  helper->get(d_score, "score");
}

Glib::ustring History_Score::dump() const
{
  return String::ucompose("Player has a score of %1.\n", d_score);
}

bool History_Score::doSave(XML_Helper* helper) const
{
  return helper->save("score", d_score);
}

//-----------------------------------------------------------------------------
//History_PlayerVanquished

History_PlayerVanquished::History_PlayerVanquished()
:History(History::PLAYER_VANQUISHED)
{
}

History_PlayerVanquished::History_PlayerVanquished(const History_PlayerVanquished &history)
:History(history)
{
}

History_PlayerVanquished::History_PlayerVanquished(XML_Helper* helper)
:History(helper)
{
}

Glib::ustring History_PlayerVanquished::dump() const
{
  return "Player has been vanquished!\n";
}

bool History_PlayerVanquished::doSave(XML_Helper* helper) const
{
  if (helper)
    return true;
  return false;
}

//-----------------------------------------------------------------------------
//History_DiplomacyPeace

History_DiplomacyPeace::History_DiplomacyPeace(Player *p)
:History(History::DIPLOMATIC_PEACE), d_opponent_id(p->getId())
{
}

History_DiplomacyPeace::History_DiplomacyPeace(const History_DiplomacyPeace &history)
:History(history), d_opponent_id(history.d_opponent_id)
{
}

History_DiplomacyPeace::History_DiplomacyPeace(XML_Helper* helper)
:History(helper)
{
  helper->get(d_opponent_id, "opponent_id");
}

Glib::ustring History_DiplomacyPeace::dump() const
{
  return String::ucompose("Peace has been won with player %1.\n", d_opponent_id);
}

bool History_DiplomacyPeace::doSave(XML_Helper* helper) const
{
  return helper->save("opponent_id", d_opponent_id);
}

//-----------------------------------------------------------------------------
//History_DiplomacyWar

History_DiplomacyWar::History_DiplomacyWar(Player *p)
:History(History::DIPLOMATIC_WAR), d_opponent_id(p->getId())
{
}

History_DiplomacyWar::History_DiplomacyWar(const History_DiplomacyWar &history)
:History(history), d_opponent_id(history.d_opponent_id)
{
}

History_DiplomacyWar::History_DiplomacyWar(XML_Helper* helper)
:History(helper)
{
  helper->get(d_opponent_id, "opponent_id");
}

Glib::ustring History_DiplomacyWar::dump() const
{
  return String::ucompose("War has been declared with player %1.\n", d_opponent_id);
}

bool History_DiplomacyWar::doSave(XML_Helper* helper) const
{
  return helper->save("opponent_id", d_opponent_id);
}

//-----------------------------------------------------------------------------
//History_DiplomacyTreachery

History_DiplomacyTreachery::History_DiplomacyTreachery(Player *p)
:History(History::DIPLOMATIC_TREACHERY), d_opponent_id(p->getId())
{
}

History_DiplomacyTreachery::History_DiplomacyTreachery(const History_DiplomacyTreachery &history)
:History(history), d_opponent_id(history.d_opponent_id)
{
}

History_DiplomacyTreachery::History_DiplomacyTreachery(XML_Helper* helper)
:History(helper)
{
  helper->get(d_opponent_id, "opponent_id");
}

Glib::ustring History_DiplomacyTreachery::dump() const
{
  return String::ucompose("Treachery on player %1.\n", d_opponent_id);
}

bool History_DiplomacyTreachery::doSave(XML_Helper* helper) const
{
  return helper->save("opponent_id", d_opponent_id);
}

//-----------------------------------------------------------------------------
//History_HeroFindsAllies

History_HeroFindsAllies::History_HeroFindsAllies(Hero *h)
:History(History::HERO_FINDS_ALLIES), d_hero(h->getName())
{
}

History_HeroFindsAllies::History_HeroFindsAllies(const History_HeroFindsAllies &history)
:History(history), d_hero(history.d_hero)
{
}

History_HeroFindsAllies::History_HeroFindsAllies(XML_Helper* helper)
:History(helper)
{
  helper->get(d_hero, "hero");
}

Glib::ustring History_HeroFindsAllies::dump() const
{
  return String::ucompose("Hero %1 finds some allies.\n", d_hero);
}

bool History_HeroFindsAllies::doSave(XML_Helper* helper) const
{
  return helper->save("hero", d_hero);
}

//-----------------------------------------------------------------------------
//History_EndTurn

History_EndTurn::History_EndTurn()
:History(History::END_TURN)
{
}

History_EndTurn::History_EndTurn(const History_EndTurn &history)
:History(history)
{
}

History_EndTurn::History_EndTurn(XML_Helper* helper)
:History(helper)
{
}

Glib::ustring History_EndTurn::dump() const
{
  return "Player ends a turn.\n";
}

bool History_EndTurn::doSave(XML_Helper* helper) const
{
  if (helper)
    return true;
  return false;
}

//-----------------------------------------------------------------------------
//History_HeroRuinExplored

History_HeroRuinExplored::History_HeroRuinExplored(Hero *h, Ruin *r)
:History(History::HERO_RUIN_EXPLORED), d_hero(h->getName()), d_ruin(r->getId())
{
}

History_HeroRuinExplored::History_HeroRuinExplored(const History_HeroRuinExplored &history)
:History(history), d_hero(history.d_hero), d_ruin(history.d_ruin)
{
}

History_HeroRuinExplored::History_HeroRuinExplored(XML_Helper* helper)
:History(helper)
{
  helper->get(d_ruin, "ruin");
  helper->get(d_hero, "hero");
}

Glib::ustring History_HeroRuinExplored::dump() const
{
  return String::ucompose("Ruin %1 has been searched by %2.\n", d_ruin, d_hero);
}

bool History_HeroRuinExplored::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->save("ruin", d_ruin);
  retval &= helper->save("hero", d_hero);

  return retval;
}

//-----------------------------------------------------------------------------
//History_HeroRewardRuin

History_HeroRewardRuin::History_HeroRewardRuin(Hero *h, Ruin *r)
:History(History::HERO_REWARD_RUIN), d_hero(h->getName()), d_ruin(r->getId())
{
}

History_HeroRewardRuin::History_HeroRewardRuin(const History_HeroRewardRuin &history)
:History(history), d_hero(history.d_hero), d_ruin(history.d_ruin)
{
}

History_HeroRewardRuin::History_HeroRewardRuin(XML_Helper* helper)
:History(helper)
{
  helper->get(d_ruin, "ruin");
  helper->get(d_hero, "hero");
}

Glib::ustring History_HeroRewardRuin::dump() const
{
  return String::ucompose("The location of ruin %1 has been given to %2.\n",
                          d_ruin, d_hero);
}

bool History_HeroRewardRuin::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->save("ruin", d_ruin);
  retval &= helper->save("hero", d_hero);

  return retval;
}

//-----------------------------------------------------------------------------
//History_HeroUseItem
History_HeroUseItem::History_HeroUseItem(Hero *h, Item *i, 
                                         Player *opponent, City *friendly_city,
                                         City *enemy_city, City *neutral_city,
                                         City *c)
:History(History::USE_ITEM), d_hero_name(h->getName()), 
    d_item_name(i->getName()), d_item_bonus(i->getBonus()), 
    d_opponent_id(0), d_friendly_city_id(0), d_enemy_city_id(0), 
    d_neutral_city_id(0), d_city_id(0)
{
  if (opponent)
    d_opponent_id = opponent->getId();
  if (friendly_city)
    d_friendly_city_id = friendly_city->getId();
  if (enemy_city)
    d_enemy_city_id = enemy_city->getId();
  if (neutral_city)
    d_neutral_city_id = neutral_city->getId();
  if (c)
    d_city_id = c->getId();
}

History_HeroUseItem::History_HeroUseItem(const History_HeroUseItem &h)
:History(h), d_hero_name(h.d_hero_name), d_item_name(h.d_item_name), 
    d_item_bonus(h.d_item_bonus), d_opponent_id(h.d_opponent_id),
    d_friendly_city_id(h.d_friendly_city_id), 
    d_enemy_city_id(h.d_enemy_city_id), d_neutral_city_id(h.d_neutral_city_id),
    d_city_id(h.d_city_id)
{
}

History_HeroUseItem::History_HeroUseItem(XML_Helper* helper)
:History(helper)
{
  helper->get(d_hero_name, "hero_name");
  helper->get(d_item_name, "item_name");
  helper->get(d_item_bonus, "item_bonus");
  helper->get(d_opponent_id, "opponent_id");
  helper->get(d_friendly_city_id, "friendly_city_id");
  helper->get(d_enemy_city_id, "enemy_city_id");
  helper->get(d_neutral_city_id, "neutral_city_id");
  helper->get(d_city_id, "city_id");
}

Glib::ustring History_HeroUseItem::dump() const
{
  return String::ucompose("Hero %1 uses item %2 on player %3, friendly city %4, enemy city %5, neutral city %6, city %7.\n", d_hero_name, d_item_name, d_opponent_id, d_friendly_city_id, d_enemy_city_id, d_neutral_city_id, d_city_id);
}

bool History_HeroUseItem::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->save("hero_name", d_hero_name);
  retval &= helper->save("item_name", d_item_name);
  retval &= helper->save("item_bonus", d_item_bonus);
  retval &= helper->save("opponent_id", d_opponent_id);
  retval &= helper->save("friendly_city_id", d_friendly_city_id);
  retval &= helper->save("enemy_city_id", d_enemy_city_id);
  retval &= helper->save("neutral_city_id", d_neutral_city_id);
  retval &= helper->save("city_id", d_city_id);

  return retval;
}

//-----------------------------------------------------------------------------
//History_HeroQuestExpired

History_HeroQuestExpired::History_HeroQuestExpired(Hero *h)
:History(History::HERO_QUEST_COMPLETED), d_hero(h->getName())
{
}

History_HeroQuestExpired::History_HeroQuestExpired(const History_HeroQuestExpired &history)
:History(history), d_hero(history.d_hero)
{
}

History_HeroQuestExpired::History_HeroQuestExpired(XML_Helper* helper)
:History(helper)
{
  helper->get(d_hero, "hero");
}

Glib::ustring History_HeroQuestExpired::dump() const
{
  return String::ucompose("Hero %1 has had a quest expire.\n", d_hero);
}

bool History_HeroQuestExpired::doSave(XML_Helper* helper) const
{
  return helper->save("hero", d_hero);
}

Glib::ustring History::historyTypeToString(const History::Type type)
{
  switch (type)
    {
    case History::START_TURN: return "History::START_TURN";
    case History::FOUND_SAGE: return "History::FOUND_SAGE";
    case History::GOLD_TOTAL: return "History::GOLD_TOTAL";
    case History::HERO_EMERGES: return "History::HERO_EMERGES";
    case History::CITY_WON: return "History::CITY_WON";
    case History::HERO_CITY_WON: return "History::HERO_CITY_WON";
    case History::CITY_RAZED: return "History::CITY_RAZED";
    case History::HERO_QUEST_STARTED: return "History::HERO_QUEST_STARTED";
    case History::HERO_QUEST_COMPLETED: return "History::HERO_QUEST_COMPLETED";
    case History::HERO_KILLED_IN_CITY: return "History::HERO_KILLED_IN_CITY";
    case History::HERO_KILLED_IN_BATTLE: return "History::HERO_KILLED_IN_BATTLE";
    case History::HERO_KILLED_SEARCHING: return "History::HERO_KILLED_SEARCHING";
    case History::SCORE: return "History::SCORE";
    case History::PLAYER_VANQUISHED: return "History::PLAYER_VANQUISHED";
    case History::DIPLOMATIC_PEACE: return "History::DIPLOMATIC_PEACE";
    case History::DIPLOMATIC_WAR: return "History::DIPLOMATIC_WAR";
    case History::DIPLOMATIC_TREACHERY: return "History::DIPLOMATIC_TREACHERY";
    case History::HERO_FINDS_ALLIES: return "History::HERO_FINDS_ALLIES";
    case History::END_TURN: return "History::END_TURN";
    case History::HERO_RUIN_EXPLORED: return "History::HERO_RUIN_EXPLORED";
    case History::HERO_REWARD_RUIN: return "History::HERO_REWARD_RUIN";
    case History::USE_ITEM: return "History::USE_ITEM";
    case History::HERO_QUEST_EXPIRED: return "History::HERO_QUEST_EXPIRED";
    }
  return "History::START_TURN";
}

History::Type History::historyTypeFromString(const Glib::ustring str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return History::Type(atoi(str.c_str()));
  if (str == "History::START_TURN") return History::START_TURN;
  else if (str == "History::FOUND_SAGE") return History::FOUND_SAGE;
  else if (str == "History::GOLD_TOTAL") return History::GOLD_TOTAL;
  else if (str == "History::HERO_EMERGES") return History::HERO_EMERGES;
  else if (str == "History::CITY_WON") return History::CITY_WON;
  else if (str== "History::HERO_CITY_WON") return History::HERO_CITY_WON;
  else if (str == "History::CITY_RAZED") return History::CITY_RAZED;
  else if (str == "History::HERO_QUEST_STARTED") return History::HERO_QUEST_STARTED;
  else if (str == "History::HERO_QUEST_COMPLETED") return History::HERO_QUEST_COMPLETED;
  else if (str == "History::HERO_KILLED_IN_CITY") return History::HERO_KILLED_IN_CITY;
  else if (str == "History::HERO_KILLED_IN_BATTLE") return History::HERO_KILLED_IN_BATTLE;
  else if (str == "History::HERO_KILLED_SEARCHING") return History::HERO_KILLED_SEARCHING;
  else if (str == "History::SCORE") return History::SCORE;
  else if (str == "History::PLAYER_VANQUISHED") return History::PLAYER_VANQUISHED;
  else if (str == "History::DIPLOMATIC_PEACE") return History::DIPLOMATIC_PEACE;
  else if (str == "History::DIPLOMATIC_WAR") return History::DIPLOMATIC_WAR;
  else if (str == "History::DIPLOMATIC_TREACHERY") return History::DIPLOMATIC_TREACHERY;
  else if (str == "History::HERO_FINDS_ALLIES") return History::HERO_FINDS_ALLIES;
  else if (str == "History::END_TURN") return History::END_TURN;
  else if (str == "History::HERO_RUIN_EXPLORED") return History::HERO_RUIN_EXPLORED;
  else if (str == "History::HERO_REWARD_RUIN") return History::HERO_REWARD_RUIN;
  else if (str == "History::USE_ITEM") return History::USE_ITEM;
  else if (str == "History::HERO_QUEST_EXPIRED") return History::HERO_QUEST_EXPIRED;
  return History::START_TURN;
}
