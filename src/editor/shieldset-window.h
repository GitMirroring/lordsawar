//  Copyright (C) 2007-2010, 2014, 2015, 2017, 2020, 2021 Ben Asselstine
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
#ifndef GUI_SHIELDSET_WINDOW_H
#define GUI_SHIELDSET_WINDOW_H

#include <memory>
#include <vector>
#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <gtkmm.h>

#include "shieldset.h"
#include "shieldstyle.h"
#include "shieldset-editor-actions.h"
#include "undo-mgr.h"

class Shield;
//! Shieldset Editor.  Edit an Shieldset.
class ShieldSetWindow: public sigc::trackable
{
 public:
    ShieldSetWindow(Glib::ustring load_filename = "");
    ~ShieldSetWindow();

    void show() {window->show();}
    void hide() {window->hide();}

    Gtk::Window &get_window() { return *window; }

    sigc::signal<void, guint32> shieldset_saved;

 private:
    bool shieldset_modified;
    bool new_shieldset_needs_saving;
    Gtk::Window* window;
    Glib::ustring current_save_filename;
    Shieldset *d_shieldset; //current shieldset
    Shield *d_shield; //current shield
    UndoMgr *umgr;
    Gtk::TreeView *shields_treeview;
    Gtk::Image *small_image;
    Gtk::Image *medium_image;
    Gtk::Image *large_image;
    Gtk::Image *left_tartan_image;
    Gtk::Image *center_tartan_image;
    Gtk::Image *right_tartan_image;
    Gtk::MenuItem *new_shieldset_menuitem;
    Gtk::MenuItem *load_shieldset_menuitem;
    Gtk::MenuItem *save_shieldset_menuitem;
    Gtk::MenuItem *save_as_menuitem;
    Gtk::MenuItem *validate_shieldset_menuitem;
    Gtk::MenuItem *edit_undo_menuitem;
    Gtk::MenuItem *edit_redo_menuitem;
    Gtk::MenuItem *edit_shieldset_info_menuitem;
    Gtk::MenuItem *edit_copy_shields_menuitem;
    Gtk::MenuItem *quit_menuitem;
    Gtk::MenuItem *help_about_menuitem;
    Gtk::MenuItem *tutorial_menuitem;
    Gtk::Button *change_smallpic_button;
    Gtk::Button *change_mediumpic_button;
    Gtk::Button *change_largepic_button;
    Gtk::Button *change_left_tartan_button;
    Gtk::Button *change_center_tartan_button;
    Gtk::Button *change_right_tartan_button;
    Gtk::ColorButton *player_colorbutton;
    Gtk::ColorButton *player_2ndcolorbutton;
    Gtk::ColorButton *player_3rdcolorbutton;
    Gtk::SpinButton *color_spinbutton;
    Gtk::Alignment *shield_alignment;

    class ShieldsColumns: public Gtk::TreeModelColumnRecord {
    public:
	ShieldsColumns() 
        { add(name); add(shield);}
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<Shield*> shield;
    };
    const ShieldsColumns shields_columns;
    Glib::RefPtr<Gtk::ListStore> shields_list;

    void on_new_shieldset_activated();
    void on_load_shieldset_activated();
    void on_save_shieldset_activated();
    void on_save_as_activated();
    void on_validate_shieldset_activated();
    void on_quit_activated();
    bool on_window_closed(GdkEventAny*);
    void on_edit_undo_activated ();
    void on_edit_redo_activated ();
    void on_edit_shieldset_info_activated();
    void on_edit_copy_shields_activated();
    void on_help_about_activated();
    void on_tutorial_video_activated ();
    void on_shield_selected();
    void on_shieldpic_changed(ShieldStyle::Type type);
    void on_tartanpic_changed (Tartan::Type type);
    void on_player_color_changed();
    void on_player_2nd_color_changed();
    void on_player_3rd_color_changed();
    void on_num_colors_changed();
    void on_num_colors_text_changed();

    bool make_new_shieldset ();
    bool load_shieldset ();
    bool load_shieldset(Glib::ustring filename);
    void add_shield_to_treeview (Shield *shield);
    bool save_current_shieldset_file(Glib::ustring filename = "");
    bool save_current_shieldset_file_as();
    bool quit();
    
    bool check_save_valid (bool existing);
    bool check_name_valid (bool existing);
    bool check_discard (Glib::ustring msg);

    void fill_shield_info(Shield *shield);
    void show_shield(ShieldStyle *ss, Shield *s, Gtk::Image *image);
    void show_tartan(Shield *s, Tartan::Type t, Gtk::Image *image);

    void process_shieldstyle(ShieldStyle *ss, Gtk::FileChooserDialog *d);
    void process_tartanpic (Tartan::Type t, Shield *s, Glib::ustring filename);

    Gtk::FileChooserDialog* shield_filechooser (Shield *s, ShieldStyle::Type t,
                                                bool clear);
    Gtk::FileChooserDialog* tartan_filechooser(Shield *s, Tartan::Type type,
                                               bool clear);
    Gtk::FileChooserDialog* image_filechooser (Glib::ustring title, bool clear);

    void update_shield_panel();
    void update_menuitems ();
    void refresh_shields();
    void update_window_title();
    void update ();

    bool isValidName ();

    void clearUndoAndRedo ();
    bool doReloadShieldset (ShieldSetEditorAction_Save *action);

    UndoAction *executeAction (UndoAction *a);
    int getCurIndex ();
    void disconnect_signals ();
    void connect_signals ();
    std::vector<sigc::connection> connections;
    void addUndo(ShieldSetEditorAction *a);
    Shield* getShieldByIndex (ShieldSetEditorAction_ShieldIndex *i);
    std::vector<Gdk::RGBA> get_current_colors ();
};

#endif
