//  Copyright (C) 2011, 2014, 2026 Ben Asselstine
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
#ifndef PROFILE_H
#define PROFILE_H

#include <gtkmm.h>

class XML_Helper;

//! A single entry in the profile list.
/**
 * This is an identity that the user assumes when playing a network game.
 */
class Profile
{
    public:

	//! The xml tag of this object in a profiles file.
	static Glib::ustring d_tag; 

	//! Loading constructor.
        /**
	 * Make a new profile object by reading it in from an opened profiles 
         * file.
	 *
         * @param helper  The opened profiles file to read the profile from.
         */
        Profile(XML_Helper* helper);

	//! Default constructor.
	/**
	 * Make a new profile object.
	 */
        Profile(Glib::ustring nickname);
        
        //! Copy constructor
        Profile(const Profile &orig);

	//! Destructor.
        virtual ~Profile() {};

	// Get Methods

        //! Get the guid of the profile.
	Glib::ustring getId() const {return d_id;};

        //! Get the name of the profile.
        Glib::ustring getNickname() const {return d_nickname;};

        //! Get the date of the last time this profile played a network game.
        Glib::DateTime getLastPlayedOn() const {return d_last_played_date;};

        //! Get the date of when this profile was created.
        Glib::DateTime getCreatedOn() const {return d_creation_date;};

        //! Get the user name of the profile.
        Glib::ustring getUserName() const {return d_user;};

	// Methods that operate on the class data but do not modify it.

	//! Save the profile to an opened file.
	bool save(XML_Helper* helper) const;

	//! Save the profile, but not the enclosing tags.
	bool saveContents(XML_Helper *helper) const;

        // Methods that operate ont he class and modify it.
        
        //! This profile is engaging in a game.  Record the date.
        void play();

	// Static Methods

	/**
	 * static load function (see XML_Helper)
	 * 
	 * @param helper       the XML_Helper instance for the profiles file.
	 */
	static Profile* handle_load(XML_Helper *helper);

    protected:

	// DATA
	
	//! The id of the profile.
	Glib::ustring d_id;

        Glib::ustring d_nickname;

        Glib::ustring d_user;

        Glib::DateTime d_creation_date;

        Glib::DateTime d_last_played_date;

};

#endif
