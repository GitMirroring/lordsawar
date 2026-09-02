//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2010, 2012, 2014, 2015, 2016, 2017, 2020,
//  2021, 2026 Ben Asselstine
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
//  Foundation, Inc., 31 Milk Street #960789, Boston, MA 02196, USA.

#include <gtkmm.h>
#ifndef SCENARIO_BUILDER_WINDOW_H
#define SCENARIO_BUILDER_WINDOW_H

class NewMapDialog;
class Army;
class Stack;
class City;
class Ruin;
class Temple;
class Stone;
class MapBackpack;
class SmallMap;
class PointerSizeMenuButton;
class ShieldMenuButton;
class UndoMgr;
class CreateScenarioRandomize;

#include "editor-map-widget.h"
#include "editor-undo-actions.h"

class ScenarioBuilderMenuButton: public Gtk::MenuButton
{
public:
    ScenarioBuilderMenuButton (Gtk::Window &p,
                               Glib::RefPtr<Gio::SimpleActionGroup> &group)
      :m_parent (p), m_actions (group)
      {
      }

    ~ScenarioBuilderMenuButton () override = default;

    void setup ()
      {
        setup_menu ();
      }

    sigc::signal<void(Glib::RefPtr<Gio::SimpleAction>)> m_signal_action_added;

private:
    Gtk::Window &m_parent;
    Glib::RefPtr<Gio::SimpleActionGroup> m_actions;

    void setup_actions (std::initializer_list<const char*> list)
      {
         for (auto s : list)
           setup_action (m_actions->add_action (s));
      }

    void setup_menu ()
      {
        auto menu = Gio::Menu::create ();

        auto new_menu = Gio::Menu::create ();
        new_menu->append (_("New Blank Map"), "lw.editor.file.new");
        new_menu->append (_("New Random Map..."), "lw.editor.file.new-random-map");
        menu->append_submenu (_("New"), new_menu);

        menu->append (_("Open..."), "lw.editor.file.open");
        menu->append (_("Save"), "lw.editor.file.save");
        menu->append (_("Save As..."), "lw.editor.file.save-as");

        auto edit_menu = Gio::Menu::create ();
        edit_menu->append (_("Properties"), "lw.editor.edit.properties");

        edit_menu->append (_("Items"), "lw.editor.edit.items");
        edit_menu->append (_("Fight Order"), "lw.editor.edit.fight-order");
        edit_menu->append (_("Miniature Map"), "lw.editor.edit.mini-map");
        edit_menu->append (_("Players"), "lw.editor.edit.players");
        edit_menu->append (_("Unique Rewards"), "lw.editor.edit.rewards");
        edit_menu->append (_("Scenario Media"), "lw.editor.edit.scenario-media");
        edit_menu->append (_("Randomize Objects"), "lw.editor.edit.randomize-objects");
        edit_menu->append (_("Assign Capital Cities"), "lw.editor.edit.assign-capitals");

        auto smooth_menu = Gio::Menu::create ();
        smooth_menu->append (_("Smooth Map"), "lw.editor.edit.smooth-map");
        smooth_menu->append (_("Smooth Screen"), "lw.editor.edit.smooth-screen");
        edit_menu->append_section (smooth_menu);

        menu->append_submenu (_("Edit"), edit_menu);

        auto item = Gio::MenuItem::create ("", "");
        auto id = Glib::Variant<Glib::ustring>::create ("zoom-menuitem");
        item->set_attribute_value ("custom", id);
        menu->append_item (item);

        auto view_menu = Gio::Menu::create ();
        view_menu->append (_("Toggle Grid"), "lw.editor.view.toggle-grid");
        menu->append_submenu (_("View"), view_menu);

        auto tools_menu = Gio::Menu::create ();
        tools_menu->append (_("Battle Calculator..."), "lw.editor.tools.battle-calculator");
        auto sets_menu = Gio::Menu::create ();
        sets_menu->append (_("Army Set Editor"), "lw.editor.edit.army-set");
        sets_menu->append (_("City Set Editor"), "lw.editor.edit.city-set");
        sets_menu->append (_("Shield Set Editor"), "lw.editor.edit.shield-set");
        sets_menu->append (_("Tile Set Editor"), "lw.editor.edit.tile-set");
        tools_menu->append_section (sets_menu);
        menu->append_submenu (_("Tools"), tools_menu);

        menu->append (_("Switch Sets..."), "lw.editor.edit.switch-sets");
        menu->append (_("Validate"), "lw.editor.file.validate");

        auto help_menu = Gio::Menu::create ();
        help_menu->append (_("Tutorial Video"), "lw.editor.help.tutorial-video");
        help_menu->append (_("About"), "lw.editor.help.about");
        menu->append_submenu (_("Help"), help_menu);

        menu->append (_("Keyboard Shortcuts"), "lw.editor.help.keyboard-shortcuts");
        menu->append (_("Quit"), "lw.editor.file.quit");

        property_primary () = true;
        set_menu_model (menu);

        setup_actions
          ({"editor.file.new", "editor.file.new-random-map",
            "editor.file.open", "editor.file.save", "editor.file.save-as",
            "editor.file.validate", "editor.file.quit",
            "editor.edit.properties", "editor.edit.mini-map",
            "editor.edit.scenario-media", "editor.edit.players",
            "editor.edit.items", "editor.edit.rewards",
            "editor.edit.smooth-screen", "editor.edit.smooth-map",
            "editor.edit.switch-sets", "editor.edit.army-set",
            "editor.edit.city-set", "editor.edit.shield-set",
            "editor.edit.tile-set", "editor.edit.fight-order",
            "editor.edit.randomize-objects",
            "editor.edit.assign-capitals",
            "editor.view.toggle-grid", "editor.tools.battle-calculator",
            "editor.help.tutorial-video", "editor.help.about",
            "editor.help.keyboard-shortcuts",

            // the other actions not in the menu, it's just easier to add these
            // here
            "editor.edit.undo", "editor.edit.redo",
            "editor.battle-calculator.set-attacking-stack",
            "editor.battle-calculator.set-defending-stack",
            "editor.battle-calculator.add-defending-stack",
            "editor.map.stack-details",
            "editor.map.city-details",
            "editor.map.ruin-details",
            "editor.map.signpost-details",
            "editor.map.temple-details",
            "editor.map.road-details",
            "editor.map.stone-details",
            "editor.map.bag-details",
            "editor.map.flag-details",
            "editor.map.tilestyle-details",
          });

        set_icon_name ("open-menu-symbolic");
      }

    void setup_action (Glib::RefPtr<Gio::SimpleAction> a)
      {
        m_signal_action_added.emit (a);
      }
};

class EditorShortcutsDialog : public Gtk::Window
{
public:
    EditorShortcutsDialog ()
      {
        set_title (_("Keyboard Shortcuts"));
        set_default_size (500, 400);
        set_modal (true);

        auto* scrolled = Gtk::make_managed<Gtk::ScrolledWindow>();
        scrolled->set_policy (Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);

        auto* flow = Gtk::make_managed<Gtk::FlowBox>();
        flow->set_max_children_per_line (1);
        flow->set_min_children_per_line (1);
        flow->set_selection_mode (Gtk::SelectionMode::NONE);
        flow->set_column_spacing (16);
        flow->set_row_spacing (16);
        flow->set_margin (16);
        flow->set_homogeneous (false);

        flow->append (*create_group
                      (_("Application"),
                       {
                           {_("New Blank Scenario"), "<Ctrl>N"},
                           {_("Open"), "<Ctrl>Q"},
                           {_("Save"), "<Ctrl>S"},
                           {_("Save As"), "<Ctrl><Shift>S"},
                           {_("Quit"), "Escape"},
                           {_("Quit"), "<Ctrl>Q"},
                       }));

        flow->append (*create_group
                      ("Editing",
                       {
                           {_("Undo"), "<Ctrl>Z"},
                           {_("Redo"), "<Shift><Ctrl>Z"},

                       }));
        scrolled->set_child (*flow);
        set_child (*scrolled);

        auto controller = Gtk::EventControllerKey::create ();
        controller->signal_key_pressed ().connect
          ([this] (guint keyval, guint, Gdk::ModifierType)
           {
             if (keyval == GDK_KEY_Escape)
               {
                 close ();
                 return true;
               }
             return false;
           }, false);
        add_controller (controller);
    }

private:
    Gtk::Widget* create_group (const Glib::ustring& title,
                              const std::vector<std::pair<Glib::ustring, Glib::ustring>>& shortcuts)
      {
        auto* frame = Gtk::make_managed<Gtk::Frame>();
        frame->add_css_class ("card");

        auto* box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
        box->set_spacing (8);
        box->set_margin (12);

        auto* title_label = Gtk::make_managed<Gtk::Label>(title);
        title_label->add_css_class ("heading");
        title_label->set_halign (Gtk::Align::START);
        box->append (*title_label);

        box->append (*Gtk::make_managed<Gtk::Separator>());

        for (const auto& [action, accel] : shortcuts)
          {
            auto* row = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
            row->set_spacing (12);

            auto* action_label = Gtk::make_managed<Gtk::Label>(action);
            action_label->set_halign (Gtk::Align::START);
            action_label->set_hexpand (true);

            auto* accel_label = Gtk::make_managed<Gtk::ShortcutLabel>();
            accel_label->set_accelerator (accel);
            accel_label->set_halign (Gtk::Align::END);

            row->append (*action_label);
            row->append (*accel_label);
            box->append (*row);
          }

        frame->set_child (*box);
        return frame;
      }
};

class ScenarioBuilderWindow: public Gtk::ApplicationWindow
{
public:

    ScenarioBuilderWindow ();
    ~ScenarioBuilderWindow ();

    void setup (std::string filename);

private:
    Glib::RefPtr<Gio::SimpleActionGroup> m_actions;
    Gtk::HeaderBar *m_header_bar;
    Gtk::Notebook *m_notebook;
    Gtk::Label *m_subtitle;
    ScenarioBuilderMenuButton *m_menu_button;
    std::map<Glib::ustring, Glib::RefPtr<Gio::SimpleAction>> m_simple_actions;
    Gtk::Label *m_pointer_position_label;
    Gtk::ToggleButton *m_pointer_button;
    Gtk::ToggleButton *m_move_button;
    Gtk::ToggleButton *m_erase_button;
    Gtk::ToggleButton *m_stack_button;
    Gtk::ToggleButton *m_city_button;
    Gtk::ToggleButton *m_ruin_button;
    Gtk::ToggleButton *m_temple_button;
    Gtk::ToggleButton *m_port_button;
    Gtk::ToggleButton *m_sign_button;
    Gtk::ToggleButton *m_road_button;
    Gtk::ToggleButton *m_bridge_button;
    Gtk::ToggleButton *m_stone_button;
    Gtk::ToggleButton *m_bag_button;
    Gtk::ToggleButton *m_standard_button;
    Gtk::ToggleButton *m_add_battle_button;
    Gtk::ToggleButton *m_tilestyle_button;
    Gtk::FlowBox *m_button_box;
    std::vector<Gtk::ToggleButton *>m_terrain_buttons;
    Gtk::DrawingArea *m_map_drawing_area;
    PointerSizeMenuButton *m_pointer_size_menu_button;
    ShieldMenuButton *m_shield_menu_button;

    bool m_dark;
    sigc::connection m_dark_style_handler;
    bool m_scenario_modified;
    bool m_new_scenario_needs_saving;
    GameScenario *m_scenario;
    enum EditorMapWidget::Pointer m_pointer;
    guint32 m_width;
    guint32 m_height;
    CreateScenarioRandomize* m_create_scenario_names;
    guint32 m_pointer_size;
    SmallMap *m_smallmap;
    EditorMapWidget *m_bigmap = NULL;
    Cairo::RefPtr<Cairo::Surface> m_smallmap_surface;
    struct MouseMotionEvent m_smallmap_mouse_motion = {};
    std::string m_current_save_filename;
    std::string m_load_filename;
    UndoMgr *m_umgr;

    std::list<sigc::connection> m_action_connections;
    std::list<sigc::connection> m_connections;
    std::list<sigc::connection> m_terrain_button_connections;
    sigc::connection m_load_tick;

    //this is from the context menu for battle calculator.
    //the stack the battle-calculator actions will operate on.
    Stack *m_selected_stack_for_battle_calculator;
    std::list<Army*> m_battle_calculator_attackers;
    std::list<Army*> m_battle_calculator_defenders;

    //context menu, which stack/city/ruin/etc is selected to be opened
    Stack *m_selected_stack_for_details;
    City *m_selected_city_for_details;
    Ruin *m_selected_ruin_for_details;
    Signpost *m_selected_signpost_for_details;
    Temple *m_selected_temple_for_details;
    Road *m_selected_road_for_details;
    Stone *m_selected_stone_for_details;
    MapBackpack *m_selected_bag_for_details;
    Vector<int> m_selected_tilestyle_for_details;
    Vector<int> m_selected_tilestyle_mouse_position;
    Vector<int> m_selected_stone_mouse_position;
    Vector<int> m_selected_road_mouse_position;

    void load_scenario (std::string filename,
                        sigc::slot<void(bool, Glib::ustring)> after);
    void clear_save_file_of_scenario_specific_data ();
    void setup ();
    void setup_header_bar (Gtk::MenuButton *menu_button);
    void draw_pointer_buttons ();
    void draw_buttons ();
    void add_connection (sigc::connection c);
    void add_action_connection (sigc::connection c);
    void action_connect (Glib::ustring name, sigc::slot<void()> slot);
    void connect_action_signals ();
    void disconnect_signals ();
    void disconnect_action_signals ();
    void block_terrain_button_signals ();
    void unblock_terrain_button_signals ();
    void block_signals ();
    void unblock_signals ();
    void connect_signals ();
    void update ();
    void update_actions ();
    void update_window_title ();
    void populate ();
    Gtk::ToggleButton* setup_toggle_button (Glib::ustring tooltip);
    Gtk::FlowBox *create_button_box ();
    Gtk::Box *create_map_box ();
    Gtk::Box *create_right_pane ();
    UndoAction* execute_action (UndoAction *action2);
    void setup_accels ();
    void destroy_accels ();
    void setup_dark_mode_change ();
    void set_button_image (Gtk::Button *button, std::string file);
    void add_terrain_buttons ();
    void connect_terrain_button_signals ();
    void add_terrain_button_connection (sigc::connection c);
    void disconnect_terrain_button_signals ();
    void set_pointer_button (enum EditorMapWidget::Pointer pointer);
    void create_blank_scenario ();
    void set_filled_map (int width, int height, int fill_style,
                         std::vector<Glib::ustring> armysets, std::string cityset,
                         std::string shieldset, std::string tileset,
                         std::vector<bool> players_active);
    std::string get_default_map_filename ();
    void setup_create_scenario_randomize ();
    void setup_smallmap ();
    void check_discard (Glib::ustring msg, sigc::slot<void(bool)> after);
    void check_save_valid (sigc::slot<void(bool)> after);
    void save_current_scenario_as (sigc::slot<void(bool)> after);
    void save_current_scenario_file (std::string filename,
                                     sigc::slot<void(bool)> after);
    void check_quit (sigc::slot<void(bool)> after);
    void setup_quit ();
    void on_save_as_activated ();
    void setup_bigmap ();
    guint32 get_editor_button_size ();
    void change_map (EditorUndoAction_ChangeMap *action);
    void reload_scenario (EditorUndoAction_Save *action);
    void reload_armyset (EditorUndoAction_ArmySet *a);
    void reload_armyset (guint32 id);
    void reload_cityset (EditorUndoAction_CitySet *a);
    void reload_cityset ();
    void reload_tileset (EditorUndoAction_TileSet *a);
    void reload_cityset (guint32 id);
    void reload_shieldset (EditorUndoAction_ShieldSet *a);
    void reload_shieldset ();
    void set_random_map (NewMapDialog *d);
    void select_object (Vector<int> pos,
                        std::vector<UniquelyIdentified*> objects);
    void popup_dialog_for_object (UniquelyIdentified *obj, Glib::ustring tag,
                                  Vector<int> mouse_pos);
    void convert_sav_to_map ();
    void change_city_ownership (City *city, Player *player);
};

#endif
