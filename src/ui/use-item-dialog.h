//  Copyright (C) 2010, 2014 Ben Asselstine
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
#ifndef USE_ITEM_DIALOG_H
#define USE_ITEM_DIALOG_H

#include <memory>
#include <vector>
#include <gtkmm.h>

#include "lw-dialog.h"
class Item;

//! Scenario editor.  Select an ItemProto object from the Itemlist.
class UseItemDialog: public LwDialog
{
 public:
    UseItemDialog(Gtk::Window &parent, std::list<Item*> items);
    ~UseItemDialog() {};

    Item *get_selected_item() { return selected_item; }

    void run();
    void hide() {dialog->hide();};
    
 private:
    Gtk::Button *select_button;

    Item *selected_item;

    Gtk::TreeView *items_treeview;
    class ItemsColumns: public Gtk::TreeModelColumnRecord {
    public:
	ItemsColumns() 
        { add(name); add(attributes);add(item);}
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<Glib::ustring> attributes;
	Gtk::TreeModelColumn<Item *> item;
    };
    const ItemsColumns items_columns;
    Glib::RefPtr<Gtk::ListStore> items_list;

    void addItem(Item *item);
    void set_select_button_state();
};

#endif
