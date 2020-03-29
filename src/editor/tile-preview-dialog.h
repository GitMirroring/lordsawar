//  Copyright (C) 2008, 2009, 2010, 2014, 2020 Ben Asselstine
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
#ifndef TILE_PREVIEW_DIALOG_H
#define TILE_PREVIEW_DIALOG_H

#include <sigc++/signal.h>
#include <gtkmm.h>
#include "Tile.h"
#include "tile-preview-scene.h"
#include "lw-editor-dialog.h"

//! Tile Preview Dialog.  Shows completeness and correctness of tilesets.
class TilePreviewDialog: public LwEditorDialog
{
 public:
    TilePreviewDialog(Gtk::Window &parent, Tile *tile, Tile *secondary);
    ~TilePreviewDialog() {}

    void run();

    sigc::signal<void, guint32> tilestyle_selected;

 private:
    Tile *d_tile;
    Gtk::Button *next_button;
    Gtk::Button *previous_button;
    Gtk::Button *refresh_button;
    Gtk::Image *preview_image;
    Gtk::EventBox *eventbox;
    Gtk::Label *selected_tilestyle_label;

    std::vector<PixMask* > tilestyle_images;

    void on_next_clicked();
    void on_previous_clicked();
    void on_refresh_clicked();
    void update_buttons();
    void update_scene(TilePreviewScene *scene);
    bool on_mouse_button_event(GdkEventButton *e);
    bool on_mouse_motion_event(GdkEventMotion *e);
    void on_tilestyle_id_hovered(guint32 id);
    void add_scene(TilePreviewScene *s);
    std::list<TilePreviewScene*> scenes;
    std::list<TilePreviewScene*>::iterator current_scene;
    guint32 d_tileSize;
};

#endif
