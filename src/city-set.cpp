//  Copyright (C) 2008, 2010, 2011, 2014, 2015, 2020, 2021 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>

#include "city-set.h"
#include "file.h"
#include "xml-helper.h"
#include "image-helpers.h"
#include "city.h"
#include "ruin.h"
#include "temple.h"
#include "tar-helper.h"
#include "configuration.h"
#include "file-compat.h"
#include "ucompose.hpp"
#include "tar-file-image.h"

Glib::ustring Cityset::d_tag = "cityset";
Glib::ustring Cityset::file_extension = CITYSET_EXT;

#include <iostream>
//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

#define DEFAULT_CITY_TILE_SIZE 40
Cityset::Cityset(guint32 id, Glib::ustring name)
 : Set(CITYSET_EXT, id, name, DEFAULT_CITY_TILE_SIZE)
{
  d_port = new TarFileImage (1, PixMask::DIMENSION_SAME_HEIGHT_AND_WIDTH);
  d_sign = new TarFileImage (1, PixMask::DIMENSION_SAME_HEIGHT_AND_WIDTH);
  d_temple = new TarFileImage (TEMPLE_TYPES,
                               PixMask::DIMENSION_WIDTH_IS_MULTIPLE_OF_HEIGHT);
  d_ruin = new TarFileImage (RUIN_TYPES,
                             PixMask::DIMENSION_WIDTH_IS_MULTIPLE_OF_HEIGHT);
  d_tower = new TarFileImage (MAX_PLAYERS,
                              PixMask::DIMENSION_WIDTH_IS_MULTIPLE_OF_HEIGHT);
  d_city = new TarFileImage (MAX_PLAYERS + 1,
                              PixMask::DIMENSION_WIDTH_IS_MULTIPLE_OF_HEIGHT);
  d_rcity = new TarFileImage (MAX_PLAYERS,
                              PixMask::DIMENSION_WIDTH_IS_MULTIPLE_OF_HEIGHT);

  d_city_tile_width = 2;
  d_temple_tile_width = 1;
  d_ruin_tile_width = 1;
}

Cityset::Cityset(const Cityset& c)
 : sigc::trackable(c), Set(c)
{
  d_port = new TarFileImage (*c.d_port);
  d_sign = new TarFileImage (*c.d_sign);
  d_temple = new TarFileImage (*c.d_temple);
  d_ruin = new TarFileImage (*c.d_ruin);
  d_tower = new TarFileImage (*c.d_tower);
  d_city = new TarFileImage (*c.d_city);
  d_rcity = new TarFileImage (*c.d_rcity);

  d_city_tile_width = c.d_city_tile_width;
  d_temple_tile_width = c.d_temple_tile_width;
  d_ruin_tile_width = c.d_ruin_tile_width;
}

Cityset::Cityset(XML_Helper *helper, Glib::ustring directory)
 : Set(CITYSET_EXT, helper, directory)
{
  d_port = new TarFileImage (1, PixMask::DIMENSION_SAME_HEIGHT_AND_WIDTH);
  d_sign = new TarFileImage (1, PixMask::DIMENSION_SAME_HEIGHT_AND_WIDTH);
  d_temple = new TarFileImage (TEMPLE_TYPES,
                              PixMask::DIMENSION_WIDTH_IS_MULTIPLE_OF_HEIGHT);
  d_ruin = new TarFileImage (RUIN_TYPES,
                              PixMask::DIMENSION_WIDTH_IS_MULTIPLE_OF_HEIGHT);
  d_tower = new TarFileImage (MAX_PLAYERS,
                              PixMask::DIMENSION_WIDTH_IS_MULTIPLE_OF_HEIGHT);
  d_city = new TarFileImage (MAX_PLAYERS + 1,
                              PixMask::DIMENSION_WIDTH_IS_MULTIPLE_OF_HEIGHT);
  d_rcity = new TarFileImage (MAX_PLAYERS,
                              PixMask::DIMENSION_WIDTH_IS_MULTIPLE_OF_HEIGHT);
  guint32 ts;
  helper->get(ts, "tilesize");
  setTileSize(ts);
  d_city->load_name (helper, "cities");
  d_rcity->load_name (helper, "razed_cities");
  d_port->load_name (helper, "port");
  d_sign->load_name (helper, "signpost");
  d_ruin->load_name (helper, "ruins");
  d_temple->load_name (helper, "temples");
  d_tower->load_name (helper, "towers");
  helper->get(d_city_tile_width, "city_tile_width");
  helper->get(d_temple_tile_width, "temple_tile_width");
  helper->get(d_ruin_tile_width, "ruin_tile_width");
}

Cityset::~Cityset()
{
  uninstantiateImages();
  delete d_port;
  delete d_sign;
  delete d_temple;
  delete d_ruin;
  delete d_tower;
  delete d_city;
  delete d_rcity;
  clean_tmp_dir();
}

//! Helper class for making a new Cityset object from a cityset file.
class CitysetLoader
{
public:
    CitysetLoader (Glib::ustring f)
      : filename (f), dir (File::get_dirname (filename)),
      file (File::get_basename (filename)), bad_version (""),
      found_top_tag (false)
      {
        if (File::nameEndsWith (filename, Cityset::file_extension) == false)
          filename += Cityset::file_extension;
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
        Glib::ustring lwcfilename = 
          t.getFirstFile (Cityset::file_extension, broken);
        if (lwcfilename.empty () == true)
          {
            Glib::ustring err =
              String::ucompose (_("City set file `%1' lacks a %2 file"),
                                filename, Cityset::file_extension);
            signal_finished.emit (NULL, true, false, err);
            return false;
          }
        if (broken)
          {
            Glib::ustring err =
              String::ucompose
              (_("Could not extract first file from city set file `%1'"),
               filename);
            signal_finished.emit (NULL, true, false, err);
            return false;
          }

        XML_Helper helper (lwcfilename, std::ios::in);
        helper.register_tag (Cityset::d_tag,
                            sigc::mem_fun(*this, &CitysetLoader::load));
        bool retval = true;
        if (!helper.parse_XML ())
          {
            if (bad_version != "")
              {
                Glib::ustring err =
                  String::ucompose (_("Expected version %1 but got %2"),
                                    LORDSAWAR_CITYSET_VERSION, bad_version);
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
                                    Cityset::d_tag);
                signal_finished.emit (NULL, true, false, err);
              }
            else
              signal_finished.emit (cityset, false, false, "");
          }
        helper.close ();
        File::erase (lwcfilename);
        t.Close ();
        return retval;
      }

    bool load (Glib::ustring tag, XML_Helper* helper)
      {
	if (tag == Cityset::d_tag)
	  {
            found_top_tag = true;
            if (helper->get_version () == LORDSAWAR_CITYSET_VERSION)
              {
                cityset = new Cityset (helper, dir);
                cityset->setBaseName (file);
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
    sigc::signal<void(Cityset*, bool, bool, Glib::ustring)> signal_finished;
    Cityset *cityset;
};

void Cityset::create(Glib::ustring filename, sigc::slot<void(Cityset*,bool,bool,Glib::ustring)> finished)
{
  CitysetLoader d(filename);
  d.signal_finished.connect
    ([finished](Cityset *cityset, bool broken, bool unsupported_version, Glib::ustring err)
     {
       finished (cityset, broken, unsupported_version, err);
     });
  d.parse ();
}

bool Cityset::save(Glib::ustring filename, Glib::ustring ext) const
{
  bool broken = false;
  Glib::ustring goodfilename = File::add_ext_if_necessary(filename, ext);
  Glib::ustring tmpfile = File::get_tmp_file();
  XML_Helper helper(tmpfile, std::ios::out);
  helper.begin(LORDSAWAR_CITYSET_VERSION);
  broken = !save(&helper);
  helper.close();
  if (broken == true)
    return false;
  std::vector<std::string> extrafiles;
  return saveTar(tmpfile, tmpfile + ".tar", goodfilename, extrafiles);
}

bool Cityset::save(XML_Helper *helper) const
{
  bool retval = true;

  retval &= helper->open_tag(d_tag);
  retval &= Set::save(helper);
  retval &= helper->save("tilesize", getTileSize());
  retval &= helper->save("cities", d_city->getName ());
  retval &= helper->save("razed_cities", d_rcity->getName ());
  retval &= helper->save("port", d_port->getName ());
  retval &= helper->save("signpost", d_sign->getName ());
  retval &= helper->save("ruins", d_ruin->getName ());
  retval &= helper->save("temples", d_temple->getName ());
  retval &= helper->save("towers", d_tower->getName ());
  retval &= helper->save("city_tile_width", d_city_tile_width);
  retval &= helper->save("temple_tile_width", d_temple_tile_width);
  retval &= helper->save("ruin_tile_width", d_ruin_tile_width);
  retval &= helper->close_tag();
  return retval;
}

void Cityset::uninstantiateImages()
{
  for (auto i : getImages ())
    i->uninstantiateImages ();
}

void Cityset::instantiateImages(bool &broken)
{
  debug("Loading images for cityset " << getName ());
  uninstantiateImages ();
  broken = false;
  Tar_Helper t (getConfigurationFile (), std::ios::in, broken);
  if (broken)
    return;

  if (d_port->load (&t))
    {
      broken = true;
      return;
    }
  d_port->instantiateImages ();

  if (d_sign->load (&t))
    {
      broken = true;
      return;
    }
  d_sign->instantiateImages ();

  if (d_temple->load (&t))
    {
      broken = true;
      return;
    }
  d_temple->instantiateImages ();

  if (d_ruin->load (&t))
    {
      broken = true;
      return;
    }
  d_ruin->instantiateImages ();

  if (d_tower->load (&t))
    {
      broken = true;
      return;
    }
  d_tower->instantiateImages ();

  if (d_city->load (&t))
    {
      broken = true;
      return;
    }
  d_city->instantiateImages ();

  if (d_rcity->load (&t))
    {
      broken = true;
      return;
    }
  d_rcity->instantiateImages ();

  t.Close ();
}

bool Cityset::validate()
{
  bool valid = true;
  if (String::utrim (getName ()) == "")
    return false;
  if (d_city->getName ().empty () == true)
    return false;
  if (d_rcity->getName ().empty () == true)
    return false;
  if (d_port->getName ().empty() == true)
    return false;
  if (d_sign->getName ().empty() == true)
    return false;
  if (d_ruin->getName ().empty () == true)
    return false;
  if (d_temple->getName ().empty () == true)
    return false;
  if (d_tower->getName ().empty () == true)
    return false;
  if (validateCityTileWidth() == false)
    return false;
  if (validateRuinTileWidth() == false)
    return false;
  if (validateTempleTileWidth() == false)
    return false;
  return valid;
}

bool Cityset::validateCityTileWidth()
{
  if (getCityTileWidth() <= 0)
    return false;
  return true; 
}

bool Cityset::validateRuinTileWidth()
{
  if (getRuinTileWidth() <= 0)
    return false;
  return true; 
}

bool Cityset::validateTempleTileWidth()
{
  if (getTempleTileWidth() <= 0)
    return false;
  return true; 
}

void Cityset::reload()
{
  CitysetLoader d(getConfigurationFile());
  d.signal_finished.connect
    ([this](Cityset *cityset, bool broken, bool unsupported_version,
            Glib::ustring)
     {
       if (!broken && !unsupported_version && cityset)
         {
           if (cityset->validate ())
             {
               //steal the values from d.cityset and then don't delete it.
               uninstantiateImages();
               Glib::ustring basename = getBaseName();
               *this = *cityset;
               instantiateImages(broken);
               setBaseName(basename);
             }
         }
     });

  d.parse ();
}

bool Cityset::calculate_preferred_tile_size(guint32 &ts) const
{
  guint32 tilesize = 0;
  std::map<guint32, guint32> sizecounts;

  if (d_city->getImage ())
    sizecounts[d_city->getImage ()->get_width() / d_city_tile_width]++;
  if (d_rcity->getImage ())
    sizecounts[d_rcity->getImage ()->get_width() / d_city_tile_width]++;
  if (d_port->getImage ())
    sizecounts[d_port->getImage ()->get_width()]++;
  if (d_sign->getImage ())
    sizecounts[d_sign->getImage ()->get_width()]++;
  if (d_ruin->getImage ())
    sizecounts[d_ruin->getImage ()->get_width() / d_ruin_tile_width]++;
  if (d_temple->getImage ())
    sizecounts[d_temple->getImage ()->get_width() / d_temple_tile_width]++;
  if (d_tower->getImage ())
    sizecounts[d_tower->getImage ()->get_width()]++;

  guint32 maxcount = 0;
  for (std::map<guint32, guint32>::iterator it = sizecounts.begin(); 
       it != sizecounts.end(); ++it)
    {
      if ((*it).second > maxcount)
        {
          maxcount = (*it).second;
          tilesize = (*it).first;
        }
    }
  bool ret = true;
  if (tilesize == 0)
    {
      ts = DEFAULT_CITY_TILE_SIZE;
      ret = false;
    }
  else
    ts = tilesize;
  return ret;
}

bool Cityset::upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version)
{
  return FileCompat::instance()->upgrade(filename, old_version, new_version,
                                            FileCompat::CITYSET, d_tag);
}

void Cityset::support_backward_compatibility()
{
  FileCompat::instance()->support_type (FileCompat::CITYSET, file_extension, 
                                           d_tag, true);
  FileCompat::instance()->support_version
    (FileCompat::CITYSET, "0.2.0", LORDSAWAR_CITYSET_VERSION,
     sigc::ptr_fun(&Cityset::upgrade));
}

Cityset* Cityset::copy(const Cityset *cityset)
{
  if (!cityset)
    return NULL;
  return new Cityset(*cityset);
}

guint32 Cityset::get_default_tile_size ()
{
  Cityset *c = new Cityset (1, "");
  guint32 ts = c->getTileSize ();
  delete c;
  return ts;
}

std::vector<TarFileImage*> Cityset::getImages ()
{
  std::vector<TarFileImage*> i;
  i.push_back (d_city);
  i.push_back (d_rcity);
  i.push_back (d_port);
  i.push_back (d_sign);
  i.push_back (d_ruin);
  i.push_back (d_temple);
  i.push_back (d_tower);
  return i;
}

void Cityset::uninstantiateSameNamedImages (Glib::ustring name)
{
  TarFileImage::uninstantiate (name, getImages ());
}

Cityset& Cityset::operator=(const Cityset& other)
{
  if (this != &other)
    {
      Set::operator=(other);
      if (d_port)
        delete d_port;
      d_port = new TarFileImage (*other.d_port);
      if (d_sign)
        delete d_sign;
      d_sign = new TarFileImage (*other.d_sign);
      if (d_temple)
        delete d_temple;
      d_temple = new TarFileImage (*other.d_temple);
      if (d_ruin)
        delete d_ruin;
      d_ruin = new TarFileImage (*other.d_ruin);
      if (d_tower)
        delete d_tower;
      d_tower = new TarFileImage (*other.d_tower);
      if (d_city)
        delete d_city;
      d_city = new TarFileImage (*other.d_city);
      if (d_rcity)
        delete d_rcity;
      d_rcity = new TarFileImage (*other.d_rcity);

      d_city_tile_width = other.d_city_tile_width;
      d_temple_tile_width = other.d_temple_tile_width;
      d_ruin_tile_width = other.d_ruin_tile_width;
    }
  return *this;
}

bool Cityset::get_images_instantiated ()
{
  for (auto i : getImages ())
    if (i->getBackingImage () != NULL)
      return true;

  return false;
}
