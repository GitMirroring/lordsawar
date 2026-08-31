//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009 Ben Asselstine
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
#ifndef STACK_INFO_TIP_H
#define STACK_INFO_TIP_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm.h>
#include "map-tip-position.h"

class StackTile;

// shows a tooltip like window with information about a stack 
class StackInfoTip: public sigc::trackable
{
 public:
    // the tip is shown above target, simply delete the object to hide it again
    StackInfoTip(Gtk::Widget *target, MapTipPosition map, StackTile *s);
    ~StackInfoTip();

 private:
    Gtk::Window* window;
    Gtk::Box *image_hbox;
    Gtk::Box *hero_name_box;
    Gtk::Box *hero_description_box;
};

#endif
