//  Copyright (C) 2009, 2010, 2011, 2012, 2014, 2015, 2020, 2021,
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
#ifndef CITYSET_WINDOW_H
#define CITYSET_WINDOW_H

class Cityset;
class FileLabel;
class UndoMgr;

#include "cityset-undo.h"

class CitySetMenuButton: public Gtk::MenuButton
{
public:
    CitySetMenuButton (Gtk::Window &p, Glib::RefPtr<Gio::SimpleActionGroup> &group)
      :m_parent (p), m_actions (group)
      {
      }

    ~CitySetMenuButton () override = default;

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

        menu->append (_("New"), "lw.cityset.file.new");
        menu->append (_("Open..."), "lw.cityset.file.open");
        menu->append (_("Save"), "lw.cityset.file.save");
        menu->append (_("Save As..."), "lw.cityset.file.save-as");
        menu->append (_("Validate"), "lw.cityset.file.validate");
        menu->append (_("Properties"), "lw.cityset.edit.properties");
        auto help_menu = Gio::Menu::create ();
        help_menu->append (_("Tutorial Video"), "lw.cityset.help.tutorial-video");
        help_menu->append (_("About"), "lw.cityset.help.about");
        menu->append_submenu (_("Help"), help_menu);
        menu->append (_("Keyboard Shortcuts"), "lw.cityset.help.keyboard-shortcuts");
        menu->append (_("Quit"), "lw.cityset.file.quit");

        property_primary () = true;
        set_menu_model (menu);

        setup_actions
          ({"cityset.file.new", "cityset.file.open", "cityset.file.save",
            "cityset.file.save-as", "cityset.file.validate",
            "cityset.file.quit", "cityset.edit.properties",
            "cityset.help.tutorial-video", "cityset.help.about",
            "cityset.help.keyboard-shortcuts",

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

class CitySetShortcutsDialog : public Gtk::Window
{
public:
    CitySetShortcutsDialog ()
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

class CitySetWindow: public Gtk::ApplicationWindow
{
public:

    CitySetWindow ();
    ~CitySetWindow ();
    void setup (Cityset *cityset);

    sigc::signal<void(guint32)> signal_cityset_saved ()
      {
        return m_cityset_saved;
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
    CitySetMenuButton *m_menu_button;
    std::map<Glib::ustring, Glib::RefPtr<Gio::SimpleAction>> m_simple_actions;
    Gtk::SpinButton *m_city_tile_width_spinbutton;
    Gtk::SpinButton *m_ruin_tile_width_spinbutton;
    Gtk::SpinButton *m_temple_tile_width_spinbutton;
    Gtk::Button *m_change_citypics_button;
    Gtk::Button *m_change_razedcitypics_button;
    Gtk::Button *m_change_portpic_button;
    Gtk::Button *m_change_signpostpic_button;
    Gtk::Button *m_change_ruinpics_button;
    Gtk::Button *m_change_templepics_button;
    Gtk::Button *m_change_towerpics_button;
    FileLabel *m_change_citypics_filelabel;
    FileLabel *m_change_razedcitypics_filelabel;
    FileLabel *m_change_portpic_filelabel;
    FileLabel *m_change_signpostpic_filelabel;
    FileLabel *m_change_ruinpics_filelabel;
    FileLabel *m_change_templepics_filelabel;
    FileLabel *m_change_towerpics_filelabel;

    std::list<sigc::connection> m_action_connections;
    std::list<sigc::connection> m_connections;

    Cityset *m_cityset;
    Glib::ustring m_current_save_filename;
    bool m_cityset_modified;
    bool m_new_cityset_needs_saving;
    UndoMgr *m_umgr;

    sigc::signal<void(guint32)> m_cityset_saved;
    sigc::signal<void()> m_signal_closed;

    void setup_header_bar (Gtk::MenuButton *menu_button);
    Gtk::Box *create_attribute_row (Glib::ustring title, Glib::ustring desc,
                                    Glib::ustring css);
    void add_connection (sigc::connection c);
    void add_action_connection (sigc::connection c);
    void action_connect (Glib::ustring name, sigc::slot<void()> slot);
    void connect_action_signals ();
    void disconnect_signals ();
    void disconnect_action_signals ();
    void load_cityset (Glib::ustring filename, sigc::slot<void(bool)> after);
    void connect_signals ();
    void update ();
    void update_actions ();
    void update_cityset_panel ();
    void update_window_title ();
    void populate ();
    UndoAction* execute_action (UndoAction *a2);
    void reload_cityset (CitySetUndoAction_Save *action);
    void check_discard (Glib::ustring msg, sigc::slot<void(bool)> after);
    void check_name_valid (bool existing, sigc::slot<void(bool)> after);
    void check_name_valid2 (bool existing, Glib::ustring newname,
                            sigc::slot<void(bool)> after);
    bool is_valid_name ();
    void check_save_valid (bool existing, sigc::slot<void(bool)> after);
    void setup_accels ();
    void on_change_clicked (Glib::ustring msg, TarFileImage *im);
    void change_image (Glib::ustring msg, TarFileImage *im,
                       sigc::slot<void(bool, bool, Glib::ustring)> after);
    void on_edit_cityset_info_activated ();
    void on_save_as_activated();
    void save_current_cityset_file_as (sigc::slot<void(bool)> after);
    void save_current_cityset_file (Glib::ustring filename, sigc::slot<void(bool)> after);
    void check_quit (sigc::slot<void(bool)> after);
    void setup_quit ();
};
#endif
