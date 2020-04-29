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
#ifndef TILESET_MOVE_BONUS_IMAGE_DIALOG_H
#define TILESET_MOVE_BONUS_IMAGE_DIALOG_H

#include <gtkmm.h>
#include "lw-editor-dialog.h"
class Tileset;
class PixMask;

class TilesetMoveBonusImageDialog: public LwEditorDialog
{
 public:
    TilesetMoveBonusImageDialog(Gtk::Window &parent, Tileset *tileset);
    ~TilesetMoveBonusImageDialog();

    bool get_changed () {return d_changed;}
 private:
    bool d_changed;
    Tileset *d_tileset;
    Gtk::Notebook *notebook;
    Gtk::Button *all_imagechooser_button;
    Gtk::Button *water_imagechooser_button;
    Gtk::Button *forest_imagechooser_button;
    Gtk::Button *hills_imagechooser_button;
    Gtk::Button *mountains_imagechooser_button;
    Gtk::Button *swamp_imagechooser_button;

    Gtk::Image *all_image;
    Gtk::Image *water_image;
    Gtk::Image *forest_image;
    Gtk::Image *hills_image;
    Gtk::Image *mountains_image;
    Gtk::Image *swamp_image;
    Gtk::Image *forest_hills_image;
    Gtk::Image *forest_mountains_image;
    Gtk::Image *forest_swamp_image;
    Gtk::Image *hills_mountains_image;
    Gtk::Image *hills_swamp_image;
    Gtk::Image *mountains_swamp_image;
    Gtk::Image *forest_hills_mountains_image;
    Gtk::Image *forest_hills_swamp_image;
    Gtk::Image *forest_mountains_swamp_image;
    Gtk::Image *hills_mountains_swamp_image;
    Gtk::Image *forest_hills_mountains_swamp_image;

    void on_water_clicked();
    void on_forest_clicked();
    void on_hills_clicked();
    void on_mountains_clicked();
    void on_swamp_clicked();
    void on_all_clicked();

    void update_button_names ();
    void update_preview ();

    void on_image_button_activated(sigc::slot<Glib::ustring> getName, sigc::slot<void,Glib::ustring> setName, PixMask *im);
};

#endif
