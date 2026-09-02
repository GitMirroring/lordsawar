//  Copyright (C) 2008, 2009, 2010, 2011, 2012, 2014, 2015, 2017, 2020, 2021,
//  2026 Ben Asselstine
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
#ifndef TILESET_WINDOW_H
#define TILESET_WINDOW_H


class LwCombo;
class Tileset;
class Tile;
class UndoMgr;
class FileLabel;
class Shieldset;
#include "tileset-undo.h"

class TileSetMenuButton: public Gtk::MenuButton
{
public:
    TileSetMenuButton (Gtk::Window &p, Glib::RefPtr<Gio::SimpleActionGroup> &group)
      :m_parent (p), m_actions (group)
      {
      }

    ~TileSetMenuButton () override = default;

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

        menu->append (_("New"), "lw.tileset.file.new");
        menu->append (_("Open..."), "lw.tileset.file.open");
        menu->append (_("Save"), "lw.tileset.file.save");
        menu->append (_("Save As..."), "lw.tileset.file.save-as");
        menu->append (_("Validate"), "lw.tileset.file.validate");
        menu->append (_("Properties"), "lw.tileset.edit.properties");
        menu->append (_("Tile Styles..."), "lw.tileset.edit.tilestyles");
        menu->append (_("Preview Tile"), "lw.tileset.view.preview-tile");
        auto help_menu = Gio::Menu::create ();
        help_menu->append (_("Tutorial Video"), "lw.tileset.help.tutorial-video");
        help_menu->append (_("About"), "lw.tileset.help.about");
        menu->append_submenu (_("Help"), help_menu);
        menu->append (_("Keyboard Shortcuts"), "lw.tileset.help.keyboard-shortcuts");
        menu->append (_("Quit"), "lw.tileset.file.quit");

        property_primary () = true;
        set_menu_model (menu);

        setup_actions
          ({"tileset.file.new", "tileset.file.open", "tileset.file.save",
            "tileset.file.save-as", "tileset.file.validate",
            "tileset.file.quit", "tileset.edit.properties",
            "tileset.edit.tilestyles", "tileset.view.preview-tile",
            "tileset.help.tutorial-video", "tileset.help.about",
            "tileset.help.keyboard-shortcuts", 

            // the other actions not in the menu, it's just easier to add these
            // here
            "tileset.add-tile", "tileset.remove-tile",
          });

        set_icon_name ("open-menu-symbolic");

      }

    void setup_action (Glib::RefPtr<Gio::SimpleAction> a)
      {
        m_signal_action_added.emit (a);
      }
};

class TileRow: public Glib::Object
{
public:
    Tile *m_tile;
    sigc::signal<void()> m_signal_changed;

    sigc::signal<void()> signal_changed ()
      {
        return m_signal_changed;
      }

    static Glib::RefPtr<TileRow> create (Tile *t)
      {
        return
          Glib::make_refptr_for_instance<TileRow> (new TileRow (t));
      }

    void changed ()
      {
        m_signal_changed.emit ();
      }

protected:
    TileRow (Tile *t)
      : m_tile (t)
      {
      }
};

class TileSetShortcutsDialog : public Gtk::Window
{
public:
    TileSetShortcutsDialog ()
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
                      (_("Window"),
                       {
                           {_("Close"), "Escape"},
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

class TileSetWindow: public Gtk::ApplicationWindow
{
public:

    TileSetWindow ();
    ~TileSetWindow ();
    void setup (Shieldset *shieldset, Tileset *tileset);

    sigc::signal<void(guint32)> signal_tileset_saved ()
      {
        return m_tileset_saved;
      }

    sigc::signal<void()> signal_closed ()
      {
        return m_signal_closed;
      }


private:
    Glib::RefPtr<Gio::SimpleActionGroup> m_actions;
    Gtk::HeaderBar *m_header_bar;
    Gtk::Notebook *m_notebook;
    Gtk::Label *m_subtitle;
    TileSetMenuButton *m_menu_button;
    Gtk::StackSwitcher *m_switcher;
    std::map<Glib::ustring, Glib::RefPtr<Gio::SimpleAction>> m_simple_actions;
    Gtk::ScrolledWindow *m_scrolled_window;
    Gtk::ScrolledWindow *m_columnview_scrolled_window;
    Gtk::Entry *m_name_entry;
    Gtk::ColorButton *m_road_colorbutton;
    Gtk::ColorButton *m_ruin_colorbutton;
    Gtk::ColorButton *m_temple_colorbutton;
    Gtk::Button *m_small_select_button;
    FileLabel *m_small_select_filelabel;
    Gtk::Button *m_large_select_button;
    FileLabel *m_large_select_filelabel;
    Gtk::Button *m_explosion_button;
    FileLabel *m_explosion_filelabel;
    Gtk::Button *m_roads_button;
    FileLabel *m_roads_filelabel;
    Gtk::Button *m_stones_button;
    FileLabel *m_stones_filelabel;
    Gtk::Button *m_bridges_button;
    FileLabel *m_bridges_filelabel;
    Gtk::Button *m_fog_button;
    FileLabel *m_fog_filelabel;
    Gtk::Button *m_white_flags_button;
    FileLabel *m_white_flags_filelabel;
    Gtk::Button *m_green_flags_button;
    FileLabel *m_green_flags_filelabel;
    Gtk::Button *m_yellow_flags_button;
    FileLabel *m_yellow_flags_filelabel;
    Gtk::Button *m_dark_blue_flags_button;
    FileLabel *m_dark_blue_flags_filelabel;
    Gtk::Button *m_orange_flags_button;
    FileLabel *m_orange_flags_filelabel;
    Gtk::Button *m_light_blue_flags_button;
    FileLabel *m_light_blue_flags_filelabel;
    Gtk::Button *m_red_flags_button;
    FileLabel *m_red_flags_filelabel;
    Gtk::Button *m_black_flags_button;
    FileLabel *m_black_flags_filelabel;
    Gtk::Button *m_neutral_flags_button;
    FileLabel *m_neutral_flags_filelabel;
    Gtk::Button *m_move_bonus_forest_button;
    FileLabel *m_move_bonus_forest_filelabel;
    Gtk::Button *m_move_bonus_hills_button;
    FileLabel *m_move_bonus_hills_filelabel;
    Gtk::Button *m_move_bonus_water_button;
    FileLabel *m_move_bonus_water_filelabel;
    Gtk::Button *m_move_bonus_swamp_button;
    FileLabel *m_move_bonus_swamp_filelabel;
    Gtk::Button *m_move_bonus_mountains_button;
    FileLabel *m_move_bonus_mountains_filelabel;
    Gtk::Button *m_move_bonus_fly_button;
    FileLabel *m_move_bonus_fly_filelabel;
    LwCombo *m_type_combobox;
    LwCombo *m_pattern_combobox;
    Gtk::SpinButton *m_moves_spinbutton;
    Gtk::ColorButton *m_first_colorbutton;
    Gtk::ColorButton *m_second_colorbutton;
    Gtk::ColorButton *m_third_colorbutton;
    Gtk::Button *m_tilestyles_button;

    Gtk::ColumnView *m_treeview;
    Glib::RefPtr<Gtk::SingleSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<TileRow>> m_store;

    std::list<sigc::connection> m_action_connections;
    std::list<sigc::connection> m_connections;

    Shieldset *m_shieldset;
    Tileset *m_tileset;
    Glib::ustring m_current_save_filename;
    bool m_tileset_modified;
    bool m_new_tileset_needs_saving;
    UndoMgr *m_umgr;

    sigc::signal<void(guint32)> m_tileset_saved;
    sigc::signal<void()> m_signal_closed;

    void setup_header_bar (Gtk::MenuButton *menu_button);
    Gtk::Box *create_attribute_row (Glib::ustring title, Glib::ustring desc,
                                    Glib::ustring css);
    void add_connection (sigc::connection c);
    void add_action_connection (sigc::connection c);
    void action_connect (Glib::ustring name, sigc::slot<void()> slot);
    void connect_action_signals ();
    void block_signals ();
    void unblock_signals ();
    void disconnect_signals ();
    void disconnect_action_signals ();
    void load_tileset (Glib::ustring filename, sigc::slot<void(bool)> after);
    Tile *get_selected_tile ();
    guint32 get_selected_index ();
    void connect_signals ();
    void update ();
    void update_actions ();
    void update_tileset_panel ();
    void update_window_title ();
    Gtk::ScrolledWindow* populate_misc_images ();
    Gtk::Box * populate_mini_map_colors ();
    Gtk::ScrolledWindow* populate_tile_panel ();
    Gtk::Box* populate_side_pane ();
    Gtk::Box* populate_tiles ();
    void populate ();
    UndoAction* execute_action (UndoAction *a2);
    void reload_tileset (TileSetUndoAction_Save *action);
    void check_discard (Glib::ustring msg, sigc::slot<void(bool)> after);
    void check_name_valid (bool existing, sigc::slot<void(bool)> after);
    void check_name_valid2 (bool existing, Glib::ustring newname,
                            sigc::slot<void(bool)> after);
    bool is_valid_name ();
    void check_save_valid (bool existing, sigc::slot<void(bool)> after);
    void setup_accels ();
    void on_edit_tileset_info_activated ();
    void on_save_as_activated ();
    void save_current_tileset_file_as (sigc::slot<void(bool)> after);
    void save_current_tileset_file (Glib::ustring filename, sigc::slot<void(bool)> after);
    void check_quit (sigc::slot<void(bool)> after);
    void setup_treeview ();
    void fill_treeview ();
    Tile* get_tile_by_index (TileSetUndoAction_TileIndex *i);
    void scroll_tile_to_top ();
    void scroll_treeview_to_bottom ();
    void setup_quit ();
    void clear_tile_panel ();
    void change_image (Glib::ustring msg, TarFileImage *im,
                       sigc::slot<void(bool, bool, Glib::ustring)> after);
    void on_change_clicked (Glib::ustring msg, TarFileImage *im);
    void change_image (Glib::ustring msg, TarFileMaskedImage *im, int lone,
                       sigc::slot<void(bool, bool, Glib::ustring)> after);
    void on_change_clicked (Glib::ustring msg, TarFileMaskedImage *im, int lone);
    void on_flag_change_clicked (Glib::ustring msg, TarFileMaskedImage *im, int lone);
    bool is_second_color_sensitive ();
    bool is_third_color_sensitive ();
    void update_colorbuttons ();
    void set_default_values_by_type ();
};
#endif

