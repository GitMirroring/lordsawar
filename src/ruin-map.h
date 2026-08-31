// Copyright (C) 2007, 2008, 2009, 2012, 2014 Ben Asselstine
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
#ifndef RUINMAP_H
#define RUINMAP_H

#include <sigc++/signal.h>

#include "overviewmap.h"
#include "input-events.h"
#include "NamedLocation.h"

//! Draw the ruins and temples onto a miniature map graphic.
/** 
  * This method draws Ruin and Temple objects onto a miniature map graphic.
  * The ruins and temples are depicted with icons instead of little white dots.
  *
  * The RuinMap is interactive.  Each Ruin and Temple is selectable with the 
  * left mouse button.
  */
class RuinMap : public OverviewMap
{
 public:
     //! Default constructor.  Make a new RuinMap.
     /**
      * @param ruin  The Ruin or Temple object that is selected initially when
      *              the miniature map graphic is created.
      */
     RuinMap(NamedLocation *ruin, Stack *stack);

     //! Destructor.
     ~RuinMap() {};

     // Get Methods
  
     //! Return the Ruin or Temple object that is currently selected.
     NamedLocation * getNamedLocation () const {return ruin;}


     // Methods that operate on the class data and modify the class.
  
     //! Realize the given mouse button event.
     void mouse_button_event(MouseButtonEvent e);


     // Signals

     //! Emitted when the objects are finished being drawn on the map surface.
     /**
      * Classes that use RuinMap must catch this signal to display the map.
      */
     sigc::signal<void, Cairo::RefPtr<Cairo::Surface> > map_changed;

 private:
     //! Draw the Ruin objects on the map.
     /**
      * @param show_selected  Whether or not to draw a box around a Ruin object
      *                       that is the selected object (RuinMap::ruin).
      */
     void draw_ruins (bool show_selected);

     //! Draw the Temple objects on the map.
     /**
      * @param show_selected  Whether or not to draw a box around a Temple object
      *                       that is the selected object (RuinMap::ruin).
      */
     void draw_temples (bool show_selected);

     //! Draw the Ruin and Temple objects objects onto the miniature map graphic.
     /**
      * This method is automatically called by the RuinMap::draw method.
      */
     virtual void after_draw();

     // DATA

     //! The currently selected Ruin or Temple object.
     NamedLocation *ruin;

     //! The stack doing the searching.
     Stack *stack;

};

#endif
