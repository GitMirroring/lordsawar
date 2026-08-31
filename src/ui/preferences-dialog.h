//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014 Ben Asselstine
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
#ifndef PREFERENCES_DIALOG_H
#define PREFERENCES_DIALOG_H

#include <map>
#include <sigc++/signal.h>
#include <gtkmm.h>

#include "vector.h"
#include "lw-dialog.h"

class Game;
class Player;
// dialog for showing sound and game preferences
class PreferencesDialog: public LwDialog
{
 public:
    PreferencesDialog(Gtk::Window &parent, bool readonly);
    ~PreferencesDialog() {};

    void run(Game *game);
    void hide() {dialog->hide();};

 private:
    Gtk::Switch *commentator_switch;
    Gtk::Scale *speed_scale;
    Gtk::Switch *play_music_switch;
    Gtk::Scale *music_volume_scale;
    Gtk::Box *players_vbox;
    Gtk::Button *game_options_button;

    bool d_readonly;
    void on_show_commentator_toggled();
    void on_play_music_toggled();
    void on_music_volume_changed();
    void on_speed_changed();
    void on_observe_toggled(Gtk::CheckButton *button);
    void on_type_changed(Gtk::ComboBoxText *combo);
    void on_game_options_clicked();

    typedef std::map<Player*, Gtk::ComboBoxText*> PlayerTypeMap;
    PlayerTypeMap player_types;

    typedef std::map<Player*, Gtk::CheckButton*> PlayerObserveMap;
    PlayerObserveMap player_observed;

    typedef std::map<Player*, Gtk::Label*> PlayerNameMap;
    PlayerNameMap player_name;
};

#endif
