// Copyright (C) 2006-2010, 2014, 2015, 2016, 2017, 2020 Ben Asselstine
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
class GameBigMap;
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
    void select_item_to_use();
    void search_stack(Stack *stack, bool &gotquest, bool &stackdied);
    void move_selected_stack_along_path();
    void move_all_stacks();
    void end_turn();
    void recalculate_moves_for_stack(Stack *s);
    void update_sidebar_stats();

    void startGame(); // initiate game flow
    void loadGame();
    void stopGame(); // stop game flow, clean up
    // save current game, returns true if successful
    bool saveGame(Glib::ustring file);

    static GameScenario *getScenario();
    GameBigMap &get_bigmap();
    SmallMap &get_smallmap();
    
    // signals
    sigc::signal<void, Vector<int> > current_map_position;
    sigc::signal<void, Cairo::RefPtr<Cairo::Surface>, Gdk::Rectangle> smallmap_changed;
    sigc::signal<void, Cairo::RefPtr<Cairo::Surface> > bigmap_changed;
    sigc::signal<void, SidebarStats> sidebar_stats_changed;
    sigc::signal<void, Glib::ustring> progress_status_changed;
    sigc::signal<void> progress_changed;
    sigc::signal<void, bool>
	can_select_next_movable_stack,
	can_center_selected_stack,
	can_defend_selected_stack,
	can_park_selected_stack,
	can_deselect_selected_stack,
	can_search_selected_stack,
	can_inspect,
	can_see_hero_levels,
	can_use_item,
	can_plant_standard_selected_stack,
	can_move_selected_stack_along_path,
	can_move_selected_stack,
	can_group_ungroup_selected_stack,
	can_move_all_stacks,
	can_disband_stack,
	can_change_signpost,
	can_see_history,
	can_see_diplomacy,
	received_diplomatic_proposal,
	city_too_poor_to_produce,
	can_end_turn;
    sigc::signal<void, Stack *> stack_info_changed;
    sigc::signal<void, Glib::ustring, MapTipPosition, bool> map_tip_changed;
    sigc::signal<void, StackTile *, MapTipPosition> stack_tip_changed;
    sigc::signal<void,  City *, MapTipPosition> city_tip_changed;
    sigc::signal<void, Ruin*, Stack*, Reward*> ruin_searched;
    sigc::signal<Reward*, Ruin*, Sage*, Stack*> sage_visited;
    sigc::signal<void, LocationBox, Fight &> fight_started;
    sigc::signal<void, LocationBox> abbreviated_fight_started;
    sigc::signal<void, Stack *, Keeper*> ruinfight_started;
    sigc::signal<void, float> advice_asked;
    sigc::signal<void, Fight::Result> ruinfight_finished;
    sigc::signal<bool, Player *, HeroProto *, City *, int> hero_offers_service;
    sigc::signal<bool, int > enemy_offers_surrender;
    sigc::signal<void, bool> surrender_answered;
    sigc::signal<bool, Stack *, Player *, Vector<int> > stack_considers_treachery;
    sigc::signal<bool, Hero *, Temple *, int> temple_searched;
    sigc::signal<void, Hero *, Quest *> quest_assigned;
    sigc::signal<CityDefeatedAction, City *, int> city_defeated;
    sigc::signal<void, City *, int, unsigned int> city_pillaged;
    sigc::signal<void, City *, int, std::list<guint32> > city_sacked;
    sigc::signal<void, City *> city_razed;
    sigc::signal<void, City *> city_visited;
    sigc::signal<void, Ruin *> ruin_visited;
    sigc::signal<void, Temple *> temple_visited;
    sigc::signal<void, Player *, unsigned int> next_player_turn;
    sigc::signal<void> remote_next_player_turn;
    sigc::signal<void, int> hero_arrives;
    sigc::signal<void, Army *, int> medal_awarded_to_army;
    sigc::signal<Army::Stat, Hero *> hero_gains_level;
    sigc::signal<void, Player *> game_loaded;
    sigc::signal<void, Player *> game_over;
    sigc::signal<void, Player *> player_died;
    sigc::signal<void> game_stopped;
    sigc::signal<void, Glib::ustring> commentator_comments;
    sigc::signal<void, Stack*, Vector<int> > stack_moves;
    sigc::signal<Item*, std::list<Item*> > select_item;
    sigc::signal<Player*> select_item_victim_player;
    sigc::signal<City*, SelectCityMap::Type> select_city_to_use_item_on;

    //! Results of using items
    sigc::signal<void, Player*, guint32> stole_gold;
    sigc::signal<void, Player*, guint32> sunk_ships;
    sigc::signal<void, Hero*, guint32> bags_picked_up;
    sigc::signal<void, Hero *, guint32> mp_added_to_hero_stack;
    sigc::signal<void, Hero *, Glib::ustring, guint32> worms_killed;
    sigc::signal<void, Hero *> bridge_burned;
    sigc::signal<void, Hero *, Ruin*, Glib::ustring> keeper_captured;
    sigc::signal<void, Hero *, Glib::ustring> monster_summoned;
    sigc::signal<void, Hero *, Glib::ustring, guint32> city_diseased;
    sigc::signal<void, Hero *, Glib::ustring, Glib::ustring, guint32> city_defended;
    sigc::signal<void, Hero *, Glib::ustring, guint32> city_persuaded;
    sigc::signal<void, Hero *, Glib::ustring> stack_teleported;
    sigc::signal<void, Stack*> popup_stack_actions_menu;
    sigc::signal<void, Vector<int> > pointing_at_new_tile;
    
    void addPlayer(Player *p);

    void inhibitAutosaveRemoval(bool inhibit);

    void endOfGameRoaming(Player *winner);
 private:
    static Game *current_game;

    // centers the map on a city of the active player
    void center_view_on_city();

    void update_control_panel();
    void update_stack_info();	// emit stack_info_changed
    void clear_stack_info();

    // locks/unlocks the input widgets during computer turns
    void lock_inputs();
    void unlock_inputs();

    //! Maybe peform treachery
    bool maybeTreachery(Stack *stack, Player *them, Vector<int> pos);

    // bigmap callbacks
    void on_stack_selected();
    void on_stack_grouped_or_ungrouped();
    void on_city_visted (City* c);
    void on_ruin_queried (Ruin* r, bool brief);
    void on_temple_queried (Temple* t, bool brief);
    void on_signpost_queried (Signpost* s);
    void on_show_path_turns (Vector<int> tile, guint32 turns);
    void on_stack_queried (Vector<int> tile);
    void on_stack_unqueried ();
    void on_city_visited(City *city); // for city window
    void on_city_queried (Vector<int>, City *city); // for city info tip
    void on_city_unqueried ();

    // smallmap callbacks
    void on_smallmap_changed(Cairo::RefPtr<Cairo::Surface> map);
    void on_bigmap_changed(Cairo::RefPtr<Cairo::Surface> map);
    
    // misc. callbacks
    void invading_city(City* city, int gold);
    void init_turn_for_player(Player* p);
    void on_player_died(Player *p);
    bool stack_searches_ruin(Stack *stack);
    bool stack_searches_temple(Stack *stack);
    void on_use_item(Item *item);
    void on_ruinfight_started(Stack *attacker, Keeper *keeper);
    void on_ruinfight_finished(Fight::Result result);

    //! Callback when the army of a human player reaches a new level.
    Army::Stat heroGainsLevel(Hero * a);
    //! Callback when an army gets a new medal.
    void newMedalArmy(Army* a, int medaltype);
    //! Called whenever a stack has changed, updates the map etc.
    void stackUpdate(Stack* s);
    //! Called whenever players fight
    void on_fight_started(Fight &fight);
    //! Called whenever a player receives an offer of surrender
    void on_surrender_offered(Player *recipient);
    void nextRound();
    //! Called after a player's stack attacks a city
    void on_city_fight_finished(City *city, Fight::Result result);
    
    void unselect_active_stack();
    void select_active_stack();
    bool recruitHero(HeroProto *hero, City *city, int gold);

    void on_stack_grouped(Stack *stack);
    void stack_arrives_on_tile(Stack *stack, Vector<int> tile);
    void stack_leaves_tile(Stack *stack, Vector<int> tile);
    void on_stack_halted(Stack *stack);
    void on_stack_stopped();
    void on_stack_starts_moving();

    bool ask_if_treachery(Stack *stack, Player *them, Vector<int> pos);
    void on_save_game(Glib::ustring filename);
    guint32 on_get_round();
    void on_pointing_at_new_tile (Vector<int> tile);

    void on_bag_dropped ();
    void on_stack_died ();

    GameScenario* d_gameScenario;
    NextTurn* d_nextTurn;
    std::unique_ptr<GameBigMap> bigmap;
    std::unique_ptr<SmallMap> smallmap;


    bool input_locked;

    std::list<sigc::connection> connections[MAX_PLAYERS + 1];
};

#endif
