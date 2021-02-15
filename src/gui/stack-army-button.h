//  Copyright (C) 2011, 2015, 2020 Ben Asselstine
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
#ifndef STACK_ARMY_BUTTON_H
#define STACK_ARMY_BUTTON_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm.h>
#include <glibmm.h>
#include "Configuration.h"

class ArmyInfoTip;
class Army;
class Stack;
// a button-pair.  shows an army button, and maybe another button for the stack.
class StackArmyButton: public Gtk::Box
{
 public:
     //! Constructor for building this object with gtk::builder
    StackArmyButton(BaseObjectType* base, const Glib::RefPtr<Gtk::Builder> &xml);

    //!Destructor.
    ~StackArmyButton();

    bool get_active() const { return army_button->get_active();}
    void update_stack_button(bool selected);
    void reset(); //go back to an empty disabled, untoggled button with the circle
    void draw(Stack *s, Army *a, guint32 circle_color_id, bool toggled);

    //Signals
    sigc::signal<void> stack_clicked;
    sigc::signal<void> army_toggled;
    
    //Statics
    static StackArmyButton * create();

 protected:

 private:
    Stack *d_stack;
    Army *d_army;
    guint32 d_circle_color_id;

    Gtk::ToggleButton *army_button;
    Gtk::Image *army_image;
    Gtk::Label *army_label;
    Gtk::Button *stack_button;
    Gtk::Image *stack_image;
    Gtk::Box *stack_button_container;
    Gtk::EventBox *eventbox;
    ArmyInfoTip *army_info_tip;

    sigc::connection stack_conn;
    sigc::connection army_conn[3];

    bool on_army_button_event(GdkEventButton *e);
    void fill_buttons();
    void fill_army_button();
    void fill_stack_button();
    void setup_signals();
    void clear_signals();
};

#endif // STACK_ARMY_BUTTON
