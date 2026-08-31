//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2011, 2014, 2015, 2016, 2017, 2020,
//  2021, 2026 Ben Asselstine
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

#ifndef STARTUP_TOOLS_H
#define STARTUP_TOOLS_H
#include "lw.h"

#include "startup-tools.h"

class StartupTools
{
  friend class Startup;
public:

  StartupTools ()
    {
    }

  ~StartupTools ()
    {
    }

  int game_host_client (Profile *profile, Glib::ustring host,
                        Glib::ustring file, Glib::ustring unhost,
                        bool show_list, bool reload, bool terminate,
                        int port);

  int game_list_client (Profile *profile, Glib::ustring host, bool advertise,
                        bool show_list, bool reload, bool terminate,
                        int port, std::list<Glib::ustring> unadvertise,
                        Glib::ustring remove_all);

  int game_list_server (int port, bool foreground);

  int game_host_server (std::string hostname, int port, bool foreground,
                        std::list<Glib::ustring> members);

  int upgrade_tool (bool identify_file, std::string rewrite,
                    std::string filename);

  int import_tool (std::string armyset_file, std::string filename);

private:

  int upgrade_file (Glib::ustring filename, Glib::ustring rewrite,
                    bool identify_file);
};
#endif
