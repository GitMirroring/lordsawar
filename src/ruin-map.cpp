// Copyright (C) 2007, 2008, 2009, 2014, 2017 Ben Asselstine
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
#include "ruinmap.h"

#include "playerlist.h"
#include "ImageCache.h"
#include "player.h"
#include "ruinlist.h"
#include "templelist.h"
#include "GameMap.h"

RuinMap::RuinMap(NamedLocation *r, Stack *s)
{
  ruin = r;
  stack = s;
}

void RuinMap::draw_ruins (bool show_selected)
{
  // Draw all ruins as pictures over their location -- showing them as
  // explored/unexplored
  for (Ruinlist::iterator it = Ruinlist::getInstance()->begin();
      it != Ruinlist::getInstance()->end(); ++it)
  {
      if ((*it)->isHidden() == true && 
          (*it)->getOwner() != Playerlist::getInstance()->getViewingplayer())
        continue;
      if ((*it)->isVisible(Playerlist::getViewingplayer()) == false)
        continue;
      PixMask *tmp;
      if ((*it)->isSearched())
        tmp = ImageCache::getInstance()->getSmallRuinExploredImage();
      else
        {
          if ((*it)->getType() == Ruin::STRONGHOLD)
            tmp = ImageCache::getInstance()->getSmallStrongholdUnexploredImage();
          else
            tmp = ImageCache::getInstance()->getSmallRuinUnexploredImage();
        }
  
      Vector<int> pos = (*it)->getPos();
      pos = mapToSurface(pos);
      tmp->blit_centered(surface, pos);
      if (show_selected)
        {
          if ((*it)->getId() == ruin->getId()) //is this the selected ruin?
            {
              for (int i = 0; i <= 4; i += 2)
                {
                  draw_rect(pos.x - (tmp->get_width()/2) - i, 
                            pos.y - (tmp->get_height()/2) - i, 
                            tmp->get_width() + (i * 2),
                            tmp->get_height() + (i * 2),
                            ACTIVE_RUIN_BOX);
                }
            }
        }
  }
}

void RuinMap::draw_temples (bool show_selected)
{
  // Draw all temples as pictures over their location
  for (auto it: *Templelist::getInstance())
  {
      if (it->isVisible(Playerlist::getViewingplayer()) == false)
        continue;
  
      Vector<int> pos = it->getPos();
      pos = mapToSurface(pos);
      PixMask *templepic = ImageCache::getInstance()->getSmallTempleImage();
      templepic->blit_centered(surface, pos);
      if (show_selected)
        {
          if (it->getId() == ruin->getId()) //is this the selected ruin?
            {
              for (int i = 0; i <= 4; i += 2)
                {
                  PixMask *tmp = templepic;
                  draw_rect(pos.x - (tmp->get_width()/2) - i, 
                            pos.y - (tmp->get_height()/2) - i, 
                            tmp->get_width() + (i * 2),
                            tmp->get_height() + (i * 2),
                            ACTIVE_RUIN_BOX);
                }
            }
        }
  }
}

void RuinMap::after_draw()
{
  draw_cities(true);
  bool show_selected = true;
  if (ruin == NULL)
    show_selected = false;
  draw_ruins (show_selected);
  draw_temples (show_selected);
  if (stack)
    draw_hero (stack->getPos(), true);
  map_changed.emit(surface);
}

void RuinMap::mouse_button_event(MouseButtonEvent e)
{
  if (e.button == MouseButtonEvent::LEFT_BUTTON && 
      e.state == MouseButtonEvent::PRESSED)
    {
      Vector<int> dest = mapFromScreen(e.pos);

      Ruin *nearestRuin = 
        Ruinlist::getInstance()->getNearestVisibleRuin(dest, 4);
      if (nearestRuin)
	{
	  ruin = nearestRuin;
          draw();
	}
      else
	{
          Temple *nearestTemple =
            Templelist::getInstance()->getNearestVisibleTemple(dest, 4);
          if (nearestTemple)
	    {
	      ruin = nearestTemple;
              draw();
	    }
	}
    }
}
