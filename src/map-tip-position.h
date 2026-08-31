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
//  Foundation, Inc., 31 Milk Street #960789, Boston, MA 02196, USA.

#pragma once
#ifndef MAP_TIP_POSITION_H
#define MAP_TIP_POSITION_H

#include "vector.h"

//! A helper struct to state where on the map a tooltip should be displayed.
struct MapTipPosition
{
    // position in pixels in the GameBigMap screen surface
    Vector<int> pos;		

    // the requested justification
    enum {
	LEFT, RIGHT, TOP, BOTTOM
    } justification;
};

#endif
