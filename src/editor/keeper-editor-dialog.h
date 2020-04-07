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
#ifndef KEEPER_EDITOR_DIALOG_H
#define KEEPER_EDITOR_DIALOG_H

#include <gtkmm.h>
#include "lw-editor-dialog.h"
#include "vector.h"

class Keeper;
class ArmyChooserButton;
class ArmyProto;
class CreateScenarioRandomize;

//! Scenario editor.  Edits the defender of a ruin.
class KeeperEditorDialog: public LwEditorDialog
{
 public:
    KeeperEditorDialog(Gtk::Window &parent, Keeper *k, Vector<int> pos,
                       CreateScenarioRandomize *randomize);
    ~KeeperEditorDialog();

    bool run();

    Keeper *get_keeper () {return d_keeper;}
    
 private:
    bool d_changed;
    Vector<int> d_pos;
    Keeper *d_keeper;
    CreateScenarioRandomize *d_randomizer;
    ArmyChooserButton *keeper_button;
    Gtk::Button *randomize_button;
    Gtk::Entry *name_entry;

    void on_randomize_clicked();
    void on_name_changed ();
    void fill_in_keeper_info ();

    void on_keeper_selected (const ArmyProto *a);
};

#endif
