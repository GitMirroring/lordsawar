//  Copyright (C) 2008, Ben Asselstine
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
#ifndef NAMED_H
#define NAMED_H

#include <glibmm.h>

class XML_Helper;

//! A game object that has a name.
/** 
 * An Named is an object on the map.
 */
class Named
{
 public:
     //! Default constructor.
     Named(Glib::ustring name);
     //! Copy constructor.
     Named(const Named&);
     //! Loading constructor.
     Named(XML_Helper* helper);
     //! Destructor.
    ~Named() {};
    
 protected:
    //! The name of the object.
    Glib::ustring d_name;
};

#endif
