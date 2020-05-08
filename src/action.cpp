// Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2007, 2008, 2010, 2011, 2014, 2015, 2017, 2020 Ben Asselstine
// Copyright (C) 2008 Ole Laursen
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

#include "action.h"
#include "stack.h"
#include "army.h"
#include "city.h"
#include "signpost.h"
#include "ruin.h"
#include "temple.h"
#include "Quest.h"
#include "QKillHero.h"
#include "QEnemyArmies.h"
#include "QEnemyArmytype.h"
#include "QCitySack.h"
#include "QCityRaze.h"
#include "QCityOccupy.h"
#include "QPillageGold.h"
#include "armysetlist.h"
#include "playerlist.h"
#include "player.h"
#include "armyprodbase.h"
#include "heroproto.h"
#include "Item.h"
#include "stacklist.h" //remove me
#include "ucompose.hpp"
#include "SightMap.h"
#include "reward.h"
#include "xmlhelper.h"

Glib::ustring Action::d_tag = "action";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<< x << std::endl<<std::flush;}
#define debug(x)

Action::Action(Type type)
    :d_type(type)
{
}

Action::Action(const Action &action)
:d_type(action.d_type)
{

}
Action::Action(XML_Helper *helper)
{
  Glib::ustring type_str;
  helper->getData(type_str, "type");
  d_type = actionTypeFromString(type_str);
}

bool Action::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(Action::d_tag);
    retval &= saveContents(helper);
    retval &= helper->closeTag();

    return retval;
}

bool Action::saveContents(XML_Helper* helper) const
{
    bool retval = true;

    Glib::ustring type_str = actionTypeToString(Action::Type(d_type));
    retval &= helper->saveData("type", type_str);
    retval &= doSave(helper);

    return retval;
}

Action* Action::handle_load(XML_Helper* helper)
{
  Glib::ustring type_str;

  helper->getData(type_str, "type");
  Action::Type t = actionTypeFromString(type_str);

  switch (t)
    {
      case STACK_MOVE:
          return (new Action_Move(helper));
      case STACK_SPLIT:
          return (new Action_Split(helper));
      case STACK_FIGHT:
          return (new Action_Fight(helper));
      case STACK_JOIN:
          return(new Action_Join(helper));
      case RUIN_SEARCH:
          return (new Action_Ruin(helper));
      case TEMPLE_SEARCH:
          return (new Action_Temple(helper));
      case CITY_OCCUPY:
          return (new Action_Occupy(helper));
      case CITY_PILLAGE:
          return (new Action_Pillage(helper));
      case CITY_SACK:
          return (new Action_Sack(helper));
      case CITY_RAZE:
          return (new Action_Raze(helper));
      case CITY_BUY:
          return (new Action_Buy(helper));
      case CITY_PROD:
          return (new Action_Production(helper));
      case REWARD: 
          return (new Action_Reward(helper));
      case QUEST:
          return (new Action_Quest(helper));
      case HERO_EQUIP:
          return (new Action_Equip(helper));
      case UNIT_ADVANCE:
          return (new Action_Level(helper));
      case STACK_DISBAND:
          return (new Action_Disband(helper));
      case MODIFY_SIGNPOST:
          return (new Action_ModifySignpost(helper));
      case CITY_RENAME:
          return (new Action_RenameCity(helper));
      case CITY_VECTOR:
          return (new Action_Vector(helper));
      case FIGHT_ORDER:
          return (new Action_FightOrder(helper));
      case RESIGN:
          return (new Action_Resign(helper));
      case ITEM_PLANT:
          return (new Action_Plant(helper));
      case PRODUCE_UNIT:
          return (new Action_Produce(helper));
      case PRODUCE_VECTORED_UNIT:
          return (new Action_ProduceVectored(helper));
      case DIPLOMATIC_STATE:
          return (new Action_DiplomacyState(helper));
      case DIPLOMATIC_PROPOSAL:
          return (new Action_DiplomacyProposal(helper));
      case DIPLOMATIC_SCORE:
          return (new Action_DiplomacyScore(helper));
      case END_TURN:
          return (new Action_EndTurn(helper));
      case CITY_CONQUER:
          return (new Action_ConquerCity(helper));
      case RECRUIT_HERO:
          return (new Action_RecruitHero(helper));
      case CITY_DESTITUTE:
          return (new Action_CityTooPoorToProduce(helper));
      case INIT_TURN:
          return (new Action_InitTurn(helper));
      case CITY_LOOT:
          return (new Action_Loot(helper));
      case USE_ITEM:
          return (new Action_UseItem(helper));
      case STACK_ORDER:
          return (new Action_ReorderArmies(helper));
      case STACKS_RESET:
          return (new Action_ResetStacks(helper));
      case RUINS_RESET:
          return (new Action_ResetRuins(helper));
      case COLLECT_TAXES_AND_PAY_UPKEEP:
          return (new Action_CollectTaxesAndPayUpkeep(helper));
      case KILL_PLAYER:
          return (new Action_Kill(helper));
      case STACK_DEFEND:
          return (new Action_DefendStack(helper));
      case STACK_UNDEFEND:
          return (new Action_UndefendStack(helper));
      case STACK_PARK:
          return (new Action_ParkStack(helper));
      case STACK_UNPARK:
          return (new Action_UnparkStack(helper));
      case STACK_SELECT:
          return (new Action_SelectStack(helper));
      case STACK_DESELECT:
          return (new Action_DeselectStack(helper));
    }

  return 0;
}


Action* Action::copy(const Action* a)
{
    switch(a->getType())
    {
        case STACK_MOVE:
            return (new Action_Move(*dynamic_cast<const Action_Move*>(a)));
        case STACK_SPLIT:
            return (new Action_Split(*dynamic_cast<const Action_Split*>(a)));
        case STACK_FIGHT:
            return (new Action_Fight(*dynamic_cast<const Action_Fight*>(a)));
        case STACK_JOIN:
            return(new Action_Join(*dynamic_cast<const Action_Join*>(a)));
        case RUIN_SEARCH:
            return (new Action_Ruin(*dynamic_cast<const Action_Ruin*>(a)));
        case TEMPLE_SEARCH:
            return (new Action_Temple(*dynamic_cast<const Action_Temple*>(a)));
        case CITY_OCCUPY:
            return (new Action_Occupy(*dynamic_cast<const Action_Occupy*>(a)));
        case CITY_PILLAGE:
            return (new Action_Pillage(*dynamic_cast<const Action_Pillage*>(a)));
        case CITY_SACK:
            return (new Action_Sack(*dynamic_cast<const Action_Sack*>(a)));
        case CITY_RAZE:
            return (new Action_Raze(*dynamic_cast<const Action_Raze*>(a)));
        case CITY_BUY:
            return (new Action_Buy(*dynamic_cast<const Action_Buy*>(a)));
        case CITY_PROD:
            return (new Action_Production(*dynamic_cast<const Action_Production*>(a)));
        case REWARD: 
            return (new Action_Reward(*dynamic_cast<const Action_Reward*>(a)));
        case QUEST:
            return (new Action_Quest(*dynamic_cast<const Action_Quest*>(a)));
        case HERO_EQUIP:
            return (new Action_Equip(*dynamic_cast<const Action_Equip*>(a)));
        case UNIT_ADVANCE:
            return (new Action_Level(*dynamic_cast<const Action_Level*>(a)));
        case STACK_DISBAND:
            return (new Action_Disband(*dynamic_cast<const Action_Disband*>(a)));
        case MODIFY_SIGNPOST:
            return (new Action_ModifySignpost(*dynamic_cast<const Action_ModifySignpost*>(a)));
        case CITY_RENAME:
            return (new Action_RenameCity(*dynamic_cast<const Action_RenameCity*>(a)));
        case CITY_VECTOR:
            return (new Action_Vector(*dynamic_cast<const Action_Vector*>(a)));
        case FIGHT_ORDER:
            return (new Action_FightOrder(*dynamic_cast<const Action_FightOrder*>(a)));
        case RESIGN:
            return (new Action_Resign(*dynamic_cast<const Action_Resign*>(a)));
        case ITEM_PLANT:
            return (new Action_Plant(*dynamic_cast<const Action_Plant*>(a)));
        case PRODUCE_UNIT:
            return (new Action_Produce(*dynamic_cast<const Action_Produce*>(a)));
        case PRODUCE_VECTORED_UNIT:
            return 
              (new Action_ProduceVectored
                (*dynamic_cast<const Action_ProduceVectored*>(a)));
        case DIPLOMATIC_STATE:
            return 
              (new Action_DiplomacyState
                (*dynamic_cast<const Action_DiplomacyState*>(a)));
        case DIPLOMATIC_PROPOSAL:
            return 
              (new Action_DiplomacyProposal
                (*dynamic_cast<const Action_DiplomacyProposal*>(a)));
        case DIPLOMATIC_SCORE:
            return 
              (new Action_DiplomacyScore
                (*dynamic_cast<const Action_DiplomacyScore*>(a)));
        case END_TURN:
            return 
              (new Action_EndTurn
                (*dynamic_cast<const Action_EndTurn*>(a)));
        case CITY_CONQUER:
            return 
              (new Action_ConquerCity
                (*dynamic_cast<const Action_ConquerCity*>(a)));
        case RECRUIT_HERO:
            return 
              (new Action_RecruitHero
                (*dynamic_cast<const Action_RecruitHero*>(a)));
        case CITY_DESTITUTE:
            return 
              (new Action_CityTooPoorToProduce
                (*dynamic_cast<const Action_CityTooPoorToProduce*>(a)));
        case INIT_TURN:
            return 
              (new Action_InitTurn
                (*dynamic_cast<const Action_InitTurn*>(a)));
        case CITY_LOOT:
            return 
              (new Action_Loot
                (*dynamic_cast<const Action_Loot*>(a)));
        case USE_ITEM:
            return 
              (new Action_UseItem
                (*dynamic_cast<const Action_UseItem*>(a)));
        case STACK_ORDER:
            return 
              (new Action_ReorderArmies
                (*dynamic_cast<const Action_ReorderArmies*>(a)));
        case STACKS_RESET:
            return 
              (new Action_ResetStacks
                (*dynamic_cast<const Action_ResetStacks*>(a)));
        case RUINS_RESET:
            return 
              (new Action_ResetRuins
                (*dynamic_cast<const Action_ResetRuins*>(a)));
        case COLLECT_TAXES_AND_PAY_UPKEEP:
            return 
              (new Action_CollectTaxesAndPayUpkeep
                (*dynamic_cast<const Action_CollectTaxesAndPayUpkeep*>(a)));
        case KILL_PLAYER:
            return (new Action_Kill (*dynamic_cast<const Action_Kill*>(a)));
        case STACK_DEFEND:
            return (new Action_DefendStack
                    (*dynamic_cast<const Action_DefendStack*>(a)));
        case STACK_UNDEFEND:
            return (new Action_UndefendStack
                    (*dynamic_cast<const Action_UndefendStack*>(a)));
        case STACK_PARK:
            return (new Action_ParkStack
                    (*dynamic_cast<const Action_ParkStack*>(a)));
        case STACK_UNPARK:
            return (new Action_UnparkStack
                    (*dynamic_cast<const Action_UnparkStack*>(a)));
        case STACK_SELECT:
            return (new Action_SelectStack
                    (*dynamic_cast<const Action_SelectStack*>(a)));
        case STACK_DESELECT:
            return (new Action_DeselectStack
                    (*dynamic_cast<const Action_DeselectStack*>(a)));
    }

    return 0;
}

//-----------------------------------------------------------------------------
//Action_Move_Step

Action_Move::Action_Move(Stack* s, Vector<int> dest)
    :Action(Action::STACK_MOVE), d_stack(s->getId()), d_dest(dest),
    d_delta(dest - s->getPos()), d_moves_left(s->getMoves()),
    d_has_ship(s->hasShip()), d_had_ship (s->hasShip())
{
}

Action_Move::Action_Move (const Action_Move &a)
:Action(a), d_stack(a.d_stack), d_dest(a.d_dest), d_delta(a.d_delta),
    d_moves_left(a.d_moves_left), d_has_ship (a.d_has_ship),
    d_had_ship (a.d_had_ship)
{
}

Action_Move::Action_Move(XML_Helper* helper)
    :Action(helper)
{
    helper->getData(d_stack, "stack");
    helper->getData(d_dest.x, "x");
    helper->getData(d_dest.y, "y");
    helper->getData(d_delta.x, "delta_x");
    helper->getData(d_delta.y, "delta_y");
    helper->getData(d_moves_left, "moves_left");
    helper->getData(d_has_ship, "has_ship");
    helper->getData(d_had_ship, "had_ship");
}

bool Action_Move::doSave(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("stack", d_stack);
    retval &= helper->saveData("x", d_dest.x);
    retval &= helper->saveData("y", d_dest.y);
    retval &= helper->saveData("delta_x", d_delta.x);
    retval &= helper->saveData("delta_y", d_delta.y);
    retval &= helper->saveData("moves_left", d_moves_left);
    retval &= helper->saveData("has_ship", d_has_ship);
    retval &= helper->saveData("had_ship", d_had_ship);

    return retval;
}

//-----------------------------------------------------------------------------
//Action_Split

Action_Split::Action_Split(Stack* orig, Stack* added)
    :Action(Action::STACK_SPLIT), d_orig(orig->getId()), d_added(added->getId())
{
  if (orig->validate() == false || added->validate() == false)
    {
      std::cerr << String::ucompose("Action_Split: stacks don't validate!  orig has %1 units, and added has %2 units.", orig->size(), added->size()) << std::endl;
      return;
    }

  for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
    d_armies_moved[i] = 0;

  unsigned int j = 0;
  for (Stack::iterator it = added->begin(); it != added->end(); ++it, j++)
    d_armies_moved[j] = (*it)->getId();
}

Action_Split::Action_Split(const Action_Split &action)
: Action(action), d_orig(action.d_orig), d_added(action.d_added)
{
  for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
    d_armies_moved[i] = action.d_armies_moved[i];
}

Action_Split::Action_Split(XML_Helper* helper)
    :Action(helper)
{
    helper->getData(d_orig, "orig_stack");
    helper->getData(d_added, "new_stack");

    Glib::ustring s;
    std::istringstream si;

    helper->getData(s, "moved");
    si.str(s);
    for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
        si >>d_armies_moved[i];
}

bool Action_Split::doSave(XML_Helper* helper) const
{
    Glib::ustring s;
    bool retval = true;

    for (unsigned int i = 0; i < MAX_STACK_SIZE - 1; i++)
      s += String::ucompose("%1 ", d_armies_moved[i]);
    s += String::ucompose("%1 ", d_armies_moved[MAX_STACK_SIZE - 1]);

    retval &= helper->saveData("orig_stack", d_orig);
    retval &= helper->saveData("new_stack", d_added);
    retval &= helper->saveData("moved", s);

    return retval;
}

//-----------------------------------------------------------------------------
//Action_Fight

Action_Fight::Action_Fight(const Fight* f)
    :Action(Action::STACK_FIGHT)
{
  std::list<Stack*> list = f->getAttackers();

  for (std::list<Stack*>::const_iterator it = list.begin();
       it != list.end(); ++it)
    d_attackers.push_back((*it)->getId());

  list = f->getDefenders();

  for (std::list<Stack*>::const_iterator it = list.begin(); it != list.end(); ++it)
    d_defenders.push_back((*it)->getId());

  for (auto fighter : f->getAttackerFighters())
    d_attacker_army_ids.push_back(fighter->army->getId());
  for (auto fighter : f->getDefenderFighters())
    d_defender_army_ids.push_back(fighter->army->getId());
  d_history = f->getCourseOfEvents();
}

Action_Fight::Action_Fight(const Action_Fight &a)
: Action(a), d_history(a.d_history), d_attackers(a.d_attackers), 
    d_defenders(a.d_defenders), d_attacker_army_ids(a.d_attacker_army_ids),
    d_defender_army_ids(a.d_defender_army_ids)
{
}

Action_Fight::Action_Fight(XML_Helper* helper)
    :Action(helper)
{
    Glib::ustring s;
    std::istringstream si;
    int ival = -1;

    helper->registerTag(Item::d_tag, 
                        sigc::hide<0>(sigc::mem_fun(this, &Action_Fight::loadItem)));

    // get attacking and defending stacks
    helper->getData(s, "attackers");
    si.str(s);
    while (si.eof() == false)
      {
        ival = -1;
        si >> ival;
        if (ival != -1)
          d_attackers.push_back((guint32)ival);
      }
    si.clear();

    helper->getData(s, "defenders");
    si.str(s);
    while (si.eof() == false)
      {
        ival = -1;
        si >> ival;
        if (ival != -1)
          d_defenders.push_back((guint32) ival);
      }
    // get attacking and defending army ids
    si.clear();
    helper->getData(s, "attacker_army_ids");
    si.str(s);
    while (si.eof() == false)
      {
        ival = -1;
        si >> ival;
        if (ival != -1)
          d_attacker_army_ids.push_back((guint32)ival);
      }
    si.clear();
    helper->getData(s, "defender_army_ids");
    si.str(s);
    while (si.eof() == false)
      {
        ival = -1;
        si >> ival;
        if (ival != -1)
          d_defender_army_ids.push_back((guint32)ival);
      }
    si.clear();
}

Glib::ustring Action_Fight::dump() const
{
  Glib::ustring s = "Battle fought.\n Attacking stacks: ";
  for (std::list<guint32>::const_iterator uit = d_attackers.begin();
       uit != d_attackers.end(); ++uit)
    s += String::ucompose("%1 ", (*uit));
  s += "\n Defending stacks: ";
  for (std::list<guint32>::const_iterator uit = d_defenders.begin();
       uit != d_defenders.end(); ++uit)
    s += String::ucompose("%1 ", (*uit));
  s +="\n Attacking armies: ";
  for (auto i : d_attacker_army_ids)
    s += String::ucompose("%1 ", i);
  s +="\n Defending armies: ";
  for (auto i : d_defender_army_ids)
    s += String::ucompose("%1 ", i);
  s +="\n Armies hit: ";
  for (auto h : d_history)
    s += String::ucompose ("%1, ", h.id);
  s +="\n";
  return s;
}

bool Action_Fight::doSave(XML_Helper* helper) const
{
  Glib::ustring s;
    bool retval = true;
    
    // save the stack's ids
    for (std::list<guint32>::const_iterator uit = d_attackers.begin();
         uit != d_attackers.end(); ++uit)
      s += String::ucompose("%1 ", (*uit));
    retval &= helper->saveData("attackers", s);

    s = "";
    for (std::list<guint32>::const_iterator uit = d_defenders.begin();
         uit != d_defenders.end(); ++uit)
      s += String::ucompose("%1 ", (*uit));
    retval &= helper->saveData("defenders", s);

    s = "";
    for (auto i : d_attacker_army_ids)
      s += String::ucompose("%1 ", i);
    retval &= helper->saveData("attacker_army_ids", s);

    s = "";
    for (auto i : d_defender_army_ids)
      s += String::ucompose("%1 ", i);
    retval &= helper->saveData("defender_army_ids", s);

    // save what happened
    for (std::list<FightItem>::const_iterator fit = d_history.begin(); 
            fit != d_history.end(); ++fit)
    {
        retval &= helper->openTag(Item::d_tag);
        retval &= helper->saveData("turn", (*fit).turn);
        retval &= helper->saveData("id", (*fit).id);
        retval &= helper->saveData("damage", (*fit).damage);
        retval &= helper->closeTag();
    }

    return retval;
}

bool Action_Fight::stack_ids_to_stacks(std::list<guint32> stack_ids, std::list<Stack*> &stacks, guint32 &stack_id) const
{
  for (std::list<guint32>::iterator i = stack_ids.begin(); i != stack_ids.end(); ++i)
    {
      bool found = false;
      for (Playerlist::iterator j = Playerlist::getInstance()->begin(), jend = Playerlist::getInstance()->end();
           j != jend; ++j) 
        {
          Stack *s = (*j)->getStacklist()->getStackById(*i);
          if (s)
            {
              found = true;
              stacks.push_back(s);
              break;
            }
        }
      if (found == false)
        {
          stack_id = *i;
          return false;
        }
    }
  return true;
}

bool Action_Fight::is_army_id_in_stacks(guint32 id, const std::list<guint32> &stack_ids) const
{
  std::list<Stack*> stacks;
  guint32 stack_id = 0;
  bool success = stack_ids_to_stacks(stack_ids, stacks, stack_id);
  if (!success)
    return false;
  bool found = false;
  for (std::list<Stack*>::iterator i = stacks.begin(); i != stacks.end(); ++i)
    {
      if ((*i)->getArmyById(id))
        {
          found = true;
          break;
        }
    }
  if (found)
    return true;
  else
    return false;
}

bool Action_Fight::loadItem(XML_Helper* helper)
{
    FightItem item;
    
    helper->getData(item.turn, "turn");
    helper->getData(item.id, "id");
    helper->getData(item.damage, "damage");

    d_history.push_back(item);

    return true;
}

//-----------------------------------------------------------------------------
//Action_Join

Action_Join::Action_Join(Stack* o, Stack* j)
    :Action(Action::STACK_JOIN), d_orig_id(o->getId()), d_joining_id(j->getId())
{
  if ((o->empty()) || (j->empty()) || (o->size() + j->size() > MAX_STACK_SIZE))
    {
      std::cerr << String::ucompose("Action_Join: wrong stack size.  expected %1, but got %2.", o->size(), j->size()) << std::endl;
      return;
    }
}

Action_Join::Action_Join(const Action_Join &a)
: Action(a), d_orig_id(a.d_orig_id), d_joining_id(a.d_joining_id)
{
}

Action_Join::Action_Join(XML_Helper* helper)
    :Action(helper)
{
    helper->getData(d_orig_id, "receiver");
    helper->getData(d_joining_id, "joining");
}

bool Action_Join::doSave(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("receiver", d_orig_id);
    retval &= helper->saveData("joining", d_joining_id);

    return retval;
}

//-----------------------------------------------------------------------------
//Action_Ruin

Action_Ruin::Action_Ruin(Ruin *r, Stack *explorers)
    :Action(Action::RUIN_SEARCH), d_ruin(r->getId()), d_stack(0), 
    d_searched(r->isSearched())
{
  if (explorers)
    d_stack = explorers->getId();
}

Action_Ruin::Action_Ruin(const Action_Ruin&a)
: Action(a), d_ruin(a.d_ruin), d_stack(a.d_stack), d_searched(a.d_searched)
{
}

Action_Ruin::Action_Ruin(XML_Helper* helper)
    :Action(helper)
{
    helper->getData(d_ruin, "ruin");
    helper->getData(d_stack, "seeker");
    helper->getData(d_searched, "searched");
}

bool Action_Ruin::doSave(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("ruin", d_ruin);
    retval &= helper->saveData("seeker", d_stack);
    retval &= helper->saveData("searched", d_searched);

    return retval;
}

//-----------------------------------------------------------------------------
//Action_Temple

Action_Temple::Action_Temple(Temple* t, Stack* s)
    :Action(Action::TEMPLE_SEARCH), d_temple(t->getId()), d_stack(s->getId())
{
}

Action_Temple::Action_Temple(const Action_Temple &action)
: Action(action), d_temple(action.d_temple), d_stack(action.d_stack)
{
}

Action_Temple::Action_Temple(XML_Helper* helper)
    :Action(helper)
{
    helper->getData(d_temple, "temple");
    helper->getData(d_stack, "stack");
}

bool Action_Temple::doSave(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("temple", d_temple);
    retval &= helper->saveData("stack", d_stack);

    return retval;
}

//-----------------------------------------------------------------------------
//Action_Occupy

Action_Occupy::Action_Occupy(City *c)
    :Action(Action::CITY_OCCUPY), d_city(c->getId())
{
}

Action_Occupy::Action_Occupy(const Action_Occupy &action)
: Action(action), d_city(action.d_city)
{
}

Action_Occupy::Action_Occupy(XML_Helper* helper)
    :Action(helper)
{
    helper->getData(d_city, "city");
}

bool Action_Occupy::doSave(XML_Helper* helper) const
{
    return helper->saveData("city", d_city);
}

//-----------------------------------------------------------------------------
//Action_Pillage

Action_Pillage::Action_Pillage(City *c)
    :Action(Action::CITY_PILLAGE), d_city(c->getId())
{
}

Action_Pillage::Action_Pillage(const Action_Pillage &action)
: Action(action), d_city(action.d_city)
{
}

Action_Pillage::Action_Pillage(XML_Helper* helper)
    :Action(helper)
{
    helper->getData(d_city, "city");
}

bool Action_Pillage::doSave(XML_Helper* helper) const
{
    return helper->saveData("city", d_city);
}

//-----------------------------------------------------------------------------
//Action_Sack

Action_Sack::Action_Sack(City *c)
    :Action(Action::CITY_SACK), d_city(c->getId())
{
}

Action_Sack::Action_Sack(const Action_Sack &action)
: Action(action), d_city(action.d_city)
{
}

Action_Sack::Action_Sack(XML_Helper* helper)
    :Action(helper)
{
    helper->getData(d_city, "city");
}

bool Action_Sack::doSave(XML_Helper* helper) const
{
    return helper->saveData("city", d_city);
}

//-----------------------------------------------------------------------------
//Action_Raze

Action_Raze::Action_Raze(City *c)
    :Action(Action::CITY_RAZE), d_city(c->getId())
{
}

Action_Raze::Action_Raze(const Action_Raze &action)
: Action(action), d_city(action.d_city)
{
}

Action_Raze::Action_Raze(XML_Helper* helper)
    :Action(helper)
{
    helper->getData(d_city, "city");
}

bool Action_Raze::doSave(XML_Helper* helper) const
{
    return helper->saveData("city", d_city);
}

//-----------------------------------------------------------------------------
//Action_Buy

Action_Buy::Action_Buy(City* c, int s, const ArmyProto *p)
    :Action(Action::CITY_BUY), d_city(c->getId()), d_slot(s), d_prod(p->getId())
{
}

Action_Buy::Action_Buy(const Action_Buy &a)
: Action(a), d_city(a.d_city), d_slot(a.d_slot), d_prod(a.d_prod)
{
}

Action_Buy::Action_Buy(XML_Helper* helper)
    :Action(helper)
{
    helper->getData(d_city, "city");
    helper->getData(d_slot, "slot");
    helper->getData(d_prod, "production");
}

bool Action_Buy::doSave(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("city", d_city);
    retval &= helper->saveData("slot", d_slot);
    retval &= helper->saveData("production", d_prod);

    return retval;
}

//-----------------------------------------------------------------------------
//Action_Change_Production

Action_Production::Action_Production(City* c, int slot)
    :Action(Action::CITY_PROD), d_city(c->getId()), d_prod(slot)
{
}

Action_Production::Action_Production (const Action_Production &action)
: Action(action), d_city(action.d_city), d_prod(action.d_prod)
{
}

Action_Production::Action_Production(XML_Helper* helper)
    :Action(helper)
{
    helper->getData(d_city, "city");
    helper->getData(d_prod, "production");
}

bool Action_Production::doSave(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("city", d_city);
    retval &= helper->saveData("production", d_prod);

    return retval;
}

//-----------------------------------------------------------------------------
//Action_Reward

Action_Reward::Action_Reward(Stack *s, Reward* r)
    :Action(Action::REWARD), d_stack(s->getId())
{
  d_reward = Reward::copy (r);
}

Action_Reward::~Action_Reward()
{
  delete d_reward;
}

bool Action_Reward::load(Glib::ustring tag, XML_Helper *helper)
{
    if (tag == Reward::d_tag)
      {
	guint32 t;
	Glib::ustring type_str;
	helper->getData(type_str, "type");
	t = Reward::rewardTypeFromString(type_str);
	switch (t)
	  {
	  case  Reward::GOLD:
	    d_reward = new Reward_Gold(helper); break;
	  case  Reward::ALLIES:
	    d_reward = new Reward_Allies(helper); break;
	  case Reward::ITEM:
	    d_reward = new Reward_Item(helper); break;
	  case Reward::RUIN:
	    d_reward = new Reward_Ruin(helper); break;
	  case Reward::MAP:
	    d_reward = new Reward_Map(helper); break;
	  }
	return true;
      }
    return false;
}

Action_Reward::Action_Reward (const Action_Reward &action)
: Action(action), d_stack(action.d_stack)
{
  const Reward *reward = action.d_reward;
  if (reward)
    d_reward = Reward::copy(reward);
  else
    d_reward = NULL;
}

Action_Reward::Action_Reward(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_stack, "stack");
  d_reward = NULL;
  helper->registerTag(Reward::d_tag, sigc::mem_fun(this, &Action_Reward::load));
}

Glib::ustring Action_Reward::dump() const
{
  if (d_reward->getType() == Reward::GOLD)
    {
      Reward_Gold *r = dynamic_cast<Reward_Gold*>(d_reward);
      return String::ucompose("Stack %1 got a reward of type %2: %3 gp.\n", d_stack, Reward::rewardTypeToString(d_reward->getType()), r->getGold());
    }
  else if (d_reward->getType() == Reward::ALLIES)
    {
      Reward_Allies*r = dynamic_cast<Reward_Allies*>(d_reward);
      return String::ucompose("Stack %1 got a reward of type %2: %3 %4.\n", d_stack, Reward::rewardTypeToString(d_reward->getType()), r->getNoOfAllies(), r->getArmy()->getName());
    }
  else if (d_reward->getType() == Reward::ITEM)
    {
      Reward_Item *r = dynamic_cast<Reward_Item*>(d_reward);
      return String::ucompose("Stack %1 got a reward of type %2: %3.\n", d_stack, Reward::rewardTypeToString(d_reward->getType()), r->getItem()->getName());
    }
  else if (d_reward->getType() == Reward::RUIN)
    {
      Reward_Ruin *r = dynamic_cast<Reward_Ruin*>(d_reward);
      return String::ucompose("Stack %1 got a reward of type %2: %3,%4 %5.\n", d_stack, Reward::rewardTypeToString(d_reward->getType()), r->getRuin()->getPos().x, r->getRuin()->getPos().y, r->getRuin()->getName());
    }
  else if (d_reward->getType() == Reward::MAP)
    {
      Reward_Map*r = dynamic_cast<Reward_Map*>(d_reward);
      return String::ucompose("Stack %1 got a reward of type %2: %3,%4 %5.\n", d_stack, Reward::rewardTypeToString(d_reward->getType()), r->getLocation().x, r->getLocation().y, r->getMapName());
    }
  return String::ucompose("Stack %1 got a reward of type %2.\n", d_stack, Reward::rewardTypeToString(d_reward->getType()));
}

bool Action_Reward::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("stack", d_stack);
  if (d_reward->getType() == Reward::GOLD)
    retval &= dynamic_cast<Reward_Gold*>(d_reward)->save(helper);
  else if (d_reward->getType() == Reward::ALLIES)
    retval &= dynamic_cast<Reward_Allies*>(d_reward)->save(helper);
  else if (d_reward->getType() == Reward::ITEM)
    retval &= dynamic_cast<Reward_Item*>(d_reward)->save(helper);
  else if (d_reward->getType() == Reward::RUIN)
    retval &= dynamic_cast<Reward_Ruin*>(d_reward)->save(helper);
  else if (d_reward->getType() == Reward::MAP)
    retval &= dynamic_cast<Reward_Map*>(d_reward)->save(helper);

  return retval;
}

//-----------------------------------------------------------------------------
// Action_Quest

Action_Quest::Action_Quest(Quest* q)
:Action(Action::QUEST), d_hero(q->getHeroId()), d_questtype(q->getType()),
    d_data(0), d_victim_player(0)
{
  // fill the data depending on the quest's type
  switch (d_questtype)
    {
    case Quest::KILLHERO:
      d_data = dynamic_cast<QuestKillHero*>(q)->getVictim();
      break;
    case Quest::KILLARMIES:
      d_data = dynamic_cast<QuestEnemyArmies*>(q)->getArmiesToKill();
      d_victim_player = dynamic_cast<QuestEnemyArmies*>(q)->getVictimPlayerId();
      break;
    case Quest::CITYSACK:
      d_data = dynamic_cast<QuestCitySack*>(q)->getCityId();
      break;
    case Quest::CITYRAZE:
      d_data = dynamic_cast<QuestCityRaze*>(q)->getCityId();
      break;
    case Quest::CITYOCCUPY:
      d_data = dynamic_cast<QuestCityOccupy*>(q)->getCityId();
      break;
    case Quest::KILLARMYTYPE:
      d_data = dynamic_cast<QuestEnemyArmytype*>(q)->getArmytypeToKill();
      break;
    case Quest::PILLAGEGOLD:
      d_data = dynamic_cast<QuestPillageGold*>(q)->getGoldToPillage();
      break;
    }
}

Action_Quest::Action_Quest (const Action_Quest &a)
: Action(a), d_hero(a.d_hero), d_questtype(a.d_questtype), d_data(a.d_data), 
    d_victim_player(a.d_victim_player)
{
}

Action_Quest::Action_Quest(XML_Helper* helper)
:Action(helper)
{

  helper->getData(d_hero, "hero");
  Glib::ustring s;
  helper->getData(s, "quest");
  d_questtype = Quest::questTypeFromString(s);
  helper->getData(d_data, "data");
  helper->getData(d_victim_player, "victim_player");
}

bool Action_Quest::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("hero", d_hero);
  Glib::ustring s = Quest::questTypeToString(Quest::Type(d_questtype));
  retval &= helper->saveData("quest", s);
  retval &= helper->saveData("data", d_data);
  retval &= helper->saveData("victim_player", d_victim_player);

  return retval;
}

//-----------------------------------------------------------------------------
// Action_Equip

Action_Equip::Action_Equip(Hero *h, Item *i, Action_Equip::Slot slot, Vector<int> pos)
:Action(Action::HERO_EQUIP), d_hero(h->getId()), d_item(i->getId()), 
    d_slot(slot), d_pos(pos)
{
}

Action_Equip::Action_Equip (const Action_Equip &a)
: Action(a), d_hero(a.d_hero), d_item(a.d_item), d_slot(a.d_slot), 
    d_pos(a.d_pos)
{
}

Action_Equip::Action_Equip(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_hero, "hero");
  helper->getData(d_item, "item");
  helper->getData(d_slot, "dest");
  int i;
  helper->getData(i, "x");
  d_pos.x = i;
  helper->getData(i, "y");
  d_pos.y = i;
}

bool Action_Equip::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("hero", d_hero);
  retval &= helper->saveData("item", d_item);
  retval &= helper->saveData("dest", d_slot);
  int i = d_pos.x;
  retval &= helper->saveData("x", i);
  i = d_pos.y;
  retval &= helper->saveData("y", i);

  return retval;
}

//-----------------------------------------------------------------------------
// Action_Level

Action_Level::Action_Level(Army *unit, Army::Stat raised)
:Action(Action::UNIT_ADVANCE), d_army(unit->getId()), d_stat(raised)
{
}

Action_Level::Action_Level (const Action_Level &action)
: Action(action), d_army(action.d_army), d_stat(action.d_stat)
{
}

Action_Level::Action_Level(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_army, "army");
  helper->getData(d_stat, "stat");
}

bool Action_Level::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("army", d_army);
  retval &= helper->saveData("stat", d_stat);

  return retval;
}

//-----------------------------------------------------------------------------
//Action_Disband

Action_Disband::Action_Disband(Stack *s)
:Action(Action::STACK_DISBAND), d_stack(s->getId())
{
}

Action_Disband::Action_Disband(const Action_Disband &action)
: Action(action), d_stack(action.d_stack)
{
}

Action_Disband::Action_Disband(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_stack, "stack");
}

bool Action_Disband::doSave(XML_Helper* helper) const
{
  return helper->saveData("stack", d_stack);
}

//-----------------------------------------------------------------------------
//Action_ModifySignpost

Action_ModifySignpost::Action_ModifySignpost(Signpost * s, Glib::ustring message)
:Action(Action::MODIFY_SIGNPOST), d_signpost(s->getId()), d_message(message)
{
}

Action_ModifySignpost::Action_ModifySignpost(const Action_ModifySignpost &a)
: Action(a), d_signpost(a.d_signpost), d_message(a.d_message)
{
}

Action_ModifySignpost::Action_ModifySignpost(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_signpost, "signpost");
  helper->getData(d_message, "message");
}

bool Action_ModifySignpost::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("signpost", d_signpost);
  retval &= helper->saveData("message", d_message);

  return retval;
}

//-----------------------------------------------------------------------------
//Action_RenameCity

Action_RenameCity::Action_RenameCity(City* c, Glib::ustring name)
:Action(Action::CITY_RENAME), d_city(c->getId()), d_name(name)
{
}

Action_RenameCity::Action_RenameCity(const Action_RenameCity &action)
: Action(action), d_city(action.d_city), d_name(action.d_name)
{
}

Action_RenameCity::Action_RenameCity(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_city, "city");
  helper->getData(d_name, "name");
}

bool Action_RenameCity::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("city", d_city);
  retval &= helper->saveData("name", d_name);

  return retval;
}

//-----------------------------------------------------------------------------
//Action_Vector

Action_Vector::Action_Vector(City* src, Vector<int> dest)
:Action(Action::CITY_VECTOR), d_city(src->getId()), d_dest(dest)
{
}

Action_Vector::Action_Vector(const Action_Vector &action)
: Action(action), d_city(action.d_city), d_dest(action.d_dest)
{
}

Action_Vector::Action_Vector(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_city, "city");
  helper->getData(d_dest.x, "x");
  helper->getData(d_dest.y, "y");
}

bool Action_Vector::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("city", d_city);
  retval &= helper->saveData("x", d_dest.x);
  retval &= helper->saveData("y", d_dest.y);

  return retval;
}

//-----------------------------------------------------------------------------
//Action_FightOrder

Action_FightOrder::Action_FightOrder(const std::list<guint32> &order)
:Action(Action::FIGHT_ORDER), d_order(order)
{
}

Action_FightOrder::Action_FightOrder(const Action_FightOrder &action)
: Action(action), d_order(action.d_order)
{
}

Action_FightOrder::Action_FightOrder(XML_Helper* helper)
:Action(helper)
{
  Glib::ustring fight_order;
  std::stringstream sfight_order;
  guint32 val;
  helper->getData(fight_order, "order");
  sfight_order.str(fight_order);
  //XXX XXX XXX this business of looking up the first living seems wrong.
  Armyset *as = Armysetlist::getInstance()->get(Playerlist::getInstance()->getFirstLiving()->getArmyset());
  for (Armyset::iterator i = as->begin(); i != as->end(); ++i)
    {
      sfight_order >> val;
      d_order.push_back(val);
    }
}

bool Action_FightOrder::doSave(XML_Helper* helper) const
{
  bool retval = true;

  Glib::ustring s;
  for (std::list<guint32>::const_iterator it = d_order.begin();
       it != d_order.end(); ++it)
    s += String::ucompose("%1 ", (*it));
  retval &= helper->saveData("order", s);

  return retval;
}

//-----------------------------------------------------------------------------
//Action_Resign

//-----------------------------------------------------------------------------
//Action_Plant

Action_Plant::Action_Plant(Hero *hero, Item *item)
:Action(Action::ITEM_PLANT), d_hero(hero->getId()), d_item(item->getId())
{
}

Action_Plant::Action_Plant(const Action_Plant &action)
: Action(action), d_hero(action.d_hero), d_item(action.d_item)
{
}

Action_Plant::Action_Plant(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_hero, "hero");
  helper->getData(d_item, "item");
}

bool Action_Plant::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("hero", d_hero);
  retval &= helper->saveData("item", d_item);

  return retval;
}

//-----------------------------------------------------------------------------
//Action_Produce

Action_Produce::Action_Produce(const ArmyProdBase *army, City *city, 
                               bool vectored, Vector<int> pos, guint32 army_id,
                               guint32 stack_id)
:Action(Action::PRODUCE_UNIT), d_army(new ArmyProdBase(*army)), 
    d_city(city->getId()), d_vectored(vectored), d_dest(pos), 
    d_army_id(army_id), d_stack_id(stack_id)
{
}

Action_Produce::Action_Produce(const Action_Produce &a)
: Action(a), d_city(a.d_city), d_vectored(a.d_vectored), d_dest(a.d_dest), 
    d_army_id(a.d_army_id), d_stack_id(a.d_stack_id)
{
  if (a.d_army)
    d_army = new ArmyProdBase (*a.d_army);
  else
    d_army = NULL;
}

Action_Produce::Action_Produce(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_city, "city");
  helper->getData(d_vectored, "vectored");
  helper->getData(d_dest.x, "dest_x");
  helper->getData(d_dest.y, "dest_y");
  helper->getData(d_army_id, "army_id");
  helper->getData(d_stack_id, "stack_id");
  d_army = NULL;
  helper->registerTag(ArmyProdBase::d_tag, sigc::mem_fun(this, &Action_Produce::load));
}

bool Action_Produce::load(Glib::ustring tag, XML_Helper *helper)
{
    if (tag == ArmyProdBase::d_tag)
      {
	d_army = new ArmyProdBase(helper);

	return true;
      }
    return false;
}

Action_Produce::~Action_Produce()
{
  if (d_army)
    delete d_army;
}

bool Action_Produce::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("city", d_city);
  retval &= helper->saveData("vectored", d_vectored);
  retval &= helper->saveData("dest_x", d_dest.x);
  retval &= helper->saveData("dest_y", d_dest.y);
  retval &= helper->saveData("army_id", d_army_id);
  retval &= helper->saveData("stack_id", d_stack_id);
  retval &= d_army->save(helper);

  return retval;
}

//-----------------------------------------------------------------------------
//Action_ProduceVectored

Action_ProduceVectored::Action_ProduceVectored(ArmyProdBase *army, 
                                               Vector<int> dest, 
                                               Vector<int> src, guint32 aid,
                                               guint32 sid)
:Action(Action::PRODUCE_VECTORED_UNIT), d_army(NULL), d_dest(dest), d_src(src),
    d_target_army_id (aid), d_target_stack_id (sid)
{
  if (army)
    d_army = new ArmyProdBase(*army);
}

Action_ProduceVectored::Action_ProduceVectored(const Action_ProduceVectored &a)
: Action(a), d_dest(a.d_dest), d_src(a.d_src),
    d_target_army_id (a.d_target_army_id),
    d_target_stack_id (a.d_target_stack_id)
{
  if (a.d_army)
    d_army = new ArmyProdBase(*a.d_army);
  else
    d_army = NULL;
}

Action_ProduceVectored::Action_ProduceVectored(XML_Helper* helper)
:Action(helper), d_army(NULL)
{
  helper->getData(d_dest.x, "dest_x");
  helper->getData(d_dest.y, "dest_y");
  helper->getData(d_src.x, "src_x");
  helper->getData(d_src.y, "src_y");
  helper->getData(d_target_army_id, "target_army_id");
  helper->getData(d_target_stack_id, "target_stack_id");
  d_army = NULL;
  helper->registerTag(ArmyProdBase::d_tag, sigc::mem_fun(this, &Action_ProduceVectored::load));
}

bool Action_ProduceVectored::load(Glib::ustring tag, XML_Helper *helper)
{
    if (tag == ArmyProdBase::d_tag)
      {
	d_army = new ArmyProdBase(helper);

	return true;
      }
    return false;
}

Action_ProduceVectored::~Action_ProduceVectored()
{
  if (d_army)
    delete d_army;
}

bool Action_ProduceVectored::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("dest_x", d_dest.x);
  retval &= helper->saveData("dest_y", d_dest.y);
  retval &= helper->saveData("src_x", d_src.x);
  retval &= helper->saveData("src_y", d_src.y);
  retval &= helper->saveData("target_army_id", d_target_army_id);
  retval &= helper->saveData("target_stack_id", d_target_stack_id);
  retval &= d_army->save(helper);

  return retval;
}

//-----------------------------------------------------------------------------
//Action_DiplomacyState

Action_DiplomacyState::Action_DiplomacyState(Player *p, 
                                             Player::DiplomaticState state)
:Action(Action::DIPLOMATIC_STATE), d_opponent_id(p->getId()),
    d_diplomatic_state(state)
{
}

Action_DiplomacyState::Action_DiplomacyState(const Action_DiplomacyState &a)
: Action(a), d_opponent_id(a.d_opponent_id), 
    d_diplomatic_state(a.d_diplomatic_state)
{
}

Action_DiplomacyState::Action_DiplomacyState(XML_Helper* helper)
:Action(helper)
{
  guint32 diplomatic_state;
  helper->getData(d_opponent_id, "opponent_id");
  helper->getData(diplomatic_state, "state");
  d_diplomatic_state = Player::DiplomaticState(diplomatic_state);
}

bool Action_DiplomacyState::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("opponent_id", d_opponent_id);
  retval &= helper->saveData("state", (guint32)d_diplomatic_state);

  return retval;
}

//-----------------------------------------------------------------------------
//Action_DiplomacyProposal

Action_DiplomacyProposal::Action_DiplomacyProposal(Player *p, 
                                                   Player::DiplomaticProposal proposal)
:Action(Action::DIPLOMATIC_PROPOSAL), d_opponent_id(p->getId()), 
    d_diplomatic_proposal(proposal)
{

}

Action_DiplomacyProposal::Action_DiplomacyProposal(const Action_DiplomacyProposal &a)
: Action(a), d_opponent_id(a.d_opponent_id), 
    d_diplomatic_proposal(a.d_diplomatic_proposal)
{
}

Action_DiplomacyProposal::Action_DiplomacyProposal(XML_Helper* helper)
:Action(helper)
{
  guint32 diplomatic_proposal;
  helper->getData(d_opponent_id, "opponent_id");
  helper->getData(diplomatic_proposal, "proposal");
  d_diplomatic_proposal = Player::DiplomaticProposal(diplomatic_proposal);
}

bool Action_DiplomacyProposal::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("opponent_id", d_opponent_id);
  retval &= helper->saveData("proposal", (guint32)d_diplomatic_proposal);

  return retval;
}

//-----------------------------------------------------------------------------
//Action_DiplomacyScore

Action_DiplomacyScore::Action_DiplomacyScore(Player *p, int amount)
:Action(Action::DIPLOMATIC_SCORE), d_opponent_id(p->getId()), d_amount(amount)
{
}

Action_DiplomacyScore::Action_DiplomacyScore(const Action_DiplomacyScore &a)
: Action(a), d_opponent_id(a.d_opponent_id), d_amount(a.d_amount)
{
}

Action_DiplomacyScore::Action_DiplomacyScore(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_opponent_id, "opponent_id");
  helper->getData(d_amount, "amount");
}

bool Action_DiplomacyScore::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("opponent_id", d_opponent_id);
  retval &= helper->saveData("amount", d_amount);

  return retval;
}

//-----------------------------------------------------------------------------
//Action_EndTurn


//-----------------------------------------------------------------------------
//Action_ConquerCity

Action_ConquerCity::Action_ConquerCity(City* c)
  :Action(Action::CITY_CONQUER), d_city(c->getId())
{
}

Action_ConquerCity::Action_ConquerCity(const Action_ConquerCity &action)
: Action(action), d_city(action.d_city)
{
}

Action_ConquerCity::Action_ConquerCity(XML_Helper* helper)
  :Action(helper)
{
    helper->getData(d_city, "city");
}

bool Action_ConquerCity::doSave(XML_Helper* helper) const
{
    return helper->saveData("city", d_city);
}

//-----------------------------------------------------------------------------
//Action_RecruitHero

Action_RecruitHero::Action_RecruitHero(HeroProto* h, City *c, int cost, int alliesCount, const ArmyProto *ally)
  :Action(Action::RECRUIT_HERO), d_hero(new HeroProto(*h)), d_city(c->getId()), d_cost(cost),
    d_allies(alliesCount)
{
    if (d_allies > 0)
      d_ally_army_type = ally->getId();
    else
      d_ally_army_type = 0;
}

Action_RecruitHero::Action_RecruitHero(XML_Helper* helper)
  :Action(helper)
{
    helper->getData(d_city, "city");
    helper->getData(d_cost, "cost");
    helper->getData(d_allies, "allies");
    helper->getData(d_ally_army_type, "ally_army_type");
    helper->registerTag(HeroProto::d_heroproto_tag, sigc::mem_fun(this, &Action_RecruitHero::load));
}

Action_RecruitHero::Action_RecruitHero(const Action_RecruitHero &a)
: Action(a), d_city(a.d_city), d_cost(a.d_cost), d_allies(a.d_allies), 
    d_ally_army_type(a.d_ally_army_type)
{
  d_hero = new HeroProto(*a.d_hero);
}

Action_RecruitHero::~Action_RecruitHero()
{
  if (d_hero)
    delete d_hero;
  d_hero = NULL;
}

bool Action_RecruitHero::load(Glib::ustring tag, XML_Helper *helper)
{
    if (tag == HeroProto::d_heroproto_tag)
      {
	d_hero = new HeroProto(helper);

	return true;
      }
    return false;
}

bool Action_RecruitHero::doSave(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("city", d_city);
    retval &= helper->saveData("cost", d_cost);
    retval &= helper->saveData("allies", d_allies);
    retval &= helper->saveData("ally_army_type", d_ally_army_type);
    retval &= d_hero->save(helper);

    return retval;
}

//-----------------------------------------------------------------------------
//Action_CityTooPoorToProduce

Action_CityTooPoorToProduce::Action_CityTooPoorToProduce(City* c, const ArmyProdBase *army)
    :Action(Action::CITY_DESTITUTE), d_city(c->getId()), 
    d_army_type(army->getTypeId())
{
}

Action_CityTooPoorToProduce::Action_CityTooPoorToProduce(const Action_CityTooPoorToProduce &action)
: Action(action), d_city(action.d_city), d_army_type(action.d_army_type)
{
}

Action_CityTooPoorToProduce::Action_CityTooPoorToProduce(XML_Helper* helper)
    :Action(helper)
{
    helper->getData(d_city, "city");
    helper->getData(d_army_type, "army_type");
}

bool Action_CityTooPoorToProduce::doSave(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("city", d_city);
    retval &= helper->saveData("army_type", d_army_type);

    return retval;
}

//-----------------------------------------------------------------------------
//Action_InitTurn

Action_InitTurn::Action_InitTurn(guint32 order)
:Action(Action::INIT_TURN), d_order(order)
{
}

Action_InitTurn::Action_InitTurn(const Action_InitTurn &action)
: Action(action), d_order(action.d_order)
{
}

Action_InitTurn::Action_InitTurn(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_order, "order");
}

bool Action_InitTurn::doSave(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->saveData("order", d_order);
  return retval;
}

//-----------------------------------------------------------------------------
//Action_Loot

Action_Loot::Action_Loot(Player *looting_player, Player *looted_player, 
                         guint32 amount_to_add, guint32 amount_to_subtract)
    :Action(Action::CITY_LOOT), d_looting_player_id(looting_player->getId()), 
    d_looted_player_id(looted_player->getId()), d_gold_added(amount_to_add), 
    d_gold_removed(amount_to_subtract)
{
}

Action_Loot::Action_Loot(const Action_Loot &a)
: Action(a), d_looting_player_id(a.d_looting_player_id),
    d_looted_player_id(a.d_looted_player_id), d_gold_added(a.d_gold_added), 
    d_gold_removed(a.d_gold_removed)
{
}

Action_Loot::Action_Loot(XML_Helper* helper)
    :Action(helper)
{
    helper->getData(d_looting_player_id, "looting_player_id");
    helper->getData(d_looted_player_id, "looted_player_id");
    helper->getData(d_gold_added, "gold_added");
    helper->getData(d_gold_removed, "gold_removed");
}

bool Action_Loot::doSave(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("looting_player_id", d_looting_player_id);
    retval &= helper->saveData("looted_player_id", d_looted_player_id);
    retval &= helper->saveData("gold_added", d_gold_added);
    retval &= helper->saveData("gold_removed", d_gold_removed);

    return retval;
}

//-----------------------------------------------------------------------------
// Action_UseItem

Action_UseItem::Action_UseItem(Hero *hero, Item *item, Player *victim, City *friendly_city, City *enemy_city, City *neutral_city, City *city)
:Action(Action::USE_ITEM), d_hero(hero->getId()), d_item(item->getId()), 
    d_victim_player(0), d_friendly_city(0), d_enemy_city(0), d_neutral_city(0),
    d_city(0)
{
  if (victim)
    d_victim_player = victim->getId();
  if (friendly_city)
    d_friendly_city = friendly_city->getId();
  if (enemy_city)
    d_enemy_city = enemy_city->getId();
  if (neutral_city)
    d_neutral_city = neutral_city->getId();
  if (city)
    d_city = city->getId();
}

Action_UseItem::Action_UseItem(const Action_UseItem &a)
: Action(a), d_hero(a.d_hero), d_item(a.d_item),
    d_victim_player(a.d_victim_player), d_friendly_city(a.d_friendly_city), 
    d_enemy_city(a.d_enemy_city), d_neutral_city(a.d_neutral_city), 
    d_city(a.d_city)
{
}

Action_UseItem::Action_UseItem(XML_Helper* helper)
:Action(helper)
{

  helper->getData(d_hero, "hero");
  helper->getData(d_item, "item");
  helper->getData(d_victim_player, "victim_player");
  helper->getData(d_friendly_city, "friendly_city");
  helper->getData(d_enemy_city, "enemy_city");
  helper->getData(d_neutral_city, "neutral_city");
  helper->getData(d_city, "city");
}

bool Action_UseItem::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("hero", d_hero);
  retval &= helper->saveData("item", d_item);
  retval &= helper->saveData("victim_player", d_victim_player);
  retval &= helper->saveData("friendly_city", d_friendly_city);
  retval &= helper->saveData("enemy_city", d_enemy_city);
  retval &= helper->saveData("neutral_city", d_neutral_city);
  retval &= helper->saveData("city", d_city);

  return retval;
}

//-----------------------------------------------------------------------------
// Action_ReorderArmies

Action_ReorderArmies::Action_ReorderArmies(Stack *s)
:Action(Action::STACK_ORDER), d_stack_id(s->getId()), 
    d_player_id(s->getOwner()->getId())
{
  for (Stack::iterator i = s->begin(); i != s->end(); ++i)
    d_army_ids.push_back((*i)->getId());
}

Action_ReorderArmies::Action_ReorderArmies(const Action_ReorderArmies &a)
: Action(a), d_stack_id(a.d_stack_id), d_player_id(a.d_player_id), 
    d_army_ids(a.d_army_ids)
{
}

Action_ReorderArmies::Action_ReorderArmies(XML_Helper* helper)
:Action(helper)
{

  helper->getData(d_stack_id, "stack_id");
  helper->getData(d_player_id, "player_id");
  Glib::ustring armies;
  helper->getData(armies, "armies");
  std::stringstream sarmies;
  sarmies.str(armies);
  int ival = -1;
  while (sarmies.eof() == false)
    {
      ival = -1;
      sarmies >> ival;
      if (ival != -1)
        d_army_ids.push_back((guint32)ival);
    }
}

bool Action_ReorderArmies::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("stack_id", d_stack_id);
  retval &= helper->saveData("player_id", d_player_id);

  Glib::ustring s;
  for (auto it: d_army_ids)
    s += String::ucompose("%1 ", it);
  retval &= helper->saveData("armies", s);
  return retval;
}

//-----------------------------------------------------------------------------
// Action_ResetStacks

Action_ResetStacks::Action_ResetStacks(Player *p)
:Action(Action::STACKS_RESET), d_player_id(p->getId())
{
}

Action_ResetStacks::Action_ResetStacks(const Action_ResetStacks &action)
: Action(action), d_player_id(action.d_player_id)
{
}

Action_ResetStacks::Action_ResetStacks(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_player_id, "player_id");
}

bool Action_ResetStacks::doSave(XML_Helper* helper) const
{
  return helper->saveData("player_id", d_player_id);
}

//-----------------------------------------------------------------------------
// Action_ResetRuins

Action_ResetRuins::Action_ResetRuins()
:Action(Action::RUINS_RESET)
{
}

Action_ResetRuins::Action_ResetRuins(const Action_ResetRuins &action)
: Action(action)
{
}

Action_ResetRuins::Action_ResetRuins(XML_Helper* helper)
:Action(helper)
{
}

bool Action_ResetRuins::doSave(XML_Helper* helper) const
{
  if (helper)
    return true;
  return false;
}

//-----------------------------------------------------------------------------
// Action_CollectTaxesAndPayUpkeep

Action_CollectTaxesAndPayUpkeep::Action_CollectTaxesAndPayUpkeep(int fromGold, int toGold)
:Action(Action::COLLECT_TAXES_AND_PAY_UPKEEP), d_from_gold(fromGold),
    d_to_gold(toGold)
{
}

Action_CollectTaxesAndPayUpkeep::Action_CollectTaxesAndPayUpkeep(const Action_CollectTaxesAndPayUpkeep &action)
: Action(action), d_from_gold(action.d_from_gold), d_to_gold(action.d_to_gold)
{
}

Action_CollectTaxesAndPayUpkeep::Action_CollectTaxesAndPayUpkeep(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_from_gold, "from_gold");
  helper->getData(d_to_gold, "to_gold");
}

bool Action_CollectTaxesAndPayUpkeep::doSave(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->saveData("from_gold", d_from_gold);
  retval &= helper->saveData("to_gold", d_to_gold);
  return retval;
}

//-----------------------------------------------------------------------------
// Action_Kill

//-----------------------------------------------------------------------------
// Action_DefendStack

Action_DefendStack::Action_DefendStack(Stack *s)
:Action(Action::STACK_DEFEND), d_stack_id(s->getId())
{
}

Action_DefendStack::Action_DefendStack(const Action_DefendStack &action)
: Action(action), d_stack_id(action.d_stack_id)
{
}

Action_DefendStack::Action_DefendStack(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_stack_id, "stack_id");
}

bool Action_DefendStack::doSave(XML_Helper* helper) const
{
  return helper->saveData("stack_id", d_stack_id);
}

//-----------------------------------------------------------------------------
// Action_UndefendStack

Action_UndefendStack::Action_UndefendStack(Stack *s)
:Action(Action::STACK_UNDEFEND), d_stack_id(s->getId())
{
}

Action_UndefendStack::Action_UndefendStack(const Action_UndefendStack &action)
: Action(action), d_stack_id(action.d_stack_id)
{
}

Action_UndefendStack::Action_UndefendStack(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_stack_id, "stack_id");
}

bool Action_UndefendStack::doSave(XML_Helper* helper) const
{
  return helper->saveData("stack_id", d_stack_id);
}

//-----------------------------------------------------------------------------
// Action_ParkStack

Action_ParkStack::Action_ParkStack(Stack *s)
:Action(Action::STACK_PARK), d_stack_id(s->getId())
{
}

Action_ParkStack::Action_ParkStack(const Action_ParkStack &action)
: Action(action), d_stack_id(action.d_stack_id)
{
}

Action_ParkStack::Action_ParkStack(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_stack_id, "stack_id");
}

bool Action_ParkStack::doSave(XML_Helper* helper) const
{
  return helper->saveData("stack_id", d_stack_id);
}

//-----------------------------------------------------------------------------
// Action_UnparkStack

Action_UnparkStack::Action_UnparkStack(Stack *s)
:Action(Action::STACK_UNPARK), d_stack_id(s->getId())
{
}

Action_UnparkStack::Action_UnparkStack(const Action_UnparkStack &action)
: Action(action), d_stack_id(action.d_stack_id)
{
}

Action_UnparkStack::Action_UnparkStack(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_stack_id, "stack_id");
}

bool Action_UnparkStack::doSave(XML_Helper* helper) const
{
  return helper->saveData("stack_id", d_stack_id);
}

//-----------------------------------------------------------------------------
// Action_SelectStack

Action_SelectStack::Action_SelectStack(Stack *s)
:Action(Action::STACK_SELECT), d_stack_id(s->getId())
{
}

Action_SelectStack::Action_SelectStack(const Action_SelectStack &action)
: Action(action), d_stack_id(action.d_stack_id)
{
}

Action_SelectStack::Action_SelectStack(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_stack_id, "stack_id");
}

bool Action_SelectStack::doSave(XML_Helper* helper) const
{
  return helper->saveData("stack_id", d_stack_id);
}

//-----------------------------------------------------------------------------
// Action_DeselectStack

Action_DeselectStack::Action_DeselectStack()
:Action(Action::STACK_DESELECT)
{
}

Action_DeselectStack::Action_DeselectStack(const Action_DeselectStack &action)
: Action(action)
{
}

Action_DeselectStack::Action_DeselectStack(XML_Helper* helper)
:Action(helper)
{
}

bool Action_DeselectStack::doSave(XML_Helper* helper) const
{
  if (helper)
    return true;
  return false;
}

Glib::ustring Action::actionTypeToString(Action::Type type)
{
  switch (type)
    {
    case Action::STACK_MOVE: return "Action::STACK_MOVE";
    case Action::STACK_SPLIT: return "Action::STACK_SPLIT";
    case Action::STACK_FIGHT: return "Action::STACK_FIGHT";
    case Action::STACK_JOIN: return "Action::STACK_JOIN";
    case Action::RUIN_SEARCH: return "Action::RUIN_SEARCH";
    case Action::TEMPLE_SEARCH: return "Action::TEMPLE_SEARCH";
    case Action::CITY_OCCUPY: return "Action::CITY_OCCUPY";
    case Action::CITY_PILLAGE: return "Action::CITY_PILLAGE";
    case Action::CITY_SACK: return "Action::CITY_SACK";
    case Action::CITY_RAZE: return "Action::CITY_RAZE";
    case Action::CITY_BUY: return "Action::CITY_BUY";
    case Action::CITY_PROD: return "Action::CITY_PROD";
    case Action::REWARD: return "Action::REWARD" ;
    case Action::QUEST: return "Action::QUEST";
    case Action::HERO_EQUIP: return "Action::HERO_EQUIP";
    case Action::UNIT_ADVANCE: return "Action::UNIT_ADVANCE";
    case Action::STACK_DISBAND: return "Action::STACK_DISBAND";
    case Action::MODIFY_SIGNPOST: return "Action::MODIFY_SIGNPOST";
    case Action::CITY_RENAME: return "Action::CITY_RENAME";
    case Action::CITY_VECTOR: return "Action::CITY_VECTOR";
    case Action::FIGHT_ORDER: return "Action::FIGHT_ORDER";
    case Action::RESIGN: return "Action::RESIGN";
    case Action::ITEM_PLANT: return "Action::ITEM_PLANT";
    case Action::PRODUCE_UNIT: return "Action::PRODUCE_UNIT";
    case Action::PRODUCE_VECTORED_UNIT: return "Action::PRODUCE_VECTORED_UNIT";
    case Action::DIPLOMATIC_STATE: return "Action::DIPLOMATIC_STATE";
    case Action::DIPLOMATIC_PROPOSAL: return "Action::DIPLOMATIC_PROPOSAL";
    case Action::DIPLOMATIC_SCORE: return "Action::DIPLOMATIC_SCORE";
    case Action::END_TURN: return "Action::END_TURN";
    case Action::CITY_CONQUER: return "Action::CITY_CONQUER";
    case Action::RECRUIT_HERO: return "Action::RECRUIT_HERO";
    case Action::CITY_DESTITUTE: return "Action::CITY_DESTITUTE";
    case Action::INIT_TURN: return "Action::INIT_TURN";
    case Action::CITY_LOOT: return "Action::CITY_LOOT";
    case Action::USE_ITEM: return "Action::USE_ITEM";
    case Action::STACK_ORDER: return "Action::STACK_ORDER";
    case Action::STACKS_RESET: return "Action::STACKS_RESET";
    case Action::RUINS_RESET: return "Action::RUINS_RESET";
    case Action::COLLECT_TAXES_AND_PAY_UPKEEP: return "Action::COLLECT_TAXES_AND_PAY_UPKEEP";
    case Action::KILL_PLAYER: return "Action::KILL_PLAYER";
    case Action::STACK_DEFEND: return "Action::STACK_DEFEND";
    case Action::STACK_UNDEFEND: return "Action::STACK_UNDEFEND";
    case Action::STACK_PARK: return "Action::STACK_PARK";
    case Action::STACK_UNPARK: return "Action::STACK_UNPARK";
    case Action::STACK_SELECT: return "Action::STACK_SELECT";
    case Action::STACK_DESELECT: return "Action::STACK_DESELECT";
    }
      
  return "Action::MOVE";
}

Action::Type Action::actionTypeFromString(Glib::ustring str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return Action::Type(atoi(str.c_str()));
  if (str == "Action::STACK_MOVE") return Action::STACK_MOVE;
  else if (str == "Action::STACK_SPLIT") return Action::STACK_SPLIT;
  else if (str == "Action::STACK_FIGHT") return Action::STACK_FIGHT;
  else if (str == "Action::STACK_JOIN") return Action::STACK_JOIN;
  else if (str == "Action::RUIN_SEARCH") return Action::RUIN_SEARCH;
  else if (str == "Action::TEMPLE_SEARCH") return Action::TEMPLE_SEARCH;
  else if (str == "Action::CITY_OCCUPY") return Action::CITY_OCCUPY;
  else if (str == "Action::CITY_PILLAGE") return Action::CITY_PILLAGE;
  else if (str == "Action::CITY_SACK") return Action::CITY_SACK;
  else if (str == "Action::CITY_RAZE") return Action::CITY_RAZE;
  else if (str == "Action::CITY_BUY") return Action::CITY_BUY;
  else if (str == "Action::CITY_PROD") return Action::CITY_PROD;
  else if (str == "Action::REWARD" ) return Action::REWARD; 
  else if (str == "Action::QUEST") return Action::QUEST;
  else if (str == "Action::HERO_EQUIP") return Action::HERO_EQUIP;
  else if (str == "Action::UNIT_ADVANCE") return Action::UNIT_ADVANCE;
  else if (str == "Action::STACK_DISBAND") return Action::STACK_DISBAND;
  else if (str == "Action::MODIFY_SIGNPOST") return Action::MODIFY_SIGNPOST;
  else if (str == "Action::CITY_RENAME") return Action::CITY_RENAME;
  else if (str == "Action::CITY_VECTOR") return Action::CITY_VECTOR;
  else if (str == "Action::FIGHT_ORDER") return Action::FIGHT_ORDER;
  else if (str == "Action::RESIGN") return Action::RESIGN;
  else if (str == "Action::ITEM_PLANT") return Action::ITEM_PLANT;
  else if (str == "Action::PRODUCE_UNIT") return Action::PRODUCE_UNIT;
  else if (str == "Action::PRODUCE_VECTORED_UNIT") return Action::PRODUCE_VECTORED_UNIT;
  else if (str == "Action::DIPLOMATIC_STATE") return Action::DIPLOMATIC_STATE;
  else if (str == "Action::DIPLOMATIC_PROPOSAL") return Action::DIPLOMATIC_PROPOSAL;
  else if (str == "Action::DIPLOMATIC_SCORE") return Action::DIPLOMATIC_SCORE;
  else if (str == "Action::END_TURN") return Action::END_TURN;
  else if (str == "Action::CITY_CONQUER") return Action::CITY_CONQUER;
  else if (str == "Action::RECRUIT_HERO") return Action::RECRUIT_HERO;
  else if (str == "Action::CITY_DESTITUTE") return Action::CITY_DESTITUTE;
  else if (str == "Action::INIT_TURN") return Action::INIT_TURN;
  else if (str == "Action::CITY_LOOT") return Action::CITY_LOOT;
  else if (str == "Action::USE_ITEM") return Action::USE_ITEM;
  else if (str == "Action::STACK_ORDER") return Action::STACK_ORDER;
  else if (str == "Action::STACKS_RESET") return Action::STACKS_RESET;
  else if (str == "Action::RUINS_RESET") return Action::RUINS_RESET;
  else if (str == "Action::COLLECT_TAXES_AND_PAY_UPKEEP") return Action::COLLECT_TAXES_AND_PAY_UPKEEP;
  else if (str == "Action::KILL_PLAYER") return Action::KILL_PLAYER;
  else if (str == "Action::STACK_DEFEND") return Action::STACK_DEFEND;
  else if (str == "Action::STACK_UNDEFEND") return Action::STACK_UNDEFEND;
  else if (str == "Action::STACK_PARK") return Action::STACK_PARK;
  else if (str == "Action::STACK_UNPARK") return Action::STACK_UNPARK;
  else if (str == "Action::STACK_SELECT") return Action::STACK_SELECT;
  else if (str == "Action::STACK_DESELECT") return Action::STACK_DESELECT;
  return Action::STACK_MOVE;
}

