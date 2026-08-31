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
#ifndef QUICK_HELP_WINDOW_H
#define QUICK_HELP_WINDOW_H

#include <gtkmm.h>


//! dialog for showing the scenario and who's joined
class QuickHelpWindow
{
public:
    QuickHelpWindow();

    ~QuickHelpWindow() {};

    Gtk::Window *get_window () {return window;}
    void hide() {window->hide();};
    void show() {window->show_all();};

private:
    Glib::RefPtr<Gtk::Builder> xml;
    Gtk::Window *window;
    Gtk::Button *close_button;
    void on_close_button_clicked ();

    void add_shortcut (Gtk::Box *box);
};

#endif
