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

#include <gtkmm.h>
#ifndef BUTTON_LABEL_H
#define BUTTON_LABEL_H
#include <string>

class ButtonLabel : public Gtk::Box
{
public:

    ButtonLabel (Gtk::Button *button, const std::string& text = "")
      : Gtk::Box (Gtk::Orientation::HORIZONTAL, 6)
      {
        m_button = button;
        m_label = Gtk::make_managed<Gtk::Label> ();
        m_label->set_max_width_chars (20);
        m_label->set_ellipsize (Pango::EllipsizeMode::END);
        set_margin_start (6);
        set_margin_end (6);

        set_label (text);

        append (*m_label);

        m_label->set_halign (Gtk::Align::START);
        m_label->set_valign (Gtk::Align::CENTER);
        
        button->set_child (*this);
      }

    void set_label (Glib::ustring text)
      {
        if (text == "")
          {
            m_label->set_css_classes ({});
            m_label->add_css_class ("button-label");
            m_label->set_text (_("Empty"));
          }
        else
          {
            m_label->set_css_classes ({});
            m_label->add_css_class ("button-label");
            m_label->set_text (text);
            if ((int)text.length () > m_label->get_max_width_chars ())
              m_button->set_tooltip_text (text);
          }
      }

private:
    Gtk::Label *m_label;
    Gtk::Button *m_button;
};
#endif
