//  Copyright (C) 2008, 2014 2014 Ben Asselstine
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
#ifndef RENAMABLE_LOCATION_H
#define RENAMABLE_LOCATION_H

#include "defs.h"
#include "vector.h"
#include "Location.h"
#include "Renamable.h"

//! Scenario Editor.  A game object that has a position and a changeable name.
class RenamableLocation: public Location, public Renamable
{
 public:
     //! Default constructor.
     RenamableLocation(Vector<int> pos);
     //! Destructor.
    ~RenamableLocation() {}

    Glib::ustring getDescription() const {return d_description;}
    void setDescription(Glib::ustring desc) {d_description = desc;}

 private:
    Glib::ustring d_description;
};

#endif

