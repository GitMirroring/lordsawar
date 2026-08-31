//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014, 2015, 2020, 2021 Ben Asselstine
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
#ifndef LOAD_SCENARIOS_DIALOG_H
#define LOAD_SCENARIOS_DIALOG_H

#include <gtkmm.h>

#include "lw-dialog.h"

class ScenarioDetails;

// dialog for choosing a scenario
class LoadScenarioDialog: public LwDialog
{
 public:
    LoadScenarioDialog(Gtk::Window &parent);
    ~LoadScenarioDialog() {};

    void run();
    void hide() {dialog->hide();};

    Glib::ustring get_scenario_filename();

 private:
    Gtk::Button *load_button;
    Gtk::TextView *description_textview;
    Gtk::Label *num_players_label;
    Gtk::Label *num_cities_label;
    Gtk::TreeView *scenarios_treeview;
    Gtk::Button *add_scenario_button;
    Gtk::Button *remove_scenario_button;

    class ScenariosColumns: public Gtk::TreeModelColumnRecord {
    public:
	ScenariosColumns() 
        { add(filename); add (details);}
	
	Gtk::TreeModelColumn<Glib::ustring> filename;
	Gtk::TreeModelColumn<ScenarioDetails *> details;

    };
    const ScenariosColumns scenarios_columns;
    Glib::RefPtr<Gtk::ListStore> scenarios_list;
    
    Gtk::CellRendererText name_renderer;
    Gtk::TreeViewColumn name_column;

    Glib::ustring selected_filename;
    
    void on_selection_changed();
    void add_scenario(ScenarioDetails *d, bool sel);
    void on_add_scenario_clicked();
    void on_remove_scenario_clicked();
    int copy_file (Glib::ustring from, Glib::ustring to);
    void on_scenario_activated();

    Gtk::Box *dialog_vbox;
    Gtk::TreeView *progress_treeview;
    class ProgressModelColumns : public Gtk::TreeModel::ColumnRecord
      {
    public:
        ProgressModelColumns ()
          { add (perc);}
        Gtk::TreeModelColumn<int> perc;
      };
    ProgressModelColumns progress_columns;
    Glib::RefPtr<Gtk::ListStore> progress_liststore;
    Gtk::TreeModel::Row progressrow;
    void setup_progress_bar ();
    void finish_progress ();
    void tick_progress ();
    void cell_data_name(Gtk::CellRenderer *renderer, const Gtk::TreeIter &i);
};

#endif
