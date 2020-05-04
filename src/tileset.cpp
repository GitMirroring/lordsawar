// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2014, 2015, 2020 Ben Asselstine
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
#include <string.h>
#include <iostream>

#include "tileset.h"

#include "defs.h"
#include "File.h"
#include "SmallTile.h"
#include "xmlhelper.h"
#include "gui/image-helpers.h"
#include "ImageCache.h"
#include "tilesetlist.h"
#include "tarhelper.h"
#include "Configuration.h"
#include "file-compat.h"
#include "ucompose.hpp"
#include "TarFileMaskedImage.h"
#include "TarFileImage.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

#define DEFAULT_TILE_SIZE 40

Glib::ustring Tileset::d_tag = "tileset";
Glib::ustring Tileset::d_road_smallmap_tag = "road_smallmap";
Glib::ustring Tileset::d_ruin_smallmap_tag = "ruin_smallmap";
Glib::ustring Tileset::d_temple_smallmap_tag = "temple_smallmap";
Glib::ustring Tileset::file_extension = TILESET_EXT;

Tileset::Tileset(guint32 id, Glib::ustring name)
	: Set(TILESET_EXT, id, name, DEFAULT_TILE_SIZE)
{
  d_selector[0] = new TarFileMaskedImage ();
  d_selector[1] = new TarFileMaskedImage ();
  d_flag = new TarFileMaskedImage ();
  d_fog = new TarFileImage (FOG_TYPES);
  d_road = new TarFileImage (ROAD_TYPES);
  d_stone = new TarFileImage (STONE_TYPES);
  d_bridge = new TarFileImage (BRIDGE_TYPES);
  d_explosion = new TarFileImage (1);

  d_road_color.set_rgba(164.0/255.0,84.0/255.0,0);
  d_ruin_color.set_rgba(1,1,1);
  d_temple_color.set_rgba(1,1,1);

  d_all_movebonus = new TarFileImage (1);
  d_water_movebonus = new TarFileImage (1);
  d_forest_movebonus = new TarFileImage (1);
  d_hills_movebonus = new TarFileImage (1);
  d_mountains_movebonus = new TarFileImage (1);
  d_swamp_movebonus = new TarFileImage (1);
}

Tileset::Tileset (const Tileset& t)
  : sigc::trackable(t), std::vector<Tile*>(), Set(t)
{
  d_selector[0] = new TarFileMaskedImage (*t.d_selector[0]);
  d_selector[1] = new TarFileMaskedImage (*t.d_selector[1]);
  d_flag = new TarFileMaskedImage (*t.d_flag);
  d_fog = new TarFileImage (*t.d_fog);
  d_road = new TarFileImage (*t.d_road);
  d_stone = new TarFileImage (*t.d_stone);
  d_bridge = new TarFileImage (*t.d_bridge);
  d_explosion = new TarFileImage (*t.d_explosion);
  d_road_color = t.d_road_color;
  d_ruin_color = t.d_ruin_color;
  d_temple_color = t.d_temple_color;

  for (Tileset::const_iterator i = t.begin(); i != t.end(); ++i)
    push_back(new Tile(*(*i)));

  for (Tileset::const_iterator i = begin(); i != end(); ++i)
    {
      for (std::list<TileStyleSet*>::const_iterator j = (*i)->begin(); j != (*i)->end(); j++)
	{
	  for (std::vector<TileStyle*>::const_iterator k = (*j)->begin(); k != (*j)->end(); k++)
            {
              d_tilestyles[(*k)->getId()] = *k;
            }
        }
    }

  d_all_movebonus = new TarFileImage (*t.d_all_movebonus);
  d_water_movebonus = new TarFileImage (*t.d_water_movebonus);
  d_forest_movebonus = new TarFileImage (*t.d_forest_movebonus);
  d_hills_movebonus = new TarFileImage (*t.d_hills_movebonus);
  d_mountains_movebonus = new TarFileImage (*t.d_mountains_movebonus);
  d_swamp_movebonus = new TarFileImage (*t.d_swamp_movebonus);
}

Tileset::Tileset(XML_Helper *helper, Glib::ustring directory)
	:Set(TILESET_EXT, helper)
{
  d_selector[0] = new TarFileMaskedImage ();
  d_selector[1] = new TarFileMaskedImage ();
  d_flag = new TarFileMaskedImage ();
  d_fog = new TarFileImage (FOG_TYPES);
  d_road = new TarFileImage (ROAD_TYPES);
  d_stone = new TarFileImage (STONE_TYPES);
  d_bridge = new TarFileImage (BRIDGE_TYPES);
  d_explosion = new TarFileImage (1);
  d_all_movebonus = new TarFileImage (1);
  d_water_movebonus = new TarFileImage (1);
  d_forest_movebonus = new TarFileImage (1);
  d_hills_movebonus = new TarFileImage (1);
  d_mountains_movebonus = new TarFileImage (1);
  d_swamp_movebonus = new TarFileImage (1);
  setDirectory(directory);
  guint32 ts;
  helper->getData(ts, "tilesize");
  setTileSize(ts);
  d_selector[1]->load_name (helper, "large_selector");
  d_selector[0]->load_name (helper, "small_selector");
  d_explosion->load_name (helper, "explosion");
  d_road->load_name (helper, "roads");
  d_stone->load_name (helper, "standing_stones");
  d_bridge->load_name (helper, "bridges");
  d_fog->load_name (helper, "fog");
  d_flag->load_name (helper, "flags");
  d_all_movebonus->load_name (helper, "movebonus_all");
  d_water_movebonus->load_name (helper, "movebonus_water");
  d_forest_movebonus->load_name (helper, "movebonus_forest");
  d_hills_movebonus->load_name (helper, "movebonus_hills");
  d_mountains_movebonus->load_name (helper, "movebonus_mountains");
  d_swamp_movebonus->load_name (helper, "movebonus_swamp");

  helper->registerTag(Tile::d_tag, sigc::mem_fun((*this), &Tileset::loadTile));
  helper->registerTag(Tileset::d_road_smallmap_tag, sigc::mem_fun((*this), &Tileset::loadTile));
  helper->registerTag(Tileset::d_ruin_smallmap_tag, sigc::mem_fun((*this), &Tileset::loadTile));
  helper->registerTag(Tileset::d_temple_smallmap_tag, sigc::mem_fun((*this), &Tileset::loadTile));
  helper->registerTag(SmallTile::d_tag, sigc::mem_fun((*this), &Tileset::loadTile));
  helper->registerTag(TileStyle::d_tag, sigc::mem_fun((*this), &Tileset::loadTile));
  helper->registerTag(TileStyleSet::d_tag, sigc::mem_fun((*this), &Tileset::loadTile));
}

Tileset::~Tileset()
{
  uninstantiateImages();
  for (unsigned int i=0; i < size(); i++)
    delete (*this)[i];
  delete d_selector[0];
  delete d_selector[1];
  delete d_flag;
  delete d_explosion;
  delete d_road;
  delete d_stone;
  delete d_bridge;
  delete d_fog;
  delete d_all_movebonus;
  delete d_water_movebonus;
  delete d_forest_movebonus;
  delete d_hills_movebonus;
  delete d_mountains_movebonus;
  delete d_swamp_movebonus;
  clear();
  clean_tmp_dir();
}

int Tileset::getIndex(Tile::Type type) const
{
  for (guint32 i = 0; i < size(); i++)
    if (type == (*this)[i]->getType())
      return i;

  // catch errors?
  return -1;
}

int Tileset::lookupIndexByType(Tile::Type type) const
{
  TileTypeIndexMap::const_iterator it = d_tiletypes.find(type);
  if (it == d_tiletypes.end())
    return -1;
  else
    return (*it).second;
}

bool Tileset::loadTile(Glib::ustring tag, XML_Helper* helper)
{
  debug("loadTile()")

    if (tag == Tile::d_tag)
      {
	// create a new tile with the information we got
	Tile* tile = new Tile(helper);
        d_tiletypes[tile->getType()] = this->size();
	this->push_back(tile);

	return true;
      }

  if (tag == Tileset::d_road_smallmap_tag)
    {
      helper->getData(d_road_color, "color");
      return true;
    }

  if (tag == Tileset::d_ruin_smallmap_tag)
    {
      helper->getData(d_ruin_color, "color");
      return true;
    }

  if (tag == Tileset::d_temple_smallmap_tag)
    {
      helper->getData(d_temple_color, "color");
      return true;
    }

  if (tag == SmallTile::d_tag)
    {
      Tile *tile = this->back();
      SmallTile* smalltile = new SmallTile(helper);
      tile->setSmallTile(smalltile);
      return true;
    }

  if (tag == TileStyle::d_tag)
    {
      Tile *tile = this->back();
      TileStyleSet *tilestyleset = tile->back();
      // create a new tile style with the information we got
      // put it on the latest tilestyleset
      TileStyle* tilestyle = new TileStyle(helper);
      tilestyleset->push_back(tilestyle);
      d_tilestyles[tilestyle->getId()] = tilestyle;

      return true;
    }

  if (tag == TileStyleSet::d_tag)
    {
      Tile *tile = this->back();
      // create a new tile style set with the information we got
      // put it on the latest tile
      TileStyleSet* tilestyleset = new TileStyleSet(helper);
      tile->push_back(tilestyleset);
      return true;
    }

  return false;
}

TileStyle *Tileset::getRandomTileStyle(guint32 index, TileStyle::Type style) const
{
  Tile *tile = (*this)[index];
  if (tile)
    return tile->getRandomTileStyle (style);
  else
    return NULL;
}

bool Tileset::save(XML_Helper *helper) const
{
  bool retval = true;

  retval &= helper->openTag(d_tag);
  retval &= Set::save(helper);
  retval &= helper->saveData("tilesize", getUnscaledTileSize());
  retval &= helper->saveData("large_selector", d_selector[1]->getName ());
  retval &= helper->saveData("small_selector", d_selector[0]->getName ());
  retval &= helper->saveData("explosion", d_explosion->getName ());
  retval &= helper->saveData("roads", d_road->getName ());
  retval &= helper->saveData("standing_stones", d_stone->getName ());
  retval &= helper->saveData("bridges", d_bridge->getName ());
  retval &= helper->saveData("fog", d_fog->getName ());
  retval &= helper->saveData("flags", d_flag->getName ());
  retval &= helper->saveData("movebonus_all", d_all_movebonus->getName ());
  retval &= helper->saveData("movebonus_water", d_water_movebonus->getName ());
  retval &= helper->saveData("movebonus_forest", d_forest_movebonus->getName ());
  retval &= helper->saveData("movebonus_hills", d_hills_movebonus->getName ());
  retval &= helper->saveData("movebonus_mountains", d_mountains_movebonus->getName ());
  retval &= helper->saveData("movebonus_swamp", d_swamp_movebonus->getName ());
  retval &= helper->openTag(d_road_smallmap_tag);
  retval &= helper->saveData("color", d_road_color);
  retval &= helper->closeTag();
  retval &= helper->openTag(d_ruin_smallmap_tag);
  retval &= helper->saveData("color", d_ruin_color);
  retval &= helper->closeTag();
  retval &= helper->openTag(d_temple_smallmap_tag);
  retval &= helper->saveData("color", d_temple_color);
  retval &= helper->closeTag();
  for (Tileset::const_iterator i = begin(); i != end(); ++i)
    retval &= (*i)->save(helper);
  retval &= helper->closeTag();

  return retval;
}

bool Tileset::save(Glib::ustring filename, Glib::ustring ext) const
{
  bool broken = false;
  Glib::ustring goodfilename = File::add_ext_if_necessary(filename, ext);
  Glib::ustring tmpfile = File::get_tmp_file();
  XML_Helper helper(tmpfile, std::ios::out);
  helper.begin(LORDSAWAR_TILESET_VERSION);
  broken = !save(&helper);
  helper.close();
  if (broken == true)
    return false;
  std::vector<Glib::ustring> extrafiles;
  return saveTar(tmpfile, tmpfile + ".tar", goodfilename, extrafiles);
}

int Tileset::getFreeTileStyleId() const
{
  int ids[65535];
  memset (ids, 0, sizeof (ids));
  for (Tileset::const_iterator i = begin(); i != end(); ++i)
    {
      for (std::list<TileStyleSet*>::const_iterator j = (*i)->begin(); j != (*i)->end(); j++)
	{
	  for (std::vector<TileStyle*>::const_iterator k = (*j)->begin(); k != (*j)->end(); k++)
	    {
	      ids[(*k)->getId()]++;
	    }
	}
    }
  //these ids range from 0 to 65535.
  for (unsigned int i = 0; i <= 65535; i++)
    {
      if (ids[i] == 0)
	return i;
    }
  return -1;
}

int Tileset::getLargestTileStyleId() const
{
  unsigned int largest = 0;
  for (Tileset::const_iterator i = begin(); i != end(); ++i)
    {
      for (std::list<TileStyleSet*>::const_iterator j = (*i)->begin(); j != (*i)->end(); j++)
	{
	  for (std::vector<TileStyle*>::const_iterator k = (*j)->begin(); k != (*j)->end(); k++)
	    {
	      if ((*k)->getId() > largest)
		largest = (*k)->getId();
	    }
	}
    }
  return largest;
}

guint32 Tileset::getDefaultTileSize()
{
  return DEFAULT_TILE_SIZE;
}

bool Tileset::validate() const
{
  if (String::utrim (getName ()) == "")
    return false;
  if (size() == 0)
    return false;
  for (Tileset::const_iterator i = begin(); i != end(); i++)
    if ((*i)->validate() == false)
      return false;
  if (countTilesWithPattern(SmallTile::SUNKEN_RADIAL) > 1)
    return false;
  if (getIndex(Tile::GRASS) == -1)
    return false;
  if (getIndex(Tile::WATER) == -1)
    return false;
  if (getIndex(Tile::FOREST) == -1)
    return false;
  if (getIndex(Tile::HILLS) == -1)
    return false;
  if (getIndex(Tile::MOUNTAIN) == -1)
    return false;
  if (getIndex(Tile::SWAMP) == -1)
    return false;
  if (d_selector[1]->getName().empty () == true)
    return false;
  if (d_selector[0]->getName().empty () == true)
    return false;
  if (d_explosion->getName().empty () == true)
    return false;
  if (d_road->getName().empty () == true)
    return false;
  if (d_stone->getName().empty () == true)
    return false;
  if (d_bridge->getName().empty () == true)
    return false;
  if (d_fog->getName().empty () == true)
    return false;
  if (d_flag->getName().empty () == true)
    return false;
  if (getAllMoveBonus()->getName ().empty () == true)
    return false;
  if (getWaterMoveBonus()->getName ().empty () == true)
    return false;
  if (getForestMoveBonus()->getName ().empty () == true)
    return false;
  if (getHillsMoveBonus()->getName ().empty () == true)
    return false;
  if (getMountainsMoveBonus()->getName ().empty () == true)
    return false;
  if (getSwampMoveBonus()->getName ().empty () == true)
    return false;

  return true;
}

//! Helper class for making a new Tileset object from a tileset file.
class TilesetLoader
{
public:
    TilesetLoader(Glib::ustring filename, bool &broken, bool &unsupported)
      {
        unsupported_version = false;
	tileset = NULL;
	dir = File::get_dirname(filename);
        file = File::get_basename(filename);
	if (File::nameEndsWith(filename, Tileset::file_extension) == false)
	  filename += Tileset::file_extension;
        Tar_Helper t(filename, std::ios::in, broken);
        if (broken)
          return;
        Glib::ustring lwtfilename = 
          t.getFirstFile(Tileset::file_extension, broken);
        if (broken)
          return;
	XML_Helper helper(lwtfilename, std::ios::in);
	helper.registerTag(Tileset::d_tag, sigc::mem_fun((*this), &TilesetLoader::load));
	if (!helper.parseXML())
	  {
            unsupported = unsupported_version;
            std::cerr << String::ucompose(_("Error!  can't load Tile Set `%1'."), filename) << std::endl;
	    if (tileset != NULL)
	      delete tileset;
	    tileset = NULL;
	  }
        helper.close();
        File::erase(lwtfilename);
        t.Close();
      };
    bool load(Glib::ustring tag, XML_Helper* helper)
      {
	if (tag == Tileset::d_tag)
	  {
            if (helper->getVersion() == LORDSAWAR_TILESET_VERSION)
              {
                tileset = new Tileset(helper, dir);
                tileset->setBaseName(file);
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
    Tileset *tileset;
    bool unsupported_version;
};

Tileset *Tileset::create(Glib::ustring file, bool &unsupported_version)
{
  bool broken = false;
  TilesetLoader d(file, broken, unsupported_version);
  if (broken)
    return NULL;
  return d.tileset;
}

void Tileset::uninstantiateImages()
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->uninstantiateImages();

  d_selector[0]->uninstantiateImages ();
  d_selector[1]->uninstantiateImages ();
  d_flag->uninstantiateImages ();
  d_bridge->uninstantiateImages ();
  d_road->uninstantiateImages ();
  d_fog->uninstantiateImages ();
  d_explosion->uninstantiateImages ();
  d_stone->uninstantiateImages ();
  d_all_movebonus->uninstantiateImages ();
  d_water_movebonus->uninstantiateImages ();
  d_forest_movebonus->uninstantiateImages ();
  d_hills_movebonus->uninstantiateImages ();
  d_mountains_movebonus->uninstantiateImages ();
  d_swamp_movebonus->uninstantiateImages ();
}

void Tileset::instantiateImages(bool scale, bool &broken)
{
  int siz = getUnscaledTileSize();
  Vector<int>scale_dim = Vector<int>(siz,siz);
  debug("Loading images for Tile Set " << getName());
  uninstantiateImages();
  broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return;
  for (iterator it = begin(); it != end(); it++)
    {
      if (!broken)
        (*it)->instantiateImages(siz, &t, scale, broken);
    }
  if (broken)
    return;


  broken = d_selector[0]->load(&t);
  if (broken)
    return;
  d_selector[0]->instantiateImages (scale_dim);

  broken = d_selector[1]->load(&t);
  if (broken)
    return;
  d_selector[1]->instantiateImages (scale_dim);

  broken = d_flag->load(&t);
  if (broken)
    return;
  d_flag->instantiateImages (scale_dim);

  broken = d_all_movebonus->load(&t);
  if (broken)
    return;
  d_all_movebonus->instantiateImages ();

  broken = d_water_movebonus->load(&t);
  if (broken)
    return;
  d_water_movebonus->instantiateImages ();

  broken = d_forest_movebonus->load(&t);
  if (broken)
    return;
  d_forest_movebonus->instantiateImages ();

  broken = d_hills_movebonus->load(&t);
  if (broken)
    return;
  d_hills_movebonus->instantiateImages ();

  broken = d_mountains_movebonus->load(&t);
  if (broken)
    return;
  d_mountains_movebonus->instantiateImages ();

  broken = d_swamp_movebonus->load(&t);
  if (broken)
    return;
  d_swamp_movebonus->instantiateImages ();

  broken = d_explosion->load(&t);
  if (broken)
    return;
  d_explosion->instantiateImages ();

  broken = d_road->load(&t);
  if (broken)
    return;
  d_road->instantiateImages ();

  broken = d_stone->load(&t);
  if (broken)
    return;
  d_stone->instantiateImages ();

  broken = d_bridge->load(&t);
  if (broken)
    return;
  d_bridge->instantiateImages ();

  broken = d_fog->load(&t);
  if (broken)
    return;
  d_fog->instantiateImages ();

  t.Close();
  return;
}

TileStyle *Tileset::getTileStyle(guint32 id) const
{
  TileStyleIdMap::const_iterator it = d_tilestyles.find(id);
  if (it == d_tilestyles.end())
    return NULL;
  else
    return (*it).second;
}

void Tileset::reload(bool &broken)
{
  broken = false;
  bool unsupported_version = false;
  TilesetLoader d(getConfigurationFile(), broken, unsupported_version);
  if (!broken && d.tileset && d.tileset->validate())
    {
      //steal the values from d.tileset and then don't delete it.
      uninstantiateImages();
      for (iterator it = begin(); it != end(); it++)
        delete *it;
      Glib::ustring basename = getBaseName();
      *this = *d.tileset;
      instantiateImages(true, broken);
      setBaseName(basename);
    }
}

bool Tileset::calculate_preferred_tile_size(guint32 &ts) const
{
  guint32 tilesize = 0;
  std::map<guint32, guint32> sizecounts;

  if (d_road->getImage ())
    sizecounts[d_road->getImage ()->get_unscaled_width()]++;
  if (d_bridge->getImage ())
    sizecounts[d_bridge->getImage ()->get_unscaled_width()]++;
  if (d_flag->getName ().empty () == false)
    sizecounts[d_flag->getImage ()->get_unscaled_width()]++;
  if (d_selector[0]->getName ().empty() == false)
    sizecounts[d_selector[0]->getImage ()->get_unscaled_width()]++;
  if (d_selector[1]->getName ().empty() == false)
    sizecounts[d_selector[1]->getImage ()->get_unscaled_width()]++;
  if (d_fog->getImage ())
    sizecounts[d_fog->getImage ()->get_unscaled_width()]++;
  if (d_explosion->getImage ())
    sizecounts[d_explosion->getImage ()->get_unscaled_width()]++;
  for (const_iterator it = begin(); it != end(); it++)
    {
      Tile *tile = *it;
      for (Tile::const_iterator i = tile->begin(); i != tile->end(); i++)
        {
          TileStyle *tilestyle = (*i)->front();
          if (tilestyle && tilestyle->getImage())
            sizecounts[tilestyle->getImage()->get_unscaled_width()]++;
        }
    }

  guint32 maxcount = 0;
  for (auto it: sizecounts)
    {
      if (it.second > maxcount)
        {
          maxcount = it.second;
          tilesize = it.first;
        }
    }
  bool ret = true;
  if (tilesize == 0)
    {
      ts = DEFAULT_TILE_SIZE;
      ret = false;
    }
  else
    ts = tilesize;
  return ret;
}

bool Tileset::addTileStyleSet(Tile *tile, Glib::ustring filename)
{
  bool success = true;
  TileStyle::Type tilestyle_type;
  tilestyle_type = TileStyle::UNKNOWN;
  TileStyleSet *set = 
    new TileStyleSet(filename, getUnscaledTileSize(), success, tilestyle_type);
  if (!success)
    {
      delete set;
      return success;
    }
  tile->push_back(set);
  for (TileStyleSet::iterator it = set->begin(); it != set->end(); it++)
    {
      guint32 tile_style_id = getFreeTileStyleId();
      d_tilestyles[tile_style_id] = (*it);
      (*it)->setId(tile_style_id);
    }
  return success;
}

bool Tileset::getTileStyle(guint32 id, Tile **tile, TileStyleSet **set, TileStyle ** style) const
{
  for (const_iterator t = begin(); t != end(); t++)
    for (std::list<TileStyleSet*>::const_iterator i = (*t)->begin(); 
         i != (*t)->end(); i++)
      for (std::vector<TileStyle*>::const_iterator j = (*i)->begin(); 
           j != (*i)->end(); j++)
        if ((*j)->getId() == id)
          {
            if (tile)
              *tile = *t;
            if (set)
              *set = *i;
            if (style)
              *style = *j;
            return true;
          }
  return false;
}

bool Tileset::upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version)
{
  return FileCompat::getInstance()->upgrade(filename, old_version, new_version,
                                            FileCompat::TILESET, d_tag);
}

void Tileset::support_backward_compatibility()
{
  FileCompat::getInstance()->support_type(FileCompat::TILESET, file_extension, 
                                          d_tag, true);
  FileCompat::getInstance()->support_version
    (FileCompat::TILESET, "0.2.1", "0.3.2",
     sigc::ptr_fun(&Tileset::upgrade));
}

Tile *Tileset::getFirstTile(SmallTile::Pattern pattern) const
{
  for (const_iterator i = begin(); i != end(); i++)
    if ((*i)->getSmallTile()->getPattern() == pattern)
      return *i;
  return NULL;
}

Tileset* Tileset::copy(const Tileset *tileset)
{
  if (!tileset)
    return NULL;
  return new Tileset(*tileset);
}

void Tileset::populateWithDefaultTiles()
{
  uninstantiateImages();
  for (unsigned int i=0; i < size(); i++)
    delete (*this)[i];
  clear();
  push_back(Tile::get_default_grass());
  push_back(Tile::get_default_water());
  push_back(Tile::get_default_forest());
  push_back(Tile::get_default_hills());
  push_back(Tile::get_default_mountains());
  push_back(Tile::get_default_swamp());
}

int Tileset::countTilesWithPattern(SmallTile::Pattern pattern) const
{
  int radial_count = 0;
  for (Tileset::const_iterator i = begin(); i != end(); i++)
    {
      if ((*i)->getSmallTile()->getPattern() == pattern)
        radial_count++;
    }
  return radial_count;
}

guint32 Tileset::get_default_tile_size ()
{
  Tileset *t = new Tileset (1, "");
  guint32 ts = t->getUnscaledTileSize ();
  delete t;
  return ts;
}

void Tileset::uninstantiateSameNamedImages (Glib::ustring name)
{
  if (d_selector[1]->getName () == name)
    d_selector[1]->clear ();
  if (d_selector[0]->getName () == name)
    d_selector[0]->clear ();
  if (d_explosion->getName () == name)
    d_explosion->clear ();
  if (d_road->getName () == name)
    d_road->clear ();
  if (d_stone->getName () == name)
    d_stone->clear ();
  if (d_bridge->getName () == name)
    d_bridge->clear ();
  if (d_fog->getName () == name)
    d_fog->clear ();
  if (d_flag->getName () == name)
    d_flag->clear ();
  if (getAllMoveBonus ()->getName () == name)
    d_all_movebonus->clear ();
  if (getWaterMoveBonus ()->getName () == name)
    d_water_movebonus->clear ();
  if (getForestMoveBonus ()->getName () == name)
    d_forest_movebonus->clear ();
  if (getHillsMoveBonus()->getName () == name)
    d_hills_movebonus->clear ();
  if (getMountainsMoveBonus ()->getName () == name)
    d_mountains_movebonus->clear ();
  if (getSwampMoveBonus ()->getName () == name)
    d_swamp_movebonus->clear ();
  std::vector<TileStyleSet*> sets;
  for (iterator i = begin (); i != end (); i++)
    {
      for (auto tst : *(*i))
        {
          if (tst->getName () == name)
            sets.push_back (tst);
        }
    }
  for (auto s : sets)
    delete s;
}
//End of file
