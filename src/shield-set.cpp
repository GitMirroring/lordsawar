//  Copyright (C) 2008, 2009, 2010, 2011, 2014, 2015, 2020, 2021,
//  2026 Ben Asselstine
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

#include <iostream>
#include "rectangle.h"
#include <sigc++/functors/mem_fun.h>

#include <string.h>
#include "shield-set.h"
#include "shield-style.h"
#include "file.h"
#include "configuration.h"
#include "tar-helper.h"
#include "file-compat.h"
#include "ucompose.hpp"
#include "xml-helper.h"
#include "tar-file-masked-image.h"

Glib::ustring Shieldset::d_tag = "shieldset";
Glib::ustring Shieldset::file_extension = SHIELDSET_EXT;

#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
//#define debug(x)

Shieldset::Shieldset(guint32 id, Glib::ustring name)
 : Set(SHIELDSET_EXT, id, name, 0)
{
}

Shieldset::Shieldset(const Shieldset& s)
 : std::list<Shield*>(), sigc::trackable(s), Set(s)
{
  for (const_iterator it = s.begin(); it != s.end(); ++it)
    push_back(new Shield(*(*it)));
}

Shieldset::Shieldset(XML_Helper *helper, Glib::ustring directory)
 : Set(SHIELDSET_EXT, helper, directory)
{
  setTileSize (0);
  helper->register_tag(Shield::d_tag, 
		      sigc::mem_fun((*this), &Shieldset::loadShield));
  helper->register_tag(ShieldStyle::d_tag, sigc::mem_fun((*this), 
							&Shieldset::loadShield));
  helper->register_tag(Tartan::d_tartan_tag, sigc::mem_fun((*this),
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
    ShieldsetLoader (Glib::ustring f)
      : filename (f), dir (File::get_dirname (filename)),
      file (File::get_basename (filename)), bad_version (""),
      found_top_tag (false)
      {
        if (File::nameEndsWith (filename, Shieldset::file_extension) == false)
          filename += Shieldset::file_extension;
      }

    bool parse ()
      {
        bool broken = false;
        Tar_Helper t (filename, std::ios::in, broken);
        if (broken)
          {
            Glib::ustring err;
            if (File::exists (filename) && File::is_readonly (filename))
              err = String::ucompose (_("Couldn't open %1 for reading"),
                                      filename);
            else
              err =
                String::ucompose
                (_("Couldn't scan archive in %1, not a valid file"), filename);

            signal_finished.emit (NULL, true, false, err);
            return false;
          }
        Glib::ustring lwsfilename = 
          t.getFirstFile (Shieldset::file_extension, broken);
        if (lwsfilename.empty () == true)
          {
            Glib::ustring err =
              String::ucompose (_("Shield set file `%1' lacks a %2 file"),
                                filename, Shieldset::file_extension);

            signal_finished.emit (NULL, true, false, err);
            return false;
          }
        if (broken)
          {
            Glib::ustring err =
              String::ucompose
              (_("Could not extract first file from shield set file `%1'"),
               filename);

            signal_finished.emit (NULL, true, false, err);
            return false;
          }

        XML_Helper helper (lwsfilename, std::ios::in);
        helper.register_tag (Shieldset::d_tag,
                            sigc::mem_fun(*this, &ShieldsetLoader::load));
        bool retval = true;
        if (!helper.parse_XML ())
          {
            if (bad_version != "")
              {
                Glib::ustring err =
                  String::ucompose (_("Expected version %1 but got %2"),
                                    LORDSAWAR_SHIELDSET_VERSION, bad_version);

                signal_finished.emit (NULL, false, true, err);
              }
            else
              signal_finished.emit (NULL, true, false,
                                    _("Unknown parsing error"));
            retval = false;
          }
        else
          {
            if (!found_top_tag)
              {
                Glib::ustring err =
                  String::ucompose (_("Couldn't find <%1> tag"),
                                    Shieldset::d_tag);
                signal_finished.emit (NULL, true, false, err);
              }
            else
              signal_finished.emit (shieldset, false, false, "");
          }
        helper.close ();
        File::erase (lwsfilename);
        t.Close ();
        return retval;
      }

    bool load (Glib::ustring tag, XML_Helper* helper)
      {
	if (tag == Shieldset::d_tag)
	  {
            if (helper->get_version () == LORDSAWAR_SHIELDSET_VERSION)
              {
                found_top_tag = true;
                shieldset = new Shieldset (helper, dir);
                shieldset->setBaseName (file);
                return true;
              }
            else
              {
                bad_version = helper->get_version ();
                return false;
              }
	  }
	return false;
      };
    Glib::ustring filename;
    Glib::ustring dir;
    Glib::ustring file;
    Glib::ustring bad_version;
    bool found_top_tag;
    sigc::signal<void(Shieldset*, bool, bool, Glib::ustring)> signal_finished;
    Shieldset *shieldset;
};

void Shieldset::create(Glib::ustring filename, sigc::slot<void(Shieldset*,bool,bool,Glib::ustring)> finished)
{
  ShieldsetLoader d(filename);
  d.signal_finished.connect
    ([finished](Shieldset *shieldset, bool broken, bool unsupported_version,
                Glib::ustring err)
     {
       finished (shieldset, broken, unsupported_version, err);
     });
  d.parse ();
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
  std::vector<std::string> extrafiles;
  return saveTar(tmpfile, tmpfile + ".tar", goodfilename, extrafiles);
}

bool Shieldset::save(XML_Helper *helper) const
{
  bool retval = true;

  retval &= helper->open_tag(d_tag);
  retval &= Set::save(helper);
  for (const_iterator it = begin(); it != end(); ++it)
    retval &= (*it)->save(helper);
  retval &= helper->close_tag();
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

void Shieldset::reload()
{
  ShieldsetLoader d(getConfigurationFile());
  d.signal_finished.connect
    ([this](Shieldset *shieldset, bool broken, bool unsupported_version,
            Glib::ustring)
     {
       if (!broken && !unsupported_version && shieldset)
         {
           if (shieldset->validate ())
             {
               //steal the values from d.shieldset and then don't delete it.
               uninstantiateImages();
               for (iterator it = begin(); it != end(); ++it)
                 delete *it;
               clear ();
               Glib::ustring basename = getBaseName();
               *this = *shieldset;
               instantiateImages(broken);
               setBaseName(basename);
             }
         }
     });

  d.parse ();
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
  return FileCompat::instance()->upgrade(filename, old_version, new_version,
                                            FileCompat::SHIELDSET, d_tag);
}

void Shieldset::support_backward_compatibility()
{
  FileCompat::instance()->support_type(FileCompat::SHIELDSET, 
                                          file_extension, d_tag, true);
  FileCompat::instance()->support_version
    (FileCompat::SHIELDSET, "0.2.1", "0.3.2",
     sigc::ptr_fun(&Shieldset::upgrade));
  FileCompat::instance()->support_version
    (FileCompat::SHIELDSET, "0.3.2", "0.3.3",
     sigc::ptr_fun(&Shieldset::upgrade));
  FileCompat::instance()->support_version
    (FileCompat::SHIELDSET, "0.3.3", "0.4.0",
     sigc::ptr_fun(&Shieldset::upgrade));
}

Shieldset* Shieldset::copy(const Shieldset *shieldset)
{
  if (!shieldset)
    return NULL;
  return new Shieldset(*shieldset);
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

void Shieldset::instantiateImages(bool &broken)
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
          TarFileMaskedImage *mim = j->getMaskedImage ();
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

Shieldset& Shieldset::operator= (const Shieldset& other)
{
  if (this != &other)
    {
      uninstantiateImages ();
      for (iterator it = begin (); it != end (); ++it)
        delete *it;
      clear ();
      clean_tmp_dir ();
    }
  Set::operator=(other);
  for (const_iterator it = other.begin (); it != other.end (); ++it)
    push_back (new Shield (*(*it)));
  return *this;
}

void Shieldset::populate_with_defaults ()
{
  for (unsigned int i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
    {
      auto colors = Shield::get_default_colors (i);
      if (i == Shield::NEUTRAL)
        colors = Shield::get_default_colors_for_neutral ();
      Shield *shield = new Shield (Shield::Color (i), colors);
      if (shield)
        {
          shield->push_back (new ShieldStyle (ShieldStyle::SMALL));
          shield->push_back (new ShieldStyle (ShieldStyle::MEDIUM));
          shield->push_back (new ShieldStyle (ShieldStyle::LARGE));
          push_back (shield);
        }
    }
}

bool Shieldset::get_images_instantiated ()
{
  for (auto i : getMaskedImages ())
    if (i->getBackingImage () != NULL)
      return true;

  return false;
}
