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
#ifndef PER_PLAYER_MASKED_IMAGE_EDITOR_DIALOG_H
#define PER_PLAYER_MASKED_IMAGE_EDITOR_DIALOG_H

#include <gtkmm.h>
#include "shield.h"
#include "lw-editor-dialog.h"


//! per player masked picture editor
/**
 * This class doesn't actually edit the image, instead it shows the image
 * being edited in each player colour.  The user can pick a new file to be
 * the new image.
 *
 * The shieldset is required to define the mask colours.
 *
 * The underlying images have the top row as the image, and the bottom row as the mask.
 * There are 8 images, one for each player (not including neutral.)
 *
 * This class could be named better.
 */
class Shieldset;
class PerPlayerMaskedImageEditorDialog: public LwEditorDialog
{
 public:
    static const int MAX_IMAGES_WIDTH;
    PerPlayerMaskedImageEditorDialog(Gtk::Window &parent, Glib::ustring filename, std::vector<PixMask *>image, std::vector<PixMask *>mask, double ratio, Shieldset *shieldset = NULL);
    ~PerPlayerMaskedImageEditorDialog();

    void set_title(Glib::ustring t) {dialog->set_title(t);}

    Glib::ustring get_filename() {return d_target_filename;}
    int run();
    void hide();

 private:
    double d_ratio;
    Glib::ustring d_target_filename;
    std::vector<PixMask *>d_image;
    std::vector<PixMask *>d_mask;
    Gtk::Button *imagebutton;
    Gtk::Image *image_white;
    Gtk::Image *image_green;
    Gtk::Image *image_yellow;
    Gtk::Image *image_light_blue;
    Gtk::Image *image_red;
    Gtk::Image *image_dark_blue;
    Gtk::Image *image_orange;
    Gtk::Image *image_black;
    Shieldset * d_shieldset;
    Gtk::ComboBoxText *shield_theme_combobox;
    Gtk::Button *clear_button;
    void on_shieldset_changed();
    void on_image_chosen(Gtk::FileChooserDialog *d);
    void show_image();
    void update_panel();
    void on_imagebutton_clicked ();
    Gtk::FileChooserDialog* image_filechooser(bool clear);
    void setup_shield_theme_combobox(Gtk::Box *box);
    bool load_image ();
};

#endif
