//  Copyright (C) 2011, 2020 Ben Asselstine
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
#ifndef GAME_BUTTON_BOX_H
#define GAME_BUTTON_BOX_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm.h>
#include <glibmm.h>
#include "Configuration.h"

class Game;
// shows the game buttons in the main game window
class GameButtonBox: public Gtk::Box
{
 public:
     //! Constructor for building this object with gtk::builder
    GameButtonBox(BaseObjectType* base, const Glib::RefPtr<Gtk::Builder> &xml);

    //!Destructor.
    ~GameButtonBox() {drop_connections();};

    void give_some_cheese();
    void setup_signals(Game *game);

    //Signals
    sigc::signal<void> diplomacy_clicked;

    // Statics
    static GameButtonBox * create();

 protected:

 private:
    std::list<sigc::connection> connections;
    Gtk::Button *next_movable_button;
    Gtk::Button *center_button;
    Gtk::Button *diplomacy_button;
    Gtk::Button *defend_button;
    Gtk::Button *park_button;
    Gtk::Button *deselect_button;
    Gtk::Button *search_button;
    Gtk::Button *move_button;
    Gtk::Button *move_all_button;
    Gtk::Button *end_turn_button;

    void setup_button(Gtk::Button *button, sigc::slot<void> slot,
                      sigc::signal<void, bool> &game_signal);

    void change_diplomacy_button_image (bool proposals_present);
    void update_diplomacy_button (bool sensitive);

    void add_pictures_to_buttons();
    void drop_connections();
    void pad_image(Gtk::Image *image);
    void add_picture_to_button (guint32 icontype, Gtk::Button *button);
};

#endif // GAME_BUTTON_BOX
