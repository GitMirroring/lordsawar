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
#ifndef MASKED_IMAGE_EDITOR_DIALOG_H
#define MASKED_IMAGE_EDITOR_DIALOG_H

#include <gtkmm.h>
#include "shield.h"
#include "lw-editor-dialog.h"


//! general masked picture editor.
/**
 * This class doesn't actually edit the image, instead it shows the image
 * being edited in each player colour.  The user can pick a new file to be
 * the new image.
 *
 * the shieldset is required to define the mask colours.
 *
 * Resize the images when 9 (max players + 1) of them exceed MAX_IMAGES_WIDTH.
 *
 */
class Shieldset;
class MaskedImageEditorDialog: public LwEditorDialog
{
 public:
    static const int MAX_IMAGES_WIDTH;
    //MaskedImageEditorDialog(Gtk::Window &parent, Glib::ustring filename, double ratio, Shieldset *shieldset = NULL);
    MaskedImageEditorDialog(Gtk::Window &parent, Glib::ustring filename, PixMask *image, PixMask *mask, double ratio, Shieldset *shieldset = NULL);
    ~MaskedImageEditorDialog();

    void set_title(Glib::ustring t) {dialog->set_title(t);}

    Glib::ustring get_filename() {return d_target_filename;}
    int run();
    void hide();

 private:
    double d_ratio;
    Glib::ustring d_target_filename;
    PixMask *d_image;
    PixMask *d_mask;
    Gtk::Button *imagebutton;
    Gtk::Image *image_white;
    Gtk::Image *image_green;
    Gtk::Image *image_yellow;
    Gtk::Image *image_light_blue;
    Gtk::Image *image_red;
    Gtk::Image *image_dark_blue;
    Gtk::Image *image_orange;
    Gtk::Image *image_black;
    Gtk::Image *image_neutral;
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
