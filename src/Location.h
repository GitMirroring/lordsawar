// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2000, 2001, 2002, 2004, 2005 Ulf Lorenz
// Copyright (C) 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2014 Ben Asselstine
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
#ifndef LOCATION_H
#define LOCATION_H

#include "vector.h"
#include "LocationBox.h"
#include "UniquelyIdentified.h"

//! A feature constructed on the game map.
/** 
 * A Location is a map feature with a location, and a size. 
 * City, Ruin, Temple, Signpost and more classes are derived from Location.
 */
class Location : public UniquelyIdentified, public LocationBox
{
 public:
     //! Default constructor.
     /**
      * @param pos     The top-right corner of the feature is located at this
      *                position on the game map.
      * @param size    The number of tiles wide and high the feature is.
      */
     Location(Vector<int> pos, guint32 size = 1);

     //! Copy constructor.
     Location(const Location&, bool sync_ids = false);

     //! Alternative copying constructor to change the position of the object.
     Location(const Location&, Vector<int> pos);

     //! Loading constructor.
     /**
      * Load the location from an opened saved-game file.
      *
      * @param helper  The opened saved-game file to read the location from.
      * @param size    The size of the feature.  This value is not read in
      *                from the saved-game file.
      */
     Location(XML_Helper* helper, guint32 size = 1);

     //! Destructor.
    ~Location() {};
};

#endif
