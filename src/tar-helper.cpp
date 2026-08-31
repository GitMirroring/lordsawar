// Copyright (C) 2010, 2011, 2014, 2015, 2020, 2021 Ben Asselstine
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

#include "tarhelper.h"
#include <algorithm>
#include <iostream>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fstream>
#include "File.h"
#include <errno.h>
#include "ucompose.hpp"
#include <archive_entry.h>

/*
 * libarchive doesn't rewind.
 * once you go through the entries, that's it.
 * to do it again you have to reopen.
 *
 * the Tar_Helper class works around this by delicately opening and closing
 * the tar file for every operation on it.
 * this has the unfortunate side effect when we add N files to a tar file 
 * using saveFile.  the file gets saved out N seperate times.
 *
 * this class was originally implemented with libtar.
 */
Tar_Helper::Tar_Helper(Glib::ustring file, std::ios::openmode mode, bool &broken)
{
  t = NULL;
  broken = Open(file, mode);
}

void Tar_Helper::reopen(Tar_Helper *t)
{
  t->Close(false);
  t->Open(t->pathname, t->openmode);
}

bool Tar_Helper::Open(Glib::ustring file, std::ios::openmode mode)
{
  t = NULL;
  if (mode == std::ios::in && is_tarfile (file) == false)
    return true;
  //int m;
  //int perms = 0;
  openmode = mode;
  if (mode & std::ios::in) 
    {
      if (File::exists (file) == false)
        return false;
      t = archive_read_new ();
      archive_read_support_format_tar(t);
      int r = archive_read_open_filename(t, file.c_str(), 8192);
      if (r != ARCHIVE_OK)
        {
          archive_read_free (t);
          return false;
        }
    }
  else if (mode & std::ios::out)
    {
      t = archive_write_new();
      archive_write_add_filter_none(t);
      archive_write_set_format_gnutar(t);
      if (archive_write_open_filename(t, file.c_str()))
        {
          archive_write_free (t);
          return false;
        }
    }
  else
    return false;

  if (mode & std::ios::in)
    {
      tmpoutdir = File::getTarTempDir(File::get_basename(file,true));
      File::create_dir(tmpoutdir);
    }
  else
    tmpoutdir = "";
  pathname = file;
  return false;
}

int Tar_Helper::dump_entry(struct archive *in, struct archive_entry *entry, struct archive *out)
{
  archive_write_header(out, entry);

  char buff[8192];
  ssize_t len = archive_read_data(in, buff, sizeof (buff));
  while (len > 0)
    {
      archive_write_data (out, buff, len);
      len = archive_read_data(in, buff, sizeof (buff));
    }
  archive_write_finish_entry (out);
  return ARCHIVE_OK;
}

bool Tar_Helper::saveFile(Tar_Helper *t, Glib::ustring filename, Glib::ustring destfile)
{
  //save each already existing file entry out, and then add ours on the end.
  //write the whole tar file to a temporary file and then copy it in place.
  Glib::ustring tmp = File::get_tmp_file();
  bool broken = false;
  Tar_Helper out(tmp, std::ios::out, broken);
  Tar_Helper in(t->pathname, std::ios::in, broken);
  if (!broken)
    {
      struct archive_entry *in_entry = NULL;
      while (1) 
        {
          int r = archive_read_next_header(in.t, &in_entry);
          if (r == ARCHIVE_EOF || r != ARCHIVE_OK)
            break;
          dump_entry(in.t, in_entry, out.t);
        }
      in.Close();
    }

  Glib::ustring b;
  if (destfile == "")
    b = File::get_basename(filename, true);
  else
    b = destfile;
  struct archive_entry *entry = archive_entry_new();
  dump_file_entry (filename, entry, b, out.t);

  out.Close();
  archive_entry_free (entry);
  archive_write_free(t->t);
  t->t = NULL;
  File::copy(tmp, t->pathname);
  File::erase(tmp);
  return true;
}

bool Tar_Helper::saveFile(Glib::ustring filename, Glib::ustring destfile)
{
  //archive_seek_data(t, 0, SEEK_SET);
  return saveFile(this, filename, destfile);
}

void Tar_Helper::Close(bool clean)
{
  if (t)
    {
      if (openmode & std::ios::out)
        {
          archive_write_close (t);
          archive_write_free (t);
        }
      else if (openmode & std::ios::in)
        {
          archive_read_close (t);
          archive_read_free (t);
        }
      t = NULL;
      if (tmpoutdir != "" && clean)
        File::clean_dir(tmpoutdir);
    }
}

Glib::ustring Tar_Helper::getFirstFile(std::list<Glib::ustring> exts, bool &broken)
{
  for (std::list<Glib::ustring>::iterator i = exts.begin(); i != exts.end(); ++i)
    {
      Glib::ustring file = getFirstFile(*i, broken);
      if (file != "")
        return file;
    }
  return "";
}

Glib::ustring Tar_Helper::getFirstFile(Glib::ustring extension, bool &broken)
{
  std::list<Glib::ustring> files = getFilenames(extension);
  if (files.size() == 0)
    return "";
  return getFile(files.front(), broken);
}

Glib::ustring Tar_Helper::getFile(Tar_Helper *t, Glib::ustring filename, bool &broken, Glib::ustring tmpoutdir)
{
  Glib::ustring f = File::getTempFile(tmpoutdir, filename);
  if (File::exists(f) == true)
    return f;
  struct archive_entry *entry = NULL;
  bool found = false;

  reopen(t);
  while (1) 
    {
      int r = archive_read_next_header(t->t, &entry);
      if (r == ARCHIVE_EOF)
        break;
      if (r != ARCHIVE_OK)
        break;

      if (filename == archive_entry_pathname(entry))
        {
          found = true;
          break;
        }
    }
  if (found)
    {
      const void *buff = NULL;
      size_t size = 0;
      int64_t offset = 0;
      broken = false;
      struct archive *ext = archive_write_disk_new();
      archive_write_disk_set_options(ext, 
                                     ARCHIVE_EXTRACT_OWNER |
                                     ARCHIVE_EXTRACT_PERM);
      Glib::ustring outfile = File::getTempFile(tmpoutdir, filename);
      archive_entry_copy_pathname(entry, outfile.c_str());

      archive_write_header(ext, entry);

      while (1)
        {
          int r = archive_read_data_block(t->t, &buff, &size, &offset);
          if (r == ARCHIVE_EOF)
            break;
          if (r != ARCHIVE_OK)
            break;
          r = archive_write_data_block(ext, buff, size, offset);
          if (r != ARCHIVE_OK)
            break;
        }
      archive_write_finish_entry(ext);
      archive_write_close(ext);
      archive_write_free(ext);
      return outfile;
    }

  //broken = true;
  // couldn't find filename in t->pathname
  return "";
}

Glib::ustring Tar_Helper::getFile(Glib::ustring filename, bool &broken)
{
  return getFile(this, filename, broken, tmpoutdir);
}

std::list<Glib::ustring> Tar_Helper::getFilenames(Tar_Helper *t)
{
  reopen(t);
  std::list<Glib::ustring> result;
  //archive_seek_data(t, 0, SEEK_SET);
  struct archive_entry *entry = NULL;
  while (1) 
    {
      int r = archive_read_next_header(t->t, &entry);
      if (r == ARCHIVE_EOF)
        break;
      if (r != ARCHIVE_OK)
        break;

      result.push_back(archive_entry_pathname(entry));
    }
  return result;
}

std::list<Glib::ustring> Tar_Helper::getFilenames()
{
  return getFilenames(this);
}

Glib::ustring Tar_Helper::getFirstFilename(Glib::ustring ext)
{
  std::list<Glib::ustring> result = getFilenames(ext);
  if (result.empty())
    return "";
  return result.front();
}

std::list<Glib::ustring> Tar_Helper::getFilenames(Glib::ustring ext)
{
  std::list<Glib::ustring> result;
  std::list<Glib::ustring> f = getFilenames(this);
  for (std::list<Glib::ustring>::iterator i = f.begin(); i != f.end(); ++i)
    {
      if (ext == "")
        result.push_back(*i);
      else
        if (File::nameEndsWith(*i, ext))
          result.push_back(*i);
    }
  return result;
}

Tar_Helper::~Tar_Helper()
{
  if (t)
    Close();
}

bool Tar_Helper::is_tarfile (Glib::ustring file)
{
  bool retval = false;
  struct archive *a = archive_read_new ();
  archive_read_support_format_tar(a);
  int r = archive_read_open_filename (a, file.c_str(), 10240);
  struct archive_entry *entry = NULL;
  archive_read_next_header(a, &entry);
  if (r == ARCHIVE_OK)
    {
      retval = (archive_format (a) & ARCHIVE_FORMAT_TAR) > 0;
      archive_read_close(a);
    }
  archive_read_free(a);
  return retval;
}

int Tar_Helper::dump_file_entry (Glib::ustring filename, struct archive_entry *entry, Glib::ustring nameinarchive, struct archive *out)
{
    struct stat st;
    stat(filename.c_str(), &st);
    archive_entry_copy_stat(entry, &st);
    archive_entry_set_pathname(entry, nameinarchive.c_str());
    archive_write_header(out, entry);
    int fd = open (filename.c_str(), O_RDONLY);
    if (fd < 0)
      return ARCHIVE_FATAL;
    char buff[8192];

    ssize_t len = read (fd, buff, sizeof (buff));
    while (len > 0)
      {
        archive_write_data (out, buff, len);
        len = read (fd, buff, sizeof (buff));
      }
    archive_write_finish_entry(out);
    close(fd);
    return ARCHIVE_OK;
}

Glib::ustring Tar_Helper::makeNameUnique(Glib::ustring name)
{
  if (name.empty () == true)
    return name;
  std::list<Glib::ustring> files = getFilenames();
  if (find(files.begin(), files.end(), name) == files.end())
    return name;
  Glib::ustring bname = File::get_basename (name, false);
  Glib::ustring ext = File::get_extension (name);

  //take any trailing numbers
  int digits = 0;
  for (Glib::ustring::reverse_iterator i = bname.rbegin ();
       i != bname.rend(); ++i)
    {
      if (g_unichar_isdigit (*i))
        digits++;
      else
        break;
    }
  int count = 2;
  if (digits > 0)
    {
      Glib::ustring numerals = bname.substr (bname.length () - digits);
      bname = bname.substr (0, bname.length () - digits);
      if (numerals.empty () == false)
        count = atoi (numerals.c_str ()) + 1;
    }
  //okay we have to munge the name now.
  while (1)
    {
      Glib::ustring newname = String::ucompose ("%1%2%3", bname, count, ext);
      if (find(files.begin(), files.end(), newname) == files.end())
        return newname;
      count++;
      if (count == 10000) //something has gone horribly wrong
        break;
    }
  return name;
}

bool Tar_Helper::replaceFile(Glib::ustring filename, Glib::ustring newfilename, Glib::ustring archive_name)
{
  if (newfilename != "" && File::exists(newfilename) == false)
    return false;
  //loop through existing file entries and copy them out.
  //when we see the one we want to replace, we do so.
  //unless newfilename is "", in which case we skip it (remove it).
  //write the whole tar file to a temporary file and then copy it in place.
  Glib::ustring tmp = File::get_tmp_file();
  bool broken = false;
  Tar_Helper out(tmp, std::ios::out, broken);
  Tar_Helper in(pathname, std::ios::in, broken);
  if (!broken)
    {
      struct archive_entry *in_entry = NULL;
      while (1) 
        {
          int r = archive_read_next_header(in.t, &in_entry);
          if (r == ARCHIVE_EOF)
            break;
          if (r != ARCHIVE_OK)
            break;
          if (filename == Glib::ustring(archive_entry_pathname(in_entry)) &&
              newfilename != "")
            {
              //hey it's the one we want to replace
              dump_file_entry (newfilename, in_entry, 
                               archive_name, out.t);
            }
          else if (filename == Glib::ustring(archive_entry_pathname(in_entry)) &&
              newfilename == "")
            ; //hey it's the one we're removing
          else
            dump_entry(in.t, in_entry, out.t);
        }
      in.Close();
    }

  out.Close();
  if (filename == "")
    {
      Tar_Helper i(tmp, std::ios::in, broken);
      saveFile (&i, newfilename, archive_name);
      i.Close();
    }
  archive_write_free(t);
  t = NULL;
  bool ret = File::copy(tmp, pathname);
  int save_errno = errno;
  if (ret)
    File::erase(tmp);
  errno = save_errno;
  return ret;
}

void Tar_Helper::clean_tmp_dir(Glib::ustring filename)
{
  Glib::ustring tmpoutdir = File::getTarTempDir (File::get_basename(filename, true));
  File::clean_dir(tmpoutdir);
}
