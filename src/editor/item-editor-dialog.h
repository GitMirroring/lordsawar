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
#ifndef ITEM_EDITOR_DIALOG_H
#define ITEM_EDITOR_DIALOG_H

#include <gtkmm.h>
#include "lw-editor-dialog.h"
#include "vector.h"
#include "undo-mgr.h"

class Item;
// dialog for showing info about an instantiated item
class ItemEditorDialog: public LwEditorDialog
{
 public:
    ItemEditorDialog(Gtk::Window &parent, Item *item);
    ~ItemEditorDialog();

    bool run();
    void hide();
    
 private:

    UndoMgr *umgr;
    bool d_changed;
    Item *d_item;
    Gtk::Entry *name_entry;
    Gtk::SpinButton *uses_spinbutton;
    Gtk::Label *bonus_label;
    Gtk::Button *undo_button;
    Gtk::Button *redo_button;

    void on_name_changed ();
    void on_uses_changed ();
    void on_uses_text_changed();
    void on_undo_activated ();
    void on_redo_activated ();
    void connect_signals ();
    void disconnect_signals ();
    std::list<sigc::connection> connections;
    void update ();
    UndoAction *executeAction (UndoAction *action);
};

#endif
