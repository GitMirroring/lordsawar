//  Copyright (C) 2007 Ole Laursen
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
#ifndef GUI_INPUT_HELPERS_H
#define GUI_INPUT_HELPERS_H

#include "input-events.h"

inline MouseButtonEvent to_input_event(GdkEventButton *e)
{
    MouseButtonEvent m;
    m.pos = make_vector(int(e->x), int(e->y));
    
    if (e->button == 1)
	m.button = MouseButtonEvent::LEFT_BUTTON;
    else if (e->button == 3)
	m.button = MouseButtonEvent::RIGHT_BUTTON;
    else if (e->button == 4)
	m.button = MouseButtonEvent::WHEEL_UP;
    else if (e->button == 5)
	m.button = MouseButtonEvent::WHEEL_DOWN;
    else
	m.button = MouseButtonEvent::MIDDLE_BUTTON;
    
    if (e->type == GDK_BUTTON_PRESS)
	m.state = MouseButtonEvent::PRESSED;
    else if (e->type == GDK_BUTTON_RELEASE)
	m.state = MouseButtonEvent::RELEASED;
    
    return m;
}

inline MouseMotionEvent to_input_event(GdkEventMotion *e)
{
    MouseMotionEvent m;
    m.pos = make_vector(int(e->x), int(e->y));
    
    m.pressed[MouseMotionEvent::LEFT_BUTTON] = e->state & GDK_BUTTON1_MASK;
    m.pressed[MouseMotionEvent::MIDDLE_BUTTON] = e->state & GDK_BUTTON2_MASK;
    m.pressed[MouseMotionEvent::RIGHT_BUTTON] = e->state & GDK_BUTTON3_MASK;
	    
    return m;
}

#endif
