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
#ifndef CITY_EDITOR_DIALOG_H
#define CITY_EDITOR_DIALOG_H

#include <gtkmm.h>
#include "lw-editor-dialog.h"
#include "undo-mgr.h"
#include "city-editor-actions.h"

class CreateScenarioRandomize;
class City;
class ArmyProdBase;
class Player;

//! Scenario editor.  Edits a City object.
class CityEditorDialog: public LwEditorDialog
{
 public:
    CityEditorDialog(Gtk::Window &parent, City *city, CreateScenarioRandomize *randomizer);
    ~CityEditorDialog();

    bool run();

 private:
    City *city;
    UndoMgr *umgr;
    bool d_changed;
    CreateScenarioRandomize *d_randomizer;
    Gtk::ComboBoxText *player_combobox;
    Gtk::Notebook *notebook;
    Gtk::Switch *capital_switch;
    Gtk::Entry *name_entry;
    Gtk::SpinButton *income_spinbutton;
    Gtk::Switch *burned_switch;
    Gtk::Switch *build_production_switch;
    Gtk::TextView *description_textview;

    Gtk::TreeView *army_treeview;

    class ArmyColumns: public Gtk::TreeModelColumnRecord {
    public:
	ArmyColumns()
	    { add(army); add(image);
	      add(strength); add(moves); add(upkeep); add(duration); add(name);}

	Gtk::TreeModelColumn<const ArmyProdBase *> army;
	Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > image;
	Gtk::TreeModelColumn<int> strength, moves, upkeep, duration;
	Gtk::TreeModelColumn<Glib::ustring> name;
    };
    const ArmyColumns army_columns;
    Glib::RefPtr<Gtk::ListStore> army_list;
    Gtk::Button *add_button;
    Gtk::Button *remove_button;
    Gtk::Button *randomize_armies_button;
    Gtk::Button *randomize_name_button;
    Gtk::Button *randomize_income_button;
    Gtk::CellRendererSpin strength_renderer;
    Gtk::TreeViewColumn strength_column;
    Gtk::CellRendererSpin moves_renderer;
    Gtk::TreeViewColumn moves_column;
    Gtk::CellRendererSpin duration_renderer;
    Gtk::TreeViewColumn duration_column;
    Gtk::CellRendererSpin upkeep_renderer;
    Gtk::TreeViewColumn upkeep_column;
    Gtk::Button *undo_button;
    Gtk::Button *redo_button;


    void on_add_clicked();
    void on_remove_clicked();
    void on_randomize_armies_clicked();
    void on_randomize_name_clicked();
    void on_randomize_income_clicked();
    void on_selection_changed();
    void on_player_changed();
    Player *get_selected_player();
    void change_city_ownership(Player *p);

    void add_army(const ArmyProdBase *a);
    void set_button_sensitivity();
    void cell_data_strength(Gtk::CellRenderer *renderer, const Gtk::TreeIter& i);
    void on_strength_edited(const Glib::ustring &path, const Glib::ustring &new_text);
    void cell_data_moves(Gtk::CellRenderer *renderer, const Gtk::TreeIter& i);
    void on_moves_edited(const Glib::ustring &path, const Glib::ustring &new_text);
    void cell_data_turns(Gtk::CellRenderer *renderer, const Gtk::TreeIter& i);
    void on_turns_edited(const Glib::ustring &path, const Glib::ustring &new_text);
    void cell_data_upkeep(Gtk::CellRenderer *renderer, const Gtk::TreeIter& i);
    void on_upkeep_edited(const Glib::ustring &path, const Glib::ustring &new_text);
    void update_buttons();
    void on_burned_changed ();
    void on_capital_changed ();
    void on_name_changed ();
    void on_income_text_changed ();
    void on_build_production_changed ();
    void on_description_changed ();

    void update_armies ();

    void on_undo_activated ();
    void on_redo_activated ();
    void update ();
    void connect_signals ();
    void disconnect_signals ();
    std::list<sigc::connection> connections;
    UndoAction *executeAction (UndoAction *action);
    const ArmyProdBase* getProdBaseByIndex (CityEditorAction_Index *a);
    int getCurIndex ();
    void replaceProdBase (int i, ArmyProdBase *a);
    void replaceProdBases (CityEditorAction_City *a);
    void fill_armies ();
    void clear_armies ();
};

#endif
