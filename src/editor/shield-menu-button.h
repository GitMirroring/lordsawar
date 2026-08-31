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
#ifndef SHIELD_MENU_BUTTON_H
#define SHIELD_MENU_BUTTON_H

class ShieldButton: public Gtk::ToggleButton
{
public:
    ShieldButton (Shield::Color shield)
      {
        m_shield = shield;
        m_image = Gtk::make_managed<Gtk::Image>(get_pixbuf (shield));
        set_child (*m_image);
        m_connection =
          signal_toggled ().connect
          ([this, shield] ()
           {
             if (get_active ())
               m_signal_shield_selected.emit (shield);
           });
      }

    Shield::Color get_shield ()
      {
        return m_shield;
      }

    ~ShieldButton ()
      {
        m_connection.disconnect ();
      }

    Glib::RefPtr<Gdk::Paintable> get_paintable ()
      {
        return m_image->get_paintable ();
      }

    sigc::signal<void(Shield::Color)> signal_shield_selected ()
      {
        return m_signal_shield_selected;
      }

private:
    Shield::Color m_shield;
    sigc::connection m_connection;
    Gtk::Image *m_image;
    sigc::signal<void(Shield::Color)> m_signal_shield_selected;

    Glib::RefPtr<Gdk::Pixbuf> get_pixbuf (Shield::Color shield)
      {
        auto pix =
          ImageCache::instance ()->getShieldPic
          (GameMap::getShieldset ()->getId (), 1, shield, false);
        return pix->to_pixbuf ();
      }
};

class ShieldMenuButton : public Gtk::MenuButton
{
public:
    ShieldMenuButton ()
      {
        m_selected = NULL;
        set_child (m_current_image);
        set_popover (m_popover);

        m_box.set_orientation (Gtk::Orientation::HORIZONTAL);
        m_box.set_spacing (3);
        m_popover.set_child (m_box);

        reload ();
      }

    void reload ()
      {
        disconnect_signals ();
        int active = -1;
        int i = 0;
        for (auto button : m_buttons)
          {
            if (button->get_active ())
              active = i;
            m_box.remove (*button);
            i++;
          }
        m_buttons.clear ();
        ShieldButton *first = NULL;
        for (auto p : *Playerlist::instance ())
          {
            auto button = Gtk::make_managed<ShieldButton> (p->get_shield ());
            if (!first)
              first = button;
            else
              button->set_group (*first);

            add_connection
              (button->signal_shield_selected ().connect
               ([this, button] (Shield::Color shield)
                {
                  m_selected = button;
                  m_signal_shield_selected.emit (shield);
                  m_current_image.set (button->get_paintable ());
                  m_popover.popdown ();
                }));
            m_buttons.push_back (button);
            m_box.append (*button);
          }

        if (active >= 0)
          m_buttons[active]->set_active (true);
        else if (m_buttons.empty () == false)
          m_buttons.front ()->set_active (true);

      }

    ~ShieldMenuButton ()
      {
        disconnect_signals ();
      }

    void set_active (Shield::Color shield)
      {
        for (auto button : m_buttons)
          {
            if (button->get_shield () == shield)
              {
                button->set_active (true);
                break;
              }
          }
      }

    sigc::signal<void(Shield::Color)> signal_shield_selected ()
      {
        return m_signal_shield_selected;
      }
private:
    sigc::signal<void(Shield::Color)> m_signal_shield_selected;

    Gtk::Popover m_popover;
    Gtk::Box m_box;

    ShieldButton *m_selected;
    Gtk::Image m_current_image;
    std::vector<ShieldButton*> m_buttons;
    std::list<sigc::connection> m_connections;

    void add_connection (sigc::connection c)
      {
        m_connections.push_back (c);
      }

    void disconnect_signals ()
      {
        for (auto c : m_connections)
          c.disconnect ();
      }
};

#endif
