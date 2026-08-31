//  Copyright (C) 2017, 2020, 2021, 2026 Ben Asselstine
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
#ifndef TARFILE_H
#define TARFILE_H
#include "file.h"
#include "defs.h"
#include "xml-helper.h"

//! An interface for dealing with tar files
/**
 * Whereas Tar_Helper is about dealing with individual tar files
 * and the files in them, TarFile is a slightly higher level of abstraction.
 *
 * The idea is that we're going to use this in the editor to add and replace
 * files in the tar file (e.g. armyset, cityset, etc).
 */
class TarFile
{
public:
    TarFile (std::string dir, std::string name, std::string ext);
    ~TarFile() {};
    TarFile(const TarFile &s);

    std::string getDirectory() const {return d_dir;}
    void setDirectory(std::string d) {d_dir = File::add_slash_if_necessary(d);}

    std::string getConfigurationFile(bool master = false) const;

    std::string getFileFromConfigurationFile(std::string file);
    bool contains (std::string ar, bool &broken);
    guint32 countImages ();
    bool replaceFileInCfgFile(std::string file, std::string new_file, std::string &out, Glib::ustring &err);
    bool addFileInCfgFile(std::string new_file, std::string &out, Glib::ustring &err);
    bool removeFileInCfgFile(std::string file, Glib::ustring &err);

    void clean_tmp_dir();

    bool saveTar(std::string tmpfile, std::string tmptar, std::string dest, std::vector<std::string> extrafiles) const;
    std::string getBaseName () const {return d_basename;}
    std::string getExtension () const {return d_extension;}

    void setBaseName(std::string bname) {d_basename = bname;}
    void setExtension(std::string ext) {d_extension = ext;}

    void moved(std::string filename);
    void created(std::string filename);

    //! when we don't have a configuration file yet, we use this
    void setNewTemporaryFile ();
    //! when we open a file, we work on a copy of it
    bool setLoadTemporaryFile ();

private:

    std::string d_dir;
    std::string d_basename;
    std::string d_extension;
    std::string d_tmp_filename;

public:
    TarFile& operator=(const TarFile& other)
      {
        if (this != &other)
          {
            d_dir = other.d_dir;
            d_basename = other.d_basename;
            d_extension = other.d_extension;
            d_tmp_filename = other.d_tmp_filename;
          }
        return *this;
      }
};

#endif //TarFile
