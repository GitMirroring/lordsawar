//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2026 Ben Asselstine
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
#ifndef INPUT_EVENTS_H
#define INPUT_EVENTS_H

#include "vector.h"

//! A helper struct for representing an event involving the mouse button.
struct MouseButtonEvent
{
    Vector<int> pos;
    
    enum Button { LEFT_BUTTON, MIDDLE_BUTTON, RIGHT_BUTTON, 
      WHEEL_UP, WHEEL_DOWN };

    Button button;

    enum State { PRESSED, RELEASED };

    State state;
};

//! A helper struct for representing an event involving mouse pointer movement.
struct MouseMotionEvent
{
    Vector<int> pos;
    
    enum Button { LEFT_BUTTON, MIDDLE_BUTTON, RIGHT_BUTTON, END_MARKER };

    bool pressed[END_MARKER];
};

struct MouseButtonEvent to_input_event (int n, double x, double y, bool press);
#endif
