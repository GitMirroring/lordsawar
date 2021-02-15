//  Copyright (C) 2008, 2009, 2011, 2014 Ben Asselstine
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
#ifndef STACK_INFO_DIALOG_H
#define STACK_INFO_DIALOG_H

#include <gtkmm.h>
#include <list>

#include "army-info-tip.h"

class Stack;
class Army;

#include "vector.h"
#include "lw-dialog.h"

// dialog for showing hero information
class StackInfoDialog: public LwDialog
{
 public:
    StackInfoDialog(Gtk::Window &parent, Vector<int> pos);
    ~StackInfoDialog();

    Stack * get_selected_stack() {return currently_selected_stack;};

 private:
    Vector<int> tile;
    Gtk::Grid *stack_table;

    ArmyInfoTip* army_info_tip;
    std::vector<Gtk::ToggleButton *> toggles;
    std::vector<const Army*> armies;
    std::vector<Gtk::RadioButton *> radios;
    Gtk::Button *group_button;
    Gtk::Button *ungroup_button;
    Stack *currently_selected_stack;

    void addArmy (bool first, Stack *s, Army *a, guint32 modified_strength ,guint32 idx, guint32 color_id);
    void addStack(Stack *s, guint32 &idx);
    void on_group_clicked();
    void on_ungroup_clicked();
    void fill_stack_info();
    void on_stack_toggled(Gtk::RadioButton *radio, Stack *s);
    void on_army_toggled(Gtk::ToggleButton *toggle, Stack *s, Army *a);
    bool on_army_button_event(GdkEventButton *e, Gtk::ToggleButton *toggle);
};

#endif
