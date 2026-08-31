//  Copyright (C) 2009, 2012, 2014, 2026 Ben Asselstine
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
#ifndef HEROESMAP_H
#define HEROESMAP_H

#include <sigc++/signal.h>

#include "overview-map.h"
#include "input-events.h"
#include "image-cache.h"

class Hero;

//! Draw a miniature map graphic indicating where the Heroes are.
/** 
  * This is a map where you can highlight the position of heroes with a hero
  * icon.
  *
  * We can click on the map to select another hero.
  * In this way we have an active hero draw in white, and the other inactive
  * ones drawn in black.
  *
  */
class HeroesMap : public OverviewMap
{
public:
    //! Default constructor.  Make a new HeroesMap.
    /**
     * @param heroes the list of all heroes belonging to a player
     */
    HeroesMap(const std::list<Hero*> &heroes);

    //! Destructor.
    ~HeroesMap() {};

    //! Realize the mouse click.
    void mouse_button_event(MouseButtonEvent e);

    Hero *get_hero ()
      {
        return active_hero;
      }

    void set_hero (Hero *h)
      {
        active_hero = h;
      }

    ImageCache::CursorType get_cursor (double x, double y)
      {
        Vector<int> pos (x, y);

        Vector<int> tile = mapFromScreen (pos);
        if (is_hot (tile))
          return ImageCache::HAND_POINTER;
        return ImageCache::POINTER;
      }
    //! Emitted when the map graphic has been altered.
    /**
     * Classes that use HeroesMap must catch this signal to display the map.
     */
    sigc::signal<void(Cairo::RefPtr<Cairo::Surface>)> map_changed;

    //! Emitted when a hero is clicked on.
    sigc::signal<void(Hero*)> hero_selected;

private:
    //! The heroes to show on the map.
    std::list<Hero*> heroes;
    Hero *active_hero;

    void create_hotmap ();

    //! Draw the Hero icons onto the miniature map graphic.
    /**
     * This draws the shields for each city as well as the icon to indicate
     * that a Hero is there.
     *
     * This method is automatically called by the HeroesMap::draw method.
     */
    virtual void after_draw();

};

#endif
