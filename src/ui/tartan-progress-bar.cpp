//  Copyright (C) 2017, 2020 Ben Asselstine
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

#include <gtkmm.h>

#include "tartan-progress-bar.h"

#include "input-helpers.h"
#include "ucompose.hpp"
#include "ImageCache.h"
#include "GameMap.h"
#include "player.h"
#include "stacklist.h"
#include "shieldset.h"
#include "PixMask.h"
#include "playerlist.h"
#include "font-size.h"

TartanProgressBar::TartanProgressBar(Player *p)
{
  player = p;
  percent = 0;
}

TartanProgressBar::~TartanProgressBar()
{
}

double TartanProgressBar::calculate_percentage () const
{
  static int c = 0;
  c++;
  guint32 count = player->getStacklist()->size() -
    player->getStacklist()->countMovableStacks();
  return (double) count / (double)player->getStacklist()->size();
}

void TartanProgressBar::pulse (Player *p)
{
  player = p;
  double new_percent = calculate_percentage ();
  if (new_percent == percent)
    return;
  percent = new_percent;
  queue_draw();
  while (g_main_context_iteration(NULL, FALSE)); //doEvents
}

bool TartanProgressBar::on_draw (const Cairo::RefPtr<Cairo::Context> &cr)
{
  // This is where we draw on the window
  Glib::RefPtr<Gdk::Window> window = get_window();
  if(window && get_parent() && player)
    {
      PixMask *p = 
        ImageCache::getInstance()->getTartanPic (player, get_width() *
                                                 TARTAN_PERCENT_WIDTH,
                                                 GameMap::getShieldset(),
                                                 FontSize::getInstance()->get_height ());

      set_size_request(-1, p->get_height());
      cr->set_source(p->get_pixmap(), 0, 0);
      if (percent < MIN_PERCENT) 
        percent = MIN_PERCENT;
      guint32 limit = (double)p->get_width() * percent;
      cr->rectangle(0, 0, limit, p->get_height());
      cr->fill();
      p = 
        ImageCache::getInstance()->getEmptyTartanPic (player, get_width() *
                                                      TARTAN_PERCENT_WIDTH,
                                                      GameMap::getShieldset(),
                                                      FontSize::getInstance()->get_height ());
      cr->set_source(p->get_pixmap(), 0, 0);
      cr->rectangle(limit, 0, p->get_width() - limit, p->get_height());
      cr->fill();
    }

  return true;
}
