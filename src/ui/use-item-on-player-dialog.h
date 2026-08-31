//  Copyright (C) 2010, 2012, 2014, 2017, 2020, 2026 Ben Asselstine
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
#ifndef USE_ITEM_ON_PLAYER_DIALOG_H
#define USE_ITEM_ON_PLAYER_DIALOG_H
#include "city-map.h"
#include "player.h"
#include "image-helpers.h"
#include "lw-column.h"
class UseItemOnPlayerRow: public Glib::Object
{
public:
    Player *m_player;

    static Glib::RefPtr<UseItemOnPlayerRow> create (Player *p)
      {
        return
          Glib::make_refptr_for_instance<UseItemOnPlayerRow> (new UseItemOnPlayerRow (p));
      }

protected:
    UseItemOnPlayerRow (Player *p)
      : m_player (p)
      {
      }
};

class UseItemOnPlayerDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "use-item-on-player.ui";
      }

    UseItemOnPlayerDialog (BaseObjectType* o,
                           const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_continue_button = load <Gtk::Button> ("continue_button");
        m_map_drawing_area = load <Gtk::DrawingArea> ("map_drawing_area");
        m_treeview = load <Gtk::ColumnView> ("treeview");
      }

    void setup ()
      {
        set_response (m_continue_button, Gtk::ResponseType::ACCEPT);

        m_city_map = new CityMap ();
        m_city_map->map_changed.connect
          ([this] (Cairo::RefPtr<Cairo::Surface> map)
           {
             cairo_surface_to_drawing_area (map, m_map_drawing_area);
           });

        m_city_map->resize ();
        m_city_map->draw ();

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

        m_store = Gio::ListStore<UseItemOnPlayerRow>::create ();
        m_selection_model = Gtk::SingleSelection::create (m_store);
        m_treeview->set_model (m_selection_model);

        for (auto p : *Playerlist::instance ())
          {
            if (p == Playerlist::getActiveplayer ())
              continue;
            if (p == Playerlist::getNeutral ())
              continue;
            m_store->append (UseItemOnPlayerRow::create (p));
          }
        setup_shield_column ();
        setup_name_column ();

        signal_response ().connect
          ([this](Gtk::ResponseType)
           {
             hide ();
             m_player_selected.emit (get_selected_player ());
           });
      }

    Player *get_selected_player ()
      {
        auto single_selection =
          std::dynamic_pointer_cast<Gtk::SingleSelection> (m_selection_model);

        auto item = single_selection->get_selected_item ();

        if (item)
          {
            auto row = std::dynamic_pointer_cast<UseItemOnPlayerRow> (item);
            return row->m_player;
          }
        return NULL;
      }

    sigc::signal<void(Player*)> signal_player_selected ()
      {
        return m_player_selected;
      }
private:
    Gtk::Button *m_continue_button;
    Gtk::DrawingArea *m_map_drawing_area;
    Gtk::ColumnView *m_treeview;
    Glib::RefPtr<Gtk::SingleSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<UseItemOnPlayerRow>> m_store;
        
    CityMap *m_city_map = NULL;

    sigc::signal<void(Player*)> m_player_selected;

    void setup_shield_column ()
      {
        LwColumn::setup_picture_column<UseItemOnPlayerRow>
          (m_treeview, LW_BUTTON_SIZE, LW_BUTTON_SIZE, "shield_image", "",
           [] (const auto& row)
           {
             auto im =
               ImageCache::instance ()->getShieldPic
               (GameMap::instance ()->getShieldsetId (), 2,
                row->m_player->getId (), false);
             return im->to_texture ();
           });
      }

    void setup_name_column ()
      {
        LwColumn::setup_text_column<UseItemOnPlayerRow>
          (m_treeview, "name_label", true, Gtk::Justification::LEFT,
           _("Players"),
           [] (const auto& row)
           {
             return row->m_player->getName ();
           });
      }

};
#endif
