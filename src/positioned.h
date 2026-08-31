//  Copyright (C) 2008, 2026 Ben Asselstine
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
#ifndef POSITIONED_H
#define POSITIONED_H

#include "vector.h"

class XML_Helper;

//! A game object that can be positioned at least once on the game map.
/** 
 * An Positioned is an object on the map.
 */

class Positioned
{
 public:

     //! Default constructor.
     Positioned(Vector<int> pos);

     //! Copy constructor.
     Positioned(const Positioned&);

     //! Loading constructor.
     Positioned(XML_Helper* helper);

     //! Destructor.
    ~Positioned() {};
    
 protected:
    //! The position of the object on the game map.
    Vector<int> d_pos;

    Positioned& operator=(const Positioned& other)
      {
        if (this != &other)
          d_pos = other.d_pos;
        return *this;
      }
};

#endif
