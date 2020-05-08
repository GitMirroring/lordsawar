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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#include "config.h"
#include <memory>
#include <iostream>
#include <glib.h>
#include <gtkmm.h>
#include <sigc++/trackable.h>
#include <sigc++/functors/mem_fun.h>
#include "gamelist-client.h"
#include "ucompose.hpp"
#include "profilelist.h"
#include "profile.h"
#include "recently-played-game-list.h"
#include "recently-played-game.h"
#include "GameScenario.h"

#include "gls-client-tool.h"


GlsClientTool::GlsClientTool(Glib::ustring host, int port, Profile *p,
                             bool show_list,
                             const std::list<Glib::ustring> &unadvertise,
                             bool advertise, bool reload,
                             Glib::ustring remove_all, bool terminate)
 : new_profile (NULL), profile (p), d_show_list (show_list),
    d_unadvertise (unadvertise), d_advertise (advertise), d_reload (reload),
    d_remove_all (remove_all), d_terminate (terminate), request_count (0)
{
  GamelistClient *gamelistclient = GamelistClient::getInstance();
  Profilelist *plist = Profilelist::getInstance();
  if (!profile)
    {
      if (plist->size() > 0)
        profile = plist->front();
      else
        {
          new_profile = new Profile("admin");
          profile = new_profile;
        } 
    }
  gamelistclient->client_could_not_connect.connect
    (sigc::mem_fun(*this, &GlsClientTool::on_could_not_connect));
  gamelistclient->client_connected.connect
    (sigc::mem_fun(*this, &GlsClientTool::on_connected));
  gamelistclient->client_forcibly_disconnected.connect
    (sigc::mem_fun(*this, &GlsClientTool::on_connection_lost));
  gamelistclient->start(host, port, profile);
}

RecentlyPlayedGame* GlsClientTool::create_game()
{
  char file[256];
  std::cout << _("Map File:") << " ";
  std::cin.getline(file, sizeof (file));
  bool broken = false;
  guint32 player_count = 0, city_count = 0;
  Glib::ustring name, comment, id;
  GameScenario::loadDetails (file, broken, player_count, city_count, name, 
                             comment, id);

  char host[256];
  std::cout << _("Host:") << " ";
  std::cin.getline(host, sizeof (host));
  long port;
  while (1)
    {
      char portstr[256];
      std::cout << _("Port:") << " ";
      std::cin.getline(portstr, sizeof (portstr));
      char* error = 0;
      port = strtol(portstr, &error, 10);
      if (error && (*error != '\0'))
        std::cerr <<_("non-numerical value for port") <<std::endl;
      if (port > 65535 || port < 1000)
        std::cerr <<_("invalid value for port") <<std::endl;
      break;
    }
  return new RecentlyPlayedNetworkedGame(id, profile->getId(), 0, city_count, 
                                         player_count, GameScenario::NETWORKED,
                                         name, host, (guint32) port);
}

GlsClientTool::~GlsClientTool()
{
  GamelistClient::deleteInstance();
  if (new_profile)
    delete new_profile;
}

void GlsClientTool::on_got_list_response(RecentlyPlayedGameList *l, Glib::ustring err)
{
  request_count--;
  if (err != "")
    {
      std::cerr << err << std::endl;
      if (request_count == 0)
        Gtk::Main::quit();
      return;
    }
  Glib::ustring s = String::ucompose(ngettext ("Listing %1 game", 
                                               "Listing %1 games", l->size()),
                                    l->size());
  std::cout << s << std::endl;
  for (RecentlyPlayedGameList::iterator i = l->begin(); i != l->end(); ++i)
    {
      std::cout << std::endl;
      RecentlyPlayedNetworkedGame *g = 
        dynamic_cast<RecentlyPlayedNetworkedGame*>(*i);
      std::cout << _("Id:") << " " << g->getId() << std::endl;
      std::cout << _("Name:") << " " << g->getName() << std::endl;
      std::cout << _("Host:") << " " << g->getHost() << std::endl;
      std::cout << _("Port:") << " " << g->getPort() << std::endl;
      std::cout << _("Profile:") << " " << g->getProfileId() << std::endl;
    }
  delete l;
  if (request_count == 0)
    Gtk::Main::quit();
  return;
}

void GlsClientTool::on_got_list_response_for_unadvertising(RecentlyPlayedGameList *l, Glib::ustring err)
{
  request_count--;
  if (err != "")
    {
      std::cerr << err << std::endl;
      if (request_count == 0)
        Gtk::Main::quit();
      return;
    }
  std::list<Glib::ustring> scenario_ids;
  for (RecentlyPlayedGameList::iterator i = l->begin(); i != l->end(); ++i)
    if ((*i)->getProfileId() == d_remove_all || d_remove_all == "-1") 
      scenario_ids.push_back((*i)->getId());

  if (scenario_ids.empty() == false)
    unadvertise_games(scenario_ids);
  else
    {
      if (request_count == 0)
        Gtk::Main::quit();
    }
  return;
}

void GlsClientTool::on_got_reload_response(Glib::ustring err)
{
  request_count--;
  if (err != "")
    std::cerr << err << std::endl;
  if (request_count == 0)
    Gtk::Main::quit();
}

void GlsClientTool::on_got_unadvertise_response(Glib::ustring id, Glib::ustring err)
{
  request_count--;
  if (err != "")
    {
      Glib::ustring s = 
        String::ucompose(_("Error: Could not remove advertised game %1"), id);
      std::cerr << s << " (" << err << ")" << std::endl;
      if (request_count == 0)
        Gtk::Main::quit();
      return;
    }
  Glib::ustring s = String::ucompose(_("Removed advertised game %1"), id);
  std::cout << s << std::endl;
  if (request_count == 0)
    Gtk::Main::quit();
  return;
}

void GlsClientTool::on_got_advertise_response(Glib::ustring id, Glib::ustring err)
{
  request_count--;
  if (err != "")
    {
      Glib::ustring s = 
        String::ucompose(_("Error: Could not advertise game %1"), id);
      std::cerr << s << " (" << err << ")" << std::endl;
      if (request_count == 0)
        Gtk::Main::quit();
      return;
    }
  Glib::ustring s = String::ucompose(_("Advertised game %1"), id);
  std::cout << s << std::endl;
  if (request_count == 0)
    Gtk::Main::quit();
  return;
}
    
void GlsClientTool::on_could_not_connect()
{
  std::cerr << _("Could not connect to game list server") << std::endl;
  exit(1);
}

void GlsClientTool::on_connection_lost()
{
  std::cerr << _("Server went away unexpectedly") << std::endl;
  exit(1);
}

void GlsClientTool::unadvertise_games(std::list<Glib::ustring> scenario_ids)
{
  GamelistClient *gamelistclient = GamelistClient::getInstance();
  gamelistclient->received_advertising_removal_response.connect
    (sigc::mem_fun(*this, &GlsClientTool::on_got_unadvertise_response));
  for (std::list<Glib::ustring>::iterator i = scenario_ids.begin(); 
       i != scenario_ids.end(); ++i)
    {
      request_count++;
      gamelistclient->request_advertising_removal(*i);
    }
}

void GlsClientTool::on_connected()
{
  GamelistClient *gamelistclient = GamelistClient::getInstance();
  if (d_show_list)
    {
      gamelistclient->received_game_list.connect
        (sigc::mem_fun(*this, &GlsClientTool::on_got_list_response));
      request_count++;
      gamelistclient->request_game_list();
    }

  if (d_unadvertise.empty() == false)
    {
      unadvertise_games(d_unadvertise);
    }
  if (d_advertise)
    {
      RecentlyPlayedGame *g = create_game();
      if (g)
        {
          gamelistclient->received_advertising_response.connect
            (sigc::mem_fun(*this, &GlsClientTool::on_got_advertise_response));
          request_count++;
          gamelistclient->request_advertising(g);
        }
    }
  if (d_reload)
    {
      request_count++;
      gamelistclient->received_reload_response.connect
        (sigc::mem_fun(*this, &GlsClientTool::on_got_reload_response));
      gamelistclient->request_reload();
    }

  if (d_remove_all != "")
    {
      gamelistclient->received_game_list.connect
        (sigc::mem_fun(*this, 
                       &GlsClientTool::on_got_list_response_for_unadvertising));
      request_count++;
      gamelistclient->request_game_list();
    }

  if (d_terminate)
    gamelistclient->request_server_terminate();
}
