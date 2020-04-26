//  Copyright (C) 2007, Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2011, 2014, 2015 Ben Asselstine
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
#ifndef GAMEBIGMAP_H
#define GAMEBIGMAP_H

#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <sigc++/connection.h>

#include "vector.h"
#include "input-events.h"
#include "ImageCache.h"
#include "bigmap.h"
#include "PixMask.h"

class Stack;
class City;
class Ruin;
class Signpost;
class Temple;
class PathCalculator;

//! like the BigMap but specifically for the game and not the scenario editor.
/** Specialization of BigMap for the game (as opposed to the editor)
  */
class GameBigMap: public BigMap
{
 public:
    GameBigMap(bool headless, bool intense_combat, bool see_opponents_production, bool see_opponents_stacks, bool military_advisor);
    virtual ~GameBigMap();

    // will center the bigmap on the stack
    void select_active_stack();
    void unselect_active_stack();

    void mouse_button_event(MouseButtonEvent e);
    void mouse_motion_event(MouseMotionEvent e);
    void set_shift_key_down (bool down);
    bool is_shift_key_down () const {return control_key_is_down;}
    void set_control_key_down (bool down);
    bool is_control_key_down () const {return control_key_is_down;}
    void update_mouse_cursor();

    // whether the map accepts input events
    void set_input_locked(bool locked) { input_locked = locked; }

    // signals for mouse clicks, deselect is signified with a null pointer
    sigc::signal<void, Stack*> stack_selected;
    sigc::signal<void, Stack*> stack_grouped_or_ungrouped;
    sigc::signal<void, City*> city_visited;  //for citywindow
    sigc::signal<void, Vector<int>, City*> city_queried;  //for city-info-tip
    sigc::signal<void> city_unqueried;
    sigc::signal<void, Ruin*, bool> ruin_queried; //true => show brief info
    sigc::signal<void, Signpost*> signpost_queried;
    sigc::signal<void, Vector<int> > stack_queried;
    sigc::signal<void> stack_unqueried;
    sigc::signal<void, Temple*, bool> temple_queried; //true=>show brief info
    sigc::signal<void, Vector<int>, guint32> path_turns;
    sigc::signal<void, Stack*> popup_stack_actions_menu;

    // emitted when a path for a stack is set
    sigc::signal<void> path_set;
    // emitted when the cursor changes
    sigc::signal<void, ImageCache::CursorType> cursor_changed;
    sigc::signal<void, Vector<int> > pointing_at_new_tile;

    void reset_path_calculator(Stack *s);

    bool d_intense_combat; 
    bool d_see_opponents_production;
    bool d_see_opponents_stacks;
    bool d_military_advisor;
 private:
    Vector<int> current_tile, prev_mouse_pos;
    
    enum mouse_state_enum {
	NONE, DRAGGING_MAP, SHOWING_CITY, SHOWING_RUIN,
	SHOWING_TEMPLE, SHOWING_SIGNPOST, SHOWING_STACK,
	DRAGGING_STACK, DRAGGING_ENDPOINT
    } mouse_state;
    bool shift_key_is_down;
    bool control_key_is_down;
	
    ImageCache::CursorType d_cursor;

    void determine_mouse_cursor(Stack *stack, Vector<int> tile);

    // for the marching ants around selected stack
    sigc::connection selection_timeout_handler;
    bool on_selection_timeout();
    void get_selector_frame_limits (Player *p, int &limitbig, int &limitsmall);

    virtual void after_draw();
    PathCalculator *path_calculator;
};

#endif
