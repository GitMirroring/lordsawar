// Copyright (C) 2010, 2011, 2014, 2015, 2020 Ben Asselstine
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
#ifndef TARHELPER_H
#define TARHELPER_H
#include <archive.h>
#include <glibmm.h>
#include <iosfwd>
#include <list>
#include <cstdio>

//! An interface for operating on tar archive files.
class Tar_Helper
{
public:

    //! Constructor
    Tar_Helper(Glib::ustring file, std::ios::openmode mode, bool &broken);

    //! Destructor
    ~Tar_Helper();

    bool saveFile(Glib::ustring file, Glib::ustring destfile = "");

    Glib::ustring getFile(Glib::ustring filename, bool &broken);

    //Glib::ustring getFirstFile(bool &broken);
    Glib::ustring getFirstFile(Glib::ustring extension, bool &broken);
    Glib::ustring getFirstFile(std::list<Glib::ustring> exts, bool &broken);

    std::list<Glib::ustring> getFilenames(Glib::ustring ext);
    Glib::ustring getFirstFilename(Glib::ustring ext);

    std::list<Glib::ustring> getFilenames();

    //munge name if necessary to make it unique
    Glib::ustring makeNameUnique (Glib::ustring name);

    //! Replaces old_filename with new_filename, or adds it if not present.
    /**
     * archive name is the name of the member in the archive.
     * new_filename is the place on disk of the file we want to add or replace.
     * old_filename is the name of the member in the archive that we want to
     * replace.
     *
     * we use this method to remove a member from the archive by passing
     * new_filename as "".
     *
     * we use htis method to add a member to the archive by passing
     * old_filename as "".
     *
     * usually archive_name ends up as the basename of new_filename,
     * but sometimes we need to change the name so it doesn't collide
     * with another member.
     * @return returns True if successful.
     */
    bool replaceFile(Glib::ustring old_filename, Glib::ustring new_filename,
                     Glib::ustring archive_name);

    bool Open(Glib::ustring file, std::ios::openmode mode);
    void Close(bool clean = true);

    static bool is_tarfile (Glib::ustring file);

    static Glib::ustring getFile(Tar_Helper *t, Glib::ustring filename, bool &broken, Glib::ustring tmpoutdir);
    static std::list<Glib::ustring> getFilenames(Tar_Helper *t);
    static bool saveFile(Tar_Helper *t, Glib::ustring filename, Glib::ustring destfile = "");
    static void clean_tmp_dir(Glib::ustring filename);
    static void reopen(Tar_Helper *t);
    static int dump_entry(struct archive *in, struct archive_entry *entry, struct archive *out);
    static int dump_file_entry(Glib::ustring filename, struct archive_entry *entry, Glib::ustring nameinarchive, struct archive *out);
private:

    // DATA
    struct archive *t;
    std::ios::openmode openmode;
    Glib::ustring tmpoutdir;
    Glib::ustring pathname;
};
#endif
