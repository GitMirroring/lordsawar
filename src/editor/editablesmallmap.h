// Copyright (C) 2010, 2014, 2015 Ben Asselstine
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
#ifndef EDITABLESMALLMAP_H
#define EDITABLESMALLMAP_H

#include <sigc++/signal.h>
#include <sigc++/connection.h>
#include <sigc++/trackable.h>

#include "overviewmap.h"

#include "input-events.h"

//! Draw a miniature map graphic and let it be changeable.
/**
 */
class EditableSmallMap: public OverviewMap
{
public:
    enum Pointer {
	POINTER = 0, 
	TERRAIN, 
	CITY, 
	RUIN, 
	TEMPLE, 
	ERASE, 
        PICK_NEW_ROAD_START,
        PICK_NEW_ROAD_FINISH
    };

    //! Default constructor.  Make a new EditableSmallMap.
    EditableSmallMap();

    //! Destructor.
    ~EditableSmallMap() {}


    // Get Methods
    
    //! Get an image of the mouse cursor.
    Glib::RefPtr<Gdk::Pixbuf> get_cursor(Vector<int> &hotspot) const;

    // Set Methods
  
    //! Set the pointer characteristics.
    void set_pointer(Pointer pointer, int size, Tile::Type terrain);

    void set_road_start(Vector<int> start);

    void set_road_finish(Vector<int> finish);

    void clear_road();

    // Methods that operate on the class data and modify the class.
 
    //! Realize the given mouse button event.
    void mouse_button_event(MouseButtonEvent e);

    //! Realize the given mouse motion event.
    void mouse_motion_event(MouseMotionEvent e);

    //! make a road from road_start to road_finish.
    bool create_road();

    //! check to see if the road can be made.
    bool check_road();

    // Signals

    // Emitted after a call to EditableSmallMap::Draw.
    /**
     * Classes that use EditableSmallMap must catch this signal to display the map.
     */
    sigc::signal<void, Cairo::RefPtr<Cairo::Surface>, Gdk::Rectangle > map_changed;
    sigc::signal<void, Vector<int> > road_start_placed;
    sigc::signal<void, Vector<int> > road_finish_placed;
    sigc::signal<void, bool> road_can_be_created;
    sigc::signal<void> map_edited;

private:

    //! Draw the City objects and little white box onto the mini-map graphic.
    /**
     * This method is automatically called by the EditableSmallMap::draw method.
     */
    virtual void after_draw();

    void change_map(Vector<int> pos);


    Rectangle get_cursor_rectangle(Vector<int> current_tile);
    // DATA
 
    Pointer pointer;
    Tile::Type pointer_terrain;
    int pointer_size;
    Vector<int> road_start;
    Vector<int> road_finish;
};

#endif
