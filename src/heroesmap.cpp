//  Copyright (C) 2007, 2008, 2009, 2014 Ben Asselstine
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
#include "heroesmap.h"

#include "playerlist.h"
#include "player.h"
#include "stacklist.h"
#include "hero.h"

HeroesMap::HeroesMap(const std::list<Hero*> &h)
 : heroes (h), active_hero (*(heroes.begin()))
{
}

void HeroesMap::after_draw()
{
  OverviewMap::after_draw();
  draw_cities(false);
  for (std::list<Hero*>::iterator it = heroes.begin(); it != heroes.end();
       ++it)
    {
      Player *player = Playerlist::getActiveplayer();
      Vector<int> pos = player->getStacklist()->getPosition((*it)->getId());
      if (*it == active_hero)
	draw_hero(pos, true);
      else
	draw_hero(pos, false);
    }

  map_changed.emit(surface);
}

void HeroesMap::mouse_button_event(MouseButtonEvent e)
{
  if (e.button == MouseButtonEvent::LEFT_BUTTON && 
      e.state == MouseButtonEvent::PRESSED)
    {
      Player *active = Playerlist::getActiveplayer();
      Vector<int> dest = mapFromScreen(e.pos);

      //is dest close to one of our heroes?
      Hero *hero = active->getStacklist()->getNearestHero(dest, 4);
      if (hero)
	{
	  active_hero = hero;
	  hero_selected.emit(hero);
	}
    }
}
