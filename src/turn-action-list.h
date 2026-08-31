//  Copyright (C) 2017, 2021 Ben Asselstine
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
#ifndef TURN_ACTION_LIST_H
#define TURN_ACTION_LIST_H

#include <gtkmm.h>
#include <list>
#include "action.h"
#include <sigc++/trackable.h>
#include "OwnerId.h"

//! A list of actions performed by a player during their turn.
/** 
 * Each one of these is associated with a player.
 * This object is equivalent to a <turn> object in the saved-game file.
 * These get aggregated into a game-actionlist object.
 *
 */
class TurnActionlist : public std::list<Action*>, public OwnerId, public sigc::trackable
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

	//! Default constructor.
        TurnActionlist (const Player *p, const std::list<Action*> &actions);

	//! Loading constructor.
        TurnActionlist (XML_Helper* helper);

        //! Copy constructor
        TurnActionlist (const TurnActionlist &c);

	//! Destructor.
        ~TurnActionlist ();

        //! Methods that modify the class.
        void add(const std::list<Action*> &actions);


	// Methods that operate on the class data but do not modify the class.

        //! Save the list of action objects to a saved-game file.
        bool save(XML_Helper* helper) const;

    private:

        //! Callback for loading the TurnActionlist from a saved-game file.
        bool load(Glib::ustring tag, XML_Helper* helper);

        // Helpers

        void add(const Action* action);
};

#endif
