//  Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
//  Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
//  Copyright (C) 2004, 2005, 2006 Andrea Paternesi
//  Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2014, 2015, 2020,
//  2021, 2026 Ben Asselstine
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

#include <config.h>

#include <fstream>
#include <iostream>
#include <string.h>
#include <string>
#include <glibmm/fileutils.h>
#include <glibmm/convert.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <regex>

#include "file.h"
#include "configuration.h"
#include "defs.h"
#include "army-set.h"
#include "tile-set.h"
#include "shield-set.h"
#include "city-set.h"
#include "file-compat.h"
#include "ucompose.hpp"
#include "rnd.h"

#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
//#define debug(x)

namespace
{
    std::list<std::string> get_files(std::string path, std::string ext)
    {
	std::list<std::string> retlist;
	Glib::Dir dir(path);
    
	for (Glib::Dir::iterator i = dir.begin(), end = dir.end(); i != end; ++i)
          {
	    std::string entry = *i;
	    std::string::size_type idx = entry.rfind(ext);
	    if (idx != std::string::npos && 
                idx == entry.length() - ext.length())
              retlist.push_back(Glib::filename_to_utf8(path + entry));
          }
	return retlist;
    }
}

bool File::nameEndsWith(std::string filename, std::string extension)
{
  std::string::size_type idx = filename.rfind(extension);
  if (idx == std::string::npos)
    return false;
  if (idx == filename.length() - extension.length())
    return true;
  return false;
}

std::string File::add_ext_if_necessary(std::string file, std::string ext)
{
  if (nameEndsWith(file, ext) == true)
    return file;
  else
    return file + ext;
}

std::string File::add_slash_if_necessary(std::string dir)
{
  if (dir.c_str()[strlen(dir.c_str())-1] == '/' ||
      dir.c_str()[strlen(dir.c_str())-1] == '\\')
    return dir;
  else
    {
      std::string d = Glib::build_filename (dir, " ");
      return String::utrim(d);
    }
}

std::string File::getVariousFile(std::string filename)
{
  return Glib::build_filename (Configuration::s_dataPath, "various", filename);
}

std::string File::getEditorGladeFile(std::string filename)
{
  return Glib::build_filename (Configuration::s_dataPath, "glade", "editor", filename);
}

std::string File::getMiscFile(std::string filename)
{
  return Glib::build_filename (Configuration::s_dataPath, filename);
}

std::string File::getXSLTFile(guint32 type, std::string old_version, std::string new_version)
{
  FileCompat::Type t = FileCompat::Type(type);
  std::string filename = String::ucompose("%1-%2-%3",
                                            FileCompat::typeToCode(t), 
                                            old_version, new_version);
  std::string file = getMiscFile(Glib::build_filename("various", "xslt", filename + ".xsl"));
  if (File::exists(file))
    return file;
  else
    return "";
}

std::string File::getUserProfilesDescription()
{
  return getConfigFile (PROFILE_LIST);
}

std::string File::getUserRecentlyPlayedGamesDescription()
{
  return getConfigFile (RECENTLY_PLAYED_LIST);
}

std::string File::getUserRecentlyHostedGamesDescription()
{
  return getConfigFile (RECENTLY_HOSTED_LIST);
}

std::string File::getUserRecentlyAdvertisedGamesDescription()
{
  return getConfigFile (RECENTLY_ADVERTISED_LIST);
}

std::string File::getUserRecentlyEditedFilesDescription()
{
  return getConfigFile (RECENTLY_EDITED_LIST);
}

std::string File::getItemDescription()
{
  return Glib::build_filename (Configuration::s_dataPath, "various", "items", "items.xml");
}

std::string File::getEditorFile(std::string filename)
{
  return Glib::build_filename (Configuration::s_dataPath,  "various", "editor", filename + ".png");
}

std::string File::getMusicFile(std::string filename)
{
  return Glib::build_filename (Configuration::s_dataPath, "music", filename);
}

std::string File::getPath()
{
  return add_slash_if_necessary(Configuration::s_dataPath);
}

std::string File::getSavePath()
{
  return add_slash_if_necessary(Configuration::s_savePath);
}

std::string File::getSaveFile(std::string filename)
{
  return Glib::build_filename (getSavePath(), filename);
}

std::string File::getTempFile(std::string tmpdir, std::string filename)
{
  return Glib::build_filename (tmpdir, filename);
}

std::string File::getCacheDir ()
{
  return Glib::build_filename (Glib::get_user_cache_dir (), PACKAGE_NAME);
}

std::string File::getUserDataDir ()
{
  return Glib::build_filename (Glib::get_user_data_dir (), PACKAGE_NAME);
}

std::string File::getConfigDir ()
{
  return Glib::build_filename (Glib::get_user_config_dir (), PACKAGE_NAME);
}

std::string File::getConfigFile(std::string filename)
{
  return Glib::build_filename (File::getConfigDir (), filename);
}

std::string File::getTarTempDir(std::string dir)
{
  return Glib::build_filename (File::getCacheDir (),
                               String::ucompose("%1.%2", dir, getpid()));
}

std::string File::getUserMapFile(std::string file)
{
  return Glib::build_filename (get_user_map_dir (), file);
}

std::string File::getMapFile(std::string file)
{
  return Glib::build_filename (get_map_dir (), file);
}

std::list<std::string> File::scanUserMaps()
{
  std::string path = File::get_user_map_dir();

  if (directory_exists(path) == false)
    create_dir(path);
  std::list<std::string> retlist;
  Glib::Dir dir(path);

  for (Glib::Dir::iterator i = dir.begin(), end = dir.end(); i != end; ++i)
    {
      std::string entry = *i;
      std::string::size_type idx = entry.find(".map");
      if (idx != std::string::npos)
        retlist.push_back(Glib::filename_to_utf8(entry));
    }

  return retlist;
}

std::list<std::string> File::scanMaps()
{
  std::string path = File::get_map_dir();
    
    std::list<std::string> retlist;
    Glib::Dir dir(path);
    
    for (Glib::Dir::iterator i = dir.begin(), end = dir.end(); i != end; ++i)
    {
      std::string entry = *i;
      std::string::size_type idx = entry.find(".map");
      if (idx != std::string::npos)
	{
	    retlist.push_back(Glib::filename_to_utf8(entry));
	}
    }
    
    if (retlist.empty())
    {
      std::cerr << _("Error: Couldn't find a single map!") << std::endl;
      std::cerr << String::ucompose(_("Please check the path settings in %1"), File::getConfigFile(DEFAULT_CONFIG_FILENAME)) << std::endl;
    }

    return retlist;
}

std::string File::get_dirname(std::string path)
{
  return Glib::path_get_dirname(path);
}

std::string File::get_basename(std::string path, bool keep_ext)
{
  if (path.empty ())
    return path;
  std::string file;
  file = Glib::path_get_basename(path);
  if (keep_ext)
    return file;
  //now strip everything past the last dot.
  const char *tmp = strrchr (file.c_str(), '.');
  if (!tmp)
    return file;
  int npos = tmp - file.c_str() + 1;
  file = file.substr(0, npos - 1);
  return file;
}

//copy_file taken from ardour-2.0rc2, gplv2+.
/*
bool File::copy (std::string from, std::string to)
{
  std::ifstream in;
  std::ofstream out;
  in.open(from.c_str(), std::ios::in | std::ios::binary);
  out.open(to.c_str(), std::ios::out | std::ios::binary);

  if (!in)
    return false;

  if (!out)
    return false;

  out << in.rdbuf();

  if (!in || !out) 
    {
      File::erase(to);
      return false;
    }

  return true;
}
*/
bool File::copy (std::string from, std::string to)
{
    auto source = Gio::File::create_for_path (from);
    auto dest = Gio::File::create_for_path (to);

    try
      {
        return source->copy (dest, Gio::File::CopyFlags::OVERWRITE);
      }
    catch (const Glib::Error& ex)
      {
        std::cerr << "Copy from " << source->get_path () << " to " <<
          dest->get_path () << " " << " failed: " << ex.what() << '\n';
        return false;
      }
}

bool File::create_dir(std::string dir)
{
  if (Glib::file_test(dir, Glib::FileTest::IS_DIR) == true)
    return true;
  if (Glib::file_test(dir, Glib::FileTest::IS_REGULAR) == true)
    File::erase (dir);
  bool retval = false;
  try
    {
      Glib::RefPtr<Gio::File> directory = Gio::File::create_for_path(dir);
      retval = directory->make_directory_with_parents();
    }
  catch (Gio::Error &ex)
    {
      ;
    }
  return retval;
}
	
bool File::directory_exists(std::string d)
{
  return Glib::file_test(d, Glib::FileTest::IS_DIR);
}

bool File::exists(std::string f)
{
  return Glib::file_test(f, Glib::FileTest::EXISTS);
}

//armysets 

std::list<std::string> File::scanForFiles(std::string dir, std::string extension)
{
  std::list<std::string> files;
  try
    {
      files = get_files (dir, extension);
    }
  catch(const Glib::Error &ex)
    {
      return files;
    }
    return files;
}

//shieldsets

std::string File::getSetDir(std::string ext, bool system)
{
  std::string dir = add_slash_if_necessary(Configuration::s_dataPath);
  if (system == false)
    dir = getSavePath();
  if (ext == ARMYSET_EXT)
    return add_slash_if_necessary (Glib::build_filename (dir, ARMYSETDIR));
  else if (ext == CITYSET_EXT)
    return add_slash_if_necessary (Glib::build_filename (dir, CITYSETDIR));
  else if (ext == TILESET_EXT)
    return add_slash_if_necessary (Glib::build_filename (dir, TILESETDIR));
  else if (ext == SHIELDSET_EXT)
    return add_slash_if_necessary (Glib::build_filename (dir, SHIELDSETDIR));
  else if (ext == MAP_EXT)
    return add_slash_if_necessary (Glib::build_filename (dir, MAPDIR));
  return "";
}

bool File::erase(std::string filename)
{
  bool success = true;
  if (File::exists(filename))
    {
      Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(filename);
      try 
        {
          file->remove();
        } 
      catch (const Glib::Error &ex) 
        {
          std::cerr << ex.what() << " " << filename << std::endl;
          success = false;
        }
    }
  else
    success = false;
  return success;
}

void File::erase_dir(std::string filename)
{
  if (Glib::file_test(filename, Glib::FileTest::IS_DIR) == true)
    erase(filename);
}

void File::clean_dir(std::string dirname)
{
  if (File::exists(dirname) == false)
    return;
  Glib::Dir dir(dirname);
  for (Glib::DirIterator it = dir.begin(); it != dir.end(); ++it)
    File::erase(File::add_slash_if_necessary(dirname) + *it);
  dir.close();
  File::erase_dir(dirname);
}

char *File::_sanify(const char *string)
{
  char *result = NULL;
  size_t resultlen = 1;
  size_t len = strlen(string);
  result = (char*) malloc (resultlen);
  result[0] = '\0';
  for (unsigned int i = 0; i < len; i++)
    {
      int letter = tolower(string[i]);
      if (strchr("abcdefghijklmnopqrstuvwxyz0123456789-", letter) == NULL)
	continue;

      resultlen++;
      result = (char *) realloc (result, resultlen);
      if (result)
	{
	  result[resultlen-2] = char(letter);
	  result[resultlen-1] = '\0';
	}
    }
  return result;
}

std::string File::sanify (std::string s)
{
  char *s1 = _sanify (s.c_str ());
  std::string ret(s1);
  free (s1);
  return ret;
}
  
std::string File::get_tmp_file (std::string ext)
{
  std::string filename;
  int fd = Glib::file_open_tmp (filename, std::string (PACKAGE) + "-");
  close (fd);
  return filename + ext;
}

std::string File::get_extension(std::string filename)
{
  if (filename.rfind('.') == std::string::npos)
    return "";
  return filename.substr(filename.rfind('.'));
}

//this method is from http://www.cplusplus.com/reference/list/list/sort/
bool case_insensitive (const std::string& first, const std::string& second)
{
  unsigned int i = 0;
  while (i < first.length () && i < second.length ())
    {
      if (tolower (first[i]) < tolower (second[i])) 
        return true;
      else if (tolower (first[i]) > tolower (second[i])) 
        return false;
      ++i;
    }
  return (first.length() < second.length());
}

bool File::rename(std::string src, std::string dest)
{
  bool result = false;
  if (File::exists(src) && File::exists(dest) == false)
    {
      try
        {
          Glib::RefPtr<Gio::File> f = Gio::File::create_for_path(src);
          result = f->move (Gio::File::create_for_path(dest));
        }
      catch (Gio::Error &ex)
        {
          ;
        }
    }
  return result;
}

bool File::add_png_if_no_ext (std::string &filename)
{
  std::string f = filename;
  //in the old days we had filenames without the extensions in our
  //army/city/shield/tilesets.
  //now we keep the extensions, but to maintain backwards compatibility
  //we rejig the filenames as we load them in just in case we have an old
  //file.
  //the altenative to this approach is to increment the version numbers on
  //those files and make xslt templates to give them an upgrade path.
  if (f != "" && File::get_extension (f) == "")
    {
      filename = f + ".png";
      return true;
    }
  return false;
}

goffset File::get_size (std::string file)
{
  auto f = Gio::File::create_for_path (file);
  return f->query_info ()->get_size ();
}

std::string File::getRandomlyGeneratedMapFile ()
{
  char buf[32];
  snprintf (buf, sizeof (buf), "%d", ::getpid ());
  return "random-" + std::string (buf) + MAP_EXT;
}

bool File::is_readonly (std::string path)
{
  auto file = Gio::File::create_for_path (path);

  try
    {
      auto info = file->query_info (G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
      return !info->get_attribute_boolean (G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
    }
  catch (const Glib::Error&)
    {
      return false; // handle errors as appropriate
    }
}

std::string File::get_user_shieldset_dir ()
{
  return getSetDir (SHIELDSET_EXT, false);
}

std::string File::get_user_cityset_dir ()
{
  return getSetDir (CITYSET_EXT, false);
}

std::string File::get_user_armyset_dir ()
{
  return getSetDir (ARMYSET_EXT, false);
}

std::string File::get_user_tileset_dir ()
{
  return getSetDir (TILESET_EXT, false);
}

std::string File::get_user_map_dir ()
{
  return getSetDir (MAP_EXT, false);
}

std::string File::get_shieldset_dir ()
{
  return getSetDir (SHIELDSET_EXT, true);
}

std::string File::get_cityset_dir ()
{
  return getSetDir (CITYSET_EXT, true);
}

std::string File::get_armyset_dir ()
{
  return getSetDir (ARMYSET_EXT, true);
}

std::string File::get_tileset_dir ()
{
  return getSetDir (TILESET_EXT, true);
}

std::string File::get_map_dir ()
{
  return getSetDir (MAP_EXT, true);
}

