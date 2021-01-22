//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014, 2020, 2021 Ben Asselstine
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
#ifndef SIGNPOST_EDITOR_DIALOG_H
#define SIGNPOST_EDITOR_DIALOG_H

#include <gtkmm.h>

#include "lw-editor-dialog.h"

class Signpost;
class CreateScenarioRandomize;

//! Scenario editor.  Change the contents of a signpost.
class SignpostEditorDialog: public LwEditorDialog
{
 public:
    SignpostEditorDialog(Gtk::Window &parent, Signpost *signpost, CreateScenarioRandomize *randomizer);
    ~SignpostEditorDialog() {}

    bool run();
    
 private:
    Gtk::TextView *sign_textview;
    Signpost *signpost;
    bool d_changed;
    Gtk::Button *randomize_button;
    CreateScenarioRandomize *d_randomizer;
    
    void on_randomize_clicked();
    void on_sign_changed ();
};

#endif
