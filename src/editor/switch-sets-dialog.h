//  Copyright (C) 2009, 2014, 2020 Ben Asselstine
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
#ifndef SWITCH_SETS_DIALOG_H
#define SWITCH_SETS_DIALOG_H

#include <gtkmm.h>

#include "Tile.h"
#include "game-parameters.h"
#include "lw-editor-dialog.h"

class Player;
class Tileset;
class Armyset;
class Cityset;
class Shieldset;

//! Switch Sets.  Change the army/tile/city/shieldsets of the map.
class SwitchSetsDialog: public LwEditorDialog
{
 public:
    SwitchSetsDialog(Gtk::Window &parent);
    ~SwitchSetsDialog() {}

    int run();

    bool get_tileset_changed() const {return tileset_changed;}

    bool get_set_changed () const {return armyset_changed || tileset_changed ||
      cityset_changed || shieldset_changed;}
    
 private:
    Gtk::ComboBoxText *tile_size_combobox;
    Gtk::ComboBoxText *tile_theme_combobox;
    Gtk::ComboBoxText *city_theme_combobox;
    Gtk::ComboBoxText *shield_theme_combobox;
    Gtk::Grid *armysets_grid;
    Gtk::Button *accept_button;
    Gtk::Button *make_same_button;
    std::vector<Gtk::ComboBoxText*> army_theme_comboboxes;

    guint32 get_active_tile_size();
    void on_tile_size_changed();
    Tileset* selected_tileset;
    Shieldset* selected_shieldset;
    Cityset* selected_cityset;
    std::list<sigc::connection> connections;
    bool armyset_changed;
    bool tileset_changed;
    bool cityset_changed;
    bool shieldset_changed;

    void switchArmyset(Armyset *armyset);
    void on_armyset_changed (Gtk::ComboBoxText *c, Player *p);
    void on_shieldset_changed ();
    void on_cityset_changed ();
    void on_tileset_changed ();
    void on_make_same_activated ();

    void fill_tile_themes (bool &empty);
    void fill_city_themes (bool &empty);
    void fill_shield_themes (bool &empty);
    void fill_army_themes (bool &empty_armysets);

    void connect_signals ();
    void disconnect_signals ();

};

#endif
