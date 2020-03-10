//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2011, 2014, 2015, 2016 Ben Asselstine
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
#ifndef GUI_SPLASH_WINDOW_H
#define GUI_SPLASH_WINDOW_H

#include <memory>
#include <gtkmm.h>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>

#include "game-parameters.h"
#include "PixMask.h"

class Profile;
class GamePreferencesDialog;
/** The opening window of the game
  * 
  * This is the first window to pop up, where the user selects whether to start
  * a new game, load an old one...
  *
  * The splash screen is also responsible for launching dialogs in response to
  * the menu choices.
  */
class SplashWindow: public sigc::trackable
{
 public:
    SplashWindow();
    ~SplashWindow();
	
    void show() {window->show_all();};
    void hide() {window->hide();};
    void open_new_game_dialog();
        
    Gtk::Window *get_window() {return window;}

    sigc::signal<void, Glib::ustring, unsigned short, Profile*> new_remote_network_game_requested;
    sigc::signal<void, GameParameters, int, Profile*, bool, bool > new_hosted_network_game_requested;
    sigc::signal<void, GameParameters, GamePreferencesDialog*> new_game_requested;
    sigc::signal<void, Glib::ustring> load_requested;
    sigc::signal<void> quit_requested;
    sigc::signal<void, Glib::ustring> editor_requested;

 private:
    Gtk::Window* window;
    Gtk::Button *crash_button;
    Gtk::Button *load_game_button;
    Gtk::Button *load_scenario_button;
    Gtk::Button *quit_button;
    Gtk::Button *new_network_game_button;
    Gtk::Button *preferences_button;
    Gtk::Button *editor_button;
    Gtk::Box *button_box; //crash button box
    Gtk::Box *main_box; //crash button box
    Gtk::EventBox *eventbox;
	    
    bool on_delete_event();
  
    Glib::ustring network_game_nickname;
    
    void on_new_network_game_clicked();
    void on_load_game_clicked();
    void on_load_scenario_clicked();
    void on_preferences_clicked();
    void on_editor_clicked();
    void on_quit_clicked();
    void on_rescue_crashed_game_clicked();
	
    void on_game_started(GameParameters g, GamePreferencesDialog *gpd);
    void on_network_game_created(GameParameters g, Profile *profile, bool advertised, bool remotely_hosted);
    void on_network_game_selected(Glib::ustring ip, unsigned short port, Profile  *profile);
    bool on_draw(const ::Cairo::RefPtr< ::Cairo::Context >& cr);
  
    PixMask *bg;
};

#endif
