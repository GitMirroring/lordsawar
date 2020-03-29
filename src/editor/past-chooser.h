//  Copyright (C) 2014 Ben Asselstine
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
#ifndef PAST_CHOOSER_H
#define PAST_CHOOSER_H

#include <map>
#include <gtkmm.h>

class PastChooser
{
public:
    //! Default constructor.
    PastChooser();

    //! Destructor.
    ~PastChooser() {}

    //! Store a folder for the file pattern and dir in the given filechooser.
    /**
     * remember the folder pointed to by the given filechooser.
     * remember it in relation to the file filter of the filechooser.
     */
    void set_dir(Gtk::FileChooser *filechooser);

    //! Return a folder for the file pattern in the given filechooser.
    Glib::ustring get_dir(Gtk::FileChooser *filechooser);

    //! Returns the singleton instance.  Creates a new one if neccessary.
    static PastChooser* getInstance();

    //! Deletes the singleton instance.
    static void deleteInstance();

private:
    std::map<Glib::ustring,Glib::ustring> pattern_dir;
    
    void set_dir(Glib::RefPtr<Gtk::FileFilter> filter, Glib::ustring dir);
    Glib::ustring get_dir(Glib::RefPtr<Gtk::FileFilter> filter);

    //! A static pointer for the singleton instance.
    static PastChooser * s_instance;
};

#endif

