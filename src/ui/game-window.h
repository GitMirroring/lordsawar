//  Copyright (C) 2007, 2008 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2011, 2012, 2014, 2015, 2016, 2017,
//  2020 Ben Asselstine
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
#ifndef GUI_GAME_WINDOW_H
#define GUI_GAME_WINDOW_H

#include <memory>
#include <vector>
#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <sigc++/connection.h>
#include <gtkmm.h>

#include "army-info-tip.h"
#include "city-info-tip.h"
#include "stack-info-tip.h"
#include "game-parameters.h"
#include "sidebar-stats.h"
#include "stack.h"
#include "fight.h"
#include "map-tip-position.h"
#include "callback-enums.h"
#include "vector.h"
#include "army.h"
#include "ImageCache.h"
#include "GameScenario.h"
#include "select-city-map.h"

class Game;
class GameButtonBox;
class Ruin;
class Fight;
class Hero;
class HeroProto;
class Player;
class Temple;
class Quest;
class City;
class Reward;
class Reward_Ruin;
class NextTurn;
class LocationBox;
class StatusBox;
class Profile;
class Keeper;

/** The main window in which all the game action is going on.
  *
  * This class takes cares of the widget side of the things around the action
  * area, delegating the actual drawing to the model classes, but is also
  * responsible for popping up dialog windows in response to the action.
  * It has a pane at the bottom with information about the currently selected
  * stack and a pane to the right with the small map, buttons and statistics. 
  */
class GameWindow: public sigc::trackable
{
 public:
     static double minimum_zoom_scale;
     static double maximum_zoom_scale;
    GameWindow();
    ~GameWindow();

    void show();
    void hide() {window->hide();};

    // initialize the big map widget 
    void init(int width, int height);

    // setup a new game
    void new_game(GameScenario *game_scenario, NextTurn *next_turn);

    // setup a new network game
    void new_network_game(GameScenario *game_scenario, NextTurn *next_turn);

    void continue_network_game(NextTurn *next_turn);
    
    // load the game
    void load_game(GameScenario *game_scenario, NextTurn *next_turn);

    // emitted when the game has ended and it is time to show the splash again
    sigc::signal<void> game_ended;

    // emitted when the game has ended and we want to start a new game.
    sigc::signal<void> game_ended_start_new;
    
    sigc::signal<void> show_lobby;
    sigc::signal<void> show_quick_help;

    sigc::signal<void> quit_requested;

    sigc::signal<void,Glib::ustring,int,Profile*,bool,bool> load_hosted_network_game;

    Gtk::Window *get_window() const {return window;};

    void on_player_replaced(Player *p);
 private:


    Gtk::Window* window;
    Gtk::Window* map_tip;	// tooltip appears over the map
    Gtk::Window* stack_tip;// tooltip appears over the map
    Gtk::Box *bigmap_container;
    Gtk::Box *map_container;
    Gtk::MenuBar *menubar;
    Gtk::CheckMenuItem *fullscreen_menuitem;
    Gtk::MenuItem *new_game_menuitem;
    Gtk::MenuItem *load_game_menuitem;
    Gtk::MenuItem *save_game_menuitem;
    Gtk::MenuItem *save_game_as_menuitem;
    Gtk::MenuItem *show_lobby_menuitem;
    Gtk::MenuItem *end_turn_menuitem;
    Gtk::MenuItem *move_all_menuitem;
    Gtk::MenuItem *search_menuitem;
    Gtk::MenuItem *use_menuitem;
    Gtk::MenuItem *disband_menuitem;
    Gtk::MenuItem *stack_info_menuitem;
    Gtk::MenuItem *signpost_menuitem;
    Gtk::MenuItem *diplomacy_report_menuitem;
    Gtk::MenuItem *item_report_menuitem;
    Gtk::MenuItem *city_history_menuitem;
    Gtk::MenuItem *ruin_history_menuitem;
    Gtk::MenuItem *event_history_menuitem;
    Gtk::MenuItem *gold_history_menuitem;
    Gtk::MenuItem *winner_history_menuitem;
    Gtk::MenuItem *plant_standard_menuitem;
    Gtk::MenuItem *group_ungroup_menuitem;
    Gtk::MenuItem *leave_menuitem;
    Gtk::MenuItem *next_menuitem;
    Gtk::MenuItem *fight_order_menuitem;
    Gtk::MenuItem *resign_menuitem;
    Gtk::MenuItem *production_menuitem;
    Gtk::MenuItem *cities_menuitem;
    Gtk::MenuItem *build_menuitem;
    Gtk::MenuItem *vectoring_menuitem;
    Gtk::MenuItem *levels_menuitem;
    Gtk::MenuItem *inspect_menuitem;
    Gtk::MenuItem *ruin_report_menuitem;
    Gtk::MenuItem *army_bonus_menuitem;
    Gtk::MenuItem *item_bonus_menuitem;
    Gtk::MenuItem *production_report_menuitem;
    Gtk::MenuItem *triumphs_menuitem;
    Gtk::MenuItem *help_about_menuitem;
    Gtk::MenuItem *tutorial_menuitem;
    Gtk::MenuItem *online_help_menuitem;
    Gtk::MenuItem *quit_menuitem;
    Gtk::MenuItem *toggle_grid_menuitem;
    Gtk::MenuItem *army_report_menuitem;
    Gtk::MenuItem *city_report_menuitem;
    Gtk::MenuItem *gold_report_menuitem;
    Gtk::MenuItem *winning_report_menuitem;
    Gtk::MenuItem *quests_menuitem;
    Gtk::MenuItem *preferences_menuitem;
    Gtk::MenuItem *zoom_in_menuitem;
    Gtk::MenuItem *zoom_out_menuitem;
    Gtk::MenuItem *best_fit_menuitem;
    Gtk::MenuItem *quick_help_menuitem;
    Gtk::Image *smallmap_image;
    Gtk::Image *bigmap_image;

    Gtk::Label *turn_label;
    Gtk::Box *turn_hbox;
    Gtk::Image *shield_image[MAX_PLAYERS];
    
    // the button control panel
    Gtk::Viewport *control_panel_viewport;

    CityInfoTip* city_info_tip;
    StackInfoTip* stack_info_tip;
    typedef std::vector<Gtk::ToggleButton *> army_buttons_type;
    army_buttons_type army_buttons;
    typedef std::vector<Gtk::RadioButton*> stack_buttons_type;
    stack_buttons_type stack_buttons;
    Gtk::EventBox *map_eventbox;
    Gtk::EventBox *bigmap_eventbox;
    Gtk::Box *status_box_container;
    StatusBox *status_box;

    Glib::ustring current_save_filename;

    Game* game;
    GameButtonBox *game_button_box;
    Gtk::Label *pos_label;

    GdkEventButton *button_event;
    
    bool on_delete_event();

    bool on_bigmap_mouse_button_event(GdkEventButton *e);
    bool on_bigmap_mouse_motion_event(GdkEventMotion *e);
    bool on_bigmap_key_event(GdkEventKey *e);

    bool on_smallmap_mouse_button_event(GdkEventButton *e);
    bool on_smallmap_mouse_motion_event(GdkEventMotion *e);
    bool on_mouse_entered_smallmap();
    
    void on_load_game_activated();
    void on_save_game_activated();
    void on_save_game_as_activated();
    void on_show_lobby_activated();
    void on_quit_activated();
    void on_new_game_activated();
    void on_game_stopped();
    void on_quests_activated();
    void on_disband_activated();
    void on_stack_info_activated();
    void on_resign_activated();
    void on_resignation_completed();
    void on_signpost_activated();
    void on_inspect_activated();
    void on_plant_standard_activated();
    void on_item_bonus_activated();
    void on_army_report_activated();
    void on_item_report_activated();
    void on_city_report_activated();
    void on_gold_report_activated();
    void on_production_report_activated();
    void on_winning_report_activated();
    void on_diplomacy_report_activated();
    void on_diplomacy_button_clicked();

    void on_fullscreen_activated();
    void on_preferences_activated();
    void on_zoom_in_activated();
    void on_zoom_out_activated();
    void on_best_fit_activated();
    void zoom (double scale);
    void on_group_ungroup_activated();
    
    void on_fight_order_activated();
    void on_levels_activated();
    void on_production_activated();
    void on_vectoring_activated();
    void on_grid_toggled();
    void on_ruin_report_activated();
    void on_army_bonus_activated();
    void on_city_history_activated();
    void on_ruin_history_activated();
    void on_event_history_activated();
    void on_gold_history_activated();
    void on_winner_history_activated();
    void on_triumphs_activated();
    void on_help_about_activated();
    void on_tutorial_activated();
    void on_online_help_activated();
    void on_quick_help_activated();

    void on_message_requested(Glib::ustring msg);
    
    // shield set on the top
    void show_shield_turn();

    // game callbacks
    void on_sidebar_stats_changed(SidebarStats s);
    void on_progress_status_changed(Glib::ustring status);
    void on_progress_changed();
    void on_smallmap_changed(Cairo::RefPtr<Cairo::Surface> map);
    void on_smallmap_slid();
    void on_bigmap_cursor_changed(ImageCache::CursorType cursor);
    void on_bigmap_changed(Cairo::RefPtr<Cairo::Surface> map);
    void on_stack_info_changed(Stack *s);
    void on_bigmap_tip_changed(Glib::ustring tip, MapTipPosition pos, bool timeout);
    void on_stack_tip_changed(StackTile *stile, MapTipPosition pos);
    void on_city_tip_changed(City *city, MapTipPosition pos);
    void on_ruin_searched(Ruin *ruin, Stack *s, Reward *reward);
    Reward* on_sage_visited(Ruin *ruin, Sage *sage, Stack *s);
    void on_ruin_rewarded(Reward_Ruin *reward);
    void on_fight_started(LocationBox box, Fight &fight);
    void on_abbreviated_fight_started(LocationBox box);
    void on_ruinfight_started(Stack *attackers, Keeper *keeper);
    void on_ruinfight_finished(Fight::Result result);
    bool on_hero_offers_service(Player *player, HeroProto *hero, City *city, int gold);
    bool on_enemy_offers_surrender(int numEnemies);
    void on_surrender_answered (bool accepted);
    bool on_stack_considers_treachery (Player *them);
    bool on_temple_searched(Hero *hero, Temple *temple, int blessCount);
    void on_quest_assigned(Hero *hero, Quest *quest);
    CityDefeatedAction on_city_defeated(City *city, int gold);
    void on_city_pillaged(City *city, int gold, int pillaged_army_type);
    void on_city_sacked(City *city, int gold, std::list<guint32> sacked_types);
    void on_city_razed(City *city);
    void on_city_visited(City *city);
    void on_ruin_visited(Ruin *ruin);
    void on_temple_visited(Temple *temple);
    void on_next_player_turn(Player *player, unsigned int turn_number);
    void on_remote_next_player_turn();
    void on_hero_brings_allies(int alliesCount);
    void on_medal_awarded_to_army(Army *army, int medaltype);
    Army::Stat on_hero_gains_level(Hero *hero);
    void on_game_loaded(Player *player);
    void on_game_over(Player *winner);
    void on_player_died(Player *player);
    void on_advice_asked(float percent);
    void on_gold_stolen(Player *victim, guint32 gold_pieces);
    void on_ships_sunk(guint32 num_armies);
    void on_bags_picked_up(Hero *hero, guint32 num_bags);
    void on_worms_killed(Hero *hero, Glib::ustring army_type_name, guint32 num_worms_killed);
    void on_city_diseased(Glib::ustring city_name, guint32 num_armies_killed);
    void on_city_defended(Glib::ustring city_name, Glib::ustring army_name, guint32 num_armies);
    void on_city_persuaded(Glib::ustring city_name, guint32 num_armies);
    void on_stack_teleported(Hero *hero, Glib::ustring city_name);
    void on_bridge_burned(Hero *hero);
    void on_keeper_captured(Hero *hero, Ruin*, Glib::ustring monster_name);
    void on_monster_summoned(Hero *hero, Glib::ustring monster_name);
    void on_mp_added_to_hero_stack(Hero *hero, guint32 mp);
    void on_stack_moves(Stack *stack, Vector<int> pos);
    void on_commentator_comments(Glib::ustring comment);
    Item* on_select_item(std::list<Item*> items);
    Player *on_select_item_victim_player();
    City *on_select_city_to_use_item_on(SelectCityMap::Type type);
    bool on_bigmap_scrolled(GdkEventScroll* event);
    void on_popup_stack_menu (Stack *stack);
    void on_pointing_at_new_tile(Vector<int> tile);

    // quest manager callbacks
    void on_quest_completed(Quest *quest, Reward *reward);
    void on_quest_expired(Quest *quest);
    
    // helpers
    void get_default_magnifying_glass_hotspot (int *x, int *y);
    void show_map_tip(Glib::ustring msg, MapTipPosition pos, bool timeout = false);
    void on_city_looted(City *city, int gold);
    bool hide_map_tip();
    void show_city_production_report (bool destitute);

    bool setup_game(GameScenario *game_scenario, NextTurn *nextTurn);
    void setup_signals(GameScenario *game_scenario);
    void stop_game(Glib::ustring action);
    std::list<sigc::connection> connections;
    
    void setup_menuitem(Gtk::MenuItem*, sigc::slot<void> , sigc::signal<void, bool> &);
    void on_bigmap_surface_changed(Gtk::Allocation box);
    void on_group_stack_toggled(bool lock);

    Player *game_winner;
    void give_some_cheese(Player *game_winner);

    bool on_window_state_event (GdkEventWindowState *e);
    bool on_configure_event (GdkEventConfigure *e);
    void set_default_bigmap_zoom ();
    bool on_bigmap_draw (const ::Cairo::RefPtr< ::Cairo::Context >& cr);
public:
    bool d_quick_fights; //do we speed up fights for this player's turn?
    Glib::ustring stop_action; //hackhackhack
    Glib::ustring d_scenario;
    int d_gold;
    std::list<Hero*> d_heroes;
    Glib::ustring d_player_name;
    Glib::ustring d_load_filename;
    Gtk::Allocation last_box;
    Gtk::Allocation unmaximized_box;
    sigc::connection map_tip_timer;
};

#endif
