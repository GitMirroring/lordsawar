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
#ifndef HEROES_DIALOG_H
#define HEROES_DIALOG_H

#include <gtkmm.h>
#include <list>

#include "lw-editor-dialog.h"

class HeroProto;

// edit the heroes that can join a player
class HeroesDialog: public LwEditorDialog
{
 public:
    HeroesDialog(Gtk::Window &parent, guint32 player_id, Glib::ustring player_name);
    ~HeroesDialog();

    bool run();

 private:
    class HeroesColumns: public Gtk::TreeModelColumnRecord {
    public:
	HeroesColumns()
	    { add(name); add(gender); add(hero); }
	
	Gtk::TreeModelColumn<Glib::ustring> name, gender;
	Gtk::TreeModelColumn<HeroProto*> hero;
    };
    const HeroesColumns hero_columns;
    Glib::RefPtr<Gtk::ListStore> hero_list;

    Gtk::CellRendererCombo gender_renderer;
    Gtk::TreeViewColumn gender_column;
    Gtk::CellRendererText name_renderer;
    Gtk::TreeViewColumn name_column;

    class HeroGenderColumns: public Gtk::TreeModelColumnRecord {
    public:
	HeroGenderColumns()
	    { add(gender); }
	
	Gtk::TreeModelColumn<Glib::ustring> gender;
    };
    const HeroGenderColumns hero_gender_columns;
    Glib::RefPtr<Gtk::ListStore> hero_gender_list;

    void cell_data_gender(Gtk::CellRenderer *renderer, const Gtk::TreeIter &i);
    void on_gender_edited(const Glib::ustring &path,
                          const Glib::ustring &new_text);
    void cell_data_name(Gtk::CellRenderer *renderer, const Gtk::TreeIter& i);
    void on_name_edited(const Glib::ustring &path, const Glib::ustring &new_text);

    bool d_changed;
    bool d_player_id;
    Gtk::TreeView *treeview;
    Gtk::Button *add_button;
    Gtk::Button *remove_button;

    void on_add_pressed();
    void on_remove_pressed();

    void fill_heroes ();
    void update_buttons ();

    HeroProto * get_selected_hero ();
    void on_hero_selected ();
    void update_hero_templates ();
};

#endif
