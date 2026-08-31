//  Copyright (C) 2008, 2009, 2010, 2014, 2015, 2020, 2026 Ben Asselstine
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
#ifndef TILE_PREVIEW_DIALOG_H
#define TILE_PREVIEW_DIALOG_H
#include "tile.h"
#include "tile-preview-scene.h"

class TilePreviewDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "tile-preview.ui";
      }

    TilePreviewDialog (BaseObjectType* o,
                       const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        set_size_request (450, 350);
        m_close_button = load <Gtk::Button> ("close_button");
        m_next_button = load <Gtk::Button> ("next_button");
        m_previous_button = load <Gtk::Button> ("previous_button");
        m_refresh_button = load <Gtk::Button> ("refresh_button");
        m_preview_drawing_area = load <Gtk::DrawingArea> ("preview_drawing_area");
        m_label = load <Gtk::Label> ("label");
      }

    ~TilePreviewDialog ()
      {
        for (auto scene : m_scenes)
          delete scene;
      }

    void setup (Tile *tile, Tile *sec, guint32 tile_size)
      {
        m_tile = tile;
        set_title (String::ucompose (_("Preview for %1"), m_tile->getName ()));

        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        m_preview_drawing_area->set_draw_func
          ([this] (const Cairo::RefPtr<Cairo::Context>& cr, int w, int h)
           {
             auto scene = *m_current_scene;
             if (!scene)
               return;

             auto pixbuf = scene->render_pixbuf ();
             if (!pixbuf)
               return;

             int img_w = pixbuf->get_width ();
             int img_h = pixbuf->get_height ();

             double x = (w - img_w) / 2.0;
             double y = (h - img_h) / 2.0;

             Gdk::Cairo::set_source_pixbuf (cr, pixbuf, x, y);

             cr->paint ();
           });

        m_next_button->set_icon_name ("go-next-symbolic");
        m_next_button->signal_clicked ().connect
          ([this] ()
           {
             if (m_scenes.end () != m_current_scene)
               {
                 m_label->set_text ("");
                 ++m_current_scene;
                 m_preview_drawing_area->queue_draw ();
                 update_buttons ();
               }
           });

        m_previous_button->set_icon_name ("go-previous-symbolic");
        m_previous_button->signal_clicked ().connect
          ([this] ()
           {
             if (m_scenes.begin () != m_current_scene)
               {
                 m_label->set_text ("");
                 --m_current_scene;
                 m_preview_drawing_area->queue_draw ();
                 update_buttons ();
               }
           });

        m_refresh_button->set_icon_name ("view-refresh-symbolic");
        m_refresh_button->signal_clicked ().connect
          ([this] ()
           {
             m_label->set_text ("");
             TilePreviewScene *scene = *m_current_scene;
             if (scene)
               scene->regenerate ();
             m_preview_drawing_area->queue_draw ();
           });

        auto motion = Gtk::EventControllerMotion::create ();
        motion->signal_motion ().connect
          ([this](double x, double y)
           {
             struct MouseMotionEvent ev = {};
             ev.pos = drawing_area_pos_to_scene_pos (x, y);
             if (ev.pos == Vector<int> (-1, -1))
               {
                 if (m_label->get_text () != "")
                   m_label->set_text ("");
                 return;
               }

             (*m_current_scene)->mouse_motion_event (ev);
           });

        motion->signal_leave ().connect
          ([this] ()
           {
             m_label->set_text ("");
           });

        m_preview_drawing_area->add_controller (motion);

        m_tile_size = tile_size;

        Glib::ustring scene;

        m_scenes.clear();

        m_tile = tile;
        m_sec = sec;
        add_scenes ();

        m_current_scene = m_scenes.begin ();
        m_preview_drawing_area->queue_draw ();
        update_buttons();
      }

private:
    Gtk::Button *m_close_button;
    Gtk::Button *m_next_button;
    Gtk::Button *m_previous_button;
    Gtk::Button *m_refresh_button;
    Gtk::DrawingArea *m_preview_drawing_area;
    Gtk::Label *m_label;
    Tile *m_tile;
    Tile *m_sec;
    
    std::vector<PixMask* > m_tilestyle_images;
    std::list<TilePreviewScene*> m_scenes;
    std::list<TilePreviewScene*>::iterator m_current_scene;
    guint32 m_tile_size;

    void update_buttons ()
      {
        auto it = m_current_scene;
        m_next_button->set_sensitive (++it != m_scenes.end ());
        m_previous_button->set_sensitive (m_current_scene != m_scenes.begin ());
      }

    void add_scene (TilePreviewScene *s)
      {
        s->signal_tilestyle_hovered ().connect
          ([this] (guint32 id)
           {
             auto t = m_tile->getTileStyle (id);
             auto type = TileStyle::getTypeName (t->getType ());
             m_label->set_text
               (String::ucompose (_("Tile Style ID: 0x%1 Type: %2"), id, type));
           });

        m_scenes.push_back (s);
      }
        
    void add_scenes ()
      {
        Glib::ustring scene;

        guint32 ts = m_tile_size;
        switch (m_tile->getType ())
          {
          case Tile::GRASS:
            scene.clear ();
            scene += "aaaaa";
            scene += "aaaaa";
            scene += "aaaaa";
            scene += "aaaaa";
            scene += "aaaaa";
            add_scene (new TilePreviewScene (m_tile, m_sec, 5, 5, scene, ts));
            break;

          case Tile::WATER:
            scene.clear ();
            scene += "bcd";
            scene += "hij";
            scene += "efg";
            add_scene (new TilePreviewScene (m_tile, m_sec, 3, 3, scene, ts));

            scene.clear ();
            scene += "iiiii";
            scene += "iiiii";
            scene += "iiiii";
            scene += "iiiii";
            scene += "iiiii";
            add_scene (new TilePreviewScene (m_tile, m_sec, 5, 5, scene, ts));

            scene.clear ();
            scene += "iiii";
            scene += "ikli";
            scene += "ijhi";
            scene += "imni";
            scene += "iiii";
            add_scene (new TilePreviewScene (m_tile, m_sec, 5, 4, scene, ts));

            scene.clear ();
            scene += "kliii";
            scene += "mplkl";
            scene += "ijhjh";
            scene += "ijeon";
            scene += "imcni";
            add_scene (new TilePreviewScene (m_tile, m_sec, 5, 5, scene, ts));
            break;

          case Tile::FOREST:
            scene.clear ();
            scene += "bcd";
            scene += "hij";
            scene += "efg";
            add_scene (new TilePreviewScene (m_tile, m_sec, 3, 3, scene, ts));

            scene.clear ();
            scene += "iiiii";
            scene += "iiiii";
            scene += "iiiii";
            scene += "iiiii";
            scene += "iiiii";
            add_scene (new TilePreviewScene (m_tile, m_sec, 5, 5, scene, ts));

            scene.clear ();
            scene += "ahiii";
            scene += "cplkf";
            scene += "ijhja";
            scene += "ijeoc";
            scene += "ijahi";
            add_scene (new TilePreviewScene (m_tile, m_sec, 5, 5, scene, ts));
            break;

          case Tile::HILLS:
            scene.clear ();
            scene += "bcd";
            scene += "hij";
            scene += "efg";
            add_scene (new TilePreviewScene (m_tile, m_sec, 3, 3, scene, ts));

            scene.clear ();
            scene += "iiiii";
            scene += "iiiii";
            scene += "iiiii";
            scene += "iiiii";
            scene += "iiiii";
            add_scene (new TilePreviewScene (m_tile, m_sec, 5, 5, scene, ts));

            scene.clear ();
            scene += "ahiii";
            scene += "cplkf";
            scene += "ijhja";
            scene += "ijeoc";
            scene += "ijahi";
            add_scene (new TilePreviewScene (m_tile, m_sec, 5, 5, scene, ts));
            break;

          case Tile::MOUNTAIN:
            scene.clear ();
            scene += "bcd";
            scene += "hij";
            scene += "efg";
            add_scene (new TilePreviewScene (m_tile, m_sec, 3, 3, scene, ts));

            scene.clear ();
            scene += "iiiii";
            scene += "iiiii";
            scene += "iiiii";
            scene += "iiiii";
            scene += "iiiii";
            add_scene (new TilePreviewScene (m_tile, m_sec, 5, 5, scene, ts));

            scene.clear ();
            scene += "III";
            scene += "IaI";
            scene += "III";
            add_scene (new TilePreviewScene (m_tile, m_sec, 3, 3, scene, ts));

            scene.clear ();
            scene += "ahiii";
            scene += "cplkf";
            scene += "ijhja";
            scene += "ijeoc";
            scene += "ijahi";
            add_scene (new TilePreviewScene (m_tile, m_sec, 5, 5, scene, ts));
            break;

          case Tile::SWAMP:
            scene.clear ();
            scene += "aaaaa";
            scene += "aaaaa";
            scene += "aaaaa";
            scene += "aaaaa";
            scene += "aaaaa";
            add_scene (new TilePreviewScene (m_tile, m_sec, 5, 5, scene, ts));

            scene.clear ();
            scene += "ahiii";
            scene += "cplkf";
            scene += "ijhja";
            scene += "ijeoc";
            scene += "ijahi";
            add_scene (new TilePreviewScene (m_tile, m_sec, 5, 5, scene, ts));

            scene.clear ();
            scene += "iiiii";
            scene += "iiiii";
            scene += "iiiii";
            scene += "iiiii";
            scene += "iiiii";
            add_scene (new TilePreviewScene (m_tile, m_sec, 5, 5, scene, ts));
            break;
          }
      }

    Vector<int> drawing_area_pos_to_scene_pos (double x, double y)
      {
        auto scene = *m_current_scene;

        int img_w = scene->get_width () * m_tile_size;
        int img_h = scene->get_height () * m_tile_size;

        int w = m_preview_drawing_area->get_width ();
        int h = m_preview_drawing_area->get_height ();

        double hx = (w - img_w) / 2.0;
        double hy = (h - img_h) / 2.0;

        LwRectangle box ((int)hx, (int)hy, img_w, img_h);
        if (is_inside (box, Vector<int>((int)x, (int)y)))
          {
            Vector<int> pos = Vector<int> ((int)x, (int)y);
            pos -= Vector<int>((int)hx, (int)hy);
            return pos;
          }
        else
          return Vector<int>(-1,-1);
      }
};
#endif

