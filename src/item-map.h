//  Copyright (C) 2010, 2012, 2014 Ben Asselstine
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
#ifndef ITEMMAP_H
#define ITEMMAP_H

#include <sigc++/signal.h>

#include "overviewmap.h"

class Stack;
class MapBackpack;

//! Draw a miniature map graphic with an indication of where items are.
/** 
 * This draws stacks that have heroes that have items, as well as bags that
 * are dropped onto the map.
 */
class ItemMap : public OverviewMap
{
 public:
     //! Default constructor.  Make a new ItemMap.
     /**
      * @param item_laden_stacks A list of stacks that contain at least one
      *                          hero that has at least one item.
      * @param bags              A list of mapbackpack objects, which are bags
      *                          of stuff dropped on the map.
      */
     ItemMap(const std::list<Stack*> &item_laden_stacks,
             const std::list<MapBackpack*> &bags);

     //! Destructor.
     ~ItemMap() {};

    //! Emitted when the icons are finished being drawn on the map surface.
    /**
     * Classes that use ItemMap must catch this signal to display the map.
     */
    sigc::signal<void, Cairo::RefPtr<Cairo::Surface> > map_changed;
    
 private:
    //! The bags to draw bag icons for.
    std::list<MapBackpack *>bags;

    //! The item laden stacks to draw hero icons for.
    std::list<Stack *>stacks;
    
    //! Draw the bag icons on the miniature map graphic.
    void draw_bags();

    //! Draw a bag icon at the given tile on the miniature map graphic.
    void draw_bag(Vector<int> pos);

    //! Draw the hero icons on the miniature map graphic.
    void draw_heroes();

    //! Draw the hero and bag icons onto the miniature map graphic.
    /**
     * This draws the shields for each city as well as icons to indicate
     * where the items are.
     *
     * This method is automatically called by the ItemMap::draw method.
     */
    virtual void after_draw();
};

#endif
