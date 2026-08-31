//  Copyright (C) 2010, 2012, 2014 Ben Asselstine
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
#ifndef ITEM_REPORT_DIALOG_H
#define ITEM_REPORT_DIALOG_H

#include <gtkmm.h>

#include "itemmap.h"
#include "player.h"
#include "lw-dialog.h"

class Stack;
class MapBackpack;
// dialog for showing where the items are
class ItemReportDialog: public LwDialog
{
 public:
    ItemReportDialog(Gtk::Window &parent,
                     const std::list<Stack*> &item_laden_stacks,
                     const std::list<MapBackpack*> &bags);
    ~ItemReportDialog() {delete itemmap;};

    void hide() {dialog->hide();};
    void run();
    
 private:
    std::list<Stack*> stacks;
    std::list<MapBackpack*> bags;
    ItemMap* itemmap;

    Gtk::Image *map_image;
    
    void on_map_changed(Cairo::RefPtr<Cairo::Surface> map);
    void fill_in_item_info();
    Gtk::Label *label;

};

#endif
