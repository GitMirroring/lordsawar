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
#ifndef ITEM_BONUS_DIALOG_H
#define ITEM_BONUS_DIALOG_H

#include <gtkmm.h>
#include <list>

#include "lw-dialog.h"

class ItemProto;

// dialog for showing the bonuses that items have
class ItemBonusDialog: public LwDialog
{
 public:
    ItemBonusDialog(Gtk::Window &parent);
    ~ItemBonusDialog() {};

 private:
    Gtk::TreeView *items_treeview;

    class ItemsColumns: public Gtk::TreeModelColumnRecord {
    public:
	ItemsColumns() 
        { add(name); add(bonus);}
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<Glib::ustring> bonus;
    };
    const ItemsColumns items_columns;
    Glib::RefPtr<Gtk::ListStore> items_list;
 private:
    void addItemProto(ItemProto *itemproto);
};

#endif
