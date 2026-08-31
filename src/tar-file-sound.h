//  Copyright (C) 2026 Ben Asselstine
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
#ifndef TAR_FILE_SOUND_H
#define TAR_FILE_SOUND_H

#include <gtkmm.h>
#include "vector.h"
#include "pixmask.h"

class Tar_Helper;
class TarFile;
class XML_Helper;

/**
 * A helper class to handle sound files in tar files.
 *
 */
class TarFileSound
{
public:

  //! Default Constructor
  TarFileSound ();

  //! Copy Constructor
  TarFileSound (const TarFileSound &i);

  //! Destructor
  ~TarFileSound ();

  //! Return the basename of the image (archive member in tar file)
  Glib::ustring get_name () const
    {
      return m_name;
    }

  //! Set the name of the archive member in the tar file that holds the image
  void set_name (Glib::ustring n)
    {
      m_name = n;
    }

  void clear ()
    {
      m_name = "";
    }

  //! Set the opened tar file
  void set_tar_file (Tar_Helper *t)
    {
      m_tarfile = t;
    }

  //! Read the data tag from an opened xml file and put it in our name member
  void load_name (XML_Helper *helper, Glib::ustring data_tag);

  bool load (TarFile *ta, Glib::ustring bname);
  bool load (Tar_Helper *t, Glib::ustring bname);
  bool loadFromFile (Glib::ustring filename);
  bool load (Glib::ustring bname);
  bool load (Tar_Helper *t);
  bool load ();


private:

  //! The opened tar file
  Tar_Helper *m_tarfile;

  //! The basename of the archive member holding the image
  Glib::ustring m_name;

  //! When we extract the file from the tar file, this is where it is
  Glib::ustring m_file_on_disk;

};

#endif
