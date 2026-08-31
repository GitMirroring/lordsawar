//  Copyright (C) 2010, 2012, 2014, 2017, 2026 Ben Asselstine
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
#ifndef ITEM_REPORT_DIALOG_H
#define ITEM_REPORT_DIALOG_H
#include "item-map.h"
class ItemReportDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "item-report.ui";
      }

    ItemReportDialog (BaseObjectType* o,
                      const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_label = load <Gtk::Label> ("label");
        m_map_drawing_area = load <Gtk::DrawingArea> ("map_drawing_area");
      }

    void setup (const std::list<Stack*> stacks, const std::list<MapBackpack *> bags)
      {
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        m_item_map = new ItemMap (stacks, bags);
        m_item_map->map_changed.connect
          ([this] (Cairo::RefPtr<Cairo::Surface> map)
           {
             cairo_surface_to_drawing_area (map, m_map_drawing_area);
           });

        auto motion = Gtk::EventControllerMotion::create ();
        motion->signal_motion ().connect
          ([this](double x, double y)
           {
             auto c = m_item_map->get_cursor (x, y);
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

        m_item_map->resize ();
        m_item_map->draw ();

        fill_item_info (stacks);
      }

private:
    Gtk::Button *m_close_button = NULL;
    Gtk::Label *m_label = NULL;
    Gtk::DrawingArea *m_map_drawing_area = NULL;
    ItemMap* m_item_map = NULL;

    void fill_item_info (const std::list<Stack*> stacks)
      {
        int count = 0;
        for (auto stack : stacks)
          count += stack->countItems ();

        Glib::ustring s = "";
        if (count > 0)
          s = String::ucompose (ngettext ("You have %1 item!",
                                          "You have %1 items!", count), count);
        else
          s += _("You don't have any items!");

        m_label->set_text (s);
      }
};
#endif
