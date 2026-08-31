// Copyright (C) 2007, 2009, 2014 Ben Asselstine
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
#ifndef CITYMAP_H
#define CITYMAP_H

#include <sigc++/signal.h>

#include "overviewmap.h"

//! Draw all of the City objects onto a miniature map graphic.
/** 
 * 
 */

class CityMap : public OverviewMap
{
 public:
    //! Default constructor.  Make a new CityMap.
    CityMap();

    //! Destructor.
    ~CityMap() {};

    //! Emitted when the cities are finished being drawn on the map surface.
    /**
     * Classes that use CityMap must catch this signal to display the map.
     */
    sigc::signal<void, Cairo::RefPtr<Cairo::Surface> > map_changed;
    
 private:
    
    //! Draw the City objects onto the miniature map graphic.
    /**
     * This method is automatically called by the CityMap::draw method.
     */
    virtual void after_draw();

};

#endif
