//  Copyright (C) 2009, 2010, 2014, 2015, 2020 Ben Asselstine
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
#ifndef GUI_CITYSET_WINDOW_H
#define GUI_CITYSET_WINDOW_H

#include <memory>
#include <vector>
#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <gtkmm.h>

#include "cityset.h"

//! Cityset Editor.  Edit an cityset.
class CitySetWindow: public sigc::trackable
{
 public:
    CitySetWindow(Glib::ustring load_filename = "");
    ~CitySetWindow();

    Gtk::Window &get_window() { return *window; }
    void show() {window->show();};
    void hide() {window->hide();};

    sigc::signal<void, guint32> cityset_saved;

 private:
    Gtk::Window* window;
    Glib::ustring current_save_filename;
    Cityset *d_cityset; //current cityset
    bool needs_saving;
    Gtk::MenuItem *new_cityset_menuitem;
    Gtk::MenuItem *load_cityset_menuitem;
    Gtk::MenuItem *save_cityset_menuitem;
    Gtk::MenuItem *save_as_menuitem;
    Gtk::MenuItem *validate_cityset_menuitem;
    Gtk::MenuItem *edit_cityset_info_menuitem;
    Gtk::MenuItem *quit_menuitem;
    Gtk::MenuItem *help_about_menuitem;
    Gtk::MenuItem *tutorial_menuitem;
    Gtk::Button *change_citypics_button;
    Gtk::Button *change_razedcitypics_button;
    Gtk::Button *change_portpic_button;
    Gtk::Button *change_signpostpic_button;
    Gtk::Button *change_ruinpics_button;
    Gtk::Button *change_templepic_button;
    Gtk::Button *change_towerpics_button;
    Gtk::SpinButton *city_tile_width_spinbutton;
    Gtk::SpinButton *ruin_tile_width_spinbutton;
    Gtk::SpinButton *temple_tile_width_spinbutton;
    Gtk::Alignment *cityset_alignment;
    Gtk::Notebook *notebook;

    void update_cityset_panel();

    bool load_cityset(Glib::ustring filename);
    bool save_current_cityset();

    //callbacks
    void on_new_cityset_activated();
    void on_load_cityset_activated();
    void on_save_cityset_activated();
    void on_save_as_activated();
    void on_validate_cityset_activated();
    void on_quit_activated();
    bool on_window_closed(GdkEventAny*);
    bool quit();
    void on_edit_cityset_info_activated();
    void on_help_about_activated();
    void on_tutorial_video_activated();
    void on_city_tile_width_changed();
    void on_city_tile_width_text_changed();
    void on_ruin_tile_width_changed();
    void on_ruin_tile_width_text_changed();
    void on_temple_tile_width_changed();
    void on_temple_tile_width_text_changed();
    void on_change_citypics_clicked();
    void on_change_razedcitypics_clicked();
    void on_change_portpic_clicked();
    void on_change_signpostpic_clicked();
    void on_change_ruinpics_clicked();
    void on_change_templepic_clicked();
    void on_change_towerpics_clicked();
    Glib::ustring change_image (Glib::ustring m, Glib::ustring i, int num, std::vector<PixMask *> frames, bool &cleared, int tile_width);
    void update_window_title();
    void show_add_file_error(Gtk::Dialog &d, Glib::ustring file);
    void show_remove_file_error(Gtk::Dialog &d, Glib::ustring file);

    bool make_new_cityset ();
    bool load_cityset ();
    bool save_current_cityset_file (Glib::ustring filename = "");
    bool save_current_cityset_file_as ();

    bool check_discard (Glib::ustring msg);
    bool check_save_valid (bool existing);
    bool check_name_valid (bool existing);
    bool isValidName ();
};

#endif
