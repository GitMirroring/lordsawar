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
#ifndef ARMYSET_INFO_DIALOG_H
#define ARMYSET_INFO_DIALOG_H

#include <gtkmm.h>
#include "armyset.h"
#include "lw-editor-dialog.h"
#include "undo-mgr.h"

//! Armyset Editor.  Edit the description of the Armyset.
class ArmySetInfoDialog: public LwEditorDialog
{
 public:
    ArmySetInfoDialog(Gtk::Window &parent, Armyset *armyset);
    ~ArmySetInfoDialog();

    bool run();

    Glib::ustring getName () const {return d_name;}
    Glib::ustring getDescription () const {return d_description;}
    Glib::ustring getCopyright () const {return d_copyright;}
    Glib::ustring getLicense () const {return d_license;}
    guint32 getTileSize () const {return d_tilesize;}
 private:
    Armyset *d_armyset;
    UndoMgr *umgr;
    bool d_changed;
    Glib::ustring d_name;
    Glib::ustring d_description;
    Glib::ustring d_copyright;
    Glib::ustring d_license;
    guint32 d_tilesize;
    Glib::ustring d_orig_name;
    Glib::ustring d_orig_description;
    Glib::ustring d_orig_copyright;
    Glib::ustring d_orig_license;
    guint32 d_orig_tilesize;
    Gtk::Entry *name_entry;
    Gtk::TextView *copyright_textview;
    Gtk::TextView *license_textview;
    Gtk::Button *close_button;
    Gtk::Label *status_label;
    Gtk::TextView *description_textview;
    Gtk::Label *location_label;
    Gtk::Notebook *notebook;
    Gtk::Button *fit_button;
    Gtk::SpinButton *size_spinbutton;
    Gtk::Button *undo_button;
    Gtk::Button *redo_button;
    Gtk::Label *armies_label;
    Gtk::Label *selectors_label;
    Gtk::Label *bag_label;
    Gtk::Label *ship_label;
    Gtk::Label *flag_label;
    Gtk::Label *images_label;

    void on_name_changed();
    void on_copyright_changed ();
    void on_license_changed ();
    void on_description_changed ();
    void on_size_changed();
    void on_fit_pressed();
    void connect_signals ();
    void disconnect_signals ();
    std::list<sigc::connection> connections;
    void update ();
    UndoAction* executeAction (UndoAction* action);
    void on_undo_activated ();
    void on_redo_activated ();
    void update_name ();
};

#endif
