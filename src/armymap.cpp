//  Copyright (C) 2007, 2008, 2009, 2014, 2015, 2020 Ben Asselstine
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

#include "armymap.h"

#include "playerlist.h"
#include "stacklist.h"
#include "stack.h"
#include "ImageCache.h"
#include "GameMap.h"
#include "FogMap.h"

ArmyMap::ArmyMap()
{
}

void ArmyMap::draw_stacks()
{
  // Draw stacks as tiny shields
  for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
       pit != Playerlist::getInstance()->end(); ++pit)
    {
      Stacklist* mylist = (*pit)->getStacklist();
      //Gdk::RGBA cross_color = (*pit)->getColor();

      for (Stacklist::iterator it= mylist->begin(); it != mylist->end(); ++it)
        {
          Vector<int> pos = (*it)->getPos();

          // don't draw stacks in cities, they could hardly be identified
          Maptile* mytile = GameMap::getInstance()->getTile(pos.x, pos.y);
          if (mytile->getBuilding() == Maptile::CITY)
            continue;

          // don't draw stacks on tiles we can't see
          if (Playerlist::getViewingplayer()->getFogMap()->isFogged (pos) == true)
            continue;

          PixMask *tmp = 
            ImageCache::getInstance()->getShieldPic(1, (*it)->getOwner(),
                                                    true, 0)->copy();
          PixMask::scale(tmp, tmp->get_width()/1.4, tmp->get_height()/1.4);

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

