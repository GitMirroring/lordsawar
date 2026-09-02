//  Copyright (C) 2007, 2008 Ole Laursen
//  Copyright (C) 2006, 2007, 2008, 2009, 2010, 2014, 2015, 2016, 2017, 2020,
//  2026 Ben Asselstine
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

#include <config.h>

#include <vector>
#include <assert.h>
#include <gtkmm.h>
#include <sigc++/functors/mem_fun.h>
#include <sigc++/adaptors/bind.h>

#include "lw.h"
#include "game.h"

#include "ucompose.hpp"
#include "rectangle.h"
#include "game-scenario.h"
#include "next-turn-networked.h"
#include "next-turn-hotseat.h"
#include "stack-ref-list.h"
#include "small-map.h"
#include "army.h"
#include "fight.h"
#include "hero.h"
#include "hero-proto.h"
#include "stack-list.h"
#include "city-list.h"
#include "ruin-list.h"
#include "temple-list.h"
#include "signpost-list.h"
#include "city.h"
#include "ruin.h"
#include "signpost.h"
#include "temple.h"
#include "game-map.h"
#include "player-list.h"
#include "path.h"
#include "configuration.h"
#include "file.h"
#include "quest.h"
#include "reward.h"
#include "action.h"
#include "game-parameters.h"
#include "fog-map.h"
#include "history.h"
#include "location-box.h"
#include "backpack.h"
#include "map-backpack.h"
#include "stack-tile.h"
#include "hero-templates.h"
#include "game-scenario-options.h"
#include "ai-fast.h"
#include "ai-smart.h"
#include "sage.h"
#include "commentator.h"
#include "select-city-map.h"
#include "item.h"
#include "rnd.h"
#include "game-server.h"
#include "map-widget.h"
#include "move-result.h"
#include "quest-manager.h"

Game *Game::current_game = 0;

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::flush<<std::endl;}
#define debug(x)

Game::Game(GameScenario* gameScenario, NextTurn *nextTurn, bool headless)
    : d_gameScenario(gameScenario), d_nextTurn(nextTurn)
{
    current_game = this;
    input_locked = false;

    // init the bigmap
    bigmap.reset(new MapWidget);
    bigmap->signal_stack_selected().connect(
	sigc::hide(sigc::mem_fun(*this, &Game::on_stack_selected)));
    bigmap->signal_stack_grouped_or_ungrouped().connect(
	sigc::hide(sigc::mem_fun(*this, &Game::on_stack_grouped_or_ungrouped)));
    bigmap->signal_path_set().connect(
	sigc::mem_fun(*this, &Game::update_actions));
    bigmap->signal_city_visited().connect(
	sigc::mem_fun(*this, &Game::on_city_visited));
    bigmap->signal_city_queried().connect(
	sigc::mem_fun(*this, &Game::on_city_queried));
    bigmap->signal_city_unqueried().connect(
	sigc::mem_fun(*this, &Game::on_city_unqueried));
    bigmap->signal_ruin_queried().connect(
	sigc::mem_fun(*this, &Game::on_ruin_queried));
    bigmap->signal_ruin_unqueried().connect(
	sigc::mem_fun(*this, &Game::on_ruin_unqueried));
    bigmap->signal_ruin_visited().connect(
	sigc::mem_fun(*this, &Game::on_ruin_visited));
    bigmap->signal_signpost_queried().connect(
	sigc::mem_fun(*this, &Game::on_signpost_queried));
    bigmap->signal_signpost_unqueried().connect(
	sigc::mem_fun(*this, &Game::on_signpost_unqueried));
    bigmap->signal_temple_queried().connect(
	sigc::mem_fun(*this, &Game::on_temple_queried));
    bigmap->signal_temple_unqueried().connect(
	sigc::mem_fun(*this, &Game::on_temple_unqueried));
    bigmap->signal_temple_visited().connect(
	sigc::mem_fun(*this, &Game::on_temple_visited));
    bigmap->signal_stack_queried().connect(
	sigc::mem_fun(*this, &Game::on_stack_queried));
    bigmap->signal_stack_unqueried().connect(
	sigc::mem_fun(*this, &Game::on_stack_unqueried));

    // init the smallmap
    smallmap.reset(new SmallMap(headless));
    // pass map changes directly through 
    smallmap->resize();

    // connect player callbacks
    for (auto p: *Playerlist::instance())
      addPlayer(p);

    QuestsManager::instance ()->signal_quest_expired ().connect
      (sigc::mem_fun (*this, &Game::on_quest_expired));
    QuestsManager::instance ()->signal_quest_completed ().connect
      (sigc::mem_fun (*this, &Game::on_quest_completed));

    Playerlist::instance()->ssurrender.connect
      (sigc::mem_fun(*this, &Game::on_surrender_offered));

    d_nextTurn->splayerStart.connect
      (sigc::mem_fun (*this, &Game::init_turn));
    d_nextTurn->snextRound.connect(
	sigc::mem_fun(*d_gameScenario, &GameScenario::nextRound));
    d_nextTurn->snextRound.connect(
	sigc::mem_fun(m_round_begins, &sigc::signal<void()>::emit));
    d_nextTurn->snextRound.connect (sigc::mem_fun (*this, &Game::nextRound));
    d_nextTurn->supdating.connect(
	sigc::mem_fun(*this, &Game::redraw));
    d_nextTurn->m_signal_player_died.connect
	(sigc::mem_fun(*this, &Game::on_player_died));

    center_view_on_city();
    update_actions();

    HeroTemplates::instance();
}

Game::~Game()
{
  for (unsigned int i = 0; i < MAX_PLAYERS + 1; i++)
    {
      for (auto it: connections[i])
	it.disconnect();
      connections[i].clear();
    }
    delete d_gameScenario;
    delete d_nextTurn;
    HeroTemplates::deleteInstance();
}

void Game::addPlayer(Player *p)
{
  for (auto it: connections[p->getId()])
    it.disconnect();
  connections[p->getId()].clear();

  //now setup the connections that are specific for human players
  if (p->getType() == Player::HUMAN)
    {
      connections[p->getId()].push_back
	(p->advice_asked.connect
	 (sigc::mem_fun(m_advice_asked, &sigc::signal<void(float)>::emit)));
      connections[p->getId()].push_back
	(p->smovingStack.connect
	 (sigc::hide(sigc::mem_fun(*this, &Game::on_stack_starts_moving))));
      connections[p->getId()].push_back
	(p->sstoppingStack.connect
	 (sigc::mem_fun(*this, &Game::on_stack_stopped)));
      connections[p->getId()].push_back
	(p->shaltedStack.connect
	 (sigc::mem_fun(*this, &Game::on_stack_halted)));
      connections[p->getId()].push_back
	(p->getStacklist()->sgrouped.connect
	 (sigc::hide(sigc::mem_fun(*this, &Game::on_stack_grouped))));
      connections[p->getId()].push_back
	(p->stole_gold.connect
	 (sigc::mem_fun(m_stole_gold, &sigc::signal<void(Player*, 
                        guint32)>::emit)));
      connections[p->getId()].push_back
	(p->sunk_ships.connect
	 (sigc::mem_fun(m_sunk_ships, &sigc::signal<void(Player*, 
                        guint32)>::emit)));
      connections[p->getId()].push_back
	(p->bags_picked_up.connect
	 (sigc::mem_fun(m_bags_picked_up, &sigc::signal<void(Hero*, guint32)>::emit)));
      connections[p->getId()].push_back
	(p->mp_added_to_hero_stack.connect
	 (sigc::mem_fun(m_mp_added_to_hero_stack, &sigc::signal<void(Hero*, guint32)>::emit)));
      connections[p->getId()].push_back
	(p->worms_killed.connect
	 (sigc::mem_fun(m_worms_killed, &sigc::signal<void(Hero*, Glib::ustring, guint32)>::emit)));
      connections[p->getId()].push_back
	(p->bridge_burned.connect
	 (sigc::mem_fun(m_bridge_burned, &sigc::signal<void(Hero*)>::emit)));

      connections[p->getId()].push_back
	(p->keeper_captured.connect
	 (sigc::mem_fun(m_keeper_captured, &sigc::signal<void(Hero*, Ruin*, Glib::ustring)>::emit)));

      connections[p->getId()].push_back
	(p->monster_summoned.connect
	 (sigc::mem_fun(m_monster_summoned, &sigc::signal<void(Hero*, Glib::ustring)>::emit)));

      connections[p->getId()].push_back
	(p->city_diseased.connect
	 (sigc::mem_fun(m_city_diseased, &sigc::signal<void(Glib::ustring, guint32)>::emit)));
      connections[p->getId()].push_back
	(p->city_defended.connect
	 (sigc::mem_fun(m_city_defended, &sigc::signal<void(Glib::ustring, Glib::ustring, guint32)>::emit)));
      connections[p->getId()].push_back
	(p->city_persuaded.connect
	 (sigc::mem_fun(m_city_persuaded, &sigc::signal<void(Glib::ustring, guint32)>::emit)));
      connections[p->getId()].push_back
	(p->stack_teleported.connect
	 (sigc::mem_fun(m_stack_teleported, &sigc::signal<void(Hero*, Glib::ustring)>::emit)));
    }
  connections[p->getId()].push_back
    (p->m_city_razed.connect (sigc::mem_fun (*this, &Game::on_city_razed)));
  connections[p->getId()].push_back
    (p->m_city_raze_query.connect (sigc::mem_fun (*this, &Game::on_city_raze_query)));
      
      
  //now do all of the common connections
  connections[p->getId()].push_back
    (p->save_game.connect(sigc::mem_fun(*this, &Game::on_save_game)));
  connections[p->getId()].push_back
    (p->get_round.connect(sigc::mem_fun(*this, &Game::on_get_round)));
  connections[p->getId()].push_back
    (p->getStacklist()->snewpos.connect
     (sigc::mem_fun(m_stack_moves, &sigc::signal<void(Stack*, Vector<int>)>::emit)));
  connections[p->getId()].push_back
    (p->srecruitingHero.connect(sigc::mem_fun(*this, &Game::recruitHero)));
  connections[p->getId()].push_back
    (p->svisitingTemple.connect
     (sigc::mem_fun(*this, &Game::stack_searches_temple)));
  connections[p->getId()].push_back
    (p->ssearchingRuin.connect
     (sigc::mem_fun(*this, &Game::stack_searches_ruin)));
  connections[p->getId()].push_back
    (p->getStacklist()->snewpos.connect
     (sigc::mem_fun(*this, &Game::stack_arrives_on_tile)));
  connections[p->getId()].push_back
    (p->getStacklist()->soldpos.connect
     (sigc::mem_fun(*this, &Game::stack_leaves_tile)));
  connections[p->getId()].push_back
    (p->getStacklist()->sstackDied.connect
     (sigc::mem_fun(*this, &Game::on_stack_died)));
  connections[p->getId()].push_back
    (p->aborted_turn.connect (sigc::mem_fun
	   (m_game_stopped, &sigc::signal<void()>::emit)));

  connections[p->getId()].push_back
    (p->schangingStats.connect 
     (sigc::mem_fun(*this, &Game::update_sidebar_stats)));
        
  connections[p->getId()].push_back
    (p->schangingStatus.connect 
	 (sigc::mem_fun(m_progress_status_changed, &sigc::signal<void(Glib::ustring)>::emit)));
        
  connections[p->getId()].push_back
    (p->sbusy.connect (sigc::mem_fun (m_progress_changed, 
				      &sigc::signal<void()>::emit)));
  connections[p->getId()].push_back
    (p->supdatingStack.connect (sigc::mem_fun(*this, &Game::stackUpdate)));
  connections[p->getId()].push_back
    (p->sbagdropped.connect (sigc::mem_fun(*this, &Game::on_bag_dropped)));
  connections[p->getId()].push_back
    (p->m_looting_city.connect(sigc::mem_fun(*this, &Game::on_looting_city)));
  connections[p->getId()].push_back
    (p->m_city_defeated.connect(sigc::mem_fun(*this, &Game::on_city_defeated)));
  connections[p->getId()].push_back
    (p->m_city_pillaged.connect(sigc::mem_fun(*this, &Game::on_city_pillaged)));
  connections[p->getId()].push_back
    (p->m_city_sacked.connect(sigc::mem_fun(*this, &Game::on_city_sacked)));
  connections[p->getId()].push_back
    (p->m_open_city_dialog.connect(sigc::mem_fun(*this, &Game::on_open_city_dialog)));
  connections[p->getId()].push_back
    (p->streacheryStack.connect(sigc::mem_fun(*this, &Game::maybeTreachery)));
  connections[p->getId()].push_back
    (p->fight_started.connect (sigc::mem_fun(*this, &Game::on_fight_started)));

  connections[p->getId()].push_back
    (p->m_ruinfight.connect (sigc::mem_fun(*this, &Game::on_ruinfight)));
  connections[p->getId()].push_back
    (p->cityfight_finished.connect (sigc::mem_fun(*this, &Game::on_city_fight_finished))); 
  connections[p->getId()].push_back
    (p->sselectStack.connect (sigc::mem_fun(*this, &Game::on_select_stack))); 
  connections[p->getId()].push_back
    (p->sdeselectStack.connect (sigc::mem_fun(*this, &Game::on_deselect_stack))); 
  if (p->getType() == Player::NETWORKED && p == Playerlist::getActiveplayer())
    lock_inputs();
  if (p->getType() == Player::HUMAN && p == Playerlist::getActiveplayer())
    unlock_inputs();
}

void Game::on_stack_starts_moving()
{
  if (Playerlist::getActiveplayer()->getType() == Player::HUMAN)
    lock_inputs();
}

void Game::on_stack_stopped()
{
  if (Playerlist::getActiveplayer()->getType() == Player::HUMAN)
    unlock_inputs();
}

void Game::on_stack_halted(Stack *stack)
{
  if (Playerlist::getActiveplayer()->getType() == Player::HUMAN)
    unlock_inputs();
  if (stack == NULL)
    return;
  bigmap->reset_path_calculator(stack);
  //tell gamebigmap that a stack just stopped
}

void Game::on_stack_grouped(Stack *stack)
{
  bigmap->reset_path_calculator(stack);
  //tell gamebigmap that we just grouped/ungrouped a stack.
  return;
}

GameScenario *Game::getScenario()
{
  return current_game->d_gameScenario;
}

void Game::end_turn()
{
  //only human players hit this.
    unselect_active_stack();
    clear_stack_info();
    update_actions();
    lock_inputs();

    d_nextTurn->endTurn();
}

void Game::update_stack_info()
{
    Stack* stack = Playerlist::getActiveplayer()->getActivestack();

    //if (Playerlist::getActiveplayer()->getType() != Player::HUMAN &&
        //GameScenario::s_hidden_map == true)
      //return;
    m_stack_info_changed.emit(stack);
}

void Game::clear_stack_info()
{
    m_stack_info_changed.emit(0);
}

void Game::update_sidebar_stats ()
{
  SidebarStats s;
  Player *player = Playerlist::getActiveplayer ();
  if (player == Playerlist::getNeutral ())
    return;

  s.gold = player->getGold ();
  s.income = player->getIncome ();
  s.cities = Citylist::instance ()->countCities (player);
  s.upkeep = player->getStacklist ()->calculateUpkeep ();

  m_sidebar_stats_changed.emit (s);
}

void Game::redraw()
{
    if (bigmap.get())
      bigmap->queue_draw ();
    if (smallmap.get())
      smallmap->draw();
}

void Game::select_next_movable_stack()
{
  Stacklist *sl = Playerlist::getActiveplayer()->getStacklist();
  Stack* stack = sl->getNextMovable();
  Playerlist::getActiveplayer ()->stackSelect (stack);
  select_active_stack();
}

void Game::move_selected_stack_along_path ()
{
  Stack *stack = Playerlist::getActiveplayer ()->getActivestack ();

  Playerlist::getActiveplayer ()->stackMove
    (stack,
     [this] (MoveResult *res)
     {
       delete res;
       //maybe we joined another stack
       Stack *s = Playerlist::getActiveplayer ()->getActivestack ();
       if (s && s->canMove () == false)
         {
           Playerlist::getActiveplayer ()->stackDeselect ();
           unselect_active_stack ();
         }
     });
}

void Game::move_all_stacks()
{
  Player *player = Playerlist::getActiveplayer();

  std::list<Vector<int>> positions = player->getStacklist ()->getPositions ();

  auto after = std::make_shared<std::function<void()>>();
  *after =
    [this, player] () mutable
      {
        auto sl = player->getStacklist ();
        if (sl->getActivestack ())
          {
            if (sl->getActivestack ()->canMove () == false)
              {
                player->stackDeselect ();
                unselect_active_stack ();
              }
          }
      };

  auto next = std::make_shared<std::function<void(std::list<Vector<int>>)>>();
  *next =
    [this, player, next, after] (std::list<Vector<int>> pos) mutable
      {
        Vector<int> p = pos.front ();
        Stack *s = GameMap::getFriendlyStack (p);
        if (s->hasPath () && s->enoughMoves ())
          {
            player->stackSelect (s);
            select_active_stack ();
            player->stackMove
              (s,
               [this, s, pos, next, after] (MoveResult *res) mutable
               {
                 delete res;
                 pos.erase (pos.begin ());
                 if (pos.empty () == false)
                   (*next) (std::move (pos));
                 else
                   (*after) ();
               });
          }
        else
          {
            pos.erase (pos.begin ());
            if (pos.empty () == false)
              (*next) (std::move (pos));
            else
              (*after) ();
          }
      };


  if (positions.empty () == false)
    (*next) (positions);
  else
    (*after)();
}

void Game::defend_selected_stack()
{
  Player *player = Playerlist::getActiveplayer();
  Stack *stack = player->getActivestack();
  assert(stack);

  player->stackDefend(stack);

  stack = player->getStacklist()->getNextMovable();
  player->stackSelect (stack);

  if (stack)
    select_active_stack();
  else
    unselect_active_stack();
}

void Game::park_selected_stack()
{
  Player *player = Playerlist::getActiveplayer();
  Stack *stack = player->getActivestack();
  assert(stack);
  player->stackPark(stack);

  stack = player->getStacklist()->getNextMovable();
  if (stack)
    player->stackSelect (stack);

  if (stack)
    select_active_stack();
  else
    unselect_active_stack();
}

void Game::deselect_selected_stack()
{
  Player *player = Playerlist::getActiveplayer();
  player->stackDeselect ();
  unselect_active_stack();
}

void Game::center_selected_stack()
{
  Stack *stack = Playerlist::getActiveplayer()->getActivestack();
  if (stack) 
    select_active_stack();
}

void Game::search_stack (Stack *stack, SearchStackCallback after)
{
  Player *player = Playerlist::getActiveplayer ();
  Ruin* ruin = GameMap::getRuin (stack);
  Temple* temple = GameMap::getTemple (stack);

  if (ruin && !ruin->isSearched () && stack->hasHero () &&
      stack->getFirstHero()->getMoves () > 0 &&
      ((ruin->isHidden () == true && ruin->getOwner () == player) ||
       ruin->isHidden () == false))
    {
      player->stack_search_ruin
        (stack, ruin,
         [this, after] (Ruin *r, Reward *reward, bool stack_died, Stack *s)
         {
           stack_search_after_ruinfight (r, reward, stack_died, s, after);
         });
    }
  else if (temple && stack->getMoves () > 0)
    {
      player->stack_search_temple
        (stack, temple,
         [this, player, after] (Stack *s, Temple *t, int num_armies_blessed)
         {
           // here we pick a hero to get a quest, it can't be one that already
           // has one
           if (player->isComputer ())
             {
               Hero *hero = s->getFirstHeroWithoutAQuest ();
               bool got_quest = false;
               if (player->chooseQuest (hero))
                 {
                   bool except_raze =
                     GameScenario::s_razing_cities != GameParameters::NEVER;
                   auto q = player->heroGetQuest (hero, t, except_raze);
                   got_quest = q != NULL;
                 }
               after (got_quest, num_armies_blessed, false);
             }
           else
             {
               Hero *hero = s->getFirstHeroWithoutAQuest ();
               if (m_search_temple.empty ())
                 {
                   printf ("shouldn't get here, temple search without gui attached and human player\n");
                   abort ();
                 }
               else
                 m_search_temple.emit (hero, t, num_armies_blessed, after);
             }
         });
    }
  else
    after (false, 0, false);//can't search
}

void Game::stack_search_after_ruinfight (Ruin *ruin, Reward *reward,
                                         bool stack_died, Stack *stack,
                                         SearchStackCallback after)
{
  if (stack_died)
    {
      after (false, 0, stack_died);
      return;
    }

  if (ruin->hasSage () == true)
    {
      if (reward)
        delete reward;
      if (Playerlist::getActiveplayer ()->isComputer ())
        {
          //fixme
          //we could have the computer player pick something
          //from the sage
          after (false, 0, stack_died);
        }
      else
        {
          Sage *sage = ruin->generateSage ();
          if (m_sage_visited.empty ())
            after (false, 0, stack_died);
          else
            m_sage_visited.emit
              (ruin, sage, stack,
               [this, sage, after, stack_died, stack, ruin] (Reward *r)
               {
                 delete sage;
                 stack_search_after_sage
                   (r, stack, ruin,
                    [after, stack_died] ()
                    {
                      after (false, 0, stack_died);
                    });
               });
        }
    }
  else
    {
      stack_search_after_sage (reward, stack, ruin,
                               [after, stack_died] ()
                               {
                                 after (false, 0, stack_died);
                               });
    }
}

void Game::stack_search_after_sage (Reward *reward, Stack *stack, Ruin *ruin, sigc::slot<void()> finish)
{
  if (reward)
    {
      StackReflist *stacks = new StackReflist ();
      Player *player = stack->getOwner ();
      player->giveReward (stack, reward, stacks, false);
      delete stacks;
      redraw ();
      update_stack_info ();
      update_actions ();
      if (Playerlist::getActiveplayer ()->isComputer ())
        ;
      else
        m_ruin_searched.emit (ruin, stack, reward);
      delete reward;
    }
  else
    {
      redraw ();
      update_stack_info ();
      update_actions ();
    }

  update_sidebar_stats ();
  finish ();
}

void Game::use_item(Item *item)
{
  if (item->getNumberOfUsesLeft () <= 0)
    return;
  if (item->usableOnVictimPlayer ())
    m_select_item_victim_player.emit (item);
  else if (item->usableOnFriendlyCity ())
    m_select_city_to_use_item_on.emit (item, SelectCityMap::FRIENDLY_CITY);
  else  if (item->usableOnEnemyCity ())
    m_select_city_to_use_item_on.emit (item, SelectCityMap::ENEMY_CITY);
  else if (item->usableOnNeutralCity ())
    m_select_city_to_use_item_on.emit (item, SelectCityMap::NEUTRAL_CITY);
  else if (item->usableOnAnyCity ())
    m_select_city_to_use_item_on.emit (item, SelectCityMap::ANY_CITY);
  else
    {
      Player *active = Playerlist::getActiveplayer ();
      Stack *stack = NULL;
      Hero *hero = NULL;
      active->getItemHolder (item, &stack, &hero);
      active->heroUseItem (hero, item, NULL, NULL, NULL, NULL, NULL);
      redraw ();
    }
}

void Game::use_item_on_player (Item *item, Player *victim)
{
  Player *active = Playerlist::getActiveplayer ();
  Stack *stack = NULL;
  Hero *hero = NULL;
  active->getItemHolder (item, &stack, &hero);
  active->heroUseItem (hero, item, victim, NULL, NULL, NULL, NULL);
  redraw ();
}

void Game::use_item_on_friendly_city (Item *item, City *city)
{
  Player *active = Playerlist::getActiveplayer ();
  Stack *stack = NULL;
  Hero *hero = NULL;
  active->getItemHolder (item, &stack, &hero);
  active->heroUseItem (hero, item, NULL, city, NULL, NULL, NULL);
  redraw ();
}

void Game::use_item_on_enemy_city (Item *item, City *city)
{
  Player *active = Playerlist::getActiveplayer ();
  Stack *stack = NULL;
  Hero *hero = NULL;
  active->getItemHolder (item, &stack, &hero);
  active->heroUseItem (hero, item, NULL, NULL, city, NULL, NULL);
  redraw ();
}

void Game::use_item_on_neutral_city (Item *item, City *city)
{
  Player *active = Playerlist::getActiveplayer ();
  Stack *stack = NULL;
  Hero *hero = NULL;
  active->getItemHolder (item, &stack, &hero);
  active->heroUseItem (hero, item, NULL, NULL, NULL, city, NULL);
  redraw ();
}

void Game::use_item_on_any_city (Item *item, City *city)
{
  Player *active = Playerlist::getActiveplayer ();
  Stack *stack = NULL;
  Hero *hero = NULL;
  active->getItemHolder (item, &stack, &hero);
  active->heroUseItem (hero, item, NULL, NULL, NULL, NULL, city);
  redraw ();
}

void Game::search_selected_stack()
{
  Stack* stack = Playerlist::getActiveplayer ()->getActivestack ();
  search_stack (stack, 
                [] (bool got_quest, int num_blessed, bool stack_died)
                {
                  (void) got_quest;
                  (void) stack_died;
                  (void) num_blessed;
                });
  return;
}

void Game::stackUpdate(Stack* s)
{
  if (!s)
    s = Playerlist::getActiveplayer()->getActivestack();

  //if player is not to be observed, bail now
  if (s != NULL && s->getOwner()->isObservable() == false)
      return;

  if (s)
    smallmap->center_view_on_tile(s->getPos(), true);

  bigmap->queue_draw ();

  update_stack_info();
  update_actions();

}

void Game::on_stack_grouped_or_ungrouped()
{
  //this only happens when we double-click on a stack on the bigmap.
  update_stack_info();
  update_actions();
}

void Game::on_stack_selected()
{
  if (input_locked)
    return;
  //this one is on the way back from the ui
  update_stack_info();
  update_actions();
}

void Game::on_select_stack (Stack *s)
{
  (void )s;
  //this one is on way to the ui
  bigmap->select_active_stack ();
}

void Game::on_deselect_stack ()
{
  bigmap->unselect_active_stack ();
}

void Game::on_city_queried (Vector<int> pos, City *c)
{
  MapTipPosition mpos = bigmap->map_tip_position(pos);
  m_city_tip_changed.emit(c, mpos);
}

void Game::on_city_unqueried ()
{
  m_city_tip_changed.emit(NULL, MapTipPosition());
}

void Game::on_city_visited(City* c)
{
  if (c)
    {
      m_city_visited.emit
        (c,
         [this] ()
         {
           redraw ();
         });
    }
}

void Game::on_ruin_unqueried ()
{
  MapTipPosition mpos = {};
  m_map_tip_changed.emit("", mpos);
}

void Game::on_ruin_queried (Ruin *r, Vector<int> pos)
{
  Glib::ustring str = r->getName();
  str += "\n";
  if (r->isSearched())
    // note to translators: whether a ruin has been searched
    str += _("Explored");
  else
    // note to translators: whether a ruin has been searched
    str += _("Unexplored");

  MapTipPosition mpos = bigmap->map_tip_position(pos);
  m_map_tip_changed.emit(str, mpos);
}

void Game::on_ruin_visited (Ruin* r)
{
  m_ruin_visited.emit(r);
}

void Game::on_signpost_queried (Signpost* s, Vector<int> pos)
{
  Glib::ustring str = s->getName();
  MapTipPosition mpos = bigmap->map_tip_position(pos);
  m_map_tip_changed.emit(str, mpos);
}

void Game::on_signpost_unqueried ()
{
  MapTipPosition mpos = {};
  m_map_tip_changed.emit("", mpos);
}

void Game::on_stack_unqueried ()
{
  MapTipPosition mpos = {};
  m_stack_tip_changed.emit(NULL, mpos);
}

void Game::on_stack_queried (Stack *stack, Vector<int> pos)
{
  MapTipPosition mpos = bigmap->map_tip_position(pos);
  m_stack_tip_changed.emit (GameMap::getStacks (stack->getPos ()), mpos);
}

void Game::on_temple_visited (Temple* t)
{
  m_temple_visited.emit(t);
}

void Game::on_temple_queried (Temple* t, Vector<int> pos)
{
  Glib::ustring str = t->getName();
  MapTipPosition mpos = bigmap->map_tip_position(pos);
  m_map_tip_changed.emit(str, mpos);
}

void Game::on_temple_unqueried ()
{
  MapTipPosition mpos = {};
  m_map_tip_changed.emit("", mpos);
}

void Game::on_city_defeated (City *city, Stack *s, sigc::slot<void(CityDefeatedChoice)> after)
{
  redraw ();
  Player *player = Playerlist::instance ()->getActiveplayer ();
  if (player->isHuman ())
    m_city_defeated.emit (city, after);
  else if (player->isComputer ())
    {
      player->invadeCity (city);
      after (player->chooseCityDefeatedAction (city, s));
    }
}

void Game::on_open_city_dialog (City *city, sigc::slot<void()> after)
{
  Player *player = Playerlist::instance ()->getActiveplayer ();
  if (player->isHuman ())
    m_open_city_dialog.emit (city, after);
  else if (player->isComputer ())
    after ();
}

void Game::on_looting_city (int gold, sigc::slot<void()> after)
{
  Player *player = Playerlist::instance ()->getActiveplayer ();
  
  if (player->isHuman ())
    m_looting_city (gold, after);
  else if (player->isComputer ())
    after ();

  redraw ();
  update_stack_info ();
  update_sidebar_stats ();
  update_actions ();
}

void Game::on_city_pillaged (City *c, int gold, int pillaged_type, sigc::slot<void()> after)
{
  Player *player = Playerlist::instance ()->getActiveplayer ();
  if (player->isHuman ())
    m_city_pillaged.emit (c, gold, pillaged_type, after);
  else if (player->isComputer ())
    after ();
}

void Game::on_city_sacked (City *c, int gold, std::list<guint32> sacked_types, sigc::slot<void()> after)
{
  Player *player = Playerlist::instance ()->getActiveplayer ();
  if (player->isHuman ())
    m_city_sacked.emit (c, gold, sacked_types, after);
  else if (player->isComputer ())
    after ();
}

void Game::on_city_razed (City *c, sigc::slot<void()> after)
{
  Player *player = Playerlist::instance ()->getActiveplayer ();
  if (player->isHuman ())
    m_city_razed.emit (c, after);
  else if (player->isComputer ())
    after ();
}

void Game::on_city_raze_query (City *c, sigc::slot<void(bool)> after)
{
  Player *player = Playerlist::instance ()->getActiveplayer ();
  if (player->isHuman ())
    m_city_raze_query.emit (c, after);
  else if (player->isComputer ())
    after (true);
}

void Game::lock_inputs()
{
  // don't accept modifying user input from now on
  bigmap->set_input_locked(true);
  smallmap->set_input_locked(true);
  input_locked = true;
  update_actions();
}

void Game::unlock_inputs()
{
  bigmap->set_input_locked(false);
  smallmap->set_input_locked(false);
  input_locked = false;
  update_actions();
}

void Game::update_actions()
{
  if (input_locked)
    {
      m_can_select_next_movable_stack.emit(false);
      m_can_center_selected_stack.emit(false);
      m_can_defend_selected_stack.emit(false);
      m_can_park_selected_stack.emit(false);
      m_can_deselect_selected_stack.emit(false);
      m_can_inspect.emit(false);
      m_can_see_hero_levels.emit(false);
      m_can_search_selected_stack.emit(false);
      m_can_use_item.emit(false);
      m_can_plant_standard_selected_stack.emit(false);
      m_can_move_selected_stack_along_path.emit(false);
      m_can_move_all_stacks.emit(false);
      m_can_group_ungroup_selected_stack.emit(false);
      m_can_end_turn.emit(false);
      m_can_disband_stack.emit(false);
      m_can_change_signpost.emit(false);
      m_can_see_city_history.emit(false);
      m_can_see_ruin_history.emit(false);
      m_can_see_event_history.emit(false);
      m_can_see_winning_history.emit(false);
      m_can_see_gold_history.emit(false);
      m_can_see_triumph_history.emit(false);
      m_can_see_diplomacy_report.emit(false);
      m_can_see_army_report.emit(false);
      m_can_see_city_report.emit(false);
      m_can_see_gold_report.emit(false);
      m_can_see_production_report.emit(false);
      m_can_see_winning_report.emit(false);
      m_can_see_quest_report.emit(false);
      m_can_see_items_report.emit(false);
      m_can_save_game.emit (false);
      m_can_load_game.emit (false);
      m_can_new_game.emit (false);
      m_can_change_fight_order.emit (false);
      m_can_resign.emit (false);
      m_can_see_view_menu_army_bonus.emit (false);
      m_can_see_view_menu_items.emit (false);
      m_can_see_view_menu_cities.emit (false);
      m_can_see_view_menu_vectoring.emit (false);
      m_can_see_view_menu_ruins.emit (false);
      m_can_see_view_menu_stack.emit (false);
      m_can_see_view_menu_diplomacy.emit (false);
      m_can_launch_tutorial_video.emit (false);
      m_can_launch_online_help.emit (false);
      m_can_see_about_dialog.emit (false);
      m_can_see_keyboard_shortcuts_dialog.emit (false);

      return;
    }

  Player *player = Playerlist::getActiveplayer();
  Stacklist* sl = player->getStacklist();

  bool all_defending_or_parked = true;
  for (Stacklist::iterator i = sl->begin(); i != sl->end(); ++i)
    if (!(*i)->getDefending() && !(*i)->getParked() &&
	*i != sl->getActivestack())
      {
	all_defending_or_parked = false;
	break;
      }

  bool all_immobile = true;
  for (Stacklist::iterator i = sl->begin(); i != sl->end(); ++i)
    if (!(*i)->getDefending() && !(*i)->getParked() && (*i)->canMove() &&
        *i != sl->getActivestack())
      {
	all_immobile = false;
	break;
      }
  m_can_select_next_movable_stack.emit(!all_defending_or_parked && !all_immobile);

  // if any stack can move, enable the moveall button
  m_can_move_all_stacks.emit(sl->enoughMoves());

  Stack *stack = player->getActivestack();

  m_can_park_selected_stack.emit(stack != 0);
  m_can_deselect_selected_stack.emit(stack != 0);
  m_can_center_selected_stack.emit(stack != 0);
  m_can_inspect.emit(Playerlist::getActiveplayer()->getHeroes().size() > 0);
  m_can_see_hero_levels.emit(Playerlist::getActiveplayer()->getHeroes().size() > 0);

  if (stack)
    {
      m_can_move_selected_stack_along_path.emit
	((!stack->getPath()->empty() && stack->enoughMoves()) ||
	 (!stack->getPath()->empty() && stack->getPath()->getMovesExhaustedAtPoint() > 0));

      m_can_plant_standard_selected_stack.emit(GameMap::can_plant_flag(stack));

      m_can_search_selected_stack.emit(GameMap::can_search(stack));

      m_can_use_item.emit(player->hasUsableItem());

      if (GameMap::getSignpost(stack))
	m_can_change_signpost.emit(true);

      m_can_disband_stack.emit(true);
      m_can_group_ungroup_selected_stack.emit(true);
      //we can't defend on cities, ruins, temples, ports, or water.
      m_can_defend_selected_stack.emit(GameMap::can_defend(stack));
    }
  else
    {
      m_can_move_selected_stack_along_path.emit(false);
      m_can_disband_stack.emit(false);
      m_can_group_ungroup_selected_stack.emit(false);
      m_can_plant_standard_selected_stack.emit(false);
      m_can_search_selected_stack.emit(false);
      m_can_defend_selected_stack.emit(false);
      m_can_change_signpost.emit(false);
      m_can_use_item.emit(false);
    }
      
  bool can_see_history = d_gameScenario->getRound () > 1;
  m_can_see_city_history.emit (can_see_history);
  m_can_see_ruin_history.emit (can_see_history);
  m_can_see_event_history.emit (can_see_history);
  m_can_see_winning_history.emit (can_see_history);
  m_can_see_gold_history.emit (can_see_history);
  m_can_see_triumph_history.emit (can_see_history);
    
  m_can_see_diplomacy_report.emit(GameScenarioOptions::s_diplomacy);
  m_can_see_army_report.emit (true);
  m_can_see_city_report.emit (true);
  m_can_see_gold_report.emit (true);
  m_can_see_production_report.emit (true);
  m_can_see_winning_report.emit (true);
  m_can_see_quest_report.emit
    (GameScenarioOptions::s_play_with_quests != GameParameters::NO_QUESTING);
  m_can_see_items_report.emit (true);

  m_can_see_view_menu_army_bonus.emit (true);
  m_can_see_view_menu_items.emit (true);
  m_can_see_view_menu_cities.emit (true);
  m_can_see_view_menu_vectoring.emit (true);
  m_can_see_view_menu_ruins.emit (true);
  m_can_see_view_menu_stack.emit (stack != NULL);
  m_can_see_view_menu_diplomacy.emit (GameScenarioOptions::s_diplomacy);

  bool networked = d_gameScenario->getPlayMode () == GameScenario::NETWORKED;
  m_can_save_game.emit (!networked);
  m_can_load_game.emit (!networked);
  m_can_new_game.emit (!networked);
  m_can_change_fight_order.emit (true);
  m_can_resign.emit (true);

  m_can_launch_tutorial_video.emit (true);
  m_can_launch_online_help.emit (true);
  m_can_see_about_dialog.emit (true);
  m_can_see_keyboard_shortcuts_dialog.emit (true);

  if (Playerlist::instance()->countPlayersAlive() <= 1)
    m_can_end_turn.emit(false);
  else
    m_can_end_turn.emit(true);

  m_can_show_lobby.emit
    (d_gameScenario->getPlayMode () == GameScenario::NETWORKED);
}

MapWidget* Game::get_bigmap()
{
  assert(bigmap.get());
  return bigmap.get();
}

SmallMap &Game::get_smallmap()
{
  assert(smallmap.get());
  return *smallmap.get();
}

void Game::startGame()
{
  debug ("start_game()");
      
  center_view_on_city();
  update_sidebar_stats();
  update_actions();
  update_stack_info();
  lock_inputs();

  if (d_gameScenario->getPlayMode() != GameScenario::NETWORKED)
    d_nextTurn->start ();
      
  if (Playerlist::instance()->countPlayersAlive())
    update_actions();
}

void Game::loadGame()
{
  Player *player = Playerlist::getActiveplayer();
  if (!player)
    {
      Playerlist::instance()->nextPlayer();
      player = Playerlist::getActiveplayer();
    }

  if (player->getType() == Player::HUMAN && (d_gameScenario->getPlayMode() == GameScenario::HOTSEAT))
    {
      //human players want access to the controls and an info box
      unlock_inputs();
      player->stackDeselect ();
      center_view_on_city();
      update_sidebar_stats();
      update_actions();
      update_stack_info();
      m_game_loaded.emit(player);
      if (player->getType() == Player::HUMAN)
	d_nextTurn->setContinuingTurn();
    }
  else
    lock_inputs();

  d_nextTurn->start ();
}

void Game::stopGame()
{
  d_nextTurn->stop();
}

bool Game::saveGame(Glib::ustring file)
{
  return d_gameScenario->saveGame(file);
}

void Game::blank(bool on)
{
  if (GameScenarioOptions::s_hidden_map == true)
    {
      bigmap->blank(on);
      smallmap->blank(on);
    }
}

void Game::init_turn_after_city_visited (Player *p)
{
  if (p->isComputer ())
    init_turn_after_offer_surrender (p);
  else
    offer_surrender
      (p,
       [this, p] ()
       {
         init_turn_after_offer_surrender (p);
       });
}

void Game::init_turn_after_city_too_poor_to_produce (Player *p)
{
  if (p->isComputer ())
    init_turn_after_city_visited (p);
  else
    {
      if (p->countEndTurnHistoryEntries () == 1 &&
          Lw::app->m_own_all_on_round_two)
        p->conquerAllCities ();

      if (d_gameScenario->getRound () == 1)
        m_city_visited.emit
          (p->getFirstCity (),
           [this, p] ()
           {
             init_turn_after_city_visited (p);
           });
      else
        init_turn_after_city_visited (p);
    }
}

void Game::init_turn_after_recruit_hero (Player *p, int num_allies)
{
  if (num_allies)
    {
      if (p->isComputer())
        init_turn_after_hero_brings_allies (p);
      else
        {
          m_hero_brings_allies.emit
            (num_allies,
             [this, p] ()
             {
               init_turn_after_hero_brings_allies (p);
             });
        }
    }
  else
    init_turn_after_hero_brings_allies (p);
}

void Game::init_turn_after_hero_brings_allies (Player *p)
{
  if (p->getType () == Player::HUMAN)
    {
      unlock_inputs ();

      update_sidebar_stats ();
      update_stack_info ();
      update_actions ();
      redraw ();

      // update the diplomacy icon if we've received a proposal
      bool proposal_received = false;
      for (auto it: *Playerlist::instance ())
        {
          if (it == Playerlist::getNeutral ())
            continue;
          if (it == p)
            continue;
          if(it->isDead ())
            continue;
          if (it->getDiplomaticProposal (p) != Player::NO_PROPOSAL)
            {
              proposal_received = true;
              break;
            }
        }
      m_received_diplomatic_proposal.emit (proposal_received);

      if (!QuestsManager::instance ()->notifyQuestExpired
          (p,
           [this, p] ()
           {
             init_turn_after_quest_expiry (p);
           }))
      init_turn_after_quest_expiry (p);
    }
  else
    init_turn_after_quest_expiry (p);
}

void Game::init_turn_after_quest_expiry (Player *p)
{
  if (p->isComputer ())
    init_turn_after_city_too_poor_to_produce (p);
  else
    {
      //check to see if we've turned off production due to destitution.
      if (p->countDestituteCitiesThisTurn () > 0)
        m_city_too_poor_to_produce.emit
          ([this, p] ()
           {
             init_turn_after_city_too_poor_to_produce (p);
           });
      else
        init_turn_after_city_too_poor_to_produce (p);
    }
}

void Game::init_turn_after_commentator_comments (Player *p)
{
  p->maybeRecruitHero
    ([this, p] (int num_allies)
     {
       init_turn_after_recruit_hero (p, num_allies);
     });
}

void Game::init_turn_after_next_turn (Player *p)
{
  if (p->getType () == Player::NETWORKED)
    {
      m_remote_next_player_turn.emit ();
      return;
    }
  blank (false);

  if (p->isObservable () == true)
    center_view_on_city ();

  if (p->getType () == Player::HUMAN)
    {
      if (Commentator::instance ()->hasComment () == true)
        {
          auto comments = Commentator::instance ()->getComments (p);
          if (comments.size () > 0)
            {
              auto comment = comments[Rnd::rand () % comments.size ()];
              m_commentator_comments.emit
                (comment,
                 [this, p] ()
                 {
                   init_turn_after_commentator_comments (p);
                 });
            }
          else
            init_turn_after_commentator_comments (p);
        }
      else
        init_turn_after_commentator_comments (p);
    }
  else
    init_turn_after_commentator_comments (p);

}

void Game::init_turn (Player* p)
{
  m_turn_begins.emit ();
  blank (true);

  if (p->getType() == Player::NETWORKED)
    {
      m_remote_next_player_turn.emit();
      return;
    }
  if (m_next_turn.empty ())
    init_turn_after_next_turn (p);
  else
    m_next_turn.emit
      (p,
       [this, p] ()
       {
         init_turn_after_next_turn (p);
       });
}

void Game::on_player_died (Player *player, std::shared_ptr<sigc::slot<void()>> after)
{
  if (Playerlist::instance ()->getNoOfPlayers () <= 1)
    m_game_over.emit
      (Playerlist::getFirstLiving (),
       [] ()
       {
         /* empty */
       });
  else
    {
      if (m_player_died.empty ())
        (*after) ();
      else
        m_player_died.emit (player, after);
    }
}

void Game::on_fight_started(Fight *fight, sigc::slot<void(Fight*)> finish)
{
  //don't show the battle if the ai is attacking neutral
  bool ai_attacking_neutral = false;
  auto defender = fight->getDefenders ().front ();
  auto attacker = fight->getAttackers ().front ();

  if (defender->getOwner () == Playerlist::getNeutral () &&
      Playerlist::getActiveplayer ()->getType () != Player::HUMAN)
    ai_attacking_neutral = true;

  //show the battle if we're attacking an observable player
  bool attacking_observable_player = false;
  if (defender->getOwner ()->isObservable ())
    attacking_observable_player = true;

  //don't show the battle if we're ai and we're on a hidden map
  bool ai_attacking_on_hidden_map = false;
  if (attacker->getOwner ()->getType () != Player::HUMAN &&
      GameScenario::s_hidden_map == true)
    ai_attacking_on_hidden_map = true;

  if ((Playerlist::getActiveplayer ()->isObservable () == true ||
      attacking_observable_player) && !ai_attacking_neutral &&
      !ai_attacking_on_hidden_map)
    {
      if (GameScenario::s_hidden_map == false)
        smallmap->center_view_on_tile (attacker->getPos (), true);
      //sometimes we don't have a gui
      if (m_fight_started.empty () == true)
        finish (fight);
      else
        m_fight_started.emit (fight, finish);
    }
  else if ((Playerlist::getActiveplayer ()->isObservable () == true ||
      attacking_observable_player) && ai_attacking_neutral &&
      !ai_attacking_on_hidden_map)
    {
      if (GameScenario::s_hidden_map == false)
        smallmap->center_view_on_tile (attacker->getPos (), true);
      //sometimes we don't have a gui
      if (m_abbreviated_fight_started.empty () == true)
        finish (fight);
      else
        m_abbreviated_fight_started.emit (fight, finish);
    }
}

void Game::center_view_on_city()
{
  const Player* p = Playerlist::instance()->getActiveplayer();

  if (p == Playerlist::getNeutral())
    return;
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN &&
      GameScenario::s_hidden_map == true)
    return;
  //FIXME: if player is not to be observed, bail now
  // preferred city is a capital city that belongs to the player 
  for (Citylist::iterator i = Citylist::instance()->begin();
       i != Citylist::instance()->end(); ++i)
    {
      City *c = *i;
      if (c->getOwner() == p && c->isCapital() &&
	  c->getCapitalOwner() == p)
	{
	  smallmap->center_view_on_tile(c->getPos(), 
					!GameScenario::s_hidden_map);
	  return;
	}
    }

  // okay, then find any city that belongs to the player and center on it
  for (Citylist::iterator i = Citylist::instance()->begin();
       i != Citylist::instance()->end(); ++i)
    {
      City *c = *i;
      if (c->getOwner() == p)
	{
	  smallmap->center_view_on_tile(c->getPos(), 
					!GameScenario::s_hidden_map);
	  break;
	}
    }
}

void Game::select_active_stack()
{
  //if (Playerlist::getActiveplayer()->getType() != Player::HUMAN &&
      //GameScenario::s_hidden_map == true)
    //return;
  Player *p = Playerlist::instance()->getActiveplayer();
  smallmap->center_view_on_tile (p->getActivestack()->getPos(), true);
  bigmap->select_active_stack();
}

void Game::unselect_active_stack()
{
  bigmap->unselect_active_stack();
}

void Game::maybeTreachery(Stack *stack, Player *them, Vector<int> pos, sigc::slot<void(bool)> finish)
{
  Player *me = stack->getOwner();
  if (me->isComputer())
    {
      bool treachery = false;
      if (me->getType() == Player::AI_FAST)
        {
          AI_Fast *ai = dynamic_cast<AI_Fast*>(me);
          treachery = ai->chooseTreachery (stack, them, pos);
        }
      else if (me->getType() == Player::AI_SMART)
        {
          AI_Smart *ai = dynamic_cast<AI_Smart*>(me);
          treachery = ai->chooseTreachery (stack, them, pos);
        }
      finish (treachery);
    }
  else if (me->isHuman ())
    {
      m_stack_considers_treachery.emit
        (them,
         [me, them, finish] (bool treachery)
         {
           if (treachery)
             {
               me->proposeDiplomacy (Player::NO_PROPOSAL, them);
               me->declareDiplomacy (Player::AT_WAR, them, true);
               them->proposeDiplomacy (Player::NO_PROPOSAL, me);
               them->declareDiplomacy (Player::AT_WAR, me, false);

               me->deteriorateDiplomaticRelationship (5);
               them->improveDiplomaticRelationship (2, me);
             }
           finish (treachery);
         });
    }

  return;
}

void Game::init_turn_after_offer_surrender (Player *p)
{
  (void)p;
  //end of chain
}

void Game::offer_surrender (Player *p, sigc::slot<void()> after)
{
  // offer surrender
  bool &surrender_already_offered =
    GameScenarioOptions::s_surrender_already_offered;
  if (Playerlist::instance ()->countHumanPlayersAlive () == 1 &&
      surrender_already_offered == false &&
      p->getType () == Player::HUMAN)
    {
      int target_level = Citylist::instance ()->size () / 2;
      if (Citylist::instance ()->countCities (p) > target_level)
        {
          surrender_already_offered = true;
          on_surrender_offered (p, after);
        }
      else
        after ();
    }
  else
    after ();
}

void Game::nextRound ()
{
  if (d_gameScenario->getPlayMode () == GameScenario::NETWORKED &&
      GameServer::instance ()->isRunning () == false)
    return;

  //we do this to prevent a bunch of records going into history
  //the first time thru, only the neutral player has ended a turn
  guint32 count = 0;
  for (auto p : *Playerlist::instance ())
    count += p->countEndTurnHistoryEntries ();

  if (count == 1)
    return;

  if (GameScenarioOptions::s_diplomacy)
    {
      Playerlist::instance ()->negotiateDiplomacy ();
      Playerlist::instance ()->calculateDiplomaticRankings ();
    }

  // update winners
  Playerlist::instance ()->calculateWinners ();
}

void Game::on_surrender_offered (Player *recipient, sigc::slot<void()> after)
{
  m_enemy_offers_surrender.emit
    (Playerlist::instance ()->countPlayersAlive () - 1,
     [this, recipient, after] (bool accepted)
     {
       m_surrender_answered.emit
         (accepted,
          [this, accepted, recipient, after] (bool)
          {
            if (accepted)
              {
                Playerlist::instance ()->surrender ();
                m_game_over.emit (recipient, after);
              }
            else
              after ();
          });
     });
}

void Game::recalculate_moves_for_stack(Stack *s)
{
  if (!s)
    s = Playerlist::getActiveplayer()->getActivestack();
  if (s)
    {
      s->getPath()->recalculate(s);
      redraw();
      update_actions();
    }
}
    
void Game::on_city_fight_finished(City *city, FightResult::Outcome result)
{
  if (result != FightResult::ATTACKER_WON)
    {
      // we didn't suceed in defeating the defenders
      //if this is a neutral city, and we're playing with 
      //active neutral cities, AND it hasn't already been attacked
      //then it's production gets turned on
      Player *neu = city->getOwner(); //neutral player
      if (GameScenario::s_neutral_cities == GameParameters::ACTIVE &&
	  neu == Playerlist::getNeutral() &&
	  city->getActiveProductionSlot() == -1)
	{
	  //great, then let's turn on the production.
	  //well, we already made a unit, and we want to produce more
	  //of it.
	  Stack *o = GameMap::getStacks(city->getPos())->getFriendlyStack(neu);
	  if (o)
	    {
	      int army_type = o->getStrongestArmy()->getTypeId();
	      for (guint32 i = 0; i < city->getMaxNoOfProductionBases(); i++)
		{
		  if (city->getArmytype(i) == army_type)
		    {
		      // hey, we found the droid we were looking for
		      city->setActiveProductionSlot(i);
		      break;
		    }
		}
	    }
	}
    }
  return;
}
    
void Game::recruitHero(HeroProto *hero, City *city, int gold, sigc::slot<void(bool,Glib::ustring,Hero::Gender)> finish)
{
  if (city->getOwner()->isComputer())
    {
      bool accepted = city->getOwner ()->chooseHero (hero, city, gold);
      finish (accepted, hero->getName (), Hero::Gender (hero->getGender ()));
    }
  else if (city->getOwner ()->isHuman ())
    {
      update_sidebar_stats ();
      m_hero_offers_service.emit (city->getOwner(), hero, city, gold, finish);
    }
  return;
}
    
void Game::inhibitAutosaveRemoval(bool inhibit)
{
  if (d_gameScenario)
    d_gameScenario->inhibitAutosaveRemoval(inhibit);
}

void Game::endOfGameRoaming(Player *winner)
{
  Playerlist::instance()->setWinningPlayer(winner);
  Playerlist::getActiveplayer()->immobilize();
  d_gameScenario->s_see_opponents_stacks = true;
  d_gameScenario->s_see_opponents_production = true;
  GameScenarioOptions::s_see_opponents_stacks = true;
  GameScenarioOptions::s_see_opponents_production = true;
  center_view_on_city();

  unlock_inputs();

  update_sidebar_stats();
  update_stack_info();
  update_actions();
  redraw();
}

void Game::stack_arrives_on_tile(Stack *stack, Vector<int> tile)
{
  StackTile *stile = GameMap::instance()->getTile(tile)->getStacks();
  stile->arriving(stack);
}

void Game::stack_leaves_tile(Stack *stack, Vector<int> tile)
{
  StackTile *stile = GameMap::instance()->getTile(tile)->getStacks();
  bool left = stile->leaving(stack);
  if (left == false)
    {
      if (stack == NULL)
	{
	  printf("stack is %p\n", (void*)stack);
	  printf("WTFFF!!!!!!!!!!!!!!!!!!!!\n");
	  return;
	}
    }
}

void Game::stack_searches_ruin(Stack *stack, sigc::slot<void(bool)> after)
{
  search_stack (stack,
                [after] (bool got_quest, int num_blessed, bool stack_died)
                {
                  //we use this to search a temple or a ruin
                  //so we ignore got quest
                  (void) got_quest;
                  (void) num_blessed;
                  after (stack_died);
                });
  return;
}
    
void Game::stack_searches_temple(Stack *stack, sigc::slot<void(bool,int)> after)
{
  search_stack (stack,
                [after] (bool got_quest, int num_blessed, bool /*stack_died*/)
                {
                  //we can't die when searching a temple so we ignore it
                  after (got_quest, num_blessed);
                });
  return;
}

void Game::on_ruinfight (Glib::ustring hero_name, Glib::ustring keeper_name, FightResult result, sigc::slot<void()> after)
{
  Player *p = Playerlist::getActiveplayer ();
  if (p->isHuman ())
    m_ruinfight.emit (hero_name, keeper_name, result, after);
  else if (p->isComputer ())
    {
      auto heroes = result.get_advancing_heroes ();
      for (auto h : heroes)
        p->heroGainsLevel (h, p->chooseStat (h));
      after ();
    }
}

void Game::on_save_game(Glib::ustring filename)
{
  if (getScenario())
    getScenario()->saveGame(filename);
}

guint32 Game::on_get_round()
{
  if (getScenario())
    return getScenario()->getRound();
  else
    return 0;
}

void Game::on_bag_dropped ()
{
  redraw ();
}

void Game::on_stack_died ()
{
  redraw ();
}

void Game::on_quest_expired (Quest *q, sigc::slot<void()> finish)
{
  if (Playerlist::getActiveplayer ()->isHuman ())
    m_quest_expired.emit (q, finish);
  else
    finish ();
}

void Game::on_quest_completed (Quest *q, sigc::slot<void()> finish)
{
  if (Playerlist::getActiveplayer ()->isHuman ())
    m_quest_completed.emit (q, finish);
  else
    finish ();
}
    
void Game::hero_plant_standard ()
{
  auto p = Playerlist::getActiveplayer ();
  auto s = p->getActivestack ();
  if (s)
    p->heroPlantStandard (s);
}
