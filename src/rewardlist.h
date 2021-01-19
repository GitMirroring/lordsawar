//  Copyright (C) 2007, 2008, 2014, 2015, 2017, 2020, 2021 Ben Asselstine
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
#ifndef REWARDLIST_H
#define REWARDLIST_H

#include <list>
#include <sigc++/trackable.h>
#include <gtkmm.h>
#include "reward.h"

class XML_Helper;

//! A list of unique Reward objects in the game.
/**
  * Some rewards like gold, and allies can be created whenever they're needed,
  * but other rewards are unique in nature.  This list is for those unique
  * rewards -- namely item rewards, and hidden ruins.
  *
  */
class Rewardlist : public std::list<Reward*>, public sigc::trackable
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

        //! Makes a copy of the rewardlist.
        Rewardlist* copy () {return new Rewardlist (*this);}

	// Methods that operate on the class data and modify the class.

        //! deletes a reward from the list
        void deleteReward(const Reward* s);

	//! Return a random reward from the list and remove it.
        Reward *pop (Reward::Type type);

        //! Behaves like std::list::clear(), but frees pointers as well
        void flClear();

        //! Behaves like std::list::erase(), but frees pointers as well
        iterator flErase(iterator object);

        //! Behaves like std::list::remove(), but frees pointers as well
        bool flRemove(const Reward* object);


	// Methods that operate on the class data and do not modify the class.

        //! Save the data. See XML_Helper for details
        bool save(XML_Helper* helper) const;


	// Static Methods

        //! Returns the singleton instance. Creates a new one if required.
        static Rewardlist* getInstance();

        //! Loads the singleton instance with a savegame.
        static Rewardlist* getInstance(XML_Helper* helper);

        //! Explicitly deletes the singleton instance.
        static void deleteInstance();
        
        //! Replace the current rewardlist with another.
        static void reset (Rewardlist *r);

	//! Destructor.
        ~Rewardlist();

    protected:    

	// Constructor.
        Rewardlist();

        //! Copy constructor
        Rewardlist (const Rewardlist &r);

	//! Copy constructor.
        Rewardlist(Rewardlist *rewardlist);

	//! Loading constructor.
        Rewardlist(XML_Helper* helper);

    private:
        //! Callback function for loading rewards.
        bool load(Glib::ustring tag, XML_Helper* helper);

	// DATA

        static Rewardlist* s_instance;
};

#endif // REWARDLIST_H

// End of file
