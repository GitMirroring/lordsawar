//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2020, 2021 Ben Asselstine
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
#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "vector.h"
#include <list>

//! A general purpose rectangle.
class LwRectangle
{
public:
    LwRectangle() : x(pos.x), y(pos.y), w(dim.x), h(dim.y) {}

    LwRectangle(int x_, int y_, int w_, int h_)
	: pos(x_, y_), dim(w_, h_), x(pos.x), y(pos.y), w(dim.x), h(dim.y) {}

    LwRectangle(Vector<int> pos_)
	: pos(pos_), dim(Vector<int>(1,1)), x(pos.x), y(pos.y), w(dim.x), h(dim.y) {}
    
    LwRectangle(Vector<int> pos_, Vector<int> dim_)
	: pos(pos_), dim(dim_), x(pos.x), y(pos.y), w(dim.x), h(dim.y) {}
    
    LwRectangle(const LwRectangle &other)
	: pos(other.pos), dim(other.dim), x(pos.x), y(pos.y), w(dim.x), h(dim.y) {}

    const LwRectangle &operator=(const LwRectangle &other)
    {
	pos = other.pos;
	dim = other.dim;
        x = pos.x; y = pos.y;
        w = dim.x; h = dim.y;
	return *this;
    }

    //! Grow the rectangle by N on all sides
    inline void grow (int n)
      {
        pos.x -= n;
        if (pos.x < 0)
          pos.x = 0;
        pos.y -= n;
        if (pos.y < 0)
          pos.y = 0;
        dim.x += n * 2;
        dim.y += n * 2;
      }

    Vector<int> pos, dim; // position and dimensions

    std::list<Vector<int> > getPoints ()
      {
        std::list<Vector<int> > points;
        for (int i = 0; i < w; i++)
          for (int j = 0; j < h; j++)
            points.push_back (Vector<int>(x, y) + Vector<int> (i, j));
        return points;
      }
        // accessors - sometimes it's easier with .x instead of .pos.x
        int &x, &y, &w, &h;
      };

inline bool operator==(const LwRectangle &lhs, const LwRectangle &rhs)
{
    return lhs.pos == rhs.pos && lhs.dim == rhs.dim;
}

inline bool operator!=(const LwRectangle &lhs, const LwRectangle &rhs)
{
    return !(lhs == rhs);
}

inline bool is_inside(const LwRectangle &r, Vector<int> v)
{
    return r.x <= v.x && v.x < r.x + r.w
	&& r.y <= v.y && v.y < r.y + r.h;
}

#endif
