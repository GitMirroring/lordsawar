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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#pragma once
#ifndef GLS_CLIENT_TOOL_H
#define GLS_CLIENT_TOOL_H

#include "config.h"
#include <memory>
#include <string>
#include <list>
#include <iostream>
#include <glib.h>
#include <gtkmm.h>
#include <sigc++/trackable.h>
#include <sigc++/functors/mem_fun.h>

class Profile;
class RecentlyPlayedGameList;
class RecentlyPlayedGame;

class GlsClientTool
{
public:
    GlsClientTool(Glib::ustring host, int port, Profile *profile, bool show_list, const std::list<Glib::ustring> &unadvertise, bool advertise, bool reload, Glib::ustring remove_all, bool terminate);
    virtual ~GlsClientTool();
private:
  Profile *new_profile;
  Profile *profile;
  bool d_show_list;
  std::list<Glib::ustring> d_unadvertise;
  bool d_advertise;
  bool d_reload;
  Glib::ustring d_remove_all;
  bool d_terminate;
  guint32 request_count;

  //callbacks
  void on_got_list_response(RecentlyPlayedGameList *list, Glib::ustring err);
  void on_got_unadvertise_response(Glib::ustring id, Glib::ustring err);
  void on_got_advertise_response(Glib::ustring id, Glib::ustring err);
  void on_got_reload_response(Glib::ustring err);
  void on_could_not_connect();
  void on_connected();
  void on_connection_lost();
  void on_got_list_response_for_unadvertising(RecentlyPlayedGameList *l, Glib::ustring err);

  //helpers
  void unadvertise_games(std::list<Glib::ustring> scenario_ids);
  RecentlyPlayedGame* create_game();

};
#endif
