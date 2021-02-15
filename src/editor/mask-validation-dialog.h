//  Copyright (C) 2021 Ben Asselstine
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
#ifndef MASK_VALIDATION_DIALOG_H
#define MASK_VALIDATION_DIALOG_H

#include <sigc++/connection.h>
#include <gtkmm.h>
#include "PixMask.h"
#include "lw-editor-dialog.h"
#include "undo-mgr.h"
#include "TarFileMaskedImage.h"

//! Mask Validation Dialog
/**
 * This is for tar file masked images, because we need a little extra
 * information to make sense of them.
 *
 * We're trying to determine the geometry of the content with respect to the
 * number of masks that will be colored in the players' colors.
 *
 * If the tar file masked image is in the horizontal orientation, we return
 * how many columns the image has minus one.
 * If vertical orientation, we return how many rows the image has minus one.
 *
 */
class MaskValidationDialog: public LwEditorDialog
{
 public:
    MaskValidationDialog(Gtk::Window &parent, Glib::ustring filename,
                         TarFileMaskedImage::MaskOrientation orientation);
    ~MaskValidationDialog();

    guint32 get_num_masks () {return d_num_masks;}
    int run();
    void hide();

 private:
    UndoMgr *umgr;

    Glib::ustring d_filename;
    guint32 d_num_masks;
    TarFileMaskedImage::MaskOrientation d_orientation;
    PixMask *masked_image;

    Gtk::SpinButton *masks_spinbutton;
    Gtk::Image *image;
    Gtk::Button *undo_button;
    Gtk::Button *redo_button;

    void show_image();
    void on_masks_text_changed();
    void on_masks_changed();
    void on_undo_activated ();
    void on_redo_activated ();
    void update ();
    void connect_signals ();
    void disconnect_signals ();
    std::list<sigc::connection> connections;
    UndoAction *executeAction (UndoAction *action);
    Glib::RefPtr<Gdk::Pixbuf> draw_lines_for_vertical ();
    Glib::RefPtr<Gdk::Pixbuf> draw_lines_for_horizontal ();
};

#endif
