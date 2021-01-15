//  Copyright (C) 2007, 2008, 2009, 2010, 2014, 2020, 2021 Ben Asselstine
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
#ifndef SHIELDSET_INFO_DIALOG_H
#define SHIELDSET_INFO_DIALOG_H

#include <gtkmm.h>
#include "shieldset.h"
#include "lw-editor-dialog.h"

//! Shieldset Info Editor.  Change the name/description/etc of the Shieldset.
class ShieldSetInfoDialog: public LwEditorDialog
{
 public:
    ShieldSetInfoDialog(Gtk::Window &parent, Shieldset *shieldset);
    ~ShieldSetInfoDialog();

    //returns true if we changed anything
    bool run();

    Glib::ustring getName () const {return d_name;}
    Glib::ustring getDescription () const {return d_description;}
    Glib::ustring getCopyright () const {return d_copyright;}
    Glib::ustring getLicense () const {return d_license;}
    guint32 getSmallWidth () const {return d_small_width;}
    guint32 getSmallHeight () const {return d_small_height;}
    guint32 getMediumWidth () const {return d_medium_width;}
    guint32 getMediumHeight () const {return d_medium_height;}
    guint32 getLargeWidth () const {return d_large_width;}
    guint32 getLargeHeight () const {return d_large_height;}

 private:
    Shieldset *d_shieldset;
    bool d_changed;
    Glib::ustring d_name;
    Glib::ustring d_description;
    Glib::ustring d_copyright;
    Glib::ustring d_license;
    guint32 d_small_width;
    guint32 d_small_height;
    guint32 d_medium_width;
    guint32 d_medium_height;
    guint32 d_large_width;
    guint32 d_large_height;
    Gtk::Entry *name_entry;
    Gtk::TextView *copyright_textview;
    Gtk::TextView *license_textview;
    Gtk::Entry *filename_entry;
    Gtk::Button *close_button;
    Gtk::Label *status_label;
    Gtk::TextView *description_textview;
    Gtk::Label *location_label;
    Gtk::Notebook *notebook;
    Gtk::SpinButton *small_width_spinbutton;
    Gtk::SpinButton *small_height_spinbutton;
    Gtk::SpinButton *medium_width_spinbutton;
    Gtk::SpinButton *medium_height_spinbutton;
    Gtk::SpinButton *large_width_spinbutton;
    Gtk::SpinButton *large_height_spinbutton;
    Gtk::Button *fit_button;

    void on_name_changed();
    void on_copyright_changed ();
    void on_license_changed ();
    void on_description_changed ();
    void on_small_width_changed ();
    void on_small_height_changed ();
    void on_medium_width_changed ();
    void on_medium_height_changed ();
    void on_large_width_changed ();
    void on_large_height_changed ();
    void on_fit_pressed ();
};

#endif
