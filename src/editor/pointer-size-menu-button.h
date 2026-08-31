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

#include <gtkmm.h>
#ifndef POINTER_SIZE_MENU_BUTTON_H
#define POINTER_SIZE_MENU_BUTTON_H

#include "file.h"

class PointerSizeButton: public Gtk::ToggleButton
{
public:
    PointerSizeButton (guint32 size)
      {
        m_size = size;
        m_image = Gtk::make_managed<Gtk::Image>(get_file (size));
        set_child (*m_image);
        m_connection =
          signal_toggled ().connect
          ([this, size] ()
           {
             if (get_active ())
               m_signal_pointer_size_selected.emit (size);
           });
      }

    ~PointerSizeButton ()
      {
        m_connection.disconnect ();
      }

    void redraw ()
      {
        m_image->set (get_file (m_size));
      }

    Glib::RefPtr<Gdk::Paintable> get_paintable ()
      {
        return m_image->get_paintable ();
      }

    sigc::signal<void(guint32)> signal_pointer_size_selected ()
      {
        return m_signal_pointer_size_selected;
      }

private:
    guint32 m_size;
    sigc::connection m_connection;
    Gtk::Image *m_image;
    sigc::signal<void(guint32)> m_signal_pointer_size_selected;

    std::string get_file (guint32 size)
      {
        if (Lw::get_dark ())
          return
            File::getEditorFile (String::ucompose ("button_%1x%1_dark", size));
        else
          return File::getEditorFile (String::ucompose ("button_%1x%1", size));
      }
};

class PointerSizeMenuButton : public Gtk::MenuButton
{
public:

    static inline const std::array<guint32, 5> sizes = { 1, 2, 3, 6, 12 };

    PointerSizeMenuButton ()
      {
        m_selected = NULL;
        set_child (m_current_image);
        set_popover (m_popover);

        m_box.set_orientation (Gtk::Orientation::VERTICAL);
        m_popover.set_child (m_box);

        PointerSizeButton *first = NULL;
        for (int i = 0; i < 5; i++)
          {
            auto button = Gtk::make_managed<PointerSizeButton> (sizes[i]);
            if (!first)
              first = button;
            else
              button->set_group (*first);

            button->signal_pointer_size_selected ().connect
              ([this, button] (guint32 size)
               {
                 m_selected = button;
                 m_signal_pointer_size_selected.emit (size);
                 m_current_image.set (button->get_paintable ());
                 m_popover.popdown ();
               });
            m_buttons.push_back (button);
            m_box.append (*button);
          }

        m_buttons.front ()->set_active (true);
      }

    void set_active (guint32 size)
      {
        int i = 0;
        for (auto button : m_buttons)
          {
            if (size == sizes[i])
              {
                button->set_active (true);
                break;
              }
            i++;
          }
      }

    void redraw ()
      {
        for (auto button : m_buttons)
          button->redraw ();
        if (m_selected)
          m_current_image.set (m_selected->get_paintable ());
      }

    sigc::signal<void(guint32)> signal_pointer_size_selected ()
      {
        return m_signal_pointer_size_selected;
      }
private:
    sigc::signal<void(guint32)> m_signal_pointer_size_selected;

    Gtk::Popover m_popover;
    Gtk::Box m_box;

    PointerSizeButton *m_selected;
    Gtk::Image m_current_image;
    std::list<PointerSizeButton*> m_buttons;
};

#endif
