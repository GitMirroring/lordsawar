//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014, 2015, 2017, 2020, 2021 Ben Asselstine
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
#ifndef NEW_MAP_DIALOG_H
#define NEW_MAP_DIALOG_H

#include <vector>
#include <gtkmm.h>
#include "lw-editor-dialog.h"

#include "Tile.h"
#include "game-parameters.h"
#include "undo-mgr.h"
#include "new-map-actions.h"

//! Scenario editor.  Edit parameters to make a new map.
class NewMapDialog: public LwEditorDialog
{
 public:
    NewMapDialog(Gtk::Window &parent);
    ~NewMapDialog();

    void run();

    Map map;
    
    bool map_set;
    
    void setup_progress_bar ();
    void tick_progress (double p);

 private:
    UndoMgr *umgr;
    Gtk::Box *dialog_vbox;
    Gtk::ComboBox *map_size_combobox;
    Gtk::SpinButton *width_spinbutton;
    Gtk::SpinButton *height_spinbutton;
    Gtk::ComboBoxText *fill_style_combobox;
    Gtk::Widget *random_map_container;
    Gtk::ComboBoxText *tile_size_combobox;
    Gtk::ComboBoxText *tile_theme_combobox;
    Gtk::ComboBoxText *city_theme_combobox;
    Gtk::ComboBoxText *army_theme_combobox;
    Gtk::ComboBoxText *shield_theme_combobox;
    Gtk::Scale *grass_scale;
    Gtk::Scale *water_scale;
    Gtk::Scale *swamp_scale;
    Gtk::Scale *forest_scale;
    Gtk::Scale *hills_scale;
    Gtk::Scale *mountains_scale;
    Gtk::Scale *cities_scale;
    Gtk::Scale *ruins_scale;
    Gtk::Scale *temples_scale;
    Gtk::Scale *signposts_scale;
    Gtk::Scale *stones_scale;
    Gtk::Button *accept_button;
    Gtk::Switch *random_roads_switch;
    Gtk::Switch *random_names_switch;
    Gtk::SpinButton *num_players_spinbutton;
    Gtk::SpinButton *stone_road_spinbutton;
    Gtk::Notebook *notebook;
    Gtk::Button *undo_button;
    Gtk::Button *redo_button;

    enum { MAP_SIZE_NORMAL = 0, MAP_SIZE_SMALL, MAP_SIZE_TINY, 
      MAP_SIZE_CUSTOM };

    void on_fill_style_changed();
    void on_map_size_changed();
    void on_random_roads_toggled ();
    void on_random_names_toggled ();
    void update_button ();
    void add_fill_style(Tile::Type tile_type);

    guint32 get_active_tile_size();
    void on_tile_size_changed();
    std::vector<int> fill_style;

    Gtk::TreeView *progress_treeview;

    class ProgressModelColumns : public Gtk::TreeModel::ColumnRecord
      {
    public:
        ProgressModelColumns ()
          { add (perc);}
        Gtk::TreeModelColumn<int> perc;
      };
    ProgressModelColumns progress_columns;
    Glib::RefPtr<Gtk::ListStore> progress_liststore;
    Gtk::TreeModel::Row progress_row;
    UndoAction * executeAction (UndoAction*action);
    void on_undo_activated ();
    void on_redo_activated ();
    void connect_signals ();
    void disconnect_signals ();
    std::list<sigc::connection> connections;
    void on_grass_changed ();
    void on_water_changed ();
    void on_swamp_changed ();
    void on_forest_changed ();
    void on_hills_changed ();
    void on_mountains_changed ();
    void on_cities_changed ();
    void on_ruins_changed ();
    void on_temples_changed ();
    void on_signposts_changed ();
    void on_stones_changed ();
    void on_width_changed ();
    void on_height_changed ();
    void on_tileset_changed ();
    void on_armyset_changed ();
    void on_cityset_changed ();
    void on_shieldset_changed ();
    void on_players_changed ();
    void on_stone_road_chance_changed ();
    void update ();
    void populate_tileset_armyset_and_cityset(guint32 tilesize);
    int selected_tileset_id; //row number in combobox
    int selected_armyset_id; //row number in combobox
    int selected_shieldset_id; //row number in combobox
    int selected_cityset_id; //row number in combobox
    int selected_map_size_id; //row number in combobox
    int selected_tile_size_id; //row number in combobox
    int selected_fill_style_id; //row number in combobox
};

#endif
