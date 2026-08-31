//  Copyright (C) 2010, 2011, 2014, 2015, 2020, 2026 Ben Asselstine
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
    Tar_Helper(std::string file, std::ios::openmode mode, bool &broken);

    //! Destructor
    ~Tar_Helper();

    bool saveFile(std::string file, std::string destfile = "");

    std::string getFile(std::string filename, bool &broken);

    std::string getFirstFile(std::string extension, bool &broken);
    std::string getFirstFile(std::list<std::string> exts, bool &broken);

    std::list<std::string> getFilenames(std::string ext);
    std::string getFirstFilename(std::string ext);

    std::list<std::string> getFilenames();

    //munge name if necessary to make it unique
    std::string makeNameUnique (std::string name);

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
    bool replaceFile(std::string old_filename, std::string new_filename,
                     std::string archive_name);

    bool Open(std::string file, std::ios::openmode mode);
    void Close(bool clean = true);

    static bool is_tarfile (std::string file);

    static std::string getFile(Tar_Helper *t, std::string filename, bool &broken, std::string tmpoutdir);
    static std::list<std::string> getFilenames(Tar_Helper *t);
    static bool saveFile(Tar_Helper *t, std::string filename, std::string destfile = "");
    static void clean_tmp_dir(std::string filename);
    static void reopen(Tar_Helper *t);
    static int dump_entry(struct archive *in, struct archive_entry *entry, struct archive *out);
    static int dump_file_entry(std::string filename, struct archive_entry *entry, std::string nameinarchive, struct archive *out);
private:

    // DATA
    struct archive *t;
    std::ios::openmode openmode;
    std::string tmpoutdir;
    std::string pathname;
};
#endif
