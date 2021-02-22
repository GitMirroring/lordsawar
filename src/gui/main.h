//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2010, 2017, 2021 Ben Asselstine
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
#ifndef GUI_MAIN_H
#define GUI_MAIN_H

#include <sigc++/trackable.h>

// initialize the GUI and run the main loop; only one instance is ever
// constructed so Main::instance is a convenience for retrieving it
class Main: public sigc::trackable
{
 public:
    Main();
    ~Main();

    // singleton interface
    static Main &instance();
    
    Glib::RefPtr<Gtk::Application> app ();
    Gtk::Window *win ();
    void initialize ();
    void start_main_loop();
    void stop_main_loop();

    bool start_stress_test;
    bool view_stress_test;
    bool start_editor;
    int start_robots;
    bool start_test_scenario;
    bool start_net_test_scenario;
    bool speedy;
    bool own_all_on_round_two;
    Glib::ustring load_filename;
    Glib::ustring turn_filename;
    guint32 random_number_seed;
    bool start_headless_server;
    guint32 port;
    Glib::Rand rnd;
    int cacheSize;
    std::string configuration_file_path;
    std::string save_path;
    Glib::ustring save_server_messages;
    
 private:
    struct Impl;
    Impl *impl;
    void override_font_size ();
    void kickoff ();
};


#endif
