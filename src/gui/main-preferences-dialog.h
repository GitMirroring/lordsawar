//  Copyright (C) 2008, 2009, 2014, 2017, 2020 Ben Asselstine
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
#ifndef MAIN_PREFERENCES_DIALOG_H
#define MAIN_PREFERENCES_DIALOG_H

#include <gtkmm.h>

#include "vector.h"
#include "lw-dialog.h"

class Game;
// dialog for showing sound and game preferences
class MainPreferencesDialog: public LwDialog
{
 public:
    MainPreferencesDialog(Gtk::Window &parent);
    ~MainPreferencesDialog() {};

    void run();
    void hide() {dialog->hide();};

 private:
    Gtk::Switch *commentator_switch;
    Gtk::Switch *play_music_switch;
    Gtk::Scale *music_volume_scale;
    void on_play_music_toggled();
    void on_show_commentator_toggled();
    void on_music_volume_changed();
};

#endif
