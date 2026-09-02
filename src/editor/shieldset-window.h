//  Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2014, 2015, 2017, 2020,
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
#ifndef SHIELDSET_WINDOW_H
#define SHIELDSET_WINDOW_H

class Shieldset;
class Shield;
class UndoMgr;
class FileLabel;

#include "shieldset-undo.h"

class ShieldRow: public Glib::Object
{
public:
    Shield *m_shield;
    Glib::ustring m_name;
    sigc::signal<void()> m_signal_changed;

    sigc::signal<void()> signal_changed ()
      {
        return m_signal_changed;
      }

    void changed ()
      {
        m_signal_changed.emit ();
      }

    static Glib::RefPtr<ShieldRow> create (Shield *s)
      {
        return Glib::make_refptr_for_instance<ShieldRow> (new ShieldRow (s));
      }

protected:
    ShieldRow (Shield *s)
      : m_shield (s)
      {
        if (s)
          m_name = Shield::colorToFriendlyName (Shield::Color (s->getOwner ()));
        else
          m_name = "";
      }
};

class ShieldSetMenuButton: public Gtk::MenuButton
{
public:
    ShieldSetMenuButton (Gtk::Window &p, Glib::RefPtr<Gio::SimpleActionGroup> &group)
      :m_parent (p), m_actions (group)
      {
      }

    ~ShieldSetMenuButton () override = default;

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

        menu->append (_("New"), "lw.shieldset.file.new");
        menu->append (_("Open..."), "lw.shieldset.file.open");
        menu->append (_("Save"), "lw.shieldset.file.save");
        menu->append (_("Save As..."), "lw.shieldset.file.save-as");
        menu->append (_("Validate"), "lw.shieldset.file.validate");
        menu->append (_("Copy White Images Down"), "lw.shieldset.edit.copy_white_down");
        menu->append (_("Preview Tartan"), "lw.shieldset.view.preview-tartan");
        menu->append (_("Properties"), "lw.shieldset.edit.properties");
        auto help_menu = Gio::Menu::create ();
        help_menu->append (_("Tutorial Video"), "lw.shieldset.help.tutorial-video");
        help_menu->append (_("About"), "lw.shieldset.help.about");
        menu->append_submenu (_("Help"), help_menu);
        menu->append (_("Keyboard Shortcuts"), "lw.shieldset.help.keyboard-shortcuts");
        menu->append (_("Quit"), "lw.shieldset.file.quit");

        property_primary () = true;
        set_menu_model (menu);

        setup_actions
          ({"shieldset.file.new", "shieldset.file.open", "shieldset.file.save",
            "shieldset.file.save-as", "shieldset.file.validate",
            "shieldset.file.quit", "shieldset.edit.copy_white_down",
            "shieldset.view.preview-tartan", "shieldset.edit.properties",
            "shieldset.help.tutorial-video", "shieldset.help.about",
            "shieldset.help.keyboard-shortcuts",

            // the other actions not in the menu, it's just easier to add these
            // here
          });

        set_icon_name ("open-menu-symbolic");

      }

    void setup_action (Glib::RefPtr<Gio::SimpleAction> a)
      {
        m_signal_action_added.emit (a);
      }
};

class ShieldSetShortcutsDialog : public Gtk::Window
{
public:
    ShieldSetShortcutsDialog ()
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

class ShieldSetWindow: public Gtk::ApplicationWindow
{
public:

    ShieldSetWindow ();
    ~ShieldSetWindow ();
    void setup (Shieldset *shieldset);

    sigc::signal<void(guint32)> signal_shieldset_saved ()
      {
        return m_shieldset_saved;
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
    ShieldSetMenuButton *m_menu_button;
    std::map<Glib::ustring, Glib::RefPtr<Gio::SimpleAction>> m_simple_actions;
    Gtk::ColumnView *m_treeview;
    Gtk::Button *m_small_shield_button;
    FileLabel *m_small_shield_file_label;
    Gtk::Button *m_medium_shield_button;
    FileLabel *m_medium_shield_file_label;
    Gtk::Button *m_large_shield_button;
    FileLabel *m_large_shield_file_label;
    Gtk::Button *m_left_tartan_button;
    FileLabel *m_left_tartan_file_label;
    Gtk::Button *m_middle_tartan_button;
    FileLabel *m_middle_tartan_file_label;
    Gtk::Button *m_right_tartan_button;
    FileLabel *m_right_tartan_file_label;
    Gtk::SpinButton *m_mask_colors_spinbutton;
    Gtk::ColorButton *m_first_colorbutton;
    Gtk::ColorButton *m_second_colorbutton;
    Gtk::ColorButton *m_third_colorbutton;

    std::list<sigc::connection> m_action_connections;
    std::list<sigc::connection> m_connections;

    Glib::RefPtr<Gtk::SingleSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<ShieldRow>> m_store;

    Shieldset *m_shieldset;
    Glib::ustring m_current_save_filename;
    bool m_shieldset_modified;
    bool m_new_shieldset_needs_saving;
    UndoMgr *m_umgr;

    sigc::signal<void(guint32)> m_shieldset_saved;
    sigc::signal<void()> m_signal_closed;

    void setup_header_bar (Gtk::MenuButton *menu_button);
    void setup_treeview ();
    void fill_treeview ();
    Gtk::Box *create_attribute_row (Glib::ustring title, Glib::ustring desc,
                                    Glib::ustring css);
    void add_connection (sigc::connection c);
    void add_action_connection (sigc::connection c);
    void action_connect (Glib::ustring name, sigc::slot<void()> slot);
    void connect_action_signals ();
    void disconnect_signals ();
    void disconnect_action_signals ();
    void load_shieldset (Glib::ustring filename, sigc::slot<void(bool)> after);
    std::vector<Gdk::RGBA> get_current_colors ();
    void change_pic (Shield::Color color, TarFileMaskedImage *mim,
                     Glib::ustring msg,
                     sigc::slot<void(bool,bool,Glib::ustring)> after);
    void connect_signals ();
    void update ();
    void update_actions ();
    Shield * get_current_shield ();
    void update_shieldset_panel ();
    void update_window_title ();
    void populate ();
    Shield* get_shield_by_index (ShieldSetUndoAction_ShieldIndex *a);
    UndoAction* execute_action (UndoAction *a2);
    void reload_shieldset (ShieldSetUndoAction_Save *action);
    bool doReloadShieldset (ShieldSetUndoAction_Save *action);
    void check_discard (Glib::ustring msg, sigc::slot<void(bool)> after);
    void check_name_valid (bool existing, sigc::slot<void(bool)> after);
    void check_name_valid2 (bool existing, Glib::ustring newname,
                            sigc::slot<void(bool)> after);
    void check_save_valid (bool existing, sigc::slot<void(bool)> after);
    void setup_accels ();
    void on_edit_shieldset_info_activated ();
    void copy_white_down ();
    void save_current_shieldset_file_as (sigc::slot<void(bool)> after);
    void save_current_shieldset_file (Glib::ustring filename, sigc::slot<void(bool)> after);
    void on_save_as_activated ();
    void check_quit (sigc::slot<void(bool)> after);
    void setup_quit ();
};
#endif
