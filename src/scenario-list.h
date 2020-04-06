//  Copyright (C) 2020 Ben Asselstine
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
#ifndef SCENARIO_LIST_H
#define SCENARIO_LIST_H

#include <gtkmm.h>
#include <list>

#include "scenario-details.h"

//! A list of scenarios we can play.
/** 
 * This class loads up the details of the scenarios in the system and
 * user directories.
 *
 * We use this list when presenting the list of maps we can load,
 * and for finding a unique map name in the editor.
 *
 * Implemented as a singleton.
 */
class ScenarioList : public std::list<ScenarioDetails*>
{
    public:

        //! Add a file to the list, returns true if it worked.
        bool add_file (Glib::ustring filename);

        //! Remove a file from the list, returns true if it worked.
        bool remove_file (Glib::ustring filename);

        //! Get a unique name among all maps we know about.
        /**
         * We add a number to make it unique. for example untitled 1,
         * untitled 2, etc.
         *
         * @param name the name we're going to search for.
         *
         * @return the unique name, or empty if we gave up.
         */
        Glib::ustring findFreeName (Glib::ustring name);

        //! Gets the singleton instance or creates a new one.
        static ScenarioList* getInstance();

        //! Explicitly deletes the singleton instance.
        static void deleteInstance();
        
        //! Our list is sorted by scenario name.
        static bool compare(const ScenarioDetails *l, const ScenarioDetails *r);

    protected:

	//! Default constructor.
        ScenarioList();

	//! Destructor.
        ~ScenarioList();

    private:

	// DATA

        //! A static pointer for the singleton instance.
        static ScenarioList* s_instance;
};

#endif
