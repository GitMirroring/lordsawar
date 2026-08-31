//  Copyright (C) 2003 Michael Bartl
//  Copyright (C) 2003, 2004, 2005 Ulf Lorenz
//  Copyright (C) 2007, 2008, 2014, 2021, 2026 Ben Asselstine
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
//  Foundation, Inc., 31 Milk Street #960789, Boston, MA 02196, USA.

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

class ID_Counter : public sigc::trackable
{
public:
    //! The xml tag of this object in a saved-game file.
    static Glib::ustring d_tag; 

    //! Makes a copy of the counter.
    ID_Counter* copy ()
      {
        return new ID_Counter (*this);
      }

    //! Initialize the counter with a start value
    ID_Counter ();

    //! Load the counter. See XML_Helper for details.
    ID_Counter (XML_Helper* helper);

    //! Copy constructor.
    ID_Counter (const ID_Counter &f);

    //! Destructor.
    ~ID_Counter ()
      {
      }

    //! Returns a unique id
    guint32 get_next_id ();

    void sync_to_id (guint32 id);

    //! Saves the current counter position
    bool save (XML_Helper* helper);

    //! Replace the current counter with another.
    static void reset (ID_Counter *f);
private:
    guint32 m_current_id;
};

extern ID_Counter* id_counter;
#endif
