//  Copyright (C) 2007, 2008, 2009, 2014, 2017, 2020 Ben Asselstine
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

#include "historymap.h"
#include "vector.h"
#include <gtkmm.h>
#include <gdkmm.h>

#include "city.h"
#include "ruin.h"
#include "ImageCache.h"
#include "playerlist.h"

HistoryMap::HistoryMap(LocationList<City*> *clist, LocationList<Ruin*> *rlist)
{
  d_clist = clist;
  d_rlist = rlist;
}

void HistoryMap::after_draw()
{
    OverviewMap::after_draw();
    drawCities();
    drawRuins();
    map_changed.emit(surface);
}

void HistoryMap::drawRuins()
{
  // Draw all cities as shields over the city location, in the colors of
  // the players.
  for (auto ruin: *d_rlist)
  {
      if (ruin->isVisible(Playerlist::getViewingplayer()) == false)
        continue;
      if (ruin->isSearched() == false)
        continue;
      if (ruin->isHidden() == true && ruin->getOwner() != 
          Playerlist::getInstance()->getActiveplayer())
        continue;

      PixMask* tmp = 
        ImageCache::getInstance()->getShieldPic(1, ruin->getOwner(),
                                                true, 0)->copy();
      PixMask::scale(tmp, tmp->get_width()/2, tmp->get_height()/2);
  
      Vector<int> pos = ruin->getPos();
      pos = mapToSurface(pos);
      tmp->blit_centered(surface, pos);
      delete tmp;
  }
}

void HistoryMap::drawCities()
{
  // Draw all cities as shields over the city location, in the colors of
  // the players.
  for (auto it: *d_clist)
    {
      PixMask *tmp;
      if (it->isVisible(Playerlist::getViewingplayer()) == false)
        continue;
      if (it->isBurnt() == true)
        tmp = ImageCache::getInstance()->getSmallRuinedCityImage();
      else
        tmp = ImageCache::getInstance()->getShieldPic(0, it->getOwner(),
                                                      true, 0);

      Vector<int> pos = it->getPos();
      pos = mapToSurface(pos);
      tmp->blit_centered(surface, pos);
    }
}

void HistoryMap::updateCities (LocationList<City*> *clist, LocationList<Ruin*> *rlist)
{
  d_clist = clist;
  d_rlist = rlist;
  draw();
  after_draw();
}
