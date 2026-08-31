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
#ifndef PROFILELIST_H
#define PROFILELIST_H

#include <gtkmm.h>
#include <list>
#include <sigc++/trackable.h>

class XML_Helper;

class Profile;
//! A list of accounts or identities that we play the game as.
/**
 * This is only used for network games at the moment.
 * It is implemented as a singleton.
 *
 */
class Profilelist: public std::list<Profile*>, public sigc::trackable
{
    public:

	//! The xml tag of this object in a profiles file.
	static Glib::ustring d_tag;

        static inline const Glib::ustring admin_username = "server";
	
	// Methods that operate on the class data and do not modify the class.

        //! Save to the profile list file.
        bool save() const;

        Profile *getDefaultProfile () const;

        Profile *findLastPlayedProfileForUser(Glib::ustring user) const;

	//! Save the profile list to an opened file.
	bool save(XML_Helper* helper) const;

        Profile *findProfileById(Glib::ustring id) const;

        Profile *get_profile_by_name (Glib::ustring name)
          {
            for (auto it = begin (); it != end (); it++)
              {
                if ((*it)->getNickname () == name)
                  return (*it);
              }
            return NULL;
          }

        Profile *getAdminProfile ()
          {
            return get_profile_by_name (admin_username);
          }

	// Methods that operate on the class data and modify the class.

        void remove (Profile* &value)
          {
            if (d_default == value->getId ())
              d_default = "";
            std::list<Profile*>::remove (value);
          }

        void setDefaultProfile (Profile *p) { if (p) d_default = p->getId (); }

        static Profile* createDefaultProfile ()
          {
            auto default_profile = new Profile (Glib::get_user_name ());
            Profilelist::instance ()->push_back (default_profile);
            Profilelist::instance ()->setDefaultProfile (default_profile);
            Profilelist::instance ()->save ();
            return default_profile;
          }

        static Profile* createAdminProfile ()
          {
            auto admin_profile = new Profile (admin_username);
            Profilelist::instance ()->push_back (admin_profile);
            Profilelist::instance ()->save ();
            return admin_profile;
          }

        bool load();

	// Static Methods

        //! return the singleton instance of this class.
        static Profilelist * instance();

        //! Loads the singleton instance from an opened file.
        static Profilelist * instance(XML_Helper *helper);

        //! Explicitly delete the singleton instance of this class.
        static void deleteInstance();

        //! Rewrite an old profiles file.
        static bool upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version);
        static void support_backward_compatibility();

    protected:
        //! Default Constructor.
        Profilelist();

	//! Loading constructor
        Profilelist(XML_Helper *helper);

        //! Destructor.
        ~Profilelist();

    private:
        //! Callback for loading profiles into this list.
	bool load_tag(Glib::ustring tag, XML_Helper *helper);

	//! Save the profile list to the given file.
	bool saveToFile(Glib::ustring filename) const;

	//! Load the profile list from the given file.
	bool loadFromFile(Glib::ustring filename);

	// DATA

        //! The profile id that we use by default.
        Glib::ustring d_default;

        //! A static pointer for the singleton instance.
        static Profilelist* s_instance;
};

#endif
