//  Copyright (C) 2008, 2009, 2014 Ben Asselstine
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
#ifndef PRODSLOT_H
#define PRODSLOT_H

#include <glibmm.h>

class XML_Helper;
class ArmyProdBase;

//! A placeholder for an army production object.
/** 
 * Slots can either be empty or point to an army production object.
 */

class ProdSlot
{
 public:
     //! The xml tag of this object in a saved-game file.
     static Glib::ustring d_tag;

     //! Default constructor.
     ProdSlot();

     //! Copy constructor.
     ProdSlot(const ProdSlot&);

     //! Loading constructor.
     ProdSlot(XML_Helper* helper);

     //! Destructor.
    ~ProdSlot();

    // Set Methods
 
    //! Assign the armyprodbase associated with this object.
    void setArmyProdBase(ArmyProdBase *prod) {d_armyprodbase = prod;};


    // Get Methods

    //! Return the armyprodbase associated with this object.
    ArmyProdBase *getArmyProdBase() const {return d_armyprodbase;};


    // Methods that operate on the class data but do not modify the class.

    //! Save the production slot to an opened saved-game file.
    bool save(XML_Helper *helper) const;


    // Methods that operate ont he class data and modify the class.
 
    //! Delete and remove the armyprodbase from this object.
    void clear();

 private:

    //! Callback to help in loading the armyprodbase into this object.
    bool load(Glib::ustring tag, XML_Helper *helper);

    //DATA

    //! The armyprodbase object that this slot contains.
    ArmyProdBase *d_armyprodbase;
};

#endif
