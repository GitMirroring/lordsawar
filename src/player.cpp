//  Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
//  Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
//  Copyright (C) 2004, 2005 Andrea Paternesi
//  Copyright (C) 2004 John Farrell
//  Copyright (C) 2005 Bryan Duff
//  Copyright (C) 2007, 2008, 2009, 2010, 2011, 2014, 2015, 2017, 2020, 2021,
//  2026 Ben Asselstine
//  Copyright (C) 2007, 2008 Ole Laursen
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

#include <stdlib.h>
#include <assert.h>
#include <fstream>
#include <sstream>
#include <sigc++/functors/mem_fun.h>

#include "move-result.h"
#include "player.h"
#include "player-list.h"
#include "stack-list.h"
#include "city-list.h"
#include "temple-list.h"
#include "city.h"
#include "path.h"
#include "army-set-list.h"
#include "real-player.h"
#include "ai-dummy.h"
#include "ai-fast.h"
#include "ai-smart.h"
#include "network-player.h"
#include "game-map.h"
#include "counter.h"
#include "army.h"
#include "hero.h"
#include "hero-proto.h"
#include "hero-templates.h"
#include "configuration.h"
#include "game-scenario-options.h"
#include "action.h"
#include "network-action.h"
#include "history.h"
#include "network-history.h"
#include "ai-analysis.h"
#include "ai-allocation.h"
#include "fog-map.h"
#include "quest-manager.h"
#include "signpost.h"
#include "vectored-unit.h"
#include "ucompose.hpp"
#include "army-prod-base.h"
#include "triumphs.h"
#include "backpack.h"
#include "map-backpack.h"
#include "path-calculator.h"
#include "stack-tile.h"
#include "temple.h"
#include "quest-city-occupy.h"
#include "quest-city-sack.h"
#include "quest-city-raze.h"
#include "quest-pillage-gold.h"
#include "quest.h"
#include "quest-kill-hero.h"
#include "quest-enemy-armies.h"
#include "quest-enemy-army-type.h"
#include "callback-enums.h"
#include "stack-ref-list.h"
#include "sight-map.h"
#include "reward-list.h"
#include "item.h"
#include "item-proto.h"
#include "xml-helper.h"
#include "rnd.h"
#include "game-action-list.h"
#include "turn-action-list.h"
#include "keeper.h"
#include "item-list.h"
#include "fight-result.h"
#include "lw.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::flush<<std::endl;}
#define debug(x)

Glib::ustring Player::d_tag = "player";

Player::Player(Glib::ustring name, guint32 armyset, Shield::Color shield, int width,
	       int height, Type type)
    :d_shield (shield), d_name(name), d_armyset(armyset), d_gold(250),
    d_dead(false), d_immortal(false), d_type(type), d_upkeep(0), d_income(0),
    d_diplomatic_rank (0), d_diplomatic_title (""),
    d_observable(true), surrendered(false), abort_requested(false)
{
    d_id = (guint32) shield;
    d_stacklist = new Stacklist();
    debug("type of " << d_name << " is " << type)

    d_fogmap = new FogMap(width, height);

    //initial fight order is the order in which the armies appear
    //in the default.xml file.
    for (auto i: *Armysetlist::instance()->get(d_armyset))
      d_fight_order.push_back(i->getId());

    for (unsigned int i = 0 ; i < MAX_PLAYERS; i++)
    {
      d_diplomatic_state[i] = AT_PEACE;
      d_diplomatic_proposal[i] = NO_PROPOSAL;
      d_diplomatic_score[i] = DIPLOMACY_STARTING_SCORE;
    }

    d_triumphs = new Triumphs();
}

Player::Player(const Player& player, bool sync_ids)
    :sigc::trackable(player), d_shield (player.d_shield), d_name(player.d_name),
    d_armyset(player.d_armyset), d_gold(player.d_gold), d_dead(player.d_dead),
    d_immortal(player.d_immortal), d_type(player.d_type), d_id(player.d_id),
    d_fight_order(player.d_fight_order), d_upkeep(player.d_upkeep),
    d_income(player.d_income), d_diplomatic_rank (player.d_diplomatic_rank),
    d_diplomatic_title (player.d_diplomatic_title),
    d_observable(player.d_observable),
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

  d_triumphs = new Triumphs(*player.getTriumphs());
}

Player::Player(XML_Helper* helper)
    :d_stacklist(0), d_fogmap(0), surrendered(false), abort_requested(false)
{
    helper->get(d_id, "id");
    helper->get(d_name, "name");
    helper->get(d_gold, "gold");
    helper->get(d_dead, "dead");
    helper->get(d_immortal, "immortal");
    Glib::ustring type_str;
    helper->get(type_str, "type");
    d_type = playerTypeFromString(type_str);
    helper->get(d_upkeep, "upkeep");
    helper->get(d_income, "income");
    Glib::ustring shield_str;
    helper->get(shield_str, "shield");
    d_shield = Shield::colorFromString (shield_str);
    helper->get(d_armyset, "armyset");

    // Read in Fight Order.  One ranking per army type.
    Glib::ustring fight_order;
    std::stringstream sfight_order;
    guint32 val;
    helper->get(fight_order, "fight_order");
    sfight_order.str(fight_order);
    for (auto i: *Armysetlist::instance()->get (d_armyset))
      {
        (void)i;
        sfight_order >> val;
        d_fight_order.push_back(val);
      }

    // Read in Diplomatic States.  One state per player.
    Glib::ustring diplomatic_states;
    std::stringstream sdiplomatic_states;
    helper->get(diplomatic_states, "diplomatic_states");
    sdiplomatic_states.str(diplomatic_states);
    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
            sdiplomatic_states >> val;
	    d_diplomatic_state[i] = DiplomaticState(val);
    }

    helper->get(d_diplomatic_rank, "diplomatic_rank");
    helper->get(d_diplomatic_title, "diplomatic_title");

    // Read in Diplomatic Proposals.  One proposal per player.
    Glib::ustring diplomatic_proposals;
    std::stringstream sdiplomatic_proposals;
    helper->get(diplomatic_proposals, "diplomatic_proposals");
    sdiplomatic_proposals.str(diplomatic_proposals);
    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
            sdiplomatic_proposals>> val;
	    d_diplomatic_proposal[i] = DiplomaticProposal(val);
    }

    // Read in Diplomatic Scores.  One score per player.
    Glib::ustring diplomatic_scores;
    std::stringstream sdiplomatic_scores;
    helper->get(diplomatic_scores, "diplomatic_scores");
    sdiplomatic_scores.str(diplomatic_scores);
    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
            sdiplomatic_scores >> val;
	    d_diplomatic_score[i] = val;
    }
    helper->get(d_observable, "observable");

    helper->register_tag(Action::d_tag, sigc::mem_fun(*this, &Player::load));
    helper->register_tag(History::d_tag, sigc::mem_fun(*this, &Player::load));
    helper->register_tag(Stacklist::d_tag, sigc::mem_fun(*this, &Player::load));
    helper->register_tag(FogMap::d_tag, sigc::mem_fun(*this, &Player::load));
    helper->register_tag(Triumphs::d_tag, sigc::mem_fun(*this, &Player::load));

}

Player::~Player()
{
    m_mover.disconnect ();
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

Player* Player::create(Glib::ustring name, guint32 armyset, Shield::Color shield, int width, int height, Type type)
{
  switch(type)
  {
  case HUMAN:
    return (new RealPlayer(name, armyset, shield, width, height));
  case AI_FAST:
    return (new AI_Fast(name, armyset, shield, width, height));
  case AI_DUMMY:
    return (new AI_Dummy(name, armyset, shield, width, height));
  case AI_SMART:
    return (new AI_Smart(name, armyset, shield, width, height));
  case NETWORKED:
    return (new NetworkPlayer(name, armyset, shield, width, height));
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

  calculateUpkeep();
  calculateIncome();

  GameActionlist::instance()->add(new TurnActionlist (this, d_actions));
  clearActionlist();
  History_StartTurn* item = new History_StartTurn();
  addHistory(item);
  guint32 order = Playerlist::instance()->getTurnOrderNumber(this);
  Action_InitTurn* action = new Action_InitTurn(order);
  addAction(action);
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
    for (auto city: *Citylist::instance())
      if (city->getOwner() == this && city->isBurnt() == false)
        Playerlist::getNeutral()->takeCityInPossession(city);

    d_diplomatic_rank = 0;
    d_diplomatic_title = Glib::ustring("");
}

bool Player::saveContents(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->save("id", d_id);
    retval &= helper->save("name", d_name);
    Glib::ustring shield_str = Shield::colorToString(d_shield);
    retval &= helper->save("shield", shield_str);
    retval &= helper->save("armyset", d_armyset);
    retval &= helper->save("gold", d_gold);
    retval &= helper->save("dead", d_dead);
    retval &= helper->save("immortal", d_immortal);
    Glib::ustring type_str = playerTypeToString(d_type);
    retval &= helper->save("type", type_str);
    debug("type of " << d_name << " is " << d_type)
    retval &= helper->save("upkeep", d_upkeep);
    retval &= helper->save("income", d_income);

    // save the fight order, one ranking per army type
    std::stringstream fight_order;
    for (std::list<guint32>::const_iterator it = d_fight_order.begin();
         it != d_fight_order.end(); ++it)
      {
        fight_order << (*it) << " ";
      }
    retval &= helper->save("fight_order", fight_order.str());

    // save the diplomatic states, one state per player
    std::stringstream diplomatic_states;
    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
      {
	diplomatic_states << d_diplomatic_state[i] << " ";
      }
    retval &= helper->save("diplomatic_states", diplomatic_states.str());

    retval &= helper->save("diplomatic_rank", d_diplomatic_rank);
    retval &= helper->save("diplomatic_title", d_diplomatic_title);

    // save the diplomatic proposals, one proposal per player
    std::stringstream diplomatic_proposals;
    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
      {
	diplomatic_proposals << d_diplomatic_proposal[i] << " ";
      }
    retval &= helper->save("diplomatic_proposals",
			       diplomatic_proposals.str());

    // save the diplomatic scores, one score per player
    std::stringstream diplomatic_scores;
    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
      {
	diplomatic_scores << d_diplomatic_score[i] << " ";
      }
    retval &= helper->save("diplomatic_scores", diplomatic_scores.str());

    retval &= helper->save("observable", d_observable);

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

bool Player::save (XML_Helper* helper) const
{
  (void) helper;
  //see real_player::save
  return false;
}

Player* Player::loadPlayer(XML_Helper* helper)
{
    Type type;
    Glib::ustring type_str;
    helper->get(type_str, "type");
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
    for (auto city: *Citylist::instance())
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

void Player::doStackJoin (Stack* receiver, Stack* joining)
{
  receiver->join (joining);
  deleteStack (joining);
  d_stacklist->setActivestack (receiver);
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

void Player::stackSplitAndMove (Stack* s, sigc::slot<void(MoveResult *,Stack *)> after)
{
  if (s->hasPath () == false)
    {
      after (new MoveResult, NULL);
      return;
    }
  Vector<int> pos = s->getLastReachablePointInPath ();
  if (pos == Vector<int> (-1,-1))
    {
      after (new MoveResult, NULL);
      return;
    }
  Stack *join = GameMap::getFriendlyStack (pos);
  if (join)
    return stackSplitAndMoveToJoin(s, join, after);
  else
    return stackSplitAndMoveToAttack(s, after);
}

void Player::stackSplitAndMoveToJoin (Stack* s, Stack *join, sigc::slot<void(MoveResult *,Stack*)> after)
{
  //the stack can't get there, but maybe part of the stack can.
  if (s->hasPath () == false)
    {
      after (new MoveResult, NULL);
      return;
    }

  std::list<guint32> ids;
  ids = s->determineReachableArmies (s->getLastPointInPath ());
  if (ids.size() == 0)
    {
      after (new MoveResult, NULL);
      return;
    }
  //if they're all reachable and we can join, just move them
  if (ids.size () == s->size () && GameMap::canJoin (s, join) == true)
    {
      stackMove
        (s,
         [after] (MoveResult *res)
         {
           after (res, NULL);
           return;
         });
    }
  else
    {
      //let's take who we can fit.
      if (ids.size () > join->getMaxArmiesToJoin ())
        {
          int diff = ids.size() - join->getMaxArmiesToJoin ();
          for (int i = 0; i < diff; i++)
            ids.pop_front ();
        }

      if (ids.size () == 0 ||
          s->fliesWithItemAndNonFlyersOverWaterOrMountains ())
        {
          after (new MoveResult, NULL);
          return;
        }
      else
        {
          //okay, ids.size armies can make the move.  but can that tile accept it?
          Stack *new_stack = stackSplitArmies (s, ids);
          if (new_stack)
            {
              setActivestack (new_stack);
              if (new_stack->calculatePath (join))
                stackMove
                  (new_stack,
                   [new_stack, after] (MoveResult *res)
                   {
                     after (res, new_stack);
                     return;
                   });
              else
                after (new MoveResult, new_stack);
            }
          else
            after (new MoveResult, NULL);
        }
    }
  return;
}

void Player::stackSplitAndMoveToAttack(Stack* s, sigc::slot<void(MoveResult *res, Stack *)> after)
{
  //the stack can't get there, but maybe part of the stack can.
  if (s->getPath ()->empty ())
    {
      after (new MoveResult, NULL);
      return;
    }

  std::list<guint32> ids;
  ids = s->determineReachableArmies (s->getLastPointInPath ());
  if (ids.size() == 0)
    {
      after (new MoveResult, NULL);
      return;
    }

  if (ids.size () == s->size ())
    {
      stackMove
        (s,
         [after] (MoveResult *res)
         {
           after (res, NULL);
           return;
         });
    }

  if (s->fliesWithItemAndNonFlyersOverWaterOrMountains ())
    {
      after (new MoveResult, NULL);
      return;
    }

  Stack *new_stack = stackSplitArmies (s, ids);
  if (new_stack)
    {
      setActivestack (new_stack);
      stackMove
        (new_stack,
         [new_stack, after] (MoveResult *res)
         {
           after (res, new_stack);
           return;
         });
    }
  return;
}

void Player::stackMove (Stack* s, sigc::slot<void(MoveResult*)> after)
{
  debug("Player::stackMove(Stack*)");
  int stepCount = 0;

  if (s->getPath ()->empty ())
    return after (NULL);

  smovingStack.emit (s);

  if (s->getPath ()->empty () || !s->getPath ()->getMovesExhaustedAtPoint ())
    {
      MoveResult *result = new MoveResult;
      result->setReachedEndOfPath (true);
      sstoppingStack.emit ();
      return after (result);
    }

  //now we show the steps, and then come back to this

  m_mover =
    Glib::signal_timeout ().connect
    ([this,
     after,
     s,
     stepCount] () mutable -> bool
     {
       if (abortRequested ())
          {
            MoveResult *result = new MoveResult;
            result->fillData(s, stepCount);
            result->setMoveAborted(true);
            after (result);
            return false;
          }

       bool cannot_step_over_friendly_stacks = false;
       if (s->getPath ()->size () > 1 && !nextStepOnEnemyStackOrCity (s))
         {
           bool step = stackMoveOneStep (s);
           if (!step)
             {
               step = stackMoveOneStepOverTooLargeFriendlyStacks (s);
               if (!step)
                 cannot_step_over_friendly_stacks = true;
             }
           if (step)
             {
               supdatingStack.emit (0);
               stepCount++;
             }
         }

       bool done =
         s->getPath ()->getMovesExhaustedAtPoint () <= 1 ||
         s->getPath ()->size () <= 1 ||
         nextStepOnEnemyStackOrCity (s) ||
         cannot_step_over_friendly_stacks;

       if (done)
         {
           MoveResult *result = new MoveResult;
           result->fillData (s, stepCount);
           m_mover.disconnect ();
           stackMoveFinalStep (s, result, after);
         }
       return !done;
     }, Configuration::s_displaySpeedDelay);

  return;
}

void Player::stackMoveFinalStep (Stack *s, MoveResult *result, sigc::slot<void(MoveResult*)> after)
{
  Glib::signal_timeout ().connect_once
    ([this, s, result, after] () mutable
     {
       int stepCount = result->getStepCount ();
       //the idea here is that we're one move away from our destination.
       //but in some cases we've already reached the end of the path
       //because a fight has to happen.

       //did we jump over a too large friendly stack to an enemy stack or city?

       //alright, we've walked up to the last place in the path.
       if (s->getPath ()->size () >= 1 && s->enoughMoves ())
         //now look for fight targets, joins etc.
         {
           Vector<int> pos = s->getFirstPointInPath ();
           City* city = GameMap::getCity (pos);
           Stack* target = GameMap::getStack (pos);

           //first fight_city to avoid ambiguity with fight_army
           if (city && (city->getOwner () != this) && (!city->isBurnt ()))
             {
               if (this->getDiplomaticState (city->getOwner ()) == AT_PEACE)
                 {
                   streacheryStack.emit
                     (s, city->getOwner (), pos,
                      [this, s, stepCount, result, pos,
                      after] (bool treachery) mutable
                      {
                        result->setTreachery (treachery);
                        result->setConsideredTreachery (true);
                        result->fillData (s, stepCount);
                        if (!treachery)
                          {
                            s->getPath()->clear ();
                            sstoppingStack.emit ();
                            after (result);
                          }
                        else
                          fight_in_the_city (s, pos, result, stepCount,
                                             after);
                      });
                 }
               else
                 fight_in_the_city (s, pos, result, stepCount, after);
               return;
             }

           //another friendly stack => share the tile if we're human
           else if (target && target->getOwner () == this)
             {
               if (stackMoveOneStep (s))
                 stepCount++;
               else
                 result->setTooLargeStackInTheWay (true);

               supdatingStack.emit (0);
               shaltedStack.emit (d_stacklist->getActivestack ());
               result->fillData (s, stepCount);

               after (result);
               return;
             }

           //enemy stack => fight
           else if (target)
             {
               if (this->getDiplomaticState (target->getOwner ()) == AT_PEACE)
                 {
                   streacheryStack.emit
                     (s, target->getOwner (), target->getPos (),
                      [this, s, stepCount, result, target,
                      after] (bool treachery) mutable
                      {
                        if (!treachery)
                          {
                            s->getPath()->clear ();
                            result->setConsideredTreachery (true);
                            result->fillData (s, stepCount);
                            sstoppingStack.emit ();
                            after (result);
                          }
                        else
                          {
                            result->setTreachery (treachery);
                            result->setConsideredTreachery (true);
                            result->fillData (s, stepCount);

                            fight_in_the_field (s, target, result, stepCount,
                                                after);
                          }
                      });
                 }
               else
                 {
                   fight_in_the_field (s, target, result, stepCount, after);
                 }
               return;
             }

           //else
           if (stackMoveOneStep (s))
             {
               supdatingStack.emit (0);
               stepCount++;
             }

           shaltedStack.emit (s);

           result->fillData (s, stepCount);
           after (result);
           return;
         }
       else if (s->getPath ()->size () >= 1 && s->enoughMoves () == false)
         {
           result->fillData (s, stepCount);
           /* if we can't attack a city, don't remember it in the stack's path. */
           Vector<int> pos = s->getFirstPointInPath ();
           City* city = GameMap::getCity (pos);
           if (city && city->getOwner () != this && city->isBurnt () == false)
             s->clearPath ();

           sstoppingStack.emit ();
           after (result);
           return;
         }

       result->setStepCount (stepCount);
       sstoppingStack.emit ();
       after (result);
       return;
     }, Configuration::s_displaySpeedDelay);
}

void Player::fight_in_the_city (Stack *s, Vector<int> target, MoveResult *result,
                                 int stepCount,
                                 sigc::slot<void(MoveResult*)> after)
{ 
  //step in, maybe not have to fight
  if (stackMoveOneStep (s))
    stepCount++;
  else
    {
      result->fillData (s, stepCount);
      shaltedStack.emit (s);
      after (result);
      return;
    }
      

  result->fillData (s, stepCount);

  City *city = GameMap::getCity (target);
  std::vector<Stack*> def_in_city = city->getDefenders ();
  if (!def_in_city.empty ())
    {
  
      Stack *defender = def_in_city[0];
      // maybe there are other defenders in the city, but we need to give
      // stackfight a single stack to start off with

      stackFight
        (s, defender,
         [this, city, result, s, defender, after] (Fight *fight) mutable
         {
           finishStackFight (fight, s, defender);
           auto outcome = fight->get_outcome ();
           result->setFightOutcome (outcome);
           supdatingStack.emit (0);
           delete fight;
           if (outcome == FightResult::ATTACKER_WON)
             shaltedStack.emit (s);
           city_fight_after_battle (city, s, result, after);
         });
    }
  else
    {
      shaltedStack.emit (s);
      result->setFightOutcome (FightResult::ATTACKER_WON);
      city_fight_after_battle (city, s, result, after);
    }
}

void Player::city_fight_after_battle (City *city, Stack *s, MoveResult *result,
                                      sigc::slot<void(MoveResult*)> after)
{
  if (result->getFightOutcome () == FightResult::DEFENDER_WON)
    {
      after (result);
      return;
    }
       
  lootCity (city, s, result, after);
}

void Player::fight_in_the_field (Stack *s, Stack *target, MoveResult *result,
                                 int stepCount,
                                 sigc::slot<void(MoveResult*)> after)
{
  stackFight
    (s, target,
     [this, result, s, target,
     stepCount, after] (Fight *fight) mutable
     {
       finishStackFight (fight, s, target);
       auto outcome = fight->get_outcome ();
       result->setFightOutcome (outcome);
       if (fight->get_outcome () == FightResult::ATTACKER_WON)
         {
           if (stackMoveOneStep (s))
             stepCount++;
           result->fillData (s, stepCount);
         }

       supdatingStack.emit (0);
       if (fight->get_outcome () == FightResult::ATTACKER_WON)
         shaltedStack.emit (s);
       else
         sstoppingStack.emit ();
       delete fight;
       field_fight_after_battle (result, after);
     });
}

void Player::field_fight_after_battle (MoveResult *result,
                                       sigc::slot<void(MoveResult*)> after)
{
  if (!QuestsManager::instance ()->notifyQuestCompleted
      (this,
       [this, result, after] ()
       {
         field_fight_after_quest_completed (result, after);
         return;
       }))
         
  field_fight_after_quest_completed (result, after);
}

void Player::field_fight_after_quest_completed (MoveResult *result,
                                                sigc::slot<void(MoveResult*)> a)
{
  //a placeholder to add more to the chain here or w/e
  a (result);
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

void Player::gainXPAfterFight (std::list<Stack*> &attackers,
                               std::list<Stack*> &defenders,
                               Fight *fight)
{
  double defender_xp = countXPFromDeadArmies (defenders);

  double attacker_xp = countXPFromDeadArmies (attackers);

  FightResult res = fight->get_fight_result ();
  if (!attackers.empty () && defender_xp != 0)
    updateArmyValues(attackers, defender_xp, &res);

  if (!defenders.empty () && attacker_xp != 0)
    updateArmyValues (defenders, attacker_xp, &res);

  supdatingStack.emit (0);
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

  debug("clean dead defenders");
  removeDeadArmies(defenders, attackerHeroes, defender_history);

  // and dead attackers
  debug("clean dead attackers");
  removeDeadArmies(attackers, defenderHeroes, attacker_history);

  debug("after fight: attackers empty? " << attackers.empty()
        << "(" << attackers.size() << ")");

  supdatingStack.emit(0);
}

void Player::stackFight(Stack* attacker, Stack* defender, sigc::slot<void(Fight*)> finish)
{
  debug("stackFight: player = " << getName()<<" at position "
        <<(*defender)->getPos().x<<","<<(*defender)->getPos().y << " with stack " << (*attacker)->getId() << " against " << (*defender)->getId() << " which is player = " <<(*defender)->getOwner()->getName());

  // I suppose, this should be always true, but one can never be sure
  bool attacker_active = attacker == d_stacklist->getActivestack ();
  if (attacker_active == false &&
      attacker->getOwner ()->isComputer () == true)
    {
      assert(0);
    }

  Fight *fight = new Fight (attacker, defender);
  fight->battle (GameScenarioOptions::s_intense_combat);

  auto att = fight->getAttackers ();
  auto def = fight->getDefenders ();
  gainXPAfterFight (att, def, fight);

  addAction (new Action_Fight (fight));

  fight_started.emit (fight, finish);
}

void Player::finishStackFight (Fight *fight, Stack *attacker, Stack *defender)
{
  (void) defender;
  std::list<Stack *> attackers = fight->getAttackers (),
    defenders = fight->getDefenders ();

  bool attacker_active = attacker == d_stacklist->getActivestack ();
  std::list<History*> attacker_history;
  std::list<History*> defender_history;
  cleanupAfterFight (attackers, defenders, attacker_history, defender_history);

  for (auto i = attacker_history.begin (); i != attacker_history.end (); ++i)
    addHistory (*i);
  for (auto i = defender_history.begin (); i != defender_history.end (); ++i)
    addHistory(*i);

  for (auto i = attackers.begin (); i != attackers.end (); ++i)
    addAction(new Action_ReorderArmies(*i));

  for (auto i = defenders.begin (); i != defenders.end (); ++i)
    addAction( new Action_ReorderArmies (*i));

  bool exists =
    std::find (d_stacklist->begin (), d_stacklist->end (), attacker)
    != d_stacklist->end ();

  if (!exists)
    {
      if (attacker_active)
        d_stacklist->setActivestack (0);
    }

  /*
  bool defender_stack_is_gone = false;
  // ...then the defender.
  exists = false;
  if (pd)
    exists =
      std::find (pd->getStacklist ()->begin (), pd->getStacklist ()->end (),
                 defender) != pd->getStacklist ()->end ();
  else
    exists = true;
  if (!exists)
    defender_stack_is_gone = true;
  */

  schangingStats.emit ();
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
FightResult::Outcome ruinfight (Stack **attacker, Stack **defender)
{
  Stack *loser;
  FightResult::Outcome result;
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
      result = FightResult::ATTACKER_WON;
      loser = *defender;
      for (Stack::iterator sit = loser->begin(); sit != loser->end();)
        {
          (*sit)->setHP (0);
          ++sit;
        }
    }
  else
    {
      result = FightResult::DEFENDER_WON;
      loser = *attacker;
      loser->getFirstHero()->setHP(0); /* only the hero dies */
    }

  return result;
}

FightResult Player::stackRuinFight (Stack **attacker, Keeper *defender,
                                    bool &stackdied,
                                    std::list<History*> &attacker_history,
                                    std::list<History*> &defender_history)
{
  FightResult res;
  res.set_outcome (FightResult::DRAW);
  if (defender->getStack () == NULL)
    {
      res.set_outcome (FightResult::ATTACKER_WON);
      return res;
    }
  debug("stackRuinFight: player = " << getName ()<<" at position "
        <<(*defender)->getStack ()->getPos ().x<<","<<(*defender)->getStack ()->getPos ().y);

  Stack *defender_stack = defender->getStack ();
  FightResult::Outcome result = ruinfight (attacker, &defender_stack);
  res.set_outcome (result);

  // cleanup

  // get attacker and defender heroes and more...
  std::list<Stack*> attackers;
  attackers.push_back (*attacker);
  std::list<Stack*> defenders;
  defenders.push_back (defender_stack);

  cleanupAfterFight (attackers, defenders, attacker_history, defender_history);
  bool exists =
    std::find (d_stacklist->begin (), d_stacklist->end (), *attacker)
    != d_stacklist->end ();

  if (!exists)
    {
      (*attacker) = 0;
      stackdied = true;
    }
  else
    stackdied = false;

  schangingStats.emit ();
  return res;
}

void Player::doStackSearchRuin(Stack *s, Ruin *r, FightResult::Outcome result)
{
  if (result == FightResult::DEFENDER_WON)
    {
      r->setSearched(false);
      return;
    }
  else if (result == FightResult::ATTACKER_WON)
    {
      r->setSearched(true);
      r->clearOccupant();
      r->setOwner(s->getOwner());
    }
  return;
}

void Player::stack_search_ruin (Stack* s, Ruin* r, sigc::slot<void(Ruin*,Reward*,bool,Stack*)> finish)
{
  bool died = false;
  std::list<History*> att_hist, def_hist;
  Keeper *keeper = r->getOccupant ();
  Glib::ustring hero_name = s->getFirstHero ()->getName ();
  Glib::ustring keeper_name = keeper ? keeper->getName () : "";
  FightResult res;
  if (keeper)
    {
      res = stackRuinFight (&s, keeper, died, att_hist, def_hist);
      //we delete it here because keepers are not in any players' stacklist.
      if (res.get_outcome () == FightResult::ATTACKER_WON)
        delete keeper; //occupant gets cleared in doStackSearchRuin
      for (auto i : att_hist)
        addHistory (i);
      clearHistorylist (def_hist);

      Reward *reward = r->getReward ();
      if (res.get_outcome () == FightResult::ATTACKER_WON && !reward &&
          r->hasSage () == false)
        r->populateWithRandomReward ();
      doStackSearchRuin (s, r, res.get_outcome ());
      addAction (new Action_Ruin (r, s));
    }
  else
    {
      res.set_outcome (FightResult::ATTACKER_WON);
      if (r->getReward () == NULL && r->hasSage () == false)
        r->populateWithRandomReward ();
      doStackSearchRuin (s, r, res.get_outcome ());
      addAction (new Action_Ruin (r, s));
    }

  m_ruinfight.emit
    (hero_name, keeper_name, res,
     [this, res, r, died, s, finish] ()
     {
       if (res.get_outcome () == FightResult::DEFENDER_WON)
         finish (r, NULL, died, s);
       else if (res.get_outcome () == FightResult::ATTACKER_WON)
         {
           Reward *reward = r->takeReward ();
           Hero *hero = dynamic_cast<Hero *>(s->getFirstHero ());
           if (r->hasSage ())
             addHistory (new History_FoundSage (hero));
           addHistory (new History_HeroRuinExplored (hero, r));

           supdatingStack.emit (0);

           finish (r, reward, died, s);
         }
     });
}

int Player::doStackSearchTemple(Stack *s)
{
  // you have your stack blessed (+1 strength)
  int count = s->bless();

  supdatingStack.emit(0);

  return count;
}

void Player::stack_search_temple (Stack* s, Temple* t, sigc::slot<void(Stack*,Temple*,int)> finish)
{
  debug("Player::stackSearchTemple");

  addAction (new Action_Temple (t, s));

  int num_armies_blessed = doStackSearchTemple (s);
  finish (s, t, num_armies_blessed);
  return;
}

Quest* Player::doHeroGetQuest(Hero *hero, bool except_raze)
{
  std::vector<Quest*> quests =
    QuestsManager::instance()->getPlayerQuests(Playerlist::getActiveplayer());
  if (quests.size() > 0 && GameScenarioOptions::s_play_with_quests == GameParameters::ONE_QUEST_PER_PLAYER)
    return NULL;

  Quest *q = NULL;
  if (hero)
    q = QuestsManager::instance()->createNewQuest (hero->getId(), except_raze);

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
      if (fight.get_outcome () == FightResult::ATTACKER_WON)
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
  if (Citylist::instance()->countCities(defender) == 1)
  {
    if (defender->getDiplomaticRank() < getDiplomaticRank())
      deteriorateDiplomaticRelationship (2);
    else if (defender->getDiplomaticRank() > getDiplomaticRank())
      improveDiplomaticRelationship (2, defender);
  }
}

bool Player::calculateLoot (Player *looted, guint32 &added, guint32 &subtracted)
{
  Player *defender = looted;

  // if the attacked city isn't neutral, loot some gold
  if (defender != Playerlist::getNeutral ())
    {
      int amt = (defender->getGold () /
                 (2 * (Citylist::instance ()->countCities (defender) + 1)) * 2);
      // give (Enemy-Gold/(2Enemy-Cities)) to the attacker
      // and then take away twice that from the defender.
      // the idea here is that some money is taken in the invasion
      // and other monies are lost forever
      // NOTE: +1 because the looted player just lost a city
      subtracted = amt;
      amt /= 2;
      added = amt;
    }

  // ensure looted gold is always 10 or more.
  if (added < MIN_LOOTED_GOLD)
    {
      subtracted = 0;
      added = 0;
    }
  return added > 0;
}

void Player::doConquerCity(City *city)
{
  takeCityInPossession(city);
}

//this helps us test.
void Player::conquerAllCities()
{
  for (auto city: *Citylist::instance())
    {
      if (city->getOwner() != this)
        {
          for (auto stack: city->getDefenders())
              GameMap::instance()->removeStack(stack);
          conquerCity (city, NULL);
        }
    }
}

void Player::conquerCity(City *city, Stack *stack)
{
  Action_ConquerCity *action = new Action_ConquerCity(city);

  doConquerCity(city);
  addAction(action);
  addHistory(new History_CityWon(city));
  if (stack && stack->hasHero())
  {
    Hero *hero = dynamic_cast<Hero *>(stack->getFirstHero());
    addHistory(new History_HeroCityWon(city, hero));
  }
}

void Player::lootCity (City *city, Stack *s, MoveResult *result,
                       sigc::slot<void(MoveResult*)> after)
{
  Player *looted = city->getOwner ();
  guint32 added = 0, subtracted = 0;
  if (calculateLoot (looted, added, subtracted))
    {
      doLootCity (looted, added, subtracted);
      addAction (new Action_Loot (this, looted, added, subtracted));
      m_looting_city.emit
        (added,
         [this, city, s, result, after] ()
         {
           city_fight_after_looting (city, s, result, after);
         });
    }
  else
    city_fight_after_looting (city, s, result, after);
  return;
}

void Player::city_fight_after_looting (City *c, Stack *s, MoveResult *result,
                                       sigc::slot<void(MoveResult*)> after)
{
  conquerCity (c, s);
  // this is the first round of checking for quest completions,
  // the battle can put us over the line on quests like:
  // kill hero, kill army unit type, and kill armies
  if (!QuestsManager::instance ()->notifyQuestCompleted
      (this,
       [this, c, s, result, after] ()
       {
         city_fight_after_quest1_completed (c, s, result, after);
         return;
       }))
    city_fight_after_quest1_completed (c, s, result, after);
}
  
void Player::city_fight_after_quest1_completed (City *city, Stack *s,
                                                MoveResult *result,
                                                sigc::slot<void(MoveResult*)> after)
{
  m_city_defeated.emit
    (city, s,
     [this, city, s, result, after] (CityDefeatedChoice a)
     {
       switch (a)
         {
         case CITY_DEFEATED_OCCUPY:
           cityOccupy (city);
           city_fight_after_city_defeated (city, a, result, after);
           break;

         case CITY_DEFEATED_PILLAGE:
             {
               int pillaged_army_type = -1, gold = 0;
               cityPillage (city, gold, &pillaged_army_type);
               m_city_pillaged.emit
                 (city, gold, pillaged_army_type,
                  [this, city, a, result, after] ()
                  {
                    city_fight_after_city_defeated (city, a, result, after);
                  });
             }
           break;

         case CITY_DEFEATED_SACK:
             {
               int gold = 0;
               std::list<guint32> sacked_types;
               citySack (city, gold, &sacked_types);
               m_city_sacked.emit
                 (city, gold, sacked_types,
                  [this, city, a, result, after] ()
                  {
                    city_fight_after_city_defeated (city, a, result, after);
                  });
             }
           break;

         case CITY_DEFEATED_RAZE:
           m_city_raze_query.emit
             (city,
              [this, city, a, result, after] (bool raze_confirmed)
              {
                if (raze_confirmed)
                  {
                    cityRaze (city);
                    deteriorateDiplomaticRelationship (5);
                  }
                city_fight_after_city_defeated (city, a, result, after);
              });
           break;
         }
     });
}

void Player::city_fight_after_city_defeated (City *c, CityDefeatedChoice a,
                                             MoveResult *result,
                                             sigc::slot<void(MoveResult*)> after)
{
  if (a == CityDefeatedChoice::CITY_DEFEATED_RAZE)
    {
      m_city_razed.emit
        (c,
         [this, c, a, result, after] ()
         {
           city_fight_after_raze_notification (c, a, result, after);
         });
    }
  else
    city_fight_after_raze_notification (c, a, result, after);
}

void Player::city_fight_after_raze_notification (City *c, CityDefeatedChoice a,
                                                 MoveResult *result,
                                                 sigc::slot<void(MoveResult*)> after)
{
  // this is the second round of checking for quest completions,
  // conquering a city can put us over the line on quests like:
  // raze city, sack city, pillage, occupy city
  if (!QuestsManager::instance ()->notifyQuestCompleted
      (this,
       [this, c, a, result, after] ()
       {
         city_fight_after_quest2_completed (c, a, result, after);
         return;
       }))
  city_fight_after_quest2_completed (c, a, result, after);
}

void Player::city_fight_after_quest2_completed (City *c, CityDefeatedChoice a,
                                                MoveResult *result,
                                                sigc::slot<void(MoveResult*)> after)
{
  if (a != CityDefeatedChoice::CITY_DEFEATED_RAZE)
    {
      m_open_city_dialog.emit
        (c,
         [this, c, result, after] ()
         {
           city_fight_after_city_window (c, result, after);
         });
      ;
    }
  else
    city_fight_after_city_window (c, result, after);
}

void Player::city_fight_after_city_window (City *c, MoveResult *result,
                                           sigc::slot<void(MoveResult *)> after)
{
  //city window closes, end of chain
  (void) c;
  after (result);
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

  QuestsManager::instance()->cityOccupied(c, getActivestack());
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
      c->squeezeProductionSlots ();

      addGold(gold);
      Stack *s = getActivestack();
      QuestsManager::instance()->cityPillaged(c, s, gold);
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
  QuestsManager::instance()->citySacked(c, s, gold);
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

  QuestsManager::instance()->cityRazed(c, getActivestack());
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
                       (*Armysetlist::instance()->getArmy(as, type)));

  // and do the rest of the neccessary actions
  withdrawGold(Armysetlist::instance()->getArmy(as, type)->getNewProductionCost());
}

bool Player::cityBuyProduction(City* c, int slot, int type)
{
  guint32 as = c->getOwner()->getArmyset();

  // sort out unusual values (-1 is allowed and means "scrap production")
  if (type <= -1 || Armysetlist::instance()->getArmy(d_armyset, type) == NULL)
    return false;

  // return if we don't have enough money
  if (type != -1 &&
      (int)Armysetlist::instance()->getArmy(as, type)->getNewProductionCost() > d_gold)
    return false;

  // return if the city already has the production
  if (c->hasProductionBase(type))
    return false;

  // can't put it in that slot
  if (slot >= (int)c->getMaxNoOfProductionBases())
    return false;

  addAction(new Action_Buy (c, slot, Armysetlist::instance()->getArmy(as, type)));

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
  if (GameMap::instance()->canDropBag(pos) == false)
    {
      h->getBackpack()->removeFromBackpack(i);
      delete i;
      splash = true;
    }
  else
    {
      GameMap::instance()->getTile(pos)->getBackpack()->addToBackpack(i);
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
  bool found = GameMap::instance()->getTile(pos)->getBackpack()->removeFromBackpack(i);
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
  MapBackpack *backpack = GameMap::instance()->getTile(pos)->getBackpack();
  while (backpack->empty() == false)
    doHeroPickupItem(h, backpack->front(), pos);
  return true;
}

bool Player::heroPickupAllItems(Hero *h, Vector<int> pos)
{
  MapBackpack *backpack = GameMap::instance()->getTile(pos)->getBackpack();
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

bool Player::heroQuestExpired (Hero *h)
{
  addHistory (new History_HeroQuestExpired (h));
  return true;
}

void Player::doResign(std::list<History*> &histories)
{
  //disband all stacks
  std::list<Stack*> stacks = getStacklist()->kill();
  removeDeadArmies(stacks, histories);

  //raze all cities
  for (auto city: *Citylist::instance())
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
      cities = Citylist::instance()->getCitiesVectoringTo(dest);
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
      if (GameMap::instance()->findPlantedStandard(this) != src)
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
      if (GameMap::instance()->findPlantedStandard(this) != dest)
	return false;
    }
  else
    {
      if (dest_city->getOwner() != this)
	return false;
    }

  //check to see if the destination has enough room to accept all of the
  //cities we want to send to it.
  std::list<City*> sources = Citylist::instance()->getCitiesVectoringTo(src);
  std::list<City*> alreadyvectored =
    Citylist::instance()->getCitiesVectoringTo(dest);

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
  GameMap::instance()->getTile(pos)->getBackpack()->addToBackpack(item);
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
            if (owner == Playerlist::getNeutral () &&
                GameMap::instance ()->getBuilding ((*it)->getPos ()) ==
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

void Player::updateArmyValues(std::list<Stack*>& stacks, double xp_sum, FightResult *result)
{
  std::list<Stack*>::iterator it;
  double numberarmy = 0;

  for (it = stacks.begin (); it != stacks.end (); ++it)
    numberarmy += (*it)->size ();

  for (it = stacks.begin (); it != stacks.end (); )
    {
      debug("Stack: " << (*it));

      for (Stack::iterator sit = (*it)->begin (); sit != (*it)->end ();)
        {
          Army *army = *sit;
          debug("Army: " << army);

          // here we adds XP
          army->gainXp ((double)((xp_sum) / numberarmy));
          debug("Army gets " << (double)((xp_sum) / numberarmy) << " XP");

          // here we adds 1 to number of battles
          army->setBattlesNumber (army->getBattlesNumber () + 1);
          debug("Army battles " <<  army->getBattlesNumber ());

          // medals only go to non-ally armies.
          if ((*it)->hasHero () && army->isHero () == false &&
              army->getAwardable () == false)
            {
              if((army->getBattlesNumber ())>10 &&
                 !(army->getMedalBonus (2)))
                {
                  army->setMedalBonus (2, true);
                  if (result)
                    result->add_medalist (army, 2);
                  // We must recalculate the XPValue of this unit since it
                  // got a medal
                  army->setXpReward(army->getXpReward () + 1);
                  // We get the medal bonus here
                  army->setStat (Army::STRENGTH, army->getStat (Army::STRENGTH, false)+1);
                }

              debug("Army hits " <<  army->getNumberHasHit ());

              // Only give medals if the unit has attacked often enough, else
              // medals lose the flair of something special; a value of n
              // means roughly to hit an equally strong unit around n
              // times. (note: one hit! An attack can consist of up to
              // strength hits)
              if((army->getNumberHasHit () > 50) && !army->getMedalBonus (0))
                {
                  army->setMedalBonus (0, true);
                  if (result)
                    result->add_medalist (army, 0);
                  // We must recalculate the XPValue of this unit since it
                  // got a medal
                  army->setXpReward (army->getXpReward () + 1);
                  // We get the medal bonus here
                  army->setStat (Army::STRENGTH, army->getStat (Army::STRENGTH, false) + 1);
                }

              debug("army being hit " <<  army->getNumberHasBeenHit ());

              // Gives the medal for good defense. The more negative the
              // number the more blows the unit evaded. n means roughly
              // avoid n hits from an equally strong unit. Since we want
              // to punish the case of the unit hiding among many others,
              // we set this value quite high.
              if((army->getNumberHasBeenHit () < -100) && !army->getMedalBonus (1))
                {
                  army->setMedalBonus (1,true);
                  if (result)
                    result->add_medalist (army, 1);
                  // We must recalculate the XPValue of this unit since it
                  // got a medal
                  army->setXpReward (army->getXpReward () + 1);
                  // We get the medal bonus here
                  army->setStat (Army::STRENGTH, army->getStat (Army::STRENGTH, false) + 1);
                }
              debug("Army hits " <<  army->getNumberHasHit ());

              for(int i= 0; i < 3; i++)
                {
                  debug("MEDAL[" << i << "]==" << army->getMedalBonus (i));
                }
            }

          // We reset the hit values after the battle
          army->setNumberHasHit (0);
          army->setNumberHasBeenHit (0);

          if (army->isHero () && getType () != Player::NETWORKED &&
              army->getOwner () != Playerlist::getNeutral ())
            {
              Hero *h = dynamic_cast<Hero*> (army);
              for (int i = 0; i < h->canGainLevels (); i++)
                {
                  if (result)
                    result->add_advancing_hero (h);
                }
              debug("Hero new XP=" << h->getXP ());
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
  Stack *s = GameMap::instance()->addArmy(city, newhero);

  if (stacks)
    {
      if (stacks->contains(s->getId()) == false)
        stacks->addStack(s);
    }

  if (alliesCount > 0)
    Reward_Allies::addAllies(this, city->getPos(), ally, alliesCount,
                             stacks);

  if (cost == 0)
    {
      // Initially give the first hero the player's standard.
      Glib::ustring name = String::ucompose(_("%1 Standard"), getName());
      Item *battle_standard = new Item (name, true, this);
      battle_standard->addBonus(Item::ADD1STACK);
      newhero->getBackpack()->addToBackpack(battle_standard, 0);
    }
  Character *c =
    HeroTemplates::instance ()->getCharacterById (hproto->getCharacterId ());
  for (auto item_id : c->get_starting_item_ids ())
    {
      ItemProto *proto = (*Itemlist::instance ())[item_id];
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

void Player::recruitHero(HeroProto* heroproto, Glib::ustring name, Hero::Gender gender, City *city, int cost, int alliesCount, const ArmyProto *ally, StackReflist *stacks)
{
  HeroProto *h = new HeroProto (*heroproto);
  h->setGender (gender);
  h->setName (name);
  addAction (new Action_RecruitHero (h, city, cost, alliesCount, ally));

  Hero *hero = doRecruitHero (h, city, cost, alliesCount, ally, stacks);
  delete h;
  if (hero)
    addHistory (new History_HeroEmerges(hero, city));
}

void Player::doDeclareDiplomacy (DiplomaticState state, Player *player)
{
  if (Playerlist::getNeutral() == player)
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
  if (Playerlist::getNeutral() == player)
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
  if (player == Playerlist::getNeutral())
    return AT_WAR;
  if (player == this)
    return AT_PEACE;
  return d_diplomatic_state[player->getId()];
}

Player::DiplomaticProposal Player::getDiplomaticProposal (Player *player) const
{
  if (player == Playerlist::getNeutral())
    return PROPOSE_WAR;
  if (player == this)
    return NO_PROPOSAL;
  return d_diplomatic_proposal[player->getId()];
}

guint32 Player::getDiplomaticScore (Player *player) const
{
  if (Playerlist::getNeutral() == player)
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
  if (Playerlist::getNeutral() == player || player == this)
    return;

  alterDiplomaticRelationshipScore (player, amount);

  addAction(new Action_DiplomacyScore(player, amount));
}

void Player::deteriorateDiplomaticRelationship (Player *player, guint32 amount)
{
  if (Playerlist::getNeutral() == player || player == this)
    return;

  alterDiplomaticRelationshipScore (player, -amount);

  addAction(new Action_DiplomacyScore(player, -amount));
}

void Player::deteriorateDiplomaticRelationship (guint32 amount)
{
  for (auto it: *Playerlist::instance())
    {
      if (it->isDead())
	continue;
      if (Playerlist::getNeutral() == it)
	continue;
      if (it == this)
	continue;
      it->deteriorateDiplomaticRelationship (this, amount);
    }
}

void Player::improveDiplomaticRelationship (guint32 amount, Player *except)
{
  for (auto it: *Playerlist::instance())
    {
      if (it->isDead())
	continue;
      if (Playerlist::getNeutral() == it)
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
  for (auto it: *Playerlist::instance())
    {
      if (it->isDead())
	continue;
      if (Playerlist::getNeutral() == it)
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
  for (auto it: *Playerlist::instance())
    {
      if (it->isDead())
	continue;
      if (Playerlist::getNeutral() == it)
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
        Armysetlist::instance()->lookupWeakestQuickestArmy(getArmyset());
      cityBuyProduction(c, free_slot, scout->getId());
    }
}

bool Player::safeFromAttack(City *c, guint32 safe_mp, guint32 min_defenders)
{
  //if there isn't an enemy city nearby to the source
  // calculate mp to nearest enemy city
  //   needs to be less than 18 mp with a scout
  //does the source city contain at least 3 defenders?

  City *enemy_city = Citylist::instance()->getNearestEnemyCity(c->getPos());
  if (enemy_city)
    {
      PathCalculator pc(c->getOwner(), c->getPos());
      int mp = pc.calculateMoves(enemy_city->getPos());
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
    Citylist::instance()->getNearestFriendlyVectorableCity(target->getPos());
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

  for (auto c: *Citylist::instance())
    {
      if (c->getOwner() != this || c->isBurnt())
	continue;
      Vector<int> dest = c->getVectoring();
      if (dest == Vector<int>(-1, -1))
	continue;
      if (safeFromAttack(c, safe_mp, min_defenders) == false)
	{
	  //City *target_city = Citylist::instance()->getObjectAt(dest);
	  //debug("stopping vectoring from " << c->getName() <<" to " << target_city->getName() << " because it's not safe to anymore!\n")
	  c->setVectoring(Vector<int>(-1,-1));
	  continue;
	}

      City *enemy_city = Citylist::instance()->getNearestEnemyCity(dest);
      if (!enemy_city)
	{
	  //City *target_city = Citylist::instance()->getObjectAt(dest);
	  //debug("stopping vectoring from " << c->getName() <<" to " << target_city->getName() << " because there aren't any more enemy cities!\n")
	  c->setVectoring(Vector<int>(-1,-1));
	  continue;
	}

      PathCalculator pc(this, dest, NULL);
      int mp = pc.calculateMoves(enemy_city->getPos());
      if (mp <= 0 || mp > (int)mp_to_front)
	{

	  //City *target_city = Citylist::instance()->getObjectAt(dest);
	  //debug("stopping vectoring from " << c->getName() <<" to " << target_city->getName() << " because it's too far away from an enemy city!\n")
	  c->setVectoring(Vector<int>(-1,-1));
	  continue;
	}
    }

  for (auto c : *Citylist::instance())
    {
      if (c->getOwner() != this || c->isBurnt())
	continue;
      City *enemy_city = Citylist::instance()->getNearestEnemyCity(c->getPos());
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
  for (std::list<Action*>::iterator bit = actions.begin();
       bit != actions.end(); ++bit)
    {
      if ((*bit)->getType() != Action::CITY_PROD)
	continue;
      if (find (keepers.begin(), keepers.end(), (*bit)) == keepers.end())
	{
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
  for (std::list<Action*>::iterator bit = actions.begin();
       bit != actions.end(); ++bit)
    {
      if ((*bit)->getType() != Action::CITY_VECTOR)
	continue;
      if (find (keepers.begin(), keepers.end(), (*bit)) == keepers.end())
	{
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

bool Player::AI_invadeCityQuestPreference(City *c, CityDefeatedChoice &action) const
{
  bool found = false;
  std::vector<Quest*> q = QuestsManager::instance()->getPlayerQuests(this);
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
void Player::maybeRecruitHero (sigc::slot<void(int)> after)
{
  City *city = NULL;
  int gold_needed = 0;
  if (Citylist::instance()->countCities(this) == 0)
    {
      after (0);
      return;
    }
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
        HeroTemplates::instance()->getRandomHero(get_shield ());
      if (gold_needed == 0)
	{
	  //we do it this way because maybe quickstart is on.
          city = Citylist::instance()->getCapitalCity(this);
          if (!city || city->isBurnt() == true)
	    city = getFirstCity();
	}
      else
        city = Citylist::instance()->getRandomCityForHero(this);

      sigc::slot<void(bool,Glib::ustring,Hero::Gender)> finish =
        [this, heroproto, city, gold_needed, after] (bool accepted,
                                                     Glib::ustring name,
                                                     Hero::Gender gender)
          {
            if (!accepted)
              return after (0);

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
            recruitHero (heroproto, name, gender, city, gold_needed,
                         alliesCount, ally, stacks);

            delete stacks;
            after (alliesCount);
          };

      //heroproto is always null for neutral
      if (city && heroproto)
        srecruitingHero.emit(heroproto, city, gold_needed, finish);
      else
        after (0);
    }
  else
    after (0);
  return;
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
      std::list<MapBackpack*> bags = GameMap::instance()->getBackpacks();
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
      for (auto j: *Playerlist::instance())
        {
          std::list<Stack*> affected =
            j->getStacklist()->killArmies(item->getArmyTypeToKill());
          if (affected.size())
            num_worms_killed += removeDeadArmies(affected, history);
        }
      const ArmyProto *a = Armysetlist::instance()->getArmy(Playerlist::getActiveplayer()->getArmyset(), item->getArmyTypeToKill());
      worms_killed.emit(hero, a->getName(), num_worms_killed);
    }
  if (item->getBonus() & ItemProto::BURN_BRIDGE)
    {
      //am i on a bridge?
      Vector<int> pos = d_stacklist->getPosition(hero->getId());
      bool burned = GameMap::instance()->burnBridge(pos);
      if (burned)
        bridge_burned.emit(hero);
    }
  if (item->getBonus() & ItemProto::CAPTURE_KEEPER)
    {
      Vector<int> pos = d_stacklist->getPosition(hero->getId());
      Ruin *ruin = GameMap::instance()->getRuin(pos);
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
      Maptile::Building building = GameMap::instance()->getBuilding(pos);
      if (building == item->getBuildingTypeToSummonOn() ||
          item->getBuildingTypeToSummonOn() == 0)
        {
          Stack *stack = getStacklist()->getArmyStackById(hero->getId());
          StackReflist *stacks = new StackReflist();
          //okay we're going to add some allies now.
          const ArmyProto *a = Armysetlist::instance()->getArmy(Playerlist::getActiveplayer()->getArmyset(), item->getArmyTypeToSummon());
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
          city_diseased.emit(enemy_city->getName(), num_armies_killed);
        }
    }
  if (item->getBonus() & ItemProto::RAISE_DEFENDERS)
    {
      if (friendly_city)
        {
          //okay we're going to add some allies now.
          const ArmyProto *a = Armysetlist::instance()->getArmy(Playerlist::getActiveplayer()->getArmyset(), item->getArmyTypeToRaise());
          GameMap::instance()->addArmies(a, item->getNumberOfArmiesToRaise(),
                                            friendly_city->getPos());
          city_defended.emit(friendly_city->getName(), a->getName(),
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
          QuestsManager::instance()->cityOccupied(neutral_city, stack);
          city_persuaded.emit(neutral_city->getName(),
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
                GameMap::instance()->addArmyAtPos(city->getPos(), *i);
              else
                GameMap::instance()->addArmy(city->getPos(), *i);
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
          Maptile *tile = GameMap::instance()->getTile((*it)->getPos());

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
            QuestsManager::instance()->armyDied(*sit, culprits);
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
  if (this != Playerlist::getNeutral())
    return;
  for (auto it: *Ruinlist::instance())
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
  Citylist::instance()->collectTaxes(this);

  //factor in the gold-per-city items that heroes may hold
  guint32 num_cities = Citylist::instance()->countCities(this);
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
  sselectStack.emit (s);
}

void Player::stackSelect (Stack *s)
{
  if (d_stacklist->getActivestack () != s)
    {
      doStackSelect(s);
      addAction(new Action_SelectStack(s));
    }
}

void Player::doStackDeselect ()
{
  d_stacklist->setActivestack(0);
  sdeselectStack.emit ();
}

void Player::stackDeselect ()
{
  if (d_stacklist->getActivestack ())
    {
      doStackDeselect();
      addAction(new Action_DeselectStack());
    }
}

void Player::reportEndOfRound(guint32 score)
{
  addHistory(new History_Score(score));
  addHistory(new History_GoldTotal(d_gold));
}

void Player::recordEndOfTurn()
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
          City *c = Citylist::instance()->getById(h->getCityId());
          if (c->isBurnt() == false && c->getOwner() == this)
            return c;
        }
    }
  return NULL;
}
        
void Player::setActivestack (Stack *s)
{
  d_stacklist->setActivestack (s);
}
        
std::list<Action_Move*> Player::getStackMoveActionEntries () const
{
  std::list<Action_Move*> actions;
  for (auto it = d_actions.begin (); it != d_actions.end (); ++it)
    if ((*it)->getType () == Action::STACK_MOVE)
      {
        Action_Move *a = dynamic_cast<Action_Move*>(*it);
        actions.push_back (a);
      }
  return actions;
}
        
guint32 Player::calculate_score (guint32 total_cities, guint32 total_gold,
                                 guint32 total_armies) const
{
  float city_component = (float)
    ((float) Citylist::instance ()->countCities(this)/ (float)total_cities) * 70.0;
  float gold_component = (float)
    ((float) getGold() / (float)total_gold) * 10.0;
  float army_component = (float)
    ((float) getStacklist()->countArmies() / 
     (float)total_armies) * 20.0;
  return (guint32) (city_component + gold_component + army_component);
}

Hero * Player::getNearestHeroWithQuest (Vector<int> pos, int distance) const
{
  auto points = GameMap::getNearbyPoints (pos, distance);
  for (auto p : points)
    {
      auto stacks = GameMap::getStacks (p);
      for (auto stack : stacks->getFriendlyStacks (this))
        {
          if (stack->hasQuest ())
            return stack->getFirstHeroWithAQuest ();
        }
    }
  return NULL;
}
