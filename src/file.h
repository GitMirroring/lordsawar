//  Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
//  Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
//  Copyright (C) 2004, 2005, 2006 Andrea Paternesi
//  Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2014, 2015,
//  2021 Ben Asselstine
//  Copyright (C) 2007 Ole Laursen
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
#ifndef FILE_H
#define FILE_H

#include <list>
#include <glibmm.h>

/** \brief Miscellaneous functions for file access
  * 
  * These functions should be the sole way to access any files. They will
  * automatically prepend the correct directory, extract the correct file etc.
  * This enables us to install and package LordsAWar (there is no fixed
  * directory structure). To use these functions, you issue e.g. an armyset name
  * and have the full path returned, which is a file you can then load.
  */

class File
{
    public:


        //! load misc file
        static std::string getMiscFile(std::string filename);

        //! load a file from the various dir
        static std::string getVariousFile(std::string filename);

        static std::string getEditorGladeFile(std::string filename);
        static std::string getSaveFile(std::string filename);
        static std::string getTempFile(std::string tmpdir, std::string filename);
        static std::string getTarTempDir(std::string dir);
        static std::string getConfigDir ();
        static std::string getConfigFile(std::string filename);
        static std::string getUserDataDir ();
        static std::string getCacheDir ();

        //! load an xslt file.
        static std::string getXSLTFile(guint32 type, std::string old_version, std::string new_version);
        
        //! Load the xml file describing the items
        static std::string getItemDescription();
        
        //! Get the path to an editor image
	static std::string getEditorFile(std::string filename);
    
        // Returns the filename of a music file (description or actual piece)
        static std::string getMusicFile(std::string filename);
        
        // get save game path
        static std::string getSavePath();

	//! get game data path
	static std::string getPath();

	//! get the path of a system scenario file called file.
	static std::string getMapFile(std::string file);

	//! get the path of a personal scenario called file.
	static std::string getUserMapFile(std::string file);

        static std::string getUserProfilesDescription();
        static std::string getUserRecentlyPlayedGamesDescription();
        static std::string getUserRecentlyHostedGamesDescription();
        static std::string getUserRecentlyAdvertisedGamesDescription();
        static std::string getUserRecentlyEditedFilesDescription();

        // get the available scenarios
        static std::list<std::string> scanMaps();

	// get the available scenarios in the user's personal collection
	static std::list<std::string> scanUserMaps();


	//! Copy a file from one place to another.
	static bool copy (std::string from, std::string to);

	//! make a directory if it doesn't already exist.
	static bool create_dir(std::string dir);

	//! simple basename routine, but also strips the file extension.
	static std::string get_basename(std::string path, bool keep_ext=false);

	//! does a file exist?
	static bool exists(std::string f);

        //! does a directory exist
        static bool directory_exists(std::string d);

	//! does filename end with extension?
	static bool nameEndsWith(std::string filename, std::string extension);

	//! delete a file from the filesystem.
	static bool erase(std::string filename);

	//! delete an empty directory from the filesystem.
	static void erase_dir(std::string filename);

        //! delete a directory and the files it contains from the filesystem.
        static void clean_dir(std::string filename);

	static std::string add_slash_if_necessary(std::string dir);

	static std::string get_dirname(std::string path);

        static std::list<std::string> scanForFiles(std::string dir, std::string extension);

        static std::string add_ext_if_necessary(std::string file, std::string ext);

        static char *_sanify(const char *string);
        static std::string sanify (std::string s);

        static std::string get_tmp_file(std::string ext = "");

        static std::string get_extension(std::string filename);

        static bool rename(std::string src, std::string dest);

        static bool add_png_if_no_ext (std::string &filename);

        static bool add_ogg_if_no_ext (std::string &filename);

        //! get the file's size in bytes
        static goffset get_size (std::string filename);

        static std::string getRandomlyGeneratedMapFile ();

        static bool is_readonly (std::string path);

        static std::string get_user_shieldset_dir ();
        static std::string get_user_cityset_dir ();
        static std::string get_user_armyset_dir ();
        static std::string get_user_tileset_dir ();
        static std::string get_user_map_dir ();

        static std::string get_shieldset_dir ();
        static std::string get_cityset_dir ();
        static std::string get_armyset_dir ();
        static std::string get_tileset_dir ();
        static std::string get_map_dir ();

        static std::string getSetDir(std::string ext, bool system = true);
};

bool case_insensitive (const std::string& first, const std::string& second);

#endif
