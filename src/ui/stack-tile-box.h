//  Copyright (C) 2011, 2014, 2015, 2020 Ben Asselstine
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
#ifndef STACK_TILE_BOX_H
#define STACK_TILE_BOX_H

#include <sigc++/trackable.h>
#include <gtkmm.h>
#include <glibmm.h>
#include "Configuration.h"

class StackTile;
class Stack;
class ArmyInfoTip;
class Army;
class StackArmyButton;

// shows the listing of army units that coexist on a single map tile.
class StackTileBox: public Gtk::Box
{
 public:
     //! Constructor for building this object with gtk::builder
    StackTileBox(BaseObjectType* base, const Glib::RefPtr<Gtk::Builder> &xml);

    //!Destructor.
    ~StackTileBox();

    static StackTileBox * create();
    void on_stack_info_changed(Stack *s);
    Stack * get_currently_selected_stack() const {return currently_selected_stack;};
    void show_stack(StackTile *s);
    void clear_selected_stack() {currently_selected_stack = NULL;};
    void set_selected_stack(Stack*s) {currently_selected_stack =s;};
    void toggle_group_ungroup();

    //! Signals
    sigc::signal<void, Stack*> stack_composition_modified;
    sigc::signal<void, bool> stack_tile_group_toggle;
 protected:

 private:
    bool d_inhibit;
    Stack *currently_selected_stack;
    ArmyInfoTip *army_info_tip;
    typedef std::vector<StackArmyButton *> stack_army_buttons_type;
    stack_army_buttons_type stack_army_buttons;
    Gtk::Box *stack_info_box;
    Gtk::Box *stack_info_container;
    Gtk::Label *group_moves_label;
    Gtk::Image *terrain_image;
    Gtk::ToggleButton *group_ungroup_toggle;
    bool d_inhibit_group_toggle;

    sigc::connection army_conn[MAX_ARMIES_ON_A_SINGLE_TILE];
    sigc::connection stack_conn[MAX_ARMIES_ON_A_SINGLE_TILE];

    void pad_image(Gtk::Image *image);
    void fill_in_group_info (StackTile *stile, Stack *s);
    void on_army_toggled(StackArmyButton *toggle, Stack *stack, Army *army);
    void on_stack_toggled(Stack *stack);
    void on_group_toggled(Gtk::ToggleButton *toggle);
    void reset_army_buttons();

};

#endif // STACK_TILE_BOX
