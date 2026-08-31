//  Copyright (C) 2026 Ben Asselstine
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
//  Foundation, Inc., 31 Milk Street #960789, Boston, MA 02196, USA.

#pragma once
#ifndef INFO_TIP_H
#define INFO_TIP_H

#include <gtkmm.h>
#include <memory>

// shows a tooltip like window

class InfoTip : public Gtk::Popover
{
public:
    InfoTip ()
      {
        set_has_arrow (false);
        set_autohide (false);
        set_position (Gtk::PositionType::TOP);

        auto click = Gtk::GestureClick::create ();
        click->set_button (0);
        click->signal_pressed ().connect
          ([this] (int, double, double)
           {
             popdown ();
           });

        add_controller (click);
      }

    ~InfoTip ()
      {
        hide ();
        unparent ();
      }

    void set_justification (int justification)
      {
        switch (justification)
          {
          case MapTipPosition::LEFT:
            set_position (Gtk::PositionType::RIGHT);
            break;
          case MapTipPosition::RIGHT:
            set_position (Gtk::PositionType::LEFT);
            break;
          case MapTipPosition::TOP:
            set_position (Gtk::PositionType::BOTTOM);
            break;
          case MapTipPosition::BOTTOM:
            set_position (Gtk::PositionType::TOP);
            break;
          }
      }
};
#endif
