//  Copyright (C) 2011, 2014, 2015, 2026 Ben Asselstine
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
#ifndef GAMELIST_SERVER_H
#define GAMELIST_SERVER_H

#include "config.h"

#include <memory>
#include <list>
#include <glibmm.h>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>

#include "network-gls-common.h"

#include "network-server.h"
#include "xml-helper.h"
#include "configuration.h"
#include "ucompose.hpp"
#include "game-list.h"
#include "recently-played-game-list.h"
#include "recently-played-game.h"
#include "hosted-game.h"
#include "advertised-game.h"
#include "file.h"
#include "lw.h"

class NetworkServer;
class XML_Helper;

class GamelistServer
{
public:
        
  //! Returns the singleton instance.  Creates a new one if neccessary.
  static GamelistServer * instance ()
    {
      if (s_instance == 0)
        s_instance = new GamelistServer ();

      return s_instance;
    }

  //! Deletes the singleton instance.
  static void deleteInstance ()
    {
      if (s_instance)
        delete s_instance;

      s_instance = 0;
    }

  bool isRunning ()
    {
      if (m_network_server.get () != NULL)
        return m_network_server->isRunning ();
      else
        return false;
    }

  void start (int port)
    {
      if (m_network_server.get () != NULL && m_network_server->isRunning ())
        return;
      m_network_server.reset (new NetworkServer ());
      m_network_server->port_in_use.connect
        (sigc::mem_fun (m_port_in_use, &sigc::signal<void(int)>::emit));
      m_network_server->got_message.connect
        (sigc::mem_fun (*this, &GamelistServer::onGotMessage));
      m_network_server->connection_lost.connect
        (sigc::hide (sigc::mem_fun (*this, &GamelistServer::onConnectionLost)));
      m_network_server->connection_made.connect
        (sigc::hide (sigc::mem_fun (*this, &GamelistServer::onConnectionMade)));

      m_network_server->startListening (port);

    }

  void reload ()
    {
      Gamelist::instance ()->loadFromFile (m_datafile);
    }

  sigc::signal<void(int)> signal_port_in_use ()
    {
      return m_port_in_use;
    }

  sigc::signal<void()> signal_terminate_request_received ()
    {
      return m_terminate_request_received;
    }

protected:
  GamelistServer ()
  : m_datafile (File::getUserRecentlyAdvertisedGamesDescription ())
    {
      Gamelist::instance ()->loadFromFile (m_datafile);
      Gamelist::instance ()->pruneGames ();
      Gamelist::instance ()->pingGames ();
    }

  ~GamelistServer ()
    {
      if (m_network_server.get () != NULL)
        {
          if (m_network_server->isRunning ())
            m_network_server->stop ();
        }
    }

private:
  std::unique_ptr<NetworkServer> m_network_server;
  Glib::ustring m_datafile;
  sigc::signal<void(int)> m_port_in_use;
  sigc::signal<void()> m_terminate_request_received;

  bool onGotMessage (void *conn, int type, Glib::ustring payload)
    {
      switch (GlsMessageType (type)) 
        {
        case GLS_MESSAGE_ADVERTISE_GAME:
            {
              std::istringstream is (payload);
              XML_Helper helper (&is);
              helper.register_tag
                (AdvertisedGame::d_tag_name, 
                 sigc::bind
                 (sigc::mem_fun (*this,
                                 &GamelistServer::loadAdvertisedGame), conn));
              helper.parse_XML ();
              Gamelist::instance ()->saveToFile (m_datafile);
            }
          break;

        case GLS_MESSAGE_UNADVERTISE_GAME:
            {
              size_t pos;
              Glib::ustring err;
              pos = payload.find (' ');
              if (pos == Glib::ustring::npos)
                return false;
              unadvertise (conn, payload.substr (0, pos),
                           payload.substr (pos + 1), err);
              if (err != "")
                m_network_server->send (conn,
                                        GLS_MESSAGE_COULD_NOT_UNADVERTISE_GAME,
                                        payload.substr (pos + 1) + " " + err);
              else
                m_network_server->send (conn, GLS_MESSAGE_GAME_UNADVERTISED, 
                                        payload.substr (pos + 1));
              Gamelist::instance ()->saveToFile (m_datafile);
            }
          break;

        case GLS_MESSAGE_REQUEST_GAME_LIST:
          sendList (conn);
          break;

        case GLS_MESSAGE_REQUEST_RELOAD:
          if (m_network_server->is_local_connection (conn))
            {
              Gamelist::instance ()->loadFromFile (m_datafile);
              m_network_server->send (conn, GLS_MESSAGE_RELOADED, "");
            }
          else
            m_network_server->send (conn, GLS_MESSAGE_COULD_NOT_RELOAD, 
                                    _("permission denied"));
          break;
        case GLS_MESSAGE_REQUEST_TERMINATION:
          if (m_network_server->is_local_connection (conn))
            {
              m_terminate_request_received.emit ();
              Lw::quit_loop ();
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

  void onConnectionLost ()
    {
    }

  void onConnectionMade ()
    {
      Gamelist::instance ()->pruneGames ();
      Gamelist::instance ()->pingGames ();
    }

  void unadvertise (void *conn, Glib::ustring profile_id,
                    Glib::ustring scenario_id, Glib::ustring &err)
    {
      HostedGame *g = Gamelist::instance ()->findGameByScenarioId (scenario_id);
      if (!g)
        {
          err = _("no such game with that scenario id");
          return;
        }
      if (g->getAdvertisedGame ()->getProfileId () != profile_id &&
          m_network_server->is_local_connection (conn) == false)
        {
          err = _("permission denied");
          return;
        }
      Gamelist::instance ()->remove (g);
      delete g;
      return;
    }

  void sendList (void *conn)
    {
      std::ostringstream os;
      XML_Helper helper (&os);
      RecentlyPlayedGameList *l;
      if (m_network_server->is_local_connection (conn))
        l = Gamelist::instance ()->getList (false);
      else
        l = Gamelist::instance ()->getList (true);

      l->save (&helper);
      m_network_server->send (conn, GLS_MESSAGE_GAME_LIST, os.str ());
      delete l;
    }

  bool loadAdvertisedGame (Glib::ustring tag, XML_Helper *helper, void *conn)
    {
      if (tag == AdvertisedGame::d_tag_name)
        {
          AdvertisedGame *a = new AdvertisedGame (helper);
          Glib::ustring host = m_network_server->get_hostname (conn);
          a->setHost (host); //find our ip.
          HostedGame *h =
            Gamelist::instance ()->findGameByScenarioId (a->getId ());
          if (h)
            {
              //replace?
              if (a->getProfileId () !=
                  h->getAdvertisedGame ()->getProfileId () &&
                  m_network_server->is_local_connection (conn) == false)
                {
                  m_network_server->send (conn,
                                          GLS_MESSAGE_COULD_NOT_ADVERTISE_GAME,
                                          a->getId () + " " +
                                          _("permission denied"));
                  return true;
                }
              std::replace (Gamelist::instance ()->begin (), 
                           Gamelist::instance ()->end (), h, new HostedGame (a));

              m_network_server->send (conn, GLS_MESSAGE_GAME_ADVERTISED,
                                      a->getId ()); //no error
              return true;
            }
          else
            {
              bool success = Gamelist::instance ()->add (new HostedGame (a));
              if (!success)
                m_network_server->send (conn,
                                        GLS_MESSAGE_COULD_NOT_ADVERTISE_GAME,
                                        a->getId () + " " + 
                                        _("could not advertise game"));
              else
                m_network_server->send (conn, GLS_MESSAGE_GAME_ADVERTISED,
                                        a->getId ()); //no error

              return true;
            }
        }

      return false;
    }

  //! A static pointer for the singleton instance.
  inline static GamelistServer* s_instance = NULL;
};

#endif
