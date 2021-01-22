// Copyright (C) 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2004, 2005 Ulf Lorenz
// Copyright (C) 2004, 2006 Andrea Paternesi
// Copyright (C) 2007, 2008, 2009, 2014, 2015, 2021 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
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
#ifndef UNIQUELYIDENTIFIED_H
#define UNIQUELYIDENTIFIED_H

#include <gtkmm.h>

class XML_Helper;

//! A game object that has a unique numeric identifier.
class UniquelyIdentified
{
 public:
    //! Default constructor.
    UniquelyIdentified();

    //! Copy constructor.
    UniquelyIdentified(const UniquelyIdentified&, bool sync_ids = false);

    //! non-default constructor. 
    UniquelyIdentified(guint32 id);

    //! Loading constructor.
    UniquelyIdentified(XML_Helper* helper);

    //! Destructor.
    virtual ~UniquelyIdentified() {};


    // Get Methods

    //! Returns the unique numeric identifer of this object.
    guint32 getId() const {return d_id;}


    //Methods that operate on class data and modify the class.

    //! Make the counter aware of this object by syncing it to one after this.
    void syncNewId();

    //! Go get a new unique identifier for this object.
    void assignNewId();
        
    //! Whether or not this Id is unique (copied objects might not be unique)
    bool isUnique() {return d_unique;}
 protected:

    //! A unique numeric identifier for an object in the game.
    guint32 d_id;

    //! Whether or not this id is actually unique.
    bool d_unique;
};

#endif

// End of file
