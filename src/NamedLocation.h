//  Copyright (C) 2008, 2015, 2021 Ben Asselstine
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
#ifndef NAMEDLOCATION_H
#define NAMEDLOCATION_H

#include "vector.h"
#include "Location.h"
#include "Renamable.h"

class XML_Helper;

//! A game object that has a position and a name.
/** 
 * A NamedLocation is a game object on the map that has a position and a name.
 * It also has a description.
 */

class NamedLocation: public Location, public Renamable
{
 public:
     //! Default constructor.
     NamedLocation(Vector<int> pos, guint32 size, Glib::ustring name, 
		   Glib::ustring desc);
     //! Copy constructor.
     NamedLocation(const NamedLocation&, bool sync_id = false);

     NamedLocation(const NamedLocation&, Vector<int> pos);

     //! Loading constructor.
     NamedLocation(XML_Helper* helper, guint32 size);

     //! Destructor.
    virtual ~NamedLocation() {};

    Glib::ustring getDescription() const {return d_description;};

    void setDescription(Glib::ustring d) {d_description = d;};

 private:
    Glib::ustring d_description;
};

#endif
