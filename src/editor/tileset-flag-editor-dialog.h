//  Copyright (C) 2009, 2010, 2014, 2020 Ben Asselstine
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
#ifndef TILESET_FLAG_EDITOR_DIALOG_H
#define TILESET_FLAG_EDITOR_DIALOG_H

#include <map>
#include <sigc++/connection.h>
#include <gtkmm.h>
#include "tileset.h"
#include "lw-editor-dialog.h"

//! Tileset flag editor.
//! Shows and manages the flags that appear on stacks
class TilesetFlagEditorDialog: public LwEditorDialog
{
 public:
    TilesetFlagEditorDialog(Gtk::Window &parent, Tileset * tileset);
    ~TilesetFlagEditorDialog();

    bool run();

 private:
    bool d_changed;
    PixMask *d_flags;
    Gtk::Button *flag_imagebutton;
    Gtk::ComboBoxText *shield_theme_combobox;
    Gtk::Grid *preview_table;
    Tileset *d_tileset;
    std::map< guint32, std::list<Glib::RefPtr<Gdk::Pixbuf> >* > flags;
    sigc::connection heartbeat;
    std::map<guint32, std::list<Glib::RefPtr<Gdk::Pixbuf> >::iterator> frame;

    void setup_shield_theme_combobox(Gtk::Box *box);
    void on_shieldset_changed();
    bool on_image_chosen(Gtk::FileChooserDialog *d);
    void update_flag_panel();
    void show_preview_flags();

    bool loadFlag();
    void clearFlag();

    void on_heartbeat();
    Gtk::FileChooserDialog* image_filechooser(bool clear);
    void on_flag_imagebutton_clicked ();
};

#endif
