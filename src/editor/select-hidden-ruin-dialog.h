//  Copyright (C) 2008, 2009, 2014 Ben Asselstine
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
#ifndef SELECT_HIDDEN_RUIN_DIALOG_H
#define SELECT_HIDDEN_RUIN_DIALOG_H

#include <gtkmm.h>
#include "lw-editor-dialog.h"

class Ruin;

//! Scenario editor.  Select a hidden Ruin object in the scenario.
class SelectHiddenRuinDialog: public LwEditorDialog
{
 public:
    SelectHiddenRuinDialog(Gtk::Window &parent);
    ~SelectHiddenRuinDialog() {}

    void run();
    const Ruin *get_selected_hidden_ruin() 
      { return selected_hidden_ruin; }
    
 private:
    Gtk::Button *select_button;

    const Ruin *selected_hidden_ruin;

    Gtk::TreeView *hidden_ruins_treeview;
    class HiddenRuinsColumns: public Gtk::TreeModelColumnRecord {
    public:
	HiddenRuinsColumns() 
        { add(name); add(ruin);}
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<Ruin *> ruin;
    };
    const HiddenRuinsColumns hidden_ruins_columns;
    Glib::RefPtr<Gtk::ListStore> hidden_ruins_list;

    void addHiddenRuin(Ruin *ruin);
    
    void set_select_button_state();
};

#endif
