// Copyright (C) 2011, 2014, 2015, 2020 Ben Asselstine
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

#include <iostream>
#include <fstream>
#include <sstream>
#include <list>

#include "gamehost-server.h"

#include "network-server.h"
#include "xmlhelper.h"
#include "Configuration.h"
#include "ucompose.hpp"
#include "gamelist.h"
#include "recently-played-game-list.h"
#include "recently-played-game.h"
#include "hosted-game.h"
#include "gamelist-client.h"
#include "advertised-game.h"
#include "GameScenario.h"
#include "profile.h"
#include "File.h"

#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
//#define debug(x)

struct HostGameRequest
{
  Glib::TimeVal created_on;
  Profile *profile;
  Glib::ustring scenario_id;
};


GamehostServer * GamehostServer::s_instance = 0;

GamehostServer* GamehostServer::getInstance()
{
    if (s_instance == 0)
        s_instance = new GamehostServer();

    return s_instance;
}

void GamehostServer::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

GamehostServer::GamehostServer()
{
  Timing::instance().timer_registered.connect
    (sigc::mem_fun(*this, &GamehostServer::on_timer_registered));
  Gamelist::getInstance()->load();
  Gamelist::getInstance()->pruneGames();
  Gamelist::getInstance()->pingGames();
}

GamehostServer::~GamehostServer()
{
  if (network_server.get() != NULL)
    {
      if (network_server->isListening())
        network_server->stop();
    }
}

void GamehostServer::reload()
{
  Gamelist::getInstance()->load();
}

bool GamehostServer::isListening()
{
  if (network_server.get() != NULL)
    return network_server->isListening();
  else
    return false;
}

void GamehostServer::start(int port)
{
  if (network_server.get() != NULL && network_server->isListening())
    return;
  network_server.reset(new NetworkServer());
  network_server->port_in_use.connect
    (sigc::mem_fun(port_in_use, &sigc::signal<void, int>::emit));
  network_server->got_message.connect
    (sigc::mem_fun(this, &GamehostServer::onGotMessage));
  network_server->connection_lost.connect
    (sigc::hide(sigc::mem_fun(this, &GamehostServer::onConnectionLost)));
  network_server->connection_made.connect
    (sigc::hide(sigc::mem_fun(this, &GamehostServer::onConnectionMade)));

  network_server->startListening(port);

}

void GamehostServer::sendList(void *conn)
{
  std::ostringstream os;
  XML_Helper helper(&os);
  RecentlyPlayedGameList *l;
  if (network_server->is_local_connection(conn))
    l = Gamelist::getInstance()->getList(false);
  else
    l = Gamelist::getInstance()->getList(true);

  l->save(&helper);
  network_server->send(conn, GHS_MESSAGE_GAME_LIST, os.str());
  delete l;
}

void GamehostServer::unhost(void *conn, Glib::ustring profile_id, Glib::ustring scenario_id, Glib::ustring &err)
{
  HostedGame *g = Gamelist::getInstance()->findGameByScenarioId(scenario_id);
  if (!g)
    {
      err = _("no such game with that scenario id");
      return;
    }
  if (g->getAdvertisedGame()->getProfileId() != profile_id &&
      network_server->is_local_connection(conn) == false)
    {
      err = _("permission denied");
      return;
    }
  if (kill (g->getPid(), SIGQUIT) != 0)
    {
      err = _("could not kill process");
      return;
    }
  Glib::spawn_close_pid(g->getPid());
  Gamelist::getInstance()->remove(g);

  //now we unadvertise it.
  GamelistClient *gsc = GamelistClient ::getInstance();
  gsc->client_connected.connect
    (sigc::bind(sigc::mem_fun(*this, &GamehostServer::on_connected_to_gamelist_server_for_advertising_removal), g->getAdvertisedGame()->getId()));
  gsc->start(Configuration::s_gamelist_server_hostname,
             Configuration::s_gamelist_server_port, g->getAdvertisedGame()->getProfile());
  delete g;
  return;
}

void GamehostServer::on_connected_to_gamelist_server_for_advertising_removal(Glib::ustring scenario_id)
{
  GamelistClient *gsc = GamelistClient::getInstance();
  gsc->received_advertising_removal_response.connect
    (sigc::hide(sigc::hide(sigc::mem_fun(*this, &GamehostServer::on_advertising_removal_response_received))));
  gsc->request_advertising_removal(scenario_id);
}
    
void GamehostServer::on_advertising_removal_response_received()
{
  GamelistClient::deleteInstance();
  return;
}

void GamehostServer::run_game(GameScenario *game_scenario, Glib::Pid *child_pid, guint32 port, Glib::ustring &err)
{
  Glib::ustring lordsawar = Glib::find_program_in_path(PACKAGE);
  if (lordsawar == "")
    {
      err = _("couldn't find lordsawar binary in path!");
      return;
    }

  Glib::ustring tmpfile = File::get_tmp_file();
  tmpfile += SAVE_EXT;
  game_scenario->saveGame(tmpfile);

  std::list<Glib::ustring> argv;
  argv.push_back(lordsawar);
  argv.push_back(tmpfile);
  argv.push_back("--host");
  argv.push_back("--port");
  argv.push_back(String::ucompose("%1", port));
  
  //run lordsawar <file> --host --port <port>
  Glib::spawn_async (File::getCacheDir (), argv,
                     Glib::SPAWN_STDOUT_TO_DEV_NULL | 
                     Glib::SPAWN_STDERR_TO_DEV_NULL, 
                     sigc::mem_fun(*this, &GamehostServer::on_child_setup), 
                     child_pid);
}

bool GamehostServer::waitForGameToBeConnectable(guint32 port)
{
  Glib::RefPtr<Gio::SocketClient>client = Gio::SocketClient::create();
  while (1)
    {
      Glib::RefPtr<Gio::SocketConnection> sock;
      try
        {
          sock = client->connect_to_host ("127.0.0.1", port);
        }
      catch (Glib::Exception &ex)
        {
          ;
        }
      if (sock)
        {
          sock.reset();
          break;
        }
      Glib::usleep(1000000);
    }
  client.reset();
  return true;
}

HostedGame * GamehostServer::host(GameScenario *game_scenario, Profile *profile, Glib::ustring &err)
{
  guint32 port = get_free_port();
  Glib::Pid child_pid;
  run_game(game_scenario, &child_pid, port, err);
  if (err != "")
    return NULL;

  //now we wait to see if everything is okay.

  bool success = waitForGameToBeConnectable(port);
  if (!success)
    {
      err = _("Game couldn't be setup properly.");
      kill (child_pid, SIGQUIT);
      Glib::spawn_close_pid(child_pid);
      return NULL;
    }

  //now we add an entry to the gamelist.
  HostedGame *g = new HostedGame(new AdvertisedGame(game_scenario, profile));
  g->setPid((guint32) child_pid);
  g->getAdvertisedGame()->fillData(getHostname(), port);
  if (Gamelist::getInstance()->add(g) == false)
    {
      err = _("could not add game to list.");
      kill (g->getPid(), SIGQUIT);
      Glib::spawn_close_pid(g->getPid());
      delete g;
      return NULL;
    }

  //now we advertise it.
  GamelistClient *gsc = GamelistClient ::getInstance();
  gsc->client_connected.connect
    (sigc::bind(sigc::mem_fun(*this, &GamehostServer::on_connected_to_gamelist_server_for_advertising), g));
  gsc->start(Configuration::s_gamelist_server_hostname,
             Configuration::s_gamelist_server_port, profile);
  return g;
}

guint32 GamehostServer::get_free_port()
{
  Glib::RefPtr<Gio::SocketListener> l = Gio::SocketListener::create();
  guint32 port = l->add_any_inet_port();
  l.reset();
  //gosh i hope this gets unbound before we run our program.
  return port;
}

void GamehostServer::on_child_setup()
{
  return;
}

void GamehostServer::on_connected_to_gamelist_server_for_advertising(HostedGame *game)
{
  GamelistClient *gsc = GamelistClient::getInstance();
  //okay, fashion the recently played game to go over the wire.
  RecentlyPlayedNetworkedGame *g = 
    new RecentlyPlayedNetworkedGame(*game->getAdvertisedGame());
  gsc->received_advertising_response.connect
    (sigc::hide(sigc::hide(sigc::mem_fun(*this, &GamehostServer::on_advertising_response_received))));
  gsc->request_advertising(g);
}

void GamehostServer::on_advertising_response_received()
{
  GamelistClient::deleteInstance();
  return;
}

void GamehostServer::get_profile_and_scenario_id(Glib::ustring payload, Profile **profile, Glib::ustring &scenario_id, Glib::ustring &err)
{
  bool broken = false;
  Glib::ustring match = "</" + Profile::d_tag + ">";
  size_t pos = payload.find(match);
  if (pos == Glib::ustring::npos)
    {
      err = _("malformed host new game message");
      return;
    }
  std::istringstream is(payload.substr(0, pos + match.length()));
  //get the profile that wants to host a game
  XML_Helper helper(&is);
  helper.registerTag 
    (Profile::d_tag, sigc::bind(sigc::mem_fun(this, 
                                              &GamehostServer::loadProfile), 
                                profile));
  if (!helper.parseXML())
    broken = true;
  helper.close();
  if (broken)
    {
      err = _("Could not parse profile information.");
      return;
    }

  //now get the scenario id that tags on the end.
  scenario_id = String::utrim(payload.substr(pos + match.length()));
}

bool GamehostServer::loadProfile(Glib::ustring tag, XML_Helper *helper, Profile **profile)
{
  if (tag == Profile::d_tag)
    {
      *profile = new Profile(helper);
      return true;
    }
  return false;
}


bool GamehostServer::onGotMessage(void *conn, int type, Glib::ustring payload)
{
  debug("got message of type " << type);
  switch (GhsMessageType(type)) 
    {
    case GHS_MESSAGE_HOST_NEW_GAME:
        {
          Glib::ustring err = "";
          Profile *profile = NULL; //take it from payload
          Glib::ustring scenario_id;
          get_profile_and_scenario_id(payload, &profile, scenario_id, err);
          if (err != "")
            {
              network_server->send(conn, GHS_MESSAGE_COULD_NOT_HOST_GAME, 
                                   "{???} " + err);
              return true;
            }
          if (is_member(profile->getId()) == false &&
              network_server->is_local_connection(conn) == false)
              {
                delete profile;
                err = _("Not authorized to host on this server.");
                network_server->send(conn, GHS_MESSAGE_COULD_NOT_HOST_GAME, 
                                     scenario_id + " " + err);
                return true;
              }
          if (add_to_profiles_awaiting_maps(profile, scenario_id) == false)
            {
              delete profile;
              err = _("Server too busy.  Try again later.");
              network_server->send(conn, GHS_MESSAGE_COULD_NOT_HOST_GAME, 
                                   scenario_id + " " + err);
              return true;
            }

          network_server->send(conn, GHS_MESSAGE_AWAITING_MAP, scenario_id);

        }
      break;
    case GHS_MESSAGE_SENDING_MAP:
        {
          Glib::ustring err = "";
          Glib::ustring tmpfile = File::get_tmp_file();
          std::ofstream f(tmpfile.c_str());
          f << payload;
          f.close();
          bool broken = false;
          GameScenario *game_scenario = new GameScenario(tmpfile, broken);
          File::erase(tmpfile);
          if (broken)
            {
              err = _("Could not read map file.");
              network_server->send(conn, GHS_MESSAGE_COULD_NOT_READ_MAP, 
                                  "{???} " + err);
              return true;
            }
          game_scenario->setPlayMode(GameScenario::NETWORKED);
          //go get associated profile.
          Profile *profile = 
            remove_from_profiles_awaiting_maps(game_scenario->getId());
          if (!profile)
            {
              err = _("protocol error.");
              network_server->send(conn, GHS_MESSAGE_COULD_NOT_START_GAME, 
                                   game_scenario->getId()+ " " + err);
              return true;
            }

          HostedGame *g = host(game_scenario, profile, err);
          if (err != "")
            network_server->send(conn, GHS_MESSAGE_COULD_NOT_START_GAME, 
                                 game_scenario->getId()+ " " + err);
          else
            network_server->send
              (conn, GHS_MESSAGE_GAME_HOSTED, 
               String::ucompose("%1 %2", game_scenario->getId(), 
                                g->getAdvertisedGame()->getPort()));

          Gamelist::getInstance()->save();
          delete game_scenario;
          delete profile;
        }
      break;
    case GHS_MESSAGE_UNHOST_GAME:
        {
          size_t pos;
          Glib::ustring err;
          pos = payload.find(' ');
          if (pos == Glib::ustring::npos)
            return false;
          unhost(conn, payload.substr(0, pos), payload.substr(pos + 1), err);
          if (err != "")
            network_server->send(conn, GHS_MESSAGE_COULD_NOT_UNHOST_GAME, 
                                 payload.substr(pos + 1) + " " + err);
          else
            network_server->send(conn, GHS_MESSAGE_GAME_UNHOSTED, 
                                 payload.substr(pos + 1));
          Gamelist::getInstance()->save();
        }
      break;
    case GHS_MESSAGE_REQUEST_GAME_LIST:
      sendList(conn);
      break;
    case GHS_MESSAGE_REQUEST_RELOAD:
      if (network_server->is_local_connection(conn))
        {
          Gamelist::getInstance()->load();
          network_server->send(conn, GHS_MESSAGE_RELOADED, "");
        }
      else
          network_server->send(conn, GHS_MESSAGE_COULD_NOT_RELOAD, 
                               _("permission denied"));
      break;
    case GHS_MESSAGE_REQUEST_TERMINATION:
      if (network_server->is_local_connection(conn))
        {
          terminate_request_received.emit();
          Gtk::Main::quit();
        }
      break;
    case GHS_MESSAGE_GAME_LIST:
    case GHS_MESSAGE_COULD_NOT_RELOAD:
    case GHS_MESSAGE_RELOADED:
    case GHS_MESSAGE_AWAITING_MAP:
    case GHS_MESSAGE_GAME_UNHOSTED:
    case GHS_MESSAGE_COULD_NOT_HOST_GAME:
    case GHS_MESSAGE_COULD_NOT_UNHOST_GAME:
    case GHS_MESSAGE_COULD_NOT_GET_GAME_LIST:
    case GHS_MESSAGE_GAME_HOSTED:
    case GHS_MESSAGE_COULD_NOT_READ_MAP:
    case GHS_MESSAGE_COULD_NOT_START_GAME:
      break;
      //faulty client
      break;
    }
  return true;
}

void GamehostServer::onConnectionMade()
{
  debug("connection made");
  Gamelist::getInstance()->pruneGames();
  Gamelist::getInstance()->pingGames();
  cleanup_old_profiles_awaiting_maps();
}

void GamehostServer::onConnectionLost()
{
  debug("connection lost");
}

sigc::connection GamehostServer::on_timer_registered(Timing::timer_slot s,
                                                     int msecs_interval)
{
    return Glib::signal_timeout().connect(s, msecs_interval);
}
          
bool GamehostServer::add_to_profiles_awaiting_maps(Profile *profile, Glib::ustring scenario_id)
{
  if (host_game_requests.size() > (guint32) TOO_MANY_PROFILES_AWAITING_MAPS &&
      TOO_MANY_PROFILES_AWAITING_MAPS != -1)
    return false;
  HostGameRequest* request = new HostGameRequest();
  request->profile = profile;
  request->scenario_id = scenario_id;
  Glib::TimeVal now;
  now.assign_current_time();
  request->created_on = now;
  host_game_requests.push_back(request);
  return true;
}

void GamehostServer::cleanup_old_profiles_awaiting_maps(int stale)
{
  Glib::TimeVal now;
  now.assign_current_time();
  for (std::list<HostGameRequest*>::iterator i = host_game_requests.begin();
       i != host_game_requests.end(); ++i)
    {
      if ((*i)->created_on.as_double() + stale < now.as_double())
        {
          delete (*i)->profile;
          delete (*i);
          i = host_game_requests.erase(i);
        }
    }
}

Profile *GamehostServer::remove_from_profiles_awaiting_maps(Glib::ustring scenario_id)
{
  for (std::list<HostGameRequest*>::iterator i = host_game_requests.begin();
       i != host_game_requests.end(); ++i)
    {
      if ((*i)->scenario_id == scenario_id)
        {
          Profile *profile = (*i)->profile;
          delete (*i);
          host_game_requests.erase(i);
          return profile;
        }
    }
  return NULL;
}

bool GamehostServer::is_member(Glib::ustring profile_id)
{
  if (members.empty())
    return true;
  Glib::ustring id = String::utrim(profile_id);
  for (std::list<Glib::ustring>::iterator i = members.begin(); i != members.end();
       ++i)
    {
      if (id == *i)
        return true;
    }
  return false;
}

std::list<Glib::ustring> GamehostServer::load_members_from_file(Glib::ustring file)
{
  char buffer[1024];
  std::list<Glib::ustring> members;
  std::ifstream f(file.c_str());
  if (f.is_open() == false)
    return members;
  while (!f.eof())
    {
      f.getline(buffer, sizeof(buffer));
      Glib::ustring line = buffer;
      size_t pos = line.find('#');
      Glib::ustring trimmed_line;
      if (pos == Glib::ustring::npos)
        trimmed_line =String::utrim(line);
      else
        trimmed_line = String::utrim(line.substr(pos));
      if (trimmed_line != "")
        members.push_back(trimmed_line);
    }
  f.close();
  return members;
}
// End of file
