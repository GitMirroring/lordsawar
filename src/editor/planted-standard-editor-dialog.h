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
#ifndef PLANTED_STANDARD_EDITOR_DIALOG_H
#define PLANTED_STANDARD_EDITOR_DIALOG_H

#include <gtkmm.h>
#include "lw-editor-dialog.h"
#include "vector.h"

class MapBackpack;
class Item;
// dialog for showing info about a hero's planted flag
class PlantedStandardEditorDialog: public LwEditorDialog
{
 public:
    PlantedStandardEditorDialog(Gtk::Window &parent, Vector<int> tile);
    ~PlantedStandardEditorDialog() {}

    bool run();
    void hide();
    
 private:

    bool d_changed;
    Item *d_item;
    MapBackpack *d_map_backpack;
    Gtk::ComboBoxText *owner_combobox;
    Gtk::ComboBoxText *orig_owner_combobox;
    Gtk::Entry *name_entry;
    Gtk::Label *bonus_label;

    void on_owner_changed ();
    void on_orig_owner_changed ();
    void on_name_changed ();
};

#endif
