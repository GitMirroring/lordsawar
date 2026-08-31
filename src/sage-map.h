//  Copyright (C) 2007, 2008, 2009, 2012, 2014, 2026 Ben Asselstine
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
#ifndef SAGEMAP_H
#define SAGEMAP_H

#include <sigc++/signal.h>

#include "overview-map.h"
#include "input-events.h"
#include "named-location.h"
#include "image-cache.h"

//! Draw the ruins and temples onto a miniature map graphic.
/** 
  * This method draws Ruin and Temple objects onto a miniature map graphic.
  * The ruins and temples are depicted with icons instead of little white dots.
  *
  * The SageMap is non-interactive, a ruin is selected by the caller and
  * a line appears from one ruin to another.
  */
class SageMap : public OverviewMap
{
 public:
     //! Default constructor.  Make a new SageMap.
     /**
      * @param ruin  The Ruin or Temple object that is selected initially when
      *              the miniature map graphic is created.
      */
     SageMap(NamedLocation *ruin, Stack *stack);

     //! Destructor.
     ~SageMap() {};

     // Get Methods
  
    ImageCache::CursorType get_cursor (double, double)
      {
        return ImageCache::POINTER;
      }
     //! Return the Ruin or Temple object that is currently selected.
     NamedLocation * getNamedLocation () const {return ruin;}

     void set_target (NamedLocation *t)
       {
         target = t;
       }

     // Signals

     //! Emitted when the objects are finished being drawn on the map surface.
     /**
      * Classes that use SageMap  must catch this signal to display the map.
      */
     sigc::signal<void(Cairo::RefPtr<Cairo::Surface>)> map_changed;

 private:
     //! Draw the Ruin objects on the map.
     /**
      * @param show_selected  Whether or not to draw a box around a Ruin object
      *                       that is the selected object (SageMap::ruin).
      */
     void draw_ruins (bool show_selected);

     //! Draw the Temple objects on the map.
     /**
      * @param show_selected  Whether or not to draw a box around a Temple object
      *                       that is the selected object (SageMap::ruin).
      */
     void draw_temples (bool show_selected);

     //! Draw an orange line from src to dest, e.g. ruin to target
     void draw_target (Vector<int> src, Vector<int> dest);

     //! Draw the Ruin and Temple objects objects onto the miniature map graphic.
     /**
      * This method is automatically called by the SageMap::draw method.
      */
     virtual void after_draw();

     // DATA

     //! The currently selected Ruin or Temple object.
     NamedLocation *ruin;

     //! The target we're pointing to.
     NamedLocation *target;

     //! The stack doing the searching.
     Stack *stack;
};

#endif
