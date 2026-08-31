//  Copyright (C) 2008, 2009, 2010, 2011, 2014, 2020, 2021, 2026 Ben Asselstine
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
#ifndef SHIELDSET_H
#define SHIELDSET_H

#include <gtkmm.h>
#include <map>
#include <vector>
#include <sigc++/trackable.h>

#include "shield.h"
#include "tartan.h"
#include "set.h"
#include "defs.h"

class XML_Helper;
class ShieldStyle;
class TarFileMaskedImage;

//! A list of Shield graphic objects in a shield theme.
/**
 * Every scenario has a shield set; it is the theme of the shield graphics 
 * within the game.  Shields come in three sizes -- small, medium and large.  
 * Small shields appear on the OverviewMap.  Medium shields appear in the turn 
 * indicator in the top right of the GameWindow.  Large shields appear in many 
 * dialogs, chiefly the FightWindow, and DiplomacyDialog.
 * Every shield belongs to one of 9 players (the ninth is the Neutral player).
 * The players aren't Player objects in this case; instead it refers to a 
 * Shield::ShieldColor.  e.g. Not `The Sirians' but rather the `White player'
 * of the scenario.
 *
 * The Shieldset dictates the dimensions of these three sizes of shields.
 *
 * Shieldsets are referred to by their basename.  This is the last part of the
 * filename, minus the file extension.
 *
 * The shieldset configuration file is a tar file that contains an XML file, 
 * and a set of png files.  Filenames have the following form:
 * shield/${Shieldset::d_basename}.lws.
 */
class Shieldset: public std::list<Shield *>, public sigc::trackable, public Set
{
    public:

	//! The xml tag of this object in a shieldset configuration file.
	static Glib::ustring d_tag; 

	//! The file extension for shieldset files.  It includes the dot.
	static Glib::ustring file_extension;


	//! Default constructor.
	/**
	 * Make a new shieldset given a unique id and a basename name.
	 */
	Shieldset(guint32 id, Glib::ustring name);

        //! Copy constructor.
        Shieldset(const Shieldset& s);

	//! Load a Shieldset from an opened shieldset configuration file.
	/**
	 * Make a new Shieldset object by reading it in from the shieldset
	 * configuration file.
	 *
	 * @param helper  The opened shieldset configuration file to load the
	 *                Shieldset from.
	 */
        Shieldset(XML_Helper* helper, Glib::ustring directory);

	//! Destructor.
        ~Shieldset();

	// Get Methods

	//! Return the mask color for the given player.
	Gdk::RGBA getColor(guint32 owner) const;

        //! Return all of the mask colors for the given player.
        std::vector<Gdk::RGBA> getColors (guint32 owner) const;

	//! Return the total number of shields in this shieldset.
        guint32 getSize() const {return size();}

	// Set Methods

        //! Load the shieldset again.
        void reload();

	// Methods that operate on the class data but do not modify the class.

	bool save(XML_Helper *helper) const;

        bool save(Glib::ustring filename, Glib::ustring extension) const;

	//! Find the shield of a given size and color in this Shieldset.
	/**
	 * Scan through all Shield objects in this set for first one that is 
	 * the desired size, and for the desired player.
	 *
	 * @param type    One of the values in Shield::ShieldType.
	 * @param color  One of the values in Shield::ShieldColor.
	 *
	 * @return A pointer to the shield that matches the size and player.
	 *         If no Shield object could be found that matches the given
	 *         parameters, NULL is returned.
	 */
	ShieldStyle* lookupShieldByTypeAndColor(guint32 type, guint32 color) const;
        Shield* lookupShieldByColor(guint32 color) const;

        //! Get the image and mask associated with the shield of a given color.
        /**
         * This gets the left tartan image and mask for a player denoted by
         * color.
         */
        TarFileMaskedImage *lookupTartanImage(guint32 color, Tartan::Type type);

	//! Check to see if this shieldset can be used in the game.
	bool validate() const;

	//! Check to see if the number of shields is sufficient.
	bool validateNumberOfShields() const;

	//! Check to see if the images for the shields are supplied.
	bool validateShieldImages(Shield::Color c) const;

	//! Check to see if the images for the tartans are supplied.
	bool validateTartanImages(Shield::Color c) const;

        guint32 countEmptyImageNames() const;

	// Methods that operate on the class data and also modify the class.

        //! Load the images associated with this shieldset.
        /**
         * Go get the image files from the shieldset file and create the
         * various pixmask objects.
         *
         * @param broken  True when couldn't read the shieldset file.
         */
	void instantiateImages(bool &broken);

	//! Destroy images associated with this shieldset.
	void uninstantiateImages();

        //! destroy any image that has this name
        void uninstantiateSameNamedImages (Glib::ustring imgname);

        void populate_with_defaults ();

	// Static Methods

	//! Create a shieldset from the given shieldset configuration file.
        static void create(Glib::ustring filename, sigc::slot<void(Shieldset*,bool,bool,Glib::ustring)> finished);

        static Shieldset *copy (const Shieldset *orig);

        //! rewrite old shieldset files.
        static bool upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version);
        static void support_backward_compatibility();

        bool get_images_instantiated ();
    private:

	//! Callback function to load Shield objects into the Shieldset.
	bool loadShield(Glib::ustring tag, XML_Helper* helper);

        std::vector<TarFileMaskedImage*> getMaskedImages ();

    public:
        Shieldset& operator=(const Shieldset& other);
};

#endif
