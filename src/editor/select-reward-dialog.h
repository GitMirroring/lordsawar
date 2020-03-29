//  Copyright (C) 2008, 2009, 2014 Ben Asselstine
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
#ifndef SELECT_REWARD_DIALOG_H
#define SELECT_REWARD_DIALOG_H

#include <gtkmm.h>

#include "lw-editor-dialog.h"
class Reward;

//! Scenario editor.  Select a Reward from the Rewardlist.
class SelectRewardDialog: public LwEditorDialog
{
 public:
    SelectRewardDialog(Gtk::Window &parent);
    ~SelectRewardDialog() {}

    void run();
    const Reward *get_selected_reward() { return selected_reward; }
    
 private:
    Gtk::Button *select_button;

    const Reward *selected_reward;

    Gtk::TreeView *rewards_treeview;
    class RewardsColumns: public Gtk::TreeModelColumnRecord {
    public:
	RewardsColumns() 
        { add(name); add(reward);}
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<Reward *> reward;
    };
    const RewardsColumns rewards_columns;
    Glib::RefPtr<Gtk::ListStore> rewards_list;

    void addReward(Reward *reward);
    
    void set_select_button_state();
};

#endif
