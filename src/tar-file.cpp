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

#include "tarfile.h"
#include "tarhelper.h"

TarFile::TarFile(Glib::ustring dir, Glib::ustring name, Glib::ustring ext)
  : d_dir(dir), d_basename(name), d_extension(ext), d_tmp_filename("")
{
  if (d_dir.empty () == false)
    d_dir = File::add_slash_if_necessary (d_dir);
}

TarFile::TarFile(const TarFile &s)
  : d_dir(s.d_dir), d_basename(s.d_basename), d_extension(s.d_extension),
    d_tmp_filename ("")
{
}

void TarFile::moved(Glib::ustring filename)
{
  created (filename);
}

void TarFile::created(Glib::ustring filename)
{
  setDirectory(File::get_dirname(filename));
  setBaseName(File::get_basename(filename, false));
  setExtension(File::get_extension(filename));
}

Glib::ustring TarFile::getConfigurationFile(bool master) const
{
  if (!master)
    {
      if (d_tmp_filename != "")
        return d_tmp_filename;
    }
  return getDirectory() + getBaseName() + d_extension;
}

Glib::ustring TarFile::getFileFromConfigurationFile(Glib::ustring file)
{
  if (getConfigurationFile () == "")
    return "";
  bool broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken == false)
    {
      Glib::ustring filename = t.getFile(file, broken);
      t.Close(false);
  
      if (broken == false)
        return filename;
    }
  return "";
}

bool TarFile::removeFileInCfgFile(Glib::ustring file)
{
  bool broken = false;
  Glib::ustring infile = d_tmp_filename;
  if (infile == "")
    infile = getConfigurationFile();
  Tar_Helper t(infile, std::ios::in, broken);
  if (broken == false)
    {
      broken = !t.replaceFile(file, "", "");
      t.Close();
    }
  return !broken;
}

bool TarFile::contains (Glib::ustring ar, bool &broken)
{
  bool found = false;
  Glib::ustring infile = d_tmp_filename;
  if (infile == "")
    infile = getConfigurationFile();
  Tar_Helper t(infile, std::ios::in, broken);
  if (broken == false)
    {
      std::list<Glib::ustring> files = t.getFilenames ();
      found = std::find (files.begin (), files.end (), ar) != files.end ();
      t.Close();
    }
  return found;
}

bool TarFile::replaceFileInCfgFile(Glib::ustring file, Glib::ustring new_file, Glib::ustring &out)
{
  bool broken = false;
  Glib::ustring infile = d_tmp_filename;
  if (infile == "")
    infile = getConfigurationFile();
  Tar_Helper t(infile, std::ios::in, broken);
  if (broken == false)
    {
      Glib::ustring bname = File::get_basename (new_file, true);
      Glib::ustring outfile = t.makeNameUnique (bname);
      Tar_Helper::reopen (&t);
      if (bname != outfile && bname == file)
        outfile = file;
      broken = !t.replaceFile(file, new_file, outfile);
      t.Close();
      if (!broken)
        out = outfile;
    }
  return !broken;
}

bool TarFile::addFileInCfgFile(Glib::ustring new_file, Glib::ustring &out)
{
  bool broken = false;
  Glib::ustring infile = d_tmp_filename;
  if (infile == "")
    infile = getConfigurationFile();
  Tar_Helper t(infile, std::ios::in, broken);
  if (broken == false)
    {
      Glib::ustring bname = File::get_basename (new_file, true);
      Glib::ustring outfile = t.makeNameUnique (bname);
      Tar_Helper::reopen (&t);
      broken = !t.replaceFile("", new_file, outfile);
      t.Close();
      if (!broken)
        out = outfile;
    }
  return !broken;
}

void TarFile::clean_tmp_dir()
{
  Tar_Helper::clean_tmp_dir(getConfigurationFile());
  if (d_tmp_filename != "" && File::exists (d_tmp_filename))
    {
      File::erase (d_tmp_filename);
      d_tmp_filename = "";
    }
}

bool TarFile::saveTar(Glib::ustring tmpfile, Glib::ustring tmptar, Glib::ustring dest, std::vector<Glib::ustring> extra_files) const
{
  bool broken = false;
  Tar_Helper t(tmptar, std::ios::out, broken);
  if (broken == true)
    return false;
  t.saveFile(tmpfile, File::get_basename(dest, true));
  //now the images, go get 'em from the tarball we were made from.
  Glib::ustring infile = d_tmp_filename;
  if (infile == "")
    infile = getConfigurationFile ();
  if (infile != "")
    {
      std::list<Glib::ustring> delfiles;
      Tar_Helper orig(infile, std::ios::in, broken);
      if (broken == false)
        {
          std::list<Glib::ustring> extensions;
          extensions.push_back (".png");
          extensions.push_back (".svg");
          extensions.push_back (".ogg");
          for (auto ext : extensions)
            {
              std::list<Glib::ustring> files = orig.getFilenames(ext);
              for (std::list<Glib::ustring>::iterator it = files.begin(); 
                   it != files.end(); ++it)
                {
                  Glib::ustring file = orig.getFile(*it, broken);
                  if (broken == false)
                    {
                      t.saveFile(file);
                      delfiles.push_back(file);
                    }
                  else
                    break;
                }
              if (broken)
                break;
            }
          orig.Close();
          for (std::list<Glib::ustring>::iterator it = delfiles.begin();
               it != delfiles.end(); ++it)
            File::erase(*it);
        }
      else
        {
          FILE *fileptr = fopen (infile.c_str(), "r");
          if (fileptr)
            fclose (fileptr);
          else
            broken = false;
        }
    }
  if (extra_files.empty () == false)
    {
      for (auto f : extra_files)
        t.saveFile(f);
    }
  t.Close();
  File::erase(tmpfile);
  if (broken == false)
    {
      if (File::copy(tmptar, dest) == true)
        File::erase(tmptar);
      else
        {
          int save_errno = errno;
          //all that work for nothing
          File::erase(tmptar);
          errno = save_errno;
          broken = true;
        }
    }
  return broken == false;
}
    
void TarFile::setNewTemporaryFile ()
{
  d_tmp_filename = File::get_tmp_file ();
  bool broken = false;
  Tar_Helper t(d_tmp_filename, std::ios::out, broken);
  if (broken == true)
    return;
  t.Close ();
}

void TarFile::setLoadTemporaryFile ()
{
  Glib::ustring f = File::get_tmp_file () + d_extension;
  File::copy (getConfigurationFile (), f);
  d_tmp_filename = f;
}

guint32 TarFile::countImages ()
{
  guint32 count = 0;
  Glib::ustring infile = d_tmp_filename;
  if (infile == "")
    infile = getConfigurationFile ();
  if (infile != "")
    {
      bool broken = false;
      std::list<Glib::ustring> delfiles;
      Tar_Helper orig(infile, std::ios::in, broken);
      if (broken == false)
        {
          std::list<Glib::ustring> extensions;
          extensions.push_back (".png");
          extensions.push_back (".svg");
          for (auto ext : extensions)
            {
              std::list<Glib::ustring> files = orig.getFilenames(ext);
              for (std::list<Glib::ustring>::iterator it = files.begin(); 
                   it != files.end(); ++it)
                count++;
            }
          orig.Close();
        }
    }
  return count;
}
