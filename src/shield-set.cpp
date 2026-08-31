//  Copyright (C) 2008, 2009, 2010, 2011, 2014, 2015, 2020,
//  2021 Ben Asselstine
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

#include <iostream>
#include "rectangle.h"
#include <sigc++/functors/mem_fun.h>

#include <string.h>
#include "shieldset.h"
#include "shieldstyle.h"
#include "File.h"
#include "Configuration.h"
#include "tarhelper.h"
#include "file-compat.h"
#include "ucompose.hpp"
#include "xmlhelper.h"
#include "TarFileMaskedImage.h"

Glib::ustring Shieldset::d_tag = "shieldset";
Glib::ustring Shieldset::file_extension = SHIELDSET_EXT;

#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
//#define debug(x)

Shieldset::Shieldset(guint32 id, Glib::ustring name)
 : Set(SHIELDSET_EXT, id, name, 0), d_small_height(0), d_small_width(0), 
    d_medium_height(0), d_medium_width(0), d_large_height(0), d_large_width(0)
{
}

Shieldset::Shieldset(const Shieldset& s)
 : std::list<Shield*>(), sigc::trackable(s), Set(s), 
    d_small_height(s.d_small_height), d_small_width(s.d_small_width),
    d_medium_height(s.d_medium_height), d_medium_width(s.d_medium_width),
    d_large_height(s.d_large_height), d_large_width(s.d_large_width)
{
  for (const_iterator it = s.begin(); it != s.end(); ++it)
    push_back(new Shield(*(*it)));
}

Shieldset::Shieldset(XML_Helper *helper, Glib::ustring directory)
 : Set(SHIELDSET_EXT, helper, directory)
{
  setTileSize(0);
  helper->getData(d_small_width, "small_width");
  helper->getData(d_small_height, "small_height");
  helper->getData(d_medium_width, "medium_width");
  helper->getData(d_medium_height, "medium_height");
  helper->getData(d_large_width, "large_width");
  helper->getData(d_large_height, "large_height");
  helper->registerTag(Shield::d_tag, 
		      sigc::mem_fun((*this), &Shieldset::loadShield));
  helper->registerTag(ShieldStyle::d_tag, sigc::mem_fun((*this), 
							&Shieldset::loadShield));
  helper->registerTag(Tartan::d_tartan_tag, sigc::mem_fun((*this),
                                                          &Shieldset::loadShield));
  clear();
}

Shieldset::~Shieldset()
{
  uninstantiateImages();
  for (iterator it = begin(); it != end(); ++it)
    delete *it;
  clean_tmp_dir();
}

ShieldStyle * Shieldset::lookupShieldByTypeAndColor(guint32 type, guint32 color) const
{
  for (const_iterator it = begin(); it != end(); ++it)
    {
      for (Shield::const_iterator i = (*it)->begin(); i != (*it)->end(); ++i)
	{
	  if ((*i)->getType() == type && (*it)->getOwner() == color)
	    return *i;
	}
    }
  return NULL;
}

Shield * Shieldset::lookupShieldByColor (guint32 color) const
{
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getOwner() == color)
        return *it;
    }
  return NULL;
}

Gdk::RGBA Shieldset::getColor(guint32 owner) const
{
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getOwner() == owner)
	return (*it)->getColor();
    }
  return Gdk::RGBA("black");
}

std::vector<Gdk::RGBA> Shieldset::getColors(guint32 owner) const
{
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getOwner() == owner)
	return (*it)->getColors();
    }
  std::vector<Gdk::RGBA> l;
  l.push_back (Gdk::RGBA("black"));
  return l;
}

bool Shieldset::loadShield(Glib::ustring tag, XML_Helper* helper)
{
  if (tag == Shield::d_tag)
    {
      Shield* sh = new Shield(helper);
      push_back(sh);
      return true;
    }
  if (tag == ShieldStyle::d_tag)
    {
      ShieldStyle *sh = new ShieldStyle(helper);
      (*back()).push_back(sh);
      return true;
    }
  if (tag == Tartan::d_tartan_tag)
    {
      Tartan * t = new Tartan(helper);
      back()->getTartanMaskedImage(Tartan::LEFT)->setName
                                   (t->getTartanMaskedImage(Tartan::LEFT)->getName ());
      back()->getTartanMaskedImage(Tartan::CENTER)->setName
                                   (t->getTartanMaskedImage(Tartan::CENTER)->getName ());
      back()->getTartanMaskedImage(Tartan::RIGHT)->setName
                                   (t->getTartanMaskedImage(Tartan::RIGHT)->getName ());
      delete t;
      return true;
    }
  return false;
}

//! Helper class for making a new Shieldset object from a shieldset file.
class ShieldsetLoader
{
public:
    ShieldsetLoader(Glib::ustring filename, bool &broken, bool &unsupported)
      : dir (File::get_dirname(filename)), file (File::get_basename(filename)),
      shieldset (NULL), unsupported_version (false)
      {
	if (File::nameEndsWith(filename, Shieldset::file_extension) == false)
	  filename += Shieldset::file_extension;
        Tar_Helper t(filename, std::ios::in, broken);
        if (broken)
          return;
        Glib::ustring lwsfilename = 
          t.getFirstFile(Shieldset::file_extension, broken);
        if (broken)
          return;
	XML_Helper helper(lwsfilename, std::ios::in);
	helper.registerTag(Shieldset::d_tag, sigc::mem_fun((*this), &ShieldsetLoader::load));
	if (!helper.parseXML())
	  {
            unsupported = unsupported_version;
            std::cerr << String::ucompose(_("Error!  can't load Shield Set `%1'."), filename) << std::endl;
	    if (shieldset != NULL)
	      delete shieldset;
	    shieldset = NULL;
	  }
        helper.close();
        File::erase(lwsfilename);
        t.Close();
      };
    bool load(Glib::ustring tag, XML_Helper* helper)
      {
	if (tag == Shieldset::d_tag)
	  {
            if (helper->getVersion() == LORDSAWAR_SHIELDSET_VERSION)
              {
                shieldset = new Shieldset(helper, dir);
                shieldset->setBaseName(file);
                return true;
              }
            else
              {
                unsupported_version = true;
                return false;
              }
	  }
	return false;
      };
    Glib::ustring dir;
    Glib::ustring file;
    Shieldset *shieldset;
    bool unsupported_version;
};

Shieldset *Shieldset::create(Glib::ustring filename, bool &unsupported_version)
{
  bool broken = false;
  ShieldsetLoader d(filename, broken, unsupported_version);
  if (broken)
    return NULL;
  return d.shieldset;
}

bool Shieldset::save(Glib::ustring filename, Glib::ustring ext) const
{
  bool broken = false;
  Glib::ustring goodfilename = File::add_ext_if_necessary(filename, ext);
  Glib::ustring tmpfile = File::get_tmp_file();
  XML_Helper helper(tmpfile, std::ios::out);
  helper.begin(LORDSAWAR_SHIELDSET_VERSION);
  broken = !save(&helper);
  helper.close();
  if (broken == true)
    return false;
  std::vector<Glib::ustring> extrafiles;
  return saveTar(tmpfile, tmpfile + ".tar", goodfilename, extrafiles);
}

bool Shieldset::save(XML_Helper *helper) const
{
  bool retval = true;

  retval &= helper->openTag(d_tag);
  retval &= Set::save(helper);
  retval &= helper->saveData("small_width", d_small_width);
  retval &= helper->saveData("small_height", d_small_height);
  retval &= helper->saveData("medium_width", d_medium_width);
  retval &= helper->saveData("medium_height", d_medium_height);
  retval &= helper->saveData("large_width", d_large_width);
  retval &= helper->saveData("large_height", d_large_height);
  for (const_iterator it = begin(); it != end(); ++it)
    retval &= (*it)->save(helper);
  retval &= helper->closeTag();
  return retval;
}

bool Shieldset::validate() const
{
  bool valid = true;
  if (String::utrim (getName ()) == "")
    return false;
  if (validateNumberOfShields() == false)
    return false;
  for (unsigned int i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
    {
      if (validateShieldImages(Shield::Color(i)) == false)
	return false;
    }
  for (unsigned int i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
    {
      if (validateTartanImages(Shield::Color(i)) == false)
	return false;
    }
  if (d_small_width == 0 || d_small_height == 0)
    return false;
  if (d_medium_width == 0 || d_medium_height == 0)
    return false;
  if (d_large_width == 0 || d_large_height == 0)
    return false;
  return valid;
}

bool Shieldset::validateNumberOfShields() const
{
  int players[MAX_PLAYERS + 1][3];
  memset(players, 0, sizeof(players));
  //need at least 3 complete player shields, one of which must be neutral.
  for (const_iterator it = begin(); it != end(); ++it)
    {
      for (Shield::const_iterator i = (*it)->begin(); i != (*it)->end(); ++i)
	{
	  int idx = 0;
	  switch ((*i)->getType())
	    {
	    case ShieldStyle::SMALL: idx = 0; break;
	    case ShieldStyle::MEDIUM: idx = 1; break;
	    case ShieldStyle::LARGE: idx = 2; break;
	    }
	  players[(*it)->getOwner()][idx]++;
	}
    }
  int count = 0;
  for (unsigned int i = 0; i < MAX_PLAYERS + 1; i++)
    {
      if (players[i][0] > 0 && players[i][1] > 0 && players[i][2] > 0)
	count++;
    }
  if (count <= 2)
    return false;
  if (players[MAX_PLAYERS][0] == 0 || players[MAX_PLAYERS][1] == 0 || players[MAX_PLAYERS][2] == 0)
    return false;
  return true;
}

bool Shieldset::validateShieldImages(Shield::Color c) const
{
  //if we have a shield, it should have all 3 sizes.
  int player[3];
  memset(player, 0, sizeof(player));
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getOwner() != guint32(c))
	continue;
      for (Shield::const_iterator i = (*it)->begin(); i != (*it)->end(); ++i)
	{
	  int idx = 0;
	  switch ((*i)->getType())
	    {
	    case ShieldStyle::SMALL: idx = 0; break;
	    case ShieldStyle::MEDIUM: idx = 1; break;
	    case ShieldStyle::LARGE: idx = 2; break;
	    }
	  if ((*i)->getMaskedImage()->getName ().empty() == false)
	    player[idx]++;
	}
    }
  int count = player[0] + player[1] + player[2];
  if (count <= 2)
    return false;
  return true;
}

bool Shieldset::validateTartanImages(Shield::Color c) const
{
  //if we have a shield, it should have all 3 portions of a tartan.
  int player[3];
  memset(player, 0, sizeof(player));
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getOwner() != guint32(c))
        continue;
      if ((*it)->getTartanMaskedImage(Tartan::LEFT)->getName ().empty () == false)
        player[0]++;
      if ((*it)->getTartanMaskedImage (Tartan::CENTER)->getName ().empty () == false)
        player[1]++;
      if ((*it)->getTartanMaskedImage (Tartan::RIGHT)->getName ().empty () == false)
        player[2]++;
    }
  int count = player[0] + player[1] + player[2];
  if (count <= 2)
    return false;
  return true;
}

void Shieldset::reload(bool &broken)
{
  broken = false;
  bool unsupported_version = false;
  ShieldsetLoader d(getConfigurationFile(), broken, unsupported_version);
  if (broken == false && d.shieldset && d.shieldset->validate())
    {
      //steal the values from d.shieldset and then don't delete it.
      uninstantiateImages();
      for (iterator it = begin(); it != end(); ++it)
        delete *it;
      Glib::ustring basename = getBaseName();
      *this = *d.shieldset;
      instantiateImages(true, broken);
      setBaseName(basename);
    }
}

guint32 Shieldset::countEmptyImageNames() const
{
  guint32 count = 0;
  for (Shieldset::const_iterator i = begin(); i != end(); ++i)
    {
      for (std::list<ShieldStyle*>::const_iterator j = (*i)->begin(); j != (*i)->end(); ++j)
        {
          if ((*j)->getMaskedImage()->getName().empty() == true)
            count++;
        }
    }
  return count;
}

bool Shieldset::upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version)
{
  return FileCompat::getInstance()->upgrade(filename, old_version, new_version,
                                            FileCompat::SHIELDSET, d_tag);
}

void Shieldset::support_backward_compatibility()
{
  FileCompat::getInstance()->support_type(FileCompat::SHIELDSET, 
                                          file_extension, d_tag, true);
  FileCompat::getInstance()->support_version
    (FileCompat::SHIELDSET, "0.2.1", "0.3.2",
     sigc::ptr_fun(&Shieldset::upgrade));
  FileCompat::getInstance()->support_version
    (FileCompat::SHIELDSET, "0.3.2", "0.3.3",
     sigc::ptr_fun(&Shieldset::upgrade));
}

Shieldset* Shieldset::copy(const Shieldset *shieldset)
{
  if (!shieldset)
    return NULL;
  return new Shieldset(*shieldset);
}

void Shieldset::setHeightsAndWidthsFromImages()
{
  setSmallHeightsAndWidthsFromImages();
  setMediumHeightsAndWidthsFromImages();
  setLargeHeightsAndWidthsFromImages();
}

void Shieldset::setHeightsAndWidthsFromImages(ShieldStyle *ss)
{
  ShieldStyle::Type t = ShieldStyle::Type(ss->getType ());
  switch (t)
    {
    case ShieldStyle::SMALL:
      return setSmallHeightsAndWidthsFromImages();
    case ShieldStyle::MEDIUM:
      return setMediumHeightsAndWidthsFromImages();
    case ShieldStyle::LARGE:
      return setLargeHeightsAndWidthsFromImages();
    }
}

void Shieldset::setSmallHeightsAndWidthsFromImages()
{
  d_small_width = 0;
  d_small_height = 0;
  std::map<Vector<int>, guint32> small_sizecounts;

  for (iterator it = begin(); it != end(); ++it)
    for (Shield::iterator i = (*it)->begin(); i != (*it)->end(); ++i)
      {
        PixMask *image = (*i)->getMaskedImage()->getImage ();
        if (image == NULL)
          continue;
        switch ((*i)->getType ())
          {
          case ShieldStyle::SMALL:
            small_sizecounts[image->get_unscaled_dim ()]++;
            break;
          }
      }

  guint32 maxcount = 0;
  for (auto i : small_sizecounts)
    {
      if (i.second > maxcount)
        {
          maxcount = i.second;
          d_small_width = i.first.x;
          d_small_height = i.first.y;
        }
    }
  return;
}

void Shieldset::setMediumHeightsAndWidthsFromImages()
{
  d_medium_width = 0;
  d_medium_height = 0;
  std::map<Vector<int>, guint32> medium_sizecounts;

  for (iterator it = begin(); it != end(); ++it)
    for (Shield::iterator i = (*it)->begin(); i != (*it)->end(); ++i)
      {
        PixMask *image = (*i)->getMaskedImage()->getImage ();
        if (image == NULL)
          continue;
        switch ((*i)->getType ())
          {
          case ShieldStyle::MEDIUM:
            medium_sizecounts[image->get_unscaled_dim ()]++;
            break;
          }
      }

  guint32 maxcount = 0;
  for (auto i : medium_sizecounts)
    {
      if (i.second > maxcount)
        {
          maxcount = i.second;
          d_medium_width = i.first.x;
          d_medium_height = i.first.y;
        }
    }
  return;
}

void Shieldset::setLargeHeightsAndWidthsFromImages()
{
  d_large_width = 0;
  d_large_height = 0;
  std::map<Vector<int>, guint32> large_sizecounts;

  for (iterator it = begin(); it != end(); ++it)
    for (Shield::iterator i = (*it)->begin(); i != (*it)->end(); ++i)
      {
        PixMask *image = (*i)->getMaskedImage()->getImage ();
        if (image == NULL)
          continue;
        switch ((*i)->getType ())
          {
          case ShieldStyle::LARGE:
            large_sizecounts[image->get_unscaled_dim ()]++;
            break;
          }
      }

  guint32 maxcount = 0;
  for (auto i : large_sizecounts)
    {
      if (i.second > maxcount)
        {
          maxcount = i.second;
          d_large_width = i.first.x;
          d_large_height = i.first.y;
        }
    }
  return;
}

TarFileMaskedImage *Shieldset::lookupTartanImage(guint32 color, Tartan::Type type)
{
  for (const_iterator it = begin(); it != end(); ++it)
    if ((*it)->getOwner() == color)
      return (*it)->getTartanMaskedImage (type);
  return NULL;
}

std::vector<TarFileMaskedImage*> Shieldset::getMaskedImages ()
{
  std::vector<TarFileMaskedImage*> i;
  for (auto s : *this)
    {
      for (auto ss : *s)
        i.push_back (ss->getMaskedImage ());
      for (guint32 k = Tartan::LEFT; k <= Tartan::RIGHT; k++)
        i.push_back (s->getTartanMaskedImage (Tartan::Type (k)));
    }
  return i;
}

void Shieldset::uninstantiateSameNamedImages (Glib::ustring name)
{
  TarFileMaskedImage::uninstantiate (name, getMaskedImages ());
}
        
bool Shieldset::isSmallHeightAndWidthSet()
{
  return d_small_width && d_small_height;
}

bool Shieldset::isMediumHeightAndWidthSet()
{
  return d_medium_width && d_medium_height;
}

bool Shieldset::isLargeHeightAndWidthSet()
{
  return d_large_width && d_large_height;
}

void Shieldset::instantiateImages(bool scale, bool &broken)
{
  uninstantiateImages ();

  broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return;

  for (iterator i = begin(); i != end(); ++i)
    {
      for (auto j : *(*i))
        {
          Vector<int> dim = Vector<int>(-1, -1);
          if (scale)
            {
              switch (j->getType())
                {
                case ShieldStyle::SMALL:
                  dim = Vector<int>(getSmallWidth(), getSmallHeight()); break;
                case ShieldStyle::MEDIUM:
                  dim = Vector<int>(getMediumWidth(), getMediumHeight()); break;
                case ShieldStyle::LARGE:
                  dim = Vector<int>(getLargeWidth(), getLargeHeight()); break;
                }
            }
          TarFileMaskedImage *mim = j->getMaskedImage ();
          if (mim->getName ().empty () == false)
            {
              mim->setTarFile (&t);
              broken = mim->load ();
              if (!broken)
                mim->instantiateImages (dim);
              else
                break;
            }
        }
      if (!broken)
        {
          for (guint32 k = Tartan::LEFT; k <= Tartan::RIGHT; k++)
            {
              TarFileMaskedImage *mim =
                (*i)->getTartanMaskedImage (Tartan::Type (k));
              if (mim->getName ().empty () == false)
                {
                  mim->setTarFile (&t);
                  broken = mim->load ();
                  if (!broken)
                    mim->instantiateImages ();
                  else
                    break;
                }
            }
        }
    }
  t.Close();
}

void Shieldset::uninstantiateImages()
{
  for (iterator i = begin(); i != end(); ++i)
    {
      for (auto j : *(*i))
        j->getMaskedImage ()->uninstantiateImages ();
      for (guint32 k = Tartan::LEFT; k <= Tartan::RIGHT; k++)
        (*i)->getTartanMaskedImage (Tartan::Type (k))->uninstantiateImages ();
    }
}
//End of file
