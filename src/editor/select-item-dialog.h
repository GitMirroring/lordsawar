//  Copyright (C) 2008, 2009, 2011, 2014, 2020 Ben Asselstine
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
#ifndef SELECT_ITEM_DIALOG_H
#define SELECT_ITEM_DIALOG_H

#include <gtkmm.h>
#include "lw-editor-dialog.h"

class ItemProto;

//! Scenario editor.  Select an ItemProto object from the Itemlist.
class SelectItemDialog: public LwEditorDialog
{
 public:
    SelectItemDialog(Gtk::Window &parent, bool clear = false);
    ~SelectItemDialog() {}

    void run();
    const ItemProto *get_selected_item(guint32 &id) { id = selected_item_type_id; return selected_item; }
    
 private:
    bool d_clear;
    Gtk::Button *select_button;
    Gtk::Button *clear_button;

    const ItemProto *selected_item;
    guint32 selected_item_type_id;

    Gtk::TreeView *items_treeview;
    class ItemsColumns: public Gtk::TreeModelColumnRecord {
    public:
	ItemsColumns() 
        { add(name); add(attributes);add(item);}
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<Glib::ustring> attributes;
	Gtk::TreeModelColumn<ItemProto *> item;
    };
    const ItemsColumns items_columns;
    Glib::RefPtr<Gtk::ListStore> items_list;


    void addItemProto(ItemProto *item);
    
    void set_select_button_state();
};

#endif
