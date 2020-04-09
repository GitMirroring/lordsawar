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
  d_large_selector = "";
  d_small_selector = "";
  d_fog = "";
  d_roads = "";
  d_standing_stones = "";
  d_bridges = "";
  d_flags = "";
  d_road_color.set_rgba(164.0/255.0,84.0/255.0,0);
  d_ruin_color.set_rgba(1,1,1);
  d_temple_color.set_rgba(1,1,1);
  for (unsigned int i = 0; i < ROAD_TYPES; i++)
    roadpic[i] = NULL;
  for (unsigned int i = 0; i < STONE_TYPES; i++)
    stonepic[i] = NULL;
  for (unsigned int i = 0; i < BRIDGE_TYPES; i++)
    bridgepic[i] = NULL;
  for (unsigned int i = 0; i < FLAG_TYPES; i++)
    flagpic[i] = NULL;
  for (unsigned int i = 0; i < FLAG_TYPES; i++)
    flagmask[i] = NULL;
  number_of_selector_frames = 0;
  selector.clear();
  selectormask.clear();
  number_of_small_selector_frames = 0;
  smallselector.clear();
  smallselectormask.clear();
  explosion = NULL;
  for (unsigned int i = 0; i < FOG_TYPES; i++)
    fogpic[i] = NULL;
}

Tileset::Tileset (const Tileset& t)
  : sigc::trackable(t), std::vector<Tile*>(), Set(t)
{
  d_large_selector = t.d_large_selector;
  d_small_selector = t.d_small_selector;
  d_explosion = t.d_explosion;
  d_fog = t.d_fog;
  d_roads = t.d_roads;
  d_standing_stones = t.d_standing_stones;
  d_bridges = t.d_bridges;
  d_flags = t.d_flags;
  d_road_color = t.d_road_color;
  d_ruin_color = t.d_ruin_color;
  d_temple_color = t.d_temple_color;
  for (unsigned int i = 0; i < ROAD_TYPES; i++)
    {
      if (t.roadpic[i])
        roadpic[i] = t.roadpic[i]->copy();
      else
        roadpic[i] = NULL;
    }
  for (unsigned int i = 0; i < STONE_TYPES; i++)
    {
      if (t.stonepic[i])
        stonepic[i] = t.stonepic[i]->copy();
      else
        stonepic[i] = NULL;
    }
  for (unsigned int i = 0; i < BRIDGE_TYPES; i++)
    {
      if (t.bridgepic[i])
        bridgepic[i] = t.bridgepic[i]->copy();
      else
        bridgepic[i] = NULL;
    }
  for (unsigned int i = 0; i < FLAG_TYPES; i++)
    {
      if (t.flagpic[i])
        flagpic[i] = t.flagpic[i]->copy();
      else
        flagpic[i] = NULL;
    }
  for (unsigned int i = 0; i < FLAG_TYPES; i++)
    {
      if (t.flagmask[i])
        flagmask[i] = t.flagmask[i]->copy();
      else
        flagmask[i] = NULL;
    }
  number_of_selector_frames = t.number_of_selector_frames;
  std::vector<PixMask*> s1 = std::vector<PixMask*>(number_of_selector_frames);
  for (unsigned int i = 0; i < number_of_selector_frames; i++)
    s1[i] = t.selector[i]->copy();
  selector = s1;
  std::vector<PixMask*> s2 = std::vector<PixMask*>(number_of_selector_frames);
  for (unsigned int i = 0; i < number_of_selector_frames; i++)
    s2[i] = t.selectormask[i]->copy();
  selectormask = s2;

  number_of_small_selector_frames = t.number_of_small_selector_frames;
  std::vector<PixMask*> s3 = std::vector<PixMask*>(number_of_small_selector_frames);
  for (unsigned int i = 0; i < number_of_small_selector_frames; i++)
    s3[i] = t.smallselector[i]->copy();
  smallselector = s3;
  std::vector<PixMask*> s4 = std::vector<PixMask*>(number_of_small_selector_frames);
  for (unsigned int i = 0; i < number_of_small_selector_frames; i++)
    s4[i] = t.smallselectormask[i]->copy();
  smallselectormask = s4;

  if (t.explosion != NULL)
    explosion = t.explosion->copy();
  else
    explosion = NULL;

  for (unsigned int i = 0; i < FOG_TYPES; i++)
    {
      if (t.fogpic[i] != NULL)
        fogpic[i] = t.fogpic[i]->copy();
      else
        fogpic[i] = NULL;
    }

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
}

Tileset::Tileset(XML_Helper *helper, Glib::ustring directory)
	:Set(TILESET_EXT, helper)
{
  setDirectory(directory);
  guint32 ts;
  helper->getData(ts, "tilesize");
  setTileSize(ts);
  helper->getData(d_large_selector, "large_selector");
  File::add_png_if_no_ext (d_large_selector);
  helper->getData(d_small_selector, "small_selector");
  File::add_png_if_no_ext (d_small_selector);
  helper->getData(d_explosion, "explosion");
  File::add_png_if_no_ext (d_explosion);
  helper->getData(d_roads, "roads");
  File::add_png_if_no_ext (d_roads);
  helper->getData(d_standing_stones, "standing_stones");
  File::add_png_if_no_ext (d_standing_stones);
  helper->getData(d_bridges, "bridges");
  File::add_png_if_no_ext (d_bridges);
  helper->getData(d_fog, "fog");
  File::add_png_if_no_ext (d_fog);
  helper->getData(d_flags, "flags");
  File::add_png_if_no_ext (d_flags);
  helper->registerTag(Tile::d_tag, sigc::mem_fun((*this), &Tileset::loadTile));
  helper->registerTag(Tileset::d_road_smallmap_tag, sigc::mem_fun((*this), &Tileset::loadTile));
  helper->registerTag(Tileset::d_ruin_smallmap_tag, sigc::mem_fun((*this), &Tileset::loadTile));
  helper->registerTag(Tileset::d_temple_smallmap_tag, sigc::mem_fun((*this), &Tileset::loadTile));
  helper->registerTag(SmallTile::d_tag, sigc::mem_fun((*this), &Tileset::loadTile));
  helper->registerTag(TileStyle::d_tag, sigc::mem_fun((*this), &Tileset::loadTile));
  helper->registerTag(TileStyleSet::d_tag, sigc::mem_fun((*this), &Tileset::loadTile));
  for (unsigned int i = 0; i < ROAD_TYPES; i++)
    roadpic[i] = NULL;
  for (unsigned int i = 0; i < STONE_TYPES; i++)
    stonepic[i] = NULL;
  for (unsigned int i = 0; i < BRIDGE_TYPES; i++)
    bridgepic[i] = NULL;
  for (unsigned int i = 0; i < FLAG_TYPES; i++)
    flagpic[i] = NULL;
  for (unsigned int i = 0; i < FLAG_TYPES; i++)
    flagmask[i] = NULL;
  number_of_selector_frames = 0;
  selector.clear();
  selectormask.clear();
  number_of_small_selector_frames = 0;
  smallselector.clear();
  smallselectormask.clear();
  explosion = NULL;
  for (unsigned int i = 0; i < FOG_TYPES; i++)
    fogpic[i] = NULL;
}

Tileset::~Tileset()
{
  uninstantiateImages();
  for (unsigned int i=0; i < size(); i++)
    delete (*this)[i];
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
  retval &= helper->saveData("large_selector", d_large_selector);
  retval &= helper->saveData("small_selector", d_small_selector);
  retval &= helper->saveData("explosion", d_explosion);
  retval &= helper->saveData("roads", d_roads);
  retval &= helper->saveData("standing_stones", d_standing_stones);
  retval &= helper->saveData("bridges", d_bridges);
  retval &= helper->saveData("fog", d_fog);
  retval &= helper->saveData("flags", d_flags);
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
  return saveTar(tmpfile, tmpfile + ".tar", goodfilename);
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
  if (getLargeSelectorFilename().empty () == true)
    return false;
  if (getSmallSelectorFilename().empty () == true)
    return false;
  if (getExplosionFilename().empty () == true)
    return false;
  if (getRoadsFilename().empty () == true)
    return false;
  if (getStonesFilename().empty () == true)
    return false;
  if (getBridgesFilename().empty () == true)
    return false;
  if (getFogFilename().empty () == true)
    return false;
  if (getFlagsFilename().empty () == true)
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

  for (unsigned int i = 0; i < ROAD_TYPES; i++)
    {
      if (roadpic[i] != NULL)
        delete roadpic[i];
      roadpic[i] = NULL;
    }

  for (unsigned int i = 0; i < STONE_TYPES; i++)
    {
      if (stonepic[i] != NULL)
        delete stonepic[i];
      stonepic[i] = NULL;
    }

  for (unsigned int i = 0; i < BRIDGE_TYPES; i++)
    {
      if (bridgepic[i] != NULL)
        delete bridgepic[i];
      bridgepic[i] = NULL;
    }

  for (unsigned int i = 0; i < FLAG_TYPES; i++)
    {
      if (flagpic[i] != NULL)
        delete flagpic[i];
      if (flagmask[i] != NULL)
        delete flagmask[i];
      flagpic[i] = NULL;
      flagmask[i] = NULL;
    }

  for (unsigned int i = 0; i < getNumberOfSelectorFrames(); i++)
    {
      if (selector[i] != NULL)
        delete selector[i];
      if (selectormask[i] != NULL)
        delete selectormask[i];
      selector[i] = NULL;
      selectormask[i] = NULL;
    }

  for (unsigned int i = 0; i < getNumberOfSmallSelectorFrames(); i++)
    {
      if (smallselector[i] != NULL)
        delete smallselector[i];
      if (smallselectormask[i] != NULL)
        delete smallselectormask[i];
      smallselector[i] = NULL;
      smallselectormask[i] = NULL;
    }

  if (explosion != NULL)
    delete explosion;
      
  explosion = NULL;

  for (unsigned int i = 0; i < FOG_TYPES; i++)
    {
      if (fogpic[i] != NULL)
	{
	  delete fogpic[i];
          fogpic[i] = NULL;
	}
    }
}

void Tileset::instantiateImages(Glib::ustring explosion_filename,
				Glib::ustring roads_filename,
				Glib::ustring stones_filename,
				Glib::ustring bridges_filename,
				Glib::ustring fog_filename,
				Glib::ustring flags_filename,
				Glib::ustring selector_filename,
				Glib::ustring small_selector_filename,
                                bool scale, bool &broken)
{
  if (explosion_filename.empty() == false && !broken)
    setExplosionImage (PixMask::create(explosion_filename, broken));

  if (roads_filename.empty() == false && !broken)
    {
      std::vector<PixMask* > roadpics;
      roadpics = disassemble_row(roads_filename, ROAD_TYPES, broken);
      if (!broken)
        {
          for (unsigned int i = 0; i < ROAD_TYPES ; i++)
            {
              if (scale)
                {
                  if (roadpics[i]->get_width() != (int)getUnscaledTileSize())
                    PixMask::scale(roadpics[i], getUnscaledTileSize(),
                                   getUnscaledTileSize());
                }
              setRoadImage(i, roadpics[i]);
            }
        }

    }

  if (stones_filename.empty() == false && !broken)
    {
      std::vector<PixMask* > stonepics;
      stonepics = disassemble_row(stones_filename, STONE_TYPES, broken);
      if (!broken)
        {
          for (unsigned int i = 0; i < STONE_TYPES ; i++)
            {
              if (scale)
                {
                  if (stonepics[i]->get_width() != (int)getUnscaledTileSize())
                    PixMask::scale(stonepics[i], getUnscaledTileSize(),
                                   getUnscaledTileSize());
                }
              setStoneImage(i, stonepics[i]);
            }
        }
    }

  if (bridges_filename.empty() == false && !broken)
    {
      std::vector<PixMask* > bridgepics;
      bridgepics = disassemble_row(bridges_filename, BRIDGE_TYPES, broken);
      if (!broken)
        {
          for (unsigned int i = 0; i < BRIDGE_TYPES ; i++)
            {
              if (scale)
                {
                  if (bridgepics[i]->get_width() != (int)getUnscaledTileSize())
                    PixMask::scale(bridgepics[i], getUnscaledTileSize(),
                                   getUnscaledTileSize());
                }
              setBridgeImage(i, bridgepics[i]);
            }
        }
    }

  if (fog_filename.empty() == false && !broken)
    {
      std::vector<PixMask* > fogpics;
      fogpics = disassemble_row(fog_filename, FOG_TYPES, broken);
      if (!broken)
        {
          for (unsigned int i = 0; i < FOG_TYPES ; i++)
            {
              if (scale)
                {
                  if (fogpics[i]->get_width() != (int)getUnscaledTileSize())
                    PixMask::scale(fogpics[i], getUnscaledTileSize(),
                                   getUnscaledTileSize());
                }
              setFogImage(i, fogpics[i]);
            }
        }
    }

  if (flags_filename.empty() == false && !broken)
    {
      std::vector<PixMask* > flagpics;
      std::vector<PixMask* > maskpics;
      bool success;
      success =
        FlagPixMaskCacheItem::loadFlagImages (flags_filename,
                                              getUnscaledTileSize(),
                                              flagpics, maskpics, scale);
      if (success)
        {
          for (unsigned int i = 0; i < flagpics.size(); i++)
            setFlagImage(i, flagpics[i]);
          for (unsigned int i = 0; i < maskpics.size(); i++)
            setFlagMask(i, maskpics[i]);
        }
      else
        broken = true;
    }

  std::vector<PixMask* > images;
  std::vector<PixMask* > masks;
  if (selector_filename.empty() == false && !broken)
    {
      bool success =
        SelectorPixMaskCacheItem::loadSelectorImages (selector_filename, 
                                                      getUnscaledTileSize(), 
                                                      images, masks, scale);
      if (success)
        {
          setNumberOfSelectorFrames(images.size());
          for (unsigned int i = 0; i < images.size(); i++)
            {
              setSelectorImage(i, images[i]);
              setSelectorMask(i, masks[i]);
            }
        }
      else
        broken = true;
    }

  images.clear();
  masks.clear();
  if (small_selector_filename.empty() == false && !broken)
    {
      bool success =
        SelectorPixMaskCacheItem::loadSelectorImages (small_selector_filename, 
                                                      getUnscaledTileSize(),
                                                      images, masks, scale);
      if (success)
        {
          setNumberOfSmallSelectorFrames(images.size());
          for (unsigned int i = 0; i < images.size(); i++)
            {
              setSmallSelectorImage(i, images[i]);
              setSmallSelectorMask(i, masks[i]);
            }
        }
      else
        broken = true;
    }
}

void Tileset::instantiateImages(bool scale, bool &broken)
{
  int siz = getUnscaledTileSize();
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
  Glib::ustring explosion_filename = "";
  Glib::ustring roads_filename = "";
  Glib::ustring stones_filename = "";
  Glib::ustring bridges_filename = "";
  Glib::ustring fog_filename = "";
  Glib::ustring flags_filename = "";
  Glib::ustring selector_filename = "";
  Glib::ustring small_selector_filename = "";

  if (getExplosionFilename().empty() == false && !broken)
    explosion_filename = t.getFile(getExplosionFilename(), broken);
  if (getRoadsFilename().empty() == false && !broken)
    roads_filename = t.getFile(getRoadsFilename(), broken);
  if (getStonesFilename().empty() == false && !broken)
    stones_filename = t.getFile(getStonesFilename(), broken);
  if (getBridgesFilename().empty() == false && !broken)
    bridges_filename = t.getFile(getBridgesFilename(), broken);
  if (getFogFilename().empty() == false && !broken)
    fog_filename = t.getFile(getFogFilename(), broken);
  if (getFlagsFilename().empty() == false && !broken)
    flags_filename = t.getFile(getFlagsFilename(), broken);
  if (getLargeSelectorFilename().empty() == false && !broken)
    selector_filename = t.getFile(getLargeSelectorFilename(), broken);
  if (getSmallSelectorFilename().empty() == false && !broken)
    small_selector_filename = 
      t.getFile(getSmallSelectorFilename(), broken);
  if (!broken)
    instantiateImages(explosion_filename, roads_filename, stones_filename,
                      bridges_filename, fog_filename, flags_filename,
                      selector_filename, small_selector_filename, scale,
                      broken);
  if (explosion_filename.empty() == false)
    File::erase(explosion_filename);
  if (roads_filename.empty() == false)
    File::erase(roads_filename);
  if (stones_filename.empty() == false)
    File::erase(stones_filename);
  if (bridges_filename.empty() == false)
    File::erase(bridges_filename);
  if (fog_filename.empty() == false)
    File::erase(fog_filename);
  if (flags_filename.empty() == false)
    File::erase(flags_filename);
  if (selector_filename.empty() == false)
    File::erase(selector_filename);
  if (small_selector_filename.empty() == false)
    File::erase(small_selector_filename);
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

  if (roadpic[0])
    sizecounts[roadpic[0]->get_unscaled_width()]++;
  if (bridgepic[0])
    sizecounts[bridgepic[0]->get_unscaled_width()]++;
  if (flagpic[0])
    sizecounts[flagpic[0]->get_unscaled_width()]++;
  if (selector.empty() == false)
    sizecounts[selector[0]->get_unscaled_width()]++;
  if (smallselector.empty() == false)
    sizecounts[smallselector[0]->get_unscaled_width()]++;
  if (fogpic[0])
    sizecounts[fogpic[0]->get_unscaled_width()]++;
  if (explosion)
    sizecounts[explosion->get_unscaled_width()]++;
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

void Tileset::clearRoadsImage (bool clear_name)
{
  if (clear_name)
    setRoadsFilename ("");

  for (unsigned int i = 0; i < ROAD_TYPES; i++)
    {
      PixMask *p = getRoadImage (i);
      if (p)
        delete p;
      setRoadImage (i, NULL);
    }
}

void Tileset::clearStonesImage (bool clear_name)
{
  if (clear_name)
    setStonesFilename ("");

  for (unsigned int i = 0; i < STONE_TYPES; i++)
    {
      PixMask *p = getStoneImage (i);
      if (p)
        delete p;
      setStoneImage (i, NULL);
    }
}

void Tileset::clearBridgesImage (bool clear_name)
{
  if (clear_name)
    setBridgesFilename ("");

  for (unsigned int i = 0; i < BRIDGE_TYPES; i++)
    {
      PixMask *p = getBridgeImage (i);
      if (p)
        delete p;
      setBridgeImage (i, NULL);
    }
}

void Tileset::clearFlagsImage (bool clear_name)
{
  if (clear_name)
    setFlagsFilename ("");

  for (unsigned int i = 0; i < FLAG_TYPES; i++)
    {
      PixMask *p = getFlagImage (i);
      if (p)
        delete p;
      setFlagImage (i, NULL);
      p = getFlagMask (i);
      if (p)
        delete p;
      setFlagMask (i, NULL);
    }
}

void Tileset::clearSmallSelectorImage (bool clear_name)
{
  if (clear_name)
    setSmallSelectorFilename ("");

  for (unsigned int i = 0; i < getNumberOfSmallSelectorFrames(); i++)
    {
      PixMask *p = getSmallSelectorImage (i);
      if (p)
        delete p;
      setSmallSelectorImage (i, NULL);
      p = getSmallSelectorMask (i);
      if (p)
        delete p;
      setSmallSelectorMask (i, NULL);
    }
}

void Tileset::clearLargeSelectorImage (bool clear_name)
{
  if (clear_name)
    setLargeSelectorFilename ("");

  for (unsigned int i = 0; i < getNumberOfSelectorFrames(); i++)
    {
      PixMask *p = getSelectorImage (i);
      if (p)
        delete p;
      setSelectorImage (i, NULL);
      p = getSelectorMask (i);
      if (p)
        delete p;
      setSelectorMask (i, NULL);
    }
}

void Tileset::clearExplosionImage (bool clear_name)
{
  if (clear_name)
    setExplosionFilename ("");
  PixMask *p = getExplosionImage ();
  if (p)
    delete p;
  setExplosionImage (NULL);
}

void Tileset::clearFogImages (bool clear_name)
{
  if (clear_name)
    setFogFilename ("");

  for (unsigned int i = 0; i < FOG_TYPES; i++)
    {
      PixMask *p = getFogImage (i);
      if (p)
        delete p;
      setFogImage (i, NULL);
    }
}

bool Tileset::instantiateRoadImages()
{
  clearRoadsImage (false);
  bool broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return broken;
  Glib::ustring imgname = getRoadsFilename();
  if (imgname.empty() == false)
    {
      Glib::ustring filename = t.getFile(imgname, broken);
      if (!broken)
        {
          std::vector<PixMask* > roadpics;
          roadpics = disassemble_row(filename, ROAD_TYPES, broken);
          if (!broken)
            {
              for (unsigned int i = 0; i < ROAD_TYPES ; i++)
                setRoadImage(i, roadpics[i]);
            }
        }
    }
  return broken;
}

bool Tileset::instantiateStoneImages()
{
  clearStonesImage (false);
  bool broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return broken;
  Glib::ustring imgname = getStonesFilename();
  if (imgname.empty() == false)
    {
      Glib::ustring filename = t.getFile(imgname, broken);
      if (!broken)
        {
          std::vector<PixMask* > stonepics;
          stonepics = disassemble_row(filename, STONE_TYPES, broken);
          if (!broken)
            {
              for (unsigned int i = 0; i < STONE_TYPES ; i++)
                setStoneImage(i, stonepics[i]);
            }
        }
    }
  return broken;
}

bool Tileset::instantiateBridgeImages()
{
  clearBridgesImage (false);
  bool broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return broken;
  Glib::ustring imgname = getBridgesFilename();
  if (imgname.empty() == false)
    {
      Glib::ustring filename = t.getFile(imgname, broken);
      if (!broken)
        {
          std::vector<PixMask* > bridgepics;
          bridgepics = disassemble_row(filename, BRIDGE_TYPES, broken);
          if (!broken)
            {
              for (unsigned int i = 0; i < BRIDGE_TYPES ; i++)
                setBridgeImage(i, bridgepics[i]);
            }
        }
    }
  return broken;
}

bool Tileset::instantiateFlagImages()
{
  clearFlagsImage (false);
  bool broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return broken;
  Glib::ustring imgname = getFlagsFilename();
  if (imgname.empty() == false)
    {
      Glib::ustring filename = t.getFile(imgname, broken);
      if (!broken)
        {
          std::vector<PixMask* > flagpics, maskpics;
          bool success =
            FlagPixMaskCacheItem::loadFlagImages (filename,
                                                  getUnscaledTileSize(),
                                                  flagpics, maskpics,
                                                  false);
          if (success)
            {
              for (unsigned int i = 0; i < flagpics.size(); i++)
                setFlagImage(i, flagpics[i]);
              for (unsigned int i = 0; i < maskpics.size(); i++)
                setFlagMask(i, maskpics[i]);
            }
          else
            broken = true;
        }
    }
  return broken;
}

bool Tileset::instantiateSmallSelectorImages()
{
  clearSmallSelectorImage (false);
  bool broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return broken;
  Glib::ustring imgname = getSmallSelectorFilename();
  if (imgname.empty() == false)
    {
      Glib::ustring filename = t.getFile(imgname, broken);
      if (!broken)
        {
          std::vector<PixMask* > images, masks;
          bool success =
            SelectorPixMaskCacheItem::loadSelectorImages
            (filename, getUnscaledTileSize(), images, masks, false);
          if (success)
            {
              setNumberOfSmallSelectorFrames(images.size());
              for (unsigned int i = 0; i < images.size(); i++)
                {
                  setSmallSelectorImage(i, images[i]);
                  setSmallSelectorMask(i, masks[i]);
                }
            }
          else
            broken = true;
        }
    }
  return broken;
}

bool Tileset::instantiateLargeSelectorImages()
{
  clearLargeSelectorImage (false);
  bool broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return broken;
  Glib::ustring imgname = getLargeSelectorFilename();
  if (imgname.empty() == false)
    {
      Glib::ustring filename = t.getFile(imgname, broken);
      if (!broken)
        {
          std::vector<PixMask* > images, masks;
          bool success =
            SelectorPixMaskCacheItem::loadSelectorImages
            (filename, getUnscaledTileSize(), images, masks, false);
          if (success)
            {
              setNumberOfSelectorFrames(images.size());
              for (unsigned int i = 0; i < images.size(); i++)
                {
                  setSelectorImage(i, images[i]);
                  setSelectorMask(i, masks[i]);
                }
            }
          else
            broken = true;
        }
    }
  return broken;
}

bool Tileset::instantiateExplosionImage()
{
  clearExplosionImage (false);
  bool broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return broken;
  Glib::ustring imgname = getExplosionFilename();
  if (imgname.empty() == false)
    {
      Glib::ustring filename = t.getFile(imgname, broken);
      if (!broken)
        setExplosionImage (PixMask::create(filename, broken));
    }
  return broken;
}

bool Tileset::instantiateFogImages()
{
  clearFogImages (false);
  bool broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return broken;
  Glib::ustring imgname = getFogFilename();
  if (imgname.empty() == false)
    {
      Glib::ustring filename = t.getFile(imgname, broken);
      if (!broken)
        {
          std::vector<PixMask* > fogpics;
          fogpics = disassemble_row(filename, FOG_TYPES, broken);
          if (!broken)
            {
              for (unsigned int i = 0; i < FOG_TYPES ; i++)
                setFogImage(i, fogpics[i]);
            }
        }
    }
  return broken;
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
  if (getLargeSelectorFilename() == name)
    clearLargeSelectorImage ();
  if (getSmallSelectorFilename() == name)
    clearSmallSelectorImage ();
  if (getExplosionFilename() == name)
    clearExplosionImage ();
  if (getRoadsFilename() == name)
    clearRoadsImage ();
  if (getStonesFilename() == name)
    clearStonesImage ();
  if (getBridgesFilename() == name)
    clearBridgesImage ();
  if (getFogFilename() == name)
    clearFogImages ();
  if (getFlagsFilename() == name)
    clearFlagsImage ();
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
