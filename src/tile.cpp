// Copyright (C) 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2002, 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2007, 2008, 2010, 2011, 2014, 2015, 2020, 2021 Ben Asselstine
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
#include <list>
#include "Tile.h"
#include "SmallTile.h"
#include "File.h"
#include "tileset.h"
#include "tarhelper.h"
#include "xmlhelper.h"
#include "rnd.h"

Glib::ustring Tile::d_tag = "tile";

Tile::Tile()
 : d_name (""), d_moves (0), d_type (Tile::GRASS), d_smalltile (new SmallTile())
{
}

Tile::Tile(Tile::Type type, Glib::ustring name, guint32 moves, SmallTile*small)
 : d_name(name), d_moves(moves), d_type(type), d_smalltile(small)
{
}

Tile::Tile(const Tile &t)
 : std::list<TileStyleSet*>(), d_name(t.d_name), d_moves(t.d_moves), 
    d_type(t.d_type)
{
  d_smalltile = new SmallTile (*t.d_smalltile);
  for (Tile::const_iterator i = t.begin(); i != t.end(); ++i)
    push_back(new TileStyleSet(*(*i)));
}

Tile::Tile(XML_Helper* helper)
{
    helper->getData(d_name, "name");
    helper->getData(d_moves, "moves");
    Glib::ustring type_str;
    helper->getData(type_str, "type");
    d_type = Tile::Type(tileTypeFromString(type_str));
}

bool Tile::save(XML_Helper *helper) const
{
  bool retval = true;

  retval &= helper->openTag(d_tag);
  retval &= helper->saveData("name", d_name);
  retval &= helper->saveData("moves", d_moves);
  Glib::ustring type_str = tileTypeToString(Tile::Type(d_type));
  retval &= helper->saveData("type", type_str);
  retval &= d_smalltile->save(helper);
  for (Tile::const_iterator i = begin(); i != end(); ++i)
    retval &= (*i)->save(helper);
  retval &= helper->closeTag();

  return retval;
}

Tile::~Tile()
{
  for (iterator it = begin(); it != end(); ++it)
    delete *it;
  clear();
  if (d_smalltile)
    delete d_smalltile;
  d_smalltile = NULL;
}

void Tile::setTypeByIndex(int idx)
{
  switch (idx)
    {
    case 0: setType(GRASS); break;
    case 1: setType(WATER); break;
    case 2: setType(FOREST); break;
    case 3: setType(HILLS); break;
    case 4: setType(MOUNTAIN); break;
    case 5: setType(SWAMP); break;
    }
}

int Tile::getTypeIndexForType(Tile::Type type)
{
  switch (type)
    {
    case GRASS: return 0; break;
    case WATER: return 1; break;
    case FOREST: return 2; break;
    case HILLS: return 3; break;
    case MOUNTAIN: return 4; break;
    case SWAMP: return 5; break;
    }
  return 0;
}

TileStyle *Tile::getRandomTileStyle (TileStyle::Type style) const
{
  std::vector<TileStyle*> tilestyles;
  for (const_iterator it = begin(); it != end(); ++it)
    {
      TileStyleSet *tilestyleset = *it;
      for (guint32 k = 0; k < tilestyleset->size(); k++)
	{
	  TileStyle *tilestyle = (*tilestyleset)[k];
	  if (tilestyle->getType() == style)
	    tilestyles.push_back(tilestyle);
	}
    }

  if (tilestyles.empty() == true)
    return NULL;
  return tilestyles[Rnd::rand() % tilestyles.size()];
}

Glib::ustring Tile::tileTypeToString(const Tile::Type type)
{
  switch (type)
    {
    case Tile::GRASS: return "Tile::GRASS";
    case Tile::WATER: return "Tile::WATER";
    case Tile::FOREST: return "Tile::FOREST";
    case Tile::HILLS: return "Tile::HILLS";
    case Tile::MOUNTAIN: return "Tile::MOUNTAIN";
    case Tile::SWAMP: return "Tile::SWAMP";
    }
  return "Tile::GRASS";
}

Glib::ustring Tile::tileTypeToFriendlyName(const Tile::Type type)
{
  switch (type)
    {
    case Tile::GRASS: return _("Grass");
    case Tile::WATER: return _("Water");
    case Tile::FOREST: return _("Forest");
    case Tile::HILLS: return _("Hills");
    case Tile::MOUNTAIN: return _("Mountain");
    case Tile::SWAMP: return _("Swamp");
    }
  return _("Grass");
}

guint32 Tile::tileTypeFromString(const Glib::ustring str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return Tile::Type(atoi(str.c_str()));
  if (str == "Tile::GRASS") return Tile::GRASS;
  else if (str == "Tile::WATER") return Tile::WATER;
  else if (str == "Tile::FOREST") return Tile::FOREST;
  else if (str == "Tile::HILLS") return Tile::HILLS;
  else if (str == "Tile::MOUNTAIN") return Tile::MOUNTAIN;
  else if (str == "Tile::SWAMP") return Tile::SWAMP;
  return Tile::GRASS;
}

      
bool Tile::validateGrass(std::list<TileStyle::Type> types) const
{
  //grass tiles only have lone styles and other styles.
  for (auto it: types)
    if (it != TileStyle::LONE && it != TileStyle::OTHER)
      return false;
  return true;
}

bool Tile::validateFeature(std::list<TileStyle::Type> types) const
{
  //forest, water and hill tiles have a full suite of styles
  //"other" styles are optional.
  if (types.size() == TileStyle::OTHER)
    return true;
  if (types.size() == TileStyle::OTHER - 1 &&
      find (types.begin(), types.end(), TileStyle::OTHER) == types.end())
    return true;
  return false;
}

bool Tile::consistsOnlyOfLoneAndOtherStyles() const
{
  std::list<TileStyle::Type> types;
  for (Tile::const_iterator i = begin(); i != end(); ++i)
    (*i)->getUniqueTileStyleTypes(types);
      return validateGrass(types);
}

bool Tile::validate() const
{
  if (empty())
    return false;

  for (Tile::const_iterator i = begin(); i != end(); ++i)
    if ((*i)->validate() == false)
      return false;

  std::list<TileStyle::Type> types;
  for (Tile::const_iterator i = begin(); i != end(); ++i)
    (*i)->getUniqueTileStyleTypes(types);

  if (types.empty())
    return false;

  switch (getType())
    {
    case Tile::GRASS:
      if (validateGrass(types) == false)
	return false;
      break;
    case Tile::FOREST: case Tile::WATER: case Tile::HILLS: 
    case Tile::SWAMP: case Tile::MOUNTAIN:
      if (validateFeature(types) == false)
	{
	  if (validateGrass(types) == false)
	    return false;
	  else
	    return true;
	}
      break;
    }
  return true;
}

void Tile::uninstantiateImages()
{
  for (iterator it = begin(); it != end(); ++it)
    (*it)->uninstantiateImages();
}

void Tile::instantiateImages(int tilesize, Tar_Helper *t, bool scale,
                             bool &broken)
{
  broken = false;
  for (iterator it = begin(); it != end(); ++it)
    {
      Glib::ustring file = "";
      if ((*it)->getName().empty() == false && !broken)
        {
          file = t->getFile((*it)->getName(), broken);
          if (!broken)
            (*it)->loadImages(tilesize, file, scale, broken);
          if (file.empty() == false)
            File::erase(file);
        }
    }
}

guint32 Tile::countTileStyles(TileStyle::Type type) const
{
  guint32 count = 0;
  for (const_iterator i = begin(); i != end(); ++i)
    for (std::vector<TileStyle*>::const_iterator j = (*i)->begin(); j != (*i)->end(); ++j)
      if ((*j)->getType() == type)
        count++;
  return count;
}

TileStyle* Tile::getTileStyle(guint32 id) const
{
  for (const_iterator i = begin(); i != end(); ++i)
    for (auto j: **i)
      if (j->getId() == id)
        return j;
  return NULL;
}

std::list<TileStyle*> Tile::getTileStyles(TileStyle::Type type) const
{
  std::list<TileStyle*> styles;
  for (const_iterator i = begin(); i != end(); ++i)
    for (auto j: **i)
      if (j->getType() == type)
        styles.push_back(j);
  return styles;
}

Tile* Tile::get_default_grass()
{
  return new Tile(Tile::GRASS, Tile::tileTypeToFriendlyName(Tile::GRASS), 2, 
                  SmallTile::get_default_grass());
}

Tile* Tile::get_default_water()
{
  return new Tile(Tile::WATER, Tile::tileTypeToFriendlyName(Tile::WATER), 2, 
                  SmallTile::get_default_water());
}

Tile* Tile::get_default_forest()
{
  return new Tile(Tile::FOREST, Tile::tileTypeToFriendlyName(Tile::FOREST), 3, 
                  SmallTile::get_default_forest());
}

Tile* Tile::get_default_hills()
{
  return new Tile(Tile::HILLS, Tile::tileTypeToFriendlyName(Tile::HILLS), 4, 
                  SmallTile::get_default_hills());
}

Tile* Tile::get_default_mountains()
{
  return new Tile(Tile::MOUNTAIN, Tile::tileTypeToFriendlyName(Tile::MOUNTAIN),
                  6, SmallTile::get_default_mountains());
}

Tile* Tile::get_default_swamp()
{
  return new Tile(Tile::SWAMP, Tile::tileTypeToFriendlyName(Tile::SWAMP), 8, 
                  SmallTile::get_default_swamp());
}
        
std::list<std::pair<guint32, TileStyle::Type> >Tile::getAllTileStyleTypes () const
{
  std::list<std::pair<guint32, TileStyle::Type> > result;
  for (const_iterator i = begin(); i != end(); ++i)
    for (auto j: **i)
      result.push_back (std::pair<guint32, TileStyle::Type> (j->getId (), 
                                                             j->getType ()));
  return result;
}
// End of file
