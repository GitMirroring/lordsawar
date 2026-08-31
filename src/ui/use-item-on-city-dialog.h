//  Copyright (C) 2011, 2012, 2014, 2015, 2017, 2026 Ben Asselstine
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
#include "lw-dialog-base.h"
#ifndef USE_ITEM_ON_CITY_DIALOG_H
#define USE_ITEM_ON_CITY_DIALOG_H
#include "city-map.h"
#include "city.h"
#include "image-helpers.h"
#include "input-events.h"

class UseItemOnCityDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "use-item-on-city.ui";
      }

    UseItemOnCityDialog (BaseObjectType* o,
                           const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_continue_button = load <Gtk::Button> ("continue_button");
        m_map_drawing_area = load <Gtk::DrawingArea> ("map_drawing_area");
        m_label = load <Gtk::Label> ("label");
        m_city_label = load <Gtk::Label> ("city_label");
      }

    void setup (SelectCityMap::Type t)
      {
        set_response (m_continue_button, Gtk::ResponseType::ACCEPT);

        m_city_map = new SelectCityMap (t);
        m_city_map->map_changed.connect
          ([this] (Cairo::RefPtr<Cairo::Surface> map)
           {
             cairo_surface_to_drawing_area (map, m_map_drawing_area);
           });

        m_city_map->city_selected.connect
          ([this] (City *city)
           {
             m_city = city;
             fill_in_city_info ();
           });

        auto click = Gtk::GestureClick::create ();
        click->set_button (1);
        click->signal_pressed ().connect
          ([this] (int n, double x, double y)
           {
             struct MouseButtonEvent ev = to_input_event (n, x, y, true);
             m_city_map->mouse_button_event (ev);
           });

        click->signal_released ().connect
          ([this] (int n, double x, double y)
           {
             struct MouseButtonEvent ev = to_input_event (n, x, y, false);
             m_city_map->mouse_button_event (ev);
           });
        m_map_drawing_area->add_controller (click);

        auto motion = Gtk::EventControllerMotion::create ();
        motion->signal_motion ().connect
          ([this](double x, double y)
           {
             auto c = m_city_map->get_cursor (x, y);
             static ImageCache::CursorType prev_cursor = ImageCache::SHIP;
             if (c != prev_cursor)
               {
                 auto hotspot = ImageCache::get_hotspot (c);
                 auto im = ImageCache::instance ()->getCursorPic (c);
                 auto cursor = Gdk::Cursor::create (im->to_texture (),
                                                    hotspot.x, hotspot.y);
                 m_map_drawing_area->set_cursor (cursor);
               }
             prev_cursor = c;
           });

        m_map_drawing_area->add_controller (motion);
        m_city_map->resize ();
        m_city_map->draw ();

        m_city_label->set_wrap (true);
        m_city_label->set_max_width_chars (24);

        switch (t)
          {
          case SelectCityMap::ANY_CITY:
            m_label->set_text (_("Select a city to target."));
            break;
          case SelectCityMap::FRIENDLY_CITY:
            m_label->set_text (_("Select one of your cities to target."));
            break;
          case SelectCityMap::ENEMY_CITY:
            m_label->set_text (_("Select an enemy city to target."));
            break;
          case SelectCityMap::NEUTRAL_CITY:
            m_label->set_text (_("Select a neutral city to target."));
            break;
          }

        signal_response ().connect
          ([this](Gtk::ResponseType)
           {
             hide ();
             m_city_selected.emit (m_city);
           });
      }

    sigc::signal<void(City*)> signal_city_selected ()
      {
        return m_city_selected;
      }
private:
    Gtk::Button *m_continue_button = NULL;
    Gtk::DrawingArea *m_map_drawing_area = NULL;
    Gtk::Label *m_label = NULL;
    Gtk::Label *m_city_label = NULL;
    City *m_city = NULL;

    SelectCityMap *m_city_map = NULL;

    sigc::signal<void(City*)> m_city_selected;

    void fill_in_city_info ()
      {
        m_city_label->set_text
          (String::ucompose (_("The city of %1 is targeted."),
                             m_city->getName ()));
      }
};
#endif
