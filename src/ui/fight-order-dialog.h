//  Copyright (C) 2007, 2008, 2009, 2011, 2014 Ben Asselstine
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
#ifndef FIGHT_ORDER_DIALOG_H
#define FIGHT_ORDER_DIALOG_H

#include <gtkmm.h>
#include <list>

#include "lw-dialog.h"
class Stack;
class Player;

// dialog for showing and changing the order in which army types fight
// in battle
class FightOrderDialog: public LwDialog
{
 public:
    FightOrderDialog(Gtk::Window &parent, Player *player);
    ~FightOrderDialog() {};

    void hide() {dialog->hide();};
    void run();

 private:
    Player *player;
    Gtk::TreeView *armies_treeview;
    Gtk::Button *reverse_button;
    Gtk::Button *reset_button;

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
 private:
    void addArmyType(guint32 army_type);
    void on_reverse_button_clicked();
    void on_reset_button_clicked();
};

#endif
