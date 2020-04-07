//  Copyright (C) 2020 Ben Asselstine
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
#ifndef ARMY_CHOOSER_BUTTON_H
#define ARMY_CHOOSER_BUTTON_H

#include <gtkmm.h>

#include "select-army-dialog.h"

class Player;
class ArmyProto;

 //! A button that shows what army we have selected.
/**
 * A convenient way to call the SelectArmyDialog.
 * It gives us a single place to say "no army selected" or whatever.
 *
 * We could beef this up with an image of the army.
 *
 */
class ArmyChooserButton
{
 public:
    ArmyChooserButton(Gtk::Window &w,
                      Glib::RefPtr<Gtk::Builder> xml,
                      Glib::ustring widget,
                      Player *p, SelectArmyDialog::Mode mode);
    ~ArmyChooserButton() {};

    void set_sensitive (bool s) {d_button->set_sensitive (s);}

    void select (guint32 selected);

    void clear_selected_army ();

    sigc::signal<void, const ArmyProto *> army_selected;
 private:
    Gtk::Window &d_window;
    Player *d_player;
    SelectArmyDialog::Mode d_mode;
    bool d_selected;
    guint32 d_selected_army_type;
    Gtk::Button *d_button;

    void on_button_pressed ();
    void update_button ();
};

#endif
