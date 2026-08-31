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
#ifndef MAP_INFO_TIP_H
#define MAP_INFO_TIP_H

#include <gtkmm.h>
#include <memory>
#include "info-tip.h"

// shows a tooltip like window with some text information

class MapInfoTip : public InfoTip
{
public:
    MapInfoTip ()
      :InfoTip ()
      {
        m_hbox = new Gtk::Box ();
        m_hbox->set_orientation (Gtk::Orientation::HORIZONTAL);
        set_child (*m_hbox);

        m_info_label = new Gtk::Label ("");
        m_info_label->set_justify (Gtk::Justification::CENTER);
        m_hbox->append (*m_info_label);
      }

    ~MapInfoTip ()
      {
        delete m_hbox;
        delete m_info_label;
      }

    void set (Glib::ustring message)
      {
        m_info_label->set_text (message);
      }

private:
    Gtk::Box *m_hbox = NULL;
    Gtk::Label *m_info_label = NULL;
};
#endif
