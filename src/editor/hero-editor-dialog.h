//  Copyright (C) 2009, 2014, 2020 Ben Asselstine
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
#ifndef HERO_EDITOR_DIALOG_H
#define HERO_EDITOR_DIALOG_H

#include <gtkmm.h>
#include "lw-editor-dialog.h"

class Hero;

//! Scenario editor.  Change the attributes of a hero.
class HeroEditorDialog: public LwEditorDialog
{
 public:
    HeroEditorDialog(Gtk::Window &parent, Hero *hero);
    ~HeroEditorDialog() {}

    int run();
    
 private:
    Hero *d_hero;
    Gtk::Entry *name_entry;
    Gtk::ComboBox *gender_combobox;
    Gtk::Button *edit_backpack_button;
	
    void on_edit_backpack_clicked ();
    void on_name_changed ();
    void on_gender_changed ();
};

#endif
