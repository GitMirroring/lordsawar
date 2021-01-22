//  Copyright (C) 2020, 2021 Ben Asselstine
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
#ifndef KEEPER_H
#define KEEPER_H

#include "stack.h"
#include "vector.h"
#include "Renamable.h"

class XML_Helper;
class ArmyProto;

//! A keeper is a named stack that guards a ruin
/**
 * it resides in the ruin xml object.
 */

class Keeper : public Renamable
{
public:

    //! The xml tag of this object in a saved-game file.
    static Glib::ustring d_tag;

    //! Default constructor.
    Keeper(const ArmyProto *army, Vector<int> pos);

    //! Copy constructor.
    Keeper(const Keeper&);

    //! Loading constructor.
    Keeper (XML_Helper* helper);

    //! Destructor.
    ~Keeper() {};

    //! Add or replace the defender.
    void add (const ArmyProto *army, Vector<int> pos);

    //! Get the stack associated with this keeper.
    Stack *getStack () {return d_stack;}

    //! Get rid of the stack.
    void clearStack ();

    //! assign the name from the first army in the stack.
    void rename ();

    //! Save the stack to an opened saved-game file.
    bool save(XML_Helper* helper) const;

    //! Go get a army unit type that is allowed to defend a ruin.
    static const ArmyProto* randomRuinDefender();

private:

    Stack *d_stack;

    bool load (Glib::ustring tag, XML_Helper *helper);
};

#endif
