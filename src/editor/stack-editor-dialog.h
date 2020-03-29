//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014 Ben Asselstine
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
#ifndef STACK_EDITOR_DIALOG_H
#define STACK_EDITOR_DIALOG_H

#include <gtkmm.h>
#include "lw-editor-dialog.h"

class Stack;
class Army;
class Player;

//! Scenario editor.  Change the contents of a Stack.
class StackEditorDialog: public LwEditorDialog
{
 public:
    StackEditorDialog(Gtk::Window &parent, Stack *stack, int min_size = 1);
    ~StackEditorDialog() {}

    int run();
    
 private:
    Gtk::ComboBoxText *player_combobox;

    Gtk::TreeView *army_treeview;
    
    class ArmyColumns: public Gtk::TreeModelColumnRecord {
    public:
	ArmyColumns()
	    { add(army); add(image); add(strength); add(moves); add(upkeep); 
	    add(name);}

	Gtk::TreeModelColumn<Army *> army;
	Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > image;
	Gtk::TreeModelColumn<int> strength, moves, upkeep;
	Gtk::TreeModelColumn<Glib::ustring> name;
    };
    const ArmyColumns army_columns;
    Glib::RefPtr<Gtk::ListStore> army_list;

    Gtk::CellRendererSpin strength_renderer;
    Gtk::TreeViewColumn strength_column;
    Gtk::CellRendererSpin moves_renderer;
    Gtk::TreeViewColumn moves_column;
    Gtk::CellRendererSpin upkeep_renderer;
    Gtk::TreeViewColumn upkeep_column;
    Gtk::Button *add_button;
    Gtk::Button *remove_button;
    Gtk::Button *copy_button;
    Gtk::Button *edit_hero_button;
    Gtk::Switch *fortified_switch;

    Stack *stack;
    int min_size;

    void on_add_clicked();
    void on_remove_clicked();
    void on_copy_clicked();
    void on_edit_hero_clicked();
    void on_selection_changed();
    void on_fortified_toggled();
    void on_player_changed();

    void add_army(Army *a);
    void set_button_sensitivity();
    void cell_data_strength(Gtk::CellRenderer *renderer, const Gtk::TreeIter& i);
    void on_strength_edited(const Glib::ustring &path, const Glib::ustring &new_text);
    void cell_data_moves(Gtk::CellRenderer *renderer, const Gtk::TreeIter& i);
    void on_moves_edited(const Glib::ustring &path, const Glib::ustring &new_text);
    void cell_data_upkeep(Gtk::CellRenderer *renderer, const Gtk::TreeIter& i);
    void on_upkeep_edited(const Glib::ustring &path, const Glib::ustring &new_text);

    Player *get_selected_player();
};

#endif
