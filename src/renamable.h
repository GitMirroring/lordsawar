//  Copyright (C) 2008, 2009, 2014, 2020 Ben Asselstine
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
#ifndef RENAMABLE_H
#define RENAMABLE_H

#include "Named.h"

#include "defs.h"

class XML_Helper;

//! A game object that has a name that can be altered.
/** 
 * A Renamable is a game object that has a name that can be changed.
 */
class Renamable: private Named
{
 public:
     //! Default constructor.
     Renamable(Glib::ustring name) : Named(name) {}

     //! Copy constructor.
     Renamable(const Renamable& o) : Named(o) {}

     //! Loading constructor.
     Renamable(XML_Helper* helper) : Named(helper) {}

     //! Destructor.
     ~Renamable() {};

     // Get Methods

     //! Return the name of the object on the game map.
     Glib::ustring getName(bool translate = false) const 
       {
	 if (translate == true) 
	   return _(d_name.c_str());
	 else return d_name; 
       }

     // Set Methods

     //! Set the name of the object on the game map.
     void setName(Glib::ustring name) {d_name = name;}

};

#endif
