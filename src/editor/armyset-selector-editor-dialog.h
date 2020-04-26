//  Copyright (C) 2020 Ben Asselstine
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
#ifndef ARMYSET_SELECTOR_EDITOR_DIALOG_H
#define ARMYSET_SELECTOR_EDITOR_DIALOG_H

#include <map>
#include <sigc++/connection.h>
#include <gtkmm.h>
#include "armyset.h"
#include "lw-editor-dialog.h"
#include "shield.h"

//! Armyset selector editor.
//! Shows and manages the large/small army unit selector animation, per side.
class ArmysetSelectorEditorDialog: public LwEditorDialog
{
 public:
    ArmysetSelectorEditorDialog(Gtk::Window &parent, Armyset *armyset);
    ~ArmysetSelectorEditorDialog();

    bool run();

 private:
    bool d_changed;
    Gtk::ComboBoxText *owner_combobox;
    Gtk::RadioButton *large_selector_radiobutton;
    Gtk::RadioButton *small_selector_radiobutton;
    Gtk::Button *selector_imagebutton;
    Gtk::ComboBoxText *shield_theme_combobox;
    Gtk::Grid *preview_table;
    Armyset *d_armyset;
    PixMask *small_selector;
    PixMask *large_selector;
    std::list<Glib::RefPtr<Gdk::Pixbuf> > selectors;
    sigc::connection heartbeat;
    std::list<Glib::RefPtr<Gdk::Pixbuf> >::iterator frame;

    void on_shieldset_changed();
    void on_owner_changed();
    bool on_image_chosen(Gtk::FileChooserDialog *d);
    void on_button_toggle ();
    void on_selector_imagebutton_clicked ();
    void on_heartbeat();

    void setup_shield_theme_combobox(Gtk::Box *box);
    void setup_owner_combobox(Gtk::Box *box);
    void update_selector_panel();
    void show_preview_selectors();
    bool loadSelector();
    void clearSelector();
    void fill_imagebutton ();
    Glib::ustring get_selector_filename ();
    void set_selector_filename (Glib::ustring f);
    bool load_selector_image (Glib::ustring filename);
    void clear_selector_image ();
    Gtk::FileChooserDialog* image_filechooser(bool clear);
    Shield::Colour get_selected_colour ();
};

#endif
