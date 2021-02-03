//  Copyright (C) 2008, 2009, 2014, 2020, 2021 Ben Asselstine
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
#include "undo-mgr.h"
#include "reward.h"

class Reward;
class Item;
class ArmyProto;
class Ruin;
class Player;
class ArmyChooserButton;

//! Scenario editor.  Edits rewards.
class RewardEditorDialog: public LwEditorDialog
{
 public:
    RewardEditorDialog(Gtk::Window &parent, Player *player, bool hidden_ruins, 
                       Reward *r);
    ~RewardEditorDialog();

    bool run();

    Reward *get_reward() {return d_reward;}
    
 private:
    bool d_changed;
    UndoMgr *umgr;
    Player *d_player;
    Reward *d_reward;
    Reward_Gold *d_gold_reward;
    Reward_Allies *d_allies_reward;
    Reward_Item *d_item_reward;
    Reward_Map *d_map_reward;
    Reward_Ruin *d_ruin_reward;
    bool d_hidden_ruins;
    Gtk::ComboBox *reward_type_combobox;
    Gtk::Notebook *notebook;
    Gtk::SpinButton *gold_spinbutton;
    Gtk::Button *randomize_gold_button;
    Gtk::Button *item_button;
    Gtk::Button *randomize_item_button;
    ArmyChooserButton *ally_button;
    Gtk::Button *randomize_allies_button;
    Gtk::SpinButton *num_allies_spinbutton;
    Gtk::SpinButton *map_x_spinbutton;
    Gtk::SpinButton *map_y_spinbutton;
    Gtk::SpinButton *map_width_spinbutton;
    Gtk::SpinButton *map_height_spinbutton;
    Gtk::Button *randomize_map_button;
    Gtk::Button *randomize_hidden_ruin_button;
    Gtk::Button *hidden_ruin_button;
    Gtk::Button *undo_button;
    Gtk::Button *redo_button;

    void on_randomize_gold_clicked();
    void on_item_clicked();
    void on_clear_item_clicked();
    void on_randomize_item_clicked();
    void update_item_name();
    void on_ally_selected(const ArmyProto *a);
    void on_randomize_allies_clicked();
    void on_randomize_map_clicked();
    void on_hidden_ruin_clicked();
    void on_randomize_hidden_ruin_clicked();
    void on_clear_hidden_ruin_clicked();
    void update_hidden_ruin_name();
    void on_map_x_text_changed();
    void on_map_y_text_changed();
    void on_map_width_text_changed();
    void on_map_height_text_changed();
    void on_num_allies_text_changed();
    void on_gold_text_changed ();
    void setup_rewards (Reward *r);
    bool switch_reward_type (Reward::Type t);

    void fill_in_reward_info();
    void on_type_changed();
    void on_undo_activated ();
    void on_redo_activated ();
    void connect_signals ();
    void disconnect_signals ();
    void update ();
    std::list<sigc::connection> connections;
    UndoAction* executeAction (UndoAction *action);
    Reward::Type row_to_reward_type (int row);

};

#endif
