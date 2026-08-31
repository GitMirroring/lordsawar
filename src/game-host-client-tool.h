//  Copyright (C) 2011, 2015 Ben Asselstine
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
#ifndef GHS_CLIENT_TOOL_H
#define GHS_CLIENT_TOOL_H
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

class GhsClientTool
{
public:
    GhsClientTool(Glib::ustring host, int port, Profile *p, bool show_list, bool reload, Glib::ustring unhost, Glib::ustring file, bool terminate);
    virtual ~GhsClientTool();
private:
  Profile *new_profile;
  Profile *profile;
  Glib::ustring d_host;
  bool d_show_list;
  bool d_reload;
  Glib::ustring d_unhost;
  Glib::ustring d_file_to_host;
  bool d_terminate;
  guint32 request_count;

  //callbacks
  void on_got_list_response(RecentlyPlayedGameList *list, Glib::ustring err);
  void on_got_reload_response(Glib::ustring err);
  void on_got_unhost_response(Glib::ustring id, Glib::ustring err);

  void on_got_host_game_response(Glib::ustring err, Glib::ustring file);
  void on_game_hosted(guint32 port, Glib::ustring err);
  void on_could_not_connect();
  void on_connected();
  void on_connection_lost();
};
#endif
