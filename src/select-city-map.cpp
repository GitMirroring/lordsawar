//  Copyright (C) 2011, 2014, 2017, 2026 Ben Asselstine
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
#include "select-city-map.h"

#include "city.h"
#include "city-list.h"
#include "player-list.h"
#include <assert.h>

SelectCityMap::SelectCityMap(SelectCityMap::Type type)
{
  d_type = type;
  create_hotmap ();
}

void SelectCityMap::after_draw()
{
  assert(surface);
  draw_cities(false);
  map_changed.emit(surface);
}

void SelectCityMap::mouse_button_event(MouseButtonEvent e)
{
  if (e.button == MouseButtonEvent::LEFT_BUTTON && 
      e.state == MouseButtonEvent::PRESSED)
    {
      Vector<int> dest = mapFromScreen(e.pos);
      if (!is_hot (dest))
        return;
      City* nearestCity = Citylist::instance()->getNearestVisibleCity(dest, 4);
      if (nearestCity)
        {
          bool valid = false;
          Player *owner = nearestCity->getOwner();
          switch (d_type)
            {
            case ANY_CITY:
              valid = true;
              break;
            case NEUTRAL_CITY:
              if (owner == Playerlist::getNeutral())
                valid = true;
              break;
            case FRIENDLY_CITY:
              if (owner == Playerlist::getActiveplayer())
                valid = true;
              break;
            case ENEMY_CITY:
              if (owner != Playerlist::getActiveplayer())
                valid = true;
              break;
            }
          if (valid)
            {
              draw();
              d_selected_city = nearestCity;
              draw_square_around_city(d_selected_city, SELECTED_CITY_BOX_COLOR);
              city_selected.emit(d_selected_city);
              map_changed.emit(surface);
            }
        }
    }
}

void SelectCityMap::create_hotmap ()
{
  clear_hotmap ();
  switch (d_type)
    {
    case ANY_CITY:
      for (auto c : *Citylist::instance ())
        if (!c->isBurnt () && !is_fogged (c))
          add_to_hotmap (*c);
      break;

    case NEUTRAL_CITY:
      for (auto c : *Citylist::instance ())
        if (!c->isBurnt () && !is_fogged (c) &&
            c->getOwner () == Playerlist::getNeutral ())
          add_to_hotmap (*c);
      break;

    case FRIENDLY_CITY:
      for (auto c : *Citylist::instance ())
        if (!c->isBurnt () && !is_fogged (c) &&
            c->getOwner () == Playerlist::getActiveplayer ())
          add_to_hotmap (*c);
      break;

    case ENEMY_CITY:
      for (auto c : *Citylist::instance ())
        if (!c->isBurnt () && !is_fogged (c) &&
            c->getOwner () != Playerlist::getActiveplayer ())
          add_to_hotmap (*c);
      break;
    }
}
