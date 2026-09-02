//  Copyright (C) 2008, 2009, 2010, 2014, 2020, 2026 Ben Asselstine
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
#ifndef TILE_PREVIEW_SCENE_H
#define TILE_PREVIEW_SCENE_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm.h>
#include "tile.h"
#include "tile-style.h"
#include <list>
#include <vector>

#include "input-events.h"
#include "image-cache.h"

struct tile_model
{
  Tile *tile;
  TileStyle::Type type;
};

class TilePreviewScene: public sigc::trackable
{
public:
    TilePreviewScene (Tile *tile, Tile *secondary_tile,
                      guint32 height, guint32 width,
                      Glib::ustring scene, guint32 tilesize)
      {
        struct tile_model model;
        std::list<struct tile_model> tilescene;
        for (const char *letter = scene.c_str (); *letter != '\0'; letter++)
          if (*letter - 'a' >= 0 && *letter - 'a' <= TileStyle::OTHER)
            {
              model.tile = tile;
              model.type = TileStyle::Type(*letter - 'a');
              tilescene.push_back (model);
            }
          else if (*letter - 'A' >= 0 && *letter - 'A' <= TileStyle::OTHER)
            {
              model.tile = secondary_tile;
              model.type = TileStyle::Type(*letter - 'A');
              tilescene.push_back (model);
            }

        if (height * width != tilescene.size ())
          return;

        m_tile = tile;
        m_secondary_tile = secondary_tile;
        m_height = height;
        m_width = width;
        m_model = tilescene;
        m_tilesize = tilesize;
        regenerate ();
      }

    void regenerate ()
      {
        //populate m_view
        m_view.clear ();
        for (auto it = m_model.begin (); it != m_model.end (); ++it)
          {
            struct tile_model model = *it;
            TileStyle *tilestyle = NULL;
            if (model.tile)
              {
                tilestyle = model.tile->getRandomTileStyle (model.type);
                m_tilestyles.push_back (tilestyle);
              }

            if (tilestyle)
              {
                PixMask *p = tilestyle->getImage ()->copy ();
                m_view.push_back (p->to_pixbuf ());
                delete p;
              }
            else
              {
                PixMask *p =
                  ImageCache::instance ()->getDefaultTileStylePic
                  (model.type, m_tilesize)->copy ();
                m_view.push_back (p->to_pixbuf ());
                delete p;
              }
          }
      }

    Glib::RefPtr<Gdk::Pixbuf> render_pixbuf ()
      {
        guint32 ts = m_tilesize;
        Glib::RefPtr<Gdk::Pixbuf> dest =
          Gdk::Pixbuf::create (Gdk::Colorspace::RGB, true, 8,
                               (int)(m_width * ts), (int)(m_height * ts));
        for (unsigned int i = 0; i < m_width; i++)
          for (unsigned int j = 0; j < m_height; j++)
            get_pixbuf (i, j)->copy_area
              (0, 0, ts, ts, dest, i * ts, j * ts);
        return dest;
      }

    Glib::RefPtr<Gdk::Pixbuf> get_pixbuf (int x, int y)
      {
        return m_view[y * m_width + x];
      }

    int get_width () const
      {
        return m_width;
      }

    int get_height () const
      {
        return m_height;
      }

    Tile *get_tile () const
      {
        return m_tile;
      }

    void mouse_motion_event (MouseMotionEvent e)
      {
        Vector<int> pos = mouse_pos_to_tile (e.pos);
        m_current_tile = pos;
        TileStyle *tilestyle = get_tilestyle (pos);
        if (tilestyle)
          m_hovered_tilestyle_id.emit (tilestyle->getId ());
        return;
      }

    sigc::signal<void(guint32)> signal_tilestyle_hovered () const
      {
        return m_hovered_tilestyle_id;
      }
private:
    std::list<struct tile_model> m_model;
    std::vector<Glib::RefPtr<Gdk::Pixbuf> > m_view;
    std::vector<TileStyle*> m_tilestyles;
    guint32 m_height;
    guint32 m_width;
    Tile *m_tile;
    Tile *m_secondary_tile;
    guint32 m_tilesize;
    guint32 m_ts;
    Vector<int> m_current_tile;
    sigc::signal<void(guint32)> m_hovered_tilestyle_id;

    TileStyle * get_tilestyle (Vector<int> tile)
      {
        guint32 idx = (tile.y * m_width) + tile.x;
        return m_tilestyles[idx];
      }

    Vector<int> mouse_pos_to_tile (Vector<int> pos)
      {
        return pos / m_tilesize;
      }
};

#endif
