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

#ifndef STARTUP_H
#define STARTUP_H
#include "lw.h"
#include "game-parameters.h"
#include "game-scenario.h"

class SplashWindow;
class GameWindow;
class GameLobbyDialog;
class NetworkGameDownloadWindow;
#include "startup-editor.h"
#include "startup-tools.h"

class Startup: public StartupEditor, public StartupTools
{
public:

    static Startup* instance ();

    sigc::signal<void()> signal_game_window_coming_up () 
      {
        return m_signal_game_window_coming_up; 
      }

    Startup ()
      {
      }

    ~Startup ()
      {
      }

    void test_scenario ();

    void net_test_scenario ()
      {
      }

    void stress_test (bool view_stress_test, GameParameters::Player::Type typ, bool viewfast);

    void server (std::string load_filename);


    void load_map (std::string load_filename);

    void load_saved_game (std::string load_filename);

    void splash ();

    void initialize ();
private:
    static Startup* s_instance;
    SplashWindow *m_splash_window = NULL;
    GameWindow *m_game_window = NULL;
    GameLobbyDialog *m_game_lobby_dialog = NULL;
    NetworkGameDownloadWindow *m_download_window = NULL;
    bool m_initialized = false;

    sigc::signal<void()> m_signal_game_window_coming_up;


    void run_stress_test (GameScenario *game_scenario, std::string path);

    void setup_game_window (bool show);

    void setup_splash_window ();

    void hotseat_game (GameScenario *g);

    void setup_scenario (std::string map_filename, GameScenario::PlayMode mode,
                         Gtk::Window *parent);

    void setup_hotseat_game (Gtk::ApplicationWindow *parent);

    void server (Gtk::ApplicationWindow *parent, Profile *profile);

    void client (Gtk::ApplicationWindow *parent, Glib::ustring host,
                 unsigned short port, Profile *p);

    void serve (GameScenario *game_scenario, GameParameters g, int port, Profile *p, bool modfirst);

    void hide_download_window ();

    void start_game_lobby_for_client (Gtk::ApplicationWindow *parent, GameScenario *game_scenario);
};
#endif
