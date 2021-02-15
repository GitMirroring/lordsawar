//  Copyright (C) 2008, 2009, 2010, 2011, 2014, 2021 Ben Asselstine
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
#ifndef SHIELDSETLIST_H
#define SHIELDSETLIST_H

#include <gtkmm.h>
#include <map>
#include <vector>
#include <sigc++/trackable.h>

#include "setlist.h"
#include "shieldset.h"
#include "tartan.h"

class Tar_Helper;
class XML_Helper;
class TarFileMaskedImage;

//! A list of Shieldset objects available to the game.
/** 
 * This class holds all of the shield themes that are located in shield/.
 * It is implemented as a singleton.
 *
 * Other classes use it to lookup Shield and Shieldset objects.
 */
class Shieldsetlist : public SetList<Shieldset>, public sigc::trackable
{
    public:

	// Methods that operate on the class data but do not modify the class.

        //! Returns the names of all Shieldset objects available to the game.
	std::list<Glib::ustring> getValidNames() const;

        std::vector<Gdk::RGBA> getColors(guint32 shieldset, guint32 owner) const;


	// Methods that operate on the class data and modify the class.

	//! Destroy all of the images associated with shieldsets in this list.
	void uninstantiateImages();

	//! Load all of the images associated with all of the shieldsets.
	void instantiateImages(bool &broken);

        //! Get the image and mask of the leftmost tartan.
        TarFileMaskedImage *getTartan (guint32 shieldset, guint32 color, Tartan::Type) const;

        ShieldStyle *getShield(guint32 shieldset, guint32 type, guint32 color) const;
	// Static Methods

        //! Return the singleton instance of this class.
        static Shieldsetlist* getInstance();

        //! Explicitly delete the singleton instance of this class.
        static void deleteInstance();

    private:
        //! Default Constructor.
	/**
	 * Loads all shieldsets it can find in the shield/ directory, and
	 * makes a new Shieldsetlist object from what it finds.
	 */
        Shieldsetlist();
        
        //! Destructor.
        ~Shieldsetlist();

        //! A static pointer for the singleton instance.
        static Shieldsetlist* s_instance;
};

#endif // SHIELDSETLIST_H

