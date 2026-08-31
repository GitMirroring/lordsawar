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
#ifndef GAME_ACTION_LIST_H
#define GAME_ACTION_LIST_H

#include <gtkmm.h>
#include <list>
#include "turn-actionlist.h"
#include <sigc++/trackable.h>

class Action;

//! A list of actions, stored by turn.
/** 
 * All actions taken by players are stored here.
 * The actions are piled up in the player object, but at the start of the
 * turn, the actions from the previous turn get dumped here.
 * They get dumped into this store as a single set of actions, called a
 * turnactionlist.
 * This object is equivalent to a <turnlist> object in the saved-game file.
 *
 * Implemented as a singleton.
 */
class GameActionlist : public std::list<TurnActionlist*>, public sigc::trackable
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

        //! Makes a copy of the gameactionlist.
        GameActionlist* copy () {return new GameActionlist (*this);}

        void add(TurnActionlist *t);

	// Methods that operate on the class data but do not modify the class.

        //! Save the list of NetworkAction objects to a saved-game file.
        bool save(XML_Helper* helper) const;

	// Static Methods

        //! Gets the singleton instance or creates a new one.
        static GameActionlist * getInstance();

        //! Loads the GameActionlist from a saved-game file.
	/**
	 * Load all NetworkAction objects in the GameActionlist from a 
	 * saved-game file.
	 *
	 * @param helper     The opened saved-game file to read from.
	 *
	 * @return The loaded GameActionlist.
	 */
        static GameActionlist* getInstance(XML_Helper* helper);

        //! Explicitly deletes the singleton instance.
        static void deleteInstance();
        
        //! Replace the current gameactionlist with another.
        static void reset (GameActionlist *g);
    protected:

	//! Default constructor.
        GameActionlist ();

	//! Loading constructor.
        GameActionlist (XML_Helper* helper);

        //! Copy constructor
        GameActionlist (const GameActionlist &g);

	//! Destructor.
        ~GameActionlist ();

    private:

        //! Callback for loading the GameActionlist from a saved-game file.
        bool load(Glib::ustring tag, XML_Helper* helper);

        // DATA
 
        //! A static pointer for the singleton instance.
        static GameActionlist * s_instance;
};

#endif
