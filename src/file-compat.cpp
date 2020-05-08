// Copyright (C) 2011, 2014 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>

#include <cstdio>
#include <limits.h>
#include <fstream>
#include <iostream>
#include <libxml/xmlmemory.h>
#include <libxslt/xslt.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>

#include "armyset.h"
#include "tileset.h"
#include "shieldset.h"
#include "cityset.h"
#include "xmlhelper.h"
#include "Configuration.h"
#include "defs.h"
#include "File.h"
#include "file-compat.h"
#include "tarhelper.h"
#include "GameScenario.h"
#include "ucompose.hpp"
#include "Itemlist.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

FileCompat* FileCompat::s_instance = 0;

FileCompat* FileCompat::getInstance()
{
  if (s_instance == 0)
    s_instance = new FileCompat();

  return s_instance;
}

void FileCompat::deleteInstance()
{
  if (s_instance)
    delete s_instance;

  s_instance = 0;
}

bool FileCompat::contains(FileCompat::Type type) const
{
  for (const_iterator i = begin(); i != end(); ++i)
    if ((*i).type == type)
      return true;
  return false;
}

void FileCompat::support_backward_compatibility_for_common_files()
{
  if (FileCompat::getInstance()->contains(CONFIGURATION) == false)
    Configuration::support_backward_compatibility();
  if (FileCompat::getInstance()->contains(ITEMLIST) == false)
    Itemlist::support_backward_compatibility();
  if (FileCompat::getInstance()->contains(ARMYSET) == false)
    Armyset::support_backward_compatibility();
  if (FileCompat::getInstance()->contains(TILESET) == false)
    Tileset::support_backward_compatibility();
  if (FileCompat::getInstance()->contains(CITYSET) == false)
    Cityset::support_backward_compatibility();
  if (FileCompat::getInstance()->contains(SHIELDSET) == false)
    Shieldset::support_backward_compatibility();
  if (FileCompat::getInstance()->contains(GAMESCENARIO) == false)
    GameScenario::support_backward_compatibility();
}

FileCompat::FileCompat()
{
}

FileCompat::Type FileCompat::getType(Glib::ustring filename) const
{
  if (File::exists(filename) == false)
    return UNKNOWN;
  for (const_iterator i = begin(); i != end(); ++i)
    {
      if (File::nameEndsWith(filename, (*i).file_extension) == true &&
          (*i).file_extension != ".xml")
        {
          return FileCompat::Type((*i).type);
        }
    }
  bool tar = false;
  return getTypeByFileInspection(filename, tar);
}
  
FileCompat::Type FileCompat::getTypeByTarFileInspection(Glib::ustring filename) const
{
  bool broken = false;
  Tar_Helper t(filename, std::ios::in, broken);
  if (broken)
    return UNKNOWN;

  std::list<Glib::ustring> files = t.getFilenames();
  t.Close();
  std::list<FileDetails> details;
  //whittle down the files it can't be
  for (std::list<Glib::ustring>::iterator i = files.begin(); i != files.end(); 
       ++i)
    {
      bool found = false;
      for (const_iterator j = begin(); j != end(); ++j)
        {
          if (File::nameEndsWith(*i, (*j).file_extension) == true)
            {
              if ((*j).type == GAMESCENARIO)
                return GAMESCENARIO;
              details.push_back(*j);
              found = true;
              break;
            }
        }
      if (!found)
        {
          i = files.erase(i);
          continue;
        }
    }

  if (details.size() == 0)
    return UNKNOWN;
  else
    return FileCompat::Type(details.front().type);
}

FileCompat::Type FileCompat::getTypeByXmlFileInspection(Glib::ustring filename) const
{
  Glib::ustring tag = XML_Helper::get_top_tag(filename);

  if (tag == "")
    return UNKNOWN;
  for (const_iterator i = begin(); i != end(); ++i)
    {
      if (tag == (*i).tag)
        return FileCompat::Type((*i).type);
    }
  return UNKNOWN;
}

FileCompat::Type FileCompat::getTypeByFileInspection(Glib::ustring filename, bool &tar) const
{
  Type type = getTypeByTarFileInspection(filename);
  tar = type != UNKNOWN;
  if (type == UNKNOWN)
    return getTypeByXmlFileInspection(filename);
  else
    return type;
}

bool FileCompat::get_tag_and_version_from_file(Glib::ustring filename, FileCompat::Type type, Glib::ustring &tag, Glib::ustring &version) const
{
  bool broken = false;
  if (isTarFile(type) == true)
    {
      std::list<Glib::ustring> ext = getFileExtensions(type);
      if (ext.empty() == true)
        return false;
      Tar_Helper t(filename, std::ios::in, broken);
      if (!broken)
        {
          Glib::ustring tmpfile = "";
          for (std::list<Glib::ustring>::iterator i = ext.begin(); 
               i != ext.end(); ++i)
            {
              tmpfile = t.getFirstFile(*i, broken);
              if (!broken && tmpfile.empty() == false)
                {
                  XML_Helper helper(tmpfile, std::ios::in);
                  tag = XML_Helper::get_top_tag(tmpfile);
                  VersionLoader l(tmpfile, tag, version, broken);
                  t.Close();
                  File::erase(tmpfile);
                  return !broken;
                }
            }
        }
    }
  else
    {
      tag = XML_Helper::get_top_tag(filename);
      VersionLoader l(filename, tag, version, broken);
    }
  return !broken;
}

bool FileCompat::isTarFile(FileCompat::Type type) const
{
  for (const_iterator i = begin(); i != end(); ++i)
    if ((*i).type == type)
      return (*i).tar;
  return false;
}

Glib::ustring FileCompat::getFileExtension(FileCompat::Type type) const
{
  for (const_iterator i = begin(); i != end(); ++i)
    if ((*i).type == type)
      return (*i).file_extension;
  return "";
}

Glib::ustring FileCompat::getTag(FileCompat::Type type) const
{
  for (const_iterator i = begin(); i != end(); ++i)
    if ((*i).type == type)
      return (*i).tag;
  return "";
}

bool FileCompat::upgrade(Glib::ustring filename, bool &same) const
{
  Glib::ustring tag, version;
  bool upgraded = false;
  if (File::exists(filename) == false)
    return false;
  Type type = getType(filename);
  if (type == UNKNOWN)
    return false;
  if (get_tag_and_version_from_file (filename, type, tag, version) == false)
    return false;

  //can we get there from here?
  Slot slot;
  Glib::ustring next_version;
  if (get_upgrade_method(type, version, next_version, slot) == false)
    {
      same = can_upgrade_to(type, version);
      return false;
    }

  bool broken = false;
  // ride the upgrade train as far as we can.
  while (1)
    {
      if (get_upgrade_method(type, version, next_version, slot))
        {
          upgraded = (slot)(filename, version, next_version);
          version = next_version;
          if (upgraded == false)
            {
              broken = true;
              break;
            }
        }
      else
        break;
    }
  return !broken;
}
      
bool FileCompat::can_upgrade_to(FileCompat::Type type, Glib::ustring version) const
{
  for (std::list<UpgradeDetails>::const_iterator i = versions[type].begin();
       i != versions[type].end(); ++i)
    {
      if ((*i).to_version == version)
        return true;
    }
  return false;
}

bool FileCompat::get_upgrade_method(FileCompat::Type type, Glib::ustring version, Glib::ustring &next_version, FileCompat::Slot &slot) const
{
  for (std::list<UpgradeDetails>::const_iterator i = versions[type].begin();
       i != versions[type].end(); ++i)
    {
      if ((*i).from_version == version)
        {
          next_version = (*i).to_version;
          slot = (*i).slot;
          return true;
        }
    }
  return false;
}

bool FileCompat::rewrite_with_updated_version(Glib::ustring filename, FileCompat::Type type, Glib::ustring tag, Glib::ustring version) const
{
  bool broken = false;
  bool upgraded = false;
  if (isTarFile(type) && type != GAMESCENARIO)
    {
      Glib::ustring ext = getFileExtension(type);

      Tar_Helper t(filename, std::ios::in, broken);
      if (broken == false)
        {
          Glib::ustring tmpfile = t.getFirstFile(ext, broken);
          if (broken == false && version != "")
            upgraded = XML_Helper::rewrite_version(tmpfile, tag, version);
          if (upgraded)
            {
              Glib::ustring n = t.getFirstFilename(ext);
              t.replaceFile (n, tmpfile, n);
            }
          t.Close();
          if (tmpfile != "")
            File::erase(tmpfile);
        }
    }
  else if (isTarFile(type) && type == GAMESCENARIO)
    {
      bool upgraded_armyset = false, upgraded_tileset = false, 
           upgraded_cityset = false, upgraded_shieldset = false;
      return upgradeGameScenario(filename, version, upgraded_armyset, 
                                 upgraded_tileset, upgraded_cityset,
                                 upgraded_shieldset);
    }
  else if (isTarFile(type) == false)
    {
      if (version != "")
        upgraded = XML_Helper::rewrite_version(filename, tag, version);
    }
  return upgraded;
}

FileCompat::Type FileCompat::getTypeByFileExtension(Glib::ustring ext) const
{
  for (const_iterator i = begin(); i != end(); ++i)
    if ((*i).file_extension == ext)
      return FileCompat::Type((*i).type);
  return UNKNOWN;
}

bool FileCompat::upgradeGameScenario(Glib::ustring filename, Glib::ustring version, bool& upgraded_armyset, bool& upgraded_tileset, bool& upgraded_cityset, bool& upgraded_shieldset) const
{
  Glib::ustring ext = File::get_extension(filename);
  if (ext == "")
    return false;
  if (getTypeByFileExtension(ext) != GAMESCENARIO)
    return false;
  bool upgraded = false;
  bool broken = false;
  Tar_Helper t(filename, std::ios::in, broken);
  if (!broken)
    {
      Glib::ustring tmpfile = t.getFirstFile(ext, broken);
      if (broken == false)
        upgraded = XML_Helper::rewrite_version(tmpfile, getTag(GAMESCENARIO), 
                                               version);
      std::list<Glib::ustring> delfiles;
      delfiles.push_back(tmpfile);
      if (upgraded)
        {
          bool same;
          Glib::ustring n = t.getFirstFilename(ext);
          t.replaceFile (n, tmpfile, n);
          //now we need to upgrade the other files.
          Glib::ustring f = t.getFirstFilename (getFileExtension(ARMYSET));
          tmpfile = t.getFile(f, broken);
          if (tmpfile != "")
            {
              same = false;
              if (upgrade(tmpfile, same))
                {
                  if (!same)
                    {
                      upgraded_armyset = true;
                      t.replaceFile (f, tmpfile, f);
                    }
                }
              delfiles.push_back(tmpfile);
            }
          tmpfile = t.getFirstFile(getFileExtension(TILESET), broken);
          if (tmpfile != "")
            {
              same = false;
              if (upgrade(tmpfile, same))
                {
                  if (!same)
                    {
                      upgraded_tileset = true;
                      n = t.getFirstFilename (getFileExtension(TILESET));
                      t.replaceFile (n, tmpfile, n);
                    }
                }
              delfiles.push_back(tmpfile);
            }
          tmpfile = t.getFirstFile(getFileExtension(CITYSET), broken);
          if (tmpfile != "")
            {
              same = false;
              if (upgrade(tmpfile, same))
                {
                  if (!same)
                    {
                      upgraded_cityset = true;
                      n = t.getFirstFilename (getFileExtension(CITYSET));
                      t.replaceFile (n, tmpfile, n);
                    }
                }
              delfiles.push_back(tmpfile);
            }
          tmpfile = t.getFirstFile(getFileExtension(SHIELDSET), broken);
          if (tmpfile != "")
            {
              same = false;
              if (upgrade(tmpfile, same))
                {
                  if (!same)
                    {
                      upgraded_shieldset = true;
                      n = t.getFirstFilename (getFileExtension(SHIELDSET));
                      t.replaceFile (n, tmpfile, n);
                    }
                }
              delfiles.push_back(tmpfile);
            }
        }
      t.Close();
      for (std::list<Glib::ustring>::iterator i = delfiles.begin(); i != delfiles.end(); ++i)
        File::erase(*i);
    }
  return upgraded;
}

void FileCompat::initialize()
{
  bool same;
  if (contains(GAMELIST))
    upgrade(File::getUserRecentlyAdvertisedGamesDescription(), same);
  if (contains(GAMELIST))
    upgrade(File::getUserRecentlyHostedGamesDescription(), same);
  if (contains(PROFILELIST))
    upgrade(File::getUserProfilesDescription(), same);
  if (contains(RECENTLYPLAYEDGAMELIST))
    upgrade(File::getUserRecentlyPlayedGamesDescription(), same);
}

Glib::ustring FileCompat::typeToString(const FileCompat::Type type)
{
  switch (type)
    {
    case UNKNOWN: return _("unknown file");
    case CONFIGURATION: return _("primary configuration file");
    case ITEMLIST: return _("item description file");
    case PROFILELIST: return _("profiles file");
    case RECENTLYPLAYEDGAMELIST: return _("recently played games file");
    case GAMELIST: return _("recently hosted or recently advertised games file");
    case ARMYSET: return _("armyset file");
    case TILESET: return _("tileset file");
    case CITYSET: return _("cityset file");
    case SHIELDSET: return _("shieldset file");
    case GAMESCENARIO: return _("map or saved-game file");
    }
  return _("unknown file");
}

Glib::ustring FileCompat::typeToCode(const FileCompat::Type type)
{
  switch (type)
    {
    case UNKNOWN: return "";
    case CONFIGURATION: return "c";
    case ITEMLIST: return "il";
    case PROFILELIST: return "pl";
    case RECENTLYPLAYEDGAMELIST: return "rpg";
    case GAMELIST: return "gl";
    case ARMYSET: return "as";
    case TILESET: return "ts";
    case CITYSET: return "cs";
    case SHIELDSET: return "ss";
    case GAMESCENARIO: return "gs";
    }
  return "";
}

void FileCompat::support_version(guint32 k, Glib::ustring from, Glib::ustring to, FileCompat::Slot slot)
{
  versions[FileCompat::Type(k)].push_back(UpgradeDetails(from, to, slot));
}

bool FileCompat::rewrite_with_xslt(Glib::ustring filename, FileCompat::Type type, Glib::ustring xsl_file) const
{
  bool broken = false;
  bool upgraded = false;
  if (isTarFile(type) && type != GAMESCENARIO)
    {
      Glib::ustring ext = getFileExtension(type);

      Tar_Helper t(filename, std::ios::in, broken);
      if (broken == false)
        {
          Glib::ustring tmpfile = t.getFirstFile(ext, broken);
          Glib::ustring n = t.getFirstFilename(ext);
          if (broken == false)
            upgraded = xsl_transform(tmpfile, xsl_file);
          if (upgraded)
            t.replaceFile (n, tmpfile, n);
          t.Close();
          if (tmpfile != "")
            File::erase(tmpfile);
        }
    }
  else if (isTarFile(type) && type == GAMESCENARIO)
    {
      bool armyset_upgraded, tileset_upgraded, cityset_upgraded, 
           shieldset_upgraded = false;
      return upgradeGameScenarioWithXslt(filename, xsl_file, armyset_upgraded,
                                         tileset_upgraded, cityset_upgraded,
                                         shieldset_upgraded);
    }
  else if (isTarFile(type) == false)
    {
      upgraded = xsl_transform(filename, xsl_file);
    }
  return upgraded;
}

bool FileCompat::upgradeGameScenarioWithXslt(Glib::ustring filename, Glib::ustring xsl_file, bool& armyset_upgraded, bool& tileset_upgraded, bool& cityset_upgraded, bool &shieldset_upgraded) const
{
  Glib::ustring ext = File::get_extension(filename);
  if (ext == "")
    return false;
  if (getTypeByFileExtension(ext) != GAMESCENARIO)
    return false;
  bool upgraded = false;
  bool broken = false;
  Tar_Helper t(filename, std::ios::in, broken);
  if (!broken)
    {
      Glib::ustring tmpfile = t.getFirstFile(ext, broken);
      if (broken == false)
        upgraded = xsl_transform(tmpfile, xsl_file);

      std::list<Glib::ustring> delfiles;
      delfiles.push_back(tmpfile);
      if (upgraded)
        {
          bool same;
          Glib::ustring n = t.getFirstFilename(ext);
          t.replaceFile (n, tmpfile, n);
          //now we need to upgrade the other files.
          Glib::ustring f = t.getFirstFilename (getFileExtension(ARMYSET));
          tmpfile = t.getFile(f, broken);
          if (tmpfile != "")
            {
              same = false;
              if (upgrade(tmpfile, same))
                {
                  if (!same)
                    {
                      armyset_upgraded = true;
                      t.replaceFile (f, tmpfile, f);
                    }
                }
              delfiles.push_back(tmpfile);
            }
          tmpfile = t.getFirstFile(getFileExtension(TILESET), broken);
          if (tmpfile != "")
            {
              same = false;
              if (upgrade(tmpfile, same))
                {
                  if (!same)
                    {
                      tileset_upgraded = true;
                      n = t.getFirstFilename (getFileExtension(TILESET));
                      t.replaceFile (n, tmpfile, n);
                    }
                }
              delfiles.push_back(tmpfile);
            }
          tmpfile = t.getFirstFile(getFileExtension(CITYSET), broken);
          if (tmpfile != "")
            {
              same = false;
              if (upgrade(tmpfile, same))
                {
                  if (!same)
                    {
                      cityset_upgraded = true;
                      n = t.getFirstFilename (getFileExtension(CITYSET));
                      t.replaceFile (n, tmpfile, n);
                    }
                }
              delfiles.push_back(tmpfile);
            }
          tmpfile = t.getFirstFile(getFileExtension(SHIELDSET), broken);
          if (tmpfile != "")
            {
              same = false;
              if (upgrade(tmpfile, same))
                {
                  if (!same)
                    {
                      shieldset_upgraded = true;
                      n = t.getFirstFilename (getFileExtension(SHIELDSET));
                      t.replaceFile (n, tmpfile, n);
                    }
                }
              delfiles.push_back(tmpfile);
            }
        }
      for (std::list<Glib::ustring>::iterator i = delfiles.begin(); i != delfiles.end(); ++i)
        File::erase(*i);
      t.Close();
    }
  return upgraded;
}

        
bool FileCompat::xsl_transform(Glib::ustring filename, Glib::ustring xsl_file) const
{
  const char *params[16 + 1];
  //int nbparams = 0;
  memset (params, 0, sizeof (params));
  xsltStylesheetPtr cur = NULL;
  xmlDocPtr doc, res;

  xmlChar *xsl = xmlCharStrdup(xsl_file.c_str());
  cur = xsltParseStylesheetFile(xsl);
  if (cur == NULL)
    return false;
  doc = xmlParseFile(filename.c_str());
  if (doc == NULL)
    return false;
  res = xsltApplyStylesheet(cur, doc, params);
  if (res == NULL)
    return false;

  Glib::ustring tmpfile = File::get_tmp_file();

  xmlChar *out = xmlCharStrdup(tmpfile.c_str());
  xsltSaveResultToFilename(tmpfile.c_str(), res, cur, 0);
  xsltFreeStylesheet(cur);
  xmlFreeDoc(res);
  xmlFreeDoc(doc);
  free (xsl);
  free (out);

  xsltCleanupGlobals();
  xmlCleanupParser();
  File::erase(filename);
  File::rename(tmpfile, filename);
  return true;
}

bool FileCompat::upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version, FileCompat::Type type, Glib::ustring tag) const
{
  Glib::ustring xsl_filename = File::getXSLTFile(type, old_version, new_version);
  if (xsl_filename != "")
    return rewrite_with_xslt (filename, type, xsl_filename);
  else
    return rewrite_with_updated_version (filename, type, tag, new_version);
}
        
std::list<Glib::ustring> FileCompat::getFileExtensions(FileCompat::Type type) const
{
  std::list<Glib::ustring> ext;
  for (const_iterator i = begin(); i != end(); ++i)
    if ((*i).type == type)
      ext.push_back((*i).file_extension);
  return ext;
}

// End of file
