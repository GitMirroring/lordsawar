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
#ifndef MAP_INFO_DIALOG_H
#define MAP_INFO_DIALOG_H

#include <gtkmm.h>
#include "lw-editor-dialog.h"

class GameScenario;

//! Scenario editor.  Edits the description of the scenario.
class MapInfoDialog: public LwEditorDialog
{
 public:
    MapInfoDialog(Gtk::Window &parent, GameScenario *game_scenario);
    ~MapInfoDialog();

    bool run();
    
 private:
    bool d_changed;
    Gtk::Entry *name_entry;
    Gtk::TextView *description_textview;
    Gtk::TextView *copyright_textview;
    Gtk::TextView *license_textview;
    Gtk::Notebook *notebook;
    GameScenario *game_scenario;
    Gtk::Label *cities_label;
    Gtk::Label *ruins_label;
    Gtk::Label *temples_label;
    Gtk::Label *signposts_label;
    Gtk::Label *stones_label;
    Gtk::Label *ports_label;
    Gtk::Label *roads_label;
    Gtk::Label *bridges_label;
    Gtk::Label *stacks_label;
    Gtk::Label *armies_label;
    Gtk::Label *keepers_label;
    Gtk::Label *items_label;
    Gtk::Label *rewards_label;
    Gtk::Label *bags_label;

    void on_name_changed();
    void on_copyright_changed ();
    void on_license_changed ();
    void on_description_changed ();
};

#endif
