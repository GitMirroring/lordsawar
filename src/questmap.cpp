//  Copyright (C) 2007, 2008, 2009, 2010, 2014 Ben Asselstine
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
#include "questmap.h"

#include "Quest.h"
#include "QuestsManager.h"
#include "ImageCache.h"
#include "stacklist.h"
#include "player.h"
#include "maptile.h"
#include "GameMap.h"
#include "hero.h"

QuestMap::QuestMap(Quest *q)
{
    quest = q;
    d_target.x = -1;
    d_target.y = -1;
}

void QuestMap::draw_stacks(Player *p, std::list< Vector<int> > targets)
{
  Gdk::RGBA cross_color = p->getColor();
  int size = int(pixels_per_tile) > 1 ? int(pixels_per_tile) : 1;
        
  for (std::list< Vector<int> >::iterator it= targets.begin(); it != targets.end(); ++it)
  {
      Vector<int> pos = (*it);

      // don't draw stacks in cities, they could hardly be identified
      Maptile* mytile = GameMap::getInstance()->getTile(pos);
      if (mytile->getBuilding() == Maptile::CITY)
          continue;

      pos = mapToSurface(pos);
      draw_line(pos.x - size, pos.y, pos.x + size, pos.y, cross_color);
      draw_line(pos.x, pos.y - size, pos.x, pos.y + size, cross_color);
  }
}
void QuestMap::draw_target(Vector<int> start, Vector<int> target)
{
  Vector<int> end;
  end = target;

  start = mapToSurface(start);
  draw_target_box(end, QUESTMAP_TARGET_BOX_COLOR);
  end = mapToSurface(end);


  start += Vector<int>(int(pixels_per_tile/2), int(pixels_per_tile/2));
  end += Vector<int>(int(pixels_per_tile/2), int(pixels_per_tile/2));

  int xsize = 11;
  int ysize = 11;
  //which corner do we connect the line to?
  if (start.x >= end.x)
    {
      //westerly
      if (start.y >= end.y)
	//northerly
	//line is heading northwesterly.  
	//connect to the southeastern corner of the box.
	end += Vector<int>((xsize / 2) - 1, (ysize / 2) - 1);
      else
	//southerly
	//line is heading southwesterly.  
	//connect to the northeastern corner of the box.
	end += Vector<int>((xsize / 2) - 1, -(ysize / 2));
    }
  else
    {
      //easterly
      if (start.y >= end.y)
	//northerly
	//line is heading northeasterly.
	//connect to the southwestern corner of the box.
	end += Vector<int>(-(xsize / 2), (ysize / 2) - 1);
      else
	//southerly
	//line is heading southeasterly.
	//connect to the northwestern corner of the box.
	end += Vector<int>(-(xsize / 2), -(ysize / 2));
    }
  draw_line(start.x, start.y, end.x, end.y, QUEST_LINE_COLOR);
}

void QuestMap::after_draw()
{
  if (!quest)
    {
      draw_cities(true);
      map_changed.emit(surface);
      return;
    }
  Vector<int> start = 
    quest->getHero()->getOwner()->getStacklist()->getPosition (quest->getHeroId ());

  if (quest->isPendingDeletion() == false)
    {
      std::list< Vector<int> > targets = quest->getTargets();
      switch (quest->getType ())
        {
          case Quest::PILLAGEGOLD:
            draw_cities(true);
            break;
          case Quest::KILLARMIES:
          case Quest::KILLARMYTYPE:
            draw_cities(false);
            //for each target draw a plus sign
            draw_stacks(quest->getHero()->getOwner(), targets);
            break;
          case Quest::KILLHERO:
          case Quest::CITYSACK:
          case Quest::CITYOCCUPY:
          case Quest::CITYRAZE:
            draw_cities(false);
            //the target list should only have one position in it
            //draw an orange line to the target and put a box around it.
            std::list< Vector<int> >::iterator it = targets.begin();
            if (targets.size() > 0)
              draw_target(start, *it);
            break;
        }
    }

  draw_target();

  // draw the hero picture

  start = mapToSurface (start);

  start += Vector<int>(int (pixels_per_tile / 2), int (pixels_per_tile / 2));

  PixMask *heropic = ImageCache::getInstance()->getSmallHeroImage(true);
  heropic->blit_centered(surface, start);
  map_changed.emit(surface);
}

void QuestMap::draw_target()
{
  if (d_target.x == -1 && d_target.y == -1)
    return;
  Player *p = quest->getHero()->getOwner();
  Stacklist *sl = p->getStacklist();
  Vector<int> start = sl->getPosition (quest->getHeroId ());
  draw_target(start, d_target);
  map_changed.emit(surface);
}
