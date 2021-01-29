//  Copyright (C) 2008, 2009, 2010, 2014, 2020, 2021 Ben Asselstine
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
#ifndef TILESET_SELECTOR_EDITOR_DIALOG_H
#define TILESET_SELECTOR_EDITOR_DIALOG_H

#include <map>
#include <sigc++/connection.h>
#include <gtkmm.h>
#include "tileset.h"
#include "lw-editor-dialog.h"
#include "undo-mgr.h"

class TarFileMaskedImage;

//! Tileset selector editor.
//! Shows and manages the large and small army unit selector animation.
class TilesetSelectorEditorDialog: public LwEditorDialog
{
 public:
    TilesetSelectorEditorDialog(Gtk::Window &parent, Tileset *tileset);
    ~TilesetSelectorEditorDialog();

    bool run();

 private:
    bool d_changed;
    UndoMgr *umgr;
    Gtk::RadioButton *large_selector_radiobutton;
    Gtk::RadioButton *small_selector_radiobutton;
    Gtk::Button *selector_imagebutton;
    Gtk::ComboBoxText *shield_theme_combobox;
    Gtk::Grid *preview_table;
    Gtk::Button *undo_button;
    Gtk::Button *redo_button;
    Tileset *d_tileset;
    TarFileMaskedImage *small_selector;
    TarFileMaskedImage *large_selector;
    std::map< guint32, std::list<Glib::RefPtr<Gdk::Pixbuf> >* > selectors;
    sigc::connection heartbeat;
    std::map<guint32, std::list<Glib::RefPtr<Gdk::Pixbuf> >::iterator> frame;
    bool d_large;
    guint32 d_shield_row;

    void on_shieldset_changed();
    bool on_image_chosen(Gtk::FileChooserDialog *d);
    void on_button_toggle ();
    void on_selector_imagebutton_clicked ();
    void on_heartbeat();

    void setup_shield_theme_combobox(Gtk::Box *box);
    void update_selector_panel();
    void show_preview_selectors();
    bool loadSelector();
    void clearSelector();
    void fill_imagebutton ();
    Glib::ustring get_selector_filename ();
    void set_selector_filename (Glib::ustring f);
    void clear_selector_image ();
    Gtk::FileChooserDialog* image_filechooser(bool clear);
    void on_undo_activated ();
    void on_redo_activated ();
    void update ();
    void connect_signals ();
    void disconnect_signals ();
    std::list<sigc::connection> connections;
    UndoAction *executeAction (UndoAction *action);
};

#endif
