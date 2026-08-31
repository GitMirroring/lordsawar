//  Copyright (C) 2011, 2014, 2026 Ben Asselstine
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

#pragma once
#ifndef SELECT_CITY_MAP_H
#define SELECT_CITY_MAP_H

#include <sigc++/signal.h>

#include "overview-map.h"
#include "input-events.h"
#include "image-cache.h"

//! Draw all of the City objects onto a miniature map graphic.
/** 
 * 
 */

class SelectCityMap : public OverviewMap
{
 public:
     //! Which cities we're going to be allowed to click on.
     enum Type {
       NEUTRAL_CITY = 0,
       FRIENDLY_CITY,
       ENEMY_CITY,
       ANY_CITY
     };
     //! Default constructor.  Make a new SelectCityMap.
    SelectCityMap(SelectCityMap::Type type);

    //! Destructor
    ~SelectCityMap() {};
 
    ImageCache::CursorType get_cursor (double x, double y)
      {
        Vector<int> pos (x, y);

        Vector<int> tile = mapFromScreen (pos);
        if (is_hot (tile))
          return ImageCache::HAND_POINTER;
        return ImageCache::POINTER;
      }

    void setType(SelectCityMap::Type type)
      {
        d_type = type;
        create_hotmap ();
      }

    guint32 getType() const
      {
        return d_type;
      }

    City *get_selected_city() const
      {
        return d_selected_city;
      }

    void mouse_button_event(MouseButtonEvent e);

    //! Emitted when the cities are finished being drawn on the map surface.
    /**
     * Classes that use CityMap must catch this signal to display the map.
     */
    sigc::signal<void(Cairo::RefPtr<Cairo::Surface>)> map_changed;

    sigc::signal<void(City *)> city_selected;
    
 private:
    SelectCityMap::Type d_type;
    City *d_selected_city;
    
    void create_hotmap ();
    //! Draw the City objects onto the miniature map graphic.
    /**
     * This method is automatically called by the SelectCityMap::draw method.
     */
    virtual void after_draw();

};

#endif
