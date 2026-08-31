//  Copyright (C) 2009, 2012, 2014 Ben Asselstine
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

#pragma once
#ifndef HEROESMAP_H
#define HEROESMAP_H

#include <sigc++/signal.h>

#include "overviewmap.h"
#include "input-events.h"

class Hero;

//! Draw a miniature map graphic with an indication of where a Hero is.
/** 
  * This is a map where you can highlight a city with a hero icon.  This
  * draws the shields for City objects and the icon for the Hero.
  *
  * @note This is used to show a map when a Hero initially emerges from a City.
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

     //! Return the currently selected/active hero.
     Hero *getSelectedHero() {return active_hero;};
     void setSelectedHero(Hero *h) {active_hero = h;};

    //! Emitted when the map graphic has been altered.
    /**
     * Classes that use HeroesMap must catch this signal to display the map.
     */
    sigc::signal<void, Cairo::RefPtr<Cairo::Surface> > map_changed;

    //! Emitted when a hero is clicked on.
    sigc::signal<void, Hero* > hero_selected;
    
 private:
    //! The heroes to show on the map.
    std::list<Hero*> heroes;
    Hero *active_hero;
    
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
