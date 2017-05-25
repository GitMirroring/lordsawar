//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014, 2017 Ben Asselstine
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
#ifndef EDITORBIGMAP_H
#define EDITORBIGMAP_H

#include <vector>
#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <sigc++/connection.h>

#include "vector.h"
#include "input-events.h"
#include "bigmap.h"
#include "Tile.h"
#include "UniquelyIdentified.h"

//! Scenario editor.  Specializatoin of the BigMap class for the editor.
class EditorBigMap: public BigMap
{
 public:
    EditorBigMap();
    ~EditorBigMap() {};

    enum Pointer {
	POINTER = 0, 
	TERRAIN, 
	STACK, 
	CITY, 
	RUIN, 
	TEMPLE, 
	SIGNPOST,
	ROAD, 
	ERASE, 
	MOVE, 
	PORT, 
	BRIDGE,
	BAG,
	FIGHT,
        STONE
    };
    void set_pointer(Pointer pointer, int size, Tile::Type terrain, 
		     int tile_style_id);

    void mouse_button_event(MouseButtonEvent e);
    void mouse_motion_event(MouseMotionEvent e);
    void mouse_leave_event();

    // something was selected
    typedef std::vector<UniquelyIdentified*> map_selection_seq;
    sigc::signal<void, map_selection_seq> objects_selected;

    // emitted whenever the user moves the mouse to a new tile
    sigc::signal<void, Vector<int> > mouse_on_tile;

    // emitted when the map is changed by the user
    sigc::signal<void, Rectangle> map_tiles_changed;

    // emitted when the water on the map is altered.
    sigc::signal<void> map_water_changed;

    // emitted when the water on the map is altered.
    sigc::signal<void, Vector<int> > bag_selected;
    sigc::signal<void, Stack*> stack_selected_for_battle_calculator;

    void smooth_view();

 private:
    Vector<int> prev_mouse_pos, mouse_pos;

    Pointer pointer;
    Tile::Type pointer_terrain;
    int pointer_size;
    int pointer_tile_style_id;
    //! moving sets if we're moving objects on the map via the move button
    Vector<int> moving_objects_from;

    enum {
	NONE, DRAGGING, MOVE_DRAGGING
    } mouse_state;

    virtual void after_draw();
    int tile_to_road_type(Vector<int> tile);
    int tile_to_bridge_type(Vector<int> tile);
    void change_map_under_cursor();
    std::vector<Vector<int> > get_cursor_tiles();
    Rectangle get_cursor_rectangle();
    std::vector<Vector<int> > get_screen_tiles();
    void bring_up_details();
    void display_moving_building(Vector<int> src, Vector<int> dest);
};

#endif
