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

#pragma once
#ifndef GAME_H
#define GAME_H

#include <sigc++/signal.h>
#include <glibmm/ustring.h>
#include <sigc++/connection.h>
#include <memory>
#include <list>

#include "sidebar-stats.h"
#include "map-tip-position.h"
#include "callback-enums.h"
#include "army.h"
#include "fight.h"
#include "player.h"
#include "select-city-map.h"

class NextTurn;
class MapWidget;
class SmallMap;
class GameScenario;
class Hero;
class City;
class Stack;
class Player;
class Temple;
class Ruin;
class Signpost;
class Fight;
class Quest;
class Reward;
class StackTile;
class Sage;

//! Connects the various game classes with the GameWindow through signals.
/** Controls a game.
  * 
  * Manages the big and small map, the game scenario and who's turn it is, etc.
  * It's mostly a puppeteer class that connects the various other classes with
  * signals and callbacks. 
  *
  */
class Game
{
 public:
    Game(GameScenario* gameScenario, NextTurn *nextTurn, bool headless = false);
    ~Game();

    void redraw();
    void blank(bool on);

    void select_next_movable_stack();
    void center_selected_stack();
    void defend_selected_stack();
    void park_selected_stack();
    void deselect_selected_stack();
    void search_selected_stack();
    using SearchStackCallback = sigc::slot<void(bool got_quest, int num_blessed, bool died)>;
    void search_stack(Stack *stack, SearchStackCallback after);
    void stack_search_after_ruinfight (Ruin *ruin, Reward *reward, bool stack_died, Stack *stack, SearchStackCallback after);
    void stack_search_after_sage (Reward *reward, Stack *stack, Ruin *ruin, sigc::slot<void()> finish);
    void move_selected_stack_along_path();
    void move_all_stacks();
    void end_turn();
    void hero_plant_standard ();
    void recalculate_moves_for_stack(Stack *s);
    void update_sidebar_stats();

    void startGame(); // initiate game flow
    void loadGame();
    void stopGame(); // stop game flow, clean up
    // save current game, returns true if successful
    bool saveGame(Glib::ustring file);

    static GameScenario *getScenario();
    MapWidget* get_bigmap();
    SmallMap &get_smallmap();
    
    // signals
    sigc::signal<void(Vector<int>)> signal_current_map_position ()
      {
        return m_current_map_position;
      }

    sigc::signal<void(Cairo::RefPtr<Cairo::Surface>)> signal_bigmap_changed ()
      {
        return m_bigmap_changed;
      }

    sigc::signal<void(SidebarStats)> signal_sidebar_stats_changed ()
      {
        return m_sidebar_stats_changed;
      }

    sigc::signal<void(Glib::ustring)> signal_progress_status_changed ()
      {
        return m_progress_status_changed;
      }

    sigc::signal<void()> signal_progress_changed ()
      {
        return m_progress_changed;
      }

    sigc::signal<void(bool)> signal_can_select_next_movable_stack ()
      {
        return m_can_select_next_movable_stack;
      }

    sigc::signal<void(bool)> signal_can_center_selected_stack ()
      {
        return m_can_center_selected_stack;
      }

    sigc::signal<void(bool)> signal_can_defend_selected_stack ()
      {
        return m_can_defend_selected_stack;
      }

    sigc::signal<void(bool)> signal_can_park_selected_stack ()
      {
        return m_can_park_selected_stack;
      }

    sigc::signal<void(bool)> signal_can_deselect_selected_stack ()
      {
        return m_can_deselect_selected_stack;
      }

    sigc::signal<void(bool)> signal_can_search_selected_stack ()
      {
        return m_can_search_selected_stack;
      }

    sigc::signal<void(bool)> signal_can_inspect ()
      {
        return m_can_inspect;
      }

    sigc::signal<void(bool)> signal_can_see_hero_levels ()
      {
        return m_can_see_hero_levels;
      }

    sigc::signal<void(bool)> signal_can_use_item ()
      {
        return m_can_use_item;
      }

    sigc::signal<void(bool)> signal_can_plant_standard_selected_stack ()
      {
        return m_can_plant_standard_selected_stack;
      }

    sigc::signal<void(bool)> signal_can_move_selected_stack_along_path ()
      {
        return m_can_move_selected_stack_along_path;
      }

    sigc::signal<void(bool)> signal_can_group_ungroup_selected_stack ()
      {
        return m_can_group_ungroup_selected_stack;
      }

    sigc::signal<void(bool)> signal_can_move_all_stacks ()
      {
        return m_can_move_all_stacks;
      }

    sigc::signal<void(bool)> signal_can_disband_stack ()
      {
        return m_can_disband_stack;
      }

    sigc::signal<void(bool)> signal_can_change_signpost ()
      {
        return m_can_change_signpost;
      }

    sigc::signal<void(bool)> signal_can_see_city_history ()
      {
        return m_can_see_city_history;
      }

    sigc::signal<void(bool)> signal_can_see_ruin_history ()
      {
        return m_can_see_ruin_history;
      }

    sigc::signal<void(bool)> signal_can_see_event_history ()
      {
        return m_can_see_event_history;
      }

    sigc::signal<void(bool)> signal_can_see_winning_history ()
      {
        return m_can_see_winning_history;
      }

    sigc::signal<void(bool)> signal_can_see_gold_history ()
      {
        return m_can_see_gold_history;
      }

    sigc::signal<void(bool)> signal_can_see_triumph_history ()
      {
        return m_can_see_triumph_history;
      }

    sigc::signal<void(bool)> signal_can_save_game ()
      {
        return m_can_save_game;
      }

    sigc::signal<void(bool)> signal_can_load_game ()
      {
        return m_can_load_game;
      }

    sigc::signal<void(bool)> signal_can_new_game ()
      {
        return m_can_new_game;
      }

    sigc::signal<void(bool)> signal_can_change_fight_order ()
      {
        return m_can_change_fight_order;
      }

    sigc::signal<void(bool)> signal_can_resign ()
      {
        return m_can_resign;
      }

    sigc::signal<void(bool)> signal_can_see_diplomacy_report ()
      {
        return m_can_see_diplomacy_report;
      }

    sigc::signal<void(bool)> signal_can_see_view_menu_army_bonus ()
      {
        return m_can_see_view_menu_army_bonus;
      }

    sigc::signal<void(bool)> signal_can_see_view_menu_items ()
      {
        return m_can_see_view_menu_items;
      }

    sigc::signal<void(bool)> signal_can_see_view_menu_cities ()
      {
        return m_can_see_view_menu_cities;
      }

    sigc::signal<void(bool)> signal_can_see_view_menu_vectoring ()
      {
        return m_can_see_view_menu_vectoring;
      }

    sigc::signal<void(bool)> signal_can_see_view_menu_ruins ()
      {
        return m_can_see_view_menu_ruins;
      }

    sigc::signal<void(bool)> signal_can_see_view_menu_stack ()
      {
        return m_can_see_view_menu_stack;
      }

    sigc::signal<void(bool)> signal_can_see_view_menu_diplomacy ()
      {
        return m_can_see_view_menu_diplomacy;
      }

    sigc::signal<void(bool)> signal_can_see_army_report ()
      {
        return m_can_see_army_report;
      }

    sigc::signal<void(bool)> signal_can_see_city_report ()
      {
        return m_can_see_city_report;
      }

    sigc::signal<void(bool)> signal_can_see_gold_report ()
      {
        return m_can_see_gold_report;
      }

    sigc::signal<void(bool)> signal_can_see_production_report ()
      {
        return m_can_see_production_report;
      }

    sigc::signal<void(bool)> signal_can_see_winning_report ()
      {
        return m_can_see_winning_report;
      }

    sigc::signal<void(bool)> signal_can_see_quest_report ()
      {
        return m_can_see_quest_report;
      }

    sigc::signal<void(bool)> signal_can_see_items_report ()
      {
        return m_can_see_items_report;
      }

    sigc::signal<void(bool)> signal_received_diplomatic_proposal ()
      {
        return m_received_diplomatic_proposal;
      }
    sigc::signal<void(bool)> signal_can_launch_tutorial_video ()
      {
        return m_can_launch_tutorial_video;
      }

    sigc::signal<void(bool)> signal_can_launch_online_help ()
      {
        return m_can_launch_online_help;
      }

    sigc::signal<void(bool)> signal_can_see_about_dialog ()
      {
        return m_can_see_about_dialog;
      }

    sigc::signal<void(bool)> signal_can_see_keyboard_shortcuts_dialog ()
      {
        return m_can_see_keyboard_shortcuts_dialog;
      }

    sigc::signal<void(sigc::slot<void()>)> signal_city_too_poor_to_produce ()
      {
        return m_city_too_poor_to_produce;
      }

    sigc::signal<void(bool)> signal_can_end_turn ()
      {
        return m_can_end_turn;
      }

    sigc::signal<void(Stack *)> signal_stack_info_changed ()
      {
        return m_stack_info_changed;
      }

    sigc::signal<void(Glib::ustring, MapTipPosition)> signal_map_tip_changed ()
      {
        return m_map_tip_changed;
      }

    sigc::signal<void(StackTile *, MapTipPosition)> signal_stack_tip_changed ()
      {
        return m_stack_tip_changed;
      }

    sigc::signal<void(City *, MapTipPosition)> signal_city_tip_changed ()
      {
        return m_city_tip_changed;
      }

    sigc::signal<void(Ruin*, Stack*, Reward*)> signal_ruin_searched ()
      {
        return m_ruin_searched;
      }

    sigc::signal<void(Ruin*, Sage*, Stack*, sigc::slot<void(Reward*)>)> signal_sage_visited ()
      {
        return m_sage_visited;
      }

    sigc::signal<void(Fight *, sigc::slot<void(Fight*)>)> signal_fight_started ()
      {
        return m_fight_started;
      }

    sigc::signal<void(Fight *, sigc::slot<void(Fight*)>)> signal_abbreviated_fight_started ()
      {
        return m_abbreviated_fight_started;
      }

    sigc::signal<void(float)> signal_advice_asked ()
      {
        return m_advice_asked;
      }

    sigc::signal<void(Glib::ustring, Glib::ustring, FightResult, sigc::slot<void()>)> signal_ruinfight ()
      {
        return m_ruinfight;
      }

    sigc::signal<void(Player *, HeroProto *, City *, int, sigc::slot<void(bool,Glib::ustring,Hero::Gender)>)> signal_hero_offers_service ()
      {
        return m_hero_offers_service;
      }

    sigc::signal<void(int,sigc::slot<void(bool)>)> signal_enemy_offers_surrender ()
      {
        return m_enemy_offers_surrender;
      }

    sigc::signal<void(bool,sigc::slot<void(bool)>)> signal_surrender_answered ()
      {
        return m_surrender_answered;
      }

    sigc::signal<void(Player*,sigc::slot<void(bool)>)> signal_stack_considers_treachery ()
      {
        return m_stack_considers_treachery;
      }

    sigc::signal<void(Hero *, Temple *, int, SearchStackCallback)> signal_search_temple ()
      {
        return m_search_temple;
      }

    sigc::signal<void(City *, sigc::slot<void(CityDefeatedChoice)>)> signal_city_defeated ()
      {
        return m_city_defeated;
      }

    sigc::signal<void(City *, sigc::slot<void()>)> signal_city_visited ()
      {
        return m_city_visited;
      }

    sigc::signal<void(Ruin *)> signal_ruin_visited ()
      {
        return m_ruin_visited;
      }

    sigc::signal<void(Ruin *)> signal_ruin_queried ()
      {
        return m_ruin_queried;
      }

    sigc::signal<void()> signal_ruin_unqueried ()
      {
        return m_ruin_unqueried;
      }

    sigc::signal<void(Temple *)> signal_temple_visited ()
      {
        return m_temple_visited;
      }

    sigc::signal<void(Temple *)> signal_temple_queried ()
      {
        return m_temple_queried;
      }

    sigc::signal<void()> signal_temple_unqueried ()
      {
        return m_temple_unqueried;
      }

    sigc::signal<void(Player *, sigc::slot<void()>)> signal_next_turn ()
      {
        return m_next_turn;
      }

    sigc::signal<void()> signal_remote_next_player_turn ()
      {
        return m_remote_next_player_turn;
      }

    sigc::signal<void(int,sigc::slot<void()>)> signal_hero_brings_allies ()
      {
        return m_hero_brings_allies;
      }

    sigc::signal<void(Player *)> signal_game_loaded ()
      {
        return m_game_loaded;
      }

    sigc::signal<void(Player *,sigc::slot<void()>)> signal_game_over ()
      {
        return m_game_over;
      }

    sigc::signal<void(Player *,std::shared_ptr<sigc::slot<void()>>)> signal_player_died ()
      {
        return m_player_died;
      }

    sigc::signal<void()> signal_game_stopped ()
      {
        return m_game_stopped;
      }

    sigc::signal<void(Glib::ustring,sigc::slot<void()>)> signal_commentator_comments ()
      {
        return m_commentator_comments;
      }

    sigc::signal<void(Stack*, Vector<int>)> signal_stack_moves ()
      {
        return m_stack_moves;
      }

    sigc::signal<void(Item*)> signal_select_item_victim_player ()
      {
        return m_select_item_victim_player;
      }

    sigc::signal<void(Item*, SelectCityMap::Type)> signal_select_city_to_use_item_on ()
      {
        return m_select_city_to_use_item_on;
      }

    sigc::signal<void(Player*, guint32)> signal_stole_gold ()
      {
        return m_stole_gold;
      }

    sigc::signal<void(Player*, guint32)> signal_sunk_ships ()
      {
        return m_sunk_ships;
      }

    sigc::signal<void(Hero*, guint32)> signal_bags_picked_up ()
      {
        return m_bags_picked_up;
      }

    sigc::signal<void(Hero *, guint32)> signal_mp_added_to_hero_stack ()
      {
        return m_mp_added_to_hero_stack;
      }

    sigc::signal<void(Hero *, Glib::ustring, guint32)> signal_worms_killed ()
      {
        return m_worms_killed;
      }

    sigc::signal<void(Hero *)> signal_bridge_burned ()
      {
        return m_bridge_burned;
      }

    sigc::signal<void(Hero *, Ruin*, Glib::ustring)> signal_keeper_captured ()
      {
        return m_keeper_captured;
      }

    sigc::signal<void(Hero *, Glib::ustring)> signal_monster_summoned ()
      {
        return m_monster_summoned;
      }

    sigc::signal<void(Glib::ustring, guint32)> signal_city_diseased ()
      {
        return m_city_diseased;
      }

    sigc::signal<void(Glib::ustring, Glib::ustring, guint32)> signal_city_defended ()
      {
        return m_city_defended;
      }

    sigc::signal<void(Glib::ustring, guint32)> signal_city_persuaded ()
      {
        return m_city_persuaded;
      }

    sigc::signal<void(Hero *, Glib::ustring)> signal_stack_teleported ()
      {
        return m_stack_teleported;
      }

    sigc::signal<void()> signal_round_begins ()
      {
        return m_round_begins;
      }

    sigc::signal<void()> signal_turn_begins ()
      {
        return m_turn_begins;
      }

    sigc::signal<void (int, sigc::slot<void()>)> signal_looting_city ()
      {
        return m_looting_city;
      }

    sigc::signal<void (City *, sigc::slot<void()>)> signal_open_city_dialog ()
      {
        return m_open_city_dialog;
      }

    sigc::signal<void(City*,int,int,sigc::slot<void()>)> signal_city_pillaged ()
      {
        return m_city_pillaged;
      }

    sigc::signal<void(City*,int,std::list<guint32>,sigc::slot<void()>)> signal_city_sacked ()
      {
        return m_city_sacked;
      }

    sigc::signal<void(City*,sigc::slot<void(bool)>)> signal_city_raze_query ()
      {
        return m_city_raze_query;
      }

    sigc::signal<void(City*,sigc::slot<void()>)> signal_city_razed ()
      {
        return m_city_razed;
      }

    sigc::signal<void(Quest*,sigc::slot<void()>)> signal_quest_expired ()
      {
        return m_quest_expired;
      }

    sigc::signal<void(Quest*,sigc::slot<void()>)> signal_quest_completed ()
      {
        return m_quest_completed;
      }

    sigc::signal<void(bool)> signal_can_show_lobby ()
      {
        return m_can_show_lobby;
      }

    void addPlayer(Player *p);

    void inhibitAutosaveRemoval(bool inhibit);

    void endOfGameRoaming(Player *winner);
    
    void use_item(Item *item);
    void use_item_on_player (Item *item, Player *player);
    void use_item_on_friendly_city (Item *item, City *city);
    void use_item_on_enemy_city (Item *item, City *city);
    void use_item_on_neutral_city (Item *item, City *city);
    void use_item_on_any_city (Item *item, City *city);
 private:
    static Game *current_game;

    // centers the map on a city of the active player
    void center_view_on_city();

    void update_actions ();
    void update_stack_info();	// emit stack_info_changed
    void clear_stack_info();

    // locks/unlocks the input widgets during computer turns
    void lock_inputs();
    void unlock_inputs();

    //! Maybe peform treachery
    void maybeTreachery(Stack *stack, Player *them, Vector<int> pos, sigc::slot<void(bool)> finish);

    // bigmap callbacks
    void on_stack_selected();
    void on_select_stack (Stack *);
    void on_deselect_stack ();
    void on_stack_grouped_or_ungrouped();
    void on_city_visted (City* c);
    void on_ruin_queried (Ruin *r, Vector<int> pos);
    void on_ruin_visited (Ruin* r);
    void on_ruin_unqueried ();
    void on_temple_queried (Temple* t, Vector<int> pos);
    void on_temple_visited (Temple* t);
    void on_temple_unqueried ();
    void on_signpost_queried (Signpost* s, Vector<int> pos);
    void on_signpost_unqueried ();
    void on_stack_queried (Stack *s, Vector<int> pos);
    void on_stack_unqueried ();
    void on_city_visited(City *city); // for city window
    void on_city_queried (Vector<int>, City *city); // for city info tip
    void on_city_unqueried ();
    void on_quest_completed (Quest *q, sigc::slot<void()> finish);
    void on_quest_expired (Quest *q, sigc::slot<void()> finish);

    // smallmap callbacks
    void on_smallmap_changed(Cairo::RefPtr<Cairo::Surface> map);
    void on_bigmap_changed(Cairo::RefPtr<Cairo::Surface> map);
    
    // misc. callbacks
    void on_looting_city (int gold, sigc::slot<void()> after);
    void on_city_pillaged (City *, int gold, int pillaged_type, sigc::slot<void()> after);
    void on_city_sacked (City *c, int gold, std::list<guint32> sacked_types, sigc::slot<void()> after);
    void on_city_raze_query (City *c, sigc::slot<void(bool)> after);
    void on_city_razed (City *c, sigc::slot<void()> after);
    void init_turn(Player* p);
    void on_player_died(Player *p, std::shared_ptr<sigc::slot<void()>> after);
    void stack_searches_ruin(Stack *stack, sigc::slot<void(bool)> after);
    void stack_searches_temple(Stack *stack, sigc::slot<void(bool,int)> after); //got quest
    void on_ruinfight (Glib::ustring hero_name, Glib::ustring keeper_name, FightResult result, sigc::slot<void()> after);

    //! Callback when an army gets a new medal.
    void newMedalArmy(Army* a, int medaltype);
    //! Called whenever a stack has changed, updates the map etc.
    void stackUpdate(Stack* s);
    //! Called whenever players fight
    void on_fight_started(Fight *fight, sigc::slot<void(Fight*)> finish);
    //! Called whenever a player receives an offer of surrender
    void on_surrender_offered(Player *recipient, sigc::slot<void()> after);
    void offer_surrender(Player *p, sigc::slot<void()> after);
    //! Called after a player's stack attacks a city
    void on_city_fight_finished(City *city, FightResult::Outcome result);
    void on_city_defeated (City *city, Stack *s, sigc::slot<void(CityDefeatedChoice)> after);
    void on_open_city_dialog (City *city, sigc::slot<void()> after);
    
    void unselect_active_stack();
    void select_active_stack();
    void recruitHero(HeroProto *hero, City *city, int gold, sigc::slot<void(bool,Glib::ustring,Hero::Gender)> finish);

    void on_stack_grouped(Stack *stack);
    void stack_arrives_on_tile(Stack *stack, Vector<int> tile);
    void stack_leaves_tile(Stack *stack, Vector<int> tile);
    void on_stack_halted(Stack *stack);
    void on_stack_stopped();
    void on_stack_starts_moving();

    bool ask_if_treachery(Stack *stack, Player *them, Vector<int> pos);
    void on_save_game(Glib::ustring filename);
    guint32 on_get_round();

    void on_bag_dropped ();
    void on_stack_died ();

    void nextRound ();
    //the init turn chain of dialogs
    void init_turn_after_next_turn (Player *p);
    void init_turn_after_commentator_comments (Player *p);
    void init_turn_after_recruit_hero (Player *p, int num_allies);
    void init_turn_after_hero_brings_allies (Player *p);
    void init_turn_after_quest_expiry (Player *p);
    void init_turn_after_city_too_poor_to_produce (Player *p);
    void init_turn_after_city_visited (Player *p);
    void init_turn_after_offer_surrender (Player *p);

    GameScenario* d_gameScenario;
    NextTurn* d_nextTurn;
    std::unique_ptr<MapWidget> bigmap;
    std::unique_ptr<SmallMap> smallmap;


    bool input_locked;

    std::list<sigc::connection> connections[MAX_PLAYERS + 1];

    sigc::signal<void(Vector<int>)> m_current_map_position;
    sigc::signal<void(Cairo::RefPtr<Cairo::Surface>)> m_bigmap_changed;
    sigc::signal<void(SidebarStats)> m_sidebar_stats_changed;
    sigc::signal<void(Glib::ustring)> m_progress_status_changed;
    sigc::signal<void()> m_progress_changed;


    //signals to control the sensitivity of menu actions
    sigc::signal<void(bool)> m_can_select_next_movable_stack;
    sigc::signal<void(bool)> m_can_center_selected_stack;
    sigc::signal<void(bool)> m_can_defend_selected_stack;
    sigc::signal<void(bool)> m_can_park_selected_stack;
    sigc::signal<void(bool)> m_can_deselect_selected_stack;
    sigc::signal<void(bool)> m_can_search_selected_stack;
    sigc::signal<void(bool)> m_can_inspect;
    sigc::signal<void(bool)> m_can_see_hero_levels;
    sigc::signal<void(bool)> m_can_use_item;
    sigc::signal<void(bool)> m_can_plant_standard_selected_stack;
    sigc::signal<void(bool)> m_can_move_selected_stack_along_path;
    sigc::signal<void(bool)> m_can_group_ungroup_selected_stack;
    sigc::signal<void(bool)> m_can_move_all_stacks;
    sigc::signal<void(bool)> m_can_disband_stack;
    sigc::signal<void(bool)> m_can_change_signpost;
    sigc::signal<void(bool)> m_can_see_city_history;
    sigc::signal<void(bool)> m_can_see_ruin_history;
    sigc::signal<void(bool)> m_can_see_event_history;
    sigc::signal<void(bool)> m_can_see_winning_history;
    sigc::signal<void(bool)> m_can_see_gold_history;
    sigc::signal<void(bool)> m_can_see_triumph_history;
    sigc::signal<void(bool)> m_can_save_game;
    sigc::signal<void(bool)> m_can_load_game;
    sigc::signal<void(bool)> m_can_new_game;
    sigc::signal<void(bool)> m_can_change_fight_order;
    sigc::signal<void(bool)> m_can_resign;
    sigc::signal<void(bool)> m_can_see_army_report;
    sigc::signal<void(bool)> m_can_see_city_report;
    sigc::signal<void(bool)> m_can_see_gold_report;
    sigc::signal<void(bool)> m_can_see_production_report;
    sigc::signal<void(bool)> m_can_see_winning_report;
    sigc::signal<void(bool)> m_can_see_quest_report;
    sigc::signal<void(bool)> m_can_see_items_report;
    sigc::signal<void(bool)> m_can_see_diplomacy_report;
    sigc::signal<void(bool)> m_can_see_view_menu_army_bonus;
    sigc::signal<void(bool)> m_can_see_view_menu_items;
    sigc::signal<void(bool)> m_can_see_view_menu_cities;
    sigc::signal<void(bool)> m_can_see_view_menu_vectoring;
    sigc::signal<void(bool)> m_can_see_view_menu_ruins;
    sigc::signal<void(bool)> m_can_see_view_menu_stack;
    sigc::signal<void(bool)> m_can_see_view_menu_diplomacy;
    sigc::signal<void(bool)> m_received_diplomatic_proposal;
    sigc::signal<void(bool)> m_can_launch_tutorial_video;
    sigc::signal<void(bool)> m_can_launch_online_help;
    sigc::signal<void(bool)> m_can_see_about_dialog;
    sigc::signal<void(bool)> m_can_see_keyboard_shortcuts_dialog;
    sigc::signal<void(bool)> m_can_end_turn;
    sigc::signal<void(bool)> m_can_show_lobby;

    sigc::signal<void(sigc::slot<void()>)> m_city_too_poor_to_produce;
    sigc::signal<void(Stack *)> m_stack_info_changed;
    sigc::signal<void(Glib::ustring, MapTipPosition)> m_map_tip_changed;
    sigc::signal<void(StackTile *, MapTipPosition)> m_stack_tip_changed;
    sigc::signal<void(City *, MapTipPosition)> m_city_tip_changed;
    sigc::signal<void(Ruin*, Stack*, Reward*)> m_ruin_searched;
    sigc::signal<void(Ruin*, Sage*, Stack*, sigc::slot<void(Reward*)>)> m_sage_visited;
    sigc::signal<void(Fight *, sigc::slot<void(Fight*)>)> m_fight_started;
    sigc::signal<void(Fight*, sigc::slot<void(Fight*)>)> m_abbreviated_fight_started;
    sigc::signal<void(float)> m_advice_asked;
    sigc::signal<void(Glib::ustring, Glib::ustring, FightResult, sigc::slot<void()>)> m_ruinfight;
    sigc::signal<void(Player *, HeroProto *, City *, int, sigc::slot<void(bool,Glib::ustring,Hero::Gender)>)> m_hero_offers_service;
    sigc::signal<void(int,sigc::slot<void(bool)>)> m_enemy_offers_surrender;
    sigc::signal<void(bool,sigc::slot<void(bool)>)> m_surrender_answered;
    sigc::signal<void(Player*,sigc::slot<void(bool)>)> m_stack_considers_treachery;
    sigc::signal<void(Hero *, Temple *, int, SearchStackCallback)> m_search_temple;
    sigc::signal<void(City *, sigc::slot<void(CityDefeatedChoice)>)> m_city_defeated;
    sigc::signal<void(City *, sigc::slot<void()>)> m_city_visited;
    sigc::signal<void(Ruin *)> m_ruin_visited;
    sigc::signal<void(Ruin *)> m_ruin_queried;
    sigc::signal<void()> m_ruin_unqueried;
    sigc::signal<void(Temple *)> m_temple_visited;
    sigc::signal<void(Temple *)> m_temple_queried;
    sigc::signal<void()> m_temple_unqueried;
    sigc::signal<void(Player *, sigc::slot<void()>)> m_next_turn;
    sigc::signal<void()> m_remote_next_player_turn;
    sigc::signal<void(int,sigc::slot<void()>)> m_hero_brings_allies;
    sigc::signal<void(Player *)> m_game_loaded;
    sigc::signal<void(Player *, sigc::slot<void()>)> m_game_over;
    sigc::signal<void(Player *, std::shared_ptr<sigc::slot<void()>>)> m_player_died;

    sigc::signal<void()> m_game_stopped;
    sigc::signal<void(Glib::ustring, sigc::slot<void()>)> m_commentator_comments;
    sigc::signal<void(Stack*, Vector<int>)> m_stack_moves;
    sigc::signal<void(Item*)> m_select_item_victim_player;
    sigc::signal<void(Item*, SelectCityMap::Type)> m_select_city_to_use_item_on;
    sigc::signal<void(Player*, guint32)> m_stole_gold;
    sigc::signal<void(Player*, guint32)> m_sunk_ships;
    sigc::signal<void(Hero*, guint32)> m_bags_picked_up;
    sigc::signal<void(Hero *, guint32)> m_mp_added_to_hero_stack;
    sigc::signal<void(Hero *, Glib::ustring, guint32)> m_worms_killed;
    sigc::signal<void(Hero *)> m_bridge_burned;
    sigc::signal<void(Hero *, Ruin*, Glib::ustring)> m_keeper_captured;
    sigc::signal<void(Hero *, Glib::ustring)> m_monster_summoned;
    sigc::signal<void(Glib::ustring, guint32)> m_city_diseased;
    sigc::signal<void(Glib::ustring, Glib::ustring, guint32)> m_city_defended;
    sigc::signal<void(Glib::ustring, guint32)> m_city_persuaded;
    sigc::signal<void(Hero *, Glib::ustring)> m_stack_teleported;
    sigc::signal<void()> m_round_begins;
    sigc::signal<void()> m_turn_begins;
    sigc::signal<void(int, sigc::slot<void()>)> m_looting_city;
    sigc::signal<void(City *, sigc::slot<void()>)> m_open_city_dialog;
    sigc::signal<void(City*,int,int,sigc::slot<void()>)> m_city_pillaged;
    sigc::signal<void(City*,int,std::list<guint32>,sigc::slot<void()>)> m_city_sacked;
    sigc::signal<void(City*,sigc::slot<void(bool)>)> m_city_raze_query;
    sigc::signal<void(City*,sigc::slot<void()>)> m_city_razed;
    sigc::signal<void(Quest*,sigc::slot<void()>)> m_quest_completed;
    sigc::signal<void(Quest*,sigc::slot<void()>)> m_quest_expired;
};

#endif
