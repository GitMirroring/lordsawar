// Copyright (C) 2011, 2014, 2015 Ben Asselstine
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

#include "gamelist-server.h"

#include "network-server.h"
#include "xmlhelper.h"
#include "Configuration.h"
#include "ucompose.hpp"
#include "gamelist.h"
#include "recently-played-game-list.h"
#include "recently-played-game.h"
#include "hosted-game.h"
#include "advertised-game.h"
#include "File.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

GamelistServer * GamelistServer::s_instance = 0;

GamelistServer* GamelistServer::getInstance()
{
    if (s_instance == 0)
        s_instance = new GamelistServer();

    return s_instance;
}

void GamelistServer::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

GamelistServer::GamelistServer()
  : datafile (File::getSaveFile(RECENTLY_ADVERTISED_LIST))
{
  Timing::instance().timer_registered.connect
    (sigc::mem_fun(*this, &GamelistServer::on_timer_registered));
  Gamelist::getInstance()->loadFromFile(datafile);
  Gamelist::getInstance()->pruneGames();
  Gamelist::getInstance()->pingGames();
}

GamelistServer::~GamelistServer()
{
  if (network_server.get() != NULL)
    {
      if (network_server->isListening())
        network_server->stop();
    }
}

void GamelistServer::reload()
{
  Gamelist::getInstance()->loadFromFile(datafile);
}

bool GamelistServer::isListening()
{
  if (network_server.get() != NULL)
    return network_server->isListening();
  else
    return false;
}

void GamelistServer::start(int port)
{
  if (network_server.get() != NULL && network_server->isListening())
    return;
  network_server.reset(new NetworkServer());
  network_server->port_in_use.connect
    (sigc::mem_fun(port_in_use, &sigc::signal<void, int>::emit));
  network_server->got_message.connect
    (sigc::mem_fun(this, &GamelistServer::onGotMessage));
  network_server->connection_lost.connect
    (sigc::hide(sigc::mem_fun(this, &GamelistServer::onConnectionLost)));
  network_server->connection_made.connect
    (sigc::hide(sigc::mem_fun(this, &GamelistServer::onConnectionMade)));

  network_server->startListening(port);

}

void GamelistServer::sendList(void *conn)
{
  std::ostringstream os;
  XML_Helper helper(&os);
  RecentlyPlayedGameList *l;
  if (network_server->is_local_connection(conn))
    l = Gamelist::getInstance()->getList(false);
  else
    l = Gamelist::getInstance()->getList(true);

  l->save(&helper);
  network_server->send(conn, GLS_MESSAGE_GAME_LIST, os.str());
  delete l;
}

void GamelistServer::unadvertise(void *conn, Glib::ustring profile_id, Glib::ustring scenario_id, Glib::ustring &err)
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
  Gamelist::getInstance()->remove(g);
  delete g;
  return;
}

bool GamelistServer::onGotMessage(void *conn, int type, Glib::ustring payload)
{
  debug("got message of type " << type);
  switch (GlsMessageType(type)) 
    {
    case GLS_MESSAGE_ADVERTISE_GAME:
        {
          std::istringstream is(payload);
          XML_Helper helper(&is);
          helper.registerTag
            (AdvertisedGame::d_tag_name, 
             sigc::bind(sigc::mem_fun(this, 
                                      &GamelistServer::loadAdvertisedGame), 
                        conn));
          helper.parseXML();
          Gamelist::getInstance()->saveToFile(datafile);
        }
      break;
    case GLS_MESSAGE_UNADVERTISE_GAME:
        {
          size_t pos;
          Glib::ustring err;
          pos = payload.find(' ');
          if (pos == Glib::ustring::npos)
            return false;
          unadvertise(conn, payload.substr(0, pos), payload.substr(pos + 1),
                      err);
          if (err != "")
            network_server->send(conn, GLS_MESSAGE_COULD_NOT_UNADVERTISE_GAME, 
                                 payload.substr(pos + 1) + " " + err);
          else
            network_server->send(conn, GLS_MESSAGE_GAME_UNADVERTISED, 
                                 payload.substr(pos + 1));
          Gamelist::getInstance()->saveToFile(datafile);
        }
      break;
    case GLS_MESSAGE_REQUEST_GAME_LIST:
      sendList(conn);
      break;
    case GLS_MESSAGE_REQUEST_RELOAD:
      if (network_server->is_local_connection(conn))
        {
          Gamelist::getInstance()->loadFromFile(datafile);
          network_server->send(conn, GLS_MESSAGE_RELOADED, "");
        }
      else
          network_server->send(conn, GLS_MESSAGE_COULD_NOT_RELOAD, 
                               _("permission denied"));
      break;
    case GLS_MESSAGE_REQUEST_TERMINATION:
      if (network_server->is_local_connection(conn))
        {
          terminate_request_received.emit();
          Gtk::Main::quit();
        }
      break;
    case GLS_MESSAGE_GAME_LIST:
    case GLS_MESSAGE_COULD_NOT_ADVERTISE_GAME:
    case GLS_MESSAGE_COULD_NOT_UNADVERTISE_GAME:
    case GLS_MESSAGE_GAME_ADVERTISED:
    case GLS_MESSAGE_GAME_UNADVERTISED:
    case GLS_MESSAGE_COULD_NOT_GET_GAME_LIST:
    case GLS_MESSAGE_COULD_NOT_RELOAD:
    case GLS_MESSAGE_RELOADED:
      //faulty client
      break;
    }
  return true;
}

void GamelistServer::onConnectionMade()
{
  debug("connection made");
  Gamelist::getInstance()->pruneGames();
  Gamelist::getInstance()->pingGames();
}

void GamelistServer::onConnectionLost()
{
  debug("connection lost");
}

sigc::connection GamelistServer::on_timer_registered(Timing::timer_slot s,
                                                     int msecs_interval)
{
    return Glib::signal_timeout().connect(s, msecs_interval);
}
             
bool GamelistServer::loadAdvertisedGame(Glib::ustring tag, XML_Helper *helper, void *conn)
{
  if (tag == AdvertisedGame::d_tag_name)
    {
      AdvertisedGame *a = new AdvertisedGame(helper);
      Glib::ustring host = network_server->get_hostname(conn);
      a->setHost(host); //find our ip.
      HostedGame *h = Gamelist::getInstance()->findGameByScenarioId(a->getId());
      if (h)
        {
          //replace?
          if (a->getProfileId() != h->getAdvertisedGame()->getProfileId() &&
              network_server->is_local_connection(conn) == false)
            {
              network_server->send(conn, GLS_MESSAGE_COULD_NOT_ADVERTISE_GAME,
                                   a->getId() + " " + _("permission denied"));
              return true;
            }
          std::replace(Gamelist::getInstance()->begin(), 
                       Gamelist::getInstance()->end(), h, new HostedGame(a));
              
          network_server->send(conn, GLS_MESSAGE_GAME_ADVERTISED,
                               a->getId()); //no error
          return true;
        }
      else
        {
          bool success = Gamelist::getInstance()->add(new HostedGame(a));
          if (!success)
              network_server->send(conn, GLS_MESSAGE_COULD_NOT_ADVERTISE_GAME,
                                   a->getId() + " " + 
                                   _("could not advertise game"));
          else
            network_server->send(conn, GLS_MESSAGE_GAME_ADVERTISED,
                                 a->getId()); //no error

          return true;
        }
    }
  return false;
}
// End of file
