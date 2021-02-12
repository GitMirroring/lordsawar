// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2014, 2015, 2020,
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

#include <config.h>

#include <fstream>
#include <iostream>
#include <string.h>
#include <string>
#include <glibmm/fileutils.h>
#include <glibmm/ustring.h>
#include <glibmm/convert.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "File.h"
#include "Configuration.h"
#include "defs.h"
#include "armyset.h"
#include "tileset.h"
#include "shieldset.h"
#include "cityset.h"
#include "file-compat.h"
#include "ucompose.hpp"
#include "rnd.h"

#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
//#define debug(x)

namespace
{
    std::list<Glib::ustring> get_files(Glib::ustring path, Glib::ustring ext)
    {
	std::list<Glib::ustring> retlist;
	Glib::Dir dir(path);
    
	for (Glib::Dir::iterator i = dir.begin(), end = dir.end(); i != end; ++i)
          {
	    Glib::ustring entry = *i;
	    Glib::ustring::size_type idx = entry.rfind(ext);
	    if (idx != Glib::ustring::npos && 
                idx == entry.length() - ext.length())
              retlist.push_back(Glib::filename_to_utf8(path + entry));
          }
	return retlist;
    }
}

bool File::nameEndsWith(Glib::ustring filename, Glib::ustring extension)
{
  Glib::ustring::size_type idx = filename.rfind(extension);
  if (idx == Glib::ustring::npos)
    return false;
  if (idx == filename.length() - extension.length())
    return true;
  return false;
}

Glib::ustring File::add_ext_if_necessary(Glib::ustring file, Glib::ustring ext)
{
  if (nameEndsWith(file, ext) == true)
    return file;
  else
    return file + ext;
}

Glib::ustring File::add_slash_if_necessary(Glib::ustring dir)
{
  if (dir.c_str()[strlen(dir.c_str())-1] == '/' ||
      dir.c_str()[strlen(dir.c_str())-1] == '\\')
    return dir;
  else
    {
      Glib::ustring d = Glib::build_filename (dir, " ");
      return String::utrim(d);
    }
}

Glib::ustring File::getVariousFile(Glib::ustring filename)
{
  return Glib::build_filename (Configuration::s_dataPath, "various", filename);
}

Glib::ustring File::getGladeFile(Glib::ustring filename)
{
  return Glib::build_filename (Configuration::s_dataPath, "glade", filename);
}

Glib::ustring File::getEditorGladeFile(Glib::ustring filename)
{
  return Glib::build_filename (Configuration::s_dataPath, "glade", "editor", filename);
}

Glib::ustring File::getMiscFile(Glib::ustring filename)
{
  return Glib::build_filename (Configuration::s_dataPath, filename);
}

Glib::ustring File::getXSLTFile(guint32 type, Glib::ustring old_version, Glib::ustring new_version)
{
  FileCompat::Type t = FileCompat::Type(type);
  Glib::ustring filename = String::ucompose("%1-%2-%3",
                                            FileCompat::typeToCode(t), 
                                            old_version, new_version);
  Glib::ustring file = getMiscFile(Glib::build_filename("various", "xslt", filename + ".xsl"));
  if (File::exists(file))
    return file;
  else
    return "";
}

Glib::ustring File::getUserProfilesDescription()
{
  return Glib::build_filename (Configuration::s_savePath, PROFILE_LIST);
}

Glib::ustring File::getUserRecentlyPlayedGamesDescription()
{
  return Glib::build_filename (Configuration::s_savePath, RECENTLY_PLAYED_LIST);
}

Glib::ustring File::getUserRecentlyHostedGamesDescription()
{
  return Glib::build_filename (Configuration::s_savePath, RECENTLY_HOSTED_LIST);
}

Glib::ustring File::getUserRecentlyAdvertisedGamesDescription()
{
  return Glib::build_filename (Configuration::s_savePath, RECENTLY_ADVERTISED_LIST);
}

Glib::ustring File::getUserRecentlyEditedFilesDescription()
{
  return Glib::build_filename (Configuration::s_savePath, RECENTLY_EDITED_LIST);
}

Glib::ustring File::getItemDescription()
{
  return Glib::build_filename (Configuration::s_dataPath, "various", "items", "items.xml");
}

Glib::ustring File::getEditorFile(Glib::ustring filename)
{
  return Glib::build_filename (Configuration::s_dataPath,  "various", "editor", filename + ".png");
}

Glib::ustring File::getMusicFile(Glib::ustring filename)
{
  return Glib::build_filename (Configuration::s_dataPath, "music", filename);
}

Glib::ustring File::getDataPath()
{
  return add_slash_if_necessary(Configuration::s_dataPath);
}

Glib::ustring File::getSavePath()
{
  return add_slash_if_necessary(Configuration::s_savePath);
}

Glib::ustring File::getSaveFile(Glib::ustring filename)
{
  return Glib::build_filename (getSavePath(), filename);
}

Glib::ustring File::getTempFile(Glib::ustring tmpdir, Glib::ustring filename)
{
  return Glib::build_filename (tmpdir, filename);
}

Glib::ustring File::getCacheDir ()
{
  return Glib::build_filename (Glib::get_user_cache_dir (), PACKAGE_NAME);
}

Glib::ustring File::getUserDataDir ()
{
  return Glib::build_filename (Glib::get_user_data_dir (), PACKAGE_NAME);
}

Glib::ustring File::getConfigDir ()
{
  return Glib::build_filename (Glib::get_user_config_dir (), PACKAGE_NAME);
}

Glib::ustring File::getConfigFile(Glib::ustring filename)
{
  return Glib::build_filename (File::getConfigDir (), filename);
}

Glib::ustring File::getTarTempDir(Glib::ustring dir)
{
  return Glib::build_filename (File::getCacheDir (),
                               String::ucompose("%1.%2", dir, getpid()));
}

Glib::ustring File::getUserMapDir()
{
  return add_slash_if_necessary(Glib::build_filename (add_slash_if_necessary(Configuration::s_savePath), MAPDIR));
}

Glib::ustring File::getMapDir()
{
  return add_slash_if_necessary (Glib::build_filename (add_slash_if_necessary(Configuration::s_dataPath), MAPDIR));
}

Glib::ustring File::getUserMapFile(Glib::ustring file)
{
  return Glib::build_filename (getUserMapDir(), file);
}

Glib::ustring File::getMapFile(Glib::ustring file)
{
  return Glib::build_filename (getMapDir(), file);
}

std::list<Glib::ustring> File::scanUserMaps()
{
  Glib::ustring path = File::getUserMapDir();
    
    std::list<Glib::ustring> retlist;
    Glib::Dir dir(path);
    
    for (Glib::Dir::iterator i = dir.begin(), end = dir.end(); i != end; ++i)
    {
      Glib::ustring entry = *i;
      Glib::ustring::size_type idx = entry.find(".map");
      if (idx != Glib::ustring::npos)
	{
	  if (entry == "random.map")
	    continue;
	  retlist.push_back(Glib::filename_to_utf8(entry));
	}
    }
    
    return retlist;
}

std::list<Glib::ustring> File::scanMaps()
{
  Glib::ustring path = File::getMapDir();
    
    std::list<Glib::ustring> retlist;
    Glib::Dir dir(path);
    
    for (Glib::Dir::iterator i = dir.begin(), end = dir.end(); i != end; ++i)
    {
      Glib::ustring entry = *i;
      Glib::ustring::size_type idx = entry.find(".map");
      if (idx != Glib::ustring::npos)
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

Glib::ustring File::get_dirname(Glib::ustring path)
{
  return Glib::path_get_dirname(path);
}

Glib::ustring File::get_basename(Glib::ustring path, bool keep_ext)
{
  if (path.empty ())
    return path;
  Glib::ustring file;
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
bool File::copy (Glib::ustring from, Glib::ustring to)
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

bool File::create_dir(Glib::ustring dir)
{
  if (Glib::file_test(dir, Glib::FILE_TEST_IS_DIR) == true)
    return true;
  bool retval = false;
  try
    {
      Glib::RefPtr<Gio::File> directory = Gio::File::create_for_path(dir);
      retval = directory->make_directory_with_parents();
    }
  catch (Gio::Error::Exception &ex)
    {
      ;
    }
  return retval;
}
	
bool File::directory_exists(Glib::ustring d)
{
  return Glib::file_test(d, Glib::FILE_TEST_IS_DIR);
}

bool File::exists(Glib::ustring f)
{
  return Glib::file_test(f, Glib::FILE_TEST_EXISTS);
}

//armysets 

std::list<Glib::ustring> File::scanForFiles(Glib::ustring dir, Glib::ustring extension)
{
  std::list<Glib::ustring> files;
  try
    {
      files = get_files (dir, extension);
    }
  catch(const Glib::Exception &ex)
    {
      return files;
    }
    return files;
}

//shieldsets

Glib::ustring File::getSetDir(Glib::ustring ext, bool system)
{
  Glib::ustring dir = add_slash_if_necessary(Configuration::s_dataPath);
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
  return "";
}

bool File::erase(Glib::ustring filename)
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

void File::erase_dir(Glib::ustring filename)
{
  if (Glib::file_test(filename, Glib::FILE_TEST_IS_DIR) == true)
    erase(filename);
}

void File::clean_dir(Glib::ustring dirname)
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

Glib::ustring File::sanify (Glib::ustring s)
{
  char *s1 = _sanify (s.c_str ());
  Glib::ustring ret(s1);
  free (s1);
  return ret;
}
  
Glib::ustring File::get_tmp_file(Glib::ustring ext)
{
  Glib::ustring file = "";
  // fixme, there's a race condition here.
  while (1)
    {
      file = Glib::build_filename (getCacheDir (),
                                   "lw." + String::ucompose ("%1", Rnd::rand () % 1000000) + ext);
      if (File::exists (file) == false)
        break;
    }
  return file;
}

Glib::ustring File::get_extension(Glib::ustring filename)
{
  if (filename.rfind('.') == Glib::ustring::npos)
    return "";
  return filename.substr(filename.rfind('.'));
}

//this method is from http://www.cplusplus.com/reference/list/list/sort/
bool case_insensitive (const Glib::ustring& first, const Glib::ustring& second)
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

bool File::rename(Glib::ustring src, Glib::ustring dest)
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

bool File::add_png_if_no_ext (Glib::ustring &filename)
{
  Glib::ustring f = filename;
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

goffset File::get_size (Glib::ustring file)
{
  auto f = Gio::File::create_for_path (file);
  return f->query_info ()->get_size ();
}
