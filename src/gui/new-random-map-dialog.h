//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014, 2020, 2021 Ben Asselstine
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
#ifndef NEW_RANDOM_MAP_DIALOG_H
#define NEW_RANDOM_MAP_DIALOG_H

#include <gtkmm.h>

#include "Tile.h"
#include "game-parameters.h"
#include "lw-dialog.h"

//! A dialog to let the user create a new random map.
class NewRandomMapDialog: public LwDialog
{
 public:
    NewRandomMapDialog(Gtk::Window &parent);
    ~NewRandomMapDialog();

    int run();

    Glib::ustring getRandomMapFilename() const {return d_filename;};

    static Glib::ustring create_and_dump_scenario(const Glib::ustring &file,
                                                  const GameParameters &g,
                                                  sigc::slot<void> *pulse);
    
 private:

    void pulse();
    GameParameters getParams();
    struct Map
    {
	int width, height;
	int grass, water, swamp, forest, hills, mountains;
	int cities, ruins, temples;
	int signposts;
	Glib::ustring tileset;
	Glib::ustring shieldset;
	Glib::ustring cityset;
	Glib::ustring armyset;
    };

    Map map;

    Gtk::Box *dialog_vbox;
    Gtk::Box *dialog_action_area;
    Gtk::ComboBox *map_size_combobox;
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
    Gtk::Button *accept_button;
    Gtk::Button *cancel_button;
    Gtk::Switch *grass_random_switch;
    Gtk::Switch *water_random_switch;
    Gtk::Switch *swamp_random_switch;
    Gtk::Switch *forest_random_switch;
    Gtk::Switch *hills_random_switch;
    Gtk::Switch *mountains_random_switch;
    Gtk::Switch *cities_random_switch;
    Gtk::CheckButton *cities_can_produce_allies_checkbutton;
    Gtk::Notebook *notebook;
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
    Gtk::TreeModel::Row row;

    enum { MAP_SIZE_NORMAL = 0, MAP_SIZE_SMALL, MAP_SIZE_TINY };

    enum ActiveTerrainType { NONE, GRASS, WATER, FOREST, HILLS, MOUNTAINS,
      SWAMP, MAX_TERRAINS };

    //callbacks
    void on_map_size_changed();
    void on_grass_random_toggled();
    void on_water_random_toggled();
    void on_swamp_random_toggled();
    void on_forest_random_toggled();
    void on_hills_random_toggled();
    void on_mountains_random_toggled();
    void on_cities_random_toggled();
    void on_accept_clicked();
    void on_cancel_clicked();
    void on_value_changed (ActiveTerrainType type);
    void on_tile_size_changed();

    //helpers
    void alter_grass ();
    void alter_terrain (ActiveTerrainType type);
    guint32 get_active_tile_size();
    void take_percentages ();
    void augment_scale_value_by_type (ActiveTerrainType type, double amt);
    void assign_random_terrain (GameParameters &g);

    int dialog_response;
    Glib::ustring d_filename;
    double percentages[MAX_TERRAINS];
    ActiveTerrainType d_active_terrain;
    bool d_inhibit_scales;
    static int cmp (std::pair<int,double> const &a, std::pair<int,double> const &b);
};

#endif
