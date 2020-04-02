//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2010, 2012, 2014, 2015, 2020 Ben Asselstine
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
#ifndef GUI_MAIN_WINDOW_H
#define GUI_MAIN_WINDOW_H

#include <memory>
#include <vector>
#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <gtkmm.h>

#include "map-tip-position.h"
#include "editorbigmap.h"

class EditorBigMap;
class SmallMap;
class GameScenario;
class CreateScenarioRandomize;
class City;
class Player;
class Signpost;
class Ruin;
class RoadEditorTip;

#include "UniquelyIdentified.h"

//! Scenario editor.  Edits a Scenario.
class MainWindow: public sigc::trackable
{
 public:
    MainWindow(Glib::ustring load_filename = "");
    ~MainWindow();

    void show();
    void hide() {window->hide();}

    void init();
    void show_initial_map();
    Gtk::Window &get_window() { return *window; }

    sigc::signal<void> editor_quit;

 private:
    Gtk::Window* window;
    EditorBigMap* bigmap;
    SmallMap* smallmap;
    GameScenario* game_scenario;
    CreateScenarioRandomize* d_create_scenario_names;

    Gtk::Image *bigmap_image;
    Gtk::EventBox *bigmap_eventbox;
    Gtk::CheckMenuItem *fullscreen_menuitem;
    Gtk::MenuItem *new_map_menuitem;
    Gtk::MenuItem *load_map_menuitem;
    Gtk::MenuItem *save_map_menuitem;
    Gtk::MenuItem *save_map_as_menuitem;
    Gtk::MenuItem *import_map_from_sav_menuitem;
    Gtk::MenuItem *validate_menuitem;
    Gtk::MenuItem *quit_menuitem;
    Gtk::MenuItem *edit_players_menuitem;
    Gtk::MenuItem *edit_map_info_menuitem;
    Gtk::MenuItem *edit_shieldset_menuitem;
    Gtk::MenuItem *edit_armyset_menuitem;
    Gtk::MenuItem *edit_cityset_menuitem;
    Gtk::MenuItem *edit_tileset_menuitem;
    Gtk::MenuItem *edit_fight_order_menuitem;
    Gtk::MenuItem *toggle_tile_graphics_menuitem;
    Gtk::MenuItem *toggle_grid_menuitem;
    Gtk::MenuItem *zoom_in_menuitem;
    Gtk::MenuItem *zoom_out_menuitem;
    Gtk::MenuItem *smooth_map_menuitem;
    Gtk::MenuItem *smooth_screen_menuitem;
    Gtk::MenuItem *switch_sets_menuitem;
    Gtk::MenuItem *edit_items_menuitem ;
    Gtk::MenuItem *edit_rewards_menuitem;
    Gtk::MenuItem *edit_smallmap_menuitem;
    Gtk::MenuItem *edit_remove_all_stacks_menuitem;
    Gtk::MenuItem *edit_scenario_media_menuitem;
    Gtk::MenuItem *random_all_cities_menuitem;
    Gtk::MenuItem *random_unnamed_cities_menuitem;
    Gtk::MenuItem *random_all_ruins_menuitem;
    Gtk::MenuItem *random_unnamed_ruins_menuitem;
    Gtk::MenuItem *random_all_temples_menuitem;
    Gtk::MenuItem *random_unnamed_temples_menuitem;
    Gtk::MenuItem *random_all_signs_menuitem;
    Gtk::MenuItem *random_unnamed_signs_menuitem;
    Gtk::MenuItem *random_assign_capital_cities_menuitem;
    Gtk::MenuItem *battle_calculator_menuitem;
    Gtk::MenuItem *help_about_menuitem;
    Gtk::Viewport *terrain_tile_style_viewport;
    Gtk::FlowBox *terrain_tile_style_grid;
    Gtk::Image *smallmap_image;
    Glib::ustring current_save_filename;
    bool needs_saving;
    Gtk::Grid *terrain_type_table;
    Gtk::Label *mouse_position_label;
    Gtk::RadioToolButton *pointer_radiobutton;
    Gtk::Box *players_hbox;

    
    GdkEventButton *button_event;

    bool on_delete_event();

    bool on_bigmap_mouse_button_event(GdkEventButton *e);
    bool on_bigmap_mouse_motion_event(GdkEventMotion *e);
    bool on_bigmap_key_event();
    bool on_bigmap_leave_event();

    bool on_smallmap_mouse_button_event(GdkEventButton *e);
    bool on_smallmap_mouse_motion_event(GdkEventMotion *e);
    
    void on_new_map_activated();
    void on_load_map_activated();
    void on_save_map_activated();
    void on_import_map_activated();
    void on_validate_activated();
    void on_save_map_as_activated();
    void on_quit_activated();
    bool quit();
    void on_edit_map_info_activated();
    void on_edit_players_activated();
    void on_edit_shieldset_activated();
    void on_shieldset_saved(guint32 id);
    void on_edit_armyset_activated();
    void on_armyset_saved(guint32 id);
    void on_edit_tileset_activated();
    void on_tileset_saved(guint32 id);
    void on_edit_cityset_activated();
    void on_edit_smallmap_activated();
    void on_cityset_saved(guint32 id);
    void on_smooth_map_activated();
    void on_smooth_screen_activated();
    void on_switch_sets_activated();
    void on_edit_items_activated();
    void on_edit_rewards_activated();
    void on_edit_fight_order_activated();
    void on_remove_all_stacks_activated();
    void on_edit_scenario_media_activated();

    void on_fullscreen_activated();
    void on_grid_toggled();
    void on_zoom_in_activated ();
    void on_zoom_out_activated ();

    void on_random_all_cities_activated();
    void on_random_unnamed_cities_activated();
    void on_random_all_ruins_activated();
    void on_random_unnamed_ruins_activated();
    void on_random_all_temples_activated();
    void on_random_unnamed_temples_activated();
    void on_random_all_signs_activated();
    void on_random_unnamed_signs_activated();
    void on_random_assign_capital_cities_activated();

    void on_help_about_activated();
    
    struct PointerItem
    {
	Gtk::RadioToolButton *button;
	EditorBigMap::Pointer pointer;
        Glib::ustring image_file;
	int size;
    };

    std::vector<PointerItem> pointer_items;
    
    struct TerrainItem
    {
	Gtk::RadioButton *button;
	Tile::Type terrain;
    };

    struct TileStyleItem
    {
	Gtk::RadioButton *button;
	int tile_style_id;
    };

    std::vector<TerrainItem> terrain_items;
    std::vector<TileStyleItem> tile_style_items;
    
    void on_pointer_radiobutton_toggled();
    void on_terrain_radiobutton_toggled();
    void on_tile_style_radiobutton_toggled();
    

    void clear_save_file_of_scenario_specific_data();
    Tile::Type get_terrain();
    int get_pointer_index();
    int get_tile_style_id();
    void setup_pointer_radiobutton(Glib::RefPtr<Gtk::Builder> xml,
	Glib::ustring prefix, Glib::ustring image_file,
	EditorBigMap::Pointer pointer, int size);
    void setup_terrain_radiobuttons();

    void init_maps();
    void set_filled_map(int width, int height, int fill_style, 
			Glib::ustring tileset, Glib::ustring shieldset, 
			Glib::ustring cityset, Glib::ustring armyset);
    void set_random_map(int width, int height,
			int grass, int water, int swamp, int forest,
			int hills, int mountains,
			int cities, int ruins, int temples, int signposts,
			Glib::ustring tileset, Glib::ustring shieldset,
			Glib::ustring cityset, Glib::ustring armyset,
                        bool generate_roads, bool random_names);

    void clear_map_state();
    void init_map_state();
    void remove_tile_style_buttons();
    void setup_tile_style_buttons(Tile::Type terrain);
    void randomize_signpost(Signpost *signpost);
    void randomize_city(City *city);
    void randomize_ruin(Ruin *ruin);

    // map callbacks
    void on_smallmap_changed(Cairo::RefPtr<Cairo::Surface> map);
    void on_bigmap_changed(Cairo::RefPtr<Cairo::Surface> map);
    void on_smallmap_water_changed();
    void on_objects_selected(std::vector<UniquelyIdentified *> objects);
    void on_mouse_on_tile(Vector<int> tile);
    
    void popup_dialog_for_object(UniquelyIdentified *object);

    void auto_select_appropriate_pointer();

    bool on_bigmap_exposed();
    bool on_smallmap_exposed();
    void on_bigmap_surface_changed(Gtk::Allocation box);
    void on_save_as_activated(Gtk::FileChooserDialog *chooser);
    bool on_bigmap_scrolled(GdkEventScroll* event);
    void redraw();
    void fill_players();
    void on_battle_calculator_activated();

    Glib::ustring getDefaultMapFilename();

    struct PlayerItem
    {
	Gtk::ToggleButton *button;
	int player_id;
    };
    std::list<PlayerItem> player_buttons;
    void on_player_toggled(PlayerItem item);
    void update_window_title();
    void on_bag_selected(Vector<int> pos);
    void on_road_edited(Vector<int> pos, int new_type);
    void on_stack_selected_for_battle_calculator(Stack *s);
    void add_attacker_to_battle_calculator(Stack *s);
    void add_defender_to_battle_calculator(Stack *s);
    void append_defender_to_battle_calculator(Stack *s);
    void change_city_ownership(City *city, Player *player);
    void update_buttons();
    void update_terrain_buttons();

    bool on_window_state_event (GdkEventWindowState *e);
    bool on_configure_event (GdkEventConfigure *e);
    void zoom (double scale);
    bool close_road_editor_tip ();

    int d_width;
    int d_height;
    Glib::ustring d_load_filename;// filename given on the command line.
    RoadEditorTip *road_editor_tip;
    std::list<Army*> battle_calculator_attackers;
    std::list<Army*> battle_calculator_defenders;
    Gtk::Allocation unmaximized_box;

    
};

#endif
