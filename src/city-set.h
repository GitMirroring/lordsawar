// Copyright (C) 2008, 2009, 2010, 2011, 2014, 2020 Ben Asselstine
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
#ifndef CITYSET_H
#define CITYSET_H

#include <vector>
#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "PixMask.h"
#include "set.h"

#include "defs.h"

class XML_Helper;
class TarFileImage;

//! A list of city graphic objects in a city theme.
/** 
 * Every scenario has a city set; it is the theme of the city graphics 
 * within the game. 
 *
 * The Cityset dictates the size of city images.
 *
 * Citysets are referred to by their base name.  The base name is the last
 * part of the file's path minus the file extension.
 *
 * The cityset configuration file is a tar file that contains an XML file, 
 * and a set of png files.  Filenames have the following form:
 * cityset/${Cityset::d_basename}.lwc.
 */
class Cityset : public sigc::trackable, public Set
{
    public:
	//! The xml tag of this object in a cityset configuration file.
	static Glib::ustring d_tag; 
	static Glib::ustring file_extension; 

	//! Default constructor.
	/**
	 * Make a new Cityset.
	 *
	 * @param id    The unique Id of this Cityset among all other Cityset
	 *              objects.  Must be more than 0.  
	 * @param name  The name of the Cityset.  Analagous to Cityset::d_name.
	 */
	Cityset(guint32 id, Glib::ustring name);

        //! Copy constructor.
        Cityset(const Cityset& c);

	//! Loading constructor.
	/**
	 * Make a new Cityset object by reading it in from the cityset
	 * configuration file.
	 *
	 * @param helper  The opened cityset configuration file to load the
	 *                Cityset from.
	 */
        Cityset(XML_Helper* helper, Glib::ustring directory);

	static Cityset *create(Glib::ustring file, bool &unsupported_version);

        static Cityset *copy (const Cityset *orig);
	//! Destructor.
        ~Cityset();

	bool save(XML_Helper *helper) const;

        bool save(Glib::ustring filename, Glib::ustring extension) const;

        TarFileImage *getPort () {return d_port;}
        TarFileImage *getSignpost () {return d_sign;}
        TarFileImage *getTemple () {return d_temple;}
        TarFileImage *getRuin () {return d_ruin;}
        TarFileImage *getTower () {return d_tower;}
        TarFileImage *getRazedCity () {return d_rcity;}
        TarFileImage *getCity () {return d_city;}

        //! Load the images associated with this cityset.
        /**
         * Go get the image files from the cityset file and create the
         * various pixmask objects.
         *
         * @param scale   The images are clamped to the tile size or not.
         * @param broken  True when things went wrong reading the cityset file.
         */
	void instantiateImages(bool scale, bool &broken);

	void uninstantiateImages();

	guint32 getCityTileWidth() {return d_city_tile_width;};
	void setCityTileWidth(guint32 tiles) {d_city_tile_width = tiles;};
	guint32 getTempleTileWidth() {return d_temple_tile_width;};
	void setTempleTileWidth(guint32 tiles) {d_temple_tile_width = tiles;};
	guint32 getRuinTileWidth() {return d_ruin_tile_width;};
	void setRuinTileWidth(guint32 tiles) {d_ruin_tile_width = tiles;};
	bool validate();
	bool validateCityTileWidth();
	bool validateRuinTileWidth();
	bool validateTempleTileWidth();

        void uninstantiateSameNamedImages (Glib::ustring name);

        //! Callback to convert old files to new ones.
        static bool upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version);
        static void support_backward_compatibility();

        static guint32 get_default_tile_size ();
        //! Load the cityset again.
        void reload(bool &broken);
        bool calculate_preferred_tile_size(guint32 &ts) const;
    private:

        // DATA

        TarFileImage *d_port;
        TarFileImage *d_sign;
        TarFileImage *d_temple;
        TarFileImage *d_ruin;
        TarFileImage *d_tower;
        TarFileImage *d_city;
        TarFileImage *d_rcity;

	guint32 d_city_tile_width;
	guint32 d_temple_tile_width;
	guint32 d_ruin_tile_width;

        std::vector<TarFileImage*> getImages ();
};

#endif // CITYSET_H

// End of file
