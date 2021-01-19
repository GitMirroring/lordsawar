// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2007, 2008, 2014, 2021 Ben Asselstine
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
#ifndef FL_COUNTER_H
#define FL_COUNTER_H

#include <gtkmm.h>
#include <sigc++/trackable.h>

class XML_Helper;

//! Generates unique numeric ids for game objects.
/** The purpose of this class is very simple. Each object (player etc.) has a
  * unique id by which it may be accessed (this isn't important for now, but
  * becomes crucial as soon as you play e.g. over the network). Therefore, each 
  * important game object queries this class for an id and gets a unique
  * identifier. The current counter position is saved together with a game.
  *
  * The implementation with the global variable could be changed in favour of
  * static functions...
  */

class FL_Counter : public sigc::trackable
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

        //! Makes a copy of the counter.
        FL_Counter* copy () {return new FL_Counter (*this);}

        //! Initialise the counter with a start value
        FL_Counter(guint32 start = 0);

        //! Load the counter. See XML_Helper for details.
        FL_Counter(XML_Helper* helper);

        //! Copy constructor.
        FL_Counter (const FL_Counter &f);

        //! Destructor.
        ~FL_Counter() {};

        //! Returns a unique id
        guint32 getNextId();

	void syncToId(guint32 id);

        //! Saves the current counter position
        bool save(XML_Helper* helper);

        //! Replace the current counter with another.
        static void reset (FL_Counter *f);
    private:
        guint32 d_curID;
};

extern FL_Counter* fl_counter;
#endif
