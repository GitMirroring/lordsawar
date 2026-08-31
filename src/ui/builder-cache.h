//  Copyright (C) 2015 Ben Asselstine
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
#ifndef BUILDER_CACHE_H
#define BUILDER_CACHE_H

#include <map>
#include <gtkmm.h>

//! A store of Gtk::Builder objects.
/**
 * Why re-instantiate builder objects when we can just open them once?
 *
 */
class BuilderCache: public std::map<Glib::ustring, Glib::RefPtr<Gtk::Builder> >
{
    public:
        //! Returns the singleton instance. Creates a new one if required.
        static BuilderCache* getInstance();

        //! Explicitly deletes the singleton instance.
        static void deleteInstance();

        //! Go get a builder object by name.
        static Glib::RefPtr<Gtk::Builder> get(Glib::ustring f);

        //! Go get a builder object by name.
        static Glib::RefPtr<Gtk::Builder> editor_get(Glib::ustring f);

    protected:    

	// Constructor.
        BuilderCache();

	//! Destructor.
        ~BuilderCache() {};

    private:

        void preloadAllBuilders(Glib::ustring dir);

	// DATA

        static BuilderCache * s_instance;
};

#endif // BUILDER_CACHE_H

// End of file
