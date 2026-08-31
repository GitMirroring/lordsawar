//
//  Copyright (C) 2011 Ben Asselstine
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
#ifndef GAMELIST_CLIENT_TOOL_H
#define GAMELIST_CLIENT_TOOL_H

#include "config.h"
#include <memory>
#include <string>
#include <list>
#include <iostream>
#include <glib.h>
#include <gtkmm.h>
#include <sigc++/trackable.h>
#include <sigc++/functors/mem_fun.h>

#include "game-list-client.h"
#include "ucompose.hpp"
#include "profile-list.h"
#include "profile.h"
#include "recently-played-game-list.h"
#include "recently-played-game.h"
#include "game-scenario.h"
#include "lw.h"

#include "game-list-client-tool.h"

class GlsClientTool
{
public:
    GlsClientTool (Glib::ustring host, int port, Profile *p, bool show_list,
                   const std::list<Glib::ustring> &unadvertise, bool advertise,
                   bool reload, Glib::ustring remove_all, bool terminate)
      : m_new_profile (NULL), m_profile (p), m_show_list (show_list),
      m_unadvertise (unadvertise), m_advertise (advertise), m_reload (reload),
      m_remove_all (remove_all), m_terminate (terminate), m_request_count (0)
  {
    GamelistClient *gamelistclient = GamelistClient::instance ();
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
    gamelistclient->client_could_not_connect.connect
      (sigc::mem_fun (*this, &GlsClientTool::on_could_not_connect));
    gamelistclient->client_connected.connect
      (sigc::mem_fun (*this, &GlsClientTool::on_connected));
    gamelistclient->client_forcibly_disconnected.connect
      (sigc::mem_fun (*this, &GlsClientTool::on_connection_lost));
    gamelistclient->start (host, port, m_profile);
  }

    ~GlsClientTool ()
      {
        GamelistClient::deleteInstance ();
        if (m_new_profile)
          delete m_new_profile;
      }

private:
    Profile *m_new_profile;
    Profile *m_profile;
    bool m_show_list;
    std::list<Glib::ustring> m_unadvertise;
    bool m_advertise;
    bool m_reload;
    Glib::ustring m_remove_all;
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

    void on_got_unadvertise_response (Glib::ustring id, Glib::ustring err)
      {
        m_request_count--;
        if (err != "")
          {
            Glib::ustring s = 
              String::ucompose (_("%1: Could not remove advertised game %2"),
                                Lw::get_prgname (), id);
            std::cerr << s << " (" << err << ")" << std::endl;
            if (m_request_count == 0)
              Lw::quit_loop ();
            return;
          }

        Glib::ustring s = String::ucompose (_("Removed advertised game %1"), id);
        std::cout << s << std::endl;
        if (m_request_count == 0)
          Lw::quit_loop ();
        return;
      }

    void on_got_advertise_response (Glib::ustring id, Glib::ustring err)
      {
        m_request_count--;
        if (err != "")
          {
            Glib::ustring s = 
              String::ucompose (_("%1: Could not advertise game %2"),
                                Lw::get_prgname (), id);
            std::cerr << s << " (" << err << ")" << std::endl;
            if (m_request_count == 0)
              Lw::quit_loop ();
            return;
          }
        Glib::ustring s = String::ucompose (_("Advertised game %1"), id);
        std::cout << s << std::endl;
        if (m_request_count == 0)
          Lw::quit_loop ();
        return;
      }

    void on_got_reload_response (Glib::ustring err)
      {
        m_request_count--;
        if (err != "")
          std::cerr <<
            String::ucompose ("%1: %2", Lw::get_prgname (), err) << std::endl;
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
        GamelistClient *gamelistclient = GamelistClient::instance ();
        if (m_show_list)
          {
            gamelistclient->received_game_list.connect
              (sigc::mem_fun (*this, &GlsClientTool::on_got_list_response));
            m_request_count++;
            gamelistclient->request_game_list ();
          }

        if (m_unadvertise.empty () == false)
          {
            unadvertise_games (m_unadvertise);
          }
        if (m_advertise)
          {
            RecentlyPlayedGame *g = create_game ();
            if (g)
              {
                gamelistclient->received_advertising_response.connect
                  (sigc::mem_fun (*this,
                                  &GlsClientTool::on_got_advertise_response));
                m_request_count++;
                gamelistclient->request_advertising (g);
              }
          }
        if (m_reload)
          {
            m_request_count++;
            gamelistclient->received_reload_response.connect
              (sigc::mem_fun (*this, &GlsClientTool::on_got_reload_response));
            gamelistclient->request_reload ();
          }

        if (m_remove_all != "")
          {
            gamelistclient->received_game_list.connect
              (sigc::mem_fun (*this, 
                              &GlsClientTool::on_got_list_response_for_unadvertising));
            m_request_count++;
            gamelistclient->request_game_list ();
          }

        if (m_terminate)
          gamelistclient->request_server_terminate ();
      }

    void on_connection_lost ()
      {
        std::cerr <<
          String::ucompose (_("%1: Server went away unexpectedly"),
                            Lw::get_prgname ()) << std::endl;
        exit (1);
      }

    void on_got_list_response_for_unadvertising (RecentlyPlayedGameList *l,
                                                 Glib::ustring err)
      {
        m_request_count--;
        if (err != "")
          {
            std::cerr << String::ucompose ("%1: %2",
                                           Lw::get_prgname (), err) <<
              std::endl;
            if (m_request_count == 0)
              Lw::quit_loop ();
            return;
          }
        std::list<Glib::ustring> scenario_ids;
        for (auto i = l->begin (); i != l->end (); ++i)
          if ((*i)->getProfileId () == m_remove_all || m_remove_all == "-1") 
            scenario_ids.push_back ((*i)->getId ());

        if (scenario_ids.empty () == false)
          unadvertise_games (scenario_ids);
        else
          {
            if (m_request_count == 0)
              Lw::quit_loop ();
          }
        return;
      }

    //helpers
    void unadvertise_games (std::list<Glib::ustring> scenario_ids)
      {
        GamelistClient *gamelistclient = GamelistClient::instance ();
        gamelistclient->received_advertising_removal_response.connect
          (sigc::mem_fun (*this, &GlsClientTool::on_got_unadvertise_response));
        for (auto i = scenario_ids.begin (); i != scenario_ids.end (); ++i)
          {
            m_request_count++;
            gamelistclient->request_advertising_removal (*i);
          }
      }
    RecentlyPlayedGame* create_game ()
      {
        char file[256];
        std::cout << _("Map File:") << " ";
        std::cin.getline (file, sizeof (file));
        bool broken = false;
        guint32 player_count = 0, city_count = 0;
        Glib::ustring name, comment, id;
        GameScenario::loadDetails (file, broken, player_count, city_count, name, 
                                   comment, id);

        char host[256];
        std::cout << _("Host:") << " ";
        std::cin.getline (host, sizeof (host));
        long port;
        while (1)
          {
            char portstr[256];
            std::cout << _("Port:") << " ";
            std::cin.getline (portstr, sizeof (portstr));
            char* error = 0;
            port = strtol (portstr, &error, 10);
            if (error && (*error != '\0'))
              std::cerr <<
                String::ucompose (_("%1: non-numerical value for port"),
                                  Lw::get_prgname ()) <<std::endl;
            if (port > 65535 || port < 1000)
              std::cerr <<
                String::ucompose (_("%1: invalid value for port"),
                                  Lw::get_prgname ()) <<std::endl;
            break;
          }
        return new
          RecentlyPlayedNetworkedGame (id, m_profile->getId (), 0, city_count, 
                                       player_count, GameScenario::NETWORKED,
                                       name, host, (guint32) port);
      }
};
#endif
