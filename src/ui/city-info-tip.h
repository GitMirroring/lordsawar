//  Copyright (C) 2009 Ben Asselstine
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
#ifndef CITY_INFO_TIP_H
#define CITY_INFO_TIP_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm.h>
#include "map-tip-position.h"

class City;

// shows a tooltip like window with information about an army
class CityInfoTip: public sigc::trackable
{
 public:
    // the tip is shown above target, simply delete the object to hide it again
    CityInfoTip(Gtk::Widget *target, MapTipPosition map, const City *city);
    ~CityInfoTip() {delete window;};

 private:
    Gtk::Window* window;
};

#endif
