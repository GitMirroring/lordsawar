// Copyright (C) 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2004 Thomas Plonka
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2014, 2015, 2017,
// 2020 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
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
#include <assert.h>

#include "smallmap.h"
#include "vector.h"
#include "GameScenarioOptions.h"
#include "GameMap.h"
#include "playerlist.h"

bool SmallMap::s_quick = false;

SmallMap::SmallMap(bool headless)
 : OverviewMap(headless)
{
    input_locked = false;
    sliding = false;
    view.pos = Vector<int>(0, 0);
    view.dim = Vector<int>(3, 3);
    if (s_quick)
      sleep_interval = 0;
    else
      sleep_interval = TIMER_SMALLMAP_REFRESH;
}

void SmallMap::set_view(const LwRectangle &new_view)
{
    if (view != new_view)
    {
	view = new_view;
	draw();
    }
}

void SmallMap::draw_selection()
{
    // draw the selection rectangle that shows the viewed part of the map
    Vector<int> pos = mapToSurface(view.pos);

    int w = int(view.w * pixels_per_tile / map_tiles_per_tile);
    int h = int(view.h * pixels_per_tile / map_tiles_per_tile);

    int width = get_width();
    int height = get_height();
    
    // this is a bit unfortunate.  we require this catch-all
    // so that our selector box isn't too big for the smallmap
    if (pos.x + w >= width)
      pos.x = width - w - 1;
    if (pos.y + h >= height)
      pos.y = height - h - 1;

    assert(pos.x >= 0 && pos.x + w < width &&
	   pos.y >= 0 && pos.y + h < height);
    
    draw_rect(pos.x, pos.y, w, h, SELECTOR_BOX_COLOR);
    //draw_rect(pos.x-1, pos.y-1, w+2,  h+2, SELECTOR_BOX_COLOR);
}

void SmallMap::center_view_on_tile(Vector<int> pos, bool slide_me)
{
  pos = clip(Vector<int>(0,0), pos - view.dim / 2, 
             GameMap::get_dim() - view.dim);

  sliding = false;
  if (slide_me && sleep_interval > 0)
    slide_view(LwRectangle(pos.x, pos.y, view.w, view.h));
  else
    set_view(LwRectangle(pos.x, pos.y, view.w, view.h));
	  
  view_changed.emit(view);
}

void SmallMap::center_view_on_pixel(Vector<int> pos, bool slide_me)
{
  pos.x = int(round(pos.x / pixels_per_tile * map_tiles_per_tile));
  pos.y = int(round(pos.y / pixels_per_tile * map_tiles_per_tile));

  /* FIXME: i have no idea why 1.65 is the number i need here.
   it controls the centeredness of the white box on where we clicked.
   this works well for the 3 standard sizes of maps.
   */

  pos.x -= (view.w / 1.65);
  pos.y -= (view.h / 1.65);

  pos = clip(Vector<int>(0, 0), pos, GameMap::get_dim() - view.dim);

  if (slide_me && sleep_interval > 0)
    slide_view(LwRectangle(pos.x, pos.y, view.w, view.h));
  else
    set_view(LwRectangle(pos.x, pos.y, view.w, view.h));

  view_changed.emit(view);
}

void SmallMap::after_draw()
{
  int width = get_width(), height = get_height();
  if (blank_screen == true)
    {
      map_changed.emit(surface, Gdk::Rectangle(0, 0, width, height));
      return;
    }
  Player *p = Playerlist::getViewingplayer();
  OverviewMap::after_draw();
  if (p->getType() == Player::HUMAN ||
      GameScenarioOptions::s_hidden_map == false)
    {
      draw_cities(false);
      draw_selection();
    }
  //for the editor...
  if (GameScenarioOptions::s_round == 0)
    {
      draw_cities(false);
      draw_selection();
    }
    map_changed.emit(surface, Gdk::Rectangle(0, 0, width, height));
}

void SmallMap::mouse_button_event(MouseButtonEvent e)
{
  if (input_locked)
    return;

  if ((e.button == MouseButtonEvent::LEFT_BUTTON ||
       e.button == MouseButtonEvent::RIGHT_BUTTON)
      && e.state == MouseButtonEvent::PRESSED)
    center_view_on_pixel(e.pos, true);
}

void SmallMap::mouse_motion_event(MouseMotionEvent e)
{
  if (input_locked)
    return;

  if (e.pressed[MouseMotionEvent::LEFT_BUTTON] ||
      e.pressed[MouseMotionEvent::RIGHT_BUTTON])
    center_view_on_pixel(e.pos, false);
}

int SmallMap::slide (int x, int y)
{
  int skip = map_tiles_per_tile + 1;
  if (x < y)
    {
      if (x + skip < y)
	x += skip;
      else if (x + map_tiles_per_tile < y)
	x += map_tiles_per_tile ;
      else
	x += 1;
    }
  else if (x > y)
    {
      if (x - skip > y)
	x -= skip;
      else if (x - map_tiles_per_tile > y)
	x -= map_tiles_per_tile;
      else
	x -= 1;
    }
  return x;
}

void SmallMap::slide_view(LwRectangle new_view)
{
  if (view != new_view)
    {
      sliding = true;
      sliding_to = new_view;
      while (1)
	{
	  LwRectangle tmp_view(view);
	  tmp_view.x = slide(tmp_view.x, new_view.x);
	  tmp_view.y = slide(tmp_view.y, new_view.y);

	  view = tmp_view;
	  draw();
          view_slid.emit(view);
          if (sliding_to != new_view)
            break;
          Glib::usleep(sleep_interval);

	  if (tmp_view.x == new_view.x && tmp_view.y == new_view.y)
	    break;
	}
      sliding = false;
    }
}

void SmallMap::center_view ()
{
  set_view (LwRectangle ((GameMap::get_dim () / 2) - (view.dim / 2), view.dim));
  view_changed.emit(view);
}
