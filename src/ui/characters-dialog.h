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
#ifndef CHARACTERS_DIALOG_H
#define CHARACTERS_DIALOG_H

#include <gtkmm.h>

#include "lw-dialog.h"
#include "character.h"
class HeroProto;

// dialog for showing the heroes that players can have
class CharactersDialog: public LwDialog
{
 public:
    CharactersDialog(Gtk::Window &parent, std::list<Gtk::Image*> shields, Glib::ustring filename);
    ~CharactersDialog();

 private:
    Gtk::TreeView *heroes_treeview;
    Gtk::Box *button_box;

    class HeroesColumns: public Gtk::TreeModelColumnRecord {
    public:
	HeroesColumns() 
        { add(name); add(desc); }
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<Glib::ustring> desc;
    };
    const HeroesColumns heroes_columns;
    Glib::RefPtr<Gtk::ListStore> heroes_list;
 private:
    void addHero(Character *c);

    void on_button_toggled (guint32 player_id);
    void fill_heroes (guint32 player_id);
    std::list<Character*> d_characters;
};

#endif
