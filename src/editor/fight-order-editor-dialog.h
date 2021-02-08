//  Copyright (C) 2015, 2021 Ben Asselstine
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
#ifndef FIGHT_ORDER_EDITOR_DIALOG_H
#define FIGHT_ORDER_EDITOR_DIALOG_H

#include <gtkmm.h>
#include <list>

#include "undo-mgr.h"
#include "lw-editor-dialog.h"
class Stack;
class Player;

// edit the default fight order of the armies controlled by every player
class FightOrderEditorDialog: public LwEditorDialog
{
 public:
    FightOrderEditorDialog(Gtk::Window &parent);
    ~FightOrderEditorDialog();

    void hide();
    bool run();

 private:
    UndoMgr *umgr;
    Gtk::TreeView *armies_treeview;
    Gtk::Button *make_same_button;
    Gtk::ComboBoxText *player_combobox;
    Gtk::Button *undo_button;
    Gtk::Button *redo_button;
    int owner_row;

    class ArmiesColumns: public Gtk::TreeModelColumnRecord {
    public:
	ArmiesColumns() 
        { add(image); add(name); add(army_type);}
	
	Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > image;
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<guint32> army_type;
    };
    const ArmiesColumns armies_columns;
    Glib::RefPtr<Gtk::ListStore> armies_list;
    bool d_changed;

    void addArmyType(guint32 army_type, Player *player);
    void on_make_same_button_clicked();
    void on_player_changed();
    Player* get_selected_player();
    void fill_armies(Player *player);
    void on_army_reordered ();
    void on_undo_activated ();
    void on_redo_activated ();
    void disconnect_signals ();
    void connect_signals ();
    std::list<sigc::connection> connections;
    void update ();
    UndoAction *executeAction (UndoAction *action);
    std::list<guint32> get_fight_order (guint32 id);
    std::list<std::list<guint32> >get_all_fight_orders ();
};

#endif
