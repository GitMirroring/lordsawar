// Copyright (C) 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2004 Thomas Plonka
// Copyright (C) 2007, 2008, 2009, 2014, 2017, 2020 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
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
#ifndef SMALLMAP_H
#define SMALLMAP_H

#include <sigc++/signal.h>
#include <sigc++/connection.h>
#include <sigc++/trackable.h>

#include "overviewmap.h"

#include "rectangle.h"
#include "input-events.h"

//! Draw a miniature map graphic and a white box around the selected region.
/**
 * SmallMap is the image at the top right of the game screen which provides an
 * overview of the whole game map.  It draws City objects on top of the graphic
 * that the OverviewMap class provides.  It is interactive in that it handles 
 * mouse clicks, moving of the currently visible portion and changes of the 
 * underlying map.  It draws an animation of the selection rectangle as it 
 * moves to the next selected region.
 */
class SmallMap: public OverviewMap
{
public:
    static bool s_quick;

    //! Default constructor.  Make a new SmallMap.
    SmallMap(bool headless);

    //! Destructor.
    ~SmallMap() {};

    // Set Methods
  
    //! Set whether or not the map can be clicked on.
    /**
     * @note This is useful when it's not our turn and we want to prevent the
     *       user from clicking on the SmallMap.
     */
    void set_input_locked(bool locked) { input_locked = locked; }

    void set_slide_speed(guint32 interval) {sleep_interval = interval;};

    // Methods that operate on the class data and modify the class.
 
    //! Set the portion of the SmallMap that has a white box around it.
    /**
     * @note This method does not perform animation.  The white box disappears
     * from it's previous location and a new one is drawn.
     *
     * @param new_view  The portion of the map graphic to highlight.
     */
    void set_view(const LwRectangle &new_view);

    LwRectangle get_view() const {return view;};

    //! Zip to the selected portion of the SmallMap from the old position.
    /**
     * Move the white box to the specified region in an animated fashion.
     *
     * @param new_view  The portion of the map graphic to highlight.
     */
    void slide_view(LwRectangle new_view);

    //! Realize the given mouse button event.
    void mouse_button_event(MouseButtonEvent e);

    //! Realize the given mouse motion event.
    void mouse_motion_event(MouseMotionEvent e);

    //! Center the view on the given map position.
    /**
     * @param pos        The position to move the little white box to.  The
     *                   referenced tile is in the center of the little white
     *                   box.
     * @param slide      Whether or not to animate the movement of the little
     *                   white box from it's current location to it's new
     *                   given location.
     */
    void center_view_on_tile(Vector<int> pos, bool slide);

    //! Center the view on the given pixel of the map graphic.
    /**
     * @param pos        The position to move the little white box to.  The
     *                   referenced pixel is in the center of the little white
     *                   box. 
     * @param slide      Whether or not to animate the movement of the little
     *                   white box from it's current location to it's new
     *                   given location.
     */
    void center_view_on_pixel(Vector<int> pos, bool slide);

    //! Center the little white box in the center of the map graphic.
    void center_view ();

    // Signals
 
    // Emitted when the white box is redrawn after a call to SmallMap::set_view.
    /**
     * Classes that use SmallMap must catch this signal to display the change
     * in position of the little white box.
     */
    sigc::signal<void, LwRectangle> view_changed;

    //! Emitted during sliding animation after a call to Smallmap::slide_view.
    /**
     * Classes that use SmallMap must catch this signal to display the 
     * animation of the little white box.
     */
    sigc::signal<void, LwRectangle> view_slid;

    // Emitted after a call to SmallMap::Draw.
    /**
     * Classes that use SmallMap must catch this signal to display the map.
     */
    sigc::signal<void, Cairo::RefPtr<Cairo::Surface>, Gdk::Rectangle> map_changed;

private:
    //! Draw the selection rectangle that shows the viewed portion of the map.
    void draw_selection();

    //! Draw the City objects and little white box onto the mini-map graphic.
    /**
     * This method is automatically called by the SmallMap::draw method.
     */
    virtual void after_draw();

    int slide (int x, int y);
    // DATA
 
    //! The position and size of the little white box.
    /**
     * This rectangle represents the selected portion of the map.
     */
    LwRectangle view;

    //! Whether or not to ignore mouse clicks and movement.
    /**
     * When this is set to true, the SmallMap will ignore mouse clicks and also
     * mouse drags.
     */
    bool input_locked;

    //! Whether or not the box is currently sliding to a new location
    bool sliding;

    //! The position that we're sliding to.
    LwRectangle sliding_to;

    //! When sliding the box, it sleeps this long per frame (in microseconds).
    guint32 sleep_interval;
};

#endif
