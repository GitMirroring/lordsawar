//  Copyright (C) 2007, 2008, 2009, 2012, 2014 Ben Asselstine
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
#ifndef ARMYMAP_H
#define ARMYMAP_H

#include <sigc++/signal.h>

#include "overviewmap.h"

//! Display a miniature map that shows where Stack objects are on the game map.
/** 
 * This is a map where you can see all the armies in the field.  Army units
 * in City objects are not shown.
 */
class ArmyMap : public OverviewMap
{
 public:
    //! Default constructor.  Make a new ArmyMap.
    ArmyMap();

    //! Destructor.
    //~ArmyMap() {};

    //! Emitted when the Army units are finished being drawn on the map surface.
    /**
     * Classes that use ArmyMap must catch this signal to display the map.
     */
    sigc::signal<void, Cairo::RefPtr<Cairo::Surface> > map_changed;
    
 private:
    
    //! Draw the City objects and Stack objects onto the miniature map graphic.
    /**
     * This method is automatically called by the ArmyMap::draw method.
     */
    virtual void after_draw();

    //! Draw just the Stack objects onto the miniature map graphic.
    void draw_stacks();

};

#endif
