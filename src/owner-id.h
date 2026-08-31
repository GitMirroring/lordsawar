//  Copyright (C) 2009, 2014, 2021 Ben Asselstine
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
#ifndef OWNER_ID_H
#define OWNER_ID_H


#include <gtkmm.h>
class XML_Helper;
class Player;

//! A game object that refers to an owner
/** 
 * An OwnerID is an object that refers to a particular player.
 */

class OwnerId
{
 public:

     //! Default constructor.
     OwnerId(guint32 owner);

     //! Alternate constructor.
     OwnerId(Player *p);

     //! Copy constructor.
     OwnerId(const OwnerId&);

     //! Loading constructor.
     OwnerId(XML_Helper* helper);

     //! Destructor.
    virtual ~OwnerId() {};
    
    // Get Methods

    guint32 getOwnerId() const {return d_owner_id;}

    //! Return the Player who this id refers to
    Player *getOwner() const ;

    // Set Methods

    void setOwner (Player *p);
    void setOwnerId(guint32 owner){d_owner_id = owner; owner_id_set = true;};

    bool save(XML_Helper *helper) const;

    bool isOwnerIdSet () const {return owner_id_set;}

    bool isFriend(Player *opponent) const;
    // Static Methods

    //! Callback for loading an OwnerId object from an opened saved-game file.
    static OwnerId load(XML_Helper *helper);

 protected:

    OwnerId();

    guint32 d_owner_id;
    bool owner_id_set;
};

#endif
