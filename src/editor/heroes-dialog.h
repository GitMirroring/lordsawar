//  Copyright (C) 2020, 2021 Ben Asselstine
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
#include "undo-mgr.h"
#include "heroes-editor-actions.h"

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
	    { add(name); add(hero); }
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<HeroProto*> hero;
    };
    const HeroesColumns hero_columns;
    Glib::RefPtr<Gtk::ListStore> hero_list;

    Gtk::CellRendererText name_renderer;
    Gtk::TreeViewColumn name_column;

    void cell_data_name(Gtk::CellRenderer *renderer, const Gtk::TreeIter& i);

    UndoMgr *umgr;
    bool d_changed;
    bool d_player_id;
    Gtk::TreeView *treeview;
    Gtk::Button *add_button;
    Gtk::Button *remove_button;
    Gtk::Button *undo_button;
    Gtk::Button *redo_button;

    Gtk::Entry *name_entry;
    Gtk::ComboBoxText *gender_combobox;
    Gtk::Box *panel_box;

    void on_name_changed ();
    void on_gender_changed ();
    void on_add_pressed();
    void on_remove_pressed();

    void fill_heroes ();
    void update_buttons ();

    HeroProto * get_selected_hero ();
    void on_hero_selected ();
    void update_hero_templates ();
    void update_panel ();
    void connect_signals ();
    void disconnect_signals ();
    std::vector<sigc::connection> connections;
    void on_undo_activated ();
    void on_redo_activated ();
    void update ();
    UndoAction *executeAction (UndoAction *action);
    void clear_heroes();
    HeroProto* getHeroByIndex (HeroesEditorAction_Index *a);
    int getCurIndex ();
};

#endif
