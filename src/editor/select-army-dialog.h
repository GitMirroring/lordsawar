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
#ifndef SELECT_ARMY_DIALOG_H
#define SELECT_ARMY_DIALOG_H

#include <vector>
#include <gtkmm.h>

#include "gui/army-info-tip.h"
#include "lw-editor-dialog.h"

class ArmyProto;
class City;
class Player;

//! Scenario editor.  Select an Army prototype.
class SelectArmyDialog: public LwEditorDialog
{
 public:
    SelectArmyDialog(Gtk::Window &parent, Player *p, bool hero_too = false,
		     bool defends_ruins = false,
		     bool rewardable = false);
    ~SelectArmyDialog() {}

    void run();

    const ArmyProto *get_selected_army() { return selected_army; }
    
 private:
    ArmyInfoTip* army_info_tip;
    Gtk::Label *army_info_label1;
    Gtk::Label *army_info_label2;
    Gtk::Grid *toggles_table;
    Gtk::Button *select_button;
    std::vector<guint32> armysets;

    const ArmyProto *selected_army;
    Player *player;
    bool d_defends_ruins;
    bool d_awardable;
    bool d_hero_too;

    std::vector<Gtk::ToggleButton *> army_toggles;
    bool ignore_toggles;
    std::vector<const ArmyProto*> selectable;

    void on_army_toggled(Gtk::ToggleButton *toggle);
    bool on_army_button_event(GdkEventButton *e, Gtk::ToggleButton *toggle);
    
    void fill_in_army_toggles();
    void fill_in_army_info();
    void set_select_button_state();
};

#endif
