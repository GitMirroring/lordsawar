// Copyright (C) 2017 Ben Asselstine
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

TarFile::TarFile(Glib::ustring name, Glib::ustring ext)
  : d_dir(""), d_basename(name), d_extension(ext)
{
}

TarFile::TarFile(const TarFile &s)
  : d_dir(s.d_dir), d_basename(s.d_basename), d_extension(s.d_extension)
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
  if (isTemporaryFile ())
    {
      File::erase (d_tmp_filename);
      d_tmp_filename = "";
    }
}

Glib::ustring TarFile::getConfigurationFile() const
{
  if (getBaseName () == "")
    {
      if (d_tmp_filename != "")
        return d_tmp_filename;
      else
        return "";
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

bool TarFile::removeFileInConfigurationFile(Glib::ustring file)
{
  return replaceFileInConfigurationFile(file, "");
}

bool TarFile::replaceFileInConfigurationFile(Glib::ustring file, Glib::ustring new_file)
{
  bool broken = false;
  Glib::ustring infile = d_tmp_filename;
  if (infile == "")
    infile = getConfigurationFile();
  Tar_Helper t(infile, std::ios::in, broken);
  if (broken == false)
    {
      broken = !t.replaceFile(file, new_file);
      t.Close();
    }
  return !broken;
}

bool TarFile::addFileInConfigurationFile(Glib::ustring new_file)
{
  return replaceFileInConfigurationFile("", new_file);
}

void TarFile::clean_tmp_dir()
{
  if (d_tmp_filename != "" && File::exists (d_tmp_filename))
    {
      File::erase (d_tmp_filename);
      d_tmp_filename = "";
    }
  return Tar_Helper::clean_tmp_dir(getConfigurationFile());
}

bool TarFile::saveTar(Glib::ustring tmpfile, Glib::ustring tmptar, Glib::ustring dest, bool add_sets) const
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
          extensions.push_back (".ogg");
          if (add_sets)
            {
              extensions.push_back (ARMYSET_EXT);
              extensions.push_back (TILESET_EXT);
              extensions.push_back (SHIELDSET_EXT);
              extensions.push_back (CITYSET_EXT);
            }
          for (auto ext : extensions)
            {
              std::list<Glib::ustring> files = orig.getFilenamesWithExtension(ext);
              for (std::list<Glib::ustring>::iterator it = files.begin(); 
                   it != files.end(); it++)
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
               it != delfiles.end(); it++)
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

bool TarFile::isTemporaryFile () const
{
  return d_tmp_filename != "";
}
