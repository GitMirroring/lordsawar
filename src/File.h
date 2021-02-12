// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2014, 2015,
// 2021 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
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

        static Glib::ustring getSetDir(Glib::ustring ext, bool system = true);

        //! load misc file
        static Glib::ustring getMiscFile(Glib::ustring filename);

        //! load a file from the various dir
        static Glib::ustring getVariousFile(Glib::ustring filename);

        static Glib::ustring getGladeFile(Glib::ustring filename);
        static Glib::ustring getEditorGladeFile(Glib::ustring filename);
        static Glib::ustring getSaveFile(Glib::ustring filename);
        static Glib::ustring getTempFile(Glib::ustring tmpdir, Glib::ustring filename);
        static Glib::ustring getTarTempDir(Glib::ustring dir);
        static Glib::ustring getConfigDir ();
        static Glib::ustring getConfigFile(Glib::ustring filename);
        static Glib::ustring getUserDataDir ();
        static Glib::ustring getCacheDir ();

        //! load an xslt file.
        static Glib::ustring getXSLTFile(guint32 type, Glib::ustring old_version, Glib::ustring new_version);
        
        //! Load the xml file describing the items
        static Glib::ustring getItemDescription();
        
        //! Get the path to an editor image
	static Glib::ustring getEditorFile(Glib::ustring filename);
    
        // Returns the filename of a music file (description or actual piece)
        static Glib::ustring getMusicFile(Glib::ustring filename);
        
        // get save game path
        static Glib::ustring getSavePath();

	//! get game data path
	static Glib::ustring getDataPath();

	//! the location of the system directory that holds scenario terrains.
	static Glib::ustring getMapDir();

	//! the location of the system directory that holds personal terrains.
	static Glib::ustring getUserMapDir();

	//! get the path of a system scenario file called file.
	static Glib::ustring getMapFile(Glib::ustring file);

	//! get the path of a personal scenario called file.
	static Glib::ustring getUserMapFile(Glib::ustring file);

        static Glib::ustring getUserProfilesDescription();
        static Glib::ustring getUserRecentlyPlayedGamesDescription();
        static Glib::ustring getUserRecentlyHostedGamesDescription();
        static Glib::ustring getUserRecentlyAdvertisedGamesDescription();
        static Glib::ustring getUserRecentlyEditedFilesDescription();

        // get the available scenarios
        static std::list<Glib::ustring> scanMaps();

	// get the available scenarios in the user's personal collection
	static std::list<Glib::ustring> scanUserMaps();


	//! Copy a file from one place to another.
	static bool copy (Glib::ustring from, Glib::ustring to);

	//! make a directory if it doesn't already exist.
	static bool create_dir(Glib::ustring dir);

	//! simple basename routine, but also strips the file extension.
	static Glib::ustring get_basename(Glib::ustring path, bool keep_ext=false);

	//! does a file exist?
	static bool exists(Glib::ustring f);

        //! does a directory exist
        static bool directory_exists(Glib::ustring d);

	//! does filename end with extension?
	static bool nameEndsWith(Glib::ustring filename, Glib::ustring extension);

	//! delete a file from the filesystem.
	static bool erase(Glib::ustring filename);

	//! delete an empty directory from the filesystem.
	static void erase_dir(Glib::ustring filename);

        //! delete a directory and the files it contains from the filesystem.
        static void clean_dir(Glib::ustring filename);

	static Glib::ustring add_slash_if_necessary(Glib::ustring dir);

	static Glib::ustring get_dirname(Glib::ustring path);

        static std::list<Glib::ustring> scanForFiles(Glib::ustring dir, Glib::ustring extension);

        static Glib::ustring add_ext_if_necessary(Glib::ustring file, Glib::ustring ext);

        static char *_sanify(const char *string);
        static Glib::ustring sanify (Glib::ustring s);

        static Glib::ustring get_tmp_file(Glib::ustring ext = "");

        static Glib::ustring get_extension(Glib::ustring filename);

        static bool rename(Glib::ustring src, Glib::ustring dest);

        static bool add_png_if_no_ext (Glib::ustring &filename);

        //! get the file's size in bytes
        static goffset get_size (Glib::ustring filename);
};

bool case_insensitive (const Glib::ustring& first, const Glib::ustring& second);

#endif //FILE_H

// End of file
