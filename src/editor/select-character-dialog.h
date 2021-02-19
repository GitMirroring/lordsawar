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
#ifndef SELECT_CHARACTER_DIALOG_H
#define SELECT_CHARACTER_DIALOG_H

#include <gtkmm.h>
#include "lw-editor-dialog.h"
class Character;
class Player;

//! Scenario editor.  Select a hero character
class SelectCharacterDialog: public LwEditorDialog
{
 public:
    SelectCharacterDialog(Gtk::Window &parent, Player *p, guint32 id);
    ~SelectCharacterDialog() {}

    bool run();
    int get_selected_id () {return selected_character;}
    
 private:
    Gtk::Button *select_button;

    Player *d_player;
    int selected_character;

    Gtk::TreeView *characters_treeview;
    class CharactersColumns: public Gtk::TreeModelColumnRecord {
    public:
	CharactersColumns() 
        { add(name); add(id);}
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<guint32> id;
    };
    const CharactersColumns characters_columns;
    Glib::RefPtr<Gtk::ListStore> characters_list;

    void addCharacter(Character *c);
    
    void set_select_button_state();
};

#endif
