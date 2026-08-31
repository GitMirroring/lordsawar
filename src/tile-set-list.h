//  Copyright (C) 2007, 2008, 2009, 2010, 2011, 2014, 2020 Ben Asselstine
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
#ifndef TILESETLIST_H
#define TILESETLIST_H

#include <gtkmm.h>
#include <map>
#include <vector>
#include <sigc++/trackable.h>

#include "Tile.h"
#include "tileset.h"
#include "setlist.h"

class Tar_Helper;
class SmallTile;

//! A list of all Tileset objects available to the game.
/**
 * This class contains a list of all Tileset objects available to the game. 
 * Since several classes access this class, it is implemented as a singleton.
 *
 * Tileset objects are usually referenced by the name of the subdirectory
 * in which they reside on disk (inside the tilesets/ directory).
 */
class Tilesetlist : public SetList<Tileset>, public sigc::trackable
{
    public:

	// Methods that operate on class data but do not modify the class.

        SmallTile *getSmallTile(Glib::ustring basename, Tile::Type type) const;

        Gdk::RGBA getColor(Glib::ustring basename, Tile::Type type) const;

	// Methods that operate on the class data and modify the class.

	//! Destroy all of the tileset images in this list.
	void uninstantiateImages();
        
	//! Load the images for all tilesets in this list.
	void instantiateImages(bool &broken);


	// Static Methods

        //! Return the singleton instance of this class.
        static Tilesetlist* getInstance();

        //! Explicitly delete the singleton instance of this class.
        static void deleteInstance();

    private:
        //! Default constructor.  Loads all tilesets it can find.
	/**
	 * The tilesets/ directory is scanned for Tileset directories.
	 */
        Tilesetlist();
        
        //! Destructor.
        ~Tilesetlist();

        //! A static pointer for the singleton instance.
        static Tilesetlist* s_instance;
};

#endif // TILESETLIST_H

