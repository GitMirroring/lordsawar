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

#include <sigc++/functors/mem_fun.h>
#include <sigc++/bind.h>
#include <errno.h>
#include <string.h>

#include "driver.h"

#include <iostream>
#include "main.h"
#include "splash-window.h"
#include "game-window.h"
#include "game-lobby-dialog.h"
#include "defs.h"
#include "ImageCache.h"
#include "GameScenario.h"
#include "CreateScenario.h"
#include "counter.h"
#include "shieldsetlist.h"
#include "File.h"
#include "armysetlist.h"
#include "playerlist.h"
#include "player.h"
#include "citylist.h"
#include "xmlhelper.h"
#include "Configuration.h"
#include "ucompose.hpp"
#include "snd.h"
#include "timed-message-dialog.h"
#include "game-preferences-dialog.h"

#include "game-client.h"
#include "game-server.h"
#include "NextTurnHotseat.h"
#include "NextTurnNetworked.h"
#include "recently-played-game-list.h"
#include "recently-played-game.h"
#include "Backpack.h"
#include "Item.h"
#include "city.h"
#include "FogMap.h"
#include "history.h"
#include "game.h"
#include "stacklist.h"
#include "smallmap.h"
#include "new-random-map-dialog.h"
#include "network_player.h"
#include "profile.h"
#include "profilelist.h"
#include "gamelist-client.h"
#include "gamehost-client.h"
#include "tilesetlist.h"
#include "citysetlist.h"
#include "herotemplates.h"
#include "new-network-game-dialog.h"
#include "../editor/main-window.h"
#include "../editor/editor-splash-window.h"
#include "smallmap.h"
#include "ScenarioMedia.h"
#include "load-progress-window.h"
#include "smallmap.h"
#include "fight-window.h"

#define method(x) sigc::mem_fun(*this, &Driver::x)

Driver::Driver(bool start_editor, Glib::ustring load_filename)
 : game_window (NULL), game_lobby_dialog (NULL), quick_help_window (NULL),
    splash_window (NULL), download_window (NULL), editor_window (NULL),
    d_load_filename (load_filename), robot_player_type (Player::AI_SMART),
    number_of_robots (MAX_PLAYERS), game_scenario_downloaded ("")
{
    //here are the command-line options that don't bring up the splash screen:
    if (Main::instance().start_stress_test) 
      {
	Snd::deleteInstance();
	stress_test(Main::instance().view_stress_test);
	exit(0);
      }
    if (Main::instance().start_robots != 0) 
      {
	run();
	return;
      }
    if (Main::instance().start_headless_server)
      {
        GameScenario *game_scenario = NULL;
        Glib::ustring path = load_filename;
        if (load_filename.empty() == true)
          {
            GameParameters g;
            get_default(MAX_PLAYERS, g);
            game_scenario = create_new_scenario(g, GameScenario::NETWORKED);
            if (!game_scenario)
              {
                std::cerr << "Error: could not load randomly generated map." << 
                  std::endl;
                exit(1);
              }
          }
        else
          {
            bool broken = false;
            GameParameters g = GameScenario::loadGameParameters(load_filename, 
                                                                broken);
            if (!broken)
              game_scenario = load_game(load_filename);
            if (!game_scenario || broken)
              {
                std::cerr << "Error: could not load file " << 
                  load_filename << std::endl;
                exit(1);
              }
            game_scenario->setPlayMode(GameScenario::NETWORKED);
            game_scenario->initialize(g);
          }
        if (game_scenario)
          serve (game_scenario);
        return;
      }

    splash_window = new SplashWindow;
    splash_window->new_game_requested.connect(method(on_new_game_requested));
    splash_window->new_hosted_network_game_requested.connect
      (method(on_new_hosted_network_game_requested));
    splash_window->new_remote_network_game_requested.connect
      (method(on_new_remote_network_game_requested));
    splash_window->load_requested.connect(method(on_load_requested));
    splash_window->editor_requested.connect(method(on_editor_requested));
    splash_window->quit_requested.connect(method(on_quit_requested));

    if (start_editor)
      {
        on_editor_requested(load_filename);
        return;
      }
    splash_window->show();
    //here are the ones that do
    run();
}

void Driver::serve (GameScenario *game_scenario)
{
  if (!game_scenario)
    return;
  Playerlist::getInstance()->syncNeutral();
  //okay we're going to host a game, using this file as a scenario.
  GameServer *game_server = GameServer::getInstance();
  if (Main::instance().save_server_messages != "") 
    game_server->saveMessages
      (Main::instance ().save_server_messages);
  game_server->port_in_use.connect(method(on_could_not_bind_to_port_for_headless_server));
  Glib::ustring id = "";
  if (Profilelist::getInstance()->empty() == false)
    id = Profilelist::getInstance()->front()->getId();
  game_server->start(game_scenario, get_port(), id, "admin");
  game_server = GameServer::getInstance();
  if (game_server->isListening() == false)
    {
      GameServer::deleteInstance();
      return;
    }
  printf("Game Server is now listening on port %d\n", get_port());
  NextTurnNetworked *next_turn = new NextTurnNetworked();
  game_server->round_ends.connect(sigc::mem_fun(next_turn, &NextTurnNetworked::finishRound));
  game_server->start_player_turn.connect(sigc::mem_fun(next_turn, &NextTurnNetworked::start_player));
  next_turn->srequestAbort.connect(sigc::mem_fun(game_server, &GameServer::on_turn_aborted));
  game_server->get_next_player.connect(sigc::mem_fun(next_turn, &NextTurnNetworked::next));
  game_server->round_ends.connect (method(on_keep_network_play_going));
  Playerlist::getInstance()->splayerDead.connect
    (sigc::mem_fun(GameServer::getInstance(), &GameServer::sendKillPlayer));

  game_server->notifyClientsGameMayBeginNow();
  Configuration::s_autosave_policy = Configuration::NO_SAVING;
  Game *game = new Game(game_scenario, next_turn, true);
  game->game_over.connect (sigc::bind (sigc::mem_fun (this, &Driver::on_game_over_for_headless_server), game_scenario));
  game_server->player_sits.connect(sigc::hide(sigc::hide(method(on_client_sits_down_in_headless_server_game))));
}

void Driver::on_client_sits_down_in_headless_server_game()
{
  static unsigned int count;
  count++;
  if (count == Playerlist::getInstance()->countPlayersAlive())
    GameServer::getInstance()->sendRoundStart();
}

void Driver::get_default(int num_players, GameParameters &g)
{
  GameParameters::Player p;
  if (num_players <= 1 || num_players > (int)MAX_PLAYERS)
    num_players = (int)MAX_PLAYERS;
  for (unsigned int i = 0; i < (unsigned int) num_players; i++)
    {
      p.type = GameParameters::Player::HUMAN;
      p.id = i;
      switch (p.id)
        {
        case 0: 
          p.name = 
            CreateScenarioRandomize::getPlayerName(Shield::WHITE); 
          break;
        case 1: 
          p.name = 
            CreateScenarioRandomize::getPlayerName(Shield::GREEN); 
          break;
        case 2: 
          p.name = 
            CreateScenarioRandomize::getPlayerName(Shield::YELLOW); 
          break;
        case 3: 
          p.name = 
            CreateScenarioRandomize::getPlayerName(Shield::DARK_BLUE); 
          break;
        case 4: 
          p.name = 
            CreateScenarioRandomize::getPlayerName(Shield::ORANGE); 
          break;
        case 5: 
          p.name = 
            CreateScenarioRandomize::getPlayerName(Shield::LIGHT_BLUE); 
          break;
        case 6: 
          p.name = 
            CreateScenarioRandomize::getPlayerName(Shield::RED); 
          break;
        case 7: 
          p.name = 
            CreateScenarioRandomize::getPlayerName(Shield::BLACK); 
          break;
        }
      g.players.push_back(p);
    }
  g.map.width = MAP_SIZE_NORMAL_WIDTH;
  g.map.height = MAP_SIZE_NORMAL_HEIGHT;
  g.map.grass = 78;
  g.map.water = 7;
  g.map.swamp = 2;
  g.map.forest = 3;
  g.map.hills = 5;
  g.map.mountains = 5;
  g.map.cities = 80;
  g.map.ruins = 15;
  g.map.temples = 3;
  g.map.signposts = 20;
  g.map_path = "";
  g.play_with_quests = GameParameters::ONE_QUEST_PER_PLAYER;
  g.hidden_map = false;
  g.neutral_cities = GameParameters::STRONG;
  g.razing_cities = GameParameters::ALWAYS;
  g.diplomacy = false;
  g.random_turns = false;
  g.quick_start = GameParameters::NO_QUICK_START;
  g.intense_combat = false;
  g.military_advisor = false;
  g.army_theme = "default";
  g.tile_theme = "default";
  g.shield_theme = "default";
  g.city_theme = "default";
  g.cities_can_produce_allies = false;
  g.cusp_of_war = false;
  g.see_opponents_stacks = false;
  g.see_opponents_production = false;
  g.vectoring_mode = GameParameters::VECTORING_ALWAYS_TWO_TURNS;
  g.build_production_mode = GameParameters::BUILD_PRODUCTION_ALWAYS;
  g.sacking_mode = GameParameters::SACKING_ALWAYS;
  g.difficulty = GameScenario::calculate_difficulty_rating(g);
}

void Driver::run()
{
  if (Main::instance().start_test_scenario) 
    {
      SmallMap::s_quick = true;
      // quick load a test scenario
      GameParameters g;
      GameParameters::Player p;
      p.type = GameParameters::Player::HUMAN;
      p.id = 0;
      p.name = "Mr. Test";
      g.players.push_back(p);
      p.type = GameParameters::Player::EASY;
      p.id = 1;
      p.name = "Evil";
      g.players.push_back(p);
      for (unsigned int i = 2; i < MAX_PLAYERS; i++)
	{
	  p.type = GameParameters::Player::OFF;
	  p.id = i;
	  p.name = "";
	  g.players.push_back(p);
	}
      //g.map.width = 50;
      //g.map.height = 75;
      //g.map.width = 70;
      //g.map.height = 105;
      //g.map.width = 112;
      //g.map.height = 156;
      g.map.width = 125;
      g.map.height = 125;
      //g.map.width = 255;
      //g.map.height = 255;
      //g.map.width = 500;
      //g.map.height = 500;
      g.map.grass = 78;
      g.map.water = 7;
      g.map.swamp = 2;
      g.map.forest = 3;
      g.map.hills = 5;
      g.map.mountains = 5;
      //g.map.cities = 3;
      g.map.cities = 40;
      //g.map.cities = 80;
      g.map.ruins = 25;
      g.map.temples = 3;
      g.map.signposts = 0;

      g.map_path = "";
      g.tile_theme = "default";
      g.army_theme = "default";
      g.shield_theme = "default";
      g.city_theme = "default";

      g.see_opponents_stacks = true;
      g.see_opponents_production = true;
      g.play_with_quests = GameParameters::NO_QUESTING;
      g.hidden_map = false;
      g.diplomacy = false;

      g.neutral_cities = GameParameters::STRONG;
      g.razing_cities = GameParameters::ALWAYS;
      g.quick_start = GameParameters::NO_QUICK_START;
      g.cusp_of_war = false;
      g.intense_combat = false;
      g.military_advisor = false;
      g.random_turns = false;
      g.cities_can_produce_allies = false;
      g.name = "";

      g.difficulty = GameScenario::calculate_difficulty_rating(g);
      on_new_game_requested(g, NULL);
    }
  else if (Main::instance().start_net_test_scenario)
    {
      // quick load a test scenario
      GameParameters g;
      GameParameters::Player p;
      for (unsigned int i = 0; i < MAX_PLAYERS; i++)
	{
	  p.type = GameParameters::Player::EASY;
	  p.id = i;
	  p.name = String::ucompose ("Player %1", i + 1);
	  g.players.push_back(p);
	}
      g.map.width = 50;
      g.map.height = 75;
      g.map.grass = 78;
      g.map.water = 7;
      g.map.swamp = 2;
      g.map.forest = 3;
      g.map.hills = 5;
      g.map.mountains = 5;
      g.map.cities = 8;
      g.map.ruins = 5;
      g.map.temples = 3;
      g.map.signposts = 0;

      g.map_path = "";
      g.tile_theme = "default";
      g.army_theme = "default";
      g.shield_theme = "default";
      g.city_theme = "default";

      g.see_opponents_stacks = true;
      g.see_opponents_production = true;
      g.play_with_quests = GameParameters::NO_QUESTING;
      g.hidden_map = false;
      g.diplomacy = false;

      g.neutral_cities = GameParameters::STRONG;
      g.razing_cities = GameParameters::ALWAYS;
      g.quick_start = GameParameters::NO_QUICK_START;
      g.cusp_of_war = false;
      g.intense_combat = false;
      g.military_advisor = false;
      g.random_turns = false;
      g.cities_can_produce_allies = false;
      g.name = "";

      g.difficulty = GameScenario::calculate_difficulty_rating(g);
      if (Profilelist::getInstance()->empty() == true)
        Profilelist::getInstance()->push_back(new Profile(Glib::get_user_name()));

      on_new_hosted_network_game_requested(g, get_port(),
                                           Profilelist::getInstance()->front(),
                                           false, false);
    }
  else if (Main::instance().start_robots != 0) 
    {
      Snd::deleteInstance();
      lordsawaromatic("127.0.0.1", get_port(), Player::AI_FAST,
		      Main::instance().start_robots);
    }
  else
    {
      splash_window->show();
      if (d_load_filename.empty() == false)
	{
	  GameParameters g;
	  g.map_path = d_load_filename;
	  size_t found = d_load_filename.find(".map");
	  if (found != Glib::ustring::npos)
	    {
	      GamePreferencesDialog d(*splash_window->get_window(), d_load_filename, GameScenario::HOTSEAT);
	      d.game_started.connect(sigc::bind(sigc::mem_fun
				     (*this, &Driver::on_new_game_requested), &d));
	      d.run();
	    }
	  else
	    {
	      found = d_load_filename.find(SAVE_EXT);
	      if (found != Glib::ustring::npos)
		on_load_requested(d_load_filename);
	      else
		on_new_game_requested(g, NULL);
	    }
	}
    }
  return;
}

Driver::~Driver()
{
  if (game_window)
    delete game_window;
  if (game_lobby_dialog)
    delete game_lobby_dialog;
  if (quick_help_window)
    delete quick_help_window;
  if (splash_window)
    delete splash_window;
  if (download_window)
    delete download_window;
}

void Driver::on_hosted_player_sat_down(Player *player)
{
  guint32 id = player->getId();
  GameServer *game_server = GameServer::getInstance();
  game_server->sit_down(player);
  player_replaced.emit(Playerlist::getInstance()->getPlayer(id));
}

void Driver::on_hosted_player_stood_up(Player *player)
{
  guint32 id = player->getId();
  GameServer *game_server = GameServer::getInstance();
  game_server->stand_up(player);
  player_replaced.emit(Playerlist::getInstance()->getPlayer(id));
}

void Driver::on_hosted_player_changed_type(Player *player, int type)
{
  GameServer *game_server = GameServer::getInstance();
  game_server->type_change(player, type);
}

void Driver::on_hosted_player_says_game_may_begin()
{
  GameServer *game_server = GameServer::getInstance();
  game_server->notifyClientsGameMayBeginNow();
}

void Driver::on_hosted_player_chat(Glib::ustring message)
{
  GameServer *game_server = GameServer::getInstance();
  game_server->chat(message);
}

void Driver::on_client_player_sat_down(Player *player)
{
  GameClient *game_client = GameClient::getInstance();
  game_client->sit_down(player);
}

void Driver::on_client_player_stood_up(Player *player)
{
  GameClient *game_client = GameClient::getInstance();
  game_client->stand_up(player);
}

void Driver::on_client_player_changed_type(Player *player, int type)
{
  GameClient *game_client = GameClient::getInstance();
  game_client->change_type(player, type);
}

void Driver::on_client_player_chat(Glib::ustring message)
{
  GameClient *game_client = GameClient::getInstance();
  game_client->chat(message);
}

GameScenario *Driver::create_new_scenario(GameParameters &g, GameScenario::PlayMode m)
{
  bool update_uuid = false;
  if (g.map_path.empty()) 
    {
      // construct new random scenario if we're not going to load the game
      Glib::ustring path = 
        NewRandomMapDialog::create_and_dump_scenario("random.map", g, NULL);
      g.map_path = path;
      start_game_progress_tick.emit ();
    }
  else
    update_uuid = true;

  bool broken = false;

  GameScenario* game_scenario = new GameScenario(g.map_path, broken);
  if (broken)
    return NULL;

  start_game_progress_tick.emit ();
  GameScenarioOptions::s_see_opponents_stacks = g.see_opponents_stacks;
  GameScenarioOptions::s_see_opponents_production = g.see_opponents_production;
  GameScenarioOptions::s_play_with_quests = g.play_with_quests;
  GameScenarioOptions::s_vectoring_mode = g.vectoring_mode;
  GameScenarioOptions::s_sacking_mode = g.sacking_mode;
  GameScenarioOptions::s_build_production_mode = g.build_production_mode;
  GameScenarioOptions::s_hidden_map = g.hidden_map;
  GameScenarioOptions::s_diplomacy = g.diplomacy;
  GameScenarioOptions::s_cusp_of_war = g.cusp_of_war;
  GameScenarioOptions::s_neutral_cities = g.neutral_cities;
  GameScenarioOptions::s_razing_cities = g.razing_cities;
  GameScenarioOptions::s_intense_combat = g.intense_combat;
  GameScenarioOptions::s_military_advisor = g.military_advisor;
  GameScenarioOptions::s_random_turns = g.random_turns;

  game_scenario->setName(g.name);
  game_scenario->setPlayMode(m);

  if (game_scenario->getRound() == 0)
    {
      if (update_uuid)
	game_scenario->setNewRandomId();
      Playerlist::getInstance()->syncPlayers(g.players);
      game_scenario->initialize(g);
    }
  start_game_progress_tick.emit ();
  return game_scenario;
}

void Driver::advertise_game(GameScenario *game_scenario, Profile *p)
{
  GamelistClient::deleteInstance();
  GamelistClient *gsc = GamelistClient ::getInstance();
  gsc->client_connected.connect
    (sigc::bind(method(on_connected_to_gamelist_server_for_advertising), game_scenario, p));
  gsc->start(Configuration::s_gamelist_server_hostname,
             Configuration::s_gamelist_server_port, p);
}

void Driver::on_connected_to_gamelist_server_for_advertising(GameScenario *game_scenario, Profile *p)
{
  GamelistClient *gsc = GamelistClient::getInstance();
  //okay, fashion the recently played game to go over the wire.
  RecentlyPlayedNetworkedGame *g = 
    new RecentlyPlayedNetworkedGame(game_scenario, p);
  g->setNumberOfPlayers(Playerlist::getInstance()->countPlayersAlive());
  gsc->received_advertising_response.connect
    (sigc::hide(method(on_advertising_response_received)));
  gsc->request_advertising(g);
}

void Driver::on_advertising_response_received(Glib::ustring scenario_id)
{
  d_advertised_scenario_id = scenario_id;
  return;
}

void Driver::remotely_serve (GameScenario *game_scenario, Profile *p)
{
  if ((Configuration::s_gamehost_server_hostname == "" &&
      Configuration::s_gamehost_server_port == 0) || !p || !game_scenario)
    return;
  GamehostClient::deleteInstance();
  GamehostClient *ghc = GamehostClient::getInstance();
  ghc->client_connected.connect(sigc::bind(method(on_connected_to_gamehost_server_for_hosting_request), game_scenario));
  ghc->client_could_not_connect.connect(method(on_could_not_connect_to_gamehost_server));
  ghc->start(Configuration::s_gamehost_server_hostname,
             Configuration::s_gamehost_server_port, p);
}

void Driver::on_could_not_connect_to_gamehost_server()
{
  GamehostClient *ghc = GamehostClient::getInstance();
  if (splash_window)
    {
      splash_window->show();
      TimedMessageDialog dialog
        (*splash_window->get_window(),
         String::ucompose(_("Could not connect to gamehost server:\n%1:%2"),
                          ghc->getHost(), ghc->getPort()), 0);
      dialog.run_and_hide();
    }
}

void Driver::on_connected_to_gamehost_server_for_hosting_request (GameScenario *game_scenario)
{
  GamehostClient *ghc = GamehostClient::getInstance();
  ghc->received_host_response.connect(sigc::bind(sigc::hide<0>(method(on_got_game_host_response)), game_scenario));
  ghc->request_game_host (game_scenario->getId());
}

void Driver::on_got_game_host_response(Glib::ustring err, GameScenario *game_scenario)
{
  if (err != "")
    {
      if (splash_window)
        {
          splash_window->show();
          Glib::ustring s = 
            String::ucompose(_("Gamehost Server Error: %1"), err);
          TimedMessageDialog dialog(*splash_window->get_window(), s, 0);
          dialog.set_title(_("Server Failure"));
          dialog.run_and_hide();
        }
      return;
    }
  GamehostClient *ghc = GamehostClient::getInstance();
  if (download_window)
    delete download_window;
  download_window = new NewNetworkGameDownloadWindow(_("Uploading..."));
  download_window->run ();
  upload_conn = ghc->payload_progress.connect
    (sigc::mem_fun (download_window, &NewNetworkGameDownloadWindow::pulse));
  ghc->received_map_response.connect
    (sigc::bind(sigc::hide<0>(method(on_remote_game_hosted)), upload_conn));
  ghc->send_map(game_scenario);
}

void Driver::on_remote_game_hosted(guint32 port, Glib::ustring err, sigc::connection con)
{
  con.disconnect ();
  if (download_window)
    download_window->hide();
  Profile *profile = 
    Profilelist::getInstance()->findProfileById
    (GamehostClient::getInstance()->getProfileId());
  if (err != "")
    {
      if (splash_window)
        {
          splash_window->show();
          Glib::ustring s = 
            String::ucompose(_("Gamehost Server Error: %1"), err);
          TimedMessageDialog dialog(*splash_window->get_window(), s, 0);
          dialog.set_title(_("Server Failure"));
          dialog.run_and_hide();
        }
    return;
    }
  //yay, we did it.  the new game is waiting for us on the gamehost server 
  //on the port they just sent us.
  //so now we start up a client.
  on_new_remote_network_game_requested
    (Configuration::s_gamehost_server_hostname, port, profile);
}

void Driver::on_new_hosted_network_game_requested(GameParameters g, int port,
						  Profile *p, bool advertised,
                                                  bool remotely_hosted)
{
    if (splash_window)
	splash_window->hide();

    GameScenario *game_scenario = 
      create_new_scenario(g, GameScenario::NETWORKED);

    if (game_scenario == NULL)
      {
	TimedMessageDialog dialog(*splash_window->get_window(),
				  _("Corrupted saved game file."), 0);
	dialog.run_and_hide();
	return;
      }
    on_load_hosted_network_game_requested(game_scenario, port, p, advertised,
                                          remotely_hosted);
}
void Driver::on_game_ended_and_load_network_game(Glib::ustring filename, int port, Profile *p, bool advertised, bool remotely_hosted)
{
  on_game_ended();
  bool broken = false;
  GameScenario* game_scenario = new GameScenario(filename, broken);
  splash_window->hide();
  if (game_window)
    delete game_window;
  game_window = NULL;
  on_load_hosted_network_game_requested(game_scenario, port, p, advertised, remotely_hosted);
}

void Driver::on_load_hosted_network_game_requested(GameScenario *game_scenario,
                                                   int port, Profile *p, 
                                                   bool advertised,
                                                   bool remotely_hosted)
{
  if (remotely_hosted)
    {
      remotely_serve (game_scenario, p);
      return;
    }

  GameServer *game_server = GameServer::getInstance();
  game_server->port_in_use.connect(method(on_could_not_bind_to_port));
  game_server->start(game_scenario, port, p->getId(), p->getNickname());
  game_server = GameServer::getInstance();
  if (game_server->isListening() == false)
    {
      GameServer::deleteInstance();
      return;
    }
  if (advertised)
    advertise_game(game_scenario, p);
  NextTurnNetworked *next_turn = new NextTurnNetworked();
  game_server->round_ends.connect(sigc::mem_fun(next_turn, &NextTurnNetworked::finishRound));
  if (advertised)
    game_server->round_ends.connect (sigc::bind(method(on_advertised_game_round_ends), game_scenario, p));
  game_server->start_player_turn.connect(sigc::mem_fun(next_turn, &NextTurnNetworked::start_player));
  next_turn->srequestAbort.connect(sigc::mem_fun(game_server, &GameServer::on_turn_aborted));
  if (game_lobby_dialog)
    delete game_lobby_dialog;
  game_lobby_dialog = new GameLobbyDialog(*splash_window->get_window(), 
                                          game_scenario, next_turn, 
                                          game_server, true);
  game_server->get_next_player.connect(sigc::mem_fun(next_turn, &NextTurnNetworked::next));
  game_server->round_ends.connect (method(on_keep_network_play_going));
  Playerlist::getInstance()->splayerDead.connect
    (sigc::mem_fun(GameServer::getInstance(), &GameServer::sendKillPlayer));
  game_lobby_dialog->player_sat_down.connect (method(on_hosted_player_sat_down));
  game_lobby_dialog->player_stood_up.connect (method(on_hosted_player_stood_up));
  game_lobby_dialog->player_changed_type.connect
    (method(on_hosted_player_changed_type));
  game_lobby_dialog->message_sent.connect (method(on_hosted_player_chat));
  game_lobby_dialog->game_may_begin.connect
    (method(on_hosted_player_says_game_may_begin));
  game_lobby_dialog->start_network_game.connect
    (method(start_network_game_requested));
  game_lobby_dialog->show();
  bool response = game_lobby_dialog->run();
  game_lobby_dialog->hide();

  if (response == false)
    {
      on_game_ended();
      delete game_scenario;
    }
}

void Driver::on_server_went_away_text()
{
  /* same as on_server_went_away, but we're not doing graphics */
  GameClient::deleteInstance();
  on_quit_requested ();
}

void Driver::on_server_went_away()
{
  download_conn.disconnect ();
  upload_conn.disconnect ();
  heartbeat_conn.disconnect();
  if (game_window)
    {
      //hide() isn't sufficient here because we need to close any open dialogs.
      //they're all set as "destroy with parent" so deleting works.
      delete game_window;
      game_window = NULL;
    }
  if (game_lobby_dialog)
    game_lobby_dialog->hide();
  if (download_window)
    download_window->hide();
  if (splash_window)
    splash_window->show();
  TimedMessageDialog dialog(*splash_window->get_window(), 
			    _("Server went away."), 0);
  dialog.set_title(_("Disconnected"));
  dialog.run_and_hide();
  GameClient::deleteInstance();
}

void Driver::on_client_could_not_connect()
{
  download_conn.disconnect ();
  upload_conn.disconnect ();
  heartbeat_conn.disconnect();
  if (game_lobby_dialog)
    game_lobby_dialog->hide();
  if (download_window)
    download_window->hide();
  if (splash_window)
    splash_window->show();
  GameClient *gc = GameClient::getInstance();
  TimedMessageDialog dialog
    (*splash_window->get_window(), 
     String::ucompose(_("Could not connect to server:\n%1 %2"),
                        gc->getHost(), gc->getPort()), 0);
  dialog.run_and_hide();
  GameClient::deleteInstance();
}

void Driver::on_new_remote_network_game_requested(Glib::ustring host, unsigned short port, Profile *p)
{
  if (splash_window)
    splash_window->hide();
  GameClient *game_client = GameClient::getInstance();
  game_client->game_scenario_received.connect (method(on_game_scenario_downloaded));
  game_client->client_disconnected.connect (method(on_server_went_away));
  game_client->client_forcibly_disconnected.connect (method(on_server_went_away));
  game_client->client_could_not_connect.connect (method(on_client_could_not_connect));
  if (download_window)
    delete download_window;
  download_window = new NewNetworkGameDownloadWindow(_("Downloading..."));
  download_window->run ();
  download_conn = game_client->payload_progress.connect
    (sigc::mem_fun (download_window, &NewNetworkGameDownloadWindow::pulse));
  recv_conn = game_scenario_received.connect
    (sigc::bind(method(on_game_scenario_received), p, download_conn));
  game_client->start(host, port, p->getId(), p->getNickname());
  heartbeat_conn = Glib::signal_timeout().connect (method(heartbeat), 1 * 1000);
}

bool Driver::heartbeat()
{
  if (game_scenario_downloaded == "")
    return true;
  
  game_scenario_received.emit(game_scenario_downloaded);
  return false;
}

void Driver::on_game_scenario_received(Glib::ustring path, Profile *p, sigc::connection con)
{
  con.disconnect ();
  recv_conn.disconnect();
  heartbeat_conn.disconnect();
  if (download_window)
    download_window->hide();
  GameScenario *game_scenario = load_game(path);
  GameClient *game_client = GameClient::getInstance();
  Glib::ustring host = game_client->getHost();
  guint32 port = game_client->getPort();
  RecentlyPlayedGameList::getInstance()->addNetworkedEntry(game_scenario, p, host, port);
  RecentlyPlayedGameList::getInstance()->save();

  NextTurnNetworked *next_turn = new NextTurnNetworked();
  game_client->start_player_turn.connect(sigc::mem_fun(next_turn, &NextTurnNetworked::start_player));
  game_client->round_ends.connect(sigc::mem_fun(next_turn, &NextTurnNetworked::finishRound));
  if (game_lobby_dialog)
    delete game_lobby_dialog;
  game_lobby_dialog = new GameLobbyDialog(*splash_window->get_window(), game_scenario, next_turn, 
					      GameClient::getInstance(), false);
  game_lobby_dialog->player_sat_down.connect (method(on_client_player_sat_down));
  game_lobby_dialog->player_stood_up.connect (method(on_client_player_stood_up));
  game_lobby_dialog->player_changed_type.connect (method(on_client_player_changed_type));
  game_lobby_dialog->message_sent.connect (method(on_client_player_chat));
  game_lobby_dialog->start_network_game.connect (method(start_network_game_requested));
  game_lobby_dialog->show();
  bool response = game_lobby_dialog->run();

  if (response == false)
    {
      GameClient::getInstance()->disconnect();
      on_game_ended();
      delete game_scenario;
    }
}

void Driver::on_game_scenario_downloaded(Glib::ustring path)
{
  game_scenario_downloaded = path;
  //emitting the signal doesn't work.
  //it stops the game client from doing more processing.
  //how can i bring up the game lobby dialog with this scenario?
  //...without stopping the game client from getting more messages
}

void Driver::on_new_game_requested(GameParameters g, GamePreferencesDialog *gpd)
{
  if (gpd)
    {
      start_game_progress_tick.connect (sigc::mem_fun (gpd, &GamePreferencesDialog::tick_progress));
      GameScenario::load_tick.connect
        (sigc::mem_fun (gpd, &GamePreferencesDialog::tick_progress));
      start_game_progress_finish.connect (sigc::mem_fun (gpd, &GamePreferencesDialog::finish_progress));
    }

    start_game_progress_tick.emit ();
    GameScenario *game_scenario = create_new_scenario(g, GameScenario::HOTSEAT);

    if (game_scenario == NULL)
      {
	TimedMessageDialog dialog(*splash_window->get_window(),
				  _("Corrupted saved game file."), 0);
	dialog.run_and_hide();
	splash_window->show();
	return;
      }

    std::list<Glib::ustring> e, w;
    if (g.map_path != "" && game_scenario->validate(e, w) == false)
      {
	TimedMessageDialog dialog
	  (*splash_window->get_window(), 
	   _("Invalid map file.\n" 
	     "Please validate it in the scenario editor."), 0);
	for (std::list<Glib::ustring>::iterator it = e.begin();
             it != e.end(); ++it)
	  {
	    printf ("error: %s\n", (*it).c_str());
	  }
	dialog.run_and_hide();
	splash_window->show();
	return;
      }
    start_game_progress_tick.emit ();
    start_game_progress_finish.emit ();

    NextTurn *next_turn = new NextTurnHotseat();
    init_game_window();
    
    if (gpd)
      gpd->hide ();
    if (splash_window)
      splash_window->hide();
    
    game_window->show();
    game_window->new_game(game_scenario, next_turn);
}

void Driver::on_load_requested(Glib::ustring filename)
{
    if (splash_window)
	splash_window->hide();

    GameScenario *game_scenario = load_game(filename);
    if (game_scenario == NULL)
      {
	splash_window->show();
	return;
      }

    if (game_scenario->getPlayMode() == GameScenario::HOTSEAT)
      {
        init_game_window();

        game_window->show();
        game_window->load_game
          (game_scenario, new NextTurnHotseat());
      }
    else if (game_scenario->getPlayMode() == GameScenario::NETWORKED)
      {
        NewNetworkGameDialog nngd(*splash_window->get_window(), true);
        bool retval = nngd.run();
        nngd.hide();
        if (retval == false)
          {
            delete game_scenario;
            GameScenario::cleanup ();
            splash_window->show();
          }
        else
          {
            on_load_hosted_network_game_requested(game_scenario, get_port(),
						  nngd.getProfile(), 
                                                  nngd.isAdvertised(),
                                                  nngd.isRemotelyHosted());
          }
      }
}

void Driver::on_editor_requested(Glib::ustring filename)
{
  if (splash_window)
    splash_window->hide();
  if (editor_window)
    delete editor_window;

  Snd::getInstance()->halt();
  Snd::getInstance()->disableBackground();
  EditorSplashWindow d;
  d.run();
  d.hide();
  editor_window = new MainWindow (filename);
  editor_window->editor_quit.connect (method(on_editor_quit));
  editor_window->show();
  editor_window->init();
}

void Driver::on_editor_quit ()
{
  if (editor_window)
    delete editor_window;
  editor_window = NULL;
  if (Main::instance().start_editor == false)
    splash_window->show();
  else
    Main::instance().stop_main_loop();
}

void Driver::on_quit_requested()
{
    if (splash_window)
	splash_window->hide();

    if (game_window)
	game_window->hide();

    Snd::deleteInstance();
    Main::instance().stop_main_loop();
}

void Driver::on_game_ended()
{
  if (game_lobby_dialog)
    {
      delete game_lobby_dialog;
      game_lobby_dialog = NULL;
    }
  if (quick_help_window)
    {
      quick_help_window->hide ();
      delete quick_help_window;
      quick_help_window = NULL;
    }

  if (game_window)
    {
      game_window->hide();
      delete game_window;
      game_window = NULL;
    }

  if (GameServer::getInstance()->isListening() &&
      d_advertised_scenario_id != "")
    {
      GameServer *gs = GameServer::getInstance();
      Profile *profile = 
        Profilelist::getInstance()->findProfileById(gs->getProfileId());
      unadvertise_game (d_advertised_scenario_id, profile);
    }

  GameClient::deleteInstance();
  GameServer::deleteInstance();

  ImageCache::deleteInstance();
  GamehostClient::getInstance()->disconnect();
  GamehostClient::deleteInstance();
  GamelistClient::getInstance()->disconnect();
  GamelistClient::deleteInstance();

  Armysetlist::deleteInstance();
  Shieldsetlist::deleteInstance();
  Tilesetlist::deleteInstance();
  Citysetlist::deleteInstance();
  HeroTemplates::deleteInstance();
  ScenarioMedia::deleteInstance();

  if (splash_window)
    splash_window->show();
}

void Driver::on_game_ended_and_start_new()
{
  on_game_ended();
  splash_window->open_new_game_dialog();
}

void Driver::init_game_window()
{
  if (game_window)
    delete game_window;
  game_window = new GameWindow;

  game_window->game_ended.connect (method(on_game_ended));
  game_window->game_ended_start_new.connect (method(on_game_ended_and_start_new));
  game_window->show_lobby.connect (method(on_show_lobby_requested));
  game_window->show_quick_help.connect (method(on_quick_help_requested));
  game_window->quit_requested.connect (method(on_quit_requested));
  game_window->load_hosted_network_game.connect
    (method(on_game_ended_and_load_network_game));

  //make the width+height suitable for the screen size.
  Glib::RefPtr<Gdk::Monitor> monitor =
    Gdk::Display::get_default ()->get_primary_monitor ();

  if (!monitor)
    monitor = Gdk::Display::get_default ()->get_monitor (0);
  
  Gdk::Rectangle monitor_geometry;
  monitor->get_workarea (monitor_geometry);
  guint32 screen_height = monitor_geometry.get_height () * monitor->get_scale_factor ();
  guint32 height = 450;
  if (screen_height <= 600)
    height = 400;
  guint32 width = (int)((float)height * 1.42223);
  game_window->init(width, height);
}

GameScenario *Driver::load_game(Glib::ustring file_path, Gtk::Window *w)
{
    bool broken = false;
    LoadProgressWindow *p = new LoadProgressWindow (w);
    GameScenario::load_tick.connect
      (sigc::mem_fun (p, &LoadProgressWindow::tick_progress));
    GameScenario::load_finish.connect
      (sigc::mem_fun (p, &LoadProgressWindow::finish_progress));
    p->run ();
    GameScenario* game_scenario = new GameScenario(file_path, broken);
    p->hide ();

    if (broken)
      {
	TimedMessageDialog dialog(*splash_window->get_window(),
				  _("Corrupted saved game file."), 0);
	dialog.run_and_hide();
	return NULL;
      }
    return game_scenario;
}

void Driver::stressTestNextRound()
{
  static int count = 1;
  count++;
  printf ("starting round %d!\n", count);
}

void Driver::stress_test(bool gui)
{
  // quick load a test scenario
  GameParameters g;
  GameParameters::Player p;
  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
      p.type = GameParameters::Player::HARD;
      p.id = i;
      switch (p.id)
	{
	case 0: p.name = "one"; break;
	case 1: p.name = "two"; break;
	case 2: p.name = "three"; break;
	case 3: p.name = "four"; break;
	case 4: p.name = "five"; break;
	case 5: p.name = "six"; break;
	case 6: p.name = "seven"; break;
	case 7: p.name = "eight"; break;
	}
      g.players.push_back(p);
    }
  g.map.width = MAP_SIZE_NORMAL_WIDTH;
  g.map.height = MAP_SIZE_NORMAL_HEIGHT;
  g.map.grass = 78;
  g.map.water = 7;
  g.map.swamp = 2;
  g.map.forest = 3;
  g.map.hills = 5;
  g.map.mountains = 5;
  g.map.cities = 20;
  g.map.ruins = 15;
  g.map.temples = 3;
  g.map.signposts = 10;
  g.map_path = "";
  g.play_with_quests = GameParameters::ONE_QUEST_PER_PLAYER;
  g.hidden_map = false;
  g.neutral_cities = GameParameters::STRONG;
  g.razing_cities = GameParameters::ALWAYS;
  g.diplomacy = false;
  g.random_turns = false;
  g.quick_start = GameParameters::NO_QUICK_START;
  g.intense_combat = false;
  g.military_advisor = false;
  g.army_theme = "default";
  g.tile_theme = "default";
  g.shield_theme = "default";
  g.city_theme = "default";
  g.cities_can_produce_allies = false;
  g.cusp_of_war = false;
  g.see_opponents_stacks = true;
  g.see_opponents_production = true;
  g.vectoring_mode = GameParameters::VECTORING_ALWAYS_TWO_TURNS;
  g.build_production_mode = GameParameters::BUILD_PRODUCTION_ALWAYS;
  g.sacking_mode = GameParameters::SACKING_ALWAYS;
  g.difficulty = GameScenario::calculate_difficulty_rating(g);
      
  bool broken = false;
  Glib::ustring path;
  path = NewRandomMapDialog::create_and_dump_scenario("random.map", g, NULL);
  g.map_path = path;

  if (gui)
    {
      Configuration::s_displaySpeedDelay = 0;
      SmallMap::s_quick = true;
      Configuration::s_displayFightRoundDelayFast = 0;
      Configuration::s_displayFightRoundDelaySlow = 0;
      FightWindow::s_quick_all = true;
      on_load_requested(path);
      return;
    }
  GameScenario* game_scenario = new GameScenario(g.map_path, broken);

  if (broken)
    return;

  Configuration::s_autosave_policy = Configuration::NO_SAVING;
  NextTurnHotseat *nextTurn;
  nextTurn = new NextTurnHotseat();
    
  nextTurn->snextRound.connect (method(stressTestNextRound));
  if (game_scenario->getRound() == 0)
    {
      Playerlist::getInstance()->syncPlayers(g.players);
      game_scenario->initialize(g);
    }

  Game game(game_scenario, nextTurn);
  game.get_smallmap().set_slide_speed(0);
  Configuration::s_displaySpeedDelay = 0;
  time_t start = time(NULL);
  nextTurn->start();
  //next turn and game_Scenario get deleted inside game.
  size_t mins = (time(NULL) - start) / 60;
  printf("duration: %lu mins, turns: %d ", mins, game_scenario->getRound());
  printf("winner type: %s\n", Player::playerTypeToString(Player::Type(Playerlist::getInstance()->getFirstLiving()->getType())).c_str());
  Glib::ustring s = 
    File::getTempFile(File::getCacheDir(),
                      String::ucompose ("run-seed-%1", 
                                        Main::instance().random_number_seed)) +
    SAVE_EXT;
  game_scenario->saveGame(s);

  fflush(stdout);

  //nextTurn and game_scenario get deleted in game.
}

void Driver::lordsawaromatic(Glib::ustring host, unsigned short port, Player::Type type, int num_players)
{
  GameClient *game_client = GameClient::getInstance();
  game_client->game_scenario_received.connect(method(on_game_scenario_downloaded));
  game_client->client_disconnected.connect (method(on_server_went_away_text));
  game_client->client_forcibly_disconnected.connect (method(on_server_went_away_text));
  game_client->client_could_not_connect.connect (method(on_client_could_not_connect));
  game_scenario_received.connect (method(on_game_scenario_received_for_robots));
  game_client->setNickname("robot");
  Glib::ustring id = "";
  if (Profilelist::getInstance()->empty() == false)
    id = Profilelist::getInstance()->front()->getId();
  game_client->start(host, port, id, "robot");
  heartbeat_conn = Glib::signal_timeout().connect (method(heartbeat), 1 * 1000);
  robot_player_type = type;
  number_of_robots = num_players;
}

void Driver::on_game_scenario_received_for_robots(Glib::ustring path)
{
  download_conn.disconnect ();
  upload_conn.disconnect ();
  heartbeat_conn.disconnect();
  GameScenario *game_scenario = load_game(path);
  if (!game_scenario)
    return;
  GameClient *game_client = GameClient::getInstance();
  NextTurnNetworked *next_turn = new NextTurnNetworked();
  game_client->start_player_turn.connect(sigc::mem_fun(next_turn, &NextTurnNetworked::start_player));
  game_client->round_ends.connect(sigc::mem_fun(next_turn, &NextTurnNetworked::finishRound));

  Playerlist *pl = Playerlist::getInstance();

  unsigned int count = 0;
  for (Playerlist::iterator it = pl->begin(); it != pl->end(); ++it)
    {
      if ((*it) == Playerlist::getInstance()->getNeutral())
        continue;
      if (count >= number_of_robots && number_of_robots > 0)
	break;
      if ((*it)->getType() == Player::NETWORKED)
        {
          on_client_player_sat_down(*it);
          count++;
        }
    }
  /*
   * this is buggy because we're sitting down as players who may have
   * already sat down.
   *
   * we can't use isConnected() because it's not saved in the save game.
   *
   * the game server doesn't seat the ai players who may already be 
   * remotely sitting.
   * the game lobby server does that.
   *
   * so we say sit, and then the game server disallows that, and we think we
   * sat down.
   *
   */
  if (count == 0)
    {
      printf("nowhere to sit!\n");
      GameClient::deleteInstance();
      delete next_turn;
      delete game_scenario;
      return;
    }

  pl->turnHumansInto(robot_player_type, number_of_robots);

  for (Playerlist::iterator it = pl->begin(); it != pl->end(); ++it)
    if (Player::Type((*it)->getType()) == robot_player_type)
      GameClient::getInstance()->listenForLocalEvents(*it);

  Configuration::s_autosave_policy = Configuration::NO_SAVING;
  Game *game = new Game(game_scenario, next_turn, true);
  game->get_smallmap().set_slide_speed(0);
  game_client->request_seat_manifest();
}

void Driver::on_game_over_for_headless_server (Player *p, GameScenario *g)
{
  printf("all done!  player '%s' was the winner on turn %d\n", p->getName().c_str(), g->getRound());
  GameServer::deleteInstance();
  exit(0);
}

void Driver::on_show_lobby_requested()
{
  if (game_lobby_dialog)
    game_lobby_dialog->show();
}
    
void Driver::on_quick_help_requested()
{
  if (!quick_help_window)
    quick_help_window = new QuickHelpWindow ();
  quick_help_window->show();
}
    
void Driver::start_network_game_requested(GameScenario *game_scenario, NextTurnNetworked *next_turn)
{
  if (game_window)
    {
      Player *active = Playerlist::getActiveplayer();
      if (active->getType() == Player::NETWORKED)
	game_window->show();
      else
	{
	  if (active->hasAlreadyInitializedTurn())
	    game_window->show();
	  else
	    game_window->continue_network_game (next_turn);
	}
    }
  else
    {
      init_game_window();
      player_replaced.connect
	(sigc::mem_fun(game_window, &GameWindow::on_player_replaced));
      game_window->show();

      game_window->new_network_game (game_scenario, next_turn);
      if (GameServer::getInstance()->isListening())
        on_keep_network_play_going();

    }
}

void Driver::on_keep_network_play_going()
{
  while (GameServer::getInstance()->isListening())
    {
      bool round_finished = GameServer::getInstance()->sendRoundStart();
      if (!round_finished)
        {
          //we are waiting for a player to finish their turn
          //or they are unavailable.
          break;
        }
    }
}
    
void Driver::on_could_not_bind_to_port_for_headless_server(int port)
{
  std::cerr << "Could not bind to port " << port << std::endl;
  exit(1);
}

void Driver::on_could_not_bind_to_port (int port)
{
  if (splash_window)
    splash_window->show();
  Glib::ustring s = String::ucompose(_("Could not bind to port %1"), port);
  TimedMessageDialog dialog(*splash_window->get_window(), s, 0);
  dialog.set_title(_("Server Failure"));
  dialog.run_and_hide();
}

void Driver::unadvertise_game(Glib::ustring scenario_id, Profile *p)
{
  GamelistClient *gsc = GamelistClient ::getInstance();
  gsc->client_connected.connect
    (sigc::bind(method(on_connected_to_gamelist_server_for_advertising_removal), scenario_id));
  gsc->start(Configuration::s_gamelist_server_hostname,
             Configuration::s_gamelist_server_port, p);
}

void Driver::on_connected_to_gamelist_server_for_advertising_removal(Glib::ustring scenario_id)
{
  GamelistClient *gsc = GamelistClient::getInstance();
  gsc->received_advertising_removal_response.connect
    (sigc::hide(sigc::hide(method(on_advertising_removal_response_received))));
  gsc->request_advertising_removal(scenario_id);
}

void Driver::on_advertising_removal_response_received()
{
  d_advertised_scenario_id = "";
  return;
}

void Driver::on_advertised_game_round_ends(GameScenario *game_scenario, Profile *p)
{
  advertise_game(game_scenario, p);
}

guint32 Driver::get_port ()
{
  guint32 port = LORDSAWAR_PORT;
  if (Main::instance().port)
    port = Main::instance().port;
  return port;
}
