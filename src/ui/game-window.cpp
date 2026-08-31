//  Copyright (C) 2007, 2008, Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2014, 2015, 2016, 2017,
//  2020, 2021 Ben Asselstine
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

#include <config.h>

#include <iomanip>
#include <queue>
#include <assert.h>
#include <string.h>

#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>
#include <sigc++/adaptors/hide.h>

#include <gtkmm.h>
#include <glib.h>
#include <gtk/gtk.h>

#include "game-window.h"

#include "input-helpers.h"
#include "driver.h"
#include "fight-window.h"
#include "city-window.h"
#include "army-gains-level-dialog.h"
#include "hero-dialog.h"
#include "SightMap.h"
#include "sage-dialog.h"
#include "ruin-rewarded-dialog.h"
#include "hero-offer-dialog.h"
#include "surrender-dialog.h"
#include "surrender-refused-dialog.h"
#include "quest-report-dialog.h"
#include "quest-assigned-dialog.h"
#include "quest-completed-dialog.h"
#include "preferences-dialog.h"
#include "fight-order-dialog.h"
#include "hero-levels-dialog.h"
#include "ruin-report-dialog.h"
#include "army-bonus-dialog.h"
#include "item-bonus-dialog.h"
#include "history-report-dialog.h"
#include "report-dialog.h"
#include "triumphs-dialog.h"
#include "diplomacy-report-dialog.h"
#include "diplomacy-dialog.h"
#include "stack-info-dialog.h"
#include "timed-message-dialog.h"
#include "destination-dialog.h"
#include "item-report-dialog.h"
#include "use-item-dialog.h"
#include "use-item-on-player-dialog.h"
#include "use-item-on-city-dialog.h"
#include "game-button-box.h"
#include "status-box.h"
#include "ucompose.hpp"
#include "defs.h"
#include "snd.h"
#include "File.h"
#include "game.h"
#include "gamebigmap.h"
#include "smallmap.h"
#include "GameScenarioOptions.h"
#include "army.h"
#include "ruin.h"
#include "ruinlist.h"
#include "path.h"
#include "player.h"
#include "signpostlist.h"
#include "playerlist.h"
#include "citylist.h"
#include "hero.h"
#include "heroproto.h"
#include "temple.h"
#include "templelist.h"
#include "city.h"
#include "cityset.h"
#include "Quest.h"
#include "stack.h"
#include "ImageCache.h"
#include "QuestsManager.h"
#include "QCitySack.h"
#include "QCityRaze.h"
#include "QCityOccupy.h"
#include "QPillageGold.h"
#include "counter.h"
#include "armysetlist.h"
#include "tilesetlist.h"
#include "CreateScenario.h"
#include "reward.h"
#include "Configuration.h"
#include "GameMap.h"
#include "Item.h"
#include "shieldsetlist.h"
#include "game-server.h"
#include "game-client.h"
#include "NextTurnHotseat.h"
#include "NextTurnNetworked.h"
#include "network_player.h"
#include "stacktile.h"
#include "MapBackpack.h"
#include "select-city-map.h"
#include "shield.h"
#include "lw-dialog.h"
#include "builder-cache.h"
#include "new-network-game-dialog.h"
#include "rnd.h"
#include "font-size.h"
#include "keeper.h"
#include "load-progress-window.h"

#define method(x) sigc::mem_fun(*this, &GameWindow::x)

double GameWindow::minimum_zoom_scale = 0.4;
double GameWindow::maximum_zoom_scale = 3.0;

GameWindow::GameWindow()
 : map_tip (NULL),
    stack_tip (NULL),
    city_info_tip (NULL),
    stack_info_tip (NULL),
    game (NULL),
    game_button_box (NULL),
     game_winner (NULL),
    last_box (Gtk::Allocation(0,0,1,1)),
    unmaximized_box (Gtk::Allocation(0,0,1,1))
{

  Glib::RefPtr<Gtk::Builder> xml = BuilderCache::get("game-window.ui");

  Gtk::Window *w = 0;
  xml->get_widget("window", w);
  window = w;
  w->set_icon_from_file(File::getVariousFile("castle_icon.png"));

  w->signal_window_state_event().connect (method(on_window_state_event));
  w->signal_delete_event().connect (sigc::hide(method(on_delete_event)));
  w->signal_configure_event().connect(method(on_configure_event));

  xml->get_widget("menubar", menubar);
  xml->get_widget("bigmap_image", bigmap_image);
  bigmap_image->signal_size_allocate().connect (method(on_bigmap_surface_changed));
  bigmap_image->signal_draw ().connect (method (on_bigmap_draw));
  bigmap_image->grab_focus();
  xml->get_widget("bigmap_eventbox", bigmap_eventbox);
  bigmap_eventbox->add_events(Gdk::KEY_PRESS_MASK | Gdk::BUTTON_PRESS_MASK | 
                              Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK |
                              Gdk::SMOOTH_SCROLL_MASK | Gdk::LEAVE_NOTIFY_MASK);
  bigmap_eventbox->signal_key_press_event().connect (method(on_bigmap_key_event));
  bigmap_eventbox->signal_key_release_event().connect (method(on_bigmap_key_event));
  bigmap_eventbox->signal_button_press_event().connect
    (method(on_bigmap_mouse_button_event));
  bigmap_eventbox->signal_button_release_event().connect
    (method(on_bigmap_mouse_button_event));
  bigmap_eventbox->signal_motion_notify_event().connect
    (method(on_bigmap_mouse_motion_event));
  bigmap_eventbox->signal_scroll_event().connect (method(on_bigmap_scrolled));
  bigmap_eventbox->signal_leave_notify_event().connect
    (sigc::hide(method(hide_map_tip)));

  xml->get_widget("status_box_container", status_box_container);

  status_box = StatusBox::create();
  status_box->reparent(*status_box_container);
  status_box->property_hexpand() = true;

  // the map image
  xml->get_widget("smallmap_image", smallmap_image);
  xml->get_widget("map_eventbox", map_eventbox);
  xml->get_widget("map_container", map_container);
  map_eventbox->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
                           Gdk::POINTER_MOTION_MASK | Gdk::SCROLL_MASK);
  map_eventbox->signal_button_press_event().connect
    (method(on_smallmap_mouse_button_event));
  map_eventbox->signal_button_release_event().connect
    (method(on_smallmap_mouse_button_event));
  map_eventbox->signal_motion_notify_event().connect
    (method(on_smallmap_mouse_motion_event));
  map_eventbox->signal_enter_notify_event().connect
    (sigc::hide(method(on_mouse_entered_smallmap)));
  xml->get_widget("control_panel_viewport", control_panel_viewport);
  game_button_box = GameButtonBox::create();
  game_button_box->reparent(*control_panel_viewport);
  game_button_box->property_halign() = Gtk::ALIGN_CENTER;

  // the stats
  xml->get_widget("turn_label", turn_label);
  xml->get_widget("turn_hbox", turn_hbox);
  xml->get_widget("shield_image_0", shield_image[0]);
  xml->get_widget("shield_image_1", shield_image[1]);
  xml->get_widget("shield_image_2", shield_image[2]);
  xml->get_widget("shield_image_3", shield_image[3]);
  xml->get_widget("shield_image_4", shield_image[4]);
  xml->get_widget("shield_image_5", shield_image[5]);
  xml->get_widget("shield_image_6", shield_image[6]);
  xml->get_widget("shield_image_7", shield_image[7]);

  // connect callbacks for the menu
  xml->get_widget("new_game_menuitem", new_game_menuitem);
  new_game_menuitem->signal_activate().connect (method(on_new_game_activated));
  xml->get_widget("load_game_menuitem", load_game_menuitem);
  load_game_menuitem->signal_activate().connect (method(on_load_game_activated));
  xml->get_widget("save_game_menuitem", save_game_menuitem);
  save_game_menuitem->signal_activate().connect (method(on_save_game_activated));
  xml->get_widget("save_game_as_menuitem", save_game_as_menuitem);
  save_game_as_menuitem->signal_activate().connect
    (method(on_save_game_as_activated));
  xml->get_widget("quit_menuitem", quit_menuitem);
  quit_menuitem->signal_activate().connect (method(on_quit_activated));
  xml->get_widget("toggle_grid_menuitem", toggle_grid_menuitem);
  toggle_grid_menuitem->signal_activate().connect (method(on_grid_toggled));
  xml->get_widget("army_report_menuitem", army_report_menuitem);
  army_report_menuitem->signal_activate().connect
    (method(on_army_report_activated));
  xml->get_widget("item_report_menuitem", item_report_menuitem);
  item_report_menuitem->signal_activate().connect
    (method(on_item_report_activated));
  xml->get_widget("city_report_menuitem", city_report_menuitem);
  city_report_menuitem->signal_activate().connect
    (method(on_city_report_activated));
  xml->get_widget("gold_report_menuitem", gold_report_menuitem);
  gold_report_menuitem->signal_activate().connect
    (method(on_gold_report_activated));
  xml->get_widget("winning_report_menuitem", winning_report_menuitem);
  winning_report_menuitem->signal_activate().connect
    (method(on_winning_report_activated));
  xml->get_widget("diplomacy_report_menuitem", diplomacy_report_menuitem);
  xml->get_widget("quests_menuitem", quests_menuitem);
  quests_menuitem->signal_activate().connect (method(on_quests_activated));
  xml->get_widget("fullscreen_menuitem", fullscreen_menuitem);
  fullscreen_menuitem->signal_activate().connect (method(on_fullscreen_activated));
  xml->get_widget("preferences_menuitem", preferences_menuitem);
  preferences_menuitem->signal_activate().connect
    (method(on_preferences_activated));
  xml->get_widget("zoom_in_menuitem", zoom_in_menuitem);
  zoom_in_menuitem->signal_activate().connect (method(on_zoom_in_activated));
  xml->get_widget("zoom_out_menuitem", zoom_out_menuitem);
  zoom_out_menuitem->signal_activate().connect (method(on_zoom_out_activated));
  xml->get_widget("best_fit_menuitem", best_fit_menuitem);
  best_fit_menuitem->signal_activate().connect (method(on_best_fit_activated));

  xml->get_widget("show_lobby_menuitem", show_lobby_menuitem);
  show_lobby_menuitem->signal_activate().connect (method(on_show_lobby_activated));
  xml->get_widget("end_turn_menuitem", end_turn_menuitem);
  xml->get_widget("move_all_menuitem", move_all_menuitem);
  xml->get_widget("disband_menuitem", disband_menuitem);
  xml->get_widget("stack_info_menuitem", stack_info_menuitem);
  xml->get_widget("signpost_menuitem", signpost_menuitem);
  xml->get_widget("search_menuitem", search_menuitem);
  xml->get_widget("use_menuitem", use_menuitem);
  xml->get_widget("inspect_menuitem", inspect_menuitem);
  xml->get_widget("plant_standard_menuitem", plant_standard_menuitem);
  xml->get_widget("city_history_menuitem", city_history_menuitem);
  xml->get_widget("ruin_history_menuitem", ruin_history_menuitem);
  xml->get_widget("event_history_menuitem", event_history_menuitem);
  xml->get_widget("gold_history_menuitem", gold_history_menuitem);
  xml->get_widget("winner_history_menuitem", winner_history_menuitem);
  xml->get_widget("group_ungroup_menuitem", group_ungroup_menuitem);
  xml->get_widget("leave_menuitem", leave_menuitem);
  xml->get_widget("next_menuitem", next_menuitem);

  xml->get_widget("fight_order_menuitem", fight_order_menuitem);
  fight_order_menuitem->signal_activate().connect
    (method(on_fight_order_activated));
  xml->get_widget("resign_menuitem", resign_menuitem);
  resign_menuitem->signal_activate().connect (method(on_resign_activated));
  xml->get_widget("production_menuitem", production_menuitem);
  production_menuitem->signal_activate().connect (method(on_production_activated));
  xml->get_widget("cities_menuitem", cities_menuitem);
  cities_menuitem->signal_activate().connect (method(on_production_activated));
  xml->get_widget("build_menuitem", build_menuitem);
  build_menuitem->signal_activate().connect (method(on_production_activated));
  xml->get_widget("vectoring_menuitem", vectoring_menuitem);
  vectoring_menuitem->signal_activate().connect (method(on_vectoring_activated));
  xml->get_widget("levels_menuitem", levels_menuitem);
  xml->get_widget("inspect_menuitem", inspect_menuitem);
  xml->get_widget("ruin_report_menuitem", ruin_report_menuitem);
  ruin_report_menuitem->signal_activate().connect
    (method(on_ruin_report_activated));
  xml->get_widget("army_bonus_menuitem", army_bonus_menuitem);
  army_bonus_menuitem->signal_activate().connect (method(on_army_bonus_activated));
  xml->get_widget("item_bonus_menuitem", item_bonus_menuitem);
  item_bonus_menuitem->signal_activate().connect (method(on_item_bonus_activated));
  xml->get_widget("production_report_menuitem", production_report_menuitem);
  production_report_menuitem->signal_activate().connect
    (method(on_production_report_activated));
  xml->get_widget("triumphs_menuitem", triumphs_menuitem);
  triumphs_menuitem->signal_activate().connect (method(on_triumphs_activated));
  xml->get_widget("help_about_menuitem", help_about_menuitem);
  help_about_menuitem->signal_activate().connect (method(on_help_about_activated));
  xml->get_widget ("tutorial_menuitem", tutorial_menuitem);
  tutorial_menuitem->signal_activate().connect (method(on_tutorial_activated));
  xml->get_widget("online_help_menuitem", online_help_menuitem);
  online_help_menuitem->signal_activate().connect
    (method(on_online_help_activated));
  xml->get_widget("quick_help_menuitem", quick_help_menuitem);
  quick_help_menuitem->signal_activate().connect (method(on_quick_help_activated));
  xml->get_widget("pos_label", pos_label);
  d_quick_fights = false;
}

GameWindow::~GameWindow()
{
  for (std::list<sigc::connection>::iterator it = connections.begin();
       it != connections.end(); ++it) 
    (*it).disconnect();
  connections.clear();
  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    shield_image[i]->clear();
  if (city_info_tip)
    {
      delete city_info_tip;
      city_info_tip = NULL;
    }
  if (stack_info_tip)
    {
      delete stack_info_tip;
      stack_info_tip = NULL;
    }
  if (game)
    {
      delete game;
      game = NULL;
    }
  if (game_button_box)
    {
      delete game_button_box;
      game_button_box = NULL;
    }
  if (status_box)
    {
      delete status_box;
      status_box = NULL;
    }
  delete window;
}

void GameWindow::show()
{
  if (game_button_box)
    {
      control_panel_viewport->show_all();
      game_button_box->show_all();
    }
    
    bigmap_image->show_all();
    window->show();
    if (status_box)
      {
        status_box->setHeightFudgeFactor(turn_label->get_height());
        status_box->enforce_height();
        status_box->show_stats();
      }
      
    on_bigmap_surface_changed(bigmap_image->get_allocation());
    if (getenv ("LORDSAWAR_GUI_TEST") != NULL)
      {
        window->set_position(Gtk::WIN_POS_NONE);
        window->move(0, 0);
      }
  Gdk::EventMask event_mask = window->get_window()->get_events ();
  event_mask |= Gdk::STRUCTURE_MASK;
  window->get_window()->set_events (event_mask);
}

void GameWindow::set_default_bigmap_zoom ()
{
  Glib::RefPtr<Gdk::Display> d = Gdk::Display::get_default ();
  zoom (BigMap::get_default_zoom_scale
        (d->get_default_screen ()->get_height ()));
}

void GameWindow::init(int width, int height)
{
    bigmap_image->set_size_request(width, height);

    Vector<int> d = SmallMap::calculate_smallmap_size();
    smallmap_image->set_size_request(d.x, d.y);
}

void GameWindow::new_network_game(GameScenario *game_scenario, NextTurn *next_turn)
{
  if (GameServer::getInstance()->isListening() == true)
    GameServer::getInstance()->round_begins.connect(method(on_remote_next_player_turn));
  else
    GameClient::getInstance()->playerlist_reorder_received.connect(method(on_remote_next_player_turn));
  bool success = false;
  //stop_game();
  success = setup_game(game_scenario, next_turn);
  if (!success)
    return;
  setup_signals(game_scenario);
  game->redraw();
  while (g_main_context_iteration(NULL, FALSE)); //doEvents fixes temporary 40x40 smallmap
  game->startGame();
  if (Playerlist::getActiveplayer()->getType() == Player::HUMAN)
    Playerlist::getInstance ()->setViewingplayer (Playerlist::getActiveplayer ());
  if (Playerlist::getActiveplayer() && GameServer::getInstance()->isListening() == false)
    if (Playerlist::getActiveplayer()->getType() != Player::NETWORKED)
      {
        dynamic_cast<NextTurnNetworked*>(next_turn)->start_player(Playerlist::getActiveplayer());
      }
}

void GameWindow::continue_network_game(NextTurn *next_turn)
{
  next_turn->start();
}

void GameWindow::new_game(GameScenario *game_scenario, NextTurn *next_turn)
{
  bool success = false;
  success = setup_game(game_scenario, next_turn);
  if (!success)
    return;
  setup_signals(game_scenario);
  game->startGame();
  //we don't get here until the game ends.
}

void GameWindow::load_game(GameScenario *game_scenario, NextTurn *next_turn)
{
  bool success = false;
  success = setup_game(game_scenario, next_turn);
  if (!success)
    return;

  game->get_bigmap().screen_size_changed(bigmap_image->get_allocation());
  setup_signals(game_scenario);
  game->loadGame();
  //we don't get here until the game ends, or a human player ends a turn.
  if (Playerlist::getInstance()->countPlayersAlive())
    game->redraw();
}

void GameWindow::setup_menuitem(Gtk::MenuItem *item,
				sigc::slot<void> slot,
				sigc::signal<void, bool> &game_signal)
{
  connections.push_back (item->signal_activate().connect(slot));
  connections.push_back 
    (game_signal.connect(sigc::mem_fun(item, &Gtk::Widget::set_sensitive)));
}

void GameWindow::setup_signals(GameScenario *game_scenario)
{
  // get rid of the connections that might be still around from last time
  for (std::list<sigc::connection>::iterator it = connections.begin();
       it != connections.end(); ++it) 
    (*it).disconnect();
  connections.clear();

  connections.push_back (game_button_box->diplomacy_clicked.connect
   (method (on_diplomacy_button_clicked)));
  connections.push_back 
    (game->city_too_poor_to_produce.connect (method(show_city_production_report)));
  connections.push_back
    (game->commentator_comments.connect (method(on_commentator_comments)));

  setup_menuitem(move_all_menuitem, sigc::mem_fun(game, &Game::move_all_stacks),
		 game->can_move_all_stacks);
  setup_menuitem(end_turn_menuitem, sigc::mem_fun(game, &Game::end_turn),
		 game->can_end_turn);
  if (game_scenario->getPlayMode() == GameScenario::NETWORKED)
    {
      load_game_menuitem->set_sensitive(false);
      if (GameServer::getInstance()->isListening() == false)
	{
	  save_game_menuitem->set_sensitive(false);
	  save_game_as_menuitem->set_sensitive(false);
	}
    }
  else
    show_lobby_menuitem->set_sensitive(false);

  setup_menuitem(disband_menuitem, method(on_disband_activated),
                 game->can_disband_stack);
  setup_menuitem(stack_info_menuitem, method(on_stack_info_activated),
		 game->can_deselect_selected_stack);
  setup_menuitem(signpost_menuitem, method(on_signpost_activated),
		 game->can_change_signpost);
  setup_menuitem(search_menuitem,
                 sigc::mem_fun(game, &Game::search_selected_stack),
		 game->can_search_selected_stack);
  setup_menuitem(use_menuitem,
		 sigc::mem_fun(game, &Game::select_item_to_use),
		 game->can_use_item);
  setup_menuitem(inspect_menuitem, method(on_inspect_activated),
		 game->can_inspect);
  setup_menuitem(levels_menuitem, method(on_levels_activated),
		 game->can_see_hero_levels);
  setup_menuitem(plant_standard_menuitem, method(on_plant_standard_activated),
		 game->can_plant_standard_selected_stack);
  setup_menuitem(city_history_menuitem, method(on_city_history_activated),
		 game->can_see_history);
  setup_menuitem(ruin_history_menuitem, method(on_ruin_history_activated),
		 game->can_see_history);
  setup_menuitem(gold_history_menuitem, method(on_gold_history_activated),
		 game->can_see_history);
  setup_menuitem(event_history_menuitem, method(on_event_history_activated),
		 game->can_see_history);
  setup_menuitem(winner_history_menuitem, method(on_winner_history_activated),
		 game->can_see_history);
  setup_menuitem(diplomacy_report_menuitem, method(on_diplomacy_report_activated),
		 game->can_see_diplomacy); 
  setup_menuitem(group_ungroup_menuitem, method(on_group_ungroup_activated),
		 game->can_group_ungroup_selected_stack);
  setup_menuitem(leave_menuitem,
		 sigc::mem_fun(game, &Game::park_selected_stack),
		 game->can_park_selected_stack);
  setup_menuitem(next_menuitem,
		 sigc::mem_fun(game, &Game::select_next_movable_stack),
		 game->can_select_next_movable_stack);

  // setup game callbacks
  connections.push_back (game->game_stopped.connect (method(on_game_stopped)));
  connections.push_back (game->sidebar_stats_changed.connect
                         (method(on_sidebar_stats_changed)));
  connections.push_back (game->progress_status_changed.connect
                         (method(on_progress_status_changed)));
  connections.push_back (game->progress_changed.connect
                         (method(on_progress_changed)));
  connections.push_back (game->bigmap_changed.connect (method(on_bigmap_changed)));
  connections.push_back (game->smallmap_changed.connect
                         (sigc::hide(method(on_smallmap_changed))));
  connections.push_back (game->get_smallmap().view_slid.connect
                         (sigc::hide(method(on_smallmap_slid))));
  connections.push_back
    (game->stack_info_changed.connect
     (sigc::mem_fun(*status_box, &StatusBox::on_stack_info_changed)));
  connections.push_back (game->map_tip_changed.connect
                         (method(on_bigmap_tip_changed)));
  connections.push_back (game->stack_tip_changed.connect
                         (method(on_stack_tip_changed)));
  connections.push_back (game->city_tip_changed.connect
                         (method(on_city_tip_changed)));
  connections.push_back (game->ruin_searched.connect (method(on_ruin_searched)));
  connections.push_back (game->sage_visited.connect (method(on_sage_visited)));
  connections.push_back (game->fight_started.connect (method(on_fight_started)));
  connections.push_back (game->abbreviated_fight_started.connect
                         (method(on_abbreviated_fight_started)));
  connections.push_back (game->ruinfight_started.connect
                         (method(on_ruinfight_started)));
  connections.push_back (game->ruinfight_finished.connect
                         (method(on_ruinfight_finished)));
  connections.push_back (game->hero_offers_service.connect
                         (method(on_hero_offers_service)));
  connections.push_back (game->enemy_offers_surrender.connect
                         (method(on_enemy_offers_surrender)));
  connections.push_back (game->surrender_answered.connect
                         (method(on_surrender_answered)));
  connections.push_back
    (game->stack_considers_treachery.connect
     (sigc::hide(sigc::hide<0>(method(on_stack_considers_treachery)))));
  connections.push_back (game->temple_searched.connect
                         (method(on_temple_searched)));
  connections.push_back (game->quest_assigned.connect (method(on_quest_assigned)));
  connections.push_back (game->city_defeated.connect (method(on_city_defeated)));
  connections.push_back (game->city_pillaged.connect (method(on_city_pillaged)));
  connections.push_back (game->city_sacked.connect (method(on_city_sacked)));
  connections.push_back (game->city_razed.connect (method(on_city_razed)));
  connections.push_back (game->city_visited.connect (method(on_city_visited)));
  connections.push_back (game->ruin_visited.connect (method(on_ruin_visited)));
  connections.push_back (game->temple_visited.connect (method(on_temple_visited)));
  connections.push_back (game->next_player_turn.connect
                         (method(on_next_player_turn)));
  connections.push_back (game->hero_arrives.connect
                         (method(on_hero_brings_allies)));
  connections.push_back (game->medal_awarded_to_army.connect
                         (method(on_medal_awarded_to_army)));
  connections.push_back (game->hero_gains_level.connect
                         (method(on_hero_gains_level)));
  connections.push_back (game->game_loaded.connect (method(on_game_loaded)));
  connections.push_back (game->game_over.connect (method(on_game_over)));
  connections.push_back (game->player_died.connect (method(on_player_died)));
  connections.push_back (game->advice_asked.connect (method(on_advice_asked)));
  connections.push_back (game->sunk_ships.connect
                         (sigc::hide<0>(method(on_ships_sunk))));
  connections.push_back (game->bags_picked_up.connect (method(on_bags_picked_up)));
  connections.push_back (game->mp_added_to_hero_stack.connect
                         (method(on_mp_added_to_hero_stack)));
  connections.push_back (game->worms_killed.connect (method(on_worms_killed)));
  connections.push_back (game->bridge_burned.connect (method(on_bridge_burned)));
  connections.push_back (game->keeper_captured.connect
                         (method(on_keeper_captured)));
  connections.push_back (game->monster_summoned.connect
                         (method(on_monster_summoned)));
  connections.push_back (game->stole_gold.connect (method(on_gold_stolen)));
  connections.push_back (game->stack_moves.connect (method(on_stack_moves)));
  connections.push_back (game->select_item.connect (method(on_select_item)));
  connections.push_back (game->select_item_victim_player.connect
                         (method(on_select_item_victim_player)));
  connections.push_back (game->select_city_to_use_item_on.connect
                         (method(on_select_city_to_use_item_on)));
  connections.push_back (game->city_diseased.connect
                         (sigc::hide<0>(method(on_city_diseased))));
  connections.push_back (game->city_defended.connect
                         (sigc::hide<0>(method(on_city_defended))));
  connections.push_back (game->city_persuaded.connect
                         (sigc::hide<0>(method(on_city_persuaded))));
  connections.push_back (game->stack_teleported.connect
                         (method(on_stack_teleported)));
  connections.push_back (game->popup_stack_actions_menu.connect
                         (method(on_popup_stack_menu)));

  // misc callbacks
  QuestsManager *q = QuestsManager::getInstance();
  connections.push_back (q->quest_completed.connect (method(on_quest_completed)));
  connections.push_back (q->quest_expired.connect (method(on_quest_expired)));
  if (game)
    connections.push_back (game->get_bigmap().cursor_changed.connect
                           (method(on_bigmap_cursor_changed)));

  connections.push_back (game->remote_next_player_turn.connect
                         (method(on_remote_next_player_turn)));
  connections.push_back (game->pointing_at_new_tile.connect (method(on_pointing_at_new_tile)));
}

void GameWindow::show_city_production_report (bool destitute)
{
  if (!destitute)
    return;
  ReportDialog d(*window, Playerlist::getActiveplayer(), ReportDialog::PRODUCTION);
  d.run();
  d.hide();
}

bool GameWindow::setup_game(GameScenario *game_scenario, NextTurn *nextTurn)
{
  status_box->clear_selected_stack();

  Snd::getInstance()->halt(true);
  Snd::getInstance()->enableBackground();

  if (game)
    delete game;
  game = new Game(game_scenario, nextTurn);

  set_default_bigmap_zoom ();

  game_button_box->setup_signals(game);
    
  status_box->stack_composition_modified.connect
      (sigc::mem_fun(game, &Game::recalculate_moves_for_stack));
  status_box->stack_tile_group_toggle.connect (method(on_group_stack_toggled));
  show_shield_turn();
      smallmap_image->set_size_request(game->get_smallmap().get_width(),
                                       game->get_smallmap().get_height());
  while (g_main_context_iteration(NULL, FALSE)); //doEvents

  return true;
}
    
void GameWindow::on_group_stack_toggled(bool lock)
{
  game->get_bigmap().set_input_locked(lock);
}

bool GameWindow::on_delete_event()
{
  on_quit_activated();

  return true;
}

bool GameWindow::on_bigmap_mouse_button_event(GdkEventButton *e)
{
  if (e->type != GDK_BUTTON_PRESS && e->type != GDK_BUTTON_RELEASE)
    return true;	// useless event

  if (game)
    {
      button_event = e;
      game->get_bigmap().mouse_button_event(to_input_event(e));
    }

  return true;
}

bool GameWindow::on_bigmap_mouse_motion_event(GdkEventMotion *e)
{
  static guint prev = 0;
  if (game)
    {
      gint delta = e->time - prev;
      if (delta > 40 || delta < 0)
	{
	  game->get_bigmap().mouse_motion_event(to_input_event(e));
	  bigmap_image->grab_focus();
	  prev = e->time;
	}
    }
  return true;
}
    
void GameWindow::on_bigmap_cursor_changed(ImageCache::CursorType cursor)
{
  if (cursor == ImageCache::POINTER)
      bigmap_image->get_window()->set_cursor ();
  else
    {
      bigmap_image->get_window()->set_cursor
        (Gdk::Cursor::create
         (Gdk::Display::get_default(),
          ImageCache::getInstance()->getCursorPic
          (cursor,
           FontSize::getInstance ()->get_height ())->to_pixbuf(), 4, 4));
    }
}

bool GameWindow::on_bigmap_key_event(GdkEventKey *e)
{
  if (e->keyval == GDK_KEY_Shift_L || e->keyval == GDK_KEY_Shift_R)
    game->get_bigmap().set_shift_key_down (e->type == GDK_KEY_PRESS);
  if (e->keyval == GDK_KEY_Control_L || e->keyval == GDK_KEY_Control_R)
    game->get_bigmap().set_control_key_down (e->type == GDK_KEY_PRESS);

  return true;
}

bool GameWindow::on_smallmap_mouse_button_event(GdkEventButton *e)
{
  if (e->type != GDK_BUTTON_PRESS && e->type != GDK_BUTTON_RELEASE)
    return true;	// useless event

  if (game)
    game->get_smallmap().mouse_button_event(to_input_event(e));

  return true;
}

bool GameWindow::on_smallmap_mouse_motion_event(GdkEventMotion *e)
{
  static guint prev = 0;
  if (game)
    {
      gint delta = e->time - prev;
      if (delta > 100 || delta < 0)
	{
	  game->get_smallmap().mouse_motion_event(to_input_event(e));
	  prev = e->time;
	}

    }

  return true;
}

void GameWindow::get_default_magnifying_glass_hotspot (int *hotspot_x, int *hotspot_y)
{
  PixMask *p =
    ImageCache::getInstance()->getCursorPic
    (ImageCache::MAGNIFYING_GLASS,
     FontSize::getInstance ()->get_height ());

  double x = p->get_width () * (8.0/11.0);
  double y = p->get_height () * (5.0/11.0);

  *hotspot_x = int(x);
  *hotspot_y = int(y);
}

bool GameWindow::on_mouse_entered_smallmap()
{
  static int hotspot_x = -1, hotspot_y = -1;
  if (hotspot_x == -1 && hotspot_y == -1)
    get_default_magnifying_glass_hotspot (&hotspot_x, &hotspot_y);
  map_eventbox->get_window()->set_cursor
    (Gdk::Cursor::create (Gdk::Display::get_default(),
                          ImageCache::getInstance()->getCursorPic
                          (ImageCache::MAGNIFYING_GLASS,
                           FontSize::getInstance ()->get_height ())->to_pixbuf(),
                          hotspot_x, hotspot_y));
  return true;
}

void GameWindow::on_bigmap_surface_changed(Gtk::Allocation box)
{
  if (game) 
    {
      if (box.get_width() != last_box.get_width() || 
          box.get_height() != last_box.get_height())
        {
          game->get_bigmap().screen_size_changed(bigmap_image->get_allocation());
          game->redraw();
        }
      last_box = box;
    }
}

bool GameWindow::on_bigmap_draw (const ::Cairo::RefPtr< ::Cairo::Context >& cr)
{
  if (game)
    {
      Cairo::RefPtr<Cairo::Surface> surf = game->get_bigmap().get_surface ();
      cr->set_source(surf, 0, 0);
      cr->rectangle(0, 0, bigmap_image->get_width(), bigmap_image->get_height());
      cr->fill();
    }
  return true;
}

void GameWindow::on_load_game_activated()
{
  Gtk::FileChooserDialog chooser(*window, _("Choose Game to Load"));
  Glib::RefPtr<Gtk::FileFilter> sav_filter = Gtk::FileFilter::create();
  sav_filter->set_name(_("LordsAWar Saved Games (*.sav)"));
  sav_filter->add_pattern("*" + SAVE_EXT);
  chooser.add_filter(sav_filter);
  chooser.set_current_folder(Configuration::s_savePath);

  chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  chooser.set_default_response(Gtk::RESPONSE_ACCEPT);

  chooser.show_all();
  int res = chooser.run();
  chooser.hide();
  while (g_main_context_iteration(NULL, FALSE)); //doEvents

  if (res == Gtk::RESPONSE_ACCEPT)
    {
      Glib::ustring filename = chooser.get_filename();
      current_save_filename = filename;
      if (filename == File::getSaveFile("autosave" + SAVE_EXT))
	game->inhibitAutosaveRemoval(true);
      d_load_filename = filename;
      stop_game("load-game");
      //now look at on_game_stopped.
    }
}

void GameWindow::on_save_game_activated()
{
  if (current_save_filename.empty())
    on_save_game_as_activated();
  else
    {
      if (game)
	{
	  bool success = game->saveGame(current_save_filename);
	  if (!success)
            {
              TimedMessageDialog dialog(*window, _("Game was not saved!"), 0);
              dialog.run_and_hide();
            }
	}
    }
}

void GameWindow::on_save_game_as_activated()
{
  Gtk::FileChooserDialog chooser(*window, _("Choose a Name"),
				 Gtk::FILE_CHOOSER_ACTION_SAVE);
  Glib::RefPtr<Gtk::FileFilter> sav_filter = Gtk::FileFilter::create();
  sav_filter->set_name(_("LordsAWar Saved Games (*.sav)"));
  sav_filter->add_pattern("*" + SAVE_EXT);
  chooser.add_filter(sav_filter);
  chooser.set_current_folder(Configuration::s_savePath);

  chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  chooser.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_ACCEPT);
  chooser.set_default_response(Gtk::RESPONSE_ACCEPT);
  chooser.set_do_overwrite_confirmation ();

  chooser.show_all();
  int res = chooser.run();
  chooser.hide();

  if (res == Gtk::RESPONSE_ACCEPT)
    {
      Glib::ustring filename = chooser.get_filename();

      current_save_filename = filename;

      if (game)
	{
	  bool success = game->saveGame(current_save_filename);
	  if (!success)
            {
              TimedMessageDialog dialog(*window, _("Error saving game!"), 0);
              dialog.run_and_hide();
            }
	}
    }
}

void GameWindow::on_new_game_activated()
{
  LwDialog dialog(*window, "game-quit-dialog.ui");
  int response = dialog.run_and_hide();
  if (response == Gtk::RESPONSE_ACCEPT) //end the game
    stop_game("new");
}

void GameWindow::on_quit_activated()
{
  if (fullscreen_menuitem->get_active())
    {
      window->unfullscreen ();
      window->set_default_size (unmaximized_box.get_width (),
                                unmaximized_box.get_height ());
    }
  if (window->is_maximized ())
    window->unmaximize ();
  LwDialog dialog(*window, "game-quit-dialog.ui");
  int response = dialog.run_and_hide();
  if (response == Gtk::RESPONSE_ACCEPT) //end the game
    stop_game("quit");
}

void GameWindow::on_game_stopped()
{
  if (stop_action == "quit")
    {
      if (game)
	{
	  delete game;
	  game = NULL;
	}
      game_ended.emit();
    }
  else if (stop_action == "new")
    {
      if (game)
	{
	  delete game;
	  game = NULL;
	}
      game_ended_start_new.emit();
    }
  else if (stop_action == "game-over")
    {
      if (game_winner)
        {
          if (game_winner->getType() != Player::HUMAN)
            {
              if (game)
                {
                  delete game;
                  game = NULL;
                }
              game_ended.emit();
            }
          else
            {
              //we need to keep the game object around
              //so that we can give out some cheese
              give_some_cheese(game_winner);
            }
        }
      else
        {
          if (game)
            {
              delete game;
              game = NULL;
            }
          game_ended.emit();
        }
    }
  else if (stop_action == "load-game")
    {
      if (game)
	{
	  delete game;
	  game = NULL;
	}
      bool broken = false;
    
      LoadProgressWindow *p = new LoadProgressWindow (window);
      GameScenario::load_tick.connect
        (sigc::mem_fun (p, &LoadProgressWindow::tick_progress));
      GameScenario::load_finish.connect
        (sigc::mem_fun (p, &LoadProgressWindow::finish_progress));
      p->run ();
      GameScenario* game_scenario = new GameScenario(d_load_filename, broken);
      p->hide ();

      if (broken)
	{
	  on_message_requested(_("Corrupted saved game file."));
	  game_ended.emit();
	  return;
	}
      if (game_scenario->getPlayMode() == GameScenario::HOTSEAT)
	load_game(game_scenario, new NextTurnHotseat());
      else if (game_scenario->getPlayMode() == GameScenario::NETWORKED)
        {
          NewNetworkGameDialog nngd(*get_window(), true);
          bool retval = nngd.run();
          nngd.hide();
          hide();
          if (retval)
            {
             load_hosted_network_game.emit 
                (d_load_filename, LORDSAWAR_PORT, nngd.getProfile(), 
                 nngd.isAdvertised(), nngd.isRemotelyHosted());
            }
        }
    }
}

void GameWindow::on_quests_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  Player *player = Playerlist::getActiveplayer();
  std::vector<Quest*> quests
    = QuestsManager::getInstance()->getPlayerQuests(player);
  Stack *s = player->getActivestack();
  Hero *hero = NULL;
  if (s)
    hero = s->getFirstHeroWithAQuest();
  QuestReportDialog d(*window, quests, hero);
  d.run();
  d.hide();
  return;
}

void GameWindow::on_fullscreen_activated()
{
  if (fullscreen_menuitem->get_active())
    window->fullscreen();
  else
    {
      window->unfullscreen();
      window->set_default_size (unmaximized_box.get_width (),
                                unmaximized_box.get_height ());
    }
}

void GameWindow::on_signpost_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  Stack *stack = Playerlist::getActiveplayer()->getActivestack();
  if (!stack)
    return;
  Signpost *s = GameMap::getSignpost(stack->getPos());
  if (!s)
    return;
  LwDialog dialog(*window, "signpost-change-dialog.ui");
  dialog.set_title(_("Signpost"));
  Glib::RefPtr<Gtk::Builder> xml = dialog.get_builder();
  Gtk::Label *l;
  xml->get_widget("label", l);
  l->set_text(_("Change the message on this sign:"));
  Gtk::Entry *e;
  xml->get_widget("message_entry", e);
  e->set_text(s->getName());
  e->set_activates_default(true);
  int response = dialog.run_and_hide();

  if (response == Gtk::RESPONSE_ACCEPT)
    Playerlist::getActiveplayer()->signpostChange(s, 
                                                  String::utrim(e->get_text()));
  return;
}

  
void GameWindow::on_stack_info_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  StackInfoDialog d(*window, status_box->get_currently_selected_stack()->getPos());
  d.run_and_hide();
  Stack *s = d.get_selected_stack();
  status_box->on_stack_info_changed(s);
}

void GameWindow::on_disband_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  Stack *stack = Playerlist::getActiveplayer()->getActivestack();
  LwDialog dialog(*window, "disband-stack-dialog.ui");
  dialog.set_title(_("Disband"));
  Glib::RefPtr<Gtk::Builder> xml = dialog.get_builder();
  Gtk::Label *l;
  xml->get_widget("label", l);

  std::vector<guint32> heroes;
  stack->getHeroes(heroes);
  Glib::ustring s = _("Are you sure you want to disband this group?");
  if (heroes.size() > 0)
    {
      s += "\n";
      s += String::ucompose( ngettext("(It contains %1 hero).",
				      "(It contains %1 heroes).", 
				      heroes.size()), heroes.size());
    }
  l->set_text(s);
  int response = dialog.run_and_hide();

  if (response == Gtk::RESPONSE_ACCEPT) //disband the active stack
    Playerlist::getActiveplayer()->stackDisband(NULL);

  return;
}

void GameWindow::on_resignation_completed()
{
  LwDialog dialog(*window, "player-resign-completed-dialog.ui");
  dialog.run_and_hide();
  return;
}

void GameWindow::on_resign_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;

  LwDialog dialog(*window, "player-resign-dialog.ui");
  dialog.set_title(_("Resign"));
  Glib::RefPtr<Gtk::Builder> xml = dialog.get_builder();
  Gtk::Label *l;
  xml->get_widget("label", l);
  l->set_text(_("Are you sure you want to resign?"));
  int response = dialog.run_and_hide();
  if (response == Gtk::RESPONSE_ACCEPT) //disband all stacks, raze all cities
    {
      Playerlist::getActiveplayer()->resign();
      on_resignation_completed();
    }
  return;
}

void GameWindow::on_vectoring_activated()
{
  City *city;
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;

  if (status_box->get_currently_selected_stack())
    {
      Vector<int> pos = status_box->get_currently_selected_stack()->getPos();
      city = Citylist::getInstance()->getNearestVisibleFriendlyCity(pos);
    }
  else
    city = Playerlist::getActiveplayer()->getFirstCity();

  if (!city)
    return;
  bool see_all = true;
  DestinationDialog d(*window, city, &see_all);
  d.run();
  d.hide();
  return;
}

void GameWindow::on_production_activated()
{
  City *city;
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;

  if (status_box->get_currently_selected_stack())
    {
      Vector<int> pos = status_box->get_currently_selected_stack()->getPos();
      city = Citylist::getInstance()->getNearestVisibleFriendlyCity(pos);
    }
  else
    city = Playerlist::getActiveplayer()->getFirstCity();

  if (!city)
    return;

  on_city_visited(city);
  return;
}

void GameWindow::zoom (double scale)
{
  if (scale < minimum_zoom_scale)
    scale = minimum_zoom_scale;
  zoom_in_menuitem->set_sensitive (scale > minimum_zoom_scale);
  if (scale > maximum_zoom_scale)
    scale = maximum_zoom_scale;
  zoom_out_menuitem->set_sensitive (scale < maximum_zoom_scale);
  GameMap::getInstance()->getTileset()->set_scale (scale);
  GameMap::getInstance()->getCityset()->set_scale (scale);
  for (auto& i : *Playerlist::getInstance())
    Armysetlist::getInstance()->get((*i).getArmyset())->set_scale (scale);
  game->get_bigmap().screen_size_changed(bigmap_image->get_allocation());
  game->redraw();
}

void GameWindow::on_zoom_in_activated()
{
  zoom (GameMap::getInstance()->getTileset()->get_scale () + ZOOM_STEP);
}

void GameWindow::on_zoom_out_activated()
{
  zoom (GameMap::getInstance()->getTileset()->get_scale () - ZOOM_STEP);
}

void GameWindow::on_preferences_activated()
{
  Player *current = Playerlist::getInstance()->getActiveplayer();
  bool readonly = false;
  if (game->getScenario()->getPlayMode() == GameScenario::NETWORKED)
    readonly = true;
  PreferencesDialog d(*window, readonly);
  d.run(game);
  d.hide();
  game->get_bigmap().set_control_key_down (false);
  if (current != Playerlist::getInstance()->getActiveplayer())
    game->end_turn();
}

void GameWindow::on_group_ungroup_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  status_box->toggle_group_ungroup();
}

void GameWindow::on_fight_order_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  FightOrderDialog d(*window, Playerlist::getActiveplayer());
  d.run();
  d.hide();
}

void GameWindow::on_levels_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  HeroLevelsDialog d(*window, Playerlist::getActiveplayer());
  d.run_and_hide();
}

void GameWindow::on_ruin_report_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  Vector<int> pos;
  pos.x = 0;
  pos.y = 0;
  if (status_box->get_currently_selected_stack())
    pos = status_box->get_currently_selected_stack()->getPos();

  if (Templelist::getInstance()->size() == 0 &&
      Ruinlist::getInstance()->size() == 0)
    {
      TimedMessageDialog dialog(*window, _("No ruins or temples to show!"), 30);
      dialog.run_and_hide();
      return;
    }
  RuinReportDialog d(*window, pos);
  d.run();
  d.hide();
}

void GameWindow::on_army_bonus_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  ArmyBonusDialog d(*window, Playerlist::getActiveplayer());
  d.run_and_hide();
}

void GameWindow::on_item_bonus_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  ItemBonusDialog d(*window);
  d.run_and_hide();
}

void GameWindow::on_army_report_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  ReportDialog d(*window, Playerlist::getActiveplayer(), ReportDialog::ARMY);
  d.run();
  d.hide();
}

void GameWindow::on_item_report_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  std::list<Stack*> stacks = Playerlist::getActiveplayer()->getStacksWithItems();
  std::list<MapBackpack*> bags = GameMap::getInstance()->getBackpacks();
  ItemReportDialog d(*window, stacks, bags);
  d.run();
  d.hide();
}

void GameWindow::on_city_report_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  ReportDialog d(*window, Playerlist::getActiveplayer(), ReportDialog::CITY);
  d.run();
  d.hide();
}

void GameWindow::on_gold_report_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  ReportDialog d(*window, Playerlist::getActiveplayer(), ReportDialog::GOLD);
  d.run();
  d.hide();
}

void GameWindow::on_production_report_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  ReportDialog d(*window, Playerlist::getActiveplayer(), ReportDialog::PRODUCTION);
  d.run();
  d.hide();
}

void GameWindow::on_winning_report_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN &&
      !Playerlist::getInstance()->getWinningPlayer())
    return;
  ReportDialog d(*window, Playerlist::getActiveplayer(), ReportDialog::WINNING);
  d.run();
  d.hide();
}

void GameWindow::on_city_history_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN &&
      !Playerlist::getInstance()->getWinningPlayer())
    return;
  HistoryReportDialog d(*window, Playerlist::getActiveplayer(), 
			HistoryReportDialog::CITY);
  d.run();
  d.hide();
}

void GameWindow::on_ruin_history_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN &&
      !Playerlist::getInstance()->getWinningPlayer())
    return;
  HistoryReportDialog d(*window, Playerlist::getActiveplayer(), 
			HistoryReportDialog::RUIN);
  d.run();
  d.hide();
}

void GameWindow::on_event_history_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN &&
      !Playerlist::getInstance()->getWinningPlayer())
    return;
  HistoryReportDialog d(*window, Playerlist::getActiveplayer(),
			HistoryReportDialog::EVENTS);
  d.run();
  d.hide();
}

void GameWindow::on_gold_history_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN &&
      !Playerlist::getInstance()->getWinningPlayer())
    return;
  HistoryReportDialog d(*window ,Playerlist::getActiveplayer(),
			HistoryReportDialog::GOLD);
  d.run();
  d.hide();
}

void GameWindow::on_winner_history_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN &&
      !Playerlist::getInstance()->getWinningPlayer())
    return;
  HistoryReportDialog d(*window, Playerlist::getActiveplayer(),
			HistoryReportDialog::WINNING);
  d.run();
  d.hide();
}

void GameWindow::on_triumphs_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  TriumphsDialog d(*window, Playerlist::getActiveplayer());
  d.run_and_hide();
}

void GameWindow::on_help_about_activated()
{
  Gtk::AboutDialog* dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file (File::getGladeFile("about-dialog.ui"));

  xml->get_widget("dialog", dialog);
  dialog->set_icon_from_file(File::getVariousFile("castle_icon.png"));
  dialog->set_version(PACKAGE_VERSION);
  dialog->set_logo(ImageCache::loadMiscImage("castle_icon.png")->to_pixbuf());
  dialog->set_transient_for(*window);
  dialog->show_all();
  dialog->run();
  dialog->hide();
  delete dialog;

  return;
}

void GameWindow::on_diplomacy_report_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  if (GameScenario::s_diplomacy == false)
    return;
  DiplomacyReportDialog d(*window, Playerlist::getActiveplayer());
  d.run_and_hide();
}

void GameWindow::on_diplomacy_button_clicked()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  DiplomacyDialog d(*window, Playerlist::getActiveplayer());
  d.run_and_hide();
}

void GameWindow::stop_game(Glib::ustring action)
{
  stop_action = action;
  Snd::getInstance()->disableBackground();
  if (game)
    {
      current_save_filename = "";
      if (action == "game-over" && game->getScenario()->getPlayMode() == GameScenario::NETWORKED)
        give_some_cheese(game_winner);
      else
        game->stopGame();
    }
}

void GameWindow::on_game_over(Player *winner)
{
  LwDialog dialog(*window, "game-over-dialog.ui");
  Glib::RefPtr<Gtk::Builder> xml = dialog.get_builder();
  Gtk::Image *image;
  xml->get_widget("image", image);

  image->property_pixbuf() =
    ImageCache::getInstance()->getDialogPic
    (ImageCache::DIALOG_WINNING,
     FontSize::getInstance ()->get_height ())->to_pixbuf();

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s;
  s += String::ucompose(_("Congratulations to %1 for conquering the world!"), 
	winner->getName());
  label->set_markup("<b>" + s + "</b>");

  dialog.run_and_hide();

  game_winner = winner;
  stop_game("game-over");
}

void GameWindow::on_player_died(Player *player)
{
  assert(player);

  Glib::ustring s;
  s += String::ucompose(_("The rule of %1 has permanently ended!"),
			player->getName());
  if (Playerlist::getInstance()->countHumanPlayersAlive() == 0 &&
      player->getType() == Player::HUMAN)
    {
      s += "\n";
      s += _("No further human resistance is possible\nbut the battle will continue!");
      s += "\n";
      s += _("Press `CTRL-P' to stop the war\nand visit the sites of thy old battles.");
    }

  TimedMessageDialog dialog(*window, s, FightWindow::s_quick_all ? 3: 30);
  dialog.run_and_hide();
}

void GameWindow::on_message_requested(Glib::ustring msg)
{
  TimedMessageDialog dialog(*window, msg, 0);
  dialog.run_and_hide ();
}

void GameWindow::on_progress_status_changed(Glib::ustring string)
{
  status_box->set_progress_label(string);
}

void GameWindow::on_progress_changed()
{
  status_box->pulse();
}

void GameWindow::on_sidebar_stats_changed(SidebarStats s)
{
  status_box->update_sidebar_stats(s);

  turn_label->set_markup(String::ucompose("<b>%1 %2</b>", 
                                          _("Turn"), s.turns));
}

void GameWindow::on_bigmap_changed(Cairo::RefPtr<Cairo::Surface> map)
{
  Gtk::Allocation old = game->get_bigmap().get_allocation();
  int width = bigmap_image->get_allocated_width();
  int height = bigmap_image->get_allocated_height();
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
    Gdk::Pixbuf::create(map, 0, 0, std::min(width, old.get_width()), std::min(height, old.get_height()));
  //bigmap_image->property_pixbuf() = pixbuf;
  bigmap_image->queue_draw();
  //while (g_main_context_iteration(NULL, FALSE)); //doEvents
  //enabling this makes dragging the smallmap freeze
}

void GameWindow::on_smallmap_changed(Cairo::RefPtr<Cairo::Surface> map)
{
  Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(map);
  double x1, x2, y1, y2;
  cr->get_clip_extents (x1, y1, x2, y2);
  int width = x2 - x1;
  int height = y2 - y1;

  if (smallmap_image->get_allocated_width() != width ||
      smallmap_image->get_allocated_height() != height)
    smallmap_image->set_size_request(width, height);
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
    Gdk::Pixbuf::create(map, 0, 0, 
                        smallmap_image->get_allocated_width(), 
                        smallmap_image->get_allocated_height());
  smallmap_image->property_pixbuf() = pixbuf;
}

void GameWindow::on_smallmap_slid ()
{
  on_smallmap_changed(game->get_smallmap().get_surface());
  while (g_main_context_iteration(NULL, FALSE)); //doEvents
}

void GameWindow::on_city_tip_changed(City *city, MapTipPosition mpos)
{
  if (city == NULL)
    {
      delete city_info_tip;
      city_info_tip = NULL;
    }
  else
    {
      city_info_tip = new CityInfoTip(bigmap_image, mpos, city);
    }
}

void GameWindow::on_stack_tip_changed(StackTile *stile, MapTipPosition mpos)
{
  if (stile == NULL)
    {
      delete stack_info_tip;
      stack_info_tip = NULL;
    }
  else
    {
      stack_info_tip = new StackInfoTip(bigmap_image, mpos, stile);
    }
}

void GameWindow::on_bigmap_tip_changed(Glib::ustring tip, MapTipPosition pos, bool timeout)
{
  if (tip.empty())
    hide_map_tip();
  else
    show_map_tip(tip, pos, timeout);
}

void GameWindow::show_map_tip(Glib::ustring msg, MapTipPosition pos, bool timeout)
{
  map_tip_timer.disconnect ();
  // init the map tip
  if (map_tip != NULL)
    delete map_tip;
  map_tip = new Gtk::Window(Gtk::WINDOW_POPUP);

  map_tip->add_events (Gdk::POINTER_MOTION_MASK);
  map_tip->signal_motion_notify_event().connect (sigc::hide(method(hide_map_tip)));
  map_tip->set_transient_for (*window);
  Gtk::Frame *f = manage(new Gtk::Frame);
  f->property_shadow_type() = Gtk::SHADOW_ETCHED_OUT;

  Gtk::Label *l = manage(new Gtk::Label);
  l->set_justify(Gtk::JUSTIFY_CENTER);
  l->set_padding(6, 6);
  l->set_text(msg);
  f->add(*l);

  map_tip->add(*f);
  f->show_all();

  // get screen position
  Vector<int> p;
  bigmap_image->get_window()->get_origin(p.x, p.y);
  p += pos.pos;

  Vector<int> size(0, 0);
  map_tip->get_size (size.x, size.y);

  switch (pos.justification)
    {
    case MapTipPosition::LEFT:
      map_tip->set_gravity(Gdk::GRAVITY_NORTH_WEST);
      break;
    case MapTipPosition::RIGHT:
      map_tip->set_gravity(Gdk::GRAVITY_NORTH_EAST);
      p.x -= size.x;
      break;
    case MapTipPosition::TOP:
      map_tip->set_gravity(Gdk::GRAVITY_NORTH_WEST);
      break;
    case MapTipPosition::BOTTOM:
      map_tip->set_gravity(Gdk::GRAVITY_SOUTH_WEST);
      p.y -= size.y;
      break;
    }

  map_tip->move(p.x, p.y);

  map_tip->show();
  if (timeout)
    {
      map_tip_timer = Glib::signal_timeout().connect (method(hide_map_tip), 1400);
    }
}

bool GameWindow::hide_map_tip()
{
  map_tip_timer.disconnect ();
  if (map_tip != NULL)
    {
      delete map_tip;
      map_tip = NULL;
    }
  return true;
}

Reward* GameWindow::on_sage_visited (Ruin *ruin, Sage *sage, Stack *stack)
{
  SageDialog d(*window, sage, static_cast<Hero*>(stack->getFirstHero()), ruin);
  Reward *reward = d.run();
  d.hide();
  return reward;
}

void GameWindow::on_ruin_rewarded (Reward_Ruin *reward)
{
  RuinRewardedDialog d(*window, reward);
  d.run();
  d.hide();
}

void GameWindow::on_ruin_searched(Ruin *ruin, Stack *stack, Reward *reward)
{
  if (reward->getType() == Reward::RUIN && ruin->getType() == Ruin::SAGE)
    return on_ruin_rewarded(static_cast<Reward_Ruin*>(reward));
  LwDialog dialog(*window, "ruin-searched-dialog.ui");
  dialog.set_title(ruin->getName());
  Glib::RefPtr<Gtk::Builder> xml = dialog.get_builder();

  Gtk::Label *label;
  xml->get_widget("label", label);

  Glib::ustring s = label->get_text();
  s += "\n\n";
  Glib::ustring hero = stack->getFirstHero()->getName();
  switch (reward->getType())
    {
    case Reward::GOLD:
        {
          Reward_Gold *gold = dynamic_cast<Reward_Gold*>(reward);
          if (ruin->hasSage())
            s += String::ucompose(_("%1 is given %2 gold pieces."), hero,
                                  gold->getGold());
          else
            s += String::ucompose(_("%1 finds %2 gold pieces."), hero,
                                  gold->getGold());
        }
      break;
    case Reward::ALLIES:
        {
          Reward_Allies *allies = dynamic_cast<Reward_Allies*>(reward);
          if (ruin->hasSage())
            s += String::ucompose(_("%1 is given %2 allies!"), hero,
                                  allies->getNoOfAllies());
          else
            s += String::ucompose(_("%1 finds %2 allies!"), hero,
                                  allies->getNoOfAllies());
        }
      break;
    case Reward::ITEM:
        {
          Reward_Item *item = dynamic_cast<Reward_Item*>(reward);
          if (ruin->hasSage())
            s += String::ucompose(_("%1 is given the %2!"), hero,
                                  item->getItem()->getName());
          else
            s += String::ucompose(_("%1 finds the %2!"), hero,
                                  item->getItem()->getName());
        }
      break;
    case Reward::MAP:
        {
          Reward_Map *map = dynamic_cast<Reward_Map*>(reward);
          if (ruin->hasSage())
            s += String::ucompose(_("%1 is given a %2!"), hero, map->getName());
          else
            s += String::ucompose(_("%1 finds a %2!"), hero, map->getName());
        }
      break;
    case Reward::RUIN:
      //ruins are not populated with ruin-rewards.
      break;
    }

  label->set_text(s);
  dialog.run_and_hide();
}

void GameWindow::on_ruinfight_started(Stack *attackers, Keeper *keeper)
{
  LwDialog dialog(*window, "ruinfight-started-dialog.ui");
  //so and so encounters a wolf...
  dialog.set_title(_("Searching"));
  Glib::RefPtr<Gtk::Builder> xml = dialog.get_builder();
  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s = label->get_text();
  s = "\n\n";
  s += String::ucompose(_("%1 encounters %2..."),
                        attackers->getFirstHero()->getName(),
                        keeper->getName());
  label->set_text(s);
  dialog.run_and_hide();
}

void GameWindow::on_ruinfight_finished(Fight::Result result)
{
  LwDialog dialog(*window, "ruinfight-finished-dialog.ui");
  Glib::RefPtr<Gtk::Builder> xml = dialog.get_builder();
  if (result == Fight::ATTACKER_WON)
    dialog.set_title(_("Hero Victorious"));
  else
    dialog.set_title(_("Hero Defeated"));

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s = label->get_text();
  s = "\n\n";
  if (result == Fight::ATTACKER_WON)
    s += _("...and is victorious!");
  else
    s += _("...and is slain by it!");
  label->set_text(s);

  Gtk::Image *image;
  xml->get_widget("image", image);
  if (result == Fight::ATTACKER_WON)
    image->property_pixbuf() =
      ImageCache::getInstance()->getDialogPic
      (ImageCache::DIALOG_RUIN_SUCCESS,
       FontSize::getInstance ()->get_height ())->to_pixbuf();
  else
    image->property_pixbuf() =
      ImageCache::getInstance()->getDialogPic
      (ImageCache::DIALOG_RUIN_DEFEAT,
       FontSize::getInstance ()->get_height ())->to_pixbuf();
  image->show();

  dialog.run_and_hide();
}

void GameWindow::on_fight_started(LocationBox box, Fight &fight)
{
  game->get_bigmap().setFighting(box);
  game->get_bigmap().draw();
  FightWindow d(*window, fight);

  while (g_main_context_iteration(NULL, FALSE)); //doEvents
  Glib::usleep (Configuration::s_displayFightRoundDelaySlow);
  d.run(&d_quick_fights);
  d.hide();
  game->get_bigmap().setFighting(LocationBox(Vector<int>(-1,-1)));
  game->get_bigmap().draw();
  if (Playerlist::getActiveplayer()->getType() == Player::HUMAN)
    d_quick_fights = false;
}

void GameWindow::on_hero_brings_allies (int numAllies)
{
  LwDialog dialog(*window, "hero-brings-allies-dialog.ui");
  dialog.set_title(_("Hero brings allies!"));
  Gtk::Label *label;
  Glib::RefPtr<Gtk::Builder> xml = dialog.get_builder();
  xml->get_widget("label", label);
  Glib::ustring s = String::ucompose
    (ngettext("The hero brings %1 ally!",
              "The hero brings %1 allies!", numAllies), numAllies);
  label->set_text(s);
  dialog.run_and_hide();
}

bool GameWindow::on_hero_offers_service(Player *player, HeroProto *hero, City *city, int gold)
{
  HeroOfferDialog d(*window, player, hero, city, gold);
  bool retval = d.run();
  d.hide();
  return retval;
}

bool GameWindow::on_enemy_offers_surrender(int numPlayers)
{
  SurrenderDialog d(*window, numPlayers);
  return d.run_and_hide() == Gtk::RESPONSE_ACCEPT;
}

void GameWindow::on_surrender_answered (bool accepted)
{
  if (accepted)
    on_message_requested
      (_("You graciously and benevolently accept their offer."));
  else
    {
      SurrenderRefusedDialog d(*window);
      d.run_and_hide();
    }
}

bool GameWindow::on_stack_considers_treachery (Player *them)
{
  LwDialog dialog(*window, "treachery-dialog.ui");
  Glib::RefPtr<Gtk::Builder> xml = dialog.get_builder();
  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s = String::ucompose(_("Are you sure you want to attack %1?"), 
                                     them->getName());
  s += "\n";
  s += _("Other players may not like this!");
  label->set_text(s);
  int response = dialog.run_and_hide();
  if (response == Gtk::RESPONSE_DELETE_EVENT)
    return false;
  else if (response == Gtk::RESPONSE_ACCEPT)
    return true;
  else
    return false;
}


void GameWindow::on_temple_visited(Temple *temple)
{
  RuinReportDialog d(*window, temple->getPos());
  d.run();
  d.hide();
}

bool GameWindow::on_temple_searched(Hero *hero, Temple *temple, int blessCount)
{
  QuestsManager *qm = QuestsManager::getInstance();
  bool hasHero = hero != NULL;
  bool ask_quest = false;

  LwDialog dialog(*window, "temple-visit-dialog.ui");
  dialog.set_title(temple->getName());
  Glib::RefPtr<Gtk::Builder> xml = dialog.get_builder();
  Gtk::Label *l;
  Gtk::Button *close_button;
  Gtk::Button *accept_button;
  xml->get_widget("label", l);
  xml->get_widget("close_button", close_button);
  xml->get_widget("accept_button", accept_button);

  if (GameScenarioOptions::s_play_with_quests == 
      GameParameters::ONE_QUEST_PER_PLAYER)
    {
      if (qm->getPlayerQuests(Playerlist::getActiveplayer()).size() == 0 &&
          hasHero)
        ask_quest = true;
    }
  else if (GameScenarioOptions::s_play_with_quests == GameParameters::ONE_QUEST_PER_HERO)
    {
      if (hasHero && hero->hasQuest() == false)
        ask_quest = true;
    }

  Glib::ustring s;
  if (blessCount > 0)
    s += String::ucompose(
			  ngettext("%1 army has been blessed!",
				   "%1 armies have been blessed!", blessCount), blessCount);
  else
    s += _("We have already blessed thee!");

  l->set_text(s);
  s = l->get_text() + "\n" + _("Seek more blessings in far temples!");
  l->set_text(s);
  if (ask_quest)
    {
      s = l->get_text() + "\n\n" + _("Do you seek a quest?");
      l->set_text(s);
    }

  if (ask_quest == false)
    {
      close_button->hide();
      close_button->set_no_show_all(true);
      s = _("_Close");
      accept_button->set_label(s);
    }

  if (blessCount > 0)
    Snd::getInstance()->play("bless", 1);

  int response = dialog.run_and_hide();

  if (ask_quest == false)
    response = Gtk::RESPONSE_CANCEL;

  if (response == Gtk::RESPONSE_ACCEPT)		// accepted a quest
    return true;
  else
    return false;
}

void GameWindow::on_quest_assigned(Hero *hero, Quest *quest)
{
  QuestAssignedDialog d(*window, hero, quest);
  d.run();
  d.hide();
}

static bool
hero_has_quest_here (Stack *s, City *c, bool *pillage, bool *sack, bool *raze, bool *occupy)
{
  Player *p = Playerlist::getActiveplayer();
  std::vector<Quest*> questlist;
  *pillage = false;
  *sack = false;
  *raze = false;
  *occupy = false;

  QuestsManager *q_mgr = QuestsManager::getInstance();
  questlist = q_mgr->getPlayerQuests(p);
  /* loop over all quests */
  /* for each quest, check the quest type */
  for (std::vector<Quest*>::iterator i = questlist.begin();
       i != questlist.end(); ++i)
    {
      if ((*i) == NULL)
	continue;
      if ((*i)->isPendingDeletion() == true)
	continue;
      switch ((*i)->getType())
	{
	case Quest::CITYSACK:
	case Quest::CITYRAZE:
	case Quest::CITYOCCUPY:
	  if ((*i)->getType() == Quest::CITYSACK)
	    {
	      if (dynamic_cast<QuestCitySack*>((*i))->getCity() != c)
		continue;
	    }
	  else if ((*i)->getType() == Quest::CITYOCCUPY)
	    {
	      if (dynamic_cast<QuestCityOccupy*>((*i))->getCity() != c)
		continue;
	    }
	  else if ((*i)->getType() == Quest::CITYRAZE)
	    {
	      if (dynamic_cast<QuestCityRaze*>((*i))->getCity() != c)
		continue;
	    }
	  /* now check if the quest's hero is in our stack */
	  for (Stack::iterator it = s->begin(); it != s->end(); ++it)
	    {
	      if ((*it)->isHero())
		{
		  if ((*it)->getId() == (*i)->getHeroId())
		    {
		      /* hey we found one, set the corresponding boolean */
		      if ((*i)->getType() == Quest::CITYSACK)
			*sack = true;
		      else if ((*i)->getType() == Quest::CITYRAZE)
			*raze = true;
		      else if ((*i)->getType() == Quest::CITYOCCUPY)
			*occupy = true;
		    }
		}
	    }
	  break;
	case Quest::PILLAGEGOLD:
	  *pillage = true;
	  *sack = true;
	  break;
	}
    }
  if ((*raze) || (*sack) || (*occupy))
    return true;
  else
    return false;
}

CityDefeatedAction GameWindow::on_city_defeated(City *city, int gold)
{
  LwDialog dialog(*window, "city-defeated-dialog.ui");
  CityDefeatedAction retval = CITY_DEFEATED_OCCUPY;
  Gtk::Button *raze_button;
  Gtk::Button *sack_button;
  Gtk::Button *pillage_button;
  Gtk::Button *occupy_button;
  if (gold)
    on_city_looted (city, gold);

  Glib::RefPtr<Gtk::Builder> xml = dialog.get_builder();
  Gtk::Image *image;
  xml->get_widget("city_image", image);
  image->property_pixbuf() =
    ImageCache::getInstance()->getDialogPic
    (ImageCache::DIALOG_CONQUERED_CITY,
     FontSize::getInstance ()->get_height ())->to_pixbuf();
  image->show();

  Gtk::Label *label;
  xml->get_widget("label", label);
  int width = 0, height = 0;
  image->get_size_request(width, height);
  label->set_size_request(width, height);

  Glib::ustring name;
  Player *p = Playerlist::getActiveplayer();
  Army *h = NULL;
  if (p->getActivestack())
    h = p->getActivestack()->getFirstHero();
  if (h)
    name = h->getName();
  else
    name = p->getName();

  Glib::ustring s;
  switch (Rnd::rand() % 4)
    {
    case 0: s = _("%1, you have triumphed in the battle of %2."); break;
    case 1: s = _("%1, you have claimed victory in the battle of %2."); break;
    case 2: s = _("%1, you have shown no mercy in the battle of %2."); break;
    case 3: s = _("%1, you have slain the foe in the battle of %2."); break;
    }

  s = String::ucompose(s, name, city->getName());
  s += "\n\n";
  s += label->get_text();
  label->set_text(s);

  xml->get_widget("raze_button", raze_button);
  xml->get_widget("sack_button", sack_button);
  xml->get_widget("pillage_button", pillage_button);
  xml->get_widget("occupy_button", occupy_button);

  switch (GameScenarioOptions::s_sacking_mode)
    {
    case GameParameters::SACKING_ALWAYS:
    case GameParameters::SACKING_ON_CAPTURE:
      sack_button->set_sensitive(true);
      pillage_button->set_sensitive(true);
      break;
    case GameParameters::SACKING_ON_QUEST:
    case GameParameters::SACKING_NEVER:
      sack_button->set_sensitive(false);
      pillage_button->set_sensitive(false);
      break;
    }
  raze_button->set_sensitive
    (GameScenarioOptions::s_razing_cities == GameParameters::ON_CAPTURE || 
     GameScenarioOptions::s_razing_cities == GameParameters::ALWAYS);

  bool quest_default = false;
  if (h) /* if there was a hero in the stack */
    {
      bool pillage, sack, raze, occupy;
      if (hero_has_quest_here (p->getActivestack(), city, 
			       &pillage, &sack, &raze, &occupy))
	{
	  if (pillage)
	    {
              quest_default = true;
              pillage_button->set_sensitive(true);
              pillage_button->property_can_focus() = true;
              pillage_button->property_has_focus() = true;
              pillage_button->property_can_default() = true;
              pillage_button->property_receives_default() = true;
              pillage_button->property_has_default() = true;
	      pillage_button->grab_default();
	    }
	  if (sack)
	    {
              quest_default = true;
              sack_button->set_sensitive(true);
              sack_button->property_can_focus() = true;
              sack_button->property_has_focus() = true;
              sack_button->property_can_default() = true;
              sack_button->property_receives_default() = true;
              sack_button->property_has_default() = true;
	      sack_button->grab_default();
	    }
	  if (raze)
	    {
              quest_default = true;
              raze_button->property_can_focus() = true;
              raze_button->property_has_focus() = true;
              raze_button->property_can_default() = true;
              raze_button->property_receives_default() = true;
              raze_button->property_has_default() = true;
	      raze_button->grab_default();
	    }
	  if (occupy)
	    {
              quest_default = true;
              occupy_button->property_can_focus() = true;
              occupy_button->property_has_focus() = true;
              occupy_button->property_can_default() = true;
              occupy_button->property_has_default() = true;
              occupy_button->property_receives_default() = true;
	      occupy_button->grab_default();
	    }
	}
    }

  if (city->getNoOfProductionBases() <= 0)
    pillage_button->hide();

  if (city->getNoOfProductionBases() <= 1)
    sack_button->hide();

  dialog.get()->show();

  if (quest_default == false)
    {
      occupy_button->property_can_focus() = true;
      occupy_button->property_has_focus() = true;
      occupy_button->property_can_default() = true;
      occupy_button->property_has_default() = true;
      occupy_button->property_receives_default() = true;
      occupy_button->grab_default();
    }

  while (1)
    {
      int response = dialog.get()->run();
      switch (response) 
	{
	case 1: retval = CITY_DEFEATED_OCCUPY; break;
	case 2: 
		{
		  bool razed = CityWindow::on_raze_clicked(city, dialog.get());
		  if (razed == false)
		    continue;
		  retval = CITY_DEFEATED_RAZE;
		  break;
		}
	case 3: retval = CITY_DEFEATED_PILLAGE; break;
	case 4: retval = CITY_DEFEATED_SACK; break;
	default: break;
	}
      if (retval)
	break;
    }
  dialog.get()->hide();
  return retval;
}

void GameWindow::on_city_looted (City *city, int gold)
{
  LwDialog dialog(*window, "city-looted-dialog.ui");
  dialog.set_title(String::ucompose(_("%1 Looted"), city->getName()));
  Glib::RefPtr<Gtk::Builder> xml = dialog.get_builder();
  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s = label->get_text();
  s += "\n\n";
  s += String::ucompose(
			ngettext("Your armies loot %1 gold piece.",
				 "Your armies loot %1 gold pieces.", gold), gold);
  label->set_text(s);
  dialog.run_and_hide();
}

void GameWindow::on_city_pillaged(City *city, int gold, int pillaged_army_type)
{
  LwDialog dialog(*window, "city-pillaged-dialog.ui");
  ImageCache *gc = ImageCache::getInstance();
  Player *player = city->getOwner();
  unsigned int as = player->getArmyset();

  dialog.set_title(String::ucompose(_("Pillaged %1"), city->getName()));
  Glib::RefPtr<Gtk::Builder> xml = dialog.get_builder();
  Gtk::Image *pillaged_army_type_image;
  Gtk::Label *pillaged_army_type_cost_label;
  xml->get_widget("pillaged_army_type_cost_label", pillaged_army_type_cost_label);
  xml->get_widget("pillaged_army_type_image", pillaged_army_type_image);
  if (gold == 0)
    {
      Glib::RefPtr<Gdk::Pixbuf> empty_pic = 
        gc->getCircledArmyPic(as, 0, player, NULL, false, Shield::NEUTRAL, 
                              false,
                              FontSize::getInstance()->get_height ())->to_pixbuf();
      pillaged_army_type_image->set(empty_pic);
      pillaged_army_type_cost_label->set_text("");
    }
  else
    {
      Glib::RefPtr<Gdk::Pixbuf> pic;
      pic = gc->getCircledArmyPic(as, pillaged_army_type, player, NULL, false,
                                  Shield::NEUTRAL, true,
                                  FontSize::getInstance()->get_height ())->to_pixbuf();
      pillaged_army_type_image->property_pixbuf() = pic;
      pillaged_army_type_cost_label->set_text(String::ucompose("%1 gp", gold));
    }
  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s = label->get_text();
  s += "\n\n";
  s += String::ucompose(
			ngettext("The loot is worth %1 gold piece.",
				 "The loot is worth %1 gold pieces.",
				 gold), gold);
  label->set_text(s);

  dialog.run_and_hide();
}

void GameWindow::on_city_sacked(City *city, int gold, std::list<guint32> sacked_types)
{
  LwDialog dialog(*window, "city-sacked-dialog.ui");
  ImageCache *gc = ImageCache::getInstance();
  Player *player = city->getOwner();
  unsigned int as = player->getArmyset();

  dialog.set_title(String::ucompose(_("Sacked %1"), city->getName()));

  Glib::RefPtr<Gtk::Builder> xml = dialog.get_builder();
  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s;
  s = String::ucompose(_("The city of %1 is sacked\nfor %2 gold!\n\n"),
		       city->getName(), gold);
  s += String::ucompose(
			ngettext("Ability to produce %1 unit has been lost\nand only 1 unit remains",
				 "Ability to produce %1 units has been lost\nand only 1 unit remains",
				 sacked_types.size()), sacked_types.size());
  label->set_text(s);

  Gtk::Image *sacked_army_1_image;
  Gtk::Image *sacked_army_2_image;
  Gtk::Image *sacked_army_3_image;
  Gtk::Label *sacked_army_1_cost_label;
  Gtk::Label *sacked_army_2_cost_label;
  Gtk::Label *sacked_army_3_cost_label;
  xml->get_widget("sacked_army_1_image", sacked_army_1_image);
  xml->get_widget("sacked_army_2_image", sacked_army_2_image);
  xml->get_widget("sacked_army_3_image", sacked_army_3_image);
  xml->get_widget("sacked_army_1_cost_label", sacked_army_1_cost_label);
  xml->get_widget("sacked_army_2_cost_label", sacked_army_2_cost_label);
  xml->get_widget("sacked_army_3_cost_label", sacked_army_3_cost_label);

  Glib::RefPtr<Gdk::Pixbuf> pic;
  Glib::RefPtr<Gdk::Pixbuf> empty_pic =
    gc->getCircledArmyPic(as, 0, player, NULL, false, Shield::NEUTRAL, 
                          false,
                          FontSize::getInstance()->get_height ())->to_pixbuf();
  int i = 0;
  Gtk::Label *sack_label = NULL;
  Gtk::Image *sack_image = NULL;
  for (std::list<guint32>::iterator it = sacked_types.begin();
       it != sacked_types.end(); ++it)
    {
      switch (i)
	{
	case 0:
	  sack_label = sacked_army_1_cost_label;
	  sack_image = sacked_army_1_image;
	  break;
	case 1:
	  sack_label = sacked_army_2_cost_label;
	  sack_image = sacked_army_2_image;
	  break;
	case 2:
	  sack_label = sacked_army_3_cost_label;
	  sack_image = sacked_army_3_image;
	  break;
	}
      pic = gc->getCircledArmyPic(as, *it, player, NULL, false, 
                                  Shield::NEUTRAL, true,
                                  FontSize::getInstance()->get_height ())->to_pixbuf();
      sack_image->property_pixbuf() = pic;
      const ArmyProto *a = 
	Armysetlist::getInstance()->getArmy (player->getArmyset(), *it);
      s = String::ucompose(_("%1 gp"), a->getNewProductionCost() / 2);
      sack_label->set_text(s);
      i++;
    }
  for (i = sacked_types.size(); i < 3; i++)
    {
      switch (i)
	{
	case 0:
	  sack_label = sacked_army_1_cost_label;
	  sack_image = sacked_army_1_image;
	  break;
	case 1:
	  sack_label = sacked_army_2_cost_label;
	  sack_image = sacked_army_2_image;
	  break;
	case 2:
	  sack_label = sacked_army_3_cost_label;
	  sack_image = sacked_army_3_image;
	  break;
	}
      sack_image->set(empty_pic);
      sack_label->set_text("");
    }
  dialog.run_and_hide();
}

void GameWindow::on_city_razed (City *city)
{
  LwDialog dialog(*window, "city-razed-dialog.ui");
  dialog.set_title(String::ucompose(_("Razed %1"), city->getName()));
  Glib::RefPtr<Gtk::Builder> xml = dialog.get_builder();
  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s = 
    String::ucompose(_("The city of %1 is in ruins!"), city->getName());
  label->set_text(s);
  dialog.run_and_hide();
}

void GameWindow::on_city_visited(City *city)
{
  CityWindow d(*window, city, 
	       GameScenarioOptions::s_razing_cities == GameParameters::ALWAYS,
	       GameScenarioOptions::s_see_opponents_production);

  d.run();
  d.hide();
}

void GameWindow::on_ruin_visited(Ruin *ruin)
{
  RuinReportDialog d(*window, ruin->getPos());
  d.run();
  d.hide();
}

void GameWindow::show_shield_turn() //show turn indicator
{
  Playerlist* pl = Playerlist::getInstance();
  ImageCache *gc = ImageCache::getInstance();
  unsigned int c = 0;
  for (Playerlist::iterator i = pl->begin(); i != pl->end(); ++i)
    {
      if (pl->getNeutral() == (*i))
	continue;
      if ((*i)->isDead()) 
	{
	  shield_image[c]->clear();
	  turn_hbox->remove(dynamic_cast<Gtk::Widget&>(*shield_image[c]));
          turn_hbox->queue_resize();
	  continue;
	}
      if (*i == pl->getActiveplayer())
        {
          PixMask *s =
            gc->getShieldPic (1, (*i), false,
                              FontSize::getInstance ()->get_height ())->copy ();
          ImageCache::add_underline (&s, pl->getActiveplayer()->getColor (),
                                     FontSize::getInstance ()->get_height ());
          shield_image[c]->property_pixbuf() = s->to_pixbuf();
          delete s;
        }
      else
        shield_image[c]->property_pixbuf() =
          gc->getShieldPic(1, (*i), false,
                           FontSize::getInstance()->get_height ())->to_pixbuf();
      if (*i == pl->getActiveplayer())
        shield_image[c]->property_margin_top() = 0;
      else
        shield_image[c]->property_margin_top() =
          FontSize::getInstance ()->get_height () / 3;
      shield_image[c]->property_tooltip_text() = (*i)->getName();
      c++;
    }
  for (unsigned int i = c; i < MAX_PLAYERS; i++)
    shield_image[i]->clear();
}

void GameWindow::on_remote_next_player_turn()
{
  status_box->reset_progress();
  status_box->on_stack_info_changed(NULL);
  while (g_main_context_iteration(NULL, FALSE)); //doEvents

  d_quick_fights = false;
  show_shield_turn();
  turn_label->set_markup(String::ucompose("<b>%1 %2</b>", _("Turn"),
                                          GameScenarioOptions::s_round));
}

void GameWindow::on_next_player_turn(Player *player, unsigned int turn_number)
{
  status_box->reset_progress();
  status_box->on_stack_info_changed(NULL);
  while (g_main_context_iteration(NULL, FALSE)); //doEvents

  d_quick_fights = false;
  show_shield_turn();
  if (player->getType() != Player::HUMAN)
    return;

  LwDialog dialog (*window, "next-player-turn-dialog.ui");

  Glib::RefPtr<Gtk::Builder> xml = dialog.get_builder();
  Gtk::Image *image;
  xml->get_widget("image", image);
  image->property_pixbuf() =
    ImageCache::getInstance()->getDialogPic
    (ImageCache::DIALOG_NEXT_TURN,
     FontSize::getInstance ()->get_height ())->to_pixbuf();

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s = String::ucompose(_("%1\nTurn %2"), player->getName(), 
                                     turn_number);
  label->set_text(s);

  dialog.run_and_hide();
  show();
}

void GameWindow::on_medal_awarded_to_army(Army *army, int medaltype)
{
  ImageCache *gc = ImageCache::getInstance();
  LwDialog dialog(*window, "medal-awarded-dialog.ui");
  Glib::RefPtr<Gtk::Builder> xml = dialog.get_builder();
  Gtk::Image *image;
  xml->get_widget("image", image);
  Player *active = Playerlist::getInstance()->getActiveplayer();
  image->property_pixbuf() = 
    gc->getCircledArmyPic(active->getArmyset(), army->getTypeId(), active, 
		   army->getMedalBonuses(), false, Shield::NEUTRAL, 
                   true,
                   FontSize::getInstance ()->get_height ())->to_pixbuf();
  Gtk::Image *medal_image;
  xml->get_widget("medal_image", medal_image);
  medal_image->property_pixbuf() = 
    gc->getMedalPic(true, medaltype,
                    FontSize::getInstance ()->get_height ())->to_pixbuf();

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s;
  if (medaltype == 0)
    s += String::ucompose(_("Your unit of %1 is awarded the avenger's medal of valour!"), army->getName());
  else if (medaltype == 1)
    s += String::ucompose(_("Your unit of %1 is awarded the defender's medal of bravery!"), army->getName());
  else if (medaltype == 2)
    s += String::ucompose(_("Your unit of %1 is awarded the veteran's medal!"), army->getName());
  else
    s += String::ucompose(_("Your unit of %1 is awarded a medal!"), army->getName());
  label->set_text(s);

  dialog.run_and_hide();
}

Army::Stat GameWindow::on_hero_gains_level(Hero *hero)
{
  ArmyGainsLevelDialog d(*window, hero, GameScenario::s_hidden_map);
  d.run_and_hide();
  return d.get_selected_stat();
}

void GameWindow::on_game_loaded(Player *player)
{
  LwDialog dialog(*window, "game-loaded-dialog.ui");
  Glib::RefPtr<Gtk::Builder> xml = dialog.get_builder();
  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s = String::ucompose(_("%1, your turn continues."), 
                                     player->getName());
  label->set_text(s);
  dialog.run_and_hide();
}

void GameWindow::on_quest_completed(Quest *quest, Reward *reward)
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  QuestCompletedDialog d(*window, quest, reward);
  d.run();
  d.hide();
}

void GameWindow::on_quest_expired(Quest *quest)
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  LwDialog dialog(*window, "quest-expired-dialog.ui");

  Glib::RefPtr<Gtk::Builder> xml = dialog.get_builder();
  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s = String::ucompose(_("%1 did not complete the quest."),
                                     quest->getHeroName());
  s += "\n\n";

  // add messages from the quest
  std::queue<Glib::ustring> msgs;
  quest->getExpiredMsg(msgs);
  while (!msgs.empty())
    {
      s += msgs.front();
      msgs.pop();
      if (!msgs.empty())
	s += "\n\n";
    }

  label->set_text(s);

  dialog.run_and_hide();
}

void GameWindow::on_inspect_activated ()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  if (Playerlist::getActiveplayer()->getHeroes().size() == 0)
    return;
  Hero *hero = NULL;
  Vector<int> pos = Vector<int>(-1,-1);
  if (status_box->get_currently_selected_stack() != NULL)
    {
      hero = dynamic_cast<Hero*>(status_box->get_currently_selected_stack()->getFirstHero());
      pos = status_box->get_currently_selected_stack()->getPos();
    }
    
  HeroDialog d(*window, hero, pos);
  d.run();
  d.hide();
}
void GameWindow::on_plant_standard_activated ()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  Playerlist::getActiveplayer()->heroPlantStandard(NULL);
}
    
void GameWindow::on_stack_moves(Stack *stack, Vector<int> pos)
{
  Player *active = Playerlist::getInstance()->getActiveplayer();
  if (!active)
    return;
  if (active->getActivestack() != stack)
    return;
  if (GameMap::getEnemyCity(pos))
    return;
  if (GameMap::getEnemyStack(pos))
    return;
  int step = TIMER_BIGMAP_SELECTOR * 1000;
  for (int i = 0; i < Configuration::s_displaySpeedDelay; i += step)
    {
      game->get_bigmap().draw();
      while (g_main_context_iteration(NULL, FALSE)); //doEvents
      if (i + step > Configuration::s_displaySpeedDelay)
        step = Configuration::s_displaySpeedDelay - i;
      Glib::usleep(step);
    }
}

void GameWindow::on_advice_asked(float percent)
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  //we asked for advice on a fight, and we're being told that we 
  //have a PERCENT chance of winning the fight
  LwDialog dialog(*window, "military-advisor-dialog.ui");

  dialog.set_title(_("Advisor!"));

  Glib::RefPtr<Gtk::Builder> xml = dialog.get_builder();
  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s;

  int num = Rnd::rand() % 5;
  if (num == 0)
    s += _("My Good Lord!");
  else if (num == 1)
    s += _("Great and Worthy Lord!");
  else if (num == 2)
    s += _("O Champion of Justice!");
  else if (num == 3)
    s += _("O Mighty Leader!");
  else if (num == 4)
    s += _("O Great Warlord!");
  s += "\n";

  num = Rnd::rand() % 7;
  num = Rnd::rand() % 7;
  if (percent >= 90.0)
    {
      if (num == 0)
	s += _("This battle will surely be as simple as butchering sleeping cattle!");
      else if (num == 1)
	s += _("A battle here would be as simple as butchering sleeping cattle!");
      else if (num == 2)
	s += _("I believe this battle will surely be as simple as butchering sleeping cattle!");
      else if (num == 3)
	s += _("This battle would be as simple as butchering sleeping cattle!");
      else if (num == 4)
	s += _("A battle here would be as simple as butchering sleeping cattle!");
      else if (num == 5)
	s += _("I believe this battle will be as simple as butchering sleeping cattle!");
      else if (num == 6)
	s += _("This battle shall be as simple as butchering sleeping cattle!");
    }
  else if (percent >= 80.0)
    {
      if (num == 0)
	s += _("This battle will surely be an easy victory!  We cannot lose!");
      else if (num == 1)
	s += _("A battle here would be an easy victory!  We cannot lose!");
      else if (num == 2)
	s += _("I believe this battle will surely be an easy victory!  We cannot lose!");
      else if (num == 3)
	s += _("This battle would be an easy victory!  We cannot lose!");
      else if (num == 4)
	s += _("A battle here would be an easy victory!  We cannot lose!");
      else if (num == 5)
	s += _("I believe this battle will be an easy victory!  We cannot lose!");
      else if (num == 6)
	s += _("This battle shall be an easy victory!  We cannot lose!");
    }
  else if (percent >= 70.0)
    {
      if (num == 0)
	s += _("This battle will surely be a comfortable victory!");
      else if (num == 1)
	s += _("A battle here would be a comfortable victory!");
      else if (num == 2)
	s += _("I believe this battle will surely be a comfortable victory!");
      else if (num == 3)
	s += _("This battle would be a comfortable victory!");
      else if (num == 4)
	s += _("A battle here would be a comfortable victory!");
      else if (num == 5)
	s += _("I believe this battle will be a comfortable victory!");
      else if (num == 6)
	s += _("This battle shall be a comfortable victory!");
    }
  else if (percent >= 60.0)
    {
      if (num == 0)
	s += _("This battle will surely be a hard fought victory! But we shall win!");
      else if (num == 1)
	s += _("A battle here would be a hard fought victory! But we shall win!");
      else if (num == 2)
	s += _("I believe this battle will surely be a hard fought victory! But we shall win!");
      else if (num == 3)
	s += _("This battle would be a hard fought victory! But we shall win!");
      else if (num == 4)
	s += _("A battle here would be a hard fought victory! But we shall win!");
      else if (num == 5)
	s += _("I believe this battle will be a hard fought victory! But we shall win!");
      else if (num == 6)
	s += _("This battle shall be a hard fought victory! But we shall win!");
    }
  else if (percent >= 50.0)
    {
      if (num == 0)
	s += _("This battle will surely be very evenly matched!");
      else if (num == 1)
	s += _("A battle here would be very evenly matched!");
      else if (num == 2)
	s += _("I believe this battle will surely be very evenly matched!");
      else if (num == 3)
	s += _("This battle would be very evenly matched!");
      else if (num == 4)
	s += _("A battle here would be very evenly matched!");
      else if (num == 5)
	s += _("I believe this battle will be very evenly matched!");
      else if (num == 6)
	s += _("This battle shall be very evenly matched!");
    }
  else if (percent >= 40.0)
    {
      if (num == 0)
	s += _("This battle will surely be difficult but not impossible to win!");
      else if (num == 1)
	s += _("A battle here would be difficult but not impossible to win!");
      else if (num == 2)
	s += _("I believe this battle will surely be difficult but not impossible to win!");
      else if (num == 3)
	s += _("This battle would be difficult but not impossible to win!");
      else if (num == 4)
	s += _("A battle here would be difficult but not impossible to win!");
      else if (num == 5)
	s += _("I believe this battle will be difficult but not impossible to win!");
      else if (num == 6)
	s += _("This battle shall be difficult but not impossible to win!");
    }
  else if (percent >= 30.0)
    {
      if (num == 0)
	s += _("This battle will surely be a brave choice! I leave it to thee!");
      else if (num == 1)
	s += _("A battle here would be a brave choice! I leave it to thee!");
      else if (num == 2)
	s += _("I believe this battle will surely be a brave choice! I leave it to thee!");
      else if (num == 3)
	s += _("This battle would be a brave choice! I leave it to thee!");
      else if (num == 4)
	s += _("A battle here would be a brave choice! I leave it to thee!");
      else if (num == 5)
	s += _("I believe this battle will be a brave choice! I leave it to thee!");
      else if (num == 6)
	s += _("This battle shall be a brave choice! I leave it to thee!");
    }
  else if (percent >= 20.0)
    {
      if (num == 0)
	s += _("This battle will surely be a foolish decision!");
      else if (num == 1)
	s += _("A battle here would be a foolish decision!");
      else if (num == 2)
	s += _("I believe this battle will surely be a foolish decision!");
      else if (num == 3)
	s += _("This battle would be a foolish decision!");
      else if (num == 4)
	s += _("A battle here would be a foolish decision!");
      else if (num == 5)
	s += _("I believe this battle will be a foolish decision!");
      else if (num == 6)
	s += _("This battle shall be a foolish decision!");
    }
  else if (percent >= 10.0)
    {
      if (num == 0)
	s += _("This battle will surely be sheerest folly!  Thou shouldst not attack!");
      else if (num == 1)
	s += _("A battle here would be sheerest folly!  Thou shouldst not attack!");
      else if (num == 2)
	s += _("I believe this battle will surely be sheerest folly!  Thou shouldst not attack!");
      else if (num == 3)
	s += _("This battle would be sheerest folly!  Thou shouldst not attack!");
      else if (num == 4)
	s += _("A battle here would be sheerest folly!  Thou shouldst not attack!");
      else if (num == 5)
	s += _("I believe this battle will be sheerest folly!  Thou shouldst not attack!");
      else if (num == 6)
	s += _("This battle shall be sheerest folly!  Thou shouldst not attack!");
    }
  else
    {
      if (num == 0)
	s += _("This battle will surely be complete and utter suicide!");
      else if (num == 1)
	s += _("A battle here would be complete and utter suicide!");
      else if (num == 2)
	s += _("I believe this battle will surely be complete and utter suicide!");
      else if (num == 3)
	s += _("This battle would be complete and utter suicide!");
      else if (num == 4)
	s += _("A battle here would be complete and utter suicide!");
      else if (num == 5)
	s += _("I believe this battle will be complete and utter suicide!");
      else if (num == 6)
	s += _("This battle shall be complete and utter suicide!");
    }
  label->set_text(s);

  dialog.run_and_hide();
  return;
}

void GameWindow::on_show_lobby_activated()
{
  show_lobby.emit();
}

void GameWindow::on_online_help_activated()
{
  GError *errs = NULL;
  gtk_show_uri(window->get_screen()->gobj(), 
               "http://www.nongnu.org/lordsawar/manual/" PACKAGE_VERSION "/lordsawar.html", 0, &errs);

  return;
}

void GameWindow::on_player_replaced(Player *p)
{
  if (game)
    game->addPlayer(p);
}

void GameWindow::on_grid_toggled()
{
  game->get_bigmap().toggle_grid();
  if (game->get_bigmap().get_toggled() == false)
    pos_label->set_text("");
}

void GameWindow::give_some_cheese(Player *winner)
{
  //this is so we only give cheese once
  if (save_game_as_menuitem->property_sensitive() == true)
    {
      game->endOfGameRoaming(winner);
      game_button_box->give_some_cheese();
      end_turn_menuitem->set_sensitive(false);
      save_game_menuitem->set_sensitive(false);
      save_game_as_menuitem->set_sensitive(false);
      Playerlist::getActiveplayer()->clearFogMap();
      show_shield_turn();
      game->redraw();
      city_history_menuitem->activate();
    }
}

void GameWindow::on_commentator_comments(Glib::ustring comment)
{
  TimedMessageDialog dialog (*window, comment, 0);
  dialog.set_title(_("The Warlord Says..."));

  dialog.set_image (ImageCache::getInstance()->getDialogPic
                    (ImageCache::DIALOG_COMMENTATOR,
                     FontSize::getInstance ()->get_height ())->to_pixbuf());
  dialog.run_and_hide();
}
      
void GameWindow::on_abbreviated_fight_started(LocationBox box)
{
  game->get_bigmap().setFighting(box);
  game->get_bigmap().draw();
  while (g_main_context_iteration(NULL, FALSE)); //doEvents
  Glib::usleep (TIMER_BIGMAP_SELECTOR * 10000);
  game->get_bigmap().setFighting(LocationBox(Vector<int>(-1,-1)));
  game->get_bigmap().draw();
}

Item* GameWindow::on_select_item(std::list<Item*> items)
{
  Item *item = NULL;
  UseItemDialog d(*window, items);
  d.run();
  item = d.get_selected_item();
  d.hide();
  return item;
}
     
Player *GameWindow::on_select_item_victim_player()
{
  Player *player = NULL;
  UseItemOnPlayerDialog d(*window);
  player = d.run();
  d.hide();
  return player;
}
    
City *GameWindow::on_select_city_to_use_item_on(SelectCityMap::Type type)
{
  UseItemOnCityDialog d(*window, type);
  City *city = d.run();
  d.hide();
  return city;
}
    
void GameWindow::on_gold_stolen(Player *victim, guint32 gold_pieces)
{
  Glib::ustring s = 
    String::ucompose(ngettext("%1 gold piece was stolen from %2!",
                              "%1 gold pieces were stolen from %2!", 
                              gold_pieces), gold_pieces, victim->getName());
  TimedMessageDialog dialog(*window, s, 30);
  dialog.run_and_hide();
  return;
}

void GameWindow::on_ships_sunk(guint32 num_armies)
{
  Glib::ustring s = 
    String::ucompose(ngettext("%1 army unit was sunk to the watery depths!",
                              "%1 army units were sunk to the watery depths!", 
                              num_armies), num_armies);
  TimedMessageDialog dialog(*window, s, 30);
  dialog.run_and_hide();
  return;
}

void GameWindow::on_bags_picked_up(Hero *hero, guint32 num_bags)
{
  Glib::ustring s = 
    String::ucompose(ngettext("%1 bag was retrieved by %2!",
                              "%1 bags were retrieved by %2!", 
                              num_bags), num_bags, hero->getName());
  TimedMessageDialog dialog(*window, s, 30);
  dialog.run_and_hide();
  return;
}

void GameWindow::on_bridge_burned(Hero *hero)
{
  Glib::ustring s = 
    String::ucompose(_("%1 has burned a bridge!  None shall pass this way again!"), hero->getName());
  TimedMessageDialog dialog(*window, s, 30);
  dialog.run_and_hide();
  return;
}

void GameWindow::on_keeper_captured(Hero *hero, Ruin *ruin, Glib::ustring name)
{
  Glib::ustring s = 
    String::ucompose(_("%1 has turned %2 from %3!"), 
                     hero->getName(), name, ruin->getName());
  TimedMessageDialog dialog(*window, s, 30);
  dialog.run_and_hide();
  return;
}

void GameWindow::on_city_diseased(Glib::ustring name, guint32 num_armies)
{
  Glib::ustring s = 
    String::ucompose(ngettext("%1 unit in %2 have perished!",
                              "%1 units in %2 have perished!", num_armies),
                     num_armies, name);
  TimedMessageDialog dialog(*window, s, 30);
  dialog.run_and_hide();
  return;
}

void GameWindow::on_city_defended(Glib::ustring city_name, Glib::ustring army_name, guint32 num_armies)
{
  Glib::ustring s = 
    String::ucompose(ngettext("%1 unit of %2 have been raised in %3!",
                              "%1 units of %2 have been raised in %3!", 
                              num_armies), num_armies, army_name, city_name);
  TimedMessageDialog dialog(*window, s, 30);
  dialog.run_and_hide();
  return;
}

void GameWindow::on_city_persuaded(Glib::ustring city_name, guint32 num_armies)
{
  if (game)
    game->redraw();
  Glib::ustring s;
  if (num_armies != 0)
    s = String::ucompose
      (ngettext("%1 unit in %2 have been persuaded to fly your flag!",
                "%1 units in %2 have been persuaded to fly your flag!", 
                num_armies),
       num_armies, city_name);
  else
    s = String::ucompose
      (_("The citizens of %1 have been persuaded to fly your flag!"), 
       city_name);
  TimedMessageDialog dialog(*window, s, 30);
  dialog.run_and_hide();
  return;
}

void GameWindow::on_stack_teleported(Hero *hero, Glib::ustring city_name)
{
  Glib::ustring s = String::ucompose(_("%1 has teleported to %2!"), 
                                     hero->getName(), city_name);
  TimedMessageDialog dialog(*window, s, 30);

  dialog.run_and_hide();
  return;
}

void GameWindow::on_monster_summoned(Hero *hero, Glib::ustring name)
{
  Glib::ustring s = String::ucompose 
    (_("A unit of %1 has come to the aid of %2!"), name, hero->getName());
  TimedMessageDialog dialog(*window, s, 30);
  dialog.run_and_hide();
  return;
}

void GameWindow::on_worms_killed(Hero *hero, Glib::ustring name, guint32 num_killed)
{
  Glib::ustring s = 
    String::ucompose(ngettext("%1 unit of %2 was banished by %3!",
                              "%1 units of %2 were banished by %3!", 
                              num_killed), num_killed, name, hero->getName());
  TimedMessageDialog dialog(*window, s, 30);
  dialog.run_and_hide();
  return;
}

void GameWindow::on_mp_added_to_hero_stack(Hero *hero, guint32 mp)
{
  Glib::ustring s =
    String::ucompose(ngettext("%1 movement point was added to %2 and accompanying units!",
                              "%1 movement points were added to %2 and accompanying units!", 
                              mp), mp, hero->getName());
  TimedMessageDialog dialog(*window, s, 30);
  dialog.run_and_hide();
  return;
}

bool GameWindow::on_bigmap_scrolled(GdkEventScroll* event)
{
  if (!game)
    return true;
  bool ret = game->get_bigmap().scroll(event);
  game->get_bigmap().update_mouse_cursor();
  return ret;
}

void GameWindow::on_popup_stack_menu (Stack *stack)
{
  Gtk::Menu *menu = manage(new Gtk::Menu);
  Glib::ustring s = _("Info...");
  Gtk::MenuItem *item = manage(new Gtk::MenuItem(s));
  item->signal_activate().connect (method(on_stack_info_activated));
  item->show();
  menu->add(*item);

  s = _("Search");
  item = manage(new Gtk::MenuItem(s));
  item->signal_activate().connect
    (sigc::mem_fun(game, &Game::search_selected_stack));
  item->set_sensitive(GameMap::can_search (stack));
  item->show();
  menu->add(*item);

  StackTile *st = GameMap::getStacks (stack->getPos());
  if (st->size () > 1)
    s = _("Group");
  else
    s = _("Ungroup");
  item = manage(new Gtk::MenuItem(s));
  item->signal_activate().connect (method(on_group_ungroup_activated));
  item->show();
  menu->add(*item);
  if (stack->hasPath() && stack->enoughMoves())
    {
      s = _("Travel Along Path");
      item = manage(new Gtk::MenuItem(s));
      item->signal_activate().connect
        (sigc::mem_fun(game, &Game::move_selected_stack_along_path));
      item->show();
      menu->add(*item);
    }

   s = _("Stay Here");
   item = manage(new Gtk::MenuItem(s));
   item->signal_activate().connect
     (sigc::mem_fun(game, &Game::park_selected_stack));
   item->show();
   menu->add(*item);

  s = _("Defend");
  item = manage(new Gtk::MenuItem(s));
  item->signal_activate().connect
    (sigc::mem_fun(game, &Game::defend_selected_stack));
  item->show();
  menu->add(*item);

  s = _("Disband...");
  item = manage(new Gtk::MenuItem(s));
  item->signal_activate().connect (method(on_disband_activated));
  item->show();
  menu->add(*item);
  menu->accelerate (*window);
  menu->popup_at_pointer (reinterpret_cast<const GdkEvent*>(button_event));
}

void GameWindow::on_pointing_at_new_tile(Vector<int> tile)
{
  pos_label->set_text(String::ucompose("(%1, %2)", tile.x, tile.y));
}

bool GameWindow::on_window_state_event (GdkEventWindowState *e)
{
  if (e->window == window->get_window ()->gobj ())
    {
      if (e->changed_mask & GDK_WINDOW_STATE_MAXIMIZED)
        {
          if (e->new_window_state & GDK_WINDOW_STATE_MAXIMIZED)
            ; //maximized
          else
            window->set_default_size (unmaximized_box.get_width (),
                                      unmaximized_box.get_height ());
        }
    }
  return false;
}

bool GameWindow::on_configure_event (GdkEventConfigure *e)
{
  if (unmaximized_box.get_width () == 0)
    {
      unmaximized_box.set_width (e->width);
      unmaximized_box.set_height (e->height);
    }
  return false;
}

void GameWindow::on_quick_help_activated()
{
  show_quick_help.emit();
}

void GameWindow::on_best_fit_activated ()
{
  set_default_bigmap_zoom();
}

void GameWindow::on_tutorial_activated ()
{
  GError *errs = NULL;
  gtk_show_uri(window->get_screen()->gobj(),
               "https://vimeo.com/409439854", 0, &errs);
  return;
}
