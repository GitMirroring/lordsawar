//  Copyright (C) 2007, 2008, 2009, 2014, 2015, 2020, 2026 Ben Asselstine
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

#include <config.h>
#include <assert.h>

#include "army-map.h"

#include "player-list.h"
#include "stack-list.h"
#include "stack.h"
#include "image-cache.h"
#include "game-map.h"
#include "fog-map.h"

ArmyMap::ArmyMap()
{
}

void ArmyMap::draw_stacks()
{
  // Draw stacks as tiny shields
  for (Playerlist::iterator pit = Playerlist::instance()->begin();
       pit != Playerlist::instance()->end(); ++pit)
    {
      Stacklist* mylist = (*pit)->getStacklist();
      //Gdk::RGBA cross_color = (*pit)->getColor();

      for (Stacklist::iterator it= mylist->begin(); it != mylist->end(); ++it)
        {
          Vector<int> pos = (*it)->getPos();

          // don't draw stacks in cities, they could hardly be identified
          Maptile* mytile = GameMap::instance()->getTile(pos.x, pos.y);
          if (mytile->getBuilding() == Maptile::CITY)
            continue;

          // don't draw stacks on tiles we can't see
          if (Playerlist::getViewingplayer()->getFogMap()->isFogged (pos) == true)
            continue;

          PixMask *tmp = 
            ImageCache::instance()->getShieldPic(1, (*it)->getOwner(),
                                                    true)->copy();

          pos = mapToSurface(pos);
          tmp->blit_centered(surface, pos);
          delete tmp;
        }
    }
}

void ArmyMap::after_draw()
{
    assert(surface);
    draw_cities(false);
    draw_stacks();
    map_changed.emit(surface);
}

