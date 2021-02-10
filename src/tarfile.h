// Copyright (C) 2017, 2020, 2021 Ben Asselstine
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
#ifndef TARFILE_H
#define TARFILE_H
#include "File.h"
#include "defs.h"
#include "xmlhelper.h"

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
    TarFile (Glib::ustring dir, Glib::ustring name, Glib::ustring ext);
    ~TarFile() {};
    TarFile(const TarFile &s);

    Glib::ustring getDirectory() const {return d_dir;}
    void setDirectory(Glib::ustring d) {d_dir = File::add_slash_if_necessary(d);}

    Glib::ustring getConfigurationFile(bool master = false) const;

    Glib::ustring getFileFromConfigurationFile(Glib::ustring file);
    bool contains (Glib::ustring ar, bool &broken);
    guint32 countImages ();
    bool replaceFileInCfgFile(Glib::ustring file, Glib::ustring new_file, Glib::ustring &out);
    bool addFileInCfgFile(Glib::ustring new_file, Glib::ustring &out);
    bool removeFileInCfgFile(Glib::ustring file);

    void clean_tmp_dir();

    bool saveTar(Glib::ustring tmpfile, Glib::ustring tmptar, Glib::ustring dest, std::vector<Glib::ustring> extrafiles) const;
    Glib::ustring getBaseName () const {return d_basename;}
    Glib::ustring getExtension () const {return d_extension;}

    void setBaseName(Glib::ustring bname) {d_basename = bname;}
    void setExtension(Glib::ustring ext) {d_extension = ext;}

    void moved(Glib::ustring filename);
    void created(Glib::ustring filename);

    //! when we don't have a configuration file yet, we use this
    void setNewTemporaryFile ();
    //! when we open a file, we work on a copy of it
    void setLoadTemporaryFile ();

private:

    Glib::ustring d_dir;
    Glib::ustring d_basename;
    Glib::ustring d_extension;
    Glib::ustring d_tmp_filename;

};

#endif //TarFile
