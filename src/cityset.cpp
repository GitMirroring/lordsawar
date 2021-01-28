// Copyright (C) 2008, 2010, 2011, 2014, 2015, 2020, 2021 Ben Asselstine
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

#include "cityset.h"
#include "File.h"
#include "xmlhelper.h"
#include "gui/image-helpers.h"
#include "city.h"
#include "ruin.h"
#include "temple.h"
#include "tarhelper.h"
#include "Configuration.h"
#include "file-compat.h"
#include "ucompose.hpp"
#include "TarFileImage.h"

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
  helper->getData(ts, "tilesize");
  setTileSize(ts);
  d_city->load_name (helper, "cities");
  d_rcity->load_name (helper, "razed_cities");
  d_port->load_name (helper, "port");
  d_sign->load_name (helper, "signpost");
  d_ruin->load_name (helper, "ruins");
  d_temple->load_name (helper, "temples");
  d_tower->load_name (helper, "towers");
  helper->getData(d_city_tile_width, "city_tile_width");
  helper->getData(d_temple_tile_width, "temple_tile_width");
  helper->getData(d_ruin_tile_width, "ruin_tile_width");
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
    CitysetLoader(Glib::ustring filename, bool &broken, bool &unsupported)
      : dir (File::get_dirname(filename)), file (File::get_basename(filename)),
      cityset (NULL), unsupported_version (false)
      {
	if (File::nameEndsWith(filename, Cityset::file_extension) == false)
	  filename += Cityset::file_extension;
        Tar_Helper t(filename, std::ios::in, broken);
        if (broken)
          return;
        Glib::ustring lwcfilename = 
          t.getFirstFile(Cityset::file_extension, broken);
        if (broken)
          return;
	XML_Helper helper(lwcfilename, std::ios::in);
	helper.registerTag(Cityset::d_tag, sigc::mem_fun((*this), &CitysetLoader::load));
	if (!helper.parseXML())
	  {
            unsupported = unsupported_version;
            std::cerr << String::ucompose(_("Error!  can't load cityset `%1'."), filename) << std::endl;
	    if (cityset != NULL)
	      delete cityset;
	    cityset = NULL;
	  }
        helper.close();
        File::erase(lwcfilename);
        t.Close();
      };
    bool load(Glib::ustring tag, XML_Helper* helper)
      {
	if (tag == Cityset::d_tag)
	  {
            if (helper->getVersion() == LORDSAWAR_CITYSET_VERSION)
              {
                cityset = new Cityset(helper, dir);
                cityset->setBaseName(file);
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
    Cityset *cityset;
    bool unsupported_version;
};

Cityset *Cityset::create(Glib::ustring file, bool &unsupported_version)
{
  bool broken = false;
  CitysetLoader d(file, broken, unsupported_version);
  if (broken)
    return NULL;
  return d.cityset;
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
  std::vector<Glib::ustring> extrafiles;
  return saveTar(tmpfile, tmpfile + ".tar", goodfilename, extrafiles);
}

bool Cityset::save(XML_Helper *helper) const
{
  bool retval = true;

  retval &= helper->openTag(d_tag);
  retval &= Set::save(helper);
  retval &= helper->saveData("tilesize", getUnscaledTileSize());
  retval &= helper->saveData("cities", d_city->getName ());
  retval &= helper->saveData("razed_cities", d_rcity->getName ());
  retval &= helper->saveData("port", d_port->getName ());
  retval &= helper->saveData("signpost", d_sign->getName ());
  retval &= helper->saveData("ruins", d_ruin->getName ());
  retval &= helper->saveData("temples", d_temple->getName ());
  retval &= helper->saveData("towers", d_tower->getName ());
  retval &= helper->saveData("city_tile_width", d_city_tile_width);
  retval &= helper->saveData("temple_tile_width", d_temple_tile_width);
  retval &= helper->saveData("ruin_tile_width", d_ruin_tile_width);
  retval &= helper->closeTag();
  return retval;
}

void Cityset::uninstantiateImages()
{
  for (auto i : getImages ())
    i->uninstantiateImages ();
}

void Cityset::instantiateImages(bool scale, bool &broken)
{
  debug("Loading images for cityset " << getName());
  uninstantiateImages();
  broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return;
  Vector<int> scale_dim = Vector<int>(-1,-1);
  if (scale)
    scale_dim = Vector<int>(getUnscaledTileSize (), getUnscaledTileSize ());

  d_port->load (&t);
  d_port->instantiateImages (scale_dim);

  d_sign->load (&t);
  d_sign->instantiateImages (scale_dim);

  d_temple->load (&t);
  if (scale)
    d_temple->instantiateImages (scale_dim * d_temple_tile_width);
  else
    d_temple->instantiateImages ();

  d_ruin->load (&t);
  if (scale)
    d_ruin->instantiateImages (scale_dim * d_ruin_tile_width);
  else
    d_ruin->instantiateImages ();

  d_tower->load (&t);
  d_tower->instantiateImages (scale_dim);

  d_city->load (&t);
  if (scale)
    d_city->instantiateImages (scale_dim * d_city_tile_width);
  else
    d_city->instantiateImages ();

  d_rcity->load (&t);
  if (scale)
    d_rcity->instantiateImages (scale_dim * d_city_tile_width);
  else
    d_rcity->instantiateImages ();

  t.Close();
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

void Cityset::reload(bool &broken)
{
  broken = false;
  bool unsupported_version = false;
  CitysetLoader d(getConfigurationFile(), broken, unsupported_version);
  if (!broken && d.cityset && d.cityset->validate())
    {
      //steal the values from d.cityset and then don't delete it.
      uninstantiateImages();
      Glib::ustring basename = getBaseName();
      *this = *d.cityset;
      instantiateImages(true, broken);
      setBaseName(basename);
    }
}

bool Cityset::calculate_preferred_tile_size(guint32 &ts) const
{
  guint32 tilesize = 0;
  std::map<guint32, guint32> sizecounts;

  if (d_city->getImage ())
    sizecounts[d_city->getImage ()->get_unscaled_width() / d_city_tile_width]++;
  if (d_rcity->getImage ())
    sizecounts[d_rcity->getImage ()->get_unscaled_width() / d_city_tile_width]++;
  if (d_port->getImage ())
    sizecounts[d_port->getImage ()->get_unscaled_width()]++;
  if (d_sign->getImage ())
    sizecounts[d_sign->getImage ()->get_unscaled_width()]++;
  if (d_ruin->getImage ())
    sizecounts[d_ruin->getImage ()->get_unscaled_width() / d_ruin_tile_width]++;
  if (d_temple->getImage ())
    sizecounts[d_temple->getImage ()->get_unscaled_width() / d_temple_tile_width]++;
  if (d_tower->getImage ())
    sizecounts[d_tower->getImage ()->get_unscaled_width()]++;

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
  return FileCompat::getInstance()->upgrade(filename, old_version, new_version,
                                            FileCompat::CITYSET, d_tag);
}

void Cityset::support_backward_compatibility()
{
  FileCompat::getInstance()->support_type (FileCompat::CITYSET, file_extension, 
                                           d_tag, true);
  FileCompat::getInstance()->support_version
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
  guint32 ts = c->getUnscaledTileSize ();
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
// End of file
