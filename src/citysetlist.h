// Copyright (C) 2008, 2010, 2011, 2014, 2020 Ben Asselstine
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
#ifndef CITYSETLIST_H
#define CITYSETLIST_H

#include <map>
#include <vector>
#include <sigc++/trackable.h>
#include "setlist.h"

#include "cityset.h"

class Tar_Helper;

//! A list of all Cityset objects available to the game.
/**
 * This class contains a list of all Cityset objects available to the game. 
 * Since several classes access this class, it is implemented as a singleton.
 *
 * Cityset objects are usually referenced by the basename of the file
 * in which they reside on disk (inside the citysets/ directory).
 */
class Citysetlist : public SetList<Cityset>, public sigc::trackable
{
    public:
        //! Return the singleton instance of this class.
        static Citysetlist* getInstance();

        //! Explicitly delete the singleton instance of this class.
        static void deleteInstance();

	void instantiateImages(bool &broken);
	void uninstantiateImages();

    private:
        //! Default constructor.  Loads all citysets it can find.
	/**
	 * The citysets/ directory is scanned for Cityset directories.
	 */
        Citysetlist();
        
        //! Destructor.
        ~Citysetlist();

        //! A static pointer for the singleton instance.
        static Citysetlist* s_instance;
};

#endif // CITYSETLIST_H

