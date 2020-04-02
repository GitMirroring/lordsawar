//  Copyright (C) 2017, 2020 Ben Asselstine
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
#ifndef STONE_EDITOR_DIALOG_H
#define STONE_EDITOR_DIALOG_H

#include <gtkmm.h>
#include "lw-editor-dialog.h"
#include "stone.h"

class Road;
class PixMask;

// dialog for changing the type of a standing stone
class StoneEditorDialog: public LwEditorDialog
{
 public:
    StoneEditorDialog(Gtk::Window &parent, Stone *stone, Road *road);
    ~StoneEditorDialog();

    bool run();
    void hide() {dialog->hide();}

    int get_selected_type () const;
    
 private:
    bool d_changed;
    Gtk::Grid *grid;
    Road *d_road;
    Stone *d_stone;
    int selected_type;

    std::vector<Gtk::ToggleButton *> type_toggles;
    bool ignore_toggles;
    std::vector<Stone::Type> types;

    void on_type_toggled(Gtk::ToggleButton *toggle);
    void fill_pixbuf (int i);
    int lookup_slot (Gtk::ToggleButton *toggle);
    PixMask *get_grass_image();
};

#endif
