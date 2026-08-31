//  Copyright (C) 2007, 2008, 2009, 2012, 2014, 2017, 2026 Ben Asselstine
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
#ifndef RUIN_REPORT_DIALOG_H
#define RUIN_REPORT_DIALOG_H
#include "ruin-map.h"
#include "image-helpers.h"
#include "named-location.h"
#include "input-events.h"
class RuinReportDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "ruin-report.ui";
      }

    RuinReportDialog (BaseObjectType* o,
                      const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_name_label = load <Gtk::Label> ("name_label");
        m_type_label = load <Gtk::Label> ("type_label");
        m_explored_label = load <Gtk::Label> ("explored_label");
        m_description_label = load <Gtk::Label> ("description_label");
        m_map_drawing_area = load <Gtk::DrawingArea> ("map_drawing_area");
      }

    void setup (NamedLocation *ruin)
      {
        m_ruin = ruin;
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        m_description_label->set_wrap (true);
        m_description_label->set_max_width_chars (30);

        m_ruin_map = new RuinMap (m_ruin, NULL);
        m_ruin_map->map_changed.connect
          ([this] (Cairo::RefPtr<Cairo::Surface> map)
           {
             cairo_surface_to_drawing_area (map, m_map_drawing_area);
           });

        m_ruin_map->resize ();
        m_ruin_map->draw ();

        auto click = Gtk::GestureClick::create ();
        click->set_button (1);
        click->signal_pressed ().connect
          ([this] (int n, double x, double y)
           {
             struct MouseButtonEvent ev = to_input_event (n, x, y, true);
             m_ruin_map->mouse_button_event (ev);
             m_ruin = m_ruin_map->getNamedLocation ();
             fill_in_location_info ();
           });

        click->signal_released ().connect
          ([this] (int n, double x, double y)
           {
             struct MouseButtonEvent ev = to_input_event (n, x, y, false);
             m_ruin_map->mouse_button_event (ev);
             m_ruin = m_ruin_map->getNamedLocation ();
             fill_in_location_info ();
           });
        m_map_drawing_area->add_controller (click);

        auto motion = Gtk::EventControllerMotion::create ();
        motion->signal_motion ().connect
          ([this](double x, double y)
           {
             auto c = m_ruin_map->get_cursor (x, y);
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

        fill_in_location_info ();

        signal_response ().connect
          ([this](Gtk::ResponseType)
           {
             hide ();
           });
      }
private:
    Gtk::Button *m_close_button = NULL;
    Gtk::Label *m_name_label = NULL;
    Gtk::Label *m_type_label = NULL;
    Gtk::Label *m_explored_label = NULL;
    Gtk::Label *m_description_label = NULL;
    Gtk::DrawingArea *m_map_drawing_area = NULL;

    RuinMap *m_ruin_map = NULL;
    NamedLocation *m_ruin = NULL;

    void fill_in_location_info ()
      {
        NamedLocation *l = m_ruin;
        m_name_label->set_text (l->getName ());
        m_description_label->set_text (l->getDescription ());
        Ruin *ruin = GameMap::getRuin (l->getPos ());
        Temple *temple = GameMap::getTemple (l->getPos ());
        if (ruin)
          {
            switch (ruin->getType ())
              {
              case Ruin::RUIN:
              case Ruin::SAGE:
                m_type_label->set_text (_("Ruin"));
                break;
              case Ruin::STRONGHOLD:
                m_type_label->set_text (_("Stronghold"));
                break;
              }

            if (ruin->isSearched ())
              m_explored_label->set_text (_("Yes"));
            else
              {
                Glib::ustring hint = "  ";
                m_explored_label->set_text (_("No"));
                //add the difficulty hint.
                if (ruin->getOccupant () != NULL)
                  {
                    Keeper *keeper = ruin->getOccupant ();
                    Stack *s = keeper->getStack ();
                    if (s)
                      {
                        switch ((*s->front ()).getStat (Army::STRENGTH))
                          {
                          case 9: 
                            hint += _("It is especially well-guarded.");
                            break;

                          case 8: 
                            hint += _("Rumour speaks of a formidable force within."); 
                            break;

                          case 7: 
                            hint += _("Even heroes are wary of this site.");
                            break;

                          case 6: 
                            hint += _("Bones litter this place.");
                            break;

                          case 5:
                          case 4:
                          case 3:
                          case 2:
                          case 1: 
                            hint += _("It is guarded.");
                            break;

                          case 0: 
                            hint += "";
                            break;

                          default: 
                            hint += "";
                            break;
                          }
                      }
                  }
                else
                  hint += _("Bones litter this place.");
                if (hint != "")
                  {
                    Glib::ustring s = m_description_label->get_text () +
                      "  " + hint;
                    m_description_label->set_text (s);
                  }
              }
          }
        else if (temple)
          {
            m_type_label->set_text (_("Temple"));
            m_explored_label->set_text (_("No"));
          }
        else
          m_type_label->set_text ("");
      }
};
#endif
