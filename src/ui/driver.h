//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2011, 2014, 2015, 2016, 2017, 2020,
//  2021 Ben Asselstine
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
#ifndef GUI_DRIVER_H
#define GUI_DRIVER_H

#include <memory>
#include <sigc++/trackable.h>

#include "splash-window.h"
#include "game-window.h"
#include "game-lobby-dialog.h"
#include "quick-help-window.h"
#include "new-network-game-download-window.h"
#include "game-parameters.h"
#include "../editor/main-window.h"
class GamePreferencesDialog;

class Profile;
// takes care of setting up the splash window and the game window, the
// interaction between them and the model classes
// it also takes care of the game lobby window.
class Driver: public sigc::trackable
{
 public:
    Driver(bool start_editor, Glib::ustring load_filename);
    ~Driver();

    void run();

 private:
    GameWindow* game_window;
    GameLobbyDialog* game_lobby_dialog;
    QuickHelpWindow* quick_help_window;
    SplashWindow* splash_window;
    NewNetworkGameDownloadWindow* download_window;
    MainWindow *editor_window;
    Glib::ustring d_load_filename;
    sigc::connection download_conn;
    sigc::connection upload_conn;
    sigc::connection heartbeat_conn;
    sigc::connection recv_conn;
    Player::Type robot_player_type;
    Glib::ustring d_advertised_scenario_id;
    unsigned int number_of_robots;
    Glib::ustring game_scenario_downloaded;
    sigc::signal<void, Glib::ustring> game_scenario_received;
    sigc::signal<void, Player*> player_replaced;
    sigc::signal<void> start_game_progress_tick;
    sigc::signal<void> start_game_progress_finish;

    void on_new_game_requested(GameParameters g, GamePreferencesDialog *gpd);
    void on_new_remote_network_game_requested(Glib::ustring host, unsigned short port, Profile *p);
    void on_new_hosted_network_game_requested(GameParameters g, int port, Profile *p, bool advertised, bool remotely_hosted);
    void on_load_hosted_network_game_requested(GameScenario *game_scenario, int port, Profile *p, bool advertised, bool remotely_hosted);
    void on_game_ended_and_load_network_game(Glib::ustring filename, int port, Profile *p, bool advertised, bool remotely_hosted);
    void on_new_pbm_game_requested(GameParameters g);
    void on_game_scenario_downloaded(Glib::ustring filename);
    void on_game_scenario_received(Glib::ustring path, Profile *p, sigc::connection con);
    void on_load_requested(Glib::ustring filename);
    void on_editor_requested(Glib::ustring filename = "");
    void on_editor_quit ();
    void on_quit_requested();

    void on_game_ended();
    void on_game_ended_and_start_new();

    void init_game_window();


    void on_hosted_player_sat_down(Player *player);
    void on_hosted_player_stood_up(Player *player);
    void on_hosted_player_changed_name(Player *player, Glib::ustring name);
    void on_hosted_player_changed_type(Player *player, int type);
    void on_client_player_sat_down(Player *player);
    void on_client_player_stood_up(Player *player);
    void on_client_player_changed_type(Player *player, int type);
    void on_server_went_away();
    void on_server_went_away_text();
    void on_client_could_not_connect();

    GameScenario *new_game(GameParameters g);
    GameScenario *load_game(Glib::ustring file_path, Gtk::Window *w = NULL);
    void stress_test(bool gui);
    void stressTestNextRound();

    void lordsawaromatic(Glib::ustring host, unsigned short port, Player::Type type, int num_players);
    void on_game_scenario_received_for_robots(Glib::ustring path);
    void on_game_over_for_headless_server(Player *p, GameScenario *g);
  

    bool heartbeat();

    void on_client_player_chat(Glib::ustring message);
    void on_hosted_player_chat(Glib::ustring message);
    void on_hosted_player_says_game_may_begin();

    void on_show_lobby_requested();
    void on_quick_help_requested();

    void start_network_game_requested(GameScenario *game_scenario,
				      NextTurnNetworked *next_turn);

    void on_keep_network_play_going(); //starts a new round

    GameScenario *create_new_scenario(GameParameters &g, GameScenario::PlayMode m);

    void get_default(int num_players, GameParameters &g);
    void serve (GameScenario *game_scenario);
    void remotely_serve (GameScenario *game_scenario, Profile *p);
    void on_client_sits_down_in_headless_server_game();
  
    void on_could_not_bind_to_port_for_headless_server(int port);
    void on_could_not_bind_to_port (int port);

    void advertise_game(GameScenario *game_scenario, Profile *p);
    void on_connected_to_gamelist_server_for_advertising(GameScenario *game_scenario, Profile *p);

    void on_advertising_response_received(Glib::ustring scenario_id);
    void unadvertise_game(Glib::ustring scenario_id, Profile *p);
    void on_connected_to_gamelist_server_for_advertising_removal(Glib::ustring scenario_id);
    void on_advertising_removal_response_received();

    void on_advertised_game_round_ends(GameScenario *game_scenario, Profile *p);

    void on_connected_to_gamehost_server_for_hosting_request (GameScenario *game_scenario);
    void on_got_game_host_response(Glib::ustring err, GameScenario *game_scenario);
    void on_remote_game_hosted(guint32 port, Glib::ustring err, sigc::connection con);
    void on_could_not_connect_to_gamehost_server();
    guint32 get_port ();
};


#endif
