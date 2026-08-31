//  Copyright (C) 2011, 2015, 2026 Ben Asselstine
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
#ifndef GAMEHOST_CLIENT_TOOL_H
#define GAMEHOST_CLIENT_TOOL_H
#include "config.h"
#include <memory>
#include <string>
#include <list>
#include <iostream>
#include <glib.h>
#include <gtkmm.h>
#include <sigc++/trackable.h>
#include <sigc++/functors/mem_fun.h>

#include "game-host-client-tool.h"
#include "game-host-client.h"
#include "ucompose.hpp"
#include "profile-list.h"
#include "profile.h"
#include "recently-played-game-list.h"
#include "recently-played-game.h"
#include "game-scenario.h"
#include "lw.h"

class GhsClientTool
{
public:
    GhsClientTool (Glib::ustring host, int port, Profile *p, bool show_list,
                   bool reload, Glib::ustring unhost, Glib::ustring file,
                   bool terminate)
      : m_new_profile (NULL), m_profile (p), m_host (host),
      m_show_list (show_list), m_reload (reload), m_unhost (unhost),
      m_file_to_host (file), m_terminate (terminate)
  {
    GamehostClient *gamehostclient = GamehostClient::instance ();
    Profilelist *plist = Profilelist::instance ();
    if (!m_profile)
      {
        if (plist->size () > 0)
          m_profile = plist->front ();
        else
          {
            m_new_profile = new Profile ("admin");
            m_profile = m_new_profile;
          } 
      }
    gamehostclient->client_could_not_connect.connect
      (sigc::mem_fun (*this, &GhsClientTool::on_could_not_connect));
    gamehostclient->client_connected.connect
      (sigc::mem_fun (*this, &GhsClientTool::on_connected));
    gamehostclient->client_forcibly_disconnected.connect
      (sigc::mem_fun (*this, &GhsClientTool::on_connection_lost));
    gamehostclient->start (host, port, m_profile);
  }

    ~GhsClientTool ()
      {
        GamehostClient::deleteInstance ();
        if (m_new_profile)
          delete m_new_profile;
      }
private:
    Profile *m_new_profile;
    Profile *m_profile;
    Glib::ustring m_host;
    bool m_show_list;
    bool m_reload;
    Glib::ustring m_unhost;
    Glib::ustring m_file_to_host;
    bool m_terminate;
    guint32 m_request_count;

    //callbacks
    void on_got_list_response (RecentlyPlayedGameList *l, Glib::ustring err)
      {
        m_request_count--;
        if (err != "")
          {
            std::cerr <<
              String::ucompose ("%1: %2", Lw::get_prgname (), err) << std::endl;
            if (m_request_count == 0)
              Lw::quit_loop ();
            return;
          }
        Glib::ustring s = String::ucompose (ngettext ("Listing %1 game", 
                                                      "Listing %1 games",
                                                      l->size ()),
                                            l->size ());
        std::cout << s << std::endl;
        for (auto i = l->begin (); i != l->end (); ++i)
          {
            std::cout << std::endl;
            RecentlyPlayedNetworkedGame *g = 
              dynamic_cast<RecentlyPlayedNetworkedGame*>(*i);
            std::cout << _("Id:") << " " << g->getId () << std::endl;
            std::cout << _("Name:") << " " << g->getName () << std::endl;
            std::cout << _("Host:") << " " << g->getHost () << std::endl;
            std::cout << _("Port:") << " " << g->getPort () << std::endl;
            std::cout << _("Profile:") << " " << g->getProfileId () << std::endl;
          }
        delete l;
        if (m_request_count == 0)
          Lw::quit_loop ();
        return;
      }

    void on_got_reload_response (Glib::ustring err)
      {
        m_request_count--;
        if (err != "")
          std::cerr << String::ucompose ("%1: %2",
                                         Lw::get_prgname (), err) << std::endl;
        if (m_request_count == 0)
          Lw::quit_loop ();
      }

    void on_got_unhost_response (Glib::ustring id, Glib::ustring err)
      {
        m_request_count--;
        if (err != "")
          std::cerr << String::ucompose ("%1: %2",
                                         Lw::get_prgname (), err) << std::endl;
        else
          std::cerr << 
            String::ucompose (_("%1: Stopped hosting game %2"),
                              Lw::get_prgname (), id) << std::endl;
        if (m_request_count == 0)
          Lw::quit_loop ();
      }

    void on_got_host_game_response (Glib::ustring err, Glib::ustring file)
      {
        m_request_count--;
        if (err != "")
          {
            std::cerr <<
              String::ucompose ("%1: %2", Lw::get_prgname (), err) << std::endl;
            if (m_request_count == 0)
              Lw::quit_loop ();
            return;
          }
        GamehostClient *ghc = GamehostClient::instance ();
        ghc->received_map_response.connect
          (sigc::hide<0> (sigc::mem_fun (*this, &GhsClientTool::on_game_hosted)));
        ghc->send_map_file (file);
      }

    void on_game_hosted (guint32 port, Glib::ustring err)
      {
        m_request_count--;
        if (err != "")
          {
            std::cerr <<
              String::ucompose ("%1: %2", Lw::get_prgname (), err) << std::endl;
            if (m_request_count == 0)
              Lw::quit_loop ();
            return;
          }

        std::cerr <<
          String::ucompose (_("%1: The game is hosted at %2, port %3"),
                            Lw::get_prgname (), m_host, port) << std::endl;
        if (m_request_count == 0)
          Lw::quit_loop ();
      }

    void on_could_not_connect ()
      {
        std::cerr <<
          String::ucompose (_("%1: Could not connect to game list server"),
                              Lw::get_prgname ()) << std::endl;
        exit (1);
      }

    void on_connected ()
      {
        GamehostClient *gamehostclient = GamehostClient::instance ();
        if (m_show_list)
          {
            gamehostclient->received_game_list.connect
              (sigc::mem_fun (*this, &GhsClientTool::on_got_list_response));
            m_request_count++;
            gamehostclient->request_game_list ();
          }

        if (m_reload)
          {
            m_request_count++;
            gamehostclient->received_reload_response.connect
              (sigc::mem_fun (*this, &GhsClientTool::on_got_reload_response));
            gamehostclient->request_reload ();
          }

        if (m_unhost.empty () == false)
          {
            m_request_count++;
            gamehostclient->received_unhost_response.connect
              (sigc::mem_fun (*this, &GhsClientTool::on_got_unhost_response));
            gamehostclient->request_game_unhost (m_unhost);
          }

        if (m_file_to_host.empty () == false)
          {
            m_request_count++;
            gamehostclient->received_host_response.connect
              (sigc::bind
               (sigc::hide<0>
                (sigc::mem_fun
                 (*this, &GhsClientTool::on_got_host_game_response)),
                m_file_to_host));
            bool broken = false;
            Glib::ustring n, com, id;
            guint32 p, c;
            GameScenario::loadDetails (m_file_to_host, broken, p, c, n, com, id);
            if (broken == false)
              gamehostclient->request_game_host (id);
            else
              {
                m_request_count--;
                std::cerr <<
                  String::ucompose ("%1: couldn't load %2",
                                    Lw::get_prgname (), m_file_to_host) <<
                  std::endl;
              }
          }
        if (m_terminate)
          gamehostclient->request_server_terminate ();
      }

    void on_connection_lost ()
      {
        std::cerr <<
          String::ucompose (_("%1: Server went away unexpectedly"),
                            Lw::get_prgname ()) << std::endl;
        exit (1);
      }
};
#endif
