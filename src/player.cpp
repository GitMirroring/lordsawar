// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2004 John Farrell
// Copyright (C) 2005 Bryan Duff
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2014, 2015, 2017, 2020,
// 2021 Ben Asselstine
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

#include <stdlib.h>
#include <assert.h>
#include <fstream>
#include <sstream>
#include <sigc++/functors/mem_fun.h>

#include "MoveResult.h"
#include "player.h"
#include "playerlist.h"
#include "stacklist.h"
#include "citylist.h"
#include "templelist.h"
#include "city.h"
#include "path.h"
#include "armysetlist.h"
#include "real_player.h"
#include "ai_dummy.h"
#include "ai_fast.h"
#include "ai_smart.h"
#include "network_player.h"
#include "GameMap.h"
#include "counter.h"
#include "army.h"
#include "hero.h"
#include "heroproto.h"
#include "herotemplates.h"
#include "Configuration.h"
#include "GameScenarioOptions.h"
#include "action.h"
#include "network-action.h"
#include "history.h"
#include "network-history.h"
#include "AI_Analysis.h"
#include "AI_Allocation.h"
#include "FogMap.h"
#include "QuestsManager.h"
#include "signpost.h"
#include "vectoredunit.h"
#include "ucompose.hpp"
#include "armyprodbase.h"
#include "Triumphs.h"
#include "Backpack.h"
#include "MapBackpack.h"
#include "PathCalculator.h"
#include "stacktile.h"
#include "temple.h"
#include "QCityOccupy.h"
#include "QCitySack.h"
#include "QCityRaze.h"
#include "QPillageGold.h"
#include "Quest.h"
#include "QKillHero.h"
#include "QEnemyArmies.h"
#include "QEnemyArmytype.h"
#include "callback-enums.h"
#include "stackreflist.h"
#include "SightMap.h"
#include "rewardlist.h"
#include "Item.h"
#include "ItemProto.h"
#include "xmlhelper.h"
#include "rnd.h"
#include "game-actionlist.h"
#include "turn-actionlist.h"
#include "keeper.h"
#include "Itemlist.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::flush<<std::endl;}
#define debug(x)

Glib::ustring Player::d_tag = "player";

Player::Player(Glib::ustring name, guint32 armyset, std::vector<Gdk::RGBA> colors, int width,
	       int height, Type type, int player_no)
    :d_colors(colors), d_name(name), d_armyset(armyset), d_gold(1000),
    d_dead(false), d_immortal(false), d_type(type), d_upkeep(0), d_income(0),
    d_observable(true), surrendered(false), abort_requested(false)
{
    if (player_no != -1)
	d_id = player_no;
    else
	d_id = fl_counter->getNextId();
    d_stacklist = new Stacklist();
    debug("type of " << d_name << " is " << type)
        
    d_fogmap = new FogMap(width, height);

    //initial fight order is the order in which the armies appear
    //in the default.xml file.
    for (auto i: *Armysetlist::getInstance()->get(d_armyset))
      d_fight_order.push_back(i->getId());

    for (unsigned int i = 0 ; i < MAX_PLAYERS; i++)
    {
      d_diplomatic_state[i] = AT_PEACE;
      d_diplomatic_proposal[i] = NO_PROPOSAL;
      d_diplomatic_score[i] = DIPLOMACY_STARTING_SCORE;
    }
    d_diplomatic_rank = 0;
    d_diplomatic_title = Glib::ustring("");

    d_triumphs = new Triumphs();
}

Player::Player(const Player& player, bool sync_ids)
    :sigc::trackable(player), d_colors(player.d_colors), d_name(player.d_name),
    d_armyset(player.d_armyset), d_gold(player.d_gold), d_dead(player.d_dead),
    d_immortal(player.d_immortal), d_type(player.d_type), d_id(player.d_id),
    d_fight_order(player.d_fight_order), d_upkeep(player.d_upkeep),
    d_income(player.d_income), d_observable(player.d_observable),
    surrendered(player.surrendered),abort_requested(player.abort_requested)
{
  // as the other player is propably dumped somehow, we need to deep copy
  // everything.
  d_stacklist = new Stacklist();
  for (Stacklist::iterator it = player.d_stacklist->begin();
       it != player.d_stacklist->end(); ++it)
    {
      Stack* mine = new Stack(**it, !sync_ids);
      // change the stack's loyalty
      mine->setPlayer(this);
      d_stacklist->add(mine);
    }

  // copy actions
  for (auto ait: player.d_actions)
    d_actions.push_back(Action::copy(ait));

  // copy events
  for (auto pit: player.d_history)
    d_history.push_back(History::copy(pit));

  // copy fogmap
  d_fogmap = new FogMap(*player.getFogMap());

  // copy diplomatic states
  for (unsigned int i = 0 ; i < MAX_PLAYERS; i++)
    {
      d_diplomatic_state[i] = player.d_diplomatic_state[i];
      d_diplomatic_proposal[i] = player.d_diplomatic_proposal[i];
      d_diplomatic_score[i] = player.d_diplomatic_score[i];
    }
  d_diplomatic_rank = player.d_diplomatic_rank;
  d_diplomatic_title = player.d_diplomatic_title;

  d_triumphs = new Triumphs(*player.getTriumphs());
}

Player::Player(XML_Helper* helper)
    :d_stacklist(0), d_fogmap(0), surrendered(false), abort_requested(false)
{
    helper->getData(d_id, "id");
    helper->getData(d_name, "name");
    helper->getData(d_gold, "gold");
    helper->getData(d_dead, "dead");
    helper->getData(d_immortal, "immortal");
    Glib::ustring type_str;
    helper->getData(type_str, "type");
    d_type = playerTypeFromString(type_str);
    helper->getData(d_upkeep, "upkeep");
    helper->getData(d_income, "income");
    helper->getData(d_colors, "color");
    helper->getData(d_armyset, "armyset");

    // Read in Fight Order.  One ranking per army type.
    Glib::ustring fight_order;
    std::stringstream sfight_order;
    guint32 val;
    helper->getData(fight_order, "fight_order");
    sfight_order.str(fight_order);
    for (auto i: *Armysetlist::getInstance()->get (d_armyset))
      {
        (void)i;
        sfight_order >> val;
        d_fight_order.push_back(val);
      }

    // Read in Diplomatic States.  One state per player.
    Glib::ustring diplomatic_states;
    std::stringstream sdiplomatic_states;
    helper->getData(diplomatic_states, "diplomatic_states");
    sdiplomatic_states.str(diplomatic_states);
    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
            sdiplomatic_states >> val;
	    d_diplomatic_state[i] = DiplomaticState(val);
    }

    helper->getData(d_diplomatic_rank, "diplomatic_rank");
    helper->getData(d_diplomatic_title, "diplomatic_title");

    // Read in Diplomatic Proposals.  One proposal per player.
    Glib::ustring diplomatic_proposals;
    std::stringstream sdiplomatic_proposals;
    helper->getData(diplomatic_proposals, "diplomatic_proposals");
    sdiplomatic_proposals.str(diplomatic_proposals);
    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
            sdiplomatic_proposals>> val;
	    d_diplomatic_proposal[i] = DiplomaticProposal(val);
    }

    // Read in Diplomatic Scores.  One score per player.
    Glib::ustring diplomatic_scores;
    std::stringstream sdiplomatic_scores;
    helper->getData(diplomatic_scores, "diplomatic_scores");
    sdiplomatic_scores.str(diplomatic_scores);
    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
            sdiplomatic_scores >> val;
	    d_diplomatic_score[i] = val;
    }
    helper->getData(d_observable, "observable");

    helper->registerTag(Action::d_tag, sigc::mem_fun(this, &Player::load));
    helper->registerTag(History::d_tag, sigc::mem_fun(this, &Player::load));
    helper->registerTag(Stacklist::d_tag, sigc::mem_fun(this, &Player::load));
    helper->registerTag(FogMap::d_tag, sigc::mem_fun(this, &Player::load));
    helper->registerTag(Triumphs::d_tag, sigc::mem_fun(this, &Player::load));

}

Player::~Player()
{
    if (d_stacklist)
    {
        delete d_stacklist;
        d_stacklist = NULL;
    }
    if (d_fogmap)
      {
        delete d_fogmap;
        d_fogmap = NULL;
      }

    delete d_triumphs;
    d_triumphs = NULL;
    clearActionlist();
    clearHistorylist();
}

Player* Player::create(Glib::ustring name, guint32 armyset, std::vector<Gdk::RGBA> colors, int width, int height, Type type)
{
  switch(type)
  {
  case HUMAN:
    return (new RealPlayer(name, armyset, colors, width, height));
  case AI_FAST:
    return (new AI_Fast(name, armyset, colors, width, height));
  case AI_DUMMY:
    return (new AI_Dummy(name, armyset, colors, width, height));
  case AI_SMART:
    return (new AI_Smart(name, armyset, colors, width, height));
  case NETWORKED:
    return (new NetworkPlayer(name, armyset, colors, width, height));
  }

  return 0;
}

Player* Player::create(Player* orig, Type type)
{
    switch(type)
    {
        case HUMAN:
            return new RealPlayer(*orig);
        case AI_FAST:
            return new AI_Fast(*orig);
        case AI_DUMMY:
            return new AI_Dummy(*orig);
        case AI_SMART:
            return new AI_Smart(*orig);
        case NETWORKED:
            return new NetworkPlayer(*orig);
    }

    return 0;
}


void Player::initTurn()
{
  //printf("local: dumping %lu actions\n", d_actions.size());
  //for (auto i: d_actions)
    //{
      //printf("\t%s %s\n", Action::actionTypeToString(i->getType()).c_str(), i->dump().c_str());
    //}

  GameActionlist::getInstance()->add(new TurnActionlist (this, d_actions));
  clearActionlist();
  History_StartTurn* item = new History_StartTurn();
  addHistory(item);
  guint32 order = Playerlist::getInstance()->getTurnOrderNumber(this);
  Action_InitTurn* action = new Action_InitTurn(order);
  addAction(action);
}

void Player::setColor(Gdk::RGBA c)
{
  d_colors[0] = c;
}

void Player::setColors(std::vector<Gdk::RGBA> l)
{
  d_colors = l;
}

void Player::addGold(int gold)
{
    d_gold += gold;
    schangingStats.emit();
}

void Player::withdrawGold(int gold)
{
    d_gold -= gold;
    if (d_gold < 0)
      d_gold = 0; /* bankrupt.  should we start turning off city production? */
    schangingStats.emit();
}

Glib::ustring Player::getName() const
{
  return d_name;
}

void Player::clearActionlist()
{
  for (auto it: d_actions)
    delete (it);
  d_actions.clear();
}

void Player::clearHistorylist(std::list<History*> &history)
{
  for (auto it: history)
    delete (it);
  history.clear();
}

void Player::clearHistorylist()
{
  clearHistorylist(d_history);
}

void Player::addStack(Stack* stack)
{
  debug("Player " << getName() << ": Stack Id: " << stack->getId() << " added to stacklist");
    stack->setPlayer(this);
    d_stacklist->add(stack);
}

bool Player::deleteStack(Stack* stack)
{
  if (isComputer() == true)
    {
      AI_Analysis::deleteStack(stack->getId());
      AI_Allocation::deleteStack(stack);
    }
    return d_stacklist->flRemove(stack);
}

void Player::kill(bool record_action)
{
  doKill();
  if (record_action)
    {
      addAction(new Action_Kill());
      if (d_immortal == false)
        addHistory(new History_PlayerVanquished());
    }
  schangingStats.emit();
}

void Player::doKill()
{
    if (d_immortal)
        // ignore it
        return;

    d_observable = false;

    d_dead = true;
    //drop the bags of stuff that the heroes might be carrying
    std::list<Hero*> h = getHeroes();
    for (std::list<Hero*>::iterator it = h.begin(); it != h.end(); ++it)
      {
	Stack *s = d_stacklist->getArmyStackById((*it)->getId());
	if (s)
	  doStackDisband(s);
      }
    //get rid of all of the other stacks.
    d_stacklist->flClear();

    // Since in some cases the player can be killed rather innocently
    // (using reactions), we also need to clear the player's traces in the
    // single cities
    for (auto city: *Citylist::getInstance())
      if (city->getOwner() == this && city->isBurnt() == false)
        Playerlist::getInstance()->getNeutral()->takeCityInPossession(city);

    d_diplomatic_rank = 0;
    d_diplomatic_title = Glib::ustring("");
}

bool Player::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("id", d_id);
    retval &= helper->saveData("name", d_name);
    retval &= helper->saveData("color", d_colors);
    retval &= helper->saveData("armyset", d_armyset);
    retval &= helper->saveData("gold", d_gold);
    retval &= helper->saveData("dead", d_dead);
    retval &= helper->saveData("immortal", d_immortal);
    Glib::ustring type_str = playerTypeToString(Player::Type(d_type));
    retval &= helper->saveData("type", type_str);
    debug("type of " << d_name << " is " << d_type)
    retval &= helper->saveData("upkeep", d_upkeep);
    retval &= helper->saveData("income", d_income);

    // save the fight order, one ranking per army type
    std::stringstream fight_order;
    for (std::list<guint32>::const_iterator it = d_fight_order.begin();
         it != d_fight_order.end(); ++it)
      {
        fight_order << (*it) << " ";
      }
    retval &= helper->saveData("fight_order", fight_order.str());

    // save the diplomatic states, one state per player
    std::stringstream diplomatic_states;
    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
      {
	diplomatic_states << d_diplomatic_state[i] << " ";
      }
    retval &= helper->saveData("diplomatic_states", diplomatic_states.str());

    retval &= helper->saveData("diplomatic_rank", d_diplomatic_rank);
    retval &= helper->saveData("diplomatic_title", d_diplomatic_title);

    // save the diplomatic proposals, one proposal per player
    std::stringstream diplomatic_proposals;
    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
      {
	diplomatic_proposals << d_diplomatic_proposal[i] << " ";
      }
    retval &= helper->saveData("diplomatic_proposals",
			       diplomatic_proposals.str());

    // save the diplomatic scores, one score per player
    std::stringstream diplomatic_scores;
    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
      {
	diplomatic_scores << d_diplomatic_score[i] << " ";
      }
    retval &= helper->saveData("diplomatic_scores", diplomatic_scores.str());

    retval &= helper->saveData("observable", d_observable);

    //save the actionlist
    for (auto it: d_actions)
        retval &= it->save(helper);
    
    //save the pasteventlist
    for (auto it: d_history)
      retval &= it->save(helper);

    retval &= d_stacklist->save(helper);
    retval &= d_fogmap->save(helper);
    retval &= d_triumphs->save(helper);

    return retval;
}

Player* Player::loadPlayer(XML_Helper* helper)
{
    Type type;
    Glib::ustring type_str;
    helper->getData(type_str, "type");
    type = playerTypeFromString(type_str);

    switch (type)
    {
        case HUMAN:
            return new RealPlayer(helper);
        case AI_FAST:
            return new AI_Fast(helper);
        case AI_SMART:
            return new AI_Smart(helper);
        case AI_DUMMY:
            return new AI_Dummy(helper);
        case NETWORKED:
            return new NetworkPlayer(helper);
    }

    return 0;
}

bool Player::load(Glib::ustring tag, XML_Helper* helper)
{
    if (tag == Action::d_tag)
    {
        Action* action;
        action = Action::handle_load(helper);
        d_actions.push_back(action);
    }
    if (tag == History::d_tag)
    {
        History* history;
        history = History::handle_load(helper);
        d_history.push_back(history);
    }

    if (tag == Stacklist::d_tag)
        d_stacklist = new Stacklist(helper);

    if (tag == FogMap::d_tag)
        d_fogmap = new FogMap(helper);

    if (tag == Triumphs::d_tag)
	d_triumphs = new Triumphs(helper);

    return true;
}

void Player::addAction(Action *action)
{
  d_actions.push_back(action);
  acting.emit (action, getId());
}

void Player::addHistory(History *history)
{
  d_history.push_back(history);
  history_written.emit(history, getId());
}

guint32 Player::getScore() const
{
  //go get our last published score in the history
  guint32 score = 0;
  std::list<History*>::const_reverse_iterator it = d_history.rbegin();
  for (; it != d_history.rend(); ++it)
    {
      if ((*it)->getType() == History::SCORE)
        {
          score = static_cast<History_Score*>(*it)->getScore();
          break;
        }
    }
  return score;
}

void Player::calculateUpkeep()
{
  d_upkeep = 0;
  Stacklist *sl = getStacklist();
  for (Stacklist::iterator i = sl->begin(), iend = sl->end(); i != iend; ++i)
    d_upkeep += (*i)->getUpkeep();
}

void Player::calculateIncome()
{
    d_income = 0;
    for (auto city: *Citylist::getInstance())
      if (city->getOwner() == this)
        d_income += city->getGold();
}

void Player::doSetFightOrder(const std::list<guint32> &order)
{
  d_fight_order = order;
}

void Player::setFightOrder(const std::list<guint32> &order) 
{
  doSetFightOrder(order);
  
  addAction(new Action_FightOrder(order));
}

bool Player::doStackSplitArmy(Stack *s, Army *a, Stack *& new_stack)
{
  new_stack = s->splitArmy(a);
  if (new_stack != NULL)
    {
      debug("1. split stack " << new_stack->getId() << " from stack " << s->getId());
      addStack(new_stack);
      supdatingStack.emit(0);
      return true;
    }
  return false;
}


bool Player::doStackSplitArmies(Stack *stack, const std::list<guint32> &armies,
				Stack *& new_stack)
{
  new_stack = stack->splitArmies(armies);
  if (new_stack != NULL)
    {
      addStack(new_stack);
      return true;
    }
  return false;
}

Stack *Player::stackSplitArmies(Stack *stack, const std::list<guint32> &armies)
{
  Stack *new_stack = NULL;
  bool retval = doStackSplitArmies(stack, armies, new_stack);
  if (retval == true)
    {
      addAction(new Action_Split(stack, new_stack));
      addAction(new Action_ReorderArmies(stack));
      addAction(new Action_ReorderArmies(new_stack));
    }
  return new_stack;
}

Stack *Player::stackSplitArmy(Stack *stack, Army *a)
{
  Stack *new_stack = NULL;
  bool retval = doStackSplitArmy(stack, a, new_stack);
  if (retval == true)
    {
      addAction(new Action_Split(stack, new_stack));
      addAction(new Action_ReorderArmies(stack));
    }
  return new_stack;
}

void Player::doStackJoin(Stack* receiver, Stack* joining)
{
   receiver->join(joining);
   deleteStack(joining);
    //d_stacklist->flRemove(joining);
    
    d_stacklist->setActivestack(receiver);
}

bool Player::stackJoin(Stack* receiver, Stack* joining)
{

    if ((receiver == 0) || (joining == 0))
        return false;
    debug("Player::stackJoin("<<receiver->getId()<<","<<joining->getId()<<")");

    assert (receiver->getPos() == joining->getPos());
    if (GameMap::canJoin(joining, receiver) == false)
      return false;
    
    Action_Join *action = new Action_Join (receiver, joining);

    doStackJoin(receiver, joining);
    addAction(action);
      
    addAction(new Action_ReorderArmies(receiver));
 
    supdatingStack.emit(0);
    return true;
}

bool Player::stackSplitAndMove(Stack* s, Stack *& new_stack)
{
  if (s->hasPath() == false)
    return false;
  Vector<int> pos = s->getLastReachablePointInPath();
  if (pos == Vector<int>(-1,-1))
    return false;
  Stack *join = GameMap::getFriendlyStack(pos);
  if (join)
    return stackSplitAndMoveToJoin(s, join, new_stack);
  else
    return stackSplitAndMoveToAttack(s, new_stack);
}

bool Player::stackSplitAndMoveToJoin(Stack* s, Stack *join, Stack *& new_stack)
{
  //the stack can't get there, but maybe part of the stack can.
  if (s->hasPath() == false)
    return false;

  std::list<guint32> ids;
  ids = s->determineReachableArmies(s->getLastPointInPath());
  if (ids.size() == 0)
    return false;
  //if they're all reachable and we can join, just move them
  if (ids.size() == s->size() && GameMap::canJoin(s, join) == true)
    return stackMove(s);

  //let's take who we can fit.
  if (ids.size() > join->getMaxArmiesToJoin())
    {
      int diff = ids.size() - join->getMaxArmiesToJoin();
      for (int i = 0; i < diff; i++)
        ids.pop_front();
    }

  if (ids.size() == 0)
    return false;
  if (s->fliesWithItemAndNonFlyersOverWaterOrMountains())
    return false;
  //okay, ids.size armies can make the move.  but can that tile accept it?
  new_stack = stackSplitArmies(s, ids);
  if (new_stack)
    {
      setActivestack(new_stack);
      return stackMove(new_stack);
    }
  return false;
}

bool Player::stackSplitAndMoveToAttack(Stack* s, Stack *& new_stack)
{
  //the stack can't get there, but maybe part of the stack can.
  if (s->getPath()->empty())
    return false;

  std::list<guint32> ids;
  ids = s->determineReachableArmies(s->getLastPointInPath());
  if (ids.size() == 0)
    return false;
  if (ids.size() == s->size())
    return stackMove(s);
  if (s->fliesWithItemAndNonFlyersOverWaterOrMountains())
    return false;

  new_stack = stackSplitArmies(s, ids);
  if (new_stack)
    {
      setActivestack(new_stack);
      return stackMove(new_stack);
    }
  return false;
}

bool Player::stackMove(Stack* s)
{
    debug("Player::stackMove(Stack*)")

    if (s->getPath()->empty())
    {
        return false;
    }

    MoveResult *result = stackMove(s, s->getLastPointInPath(), true);
    bool ret = result->didSomething();//result->moveSucceeded();
    delete result;
    result = 0;
    return ret;
}

bool Player::nextStepOnEnemyStackOrCity(Stack *s) const
{
  Vector<int> dest = s->getFirstPointInPath();
  if (dest != Vector<int>(-1,-1))
    {
      if (GameMap::getEnemyStack(dest))
	return true;
      City *enemy = GameMap::getEnemyCity(dest);
      if (enemy && enemy->isBurnt() == false)
	return true;
    }
  return false;
}

MoveResult *Player::stackMove(Stack* s, Vector<int> dest)
{
  if (dest == Vector<int>(-1,-1))
    return stackMove(s, dest, true);
  else
    return stackMove(s, dest, false);
}

MoveResult *Player::stackMove(Stack* s, Vector<int> dest, bool follow)
{
    bool searched_temple = false;
    bool searched_ruin = false;
    bool got_quest = false;
    bool picked_up = false;
    debug("Player::stack_move()");
    //if follow is set to true, follow an already calculated way, else
    //calculate it here
		
    smovingStack.emit(s);
    if (!follow)
    {
        s->getPath()->calculate(s, dest);
    }

    if (s->getPath()->empty())
      {
	MoveResult *result = new MoveResult;
	result->setReachedEndOfPath(true);
        sstoppingStack.emit();
	return result;
      }

    int stepCount = 0;
    int moves_left = s->getPath()->getMovesExhaustedAtPoint();
    while (1)
      {
        if (abortRequested())
          {
            MoveResult *result = new MoveResult;
            result->fillData(s, stepCount, searched_temple, searched_ruin, got_quest, picked_up);
            result->setMoveAborted(true);
            return result;
          }
	if (s->getPath()->size() <= 1)
	  break;
	if (nextStepOnEnemyStackOrCity(s) == true)
	  break;

        bool step = false;
        step = stackMoveOneStep(s);
        if (!step)
          step = stackMoveOneStepOverTooLargeFriendlyStacks(s);
        if (step)
	  {
	    stepCount++;
	    supdatingStack.emit(0);
            if (isComputer())
              {
                MoveResult *result = new MoveResult;
                result->fillData(s, stepCount, searched_temple, searched_ruin, got_quest, picked_up);
                bool stack_died = computerSearch(s, result);
                searched_temple = result->getComputerSearchedTemple();
                searched_ruin = result->getComputerSearchedRuin();
                got_quest = result->getComputerGotQuest();
                picked_up = result->getComputerPickedUpBag();
                if (stack_died)
                  return result;
                else
                  delete result;
              }
	    moves_left--;
	    if (moves_left == 1)
	      break;
	  }
	else
	  break;
      }

    //the idea here is that we're one move away from our destination.
    //but in some cases we've already reached the end of the path
    //because a fight has to happen.

    //did we jump over a too large friendly stack to an enemy stack or city?
  
    //alright, we've walked up to the last place in the path.
    if (s->getPath()->size() >= 1 && s->enoughMoves())
    //now look for fight targets, joins etc.
    {
    
        Vector<int> pos = s->getFirstPointInPath();
        City* city = GameMap::getCity(pos);
        Stack* target = GameMap::getStack(pos);


        //first fight_city to avoid ambiguity with fight_army
        if (city && (city->getOwner() != this) && (!city->isBurnt()))
	  {
	    bool treachery = false;
	    if (this->getDiplomaticState (city->getOwner()) != AT_WAR)
	      {
		if (streacheryStack.emit (s, city->getOwner(),
					  city->getPos()) == false)
		  {
		    //we decided not to be treacherous
		    s->getPath()->clear();
		    MoveResult *moveResult = new MoveResult;
		    moveResult->setConsideredTreachery(true);
		    moveResult->fillData(s, stepCount, searched_temple, searched_ruin, got_quest, picked_up);
                    if (isComputer())
                      computerSearch(s, moveResult);
                    sstoppingStack.emit();
		    return moveResult;
		  }
		else
		  treachery = true;
	      }
	    MoveResult *moveResult = new MoveResult;
	    moveResult->setTreachery(treachery);
	    moveResult->setConsideredTreachery(treachery);
	    if (stackMoveOneStep(s))
	      {
		stepCount++;
	      }
	    else
	      {
		moveResult->fillData(s, stepCount, searched_temple, searched_ruin, got_quest, picked_up);
		shaltedStack.emit(s);
		return moveResult;
	      }

	    moveResult->fillData(s, stepCount, searched_temple, searched_ruin, got_quest, picked_up);
            if (isComputer())
              computerSearch(s, moveResult);

            Fight::Result result;
            std::vector<Stack*> def_in_city = city->getDefenders();
            if (!def_in_city.empty())
            {
                // This is a hack to circumvent the limitations of stackFight.
                if (!target)
                  target = def_in_city[0];
 
                Fight *fight = stackFight(&s, &target);
                result = fight->getResult();
                finishStackFight (fight, &s, &target);
                delete fight;
            }
            else
                result = Fight::ATTACKER_WON;

            moveResult->setFightResult(result);

            // We may only take the city if we have defeated all defenders
            if (result == Fight::ATTACKER_WON)
            {
                adjustDiplomacyFromConqueringCity(city);
                conquerCity(city, s);
                invadeCity(city); //let AIs determine what to do with city
		shaltedStack.emit(s);
            }
            else
              sstoppingStack.emit();
            
	    cityfight_finished(city, result);
            supdatingStack.emit(0);
            
            return moveResult;
        }
        
        //another friendly stack => share the tile if we're human
        else if (target && target->getOwner() == this /*&& 
		 getType() == Player::HUMAN*/)
          {
            MoveResult *moveResult = new MoveResult;
            if (stackMoveOneStep(s))
              stepCount++;
            else
              moveResult->setTooLargeStackInTheWay(true);
	      
	    supdatingStack.emit(0);
	    shaltedStack.emit(d_stacklist->getActivestack());
	    moveResult->fillData(s, stepCount, searched_temple, searched_ruin, got_quest, picked_up);
            if (isComputer())
              computerSearch(s, moveResult);
    
            return moveResult;
         }
        
        //enemy stack => fight
        else if (target)
        {
	  bool treachery = false;
	  if (this->getDiplomaticState (target->getOwner()) == AT_PEACE)
	    {
	      if (streacheryStack.emit (s, target->getOwner(),
					target->getPos()) == false)
		{
		  s->getPath()->clear();
		  MoveResult *moveResult = new MoveResult;
		  moveResult->setConsideredTreachery(true);
		  moveResult->fillData(s, stepCount, searched_temple, searched_ruin, got_quest, picked_up);
                  if (isComputer())
                    computerSearch(s, moveResult);
                  sstoppingStack.emit();
		  return moveResult;
		}
	      else
		treachery = true;
	    }
            MoveResult *moveResult = new MoveResult;
	    moveResult->setTreachery(treachery);
	    moveResult->setConsideredTreachery(treachery);
        
	    moveResult->fillData(s, stepCount, searched_temple, searched_ruin, got_quest, picked_up);
            Fight *fight = stackFight(&s, &target);
            Fight::Result result = fight->getResult();
            moveResult->setFightResult(result);
            finishStackFight(fight, &s, &target);
            delete fight;
            if (result == Fight::ATTACKER_WON)
	      {
                if (stackMoveOneStep(s))
		  stepCount++;
		moveResult->fillData(s, stepCount, searched_temple, searched_ruin, got_quest, picked_up);
                if (isComputer())
                  computerSearch(s, moveResult);
	      }
            
            supdatingStack.emit(0);
            if (result == Fight::ATTACKER_WON)
	      shaltedStack.emit(s);
            else
              sstoppingStack.emit();
            return moveResult;
        }
        
        //else
        if (stackMoveOneStep(s))
          {
            supdatingStack.emit(0);
            stepCount++;
          }

	shaltedStack.emit(s);
    
        MoveResult *moveResult = new MoveResult;
	moveResult->fillData(s, stepCount, searched_temple, searched_ruin, got_quest, picked_up);
        if (isComputer())
          computerSearch(s, moveResult);
        return moveResult;
    }
    else if (s->getPath()->size() >= 1 && s->enoughMoves() == false)
    {
    
        MoveResult *moveResult = new MoveResult;
	moveResult->fillData(s, stepCount, searched_temple, searched_ruin, got_quest, picked_up);
      /* if we can't attack a city, don't remember it in the stack's path. */
        Vector<int> pos = s->getFirstPointInPath();
        City* city = GameMap::getCity(pos);
	if (city && city->getOwner() != this && city->isBurnt() == false)
	  s->clearPath();
    
        if (isComputer())
          computerSearch(s, moveResult);
        sstoppingStack.emit();
        return moveResult;
    }

    MoveResult *moveResult = new MoveResult;
    moveResult->setStepCount(stepCount);
    sstoppingStack.emit();
    return moveResult;
}

	   
bool Player::stackMoveOneStepOverTooLargeFriendlyStacks(Stack *s)
{
  if (!s)
    return false;

  if (!s->enoughMoves())
    return false;

  if (s->getPath()->size() <= 1)
    return false;

  Vector<int> dest = s->getFirstPointInPath();
  Stack *another_stack = GameMap::getStack(dest);
  if (!another_stack)
    return false;

  if (another_stack->getOwner() != s->getOwner())
    return false;

  if (d_stacklist->canJumpOverTooLargeStack(s) == false)
    return false;

  Action_Move *a = new Action_Move(s, dest);
  s->moveOneStep(true);
  a->setMovesLeft(s->getMoves());
  a->setHasShip(s->hasShip());
  addAction(a);
  return true;
}

bool Player::computerSearch(Stack *s, MoveResult *r)
{
  bool stack_died = false;

  Maptile *tile = GameMap::getInstance()->getTile(s->getPos());

  if (tile->getBackpack()->size() > 0)
    {
      if (computerChoosePickupBag(s, s->getPos(), 0, 0) == true)
        {
          r->setComputerPickedUpBag(true);
          Hero *hero = static_cast<Hero*>(s->getFirstHero());
          if (hero)
            heroPickupAllItems(hero, s->getPos());
        }
    }

  //are we at a computer and happen to be a temple, or ruin?
  if (GameMap::can_search(s) == false)
    return false;

  if (tile->getBuilding() == Maptile::TEMPLE)
    {
      if (s->hasHero())
        {
          if (computerChooseVisitTempleForQuest(s, s->getPos(), 0, 0) == true)
            {
              r->setComputerSearchedTemple(true);
              svisitingTemple.emit(GameMap::getTemple(s->getPos()), s);
            }
        }
      else
        {
          if (computerChooseVisitTempleForBlessing(s, s->getPos(), 0, 0) == true)
            {
              r->setComputerSearchedTemple(true);
              bool got_quest =
                svisitingTemple.emit(GameMap::getTemple(s->getPos()), s);
              r->setComputerGotQuest(got_quest);
            }
        }
    }
  else if (tile->getBuilding() == Maptile::RUIN)
    {
      if (s->hasHero() == true)
        {
          if (computerChooseVisitRuin(s, s->getPos(), 0, 0) == true)
            {
              r->setComputerSearchedRuin(true);
              guint32 oldsize = s->size();
              stack_died =
                ssearchingRuin.emit(GameMap::getRuin(s->getPos()), s);
              if (stack_died)
                r->setRuinFightResult(Fight::DEFENDER_WON);
              else
                {
                  if (oldsize >= s->size())
                    r->setRuinFightResult(Fight::ATTACKER_WON);
                  else
                    r->setRuinFightResult(Fight::DEFENDER_WON);
                }
            }
        }
    }
  return stack_died;
}

bool Player::stackMoveOneStep(Stack* s)
{
  if (!s)
    return false;

  if (!s->enoughMoves())
    return false;

  Vector<int> dest = s->getFirstPointInPath();
  
  Stack *another_stack = GameMap::getStack(dest);
  if (another_stack)
    {
      if (another_stack->getOwner() == s->getOwner())
	{
	  if (GameMap::canJoin(s,another_stack) == false)
	    return false;
	}
      else
	{
	  //if we're attacking, then jump onto the square with the enemy.
	  if (s->getPath()->size() != 1)
	    return false;
	}

    }
  Action_Move *a = new Action_Move(s, dest);

  s->moveOneStep();
  a->setMovesLeft(s->getMoves());
  a->setHasShip(s->hasShip());
  addAction(a);


  return true;
}

void Player::cleanupAfterFight(std::list<Stack*> &attackers,
                               std::list<Stack*> &defenders,
                               std::list<History*> &attacker_history,
                               std::list<History*> &defender_history)
{
  // get attacker and defender heroes and more...
  std::vector<guint32> attackerHeroes, defenderHeroes;
    
  getHeroes(attackers, attackerHeroes);
  getHeroes(defenders, defenderHeroes);

  // here we calculate also the total XP to add when a player have a battle
  // clear dead defenders
  //
  double defender_xp = countXPFromDeadArmies(defenders);
  debug("clean dead defenders");
  removeDeadArmies(defenders, attackerHeroes, defender_history);

  // and dead attackers
  double attacker_xp = countXPFromDeadArmies(attackers);
  debug("clean dead attackers");
  removeDeadArmies(attackers, defenderHeroes, attacker_history);

  debug("after fight: attackers empty? " << attackers.empty()
        << "(" << attackers.size() << ")");

  if (!attackers.empty() && defender_xp != 0)
    updateArmyValues(attackers, defender_xp);
    
  if (!defenders.empty() && attacker_xp != 0)
    updateArmyValues(defenders, attacker_xp);

  supdatingStack.emit(0);
}

Fight* Player::stackFight(Stack** attacker, Stack** defender)
{
    debug("stackFight: player = " << getName()<<" at position "
          <<(*defender)->getPos().x<<","<<(*defender)->getPos().y << " with stack " << (*attacker)->getId() << " against " << (*defender)->getId() << " which is player = " <<(*defender)->getOwner()->getName());

    // I suppose, this should be always true, but one can never be sure
    bool attacker_active = *attacker == d_stacklist->getActivestack();
    if (attacker_active == false && (*attacker)->getOwner()->isComputer() == true)
      {
        assert(0);
      }

    Fight *fight = new Fight(*attacker, *defender);
    fight->battle(GameScenarioOptions::s_intense_combat);
    // add a fight item about the combat
    addAction(new Action_Fight(fight));

    fight_started.emit(*fight);
    return fight;
}

void Player::finishStackFight (Fight *fight, Stack **attacker, Stack **defender)
{
    std::list<Stack *> attackers = fight->getAttackers(),
      defenders = fight->getDefenders();

    Player* pd = (*defender)->getOwner();
    bool attacker_active = *attacker == d_stacklist->getActivestack();
    std::list<History*> attacker_history;
    std::list<History*> defender_history;
    cleanupAfterFight(attackers, defenders, attacker_history, defender_history);

    for (std::list<History*>::iterator i = attacker_history.begin();
         i != attacker_history.end(); ++i)
      addHistory(*i);
    for (std::list<History*>::iterator i = defender_history.begin();
         i != defender_history.end(); ++i)
      addHistory(*i);
  
    for (std::list<Stack*>::iterator i = attackers.begin();
         i != attackers.end(); ++i)
      addAction(new Action_ReorderArmies(*i));

    for (std::list<Stack*>::iterator i = defenders.begin();
         i != defenders.end(); ++i)
      addAction(new Action_ReorderArmies(*i));
    
    // Set the attacker and defender stack to 0 if neccessary. This is a great
    // help for the functions calling stackFight (e.g. if a stack attacks
    // another stack and destroys it without winning the battle, it may take the
    // position of this stack)

    // First, the attacker...
    bool exists =
	std::find(d_stacklist->begin(), d_stacklist->end(), *attacker)
	!= d_stacklist->end();
    
    if (!exists)
    {
        (*attacker) = 0;
        if (attacker_active)
            d_stacklist->setActivestack(0);
    }

    // ...then the defender.
    exists = false;
    if (pd)
      exists =
	std::find(pd->getStacklist()->begin(), pd->getStacklist()->end(),
		  *defender) != pd->getStacklist()->end();
    else
        exists = true;
    if (!exists)
        (*defender) = 0;

    schangingStats.emit();
    return;
}

/*
 *
 * To help factor in the advantage of hero experience/strength and 
 * ruin-monster strength as well as the stack strength, I think you'll 
 * find it'll be easier to calculate in terms of the odds of failure [than
 * the odds of success].  A new hero (minimum strength) with nothing in 
 * the stack to help him might have 10-20% odds of failure at a wimpy ruin.
 * The same novice hero facing a dragon in the ruin might have 50% odds of 
 * failure.  So a rule of thumb would be to start with a 25% chance of
 * failure.  The odds would be doubled by the worst monster and halved by 
 * the easiest.  I agree that a strength-9 hero with 8 in the stack should i
 * definitely be at 99%.  A reasonable formula might be:
 *
 * OddsOfFailure = BaseOdds * MonsterFactor * StackFactor * HeroFactor,
 *
 * with
 *        BaseOdds = 0.10
 * and
 *        MonsterFactor = 2, 1 or 0.5 depending on hard vs. easy
 * and
 *        StackFactor = (9 - SizeOfStack)/8,
 * and
 *        HeroFactor = (10-StrengthOfHero)/5.
 */
Fight::Result ruinfight (Stack **attacker, Stack **defender)
{
  Stack *loser;
  Fight::Result result;
  guint32 hero_strength, monster_strength;
  hero_strength = (*attacker)->getFirstHero()->getStat(Army::STRENGTH, true);
  monster_strength = (*defender)->getStrongestArmy()->getStat(Army::STRENGTH, true);
  float base_factor = 0.28;
  float stack_factor = ((float)(MAX_STACK_SIZE + 1) - (*attacker)->size()) / (float)MAX_STACK_SIZE;
  float hero_factor = (10.0 - hero_strength) / 5.0;
  float monster_factor;
  if (monster_strength >= 8)
    monster_factor = 2.0;
  else if (monster_strength >= 6)
    monster_factor = 1.0;
  else
    monster_factor = 0.5;
  float fail = base_factor * monster_factor * stack_factor * hero_factor;

  if (Rnd::rand() % 100 > fail * 100.0)
    {
      result = Fight::ATTACKER_WON;
      loser = *defender;
      for (Stack::iterator sit = loser->begin(); sit != loser->end();)
        {
          (*sit)->setHP (0);
          ++sit;
        }
    }
  else
    {
      result = Fight::DEFENDER_WON;
      loser = *attacker;
      loser->getFirstHero()->setHP(0); /* only the hero dies */
    }
        
  return result;
}

Fight::Result Player::stackRuinFight (Stack **attacker, Keeper *defender,
                                      bool &stackdied,
                                      std::list<History*> &attacker_history,
                                      std::list<History*> &defender_history)
{
    Fight::Result result = Fight::DRAW;
    if (defender->getStack () == NULL)
      return Fight::ATTACKER_WON;
    debug("stackRuinFight: player = " << getName()<<" at position "
          <<(*defender)->getStack ()->getPos().x<<","<<(*defender)->getStack ()->getPos().y);

    ruinfight_started.emit(*attacker, defender);
    Stack *defender_stack = defender->getStack ();
    result = ruinfight (attacker, &defender_stack);

    ruinfight_finished.emit(result);
    // cleanup
    
    // get attacker and defender heroes and more...
    std::list<Stack*> attackers;
    attackers.push_back(*attacker);
    std::list<Stack*> defenders;
    defenders.push_back(defender_stack);

    cleanupAfterFight(attackers, defenders, attacker_history, defender_history);
    bool exists =
	std::find(d_stacklist->begin(), d_stacklist->end(), *attacker)
	!= d_stacklist->end();
    
    if (!exists)
      {
        (*attacker) = 0;
        stackdied = true;
      }
    else
      stackdied = false;

    schangingStats.emit();
    return result;
}

void Player::doStackSearchRuin(Stack *s, Ruin *r, Fight::Result result)
{
  if (result == Fight::DEFENDER_WON)
    {
      r->setSearched(false);
      return;
    }
  else if (result == Fight::ATTACKER_WON)
    {
      r->setSearched(true);
      r->clearOccupant();
      r->setOwner(s->getOwner());
    }
  return;
}

Reward* Player::stackSearchRuin(Stack* s, Ruin* r, bool &stackdied)
{
  std::list<History*> attacker_history;
  std::list<History*> defender_history;
  Keeper *keeper = r->getOccupant();
  if (keeper)
    {
      Fight::Result result = stackRuinFight(&s, keeper, stackdied,
                                            attacker_history, defender_history);
      //we delete it here because keepers are not in any players' stacklist.
      if (result == Fight::ATTACKER_WON)
        delete keeper;
      for (std::list<History*>::iterator i = attacker_history.begin();
           i != attacker_history.end(); ++i)
        addHistory(*i);
      clearHistorylist(defender_history);

      if (result == Fight::ATTACKER_WON &&
          r->getReward() == NULL && r->hasSage() == false)
        r->populateWithRandomReward();
      doStackSearchRuin(s, r, result);
      if (result == Fight::DEFENDER_WON)
        {
          addAction(new Action_Ruin(r,s));
          return NULL;
        }
    }
  else
    {
      if (r->getReward() == NULL && r->hasSage() == false)
        r->populateWithRandomReward();
      doStackSearchRuin(s, r, Fight::ATTACKER_WON);
    }

  Reward *reward = r->takeReward();
  addAction(new Action_Ruin(r, s));
  if (r->isSearched())
    {
      if (r->hasSage())
        addHistory(new History_FoundSage(dynamic_cast<Hero *>(s->getFirstHero())));
      addHistory(new History_HeroRuinExplored(dynamic_cast<Hero*>(s->getFirstHero()), r));
    }
  supdatingStack.emit(0);
  return reward;
}

int Player::doStackVisitTemple(Stack *s)
{
  // you have your stack blessed (+1 strength)
  int count = s->bless();

  supdatingStack.emit(0);
  
  return count;
}

int Player::stackVisitTemple(Stack* s, Temple* t)
{
  debug("Player::stackVisitTemple");

  addAction(new Action_Temple(t,s));
  
  return doStackVisitTemple(s);
}

Quest* Player::doHeroGetQuest(Hero *hero, bool except_raze)
{
  std::vector<Quest*> quests =
    QuestsManager::getInstance()->getPlayerQuests(Playerlist::getActiveplayer());
  if (quests.size() > 0 && GameScenarioOptions::s_play_with_quests == GameParameters::ONE_QUEST_PER_PLAYER)
    return NULL;

  Quest *q = NULL;
  if (hero)
    q = QuestsManager::getInstance()->createNewQuest (hero->getId(), except_raze);

  supdatingStack.emit(0);
  // couldn't assign a quest for various reasons
  if (!q)
    return NULL;
  return q;
}

Quest* Player::heroGetQuest(Hero *hero, Temple* t, bool except_raze)
{
  debug("Player::stackGetQuest")
  (void) t;
  Quest *q = doHeroGetQuest(hero, except_raze);
  if (q == NULL)
    return q;

  // Now fill the action item
  addAction(new Action_Quest(q));

  // and record it for posterity
  addHistory(new History_HeroQuestStarted(hero));
  return q;
}

float Player::stackFightAdvise(Stack* s, Vector<int> tile,
                               bool intense_combat)
{
  float percent = 0.0;
        
  City* city = GameMap::getCity(tile);
  Stack* target = GameMap::getEnemyStack(tile);
                
  if (!target && city)
    {
      std::vector<Stack*> def_in_city = city->getDefenders();
      if (def_in_city.empty())
	return 100.0;
      target = def_in_city[0];
    }

  //what chance is there that stack will defeat defenders?
    
  for (unsigned int i = 0; i < 100; i++)
    {
      Fight fight(s, target, Fight::FOR_KICKS);
      fight.battle(intense_combat);
      if (fight.getResult() == Fight::ATTACKER_WON)
	percent += 1.0;
    }

  advice_asked.emit(percent);
  return percent;
}

void Player::adjustDiplomacyFromConqueringCity(City *city)
{
  Player *defender = city->getOwner();
  
  // See if this is the last city for that player, and alter the 
  // diplomatic scores.
  if (Citylist::getInstance()->countCities(defender) == 1)
  {
    if (defender->getDiplomaticRank() < getDiplomaticRank())
      deteriorateDiplomaticRelationship (2);
    else if (defender->getDiplomaticRank() > getDiplomaticRank())
      improveDiplomaticRelationship (2, defender);
  }
}

void Player::calculateLoot(Player *looted, guint32 &added, guint32 &subtracted)
{
  Player *defender = looted;

  // if the attacked city isn't neutral, loot some gold
  if (defender != Playerlist::getInstance()->getNeutral())
  {
    int amt = (defender->getGold() / 
               (2 * (Citylist::getInstance()->countCities (defender)+1)) * 2);
    // give (Enemy-Gold/(2Enemy-Cities)) to the attacker 
    // and then take away twice that from the defender.
    // the idea here is that some money is taken in the invasion
    // and other monies are lost forever
    // NOTE: +1 because the looted player just lost a city
    subtracted = amt;
    amt /= 2;
    added = amt;
  }

  return;
}

void Player::doConquerCity(City *city)
{
  takeCityInPossession(city);
}

//this helps us test.
void Player::conquerAllCities()
{
  for (auto city: *Citylist::getInstance())
    {
      if (city->getOwner() != this)
        {
          for (auto stack: city->getDefenders())
              GameMap::getInstance()->removeStack(stack);
          conquerCity (city, NULL);
        }
    }
}

void Player::conquerCity(City *city, Stack *stack)
{
  /*
     fixme:
     there is some weirdness here where we conquer the initial cities twice.
     we have to make sure it works with the city history report dialog,
     and getFirstCity.
     i guess the histories are getting erased somewhere after we conquer
     the city the first time.
   */
  Player *original_owner = city->getOwner();
  Action_ConquerCity *action = new Action_ConquerCity(city);

  doConquerCity(city);
  addAction(action);
  addHistory(new History_CityWon(city));
  if (stack && stack->hasHero())
  {
    Hero *hero = dynamic_cast<Hero *>(stack->getFirstHero());
    addHistory(new History_HeroCityWon(city, hero));
  }
  if (original_owner != this)
    lootCity(city, original_owner);
}

void Player::lootCity(City *city, Player *looted)
{
  guint32 added = 0;
  guint32 subtracted = 0;
  calculateLoot(looted, added, subtracted);
  sinvadingCity.emit(city, added);
  doLootCity(looted, added, subtracted);
  addAction(new Action_Loot(this, looted, added, subtracted));
  return;
}

void Player::doLootCity(Player *looted, guint32 added, guint32 subtracted)
{
  addGold(added);
  looted->withdrawGold(subtracted);
  return;
}

void Player::takeCityInPossession(City* c)
{
  c->conquer(this);

  //set the production to the cheapest armytype
  c->setActiveProductionSlot(-1);
  if (c->getArmytype(0) != -1)
    c->setActiveProductionSlot(0);

  supdatingCity.emit(c);
}

void Player::doCityOccupy(City *c)
{
  assert (c->getOwner() == this);
  
  soccupyingCity.emit(c, getActivestack());
  QuestsManager::getInstance()->cityOccupied(c, getActivestack());
}

void Player::cityOccupy(City* c)
{
  debug("cityOccupy");
  doCityOccupy(c);

  addAction(new Action_Occupy(c));
}

void Player::doCityPillage(City *c, int& gold, int* pillaged_army_type)
{
  gold = 0;
  if (pillaged_army_type)
    *pillaged_army_type = -1;
  
  // get rid of the most expensive army type and trade it in for 
  // half it's cost
  // it is presumed that the last army type is the most expensive

  if (c->getNoOfProductionBases() > 0)
    {
      unsigned int i;
      unsigned int max_cost = 0;
      int slot = -1;
      for (i = 0; i < c->getNoOfProductionBases(); i++)
	{
	  const ArmyProdBase *a = c->getProductionBase(i);
	  if (a != NULL)
	    {
	      if (a->getNewProductionCost() == 0)
		{
		  slot = i;
		  break;
		}
	      if (a->getNewProductionCost() > max_cost)
		{
		  max_cost = a->getNewProductionCost();
		  slot = i;
		}
	    }
	}
      if (slot > -1)
	{
	  const ArmyProdBase *a = c->getProductionBase(slot);
	  if (pillaged_army_type)
	    *pillaged_army_type = a->getTypeId();
	  if (a->getNewProductionCost() == 0)
	    gold += 1500;
	  else
	    gold += a->getNewProductionCost() / 2;
	  c->removeProductionBase(slot);
	}
      addGold(gold);
      Stack *s = getActivestack();
      spillagingCity.emit(c, s, gold, *pillaged_army_type);
      QuestsManager::getInstance()->cityPillaged(c, s, gold);
    }

}

void Player::cityPillage(City* c, int& gold, int* pillaged_army_type)
{
  debug("Player::cityPillage");
  
  addAction(new Action_Pillage(c));

  doCityPillage(c, gold, pillaged_army_type);
}

void Player::doCitySack(City* c, int& gold, std::list<guint32> *sacked_types)
{
  gold = 0;
  //trade in all of the army types except for one
  //presumes that the army types are listed in order of expensiveness

  if (c->getNoOfProductionBases() > 1)
    {
      const ArmyProdBase *a;
      unsigned int i, max = 0;
      for (i = 0; i < c->getNoOfProductionBases(); i++)
	{
	  a = c->getProductionBase(i);
	  if (a)
	    max++;
	}

      i = c->getNoOfProductionBases() - 1;
      while (max > 1)
	{
	  a = c->getProductionBase(i);
	  if (a != NULL)
	    {
	      sacked_types->push_back(a->getTypeId());
	      if (a->getNewProductionCost() == 0)
		gold += 1500;
	      else
		gold += a->getNewProductionCost() / 2;
	      c->removeProductionBase(i);
	      max--;
	    }
	  i--;
	}
    }

  addGold(gold);
  Stack *s = getActivestack();
  ssackingCity.emit(c, s, gold, *sacked_types);
  QuestsManager::getInstance()->citySacked(c, s, gold);
}

void Player::citySack(City* c, int& gold, std::list<guint32> *sacked_types)
{
  debug("Player::citySack");

  addAction(new Action_Sack(c));

  doCitySack(c, gold, sacked_types);
}

void Player::doCityRaze(City *c)
{
  c->conquer(this);
  c->setBurnt(true);

  supdatingCity.emit(c);

  srazingCity.emit(c, getActivestack());
  QuestsManager::getInstance()->cityRazed(c, getActivestack());
}

void Player::cityRaze(City* c)
{
  debug("Player::cityRaze");

  addAction(new Action_Raze(c));

  addHistory(new History_CityRazed(c));

  doCityRaze(c);
}

void Player::doCityBuyProduction(City* c, int slot, int type)
{
  guint32 as = c->getOwner()->getArmyset();

  c->removeProductionBase(slot);
  c->addProductionBase(slot, new ArmyProdBase
                       (*Armysetlist::getInstance()->getArmy(as, type)));

  // and do the rest of the neccessary actions
  withdrawGold(Armysetlist::getInstance()->getArmy(as, type)->getNewProductionCost());
}

bool Player::cityBuyProduction(City* c, int slot, int type)
{
  guint32 as = c->getOwner()->getArmyset();

  // sort out unusual values (-1 is allowed and means "scrap production")
  if (type <= -1 || Armysetlist::getInstance()->getArmy(d_armyset, type) == NULL)
    return false;

  // return if we don't have enough money
  if (type != -1 && 
      (int)Armysetlist::getInstance()->getArmy(as, type)->getNewProductionCost() > d_gold)
    return false;

  // return if the city already has the production
  if (c->hasProductionBase(type))
    return false;

  // can't put it in that slot
  if (slot >= (int)c->getMaxNoOfProductionBases())
    return false;
  
  addAction(new Action_Buy (c, slot, Armysetlist::getInstance()->getArmy(as, type)));

  doCityBuyProduction(c, slot, type);

  return true;
}

void Player::doCityChangeProduction(City* c, int slot)
{
  c->setActiveProductionSlot(slot);
  if (slot < 0)
    c->setVectoring(Vector<int>(-1,-1));
}

bool Player::cityChangeProduction(City* c, int slot)
{
  doCityChangeProduction(c, slot);
  addAction(new Action_Production(c, slot));
  return true;
}

void Player::doGiveReward(Stack *s, Reward *reward, StackReflist *stacks)
{
  switch (reward->getType())
    {
    case Reward::GOLD:
      addGold(dynamic_cast<Reward_Gold*>(reward)->getGold());
      break;
    case Reward::ALLIES:
        {
          const ArmyProto *a = dynamic_cast<Reward_Allies*>(reward)->getArmy();

          Reward_Allies::addAllies(s->getOwner(), s->getPos(), a,
      			     dynamic_cast<Reward_Allies*>(reward)->getNoOfAllies(), stacks);
        }
      break;
    case Reward::ITEM:
        {
          Item *i = new Item (*dynamic_cast<Reward_Item*>(reward)->getItem());
          Hero *hero = static_cast<Hero*>(s->getFirstHero());
          hero->getBackpack()->addToBackpack(i);
        }
      break;
    case Reward::RUIN:
        {
          //assign the hidden ruin to this player
          Ruin *r = dynamic_cast<Reward_Ruin*>(reward)->getRuin();
          r->setHidden(true);
          r->setOwner(this);
	  r->deFog(this);
        }
      break;
    case Reward::MAP:
        {
          Reward_Map *map = dynamic_cast<Reward_Map*>(reward);
          d_fogmap->alterFog(map->getSightMap());
        }
      break;
    }
}

bool Player::giveReward(Stack *s, Reward *reward, StackReflist *stacks, bool quest)
{
  debug("Player::give_reward");

  Action_Reward *action = new Action_Reward(s, reward);
  doGiveReward(s, reward, stacks);
  
  addAction(action);

  if (reward->getType() == Reward::RUIN && !quest)
    {
      Ruin *r = dynamic_cast<Reward_Ruin*>(reward)->getRuin();
      addHistory(new History_HeroRewardRuin(dynamic_cast<Hero*>(s->getFirstHero()), r));
    }
  schangingStats.emit();
  return true;
}

bool Player::doStackDisband(Stack* s)
{
    getStacklist()->setActivestack(0);
    s->kill();
    std::list<History*> history;
    removeDeadArmies(s, history);
    clearHistorylist(history);
    supdatingStack.emit(0);
    return true;
}

bool Player::stackDisband(Stack* s)
{
  debug("Player::stackDisband(Stack*)")
    if (!s)
      s = getActivestack();

  addAction(new Action_Disband(s));

  bool retval = doStackDisband(s);
  schangingStats.emit();
  return retval;
}

void Player::doHeroDropItem(Hero *h, Item *i, Vector<int> pos, bool &splash)
{
  if (GameMap::getInstance()->canDropBag(pos) == false)
    {
      h->getBackpack()->removeFromBackpack(i);
      delete i;
      splash = true;
    }
  else
    {
      GameMap::getInstance()->getTile(pos)->getBackpack()->addToBackpack(i);
      h->getBackpack()->removeFromBackpack(i);
      splash = false;
    }
  supdatingStack.emit(0);
}

bool Player::heroDropItem(Hero *h, Item *i, Vector<int> pos, bool &splash)
{
  doHeroDropItem(h, i, pos, splash);
  addAction(new Action_Equip(h, i, Action_Equip::GROUND, pos));
  return true;
}

bool Player::doHeroDropAllItems(Hero *h, Vector<int> pos, bool &splash)
{
  while (h->getBackpack()->empty() == false)
    doHeroDropItem(h, h->getBackpack()->front(), pos, splash);
  sbagdropped.emit ();
  supdatingStack.emit(0);
  return true;
}

void Player::doHeroPickupItem(Hero *h, Item *i, Vector<int> pos)
{
  bool found = GameMap::getInstance()->getTile(pos)->getBackpack()->removeFromBackpack(i);
  if (found)
    h->getBackpack()->addToBackpack(i);
  supdatingStack.emit(0);
}

bool Player::heroPickupItem(Hero *h, Item *i, Vector<int> pos)
{
  doHeroPickupItem(h, i, pos);
  addAction(new Action_Equip(h, i, Action_Equip::BACKPACK, pos));
  return true;
}

bool Player::doHeroPickupAllItems(Hero *h, Vector<int> pos)
{
  MapBackpack *backpack = GameMap::getInstance()->getTile(pos)->getBackpack();
  while (backpack->empty() == false)
    doHeroPickupItem(h, backpack->front(), pos);
  return true;
}

bool Player::heroPickupAllItems(Hero *h, Vector<int> pos)
{
  MapBackpack *backpack = GameMap::getInstance()->getTile(pos)->getBackpack();
  while (backpack->empty() == false)
    heroPickupItem(h, backpack->front(), pos);
  return true;
}

bool Player::heroCompletesQuest(Hero *h)
{
  // record it for posterity
  addHistory(new History_HeroQuestCompleted(h));
  return true;
}

void Player::doResign(std::list<History*> &histories)
{
  //disband all stacks
  std::list<Stack*> stacks = getStacklist()->kill();
  removeDeadArmies(stacks, histories);

  //raze all cities
  for (auto city: *Citylist::getInstance())
    {
      if (city->getOwner() == this)
	{
	  city->setBurnt(true);
          histories.push_back(new History_CityRazed(city));
	}
    }
  withdrawGold(getGold()); //empty the coffers!

  getStacklist()->setActivestack(0);
  supdatingStack.emit(0);
}

void Player::resign() 
{
  std::list<History*> history;
  doResign(history);
  for (std::list<History*>::iterator i = history.begin(); i != history.end();
       ++i)
    addHistory(*i);
  
  addAction(new Action_Resign());
  schangingStats.emit();
}

void Player::doSignpostChange(Signpost *s, Glib::ustring message)
{
  s->setName(message);
}

bool Player::signpostChange(Signpost *s, Glib::ustring message)
{
  if (!s)
    return false;
  
  doSignpostChange(s, message);
  
  addAction(new Action_ModifySignpost(s, message));
  return true;
}

void Player::doCityRename(City *c, Glib::ustring name)
{
  c->setName(name);
}

bool Player::cityRename(City *c, Glib::ustring name)
{
  if (!c)
    return false;

  doCityRename(c, name);
  
  addAction(new Action_RenameCity(c, name));
  return true;
}

void Player::doVectorFromCity(City * c, Vector<int> dest)
{
  c->setVectoring(dest);
}

bool Player::vectorFromCity(City * c, Vector<int> dest)
{
  if (dest != Vector<int>(-1,-1))
    {
      std::list<City*> cities;
      cities = Citylist::getInstance()->getCitiesVectoringTo(dest);
      if (cities.size() >= MAX_CITIES_VECTORED_TO_ONE_CITY)
	return false;
    }
  doVectorFromCity(c, dest);
  
  addAction(new Action_Vector(c, dest));
  return true;
}

bool Player::doChangeVectorDestination(Vector<int> src, Vector<int> dest,
				       std::list<City*> &vectored)
{
  //DEST can be a flag.
  //SRC can be a flag too.
  //Note: we don't actually have a way in the gui to change the vectoring 
  //from the planted standard (flag).
  bool retval = true;
  //sanity checks:
  //disallow changing vectoring from or to a city that isn't ours
  //disallow vectoring to something that isn't our city or our planted 
  //standard.
  City *src_city = GameMap::getCity(src);
  if (src_city == NULL)
    {
      //maybe it's a flag we're changing the vector destination from.
      if (GameMap::getInstance()->findPlantedStandard(this) != src)
	return false;
    }
  else
    {
      if (src_city->getOwner() != this)
	return false;
    }
  City *dest_city = GameMap::getCity(dest);
  if (dest_city == NULL)
    {
      if (GameMap::getInstance()->findPlantedStandard(this) != dest)
	return false;
    }
  else
    {
      if (dest_city->getOwner() != this)
	return false;
    }

  //check to see if the destination has enough room to accept all of the
  //cities we want to send to it.
  std::list<City*> sources = Citylist::getInstance()->getCitiesVectoringTo(src);
  std::list<City*> alreadyvectored =
    Citylist::getInstance()->getCitiesVectoringTo(dest);

  if (alreadyvectored.size() + sources.size() > MAX_CITIES_VECTORED_TO_ONE_CITY)
    return false;

  //okay, do the vectoring changes.
  std::list<City*>::iterator it = sources.begin();
  for (; it != sources.end(); ++it)
    retval &= (*it)->changeVectorDestination(dest);
  vectored = sources;
  return retval;
}

bool Player::changeVectorDestination(Vector<int> src, Vector<int> dest)
{
  std::list<City*> vectored;
  bool retval = doChangeVectorDestination(src, dest, vectored);
  if (retval == false)
    return retval;

  std::list<City*>::iterator it = vectored.begin();
  for (; it != vectored.end(); ++it)
    addAction(new Action_Vector((*it), dest));
  return true;
}

bool Player::heroPlantStandard(Stack* s)
{
  debug("Player::heroPlantStandard(Stack*)");
  if (!s)
    s = getActivestack();
  
  for (Stack::iterator it = s->begin(); it != s->end(); ++it)
  {
    if ((*it)->isHero())
    {
      Hero *hero = dynamic_cast<Hero*>((*it));
      Item *item = hero->getBackpack()->getPlantableItem(this);
      if (item)
        {
          //drop the item, and plant it
          doHeroPlantStandard(hero, item, s->getPos());
                  
          addAction(new Action_Plant(hero, item));
          return true;
        }
    }
  }
  return true;
}

void Player::doHeroPlantStandard(Hero *hero, Item *item, Vector<int> pos)
{
  item->setPlanted(true);
  GameMap::getInstance()->getTile(pos)->getBackpack()->addToBackpack(item);
  hero->getBackpack()->removeFromBackpack(item);
  supdatingStack.emit(0);
}

void Player::getHeroes(const std::list<Stack*> &stacks, std::vector<guint32>& dst)
{
  for (std::list<Stack*>::const_iterator it = stacks.begin();
       it != stacks.end(); ++it)
    (*it)->getHeroes(dst);
}

guint32 Player::removeDeadArmies(Stack *stack, std::list<History*> &history)
{
  std::list<Stack*> stacks;
  stacks.push_back(stack);
  return removeDeadArmies(stacks, history);
}

guint32 Player::removeDeadArmies(std::list<Stack*>& stacks,
                                 std::list<History*> &history)
{
  std::vector<guint32> culprits;
  return removeDeadArmies(stacks, culprits, history);
}

guint32 Player::removeDeadArmies(std::list<Stack*>& stacks,
                                 std::vector<guint32>& culprits,
                                 std::list<History*> &history)
{
  guint32 count = 0;
  Player *owner = NULL;
  if (stacks.empty() == false)
    {
      owner = (*stacks.begin())->getOwner();
      debug("Owner = " << owner);
      if (owner)
        {
          debug("Owner of the stacks: " << owner->getName()
                << ", his stacklist = " << owner->getStacklist());
        }
    }
  for (unsigned int i = 0; i < culprits.size(); i++)
    debug("Culprit: " << culprits[i]);

  tallyDeadArmyTriumphs(stacks);
  handleDeadHeroes(stacks, history);
  handleDeadArmiesForQuests(stacks, culprits);

  std::list<Stack*>::iterator it;
  for (it = stacks.begin(); it != stacks.end(); )
    {
      debug("Stack: " << (*it))
        if ((*it))
          {
            debug("Stack id: " << (*it)->getId());
          }
      for (Stack::iterator sit = (*it)->begin(); sit != (*it)->end();)
        {
          debug("Army: " << (*sit) << " " << (*sit)->getId())
            if ((*sit)->getHP() <= 0)
              {
                debug("Dead Army: " << (*sit)->getName())

                  count++;
                sit = (*it)->flErase(sit);
                continue;
              }

          ++sit;
        }

      debug("Is stack empty?")

        if ((*it)->empty())
          {
            bool ruinstack = false;
            if (owner == Playerlist::getInstance ()->getNeutral () &&
                GameMap::getInstance ()->getBuilding ((*it)->getPos ()) ==
                Maptile::RUIN)
              ruinstack = true;

            if (!ruinstack)
              {
                debug("Yes, removing this stack from the owner's stacklist");
                bool found = owner->deleteStack(*it);
                if (found == false)
                  {
                    printf("couldn't find stack id %d for player %d\n", (*it)->getId(), owner->getId());
                    printf("is it in our own stacklist?");
                    Stack *a = getStacklist()->getStackById((*it)->getId());
                    if  (a)
                      printf(" yes\n");
                    else
                      printf(" no\n");
                  }
                assert (found == true);
              }
            else // there is no owner - like for the ruin's occupants
              {
                debug("No owner for this stack - do stacklist too");
              }

            debug("Removing from the vector too (the vector had "
                  << stacks.size() << " left)");
            it = stacks.erase(it);
          }
        else
          ++it;
    }
  debug("after removeDead: num stacks = " << stacks.size());
  return count;
}

void Player::doHeroGainsLevel(Hero *hero, Army::Stat stat)
{
  hero->gainLevel(stat);
}

void Player::updateArmyValues(std::list<Stack*>& stacks, double xp_sum)
{
  std::list<Stack*>::iterator it;
  double numberarmy = 0;

  for (it = stacks.begin(); it != stacks.end(); ++it)
    numberarmy += (*it)->size();

  for (it = stacks.begin(); it != stacks.end(); )
    {
      debug("Stack: " << (*it))

	for (Stack::iterator sit = (*it)->begin(); sit != (*it)->end();)
	  {
	    Army *army = *sit;
	    debug("Army: " << army)

	      // here we adds XP
	      army->gainXp((double)((xp_sum)/numberarmy));
	    debug("Army gets " << (double)((xp_sum)/numberarmy) << " XP")

	      // here we adds 1 to number of battles
	      army->setBattlesNumber(army->getBattlesNumber()+1);
	    debug("Army battles " <<  army->getBattlesNumber())

	      // medals only go to non-ally armies.
	      if ((*it)->hasHero() && army->isHero() == false && 
		  army->getAwardable() == false)
		{
		  if((army->getBattlesNumber())>10 && 
		     !(army->getMedalBonus(2)))
		    {
		      army->setMedalBonus(2,true);
		      // We must recalculate the XPValue of this unit since it 
		      // got a medal
		      army->setXpReward(army->getXpReward()+1);
		      // We get the medal bonus here
		      army->setStat(Army::STRENGTH, army->getStat(Army::STRENGTH, false)+1);
		      // Emit signal
		      snewMedalArmy.emit(army, 2);
		    }

		  debug("Army hits " <<  army->getNumberHasHit())

		    // Only give medals if the unit has attacked often enough, else
		    // medals lose the flair of something special; a value of n 
		    // means roughly to hit an equally strong unit around n 
		    // times. (note: one hit! An attack can consist of up to 
		    // strength hits)
		    if((army->getNumberHasHit()>50) && !army->getMedalBonus(0))
		      {
			army->setMedalBonus(0,true);
			// We must recalculate the XPValue of this unit since it
			// got a medal
			army->setXpReward(army->getXpReward()+1);
			// We get the medal bonus here
			army->setStat(Army::STRENGTH, army->getStat(Army::STRENGTH, false)+1);
			// Emit signal
			snewMedalArmy.emit(army, 0);
		      }

		  debug("army being hit " <<  army->getNumberHasBeenHit())

		    // Gives the medal for good defense. The more negative the 
		    // number the more blows the unit evaded. n means roughly 
		    // avoid n hits from an equally strong unit. Since we want 
		    // to punish the case of the unit hiding among many others,
		    // we set this value quite high.
		    if((army->getNumberHasBeenHit() < -100) && !army->getMedalBonus(1))
		      {
			army->setMedalBonus(1,true);
			// We must recalculate the XPValue of this unit since it 
			// got a medal
			army->setXpReward(army->getXpReward()+1);
			// We get the medal bonus here
			army->setStat(Army::STRENGTH, army->getStat(Army::STRENGTH, false)+1);
			// Emit signal
			snewMedalArmy.emit(army, 1);
		      }
		  debug("Army hits " <<  army->getNumberHasHit())

		    for(int i=0;i<3;i++)
		      {
			debug("MEDAL[" << i << "]==" << army->getMedalBonus(i))
		      }
		}

	    // We reset the hit values after the battle
	    army->setNumberHasHit(0);
	    army->setNumberHasBeenHit(0);

	    if (army->isHero() && getType() != Player::NETWORKED)
	      {
		Hero *h = dynamic_cast<Hero*>(army);
		while(h->canGainLevel())
		  {
		    // Units not associated to a player never raise levels.
		    if (h->getOwner() == Playerlist::getInstance()->getNeutral())
		      break;

		    //Here this for is to check if army must raise 2 or more 
		    //levels per time depending on the XP and level itself

		    h->getOwner()->heroGainsLevel(h);
		  }
		debug("Hero new XP=" << h->getXP())
	      }
	    ++sit;
	  }
      ++it;
    }
}

Hero* Player::doRecruitHero(HeroProto* hproto, City *city, int cost, int alliesCount, const ArmyProto *ally, StackReflist *stacks)
{
  Hero *newhero = new Hero(*hproto);
  newhero->setOwner(this);
  Stack *s = GameMap::getInstance()->addArmy(city, newhero);
  if (stacks)
    {
      if (stacks->contains(s->getId()) == false)
        stacks->addStack(s);
    }

  if (alliesCount > 0)
    {
      Reward_Allies::addAllies(this, city->getPos(), ally, alliesCount,
                               stacks);
      hero_arrives_with_allies.emit(alliesCount);
    }

  if (cost == 0)
    {
      // Initially give the first hero the player's standard.
      Glib::ustring name = String::ucompose(_("%1 Standard"), getName());
      Item *battle_standard = new Item (name, true, this);
      battle_standard->addBonus(Item::ADD1STACK);
      newhero->getBackpack()->addToBackpack(battle_standard, 0);
    }
  Character *c =
    HeroTemplates::getInstance ()->getCharacterById (hproto->getHeroId ());
  for (auto item_id : c->item_ids)
    {
      ItemProto *proto = (*Itemlist::getInstance ())[item_id];
      Item *item = new Item (*proto, item_id);
      if (proto->getBonus (ItemProto::PLANT_TO_VECTOR))
        {
          item->setPlantableOwnerId (d_id);
          item->setPlantableOriginalOwnerId (d_id);
          item->setPlanted (false);
        }
      newhero->getBackpack()->addToBackpack(item);
    }
  withdrawGold(cost);
  supdatingStack.emit(0);
  return newhero;
}

void Player::recruitHero(HeroProto* heroproto, City *city, int cost, int alliesCount, const ArmyProto *ally, StackReflist *stacks)
{
  //alright, we may have picked another sex for the hero.
  HeroProto *h;
  Glib::ustring name = heroproto->getName();
  Hero::Gender g = Hero::Gender(heroproto->getGender());
  h = HeroTemplates::getInstance()->getRandomHero(g, getId());
  h->setGender(g);
  h->setName(name);
  addAction(new Action_RecruitHero(h, city, cost, alliesCount, ally));

  Hero *hero = doRecruitHero(h, city, cost, alliesCount, ally, stacks);
  if (hero)
    addHistory(new History_HeroEmerges(hero, city));
}

void Player::doDeclareDiplomacy (DiplomaticState state, Player *player)
{
  if (Playerlist::getInstance()->getNeutral() == player)
    return;
  if (player == this)
    return;
  if (state == d_diplomatic_state[player->getId()])
    return;
  d_diplomatic_state[player->getId()] = state;
}

void Player::declareDiplomacy (DiplomaticState state, Player *player, bool treachery)
{
  doDeclareDiplomacy(state, player);

  addAction(new Action_DiplomacyState(player, state));

  switch (state)
    {
    case AT_PEACE:
      addHistory(new History_DiplomacyPeace(player));
      break;
    case AT_WAR_IN_FIELD:
      break;
    case AT_WAR:
      addHistory(new History_DiplomacyWar(player));
      break;
    }
  if (treachery)
    addHistory(new History_DiplomacyTreachery(player));
  // FIXME: update diplomatic scores? 
}

void Player::doProposeDiplomacy (DiplomaticProposal proposal, Player *player)
{
  if (GameScenarioOptions::s_diplomacy == false)
    return;
  if (Playerlist::getInstance()->getNeutral() == player)
    return;
  if (player == this)
    return;
  if (proposal == d_diplomatic_proposal[player->getId()])
    return;
  if (proposal == PROPOSE_PEACE)
    {
      Glib::ustring s =
        String::ucompose(_("Peace negotiated with %1."),player->getName());
      if (getDiplomaticState(player) == AT_PEACE ||
	  getDiplomaticProposal(player) == PROPOSE_PEACE)
	schangingStatus.emit(s);
    }
  else if (proposal == PROPOSE_WAR)
    {
      Glib::ustring s =
        String::ucompose(_("War declared with %1."), player->getName());
      if (getDiplomaticState(player) == AT_WAR ||
	  getDiplomaticProposal(player) == PROPOSE_WAR)
      schangingStatus.emit(s);
    }
  d_diplomatic_proposal[player->getId()] = proposal;
}

void Player::proposeDiplomacy (DiplomaticProposal proposal, Player *player)
{
  doProposeDiplomacy(proposal, player);

  addAction(new Action_DiplomacyProposal(player, proposal));

  // FIXME: update diplomatic scores? 
}

Player::DiplomaticState Player::negotiateDiplomacy (Player *player)
{
  DiplomaticState state = getDiplomaticState(player);
  DiplomaticProposal them = player->getDiplomaticProposal(this);
  DiplomaticProposal me = getDiplomaticProposal(player);
  DiplomaticProposal winning_proposal;

  /* Check if we both want the status quo. */
  if (me == NO_PROPOSAL && them == NO_PROPOSAL)
    return state;

  /* Okay, we both want a change from the status quo. */

  /* In the absense of a new proposal, the status quo is the proposal. */
  if (me == NO_PROPOSAL)
    {
      switch (state)
	{
	case AT_PEACE: me = PROPOSE_PEACE; break;
	case AT_WAR_IN_FIELD: me = PROPOSE_WAR_IN_FIELD; break;
	case AT_WAR: me = PROPOSE_WAR; break;
	}
    }
  if (them == NO_PROPOSAL)
    {
      switch (state)
	{
	case AT_PEACE: them = PROPOSE_PEACE; break;
	case AT_WAR_IN_FIELD: them = PROPOSE_WAR_IN_FIELD; break;
	case AT_WAR: them = PROPOSE_WAR; break;
	}
    }

  /* Check if we have agreement. */
  if (me == PROPOSE_PEACE && them == PROPOSE_PEACE)
    return AT_PEACE;
  else if (me == PROPOSE_WAR_IN_FIELD && them == PROPOSE_WAR_IN_FIELD)
    return AT_WAR_IN_FIELD;
  else if (me == PROPOSE_WAR && them == PROPOSE_WAR)
    return AT_WAR;

  /* Still we don't have an agreement.  
     Unfortunately the greater violence is the new diplomatic state. 
     Because there are two different proposals and the proposal with
     greater violence will be the new status quo, there can't 
     possibly be peace at this juncture.  */

  winning_proposal = me;
  if (them > me)
    winning_proposal = them;

  switch (winning_proposal)
    {
    case PROPOSE_WAR_IN_FIELD: return AT_WAR_IN_FIELD; break;
    case PROPOSE_WAR: return AT_WAR; break;
    default: return AT_PEACE; break; //impossible
    }

}

Player::DiplomaticState Player::getDiplomaticState (Player *player) const
{
  if (player == Playerlist::getInstance()->getNeutral())
    return AT_WAR;
  if (player == this)
    return AT_PEACE;
  return d_diplomatic_state[player->getId()];
}

Player::DiplomaticProposal Player::getDiplomaticProposal (Player *player) const
{
  if (player == Playerlist::getInstance()->getNeutral())
    return PROPOSE_WAR;
  if (player == this)
    return NO_PROPOSAL;
  return d_diplomatic_proposal[player->getId()];
}

guint32 Player::getDiplomaticScore (Player *player) const
{
  if (Playerlist::getInstance()->getNeutral() == player)
    return 8;
  return d_diplomatic_score[player->getId()];
}

void Player::alterDiplomaticRelationshipScore (Player *player, int amount)
{
  if (amount > 0)
    {
      if (d_diplomatic_score[player->getId()] + amount > DIPLOMACY_MAX_SCORE)
	d_diplomatic_score[player->getId()] = DIPLOMACY_MAX_SCORE;
      else
	d_diplomatic_score[player->getId()] += amount;
    }
  else if (amount < 0)
    {
      if ((guint32) (amount * -1) > d_diplomatic_score[player->getId()])
	d_diplomatic_score[player->getId()] = DIPLOMACY_MIN_SCORE;
      else
	d_diplomatic_score[player->getId()] += amount;
    }
}

void Player::improveDiplomaticRelationship (Player *player, guint32 amount)
{
  if (Playerlist::getInstance()->getNeutral() == player || player == this)
    return;

  alterDiplomaticRelationshipScore (player, amount);

  addAction(new Action_DiplomacyScore(player, amount));
}

void Player::deteriorateDiplomaticRelationship (Player *player, guint32 amount)
{
  if (Playerlist::getInstance()->getNeutral() == player || player == this)
    return;

  alterDiplomaticRelationshipScore (player, -amount);

  addAction(new Action_DiplomacyScore(player, -amount));
}

void Player::deteriorateDiplomaticRelationship (guint32 amount)
{
  for (auto it: *Playerlist::getInstance())
    {
      if (it->isDead())
	continue;
      if (Playerlist::getInstance()->getNeutral() == it)
	continue;
      if (it == this)
	continue;
      it->deteriorateDiplomaticRelationship (this, amount);
    }
}

void Player::improveDiplomaticRelationship (guint32 amount, Player *except)
{
  for (auto it: *Playerlist::getInstance())
    {
      if (it->isDead())
	continue;
      if (Playerlist::getInstance()->getNeutral() == it)
	continue;
      if (it == this)
	continue;
      if (except && it == except)
	continue;
      it->improveDiplomaticRelationship (this, amount);
    }
}

void Player::deteriorateAlliesRelationship(Player *player, guint32 amount,
					   Player::DiplomaticState state)
{
  for (auto it: *Playerlist::getInstance())
    {
      if (it->isDead())
	continue;
      if (Playerlist::getInstance()->getNeutral() == it)
	continue;
      if (it == this)
	continue;
      if (getDiplomaticState(it) == state)
	it->deteriorateDiplomaticRelationship (player, amount);
    }
}

void Player::improveAlliesRelationship(Player *player, guint32 amount,
				       Player::DiplomaticState state)
{
  for (auto it: *Playerlist::getInstance())
    {
      if (it->isDead())
	continue;
      if (Playerlist::getInstance()->getNeutral() == it)
	continue;
      if (it == this)
	continue;
      if (player->getDiplomaticState(it) == state)
	it->improveDiplomaticRelationship (this, amount);
    }
}

void Player::AI_maybeBuyScout(City *c)
{
  if (c->getBuildProduction() == false)
    return;
  bool one_turn_army_exists = false;
  //do we already have something that can be produced in one turn?
  for (unsigned int i = 0; i < c->getMaxNoOfProductionBases(); i++)
    {
      if (c->getArmytype(i) == -1)    // no production in this slot
        continue;

      const ArmyProdBase *proto = c->getProductionBase(i);
      if (proto->getProduction() == 1)
        {
          one_turn_army_exists = true;
          break;
        }
    }
  if (one_turn_army_exists == false)
    {
      int free_slot = c->getFreeSlot();
      if (free_slot == -1)
        free_slot = 0;
      ArmyProto *scout =
        Armysetlist::getInstance()->lookupWeakestQuickestArmy(getArmyset());
      cityBuyProduction(c, free_slot, scout->getId());
    }
}

bool Player::AI_maybeContinueQuest(Stack *s, Quest *quest, bool &completed_quest, bool &stack_died)
{
  bool stack_moved = false;
  Vector<int> quest_tile = AI_getQuestDestination(quest, s);

  if (quest_tile == Vector<int>(-1,-1))
    return false;

  //are we not standing on it?
  if (s->getPos() != quest_tile)
    {
      //can we really reach it?
      Vector<int> old_dest(-1,-1);
      if (s->getPath()->size())
	old_dest = s->getLastPointInPath();
      guint32 moves = 0, turns = 0, left = 0;
      s->getPath()->calculate(s, quest_tile, moves, turns, left);
      bool go_there = computerChooseContinueQuest(s, quest, quest_tile, moves,
                                                  turns);
      if (!go_there)
        {
          s->clearPath();
	  if (old_dest != Vector<int>(-1,-1))
	    s->getPath()->calculate(s, old_dest);
          return false;
        }
      d_stacklist->setActivestack(s);
      stack_moved = stackMove(s);
      //maybe we died either en route or at our destination.
      if (!d_stacklist->getActivestack())
	{
	  stack_died = true;
	  return true;
	}
      s = d_stacklist->getActivestack();
    }

  //are we standing on it now?
  if (s->getPos() == quest_tile)
    completed_quest = true;

  return stack_moved;
}

bool Player::AI_maybePickUpItems(Stack *s, int max_dist,
				 bool &picked_up, bool &stack_died)
{
  int min_dist = -1;
  bool stack_moved = false;
  Vector<int> item_tile(-1, -1);

  // do we not have a hero?
  if (s->hasHero() == false)
    return false;

  //ok, which bag of stuff is closest?
  for (auto tile: GameMap::getInstance()->getItems())
    {
      //don't consider bags of stuff that are inside enemy cities
      City *c = GameMap::getCity(tile);
      if (c)
	{
	  if (c->getOwner() != s->getOwner())
	    continue;
	}

      int distance = dist (tile, s->getPos());
      if (distance < min_dist || min_dist == -1)
	{
	  min_dist = distance;
	  item_tile = tile;
	}
    }

  //if no bags of stuff, or the bag is too far away
  if (min_dist == -1 || min_dist > max_dist)
    return false;

  //are we not standing on it?
  if (s->getPos() != item_tile)
    {
      //can we really reach it?
      Vector<int> old_dest(-1,-1);
      if (s->getPath()->size())
	old_dest = s->getLastPointInPath();
      guint32 moves = 0, turns = 0, left = 0;
      s->getPath()->calculate(s, item_tile, moves, turns, left);
      bool go_there = computerChoosePickupBag(s, item_tile, moves, turns);
      if (!go_there)
        {
          s->clearPath();
	  if (old_dest != Vector<int>(-1,-1))
	    s->getPath()->calculate(s, old_dest);
          return false;
        }
      d_stacklist->setActivestack(s);
      stack_moved = stackMove(s);
      //maybe we died -- an enemy stack was guarding the bag.
      if (!d_stacklist->getActivestack())
	{
	  stack_died = true;
	  return true;
	}
      s = d_stacklist->getActivestack();
    }

  //are we standing on it now?
  if (s->getPos() == item_tile)
    {
      bool pickitup = computerChoosePickupBag(s, item_tile, 0, 0);
      if (!pickitup)
        {
          s->clearPath();
          return stack_moved;
        }
      Hero *hero = static_cast<Hero*>(s->getFirstHero());
      if (hero)
	picked_up = heroPickupAllItems(hero, s->getPos());
    }

  return stack_moved;
}

bool Player::AI_maybeVisitTempleForQuest(Stack *s, int dist, bool &got_quest, bool &stack_died)
{
  bool stack_moved = false;

  //if this stack doesn't have a hero then we can't get a quest with this stack.
  if (s->hasHero() == false)
    return false;

  //if the player already has a hero who has a quest, then we can't get a
  //quest with this stack when playing one quest per player.
  if (QuestsManager::getInstance()->getPlayerQuests(this).size() > 0 &&
      GameScenarioOptions::s_play_with_quests ==
      GameParameters::ONE_QUEST_PER_PLAYER)
    return false;

  Temple *temple =
    Templelist::getInstance()->getNearestVisibleTemple(s->getPos(), dist);
  if (!temple)
    return false;

  //if we're not there yet
  if (temple->contains(s->getPos()) == false)
    {
      //can we really reach it?
      Vector<int> old_dest(-1,-1);
      if (s->getPath()->size())
	old_dest = s->getLastPointInPath();
      guint32 moves = 0, turns = 0, left = 0;
      s->getPath()->calculate(s, s->getPos(), moves, turns, left);
      bool go_there = computerChooseVisitTempleForQuest(s, temple->getPos(), moves, turns);
      if (!go_there)
        {
          s->clearPath();
	  if (old_dest != Vector<int>(-1,-1))
	    s->getPath()->calculate(s, old_dest);
          return false;
        }
      d_stacklist->setActivestack(s);
      stack_moved = stackMove(s);

      //maybe we died -- an enemy stack was guarding the temple
      if (!d_stacklist->getActivestack())
	{
	  stack_died = true;
	  return true;
	}
      s = d_stacklist->getActivestack();
    }

  //are we there yet?
  if (temple->contains(s->getPos()) == true && GameMap::can_search(s))
    {
      bool searchit = computerChooseVisitTempleForQuest(s, s->getPos(), 0, 0);
      if (!searchit)
        {
          s->clearPath();
          return stack_moved;
        }
      svisitingTemple.emit(temple, s);
      got_quest = true;
    }

  return stack_moved;
}

bool Player::AI_maybeVisitRuin(Stack *s, int dist, bool &visited_ruin, bool &stack_died)
{
  bool stack_moved = false;

  //if this stack doesn't have a hero then we can't search the ruin.
  if (s->hasHero() == false)
    return false;

  Ruin *ruin = Ruinlist::getInstance()->getNearestUnsearchedRuin(s->getPos(), dist);
  if (!ruin)
    return false;

  //if we're not there yet
  if (ruin->contains(s->getPos()) == false)
    {
      //can we really reach it?
      Vector<int> old_dest(-1,-1);
      if (s->getPath()->size())
	old_dest = s->getLastPointInPath();
      guint32 moves = 0, turns = 0, left = 0;
      s->getPath()->calculate(s, ruin->getPos(), moves, turns, left);
      bool go_there = computerChooseVisitRuin(s, ruin->getPos(), moves, turns);
      if (!go_there)
        {
          s->clearPath();
	  if (old_dest != Vector<int>(-1,-1))
	    s->getPath()->calculate(s, old_dest);
          return false;
        }
      d_stacklist->setActivestack(s);
      stack_moved = stackMove(s);

      //maybe we died -- an enemy stack was guarding the temple
      if (!d_stacklist->getActivestack())
	{
	  stack_died = true;
	  return true;
	}
      s = d_stacklist->getActivestack();
    }

  //are we there yet?
  if (ruin->contains(s->getPos()) == true && GameMap::can_search(s))
    {
      bool searchit = computerChooseVisitRuin(s, s->getPos(), 0, 0);
      if (!searchit)
        {
          s->clearPath();
          return stack_moved;
        }
      stack_died = ssearchingRuin.emit(ruin, s);
      if (!stack_died)
        visited_ruin = true;
    }

  return stack_moved;
}

bool Player::AI_maybeVisitTempleForBlessing(Stack *s, int dist,
					    double percent_can_be_blessed,
					    bool &blessed, bool &stack_died)
{
  bool stack_moved = false;

  Temple *temple = Templelist::getInstance()->getNearestVisibleAndUsefulTemple
    (s, percent_can_be_blessed, dist);
  if (!temple)
    return false;

  //if we're not there yet
  if (s->getPos() != temple->getPos())
    {
      //can we really reach it?
      Vector<int> old_dest(-1,-1);
      if (s->getPath()->size())
	old_dest = s->getLastPointInPath();
      guint32 moves = 0, turns = 0, left = 0;
      s->getPath()->calculate(s, temple->getPos(), moves, turns, left);
      bool go_there = computerChooseVisitTempleForBlessing(s, temple->getPos(), moves, turns);
      if (!go_there)
        {
          s->clearPath();
	  if (old_dest != Vector<int>(-1,-1))
	    s->getPath()->calculate(s, old_dest);
          return false;
        }
      d_stacklist->setActivestack(s);
      stack_moved = stackMove(s);

      //maybe we died -- an enemy stack was guarding the temple
      if (!d_stacklist->getActivestack())
	{
	  stack_died = true;
	  return true;
	}
      s = d_stacklist->getActivestack();
    }

  int num_blessed = 0;
  //are we there yet?
  if (temple->contains(s->getPos()) == true && GameMap::can_search(s))
    {
      bool searchit = computerChooseVisitTempleForBlessing(s, s->getPos(),
                                                           0, 0);
      if (!searchit)
        {
          s->clearPath();
          return stack_moved;
        }
      num_blessed = stackVisitTemple(s, temple);
    }

  blessed = num_blessed > 0;
  return stack_moved;
}

bool Player::safeFromAttack(City *c, guint32 safe_mp, guint32 min_defenders)
{
  //if there isn't an enemy city nearby to the source
  // calculate mp to nearest enemy city
  //   needs to be less than 18 mp with a scout
  //does the source city contain at least 3 defenders?

  City *enemy_city = Citylist::getInstance()->getNearestEnemyCity(c->getPos());
  if (enemy_city)
    {
      PathCalculator pc(c->getOwner(), c->getPos());
      int mp = pc.calculate(enemy_city->getPos());
      if (mp <= 0 || mp >= (int)safe_mp)
	{
	  if (c->countDefenders() >= min_defenders)
	    return true;
	}
    }

  return false;
}

bool Player::AI_maybeDisband(Stack *s, int safe_mp, bool &stack_killed)
{
  bool disbanded = false;
  //see if we're near to enemy stacks
  PathCalculator pc(s);
  if (GameMap::getEnemyStacks(pc.getReachablePositions(safe_mp)).size() > 0)
    return false;

  //upgroup the whole stack if it doesn't contain a hero
  if (s->hasHero() == false)
    {
      stack_killed = stackDisband (s);
      return stack_killed;
    }

  //ungroup the lucky ones not being disbanded
  for (Stack::reverse_iterator i = s->rbegin(); i != s->rend(); ++i)
    {
      if ((*i)->isHero() == false)
	{
	  Stack *new_stack = stackSplitArmy(s, *i);
	  if (new_stack)
	    {
	    if (stackDisband(new_stack))
	      disbanded = true;
	    }
	}
    }
  return disbanded;
}

bool Player::AI_maybeDisband(Stack *s, City *city, guint32 min_defenders,
			     int safe_mp, bool &stack_killed)
{
  bool disbanded = false;
  //is the city in danger from a city?
  if (safeFromAttack(city, safe_mp, 0) == false)
    return false;

  if (city->countDefenders() - s->size() >= min_defenders)
    {
      if (s->hasHero())
	min_defenders = s->size() + 1;
      else
	{
	  stack_killed = stackDisband(s);
	  return stack_killed;
	}
    }

  //okay, we need to disband part of our stack

  //before we move, ungroup the lucky ones not being disbanded
  unsigned int count = 0;
  for (Stack::reverse_iterator i = s->rbegin(); i != s->rend(); ++i)
    {
      if (count == min_defenders)
	break;
      if ((*i)->isHero() == false)
	{
	  Stack *new_stack = stackSplitArmy(s, *i);
	  if (new_stack)
	    {
	      count++;
	      if (stackDisband(new_stack))
		disbanded = true;
	    }
	}
    }
  return disbanded;
}

bool Player::AI_maybeVector(City *c, guint32 safe_mp, guint32 min_defenders,
			    City *target, City **vector_city)
{
  assert (c->getOwner() == this);
  if (vector_city)
    *vector_city = NULL;

  //is this city producing anything that we can vector?
  if (c->getActiveProductionSlot() == -1)
    return false;

  //is it safe to vector from this city?
  bool safe = safeFromAttack(c, safe_mp, min_defenders);

  if (!safe)
    return false;

  //get the nearest city to the enemy city that can accept vectored units
  City *near_city =
    Citylist::getInstance()->getNearestFriendlyVectorableCity(target->getPos());
  if (!near_city)
    return false;
  assert (near_city->getOwner() == this);
  if (GameMap::getCity(near_city->getPos()) != near_city)
    {
      printf("nearCity is %s (%d)\n", near_city->getName().c_str(), near_city->getId());
      printf("it is located at %d,%d\n", near_city->getPos().x, near_city->getPos().y);
      City *other = GameMap::getCity(near_city->getPos());
      if (other)
	{
      printf("the OTHER nearCity is %s (%d)\n", other->getName().c_str(), other->getId());
      printf("it is located at %d,%d\n", other->getPos().x, other->getPos().y);
	}
      else
	printf("no city there!\n");
      assert (1 == 0);
    }

  //if it's us then it's easier to just walk.
  if (near_city == c)
    return false;

  //is that city already vectoring?
  if (near_city->getVectoring() != Vector<int>(-1, -1))
    return false;

  //can i just walk there faster?

  //find turns from source to target city
  const ArmyProdBase *proto = c->getActiveProductionBase();
  PathCalculator pc1(c->getOwner(), c->getPos(), proto);
  guint32 moves1 = 0, turns1 = 0, left1 = 0;
  guint32 moves2 = 0, turns2 = 0, left2 = 0;
  Path *p = pc1.calculate(target->getPos(), moves1, turns1, left1);
  if (p)
    delete p;

  //find turns from nearer vectorable city to target city
  PathCalculator pc2(c->getOwner(), near_city->getPos(), proto);
  p = pc2.calculate(target->getPos(), moves2, turns2, left2);
  if (p)
    delete p;
  turns2+=VectoredUnit::get_travel_turns(near_city->getPos(), target->getPos());
  if (turns1 <= turns2)
    return false;

  //great.  now do the vectoring.
  c->changeVectorDestination(near_city->getPos());

  if (vector_city)
    *vector_city = near_city;
  return true;
}

void Player::AI_setupVectoring(guint32 safe_mp, guint32 min_defenders,
			       guint32 mp_to_front)
{
  //turn off vectoring where it isn't safe anymore
  //turn off vectoring for destinations that are far away from the
  //nearest enemy city

  for (auto c: *Citylist::getInstance())
    {
      if (c->getOwner() != this || c->isBurnt())
	continue;
      Vector<int> dest = c->getVectoring();
      if (dest == Vector<int>(-1, -1))
	continue;
      if (safeFromAttack(c, safe_mp, min_defenders) == false)
	{
	  //City *target_city = Citylist::getInstance()->getObjectAt(dest);
	  //debug("stopping vectoring from " << c->getName() <<" to " << target_city->getName() << " because it's not safe to anymore!\n")
	  c->setVectoring(Vector<int>(-1,-1));
	  continue;
	}

      City *enemy_city = Citylist::getInstance()->getNearestEnemyCity(dest);
      if (!enemy_city)
	{
	  //City *target_city = Citylist::getInstance()->getObjectAt(dest);
	  //debug("stopping vectoring from " << c->getName() <<" to " << target_city->getName() << " because there aren't any more enemy cities!\n")
	  c->setVectoring(Vector<int>(-1,-1));
	  continue;
	}

      PathCalculator pc(this, dest, NULL);
      int mp = pc.calculate(enemy_city->getPos());
      if (mp <= 0 || mp > (int)mp_to_front)
	{

	  //City *target_city = Citylist::getInstance()->getObjectAt(dest);
	  //debug("stopping vectoring from " << c->getName() <<" to " << target_city->getName() << " because it's too far away from an enemy city!\n")
	  c->setVectoring(Vector<int>(-1,-1));
	  continue;
	}
    }

  for (auto c : *Citylist::getInstance())
    {
      if (c->getOwner() != this || c->isBurnt())
	continue;
      City *enemy_city = Citylist::getInstance()->getNearestEnemyCity(c->getPos());
      if (!enemy_city)
	continue;
      City *vector_city = NULL;
      //if the city isn't already vectoring
      if (c->getVectoring() == Vector<int>(-1,-1))
	{
	  bool vectored = AI_maybeVector(c, safe_mp, min_defenders, enemy_city,
					 &vector_city);
	  if (vectored)
            {
              debug("begin vectoring from " << c->getName() <<" to " << vector_city->getName() << "!\n");
            }
	}
    }
}

const Army * Player::doCityProducesArmy(City *city, Stack *& s, bool &vectored)
{
  vectored = false;
  int cost = city->getActiveProductionBase()->getProductionCost();
  if (cost > d_gold)
    return NULL;
  withdrawGold(cost);
  const Army *a = city->armyArrives(s);
  if (city->getVectoring() != Vector<int>(-1,-1))
    vectored = true;
  return a;
}

bool Player::cityProducesArmy(City *city)
{
  assert(city->getOwner() == this);
  Stack *stack = NULL;
  bool vectored = false;
  const Army *army = doCityProducesArmy(city, stack, vectored);
  if (army)
    {
      if (!stack)
        {
          printf("we dropped an army down but it doesn't have a stack!\n");
          return false;
        }
      const ArmyProdBase *source_army;
      source_army = city->getProductionBaseBelongingTo(army);
      if (stack)
        {
          addAction(new Action_Produce(source_army, city, false, stack->getPos(), army->getId(), stack->getId()));
          addAction(new Action_ReorderArmies(stack));
        }
    }
  else
    {
      if (vectored)
        {
          //send vectoring action.
          const ArmyProdBase *source_army = city->getActiveProductionBase();
          addAction(new Action_Produce(source_army, city, true,
                                       city->getVectoring(), 0, 0));
        }
    }
  return true;
}

Army* Player::doVectoredUnitArrives(VectoredUnit *unit, Stack *& s)
{
  Army *army = unit->armyArrives(s);
  return army;
}

bool Player::vectoredUnitArrives(VectoredUnit *unit)
{
  Stack *stack = NULL;
  Army *army = doVectoredUnitArrives(unit, stack);
  if (!army)
    {
      printf("this was supposed to be impossible because of operations on the vectoredunitlist after the city is conquered.\n");
      printf("whooops... this vectored unit failed to show up.\n");
      City *dest = GameMap::getCity(unit->getDestination());
      printf("the unit was being vectored to: %s, from %s by %s\n",
             dest->getName().c_str(),
             GameMap::getCity(unit->getPos())->getName().c_str(), getName().c_str());
      printf("Army is a %s, turns is %d + 1\n", unit->getArmy()->getName().c_str(), unit->getArmy()->getProduction());


      int turn = -1;
      std::list<History*> h = dest->getOwner()->getHistoryForCityId(dest->getId());
      std::list<History*>::const_iterator pit;
      for (pit = h.begin(); pit != h.end(); ++pit)
        {
          switch ((*pit)->getType())
            {
            case History::START_TURN:
                {
                  turn++;
                  break;
                }
            case History::CITY_WON:
              break;
            case History::CITY_RAZED:
              break;
            default:
              break;
            }
        }
      printf("was the destination city owned by us way back then?\n");
      exit (1);
    }
  else
    addAction(new Action_ProduceVectored(unit->getArmy(),
                                         unit->getDestination(),
                                         unit->getPos(), army->getId(),
                                         stack->getId()));

  return true;
}

std::list<Action_Produce *> Player::getUnitsProducedThisTurn() const
{
  std::list<Action_Produce *> actions;
  for (std::list<Action *>::const_reverse_iterator it = d_actions.rbegin();
       it != d_actions.rend(); ++it)
    {
      if ((*it)->getType() == Action::PRODUCE_UNIT)
	actions.push_back(dynamic_cast<Action_Produce*>(*it));
      else if ((*it)->getType() == Action::INIT_TURN)
	break;
    }
  return actions;
}

std::list<Action *> Player::getReportableActions() const
{
  std::list<Action *> actions;
  for (auto it: d_actions)
    {
      if (it->getType() == Action::PRODUCE_UNIT ||
	  it->getType() == Action::PRODUCE_VECTORED_UNIT ||
	  it->getType() == Action::CITY_DESTITUTE)
	actions.push_back(it);
    }
  return actions;
}

void Player::cityTooPoorToProduce(City *city, int slot)
{
  cityChangeProduction(city, -1);
  const ArmyProdBase *a = city->getProductionBase(slot);
  addAction(new Action_CityTooPoorToProduce(city, a));
}

void Player::pruneActionlist()
{
  pruneActionlist(d_actions);
}

void Player::pruneCityProductions(std::list<Action*> &actions)
{
  //remove duplicate city production actions

  //enumerate the ones we want
  std::list<Action_Production*> keepers;
  for ( std::list<Action*>::reverse_iterator ait = actions.rbegin();
        ait != actions.rend(); ++ait)
    {
      if ((*ait)->getType() != Action::CITY_PROD)
	continue;
      //if this city isn't already in the keepers list, then add it.

      Action_Production *action = static_cast<Action_Production*>(*ait);
      bool found = false;
      for (std::list<Action_Production*>::const_iterator it = keepers.begin();
           it != keepers.end(); ++it)
	{
	  if (action->getCityId() == (*it)->getCityId())
	    {
	      found = true;
	      break;
	    }
	}
      if (found == false)
	keepers.push_back(action);

    }

  //now delete all city production events that aren't in keepers
  int total = 0;
  for (std::list<Action*>::iterator bit = actions.begin();
       bit != actions.end(); ++bit)
    {
      if ((*bit)->getType() != Action::CITY_PROD)
	continue;
      if (find (keepers.begin(), keepers.end(), (*bit)) == keepers.end())
	{
	  total++;
          delete *bit;
	  actions.erase (bit);
	  bit = actions.begin();
	  continue;
	}
    }
}

void Player::pruneCityVectorings(std::list<Action*> &actions)
{
  //remove duplicate city vectoring actions

  //enumerate the ones we want
  std::list<Action_Vector*> keepers;
  for (std::list<Action*>::reverse_iterator ait = actions.rbegin();
       ait != actions.rend(); ++ait)
    {
      if ((*ait)->getType() != Action::CITY_VECTOR)
	continue;
      //if this city isn't already in the keepers list, then add it.

      Action_Vector *action = static_cast<Action_Vector *>(*ait);
      bool found = false;
      for (std::list<Action_Vector*>::const_iterator it = keepers.begin();
           it != keepers.end(); ++it)
	{
	  if (action->getCityId() == (*it)->getCityId())
	    {
	      found = true;
	      break;
	    }
	}
      if (found == false)
	keepers.push_back(action);

    }

  //now delete all city vector events that aren't in keepers
  int total = 0;
  for (std::list<Action*>::iterator bit = actions.begin();
       bit != actions.end(); ++bit)
    {
      if ((*bit)->getType() != Action::CITY_VECTOR)
	continue;
      if (find (keepers.begin(), keepers.end(), (*bit)) == keepers.end())
	{
	  total++;
          delete *bit;
	  actions.erase (bit);
	  bit = actions.begin();
	  continue;
	}
    }
}

void Player::pruneActionlist(std::list<Action*> &actions)
{
  pruneCityProductions(actions);
  pruneCityVectorings(actions);

}

Glib::ustring Player::playerTypeToString(const Player::Type type)
{
  switch (type)
    {
    case Player::HUMAN: return "Player::HUMAN";
    case Player::AI_FAST: return "Player::AI_FAST";
    case Player::AI_DUMMY: return "Player::AI_DUMMY";
    case Player::AI_SMART: return "Player::AI_SMART";
    case Player::NETWORKED: return "Player::NETWORKED";
    }
  return "Player::HUMAN";
}

Player::Type Player::playerTypeFromString(const Glib::ustring str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return Player::Type(atoi(str.c_str()));
  if (str == "Player::HUMAN") return Player::HUMAN;
  else if (str == "Player::AI_FAST") return Player::AI_FAST;
  else if (str == "Player::AI_DUMMY") return Player::AI_DUMMY;
  else if (str == "Player::AI_SMART") return Player::AI_SMART;
  else if (str == "Player::NETWORKED") return Player::NETWORKED;
  return Player::HUMAN;
}

bool Player::hasAlreadyInitializedTurn() const
{
  for (auto it: d_actions)
    if (it->getType() == Action::INIT_TURN)
      return true;
  return false;
}

bool Player::hasAlreadyCollectedTaxesAndPaidUpkeep() const
{
  for (auto it: d_actions)
    if (it->getType() == Action::COLLECT_TAXES_AND_PAY_UPKEEP)
      return true;
  return false;
}

bool Player::hasAlreadyEndedTurn() const
{
  for (auto it: d_actions)
    if (it->getType() == Action::END_TURN)
      return true;
  return false;
}

guint32 Player::countEndTurnHistoryEntries() const
{
  guint32 count = 0;
  for (std::list<History*>::const_iterator it = d_history.begin();
       it != d_history.end(); ++it)
    {
      if ((*it)->getType() == History::END_TURN)
	count++;
    }
  return count;
}

bool Player::searchedRuin(Ruin *r) const
{
  if (!r)
    return false;
  for (std::list<History*>::const_iterator it = d_history.begin();
       it != d_history.end(); ++it)
    {
      if ((*it)->getType() == History::HERO_RUIN_EXPLORED)
	{
	  History_HeroRuinExplored *event =
            dynamic_cast<History_HeroRuinExplored*>(*it);
	  if (event->getRuinId() == r->getId())
	    return true;
	}
    }    
  return false;
}

bool Player::conqueredCity(City *c, guint32 &turns_ago) const
{
  if (!c)
    return false;
  for (std::list<History*>::const_reverse_iterator it = d_history.rbegin();
       it != d_history.rend(); ++it)
    {
      if ((*it)->getType() == History::CITY_WON)
	{
	  History_CityWon *event = dynamic_cast<History_CityWon*>(*it);
	  if (event->getCityId() == c->getId())
	    return true;
	}
      else if ((*it)->getType() == History::START_TURN)
        turns_ago++;

    }    
  return false;
}

std::list<Vector<int> > Player::getStackTrack(Stack *s) const
{
  std::list<Vector<int> > points;
  Vector<int> delta = Vector<int>(0,0);
  for (auto it: d_actions)
    {
      if (it->getType() == Action::STACK_MOVE)
	{
	  Action_Move *action = dynamic_cast<Action_Move*>(it);
	  if (action->getStackId() == s->getId())
	    {
	      if (points.size() == 0)
		delta = action->getPositionDelta();
	      points.push_back(action->getEndingPosition());
	    }
	}
    }
  if (points.size() >= 1)
    {
      Vector<int> pos = points.front() - delta;
      if (pos != points.front())
	points.push_front(pos);
    }
  return points;
}
	
std::list<History *>Player::getHistoryForCityId(guint32 id) const
{
  std::list<History*> events;
  
  for (std::list<History*>::const_iterator pit = d_history.begin();
       pit != d_history.end(); ++pit)
    {
      switch ((*pit)->getType())
	{
	case History::START_TURN:
	    {
	      events.push_back(*pit);
	      break;
	    }
	case History::CITY_WON:
	    {
	      History_CityWon *event;
	      event = dynamic_cast<History_CityWon*>(*pit);
	      if (event->getCityId() == id)
		events.push_back(*pit);
	      break;
	    }
	case History::CITY_RAZED:
	    {
	      History_CityRazed *event;
	      event = dynamic_cast<History_CityRazed*>(*pit);
	      if (event->getCityId() == id)
		events.push_back(*pit);
	      break;
	    }
	default:
	  break;
	}
    }
  return events;
}

std::list<History *>Player::getHistoryForHeroId(guint32 id) const
{
  Glib::ustring hero_name = "";
  std::list<History*> events;
  for (std::list<History*>::const_iterator pit = d_history.begin();
       pit != d_history.end(); ++pit)
    {
      switch ((*pit)->getType())
	{
	case History::HERO_EMERGES:
	    {
	      History_HeroEmerges *event;
	      event = dynamic_cast<History_HeroEmerges *>(*pit);
	      if (event->getHeroId() == id)
		{
		  hero_name = event->getHeroName();
		  events.push_back(*pit);
		}
	      break;
	    }
	case History::FOUND_SAGE:
	    {
	      History_FoundSage *event;
	      event = dynamic_cast<History_FoundSage*>(*pit);
	      if (event->getHeroName() == hero_name)
		events.push_back(*pit);
	      break;
	    }
	case History::HERO_QUEST_STARTED:
	    {
	      History_HeroQuestStarted *event;
	      event = dynamic_cast<History_HeroQuestStarted*>(*pit);
	      if (event->getHeroName() == hero_name)
		events.push_back(*pit);
	      break;
	    }
	case History::HERO_QUEST_COMPLETED:
	    {
	      History_HeroQuestCompleted *event;
	      event = dynamic_cast<History_HeroQuestCompleted*>(*pit);
	      if (event->getHeroName() == hero_name)
		events.push_back(*pit);
	      break;
	    }
	case History::HERO_KILLED_IN_CITY:
	    {
	      History_HeroKilledInCity *event;
	      event = dynamic_cast<History_HeroKilledInCity*>(*pit);
	      if (event->getHeroName() == hero_name)
		events.push_back(*pit);
	      break;
	    }
	case History::HERO_KILLED_IN_BATTLE:
	    {
	      History_HeroKilledInBattle *event;
	      event = dynamic_cast<History_HeroKilledInBattle*>(*pit);
	      if (event->getHeroName() == hero_name)
		events.push_back(*pit);
	      break;
	    }
	case History::HERO_KILLED_SEARCHING:
	    {
	      History_HeroKilledSearching*event;
	      event = dynamic_cast<History_HeroKilledSearching*>(*pit);
	      if (event->getHeroName() == hero_name)
		events.push_back(*pit);
	      break;
	    }
	case History::HERO_CITY_WON:
	    {
	      History_HeroCityWon *event;
	      event = dynamic_cast<History_HeroCityWon*>(*pit);
	      if (event->getHeroName() == hero_name)
		events.push_back(*pit);
	      break;
	    }
	case History::HERO_FINDS_ALLIES:
	    {
	      History_HeroFindsAllies *event;
	      event = dynamic_cast<History_HeroFindsAllies*>(*pit);
	      if (event->getHeroName() == hero_name)
		events.push_back(*pit);
	      break;
	    }
	default:
	  break;
	}
    }
  return events;
}

void Player::setSurrendered(bool surr)
{
  surrendered = surr;
}

std::list<Hero*> Player::getHeroes() const
{
  return d_stacklist->getHeroes();
}

guint32 Player::countArmies() const
{
  return d_stacklist->countArmies();
}

Stack * Player::getActivestack() const
{
  return d_stacklist->getActivestack();
}

void Player::setActivestack(Stack *s)
{
  d_stacklist->setActivestack(s);
}
	
Vector<int> Player::getPositionOfArmyById(guint32 id) const
{
  return d_stacklist->getPosition(id);
}

void Player::immobilize()
{
  d_stacklist->drainAllMovement();
}

void Player::clearStacklist()
{
  d_stacklist->flClear();
}

void Player::clearFogMap()
{
  d_fogmap->fill(FogMap::OPEN);
}

std::list<Action *> Player::getActionsThisTurn(int type) const
{
  std::list<Action *> actions;
  for (auto it: d_actions)
    if (it->getType() == Action::Type(type))
      actions.push_back(it);
  return actions;
}

std::list<Action *> Player::getMovesThisTurn() const
{
  return getActionsThisTurn(Action::STACK_MOVE);
}

int Player::countDestituteCitiesThisTurn() const
{
  return getActionsThisTurn(Action::CITY_DESTITUTE).size();
}

Vector<int> Player::AI_getQuestDestination(Quest *quest, Stack *stack) const
{
  Vector<int> dest = Vector<int>(-1,-1);
  switch (quest->getType())
    {
    case Quest::KILLHERO:
        {
          QuestKillHero *q = dynamic_cast<QuestKillHero*>(quest);
          guint32 hero_id = q->getVictim();
          Stack *enemy = NULL;
          for (auto it: *Playerlist::getInstance())
            {
              if (it == this)
                continue;
              enemy = it->getStacklist()->getArmyStackById(hero_id);
              if(enemy)
                break;
            }
          if (enemy)
            dest = enemy->getPos();

        }
      break;
    case Quest::KILLARMYTYPE:
        {
          QuestEnemyArmytype *q = dynamic_cast<QuestEnemyArmytype*>(quest);
          guint32 army_type = q->getArmytypeToKill();
          std::vector<Stack*> s =
            GameMap::getNearbyEnemyStacks(stack->getPos(), GameMap::getWidth());
          for (std::vector<Stack*>::iterator i = s.begin(); i != s.end(); ++i)
            {
              if ((*i)->hasArmyType(army_type) == true)
                {
                  dest = (*i)->getPos();
                  break;
                }
            }
        }
      break;
    case Quest::KILLARMIES:
        {
          QuestEnemyArmies *q = dynamic_cast<QuestEnemyArmies*>(quest);
          auto enemy = Playerlist::getInstance()->getPlayer(q->getVictimPlayerId());
          auto s = GameMap::getNearbyEnemyStacks(stack->getPos(), GameMap::getWidth());
          for (std::vector<Stack*>::iterator i = s.begin(); i != s.end(); ++i)
            {
              if ((*i)->getOwner() != enemy)
                continue;
              dest = (*i)->getPos();
            }
        }
      break;

    case Quest::PILLAGEGOLD:
    case Quest::CITYSACK:
    case Quest::CITYRAZE:
    case Quest::CITYOCCUPY:
      //attack the nearest enemy city.
        {
          City *c = Citylist::getInstance()->getClosestEnemyCity(stack);
          if (c)
            dest = c->getNearestPos(stack->getPos());
        }
      break;
    }
  return dest;
}

bool Player::AI_invadeCityQuestPreference(City *c, CityDefeatedAction &action) const
{
  bool found = false;
  std::vector<Quest*> q = QuestsManager::getInstance()->getPlayerQuests(this);
  for (std::vector<Quest*>::iterator i = q.begin(); i != q.end(); ++i)
    {
      if (*i == NULL)
        continue;
      switch ((*i)->getType())
        {
        case Quest::CITYOCCUPY:
            {
              QuestCityOccupy* qu = dynamic_cast<QuestCityOccupy*>(*i);
              if (qu->getCityId() == c->getId())
                {
                  action = CITY_DEFEATED_OCCUPY;
                  found = true;
                }
            }
          break;
        case Quest::CITYSACK:
            {
              QuestCitySack * qu = dynamic_cast<QuestCitySack*>(*i);
              if (qu->getCityId() == c->getId())
                {
                  action = CITY_DEFEATED_SACK;
                  found = true;
                }
            }
          break;
        case Quest::CITYRAZE:
            {
              QuestCityRaze* qu = dynamic_cast<QuestCityRaze*>(*i);
              if (qu->getCityId() == c->getId())
                {
                  action = CITY_DEFEATED_RAZE;
                  found = true;
                }
            }
          break;
        case Quest::PILLAGEGOLD:
          action = CITY_DEFEATED_SACK;
          found = true;
          break;
        }
    }
  return found;
}

/*
 *
 * what are the chances of a hero showing up?
 *
 * 1 in 6 if you have enough gold, where "enough gold" is...
 *
 * ... 1500 if the player already has a hero, then:  1500 is generally 
 * enough to buy all the heroes.  I forget the exact distribution of 
 * hero prices but memory says from 1000 to 1500.  (But, if you don't 
 * have 1500 gold, and the price is less, you still get the offer...  
 * So, calculate price, compare to available gold, then decided whether 
 * or not to offer...)
 *
 * ...500 if all your heroes are dead: then prices are cut by about 
 * a factor of 3.
 */
bool Player::maybeRecruitHero ()
{
  bool accepted = false;
  
  City *city = NULL;
  int gold_needed = 0;
  if (Citylist::getInstance()->countCities(this) == 0)
    return false;
  //give the player a hero if it's the first round.
  //otherwise we get a hero based on chance
  //a hero costs a random number of gold pieces
  if (GameScenarioOptions::s_round == 1 && getHeroes().size() == 0)
    gold_needed = 0;
  else
    {
      bool exists = false;
      if (getHeroes().size() > 0)
        exists = true;

      gold_needed = (Rnd::rand() % 500) + 1000;
      if (exists == false)
	gold_needed /= 2;
    }

  if ((((Rnd::rand() % 6) == 0 && gold_needed < getGold()) || gold_needed == 0))
    {
      HeroProto *heroproto =
        HeroTemplates::getInstance()->getRandomHero(getId());
      if (gold_needed == 0)
	{
	  //we do it this way because maybe quickstart is on.
          city = Citylist::getInstance()->getCapitalCity(this);
          if (!city || city->isBurnt() == true)
	    city = getFirstCity();
	}
      else
        city = Citylist::getInstance()->getRandomCityForHero(this);

      if (srecruitingHero.empty())
        accepted = true;
      else if (city)
        accepted = srecruitingHero.emit(heroproto, city, gold_needed);

      if (accepted) {
        /* now maybe add a few allies */
        int alliesCount;
        if (gold_needed > 1300)
          alliesCount = 3;
        else if (gold_needed > 1000)
          alliesCount = 2;
        else if (gold_needed > 800)
          alliesCount = 1;
        else
          alliesCount = 0;

        const ArmyProto *ally = 0;
        if (alliesCount > 0)
        {
          ally = Reward_Allies::randomArmyAlly();
          if (!ally)
            alliesCount = 0;
        }
        
        StackReflist *stacks = new StackReflist();
        recruitHero(heroproto, city, gold_needed, alliesCount, ally, stacks);
        delete stacks;
      }
    }
  return accepted;
}
        
std::list<Stack*> Player::getStacksWithItems() const
{
  return getStacklist()->getStacksWithItems();
}

bool Player::setPathOfStackToPreviousDestination(Stack *stack)
{
  std::list<Action*>moves = getMovesThisTurn();
  if (moves.size() > 0)
    {
      Vector<int> dest = Vector<int>(-1,-1);
      for (std::list<Action*>::const_reverse_iterator it = moves.rbegin();
           it != moves.rend(); ++it)
        {
          if ((*it)->getType() != Action::STACK_MOVE)
            continue;
          Action_Move *move = dynamic_cast<Action_Move*>(*it);
          guint32 id = move->getStackId();
          if (id == stack->getId())
            continue;
          Stack *prev = d_stacklist->getStackById(id);
          if (!prev)
            dest = move->getEndingPosition();
          else
            {
              dest = prev->getLastPointInPath();
              if (dest == Vector<int>(-1,-1))
                dest = move->getEndingPosition();
            }
          break;
        }
      if (dest != Vector<int>(-1,-1))
        {
          PathCalculator *path_calculator = new PathCalculator(stack);
          guint32 total_moves = 0, turns = 0, left = 0;
          Path *new_path = path_calculator->calculate(dest, total_moves, turns,
                                                      left, true);
          if (new_path->size())
            stack->setPath(*new_path);
          delete new_path;
          delete path_calculator;

          return true;
        }
    }
  return false;
}

bool Player::doHeroUseItem(Hero *hero, Item *item, Player *victim,
                           City *friendly_city, City *enemy_city, City *neutral_city, City *city)
{
  if (item->getBonus() & ItemProto::STEAL_GOLD)
    {
      assert (victim != NULL);
      double percent = item->getPercentGoldToSteal();
      if (percent > 100)
        percent = 100;
      else if (percent < 0)
        percent = 0;
      int gold = victim->getGold() * (percent / 100.0);
      if (gold > 0)
        {
          victim->withdrawGold(gold);
          addGold(gold);
          stole_gold.emit(victim, gold);
        }
    }
  if (item->getBonus() & ItemProto::SINK_SHIPS)
    {
      assert (victim != NULL);
      std::list<Stack*> sunk = victim->getStacklist()->killArmyUnitsInBoats();
      std::list<History*> history;
      guint32 num_armies = removeDeadArmies(sunk, history);
      sunk_ships.emit(victim, num_armies);
    }
  if (item->getBonus() & ItemProto::PICK_UP_BAGS)
    {
      guint32 num_bags = 0;
      std::list<MapBackpack*> bags = GameMap::getInstance()->getBackpacks();
      num_bags = bags.size();
      for (std::list<MapBackpack*>::iterator it = bags.begin();
           it != bags.end(); ++it)
        doHeroPickupAllItems(hero, (*it)->getPos());
      bags_picked_up.emit(hero, num_bags);
    }
  if (item->getBonus() & ItemProto::ADD_2MP_STACK)
    {
      guint32 mp = 2;
      Stack *stack = getStacklist()->getArmyStackById(hero->getId());
      stack->incrementMoves(mp);
      mp_added_to_hero_stack.emit(hero, mp);
    }
  if (item->getBonus() & ItemProto::BANISH_WORMS)
    {
      guint32 num_worms_killed = 0;
      std::list<History*> history;
      for (auto j: *Playerlist::getInstance())
        {
          std::list<Stack*> affected =
            j->getStacklist()->killArmies(item->getArmyTypeToKill());
          if (affected.size())
            num_worms_killed += removeDeadArmies(affected, history);
        }
      const ArmyProto *a = Armysetlist::getInstance()->getArmy(Playerlist::getActiveplayer()->getArmyset(), item->getArmyTypeToKill());
      worms_killed.emit(hero, a->getName(), num_worms_killed);
    }
  if (item->getBonus() & ItemProto::BURN_BRIDGE)
    {
      //am i on a bridge?
      Vector<int> pos = d_stacklist->getPosition(hero->getId());
      bool burned = GameMap::getInstance()->burnBridge(pos);
      if (burned)
        bridge_burned.emit(hero);
    }
  if (item->getBonus() & ItemProto::CAPTURE_KEEPER)
    {
      Vector<int> pos = d_stacklist->getPosition(hero->getId());
      Ruin *ruin = GameMap::getInstance()->getRuin(pos);
      if (ruin && ruin->isSearched() == false)
        {
          if (ruin->getOccupant() && ruin->getOccupant ()->getStack () &&
              ruin->getOccupant()->getStack ()->size() > 0)
            {
              Glib::ustring name = ruin->getOccupant()->getName();
              addStack(ruin->getOccupant()->getStack ());
              ruin->clearOccupant();
              keeper_captured.emit(hero, ruin, name);
            }
        }
    }
  if (item->getBonus() & ItemProto::SUMMON_MONSTER)
    {
      Vector<int> pos = d_stacklist->getPosition(hero->getId());
      Maptile::Building building = GameMap::getInstance()->getBuilding(pos);
      if (building == item->getBuildingTypeToSummonOn() ||
          item->getBuildingTypeToSummonOn() == 0)
        {
          Stack *stack = getStacklist()->getArmyStackById(hero->getId());
          StackReflist *stacks = new StackReflist();
          //okay we're going to add some allies now.
          const ArmyProto *a = Armysetlist::getInstance()->getArmy(Playerlist::getActiveplayer()->getArmyset(), item->getArmyTypeToSummon());
          Reward *reward = new Reward_Allies(a, 1);
          giveReward(stack, reward, stacks, false);
          delete reward;
          delete stacks;
          monster_summoned.emit(hero, a->getName());
        }
    }
  if (item->getBonus() & ItemProto::DISEASE_CITY)
    {
      if (enemy_city)
        {
          std::list<History*> history;
          std::list<Stack*> affected =
            enemy_city->diseaseDefenders(item->getPercentArmiesToKill());
          guint32 num_armies_killed = removeDeadArmies(affected, history);
          city_diseased.emit(hero, enemy_city->getName(), num_armies_killed);
        }
    }
  if (item->getBonus() & ItemProto::RAISE_DEFENDERS)
    {
      if (friendly_city)
        {
          //okay we're going to add some allies now.
          const ArmyProto *a = Armysetlist::getInstance()->getArmy(Playerlist::getActiveplayer()->getArmyset(), item->getArmyTypeToRaise());
          GameMap::getInstance()->addArmies(a, item->getNumberOfArmiesToRaise(),
                                            friendly_city->getPos());
          city_defended.emit(hero, friendly_city->getName(), a->getName(),
                             item->getNumberOfArmiesToRaise());
        }
    }
  if (item->getBonus() & ItemProto::PERSUADE_NEUTRALS)
    {
      if (neutral_city)
        {
          Stack *stack = getStacklist()->getArmyStackById(hero->getId());
          neutral_city->persuadeDefenders(this);
          takeCityInPossession(neutral_city);
          QuestsManager::getInstance()->cityOccupied(neutral_city, stack);
          city_persuaded.emit(hero, neutral_city->getName(),
                              neutral_city->countDefenders());
        }
    }
  if (item->getBonus() & ItemProto::TELEPORT_TO_CITY)
    {
      if (city)
        {
          Stack *s = getStacklist()->getArmyStackById(hero->getId());
          for (Stack::iterator i = s->begin(); i != s->end(); ++i)
            {
              if (city->getOwner() != s->getOwner())
                GameMap::getInstance()->addArmyAtPos(city->getPos(), *i);
              else
                GameMap::getInstance()->addArmy(city->getPos(), *i);
            }
          s->clear();
          deleteStack(s);
          //where do we teleport to?
          stack_teleported.emit(hero, city->getName());
          supdatingStack.emit(getStacklist()->getArmyStackById(hero->getId()));
        }
    }

  hero->getBackpack()->useItem(item);
  supdatingStack.emit(0);
  return true;
}

bool Player::heroUseItem(Hero *hero, Item *item, Player *victim,
                         City *friendly_city, City *enemy_city,
                         City *neutral_city, City *city)
{
  if (doHeroUseItem(hero, item, victim, friendly_city, enemy_city,
                    neutral_city, city))
    {
      addAction(new Action_UseItem(hero, item, victim, friendly_city,
                                   enemy_city, neutral_city, city));
      addHistory (new History_HeroUseItem(hero, item, victim, friendly_city,
                                          enemy_city, neutral_city, city));
      return true;
    }
  return false;
}

std::list<Item*> Player::getUsableItems() const
{
  return d_stacklist->getUsableItems();
}

bool Player::hasUsableItem() const
{
  return d_stacklist->hasUsableItem();
}

bool Player::getItemHolder(Item *item, Stack **stack, Hero **hero) const
{
  return d_stacklist->getItemHolder(item, stack, hero);
}

void Player::tallyDeadArmyTriumphs(std::list<Stack*> &stacks)
{
  for (std::list<Stack*>::iterator it = stacks.begin();
       it != stacks.end(); ++it)
    {
      for (Stack::iterator sit = (*it)->begin(); sit != (*it)->end(); ++sit)
        {
          if ((*sit)->getHP() > 0)
            continue;
          //Tally up the triumphs
          Player *enemy = (*sit)->getOwner();
          if ((*sit)->getAwardable()) //hey a special ally died
            d_triumphs->tallyTriumph(enemy, Triumphs::TALLY_SPECIAL);
          else if ((*sit)->isHero())
            {
              d_triumphs->tallyTriumph(enemy, Triumphs::TALLY_HERO);
              Hero *hero = dynamic_cast<Hero*>((*sit));
              guint32 count = hero->getBackpack()->countPlantableItems();
              for (guint32 i = 0; i < count; i++)
                d_triumphs->tallyTriumph(enemy, Triumphs::TALLY_FLAG);
            }
          else if ((*sit)->getStat(Army::SHIP, false)) //hey it was on a boat
            d_triumphs->tallyTriumph(enemy, Triumphs::TALLY_SHIP);
          else if ((*sit)->isHero() == false)
            d_triumphs->tallyTriumph(enemy, Triumphs::TALLY_NORMAL);
        }
    }
  return;
}

History* Player::handleDeadHero(Hero *h, Maptile *tile, Vector<int> pos)
{
  if (tile->getBuilding() == Maptile::RUIN)
    return new History_HeroKilledSearching(h);
  else if (tile->getBuilding() == Maptile::CITY)
    {
      City* c = GameMap::getCity(pos);
      return new History_HeroKilledInCity(h, c);
    }
  else //somewhere else
    return new History_HeroKilledInBattle(h);
  return NULL;
}

void Player::handleDeadHeroes(std::list<Stack*> &stacks, std::list<History*> &history)
{
  for (std::list<Stack*>::iterator it = stacks.begin();
       it != stacks.end(); ++it)
    {
      for (Stack::iterator sit = (*it)->begin(); sit != (*it)->end(); ++sit)
        {
          if ((*sit)->getHP() > 0)
            continue;
          if ((*sit)->isHero() == false)
            continue;
          //one of our heroes died
          //drop hero's stuff
          //now record the details of the death
      
          bool splash = false;
          doHeroDropAllItems (static_cast<Hero*>(*sit), (*it)->getPos(),
                              splash);
          Maptile *tile = GameMap::getInstance()->getTile((*it)->getPos());

          History *item = handleDeadHero (static_cast<Hero*>(*sit), tile,
                                          (*it)->getPos());
          if (item)
            history.push_back(item);
        }
    }
  return;
}

void Player::handleDeadArmiesForQuests(std::list<Stack*> &stacks,
                                       std::vector<guint32> &culprits)
{
  for (std::list<Stack*>::iterator it = stacks.begin();
       it != stacks.end(); ++it)
    {
      if ((*it)->getOwner() == this)
        continue;
      for (Stack::iterator sit = (*it)->begin(); sit != (*it)->end(); ++sit)
        {
          if ((*sit)->getHP() == 0)
            QuestsManager::getInstance()->armyDied(*sit, culprits);
        }
    }
  return;
}

double Player::countXPFromDeadArmies(std::list<Stack*>& stacks)
{
  double total = 0.0;
  for (std::list<Stack*>::iterator it = stacks.begin();
       it != stacks.end(); ++it)
    {
      if ((*it)->getOwner() == this)
        continue;
      for (Stack::iterator sit = (*it)->begin(); sit != (*it)->end(); ++sit)
        {
          if ((*sit)->getHP() > 0)
            continue;
		
          //Add the XP bonus to the total of the battle;
          total += (*sit)->getXpReward();
        }
    }
  return total;
}

void Player::doStackSort(Stack *s, std::list<guint32> army_ids)
{
  return s->sortByIds(army_ids);
}

void Player::doStacksReset()
{
  getStacklist()->resetStacks();
}

void Player::stacksReset()
{
  doStacksReset();
  addAction(new Action_ResetStacks(this));
}

void Player::doRuinsReset()
{
  if (this != Playerlist::getInstance()->getNeutral())
    return;
  for (auto it: *Ruinlist::getInstance())
    {
      Keeper* keeper = it->getOccupant();
      if (keeper)
        {
          if (keeper->getStack ())
            keeper->getStack ()->reset();
        }
    }
}

void Player::ruinsReset()
{
  doRuinsReset();
  addAction(new Action_ResetRuins());
}

void Player::doCollectTaxesAndPayUpkeep()
{
  //collect monies from cities
  Citylist::getInstance()->collectTaxes(this);

  //factor in the gold-per-city items that heroes may hold
  guint32 num_cities = Citylist::getInstance()->countCities(this);
  getStacklist()->collectTaxes(this, num_cities);

  //pay for existing armies
  getStacklist()->payUpkeep(this);
}

void Player::collectTaxesAndPayUpkeep()
{
  if (hasAlreadyCollectedTaxesAndPaidUpkeep())
    return;
  int prev_gold = getGold();
  doCollectTaxesAndPayUpkeep();
  Action_CollectTaxesAndPayUpkeep *a =
    new Action_CollectTaxesAndPayUpkeep (prev_gold, getGold());
  addAction(a);
}

void Player::doStackDefend(Stack *s)
{
  s->setDefending(true);
}

void Player::stackDefend(Stack *s)
{
  doStackDefend(s);
  addAction(new Action_DefendStack(s));
}

void Player::doStackUndefend(Stack *s)
{
  s->setDefending(false);
}

void Player::stackUndefend(Stack *s)
{
  doStackUndefend(s);
  addAction(new Action_UndefendStack(s));
}

void Player::doStackPark(Stack *s)
{
  s->setParked(true);
}

void Player::stackPark(Stack *s)
{
  doStackPark(s);
  addAction(new Action_ParkStack(s));
}

void Player::parkAllStacks()
{
  for (auto s : *getStacklist())
    {
      if (s->getParked() == false)
        stackPark (s);
    }
}

void Player::doStackUnpark(Stack *s)
{
  s->setParked(false);
}

void Player::stackUnpark(Stack *s)
{
  doStackUnpark(s);
  addAction(new Action_UnparkStack(s));
}

void Player::doStackSelect(Stack *s)
{
  d_stacklist->setActivestack(s);
}

void Player::stackSelect(Stack *s)
{
  doStackSelect(s);
  addAction(new Action_SelectStack(s));
}

void Player::doStackDeselect ()
{
  d_stacklist->setActivestack(0);
}

void Player::stackDeselect ()
{
  doStackDeselect();
  addAction(new Action_DeselectStack());
}

void Player::reportEndOfRound(guint32 score)
{
  addHistory(new History_Score(score));
  addHistory(new History_GoldTotal(d_gold));
}

void Player::reportEndOfTurn()
{
  addHistory(new History_EndTurn);
  addAction(new Action_EndTurn);
}

City *Player::getFirstCity() const
{
  for (std::list<History*>::const_iterator it = d_history.begin();
       it != d_history.end(); ++it)
    {
      if ((*it)->getType() == History::CITY_WON)
        {
          History_CityWon *h = dynamic_cast<History_CityWon*>(*it);
          City *c = Citylist::getInstance()->getById(h->getCityId());
          if (c->isBurnt() == false && c->getOwner() == this)
            return c;
        }
    }
  return NULL;
}
// End of file
