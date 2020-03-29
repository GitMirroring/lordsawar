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
#ifndef TILESET_EXPLOSION_PICTURE_EDITOR_DIALOG_H
#define TILESET_EXPLOSION_PICTURE_EDITOR_DIALOG_H

#include <map>
#include <sigc++/connection.h>
#include <gtkmm.h>
#include "tileset.h"
#include "lw-editor-dialog.h"

class TilePreviewScene;

//! Tileset explosion picture editor.
class TilesetExplosionPictureEditorDialog: public LwEditorDialog
{
 public:
    TilesetExplosionPictureEditorDialog(Gtk::Window &parent, Tileset * tileset);
    ~TilesetExplosionPictureEditorDialog();

    bool run();

 private:
    bool d_changed;
    Gtk::RadioButton *large_explosion_radiobutton;
    Gtk::RadioButton *small_explosion_radiobutton;
    Gtk::Button *explosion_imagebutton;
    Gtk::Image *scene_image;
    Tileset *d_tileset;
    PixMask *d_explosion;

    bool on_image_chosen(Gtk::FileChooserDialog *d);
    void on_large_toggled();
    void on_small_toggled();
    void show_explosion_image();
    void update_panel();

    void update_scene(TilePreviewScene *scene);

    Gtk::FileChooserDialog* image_filechooser(bool clear);
    void on_explosion_imagebutton_clicked ();

};

#endif
