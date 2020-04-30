//  Copyright (C) 2007, 2008, Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014, 2015, 2020 Ben Asselstine
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
#ifndef FIGHT_WINDOW_H
#define FIGHT_WINDOW_H

#include <memory>
#include <vector>
#include <list>
#include <sigc++/trackable.h>
#include <glibmm/main.h>
#include <gtkmm.h>

#include "game-parameters.h"
#include "fight.h"

class Fight;
class Army;

// window for displaying the course of a fight
class FightWindow: public sigc::trackable
{
 public:

    static bool s_quick_all; 

    FightWindow(Gtk::Window &parent, Fight &fight);
    ~FightWindow();

    void hide() {window->hide();};
    void run(bool *quick);
    
 private:
    Gtk::Window* window;
    Gtk::Label *decision_label;
    static const int max_cols = 8;

    struct ArmyItem
    {
	Army *army;
	int hp;
        Gtk::Image *water_image;
	Gtk::Image *image;
        bool exploding;
    };
    
    typedef std::vector<ArmyItem> army_items_type;
    army_items_type army_items;

    typedef std::list<FightItem> actions_type;
    actions_type actions;

    typedef std::vector<Army *> armies_type; // for convenience

    Glib::RefPtr<Glib::MainLoop> main_loop;
    
    int round;
    actions_type::iterator action_iterator;
    
    // add an army to the window
    void add_army(Army *army, int initial_hp,
                  std::vector<Gtk::Box *> &hboxes,
		  Gtk::Box *vbox, int current_no);

    void on_key_release_event(GdkEventKey *ev);

    bool do_round();
    bool d_quick;
    int normal_round_speed;
    int fast_round_speed;
    Glib::ustring d_decision;
    guint32 armypic_height;
    guint32 armypic_width;
};

#endif
