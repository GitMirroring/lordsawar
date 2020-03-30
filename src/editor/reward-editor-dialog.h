//  Copyright (C) 2008, 2009, 2014, 2020 Ben Asselstine
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
#ifndef REWARD_EDITOR_DIALOG_H
#define REWARD_EDITOR_DIALOG_H

#include <gtkmm.h>
#include "lw-editor-dialog.h"

class Reward;
class Item;
class ArmyProto;
class Ruin;
class Player;

//! Scenario editor.  Edits rewards.
class RewardEditorDialog: public LwEditorDialog
{
 public:
    RewardEditorDialog(Gtk::Window &parent, Player *player, bool hidden_ruins, 
                       Reward *r);
    ~RewardEditorDialog();

    int run();

    Reward *get_reward() {return reward;}
    
 private:
    Player *d_player;
    Reward *reward;
    Item *item;
    ArmyProto *ally;
    Ruin *hidden_ruin;
    bool d_hidden_ruins;
    Gtk::ComboBox *reward_type_combobox;
    Gtk::Notebook *notebook;
    Gtk::SpinButton *gold_spinbutton;
    Gtk::Button *randomize_gold_button;
    Gtk::Button *item_button;
    Gtk::Button *randomize_item_button;
    Gtk::Button *ally_button;
    Gtk::Button *randomize_allies_button;
    Gtk::SpinButton *num_allies_spinbutton;
    Gtk::SpinButton *map_x_spinbutton;
    Gtk::SpinButton *map_y_spinbutton;
    Gtk::SpinButton *map_width_spinbutton;
    Gtk::SpinButton *map_height_spinbutton;
    Gtk::Button *randomize_map_button;
    Gtk::Button *randomize_hidden_ruin_button;
    Gtk::Button *clear_hidden_ruin_button;
    Gtk::Button *hidden_ruin_button;

    void on_randomize_gold_clicked();
    void on_item_clicked();
    void on_clear_item_clicked();
    void on_randomize_item_clicked();
    void set_item_name();
    void on_ally_clicked();
    void on_randomize_allies_clicked();
    void on_clear_ally_clicked();
    void set_ally_name();
    void on_randomize_map_clicked();
    void on_hidden_ruin_clicked();
    void on_randomize_hidden_ruin_clicked();
    void on_clear_hidden_ruin_clicked();
    void set_hidden_ruin_name();

    void fill_in_reward_info();
    void on_reward_type_changed();

};

#endif
