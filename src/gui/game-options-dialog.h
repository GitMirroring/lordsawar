//  Copyright (C) 2007, 2008, 2009, 2014, 2017 Ben Asselstine
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
#ifndef GAME_OPTIONS_DIALOG_H
#define GAME_OPTIONS_DIALOG_H

#include <gtkmm.h>
#include <sigc++/signal.h>

#include "game-parameters.h"
#include "lw-dialog.h"

// dialog for setting game options before the game starts
class GameOptionsDialog: public LwDialog
{
 public:
    GameOptionsDialog(Gtk::Window &parent, bool readonly);
    ~GameOptionsDialog();

    bool run();
    void hide() {dialog->hide();};
    
    sigc::signal<void> difficulty_option_changed;

 private:
    Gtk::Grid *difficultoptionstable;
    Gtk::Grid *notdifficultoptionstable;
    Gtk::ComboBox *quests_combobox;
    Gtk::Switch *view_enemies_switch;
    Gtk::Switch *view_production_switch;
    Gtk::Switch *hidden_map_switch;
    Gtk::ComboBox *neutral_cities_combobox;
    Gtk::ComboBox *razing_cities_combobox;
    Gtk::Switch *diplomacy_switch;
    Gtk::Switch *intense_combat_switch;
    Gtk::Switch *military_advisor_switch;
    Gtk::Switch *random_turns_switch;
    Gtk::ComboBox *quick_start_combobox;
    Gtk::Switch *cusp_of_war_switch;
    Gtk::ComboBox *vectoring_combobox;
    Gtk::ComboBox *build_production_combobox;
    Gtk::ComboBox *sack_combobox;
    Gtk::Notebook *notebook;
    void fill_in_options();
    void on_view_enemies_switch_clicked();
    void on_view_production_switch_clicked();
    void on_quests_combobox_changed();
    void on_hidden_map_switch_clicked();
    void on_neutral_cities_combobox_changed();
    void on_razing_cities_combobox_changed();
    void on_diplomacy_switch_clicked();
    void on_cusp_of_war_switch_clicked();
    void on_random_turns_switch_clicked();
    void on_quick_start_combobox_changed();
    void on_vectoring_combobox_changed();
    void on_build_production_combobox_changed();
    void on_sacking_combobox_changed();
    void on_intense_combat_switch_clicked();
    void on_military_advisor_switch_clicked();
    bool d_readonly;
};

#endif
