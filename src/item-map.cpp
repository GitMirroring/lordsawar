//  Copyright (C) 2010, 2014 Ben Asselstine
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
#include "itemmap.h"

#include "stack.h"
#include "MapBackpack.h"
#include "ImageCache.h"

ItemMap::ItemMap(const std::list<Stack*> &item_laden_stacks,
                 const std::list<MapBackpack*> &bags_of_stuff)
 : bags (bags_of_stuff), stacks (item_laden_stacks)
{
}

void ItemMap::draw_bag(Vector<int> pos)
{
    Vector<int> start = pos;
    start = mapToSurface(start);
    start += Vector<int>(int(pixels_per_tile/2), int(pixels_per_tile/2));
    PixMask *bagpic = ImageCache::getInstance()->getSmallBagImage();
    bagpic->blit_centered(surface, start);
}

void ItemMap::draw_bags()
{
  for (std::list<MapBackpack*>::iterator it = bags.begin(); it != bags.end();
       ++it)
    draw_bag((*it)->getPos());
}

void ItemMap::draw_heroes()
{
  for (std::list<Stack*>::iterator it = stacks.begin(); it != stacks.end();
       ++it)
    draw_hero((*it)->getPos(), true);
}

void ItemMap::after_draw()
{
    OverviewMap::after_draw();
    draw_cities(false);
    draw_heroes();
    draw_bags();
    map_changed.emit(surface);
}

