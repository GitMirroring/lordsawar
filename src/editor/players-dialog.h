//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014, 2020 Ben Asselstine
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
#ifndef PLAYERS_DIALOG_H
#define PLAYERS_DIALOG_H

#include <vector>
#include <gtkmm.h>
#include "lw-editor-dialog.h"
#include "game-parameters.h"

class Player;
class CreateScenarioRandomize;

//! Scenario editor.  Edit Player objects in the scenario.
class PlayersDialog: public LwEditorDialog
{
 public:
    PlayersDialog(Gtk::Window &parent, CreateScenarioRandomize *randomizer);
    ~PlayersDialog() {}

    bool run();

 private:
    bool d_changed;
    Gtk::Grid *players_grid;

    Gtk::Button *randomize_gold_button;

    typedef std::vector<Glib::ustring> player_name_seq;
    player_name_seq default_player_names;

    void add_player(int row, Glib::ustring name, int gold, Player *player);
    void on_randomize_gold_pressed();
    CreateScenarioRandomize *d_random;
    std::vector<Gtk::ComboBoxText*> player_type_comboboxes;
    std::vector<Gtk::Entry*> player_name_entries;
    std::vector<Gtk::SpinButton*> player_gold_spinbuttons;
    std::vector<Gtk::Button*> player_heroes_buttons;

    void update_player (int row);

    Gtk::ComboBoxText* add_combo_for_player_type (int row, Player *p);
    Gtk::Entry* add_entry_for_player_name(int row, Glib::ustring name);
    Gtk::SpinButton* add_spinbutton_for_player_gold(int row, int gold);
    Gtk::Button* add_button_for_player_heroes(int row);

    void on_player_type_changed (int row);
    void on_player_name_changed (int row);
    void on_player_gold_changed (int row);
    void on_player_gold_edited (const Glib::ustring &text, int *p, int row);
    void on_player_heroes_clicked (int row);
    void sensitize_row (int i);
    GameParameters::Player to_player (int row);
};

#endif
