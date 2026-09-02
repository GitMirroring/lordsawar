//  Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2014, 2015, 2020, 2021,
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
#ifndef ARMYSET_WINDOW_H
#define ARMYSET_WINDOW_H

class Shieldset;
class Armyset;
class ArmyProto;
class UndoMgr;
class LwCombo;
class FileLabel;
class TarFileMaskedImage;
class TarFileImage;
#include "armyset-undo.h"

class ArmySetMenuButton: public Gtk::MenuButton
{
public:
    ArmySetMenuButton (Gtk::Window &p, Glib::RefPtr<Gio::SimpleActionGroup> &group)
      :m_parent (p), m_actions (group)
      {
      }

    ~ArmySetMenuButton () override = default;

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

        menu->append (_("New"), "lw.armyset.file.new");
        menu->append (_("Open..."), "lw.armyset.file.open");
        menu->append (_("Save"), "lw.armyset.file.save");
        menu->append (_("Save As..."), "lw.armyset.file.save-as");
        menu->append (_("Validate"), "lw.armyset.file.validate");
        menu->append (_("Properties"), "lw.armyset.edit.properties");
        menu->append (_("Preview Armies"), "lw.armyset.view.preview-armies");
        auto help_menu = Gio::Menu::create ();
        help_menu->append (_("Tutorial Video"), "lw.armyset.help.tutorial-video");
        help_menu->append (_("About"), "lw.armyset.help.about");
        menu->append_submenu (_("Help"), help_menu);
        menu->append (_("Keyboard Shortcuts"), "lw.armyset.help.keyboard-shortcuts");
        menu->append (_("Quit"), "lw.armyset.file.quit");

        property_primary () = true;
        set_menu_model (menu);

        setup_actions
          ({"armyset.file.new", "armyset.file.open", "armyset.file.save",
            "armyset.file.save-as", "armyset.file.validate",
            "armyset.file.quit", "armyset.edit.properties",
            "armyset.edit.make-same", "armyset.view.preview-armies",
            "armyset.help.tutorial-video", "armyset.help.about",
            "armyset.help.keyboard-shortcuts",

            // the other actions not in the menu, it's just easier to add these
            // here
            "armyset.add-army", "armyset.remove-army",
            "armyset.army-up", "armyset.army-down",
          });

        set_icon_name ("open-menu-symbolic");

      }

    void setup_action (Glib::RefPtr<Gio::SimpleAction> a)
      {
        m_signal_action_added.emit (a);
      }
};

class ArmyProtoRow: public Glib::Object
{
public:
    ArmyProto *m_army;
    sigc::signal<void()> m_signal_changed;

    sigc::signal<void()> signal_changed ()
      {
        return m_signal_changed;
      }

    static Glib::RefPtr<ArmyProtoRow> create (ArmyProto *a)
      {
        return
          Glib::make_refptr_for_instance<ArmyProtoRow> (new ArmyProtoRow (a));
      }

    void changed ()
      {
        m_signal_changed.emit ();
      }

protected:
    ArmyProtoRow (ArmyProto *a)
      : m_army (a)
      {
      }
};

class ArmySetShortcutsDialog : public Gtk::Window
{
public:
    ArmySetShortcutsDialog ()
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

class ArmySetWindow: public Gtk::ApplicationWindow
{
public:

    ArmySetWindow ();
    ~ArmySetWindow ();
    void setup (Shieldset *shieldset, Armyset *armyset);

    sigc::signal<void(guint32)> signal_armyset_saved ()
      {
        return m_armyset_saved;
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
    ArmySetMenuButton *m_menu_button;
    Gtk::StackSwitcher *m_switcher;
    std::map<Glib::ustring, Glib::RefPtr<Gio::SimpleAction>> m_simple_actions;
    Gtk::Button *m_ship_button;
    FileLabel *m_ship_filelabel;
    Gtk::Button *m_hero_flag_button;
    FileLabel *m_hero_flag_filelabel;
    Gtk::Button *m_bag_button;
    FileLabel *m_bag_filelabel;
    Gtk::Button *m_white_small_select_button;
    FileLabel *m_white_small_select_filelabel;
    Gtk::Button *m_white_large_select_button;
    FileLabel *m_white_large_select_filelabel;
    Gtk::Button *m_green_small_select_button;
    FileLabel *m_green_small_select_filelabel;
    Gtk::Button *m_green_large_select_button;
    FileLabel *m_green_large_select_filelabel;
    Gtk::Button *m_yellow_small_select_button;
    FileLabel *m_yellow_small_select_filelabel;
    Gtk::Button *m_yellow_large_select_button;
    FileLabel *m_yellow_large_select_filelabel;
    Gtk::Button *m_dark_blue_small_select_button;
    FileLabel *m_dark_blue_small_select_filelabel;
    Gtk::Button *m_dark_blue_large_select_button;
    FileLabel *m_dark_blue_large_select_filelabel;
    Gtk::Button *m_orange_small_select_button;
    FileLabel *m_orange_small_select_filelabel;
    Gtk::Button *m_orange_large_select_button;
    FileLabel *m_orange_large_select_filelabel;
    Gtk::Button *m_light_blue_small_select_button;
    FileLabel *m_light_blue_small_select_filelabel;
    Gtk::Button *m_light_blue_large_select_button;
    FileLabel *m_light_blue_large_select_filelabel;
    Gtk::Button *m_red_small_select_button;
    FileLabel *m_red_small_select_filelabel;
    Gtk::Button *m_red_large_select_button;
    FileLabel *m_red_large_select_filelabel;
    Gtk::Button *m_black_small_select_button;
    FileLabel *m_black_small_select_filelabel;
    Gtk::Button *m_black_large_select_button;
    FileLabel *m_black_large_select_filelabel;
    Gtk::Entry *m_name_entry;
    Gtk::Button *m_white_image_button;
    FileLabel *m_white_image_filelabel;
    Gtk::Switch *m_make_same_switch;
    Gtk::Button *m_green_image_button;
    FileLabel *m_green_image_filelabel;
    Gtk::Button *m_yellow_image_button;
    FileLabel *m_yellow_image_filelabel;
    Gtk::Button *m_light_blue_image_button;
    FileLabel *m_light_blue_image_filelabel;
    Gtk::Button *m_orange_image_button;
    FileLabel *m_orange_image_filelabel;
    Gtk::Button *m_dark_blue_image_button;
    FileLabel *m_dark_blue_image_filelabel;
    Gtk::Button *m_red_image_button;
    FileLabel *m_red_image_filelabel;
    Gtk::Button *m_black_image_button;
    FileLabel *m_black_image_filelabel;
    Gtk::Button *m_neutral_image_button;
    FileLabel *m_neutral_image_filelabel;
    Gtk::SpinButton *m_production_spinbutton;
    Gtk::SpinButton *m_cost_spinbutton;
    Gtk::SpinButton *m_new_cost_spinbutton;
    Gtk::SpinButton *m_upkeep_spinbutton;
    Gtk::SpinButton *m_strength_spinbutton;
    Gtk::SpinButton *m_moves_spinbutton;
    Gtk::SpinButton *m_exp_spinbutton;
    Gtk::SpinButton *m_id_spinbutton;
    LwCombo *m_hero_combobox;
    Gtk::Switch *m_awardable_switch;
    Gtk::Switch *m_defends_ruins_switch;
    Gtk::SpinButton *m_sight_spinbutton;
    Gtk::Switch *m_move_forests_switch;
    Gtk::Switch *m_move_marshes_switch;
    Gtk::Switch *m_move_hills_switch;
    Gtk::Switch *m_move_mountains_switch;
    Gtk::Switch *m_can_fly_switch;
    Gtk::Switch *m_add1strinopen_switch;
    Gtk::Switch *m_add2strinopen_switch;
    Gtk::Switch *m_add1strinforest_switch;
    Gtk::Switch *m_add2strinforest_switch;
    Gtk::Switch *m_add1strinhills_switch;
    Gtk::Switch *m_add2strinhills_switch;
    Gtk::Switch *m_add1strincity_switch;
    Gtk::Switch *m_add2strincity_switch;
    Gtk::Switch *m_add1stackinhills_switch;
    Gtk::Switch *m_suballcitybonus_switch;
    Gtk::Switch *m_sub1enemystack_switch;
    Gtk::Switch *m_sub2enemystack_switch;
    Gtk::Switch *m_add1stack_switch;
    Gtk::Switch *m_add2stack_switch;
    Gtk::Switch *m_suballnonherobonus_switch;
    Gtk::Switch *m_suballherobonus_switch;
    Gtk::Switch *m_confer_move_bonus_switch;
    Gtk::ScrolledWindow *m_scrolled_window;
    Gtk::ScrolledWindow *m_columnview_scrolled_window;

    Gtk::ColumnView *m_treeview;
    Glib::RefPtr<Gtk::SingleSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<ArmyProtoRow>> m_store;

    std::list<sigc::connection> m_action_connections;
    std::list<sigc::connection> m_connections;

    Shieldset *m_shieldset;
    Armyset *m_armyset;
    Glib::ustring m_current_save_filename;
    bool m_armyset_modified;
    bool m_new_armyset_needs_saving;
    UndoMgr *m_umgr;

    bool m_make_same;

    sigc::signal<void(guint32)> m_armyset_saved;
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
    void load_armyset (Glib::ustring filename, sigc::slot<void(bool)> after);
    ArmyProto *get_selected_army ();
    guint32 get_selected_index ();
    void set_move_bonus (Gtk::Switch *sw, ArmyProto *a, guint32 val);
    void set_army_bonus (Gtk::Switch *sw, ArmyProto *a, guint32 val);
    void connect_signals ();
    void update ();
    bool is_bottom_row_selected ();
    bool is_top_row_selected ();
    void update_actions ();
    void update_armyset_panel ();
    void update_window_title ();
    Gtk::ScrolledWindow* populate_misc_images ();
    Gtk::ScrolledWindow* populate_selector_images ();
    Gtk::ScrolledWindow* populate_army_panel ();
    void reselection (const Glib::RefPtr<ArmyProtoRow>& target);
    Gtk::Box* populate_side_pane ();
    Gtk::Box* populate_armies ();
    void populate ();
    UndoAction* execute_action (UndoAction *a2);
    void reload_armyset (ArmySetUndoAction_Save *action);
    void check_discard (Glib::ustring msg, sigc::slot<void(bool)> after);
    void check_name_valid (bool existing, sigc::slot<void(bool)> after);
    void check_name_valid2 (bool existing, Glib::ustring newname,
                            sigc::slot<void(bool)> after);
    bool is_valid_name ();
    void check_save_valid (bool existing, sigc::slot<void(bool)> after);
    void setup_accels ();
    void on_edit_armyset_info_activated ();
    void on_save_as_activated ();
    void save_current_armyset_file_as (sigc::slot<void(bool)> after);
    void save_current_armyset_file (Glib::ustring filename, sigc::slot<void(bool)> after);
    void check_quit (sigc::slot<void(bool)> after);
    void change_image (Glib::ustring msg, TarFileImage *im,
                       sigc::slot<void(bool, bool, Glib::ustring)> after);
    void on_change_clicked (Glib::ustring msg, TarFileImage *im);
    void change_image (Glib::ustring msg, TarFileMaskedImage *im, int lone,
                       sigc::slot<void(bool, bool, Glib::ustring)> after);
    void on_change_clicked (Glib::ustring msg, TarFileMaskedImage *im, int lone);
    void on_army_change_clicked (Glib::ustring msg, TarFileMaskedImage *im, int lone);
    void setup_treeview ();
    void fill_treeview ();
    ArmyProto* get_army_by_index (ArmySetUndoAction_ArmyIndex *i);
    void scroll_army_to_top ();
    void scroll_treeview_to_bottom ();
    void setup_quit ();
    void clear_army_panel ();
    bool calculate_make_same ();
    void sync_make_same ();
};
#endif
