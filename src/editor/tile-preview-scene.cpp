//  Copyright (C) 2008, 2009, 2010, 2014, 2020 Ben Asselstine
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#include <config.h>

#include <sigc++/functors/mem_fun.h>

#include "tile-preview-scene.h"

#include "ucompose.hpp"
#include "defs.h"
#include "File.h"
#include "ImageCache.h"

TilePreviewScene::TilePreviewScene (Tile *tile, Tile *secondary_tile,
				    guint32 height, guint32 width,
				    Glib::ustring scene, guint32 tilesize)
{
  struct tile_model model;
  std::list<struct tile_model> tilescene;
  for (const char *letter = scene.c_str(); *letter != '\0'; letter++)
    if (*letter - 'a' >=0 && *letter - 'a' <= TileStyle::OTHER)
      {
        model.tile = tile;
        model.type = TileStyle::Type(*letter - 'a');
        tilescene.push_back(model);
      }
    else if (*letter - 'A' >=0 && *letter - 'A' <= TileStyle::OTHER)
      {
        model.tile = secondary_tile;
        model.type = TileStyle::Type(*letter - 'A');
        tilescene.push_back(model);
      }

  if (height * width != tilescene.size())
    return;

  d_tile = tile;
  d_secondary_tile = secondary_tile;
  d_height = height;
  d_width = width;
  d_model = tilescene;
  d_tilesize = tilesize;
  regenerate();
}

Glib::RefPtr<Gdk::Pixbuf> TilePreviewScene::getTileStylePixbuf(int x, int y)
{
  return d_view[y * d_width + x];
}

void TilePreviewScene::regenerate()
{
  //populate d_view
  d_view.clear();
  for (std::list<struct tile_model>::iterator it = d_model.begin();
       it != d_model.end(); it++)
    {
      struct tile_model model = *it;
      TileStyle *tilestyle = NULL;
      if (model.tile)
	{
	  tilestyle = model.tile->getRandomTileStyle(model.type);
	  d_tilestyles.push_back(tilestyle);
	}

      if (tilestyle)
        {
          PixMask *p = tilestyle->getImage()->copy ();
          double new_height = d_tilesize;
          int new_width =
            ImageCache::calculate_width_from_adjusted_height (p, new_height);
          PixMask::scale (p, new_width, new_height);
          d_view.push_back(p->to_pixbuf());
          delete p;
        }
      else
        {
          PixMask *p =
            ImageCache::getInstance()->getDefaultTileStylePic
            (model.type, d_tilesize)->copy();
          double new_height = d_tilesize;
          int new_width =
            ImageCache::calculate_width_from_adjusted_height (p, new_height);
          PixMask::scale (p, new_width, new_height);
          d_view.push_back(p->to_pixbuf());
          delete p;
        }
    }
}

Glib::RefPtr<Gdk::Pixbuf> TilePreviewScene::renderScene()
{
  guint32 ts = d_tilesize;
  Glib::RefPtr<Gdk::Pixbuf> dest;
  dest = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, (int)(d_width * ts),
                             (int)(d_height * ts));
  for (unsigned int i = 0; i < d_width; i++)
    for (unsigned int j = 0; j < d_height; j++)
      getTileStylePixbuf(i, j)->copy_area (0, 0, ts, ts, dest, i * ts, j * ts);
  return dest;
}

Vector<int> TilePreviewScene::mouse_pos_to_tile(Vector<int> pos)
{
  return pos / d_tilesize;
}

TileStyle * TilePreviewScene::get_tilestyle(Vector<int> tile)
{
  guint32 idx = (tile.y * d_width) + tile.x;
  return d_tilestyles[idx];
}

void TilePreviewScene::mouse_button_event(MouseButtonEvent e)
{
  if (e.button == MouseButtonEvent::LEFT_BUTTON
      && e.state == MouseButtonEvent::PRESSED)
    {
      Vector<int> pos = mouse_pos_to_tile(e.pos);
      current_tile = pos;
      TileStyle *tilestyle = get_tilestyle(pos);
      selected_tilestyle_id.emit(tilestyle->getId());
    }
  return;
}

void TilePreviewScene::mouse_motion_event(MouseMotionEvent e)
{
  Vector<int> pos = mouse_pos_to_tile(e.pos);
  current_tile = pos;
  TileStyle *tilestyle = get_tilestyle(pos);
  if (tilestyle)
    hovered_tilestyle_id.emit(tilestyle->getId());
  return;
}
