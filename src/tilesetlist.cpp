//  Copyright (C) 2007, 2008, 2009, 2010, 2011, 2014, 2015, 2020 Ben Asselstine
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
#include <assert.h>
#include "rectangle.h"
#include "ucompose.hpp"
#include <sigc++/functors/mem_fun.h>

#include "tilesetlist.h"
#include "File.h"
#include "defs.h"
#include "tileset.h"
#include "tarhelper.h"
#include "SmallTile.h"
#include "setlist.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Tilesetlist* Tilesetlist::s_instance = 0;

Tilesetlist* Tilesetlist::getInstance()
{
    if (!s_instance)
        s_instance = new Tilesetlist();

    return s_instance;
}

void Tilesetlist::deleteInstance()
{
    if (s_instance)
      delete s_instance;

    s_instance = 0;
}

Tilesetlist::Tilesetlist()
 : SetList(Tileset::file_extension)
{
    // load all tilesets
    loadSets(SetList::scan(Tileset::file_extension));
    loadSets(SetList::scan(Tileset::file_extension, false));
}

Tilesetlist::~Tilesetlist()
{
  uninstantiateImages();
  for (iterator it = begin(); it != end(); ++it)
    delete *it;
  clear();
}

void Tilesetlist::uninstantiateImages()
{
  for (iterator it = begin(); it != end(); ++it)
    (*it)->uninstantiateImages();
}

void Tilesetlist::instantiateImages(bool &broken)
{
  broken = false;
  for (iterator it = begin(); it != end(); ++it)
    {
      if (!broken)
        {
          if ((*it)->validate () == true)
            (*it)->instantiateImages(true, broken);
        }
    }
}

SmallTile *Tilesetlist::getSmallTile(Glib::ustring basename, Tile::Type type) const
{
  Tileset *ts = get(basename);
  if (!ts)
    return NULL;
  int idx = ts->getIndex(type);
  return (*ts)[idx]->getSmallTile();
}

Gdk::RGBA Tilesetlist::getColor(Glib::ustring basename, Tile::Type type) const
{
  SmallTile *smalltile = getSmallTile(basename, type);
  if (!smalltile)
    return Gdk::RGBA("black");
  else
    return smalltile->getColor();
}
