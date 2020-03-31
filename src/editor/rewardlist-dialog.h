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
#ifndef GUI_REWARDLIST_DIALOG_H
#define GUI_REWARDLIST_DIALOG_H

#include <gtkmm.h>

#include "rewardlist.h"
#include "lw-editor-dialog.h"

//! Scenario editor.  Manages Reward objects in the Rewardlist.
class RewardlistDialog: public LwEditorDialog
{
 public:
    RewardlistDialog(Gtk::Window &parent, bool select, bool clear);
    ~RewardlistDialog() {};

    int run ();
    Reward *get_reward () {return d_reward;}

 private:
    bool d_select;
    bool d_clear;
    Reward *d_reward; //current reward
    Gtk::TreeView *rewards_treeview;
    Gtk::Button *add_button;
    Gtk::Button *remove_button;
    Gtk::Button *edit_button;
    Gtk::Button *clear_button;
    Gtk::Button *close_button;

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
    void update_rewardlist_buttons();

    //callbacks
    void on_add_clicked();
    void on_remove_clicked();
    void on_edit_clicked();
    void on_reward_selected();
};

#endif
