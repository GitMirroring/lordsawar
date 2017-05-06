//  Copyright (C) 2017 Ben Asselstine
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
#ifndef BATTLE_CALCULATOR_DIALOG_H
#define BATTLE_CALCULATOR_DIALOG_H

#include <gtkmm.h>
#include <list>
#include "lw-editor-dialog.h"
#include "fight.h"

class Stack;
class Army;
class Player;

//! Scenario editor.  Simulate a battle and see the results.
class BattleCalculatorDialog: public LwEditorDialog
{
 public:
    BattleCalculatorDialog(Gtk::Window &parent);
    ~BattleCalculatorDialog() {};

    int run();
    
 private:
    Gtk::ComboBoxText *terrain_combobox;
    Gtk::ComboBoxText *attacker_player_combobox;
    Gtk::ComboBoxText *defender_player_combobox;
    Gtk::ComboBox *die_sides_combobox;

    Gtk::TreeView *attackers_treeview;
    Gtk::TreeView *defenders_treeview;
    
    class CombatantColumns: public Gtk::TreeModelColumnRecord {
    public:
	CombatantColumns()
	    { add(army); add(image); add(strength); add(augmented_strength);
              add(hp); }

	Gtk::TreeModelColumn<Army *> army;
	Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > image;
	Gtk::TreeModelColumn<int> strength, augmented_strength, hp;
    };
    const CombatantColumns combatant_columns;
    Glib::RefPtr<Gtk::ListStore> attackers_list;
    Glib::RefPtr<Gtk::ListStore> defenders_list;

    Gtk::CellRendererSpin attacker_strength_renderer;
    Gtk::TreeViewColumn attacker_strength_column;
    Gtk::CellRendererSpin defender_strength_renderer;
    Gtk::TreeViewColumn defender_strength_column;
    Gtk::Button *attacker_add_button;
    Gtk::Button *attacker_remove_button;
    Gtk::Button *attacker_copy_button;
    Gtk::Button *attacker_edit_hero_button;
    Gtk::Button *defender_add_button;
    Gtk::Button *defender_remove_button;
    Gtk::Button *defender_copy_button;
    Gtk::Button *defender_edit_hero_button;
    Gtk::Switch *fortified_switch;
    Gtk::Switch *city_switch;
    Gtk::Box *terrain_box;

    Gtk::Button *fight_button;
    Gtk::Button *fight100_button;
    int min_size;

    void on_attacker_add_clicked();
    void on_attacker_remove_clicked();
    void on_attacker_copy_clicked();
    void on_attacker_edit_hero_clicked();
    void on_attacker_selection_changed();
    void on_defender_add_clicked();
    void on_defender_remove_clicked();
    void on_defender_copy_clicked();
    void on_defender_edit_hero_clicked();
    void on_defender_selection_changed();
    void on_fortified_toggled();
    void on_attacker_player_changed();
    void on_defender_player_changed();

    void add_attacker_army(Army *a);
    void add_defender_army(Army *a);
    void set_button_sensitivity();
    void cell_data_attacker_strength(Gtk::CellRenderer *renderer, const Gtk::TreeIter& i);
    void on_attacker_strength_edited(const Glib::ustring &path, const Glib::ustring &new_text);
    void cell_data_defender_strength(Gtk::CellRenderer *renderer, const Gtk::TreeIter& i);
    void on_defender_strength_edited(const Glib::ustring &path, const Glib::ustring &new_text);
    void on_city_toggled();
    void on_fight_clicked();
    void on_fight100_clicked();

    Fight::Result run_battle ();

    Player *get_attacker_player();
    Player *get_defender_player();
};

#endif
