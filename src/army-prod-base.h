// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2007, 2008, 2014 Ben Asselstine
// Copyright (C) 2007, 2008 Ole Laursen
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
#ifndef ARMY_PRODBASE_H
#define ARMY_PRODBASE_H



#include "armyprotobase.h"
class ArmyProto;

class XML_Helper;

//! A basic set of properties belonging to an army production base.
/**
 * An army production base is the army type in a city that can be instantiated.
 */
class ArmyProdBase: public ArmyProtoBase
{
    public:

	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

	//! Copy constructor.
        ArmyProdBase(const ArmyProdBase& prodbase);

	//! Copy constructor.
        ArmyProdBase(const ArmyProto& prodbase);

	//! Loading constructor.
        ArmyProdBase(XML_Helper* helper);
        
	//! Destructor.
        ~ArmyProdBase() {};

	// Methods that operate on the class data and modify the class.

	//! Change the army production base to be just like another army.
	void morph(const ArmyProto *army);

	// Methods that operate on the class data and do not modify the class.

        //! Saves the Army prototype to an opened armyset file.
        virtual bool save(XML_Helper* helper) const;
        
        guint32 getTypeId() const {return d_type_id;};
    private:

        void setTypeId(guint32 id) {d_type_id = id;};

        guint32 d_type_id;

};

#endif // ARMY_PRODBASE_H
