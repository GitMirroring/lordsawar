//  Copyright (C) 2009, 2014, 2020, 2021 Ben Asselstine
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
#ifndef IMAGE_EDITOR_DIALOG_H
#define IMAGE_EDITOR_DIALOG_H

#include <sigc++/connection.h>
#include <gtkmm.h>
#include "PixMask.h"
#include "lw-editor-dialog.h"
#include "undo-mgr.h"

class TarFileImage;
class TarFile;

//! general picture editor.
/**
 * This class doesn't actually edit the image, instead it shows the image
 * being edited in each player color.  The user can pick a new file to be
 * the new image.
 */
class ImageEditorDialog: public LwEditorDialog
{
 public:
    ImageEditorDialog(Gtk::Window &parent, TarFileImage *im, double ratio, Glib::ustring empty_str = "");
    ~ImageEditorDialog();

    Glib::ustring get_filename() {return d_target_filename;}
    int run();
    void hide();

    void set_title(Glib::ustring s) {dialog->set_title(s);}

    bool installFile (TarFile *t, TarFileImage *im, Glib::ustring filename);
    bool uninstallFile (TarFile *t, TarFileImage *im);
 private:
    UndoMgr *umgr;
    double d_ratio;
    guint32 d_num_frames;
    guint32 d_active_frame;
    Glib::ustring d_target_filename;
    Glib::ustring d_orig_target_filename;
    Glib::ustring d_empty_str;
    TarFileImage *d_im;
    std::vector<PixMask*> frames;

    sigc::connection heartbeat;
    Gtk::Button *imagebutton;
    Gtk::Image *image;
    Gtk::Button *clear_button;
    Gtk::Button *undo_button;
    Gtk::Button *redo_button;

    void on_image_chosen(Gtk::FileChooserDialog *d);
    void show_image();
    void on_heartbeat(bool incr);
    void on_imagebutton_clicked ();
    Gtk::FileChooserDialog* image_filechooser(bool clear);
    void update_imagebutton_label (Glib::ustring filename);
    bool load_frames (Glib::ustring filename);
    void on_undo_activated ();
    void on_redo_activated ();
    void update ();
    void connect_signals ();
    void disconnect_signals ();
    std::list<sigc::connection> connections;
    UndoAction *executeAction (UndoAction *action);
};

#endif
