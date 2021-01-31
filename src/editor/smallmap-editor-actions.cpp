// Copyright (C) 2021 Ben Asselstine
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

#include <sstream>
#include <sigc++/functors/mem_fun.h>

#include "smallmap-editor-actions.h"
#include "File.h"
#include "GameMap.h"
#include "tileset.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<< x << std::endl<<std::flush;}
#define debug(x)


//-----------------------------------------------------------------------------
//SmallmapEditorAction_ChangeMap
SmallmapEditorAction_ChangeMap::SmallmapEditorAction_ChangeMap (Type t, LwRectangle r, bool grow, bool agg, bool only_maptiles)
: SmallmapEditorAction (t, agg ? AGGREGATE_BLANK : AGGREGATE_NONE),
  rects (std::list<LwRectangle>()), d_only_maptiles (only_maptiles)
{
  rects.push_back (r);
  LwRectangle rr = r;
  if (grow)
    rr.grow (1);
  std::list<LwRectangle> grown_rr;
  grown_rr.push_back (rr);

  if (!only_maptiles)
    objects = GameMap::getInstance ()->copyObjects (grown_rr);
  maptiles = GameMap::getInstance ()->copyMaptiles (grown_rr);
}

void SmallmapEditorAction_ChangeMap::clearObjects ()
{
  objects = std::list<UniquelyIdentified*>();
}

SmallmapEditorAction_ChangeMap::SmallmapEditorAction_ChangeMap (Type t, LwRectangle r1, LwRectangle r2, bool grow, bool agg, bool only_maptiles)
: SmallmapEditorAction (t, agg ? AGGREGATE_BLANK : AGGREGATE_NONE),
  rects (std::list<LwRectangle>()), d_only_maptiles (only_maptiles)
{
  rects.push_back (r1);
  rects.push_back (r2);

  LwRectangle rr1 = r1;
  if (grow)
    rr1.grow (1);

  LwRectangle rr2 = r2;
  if (grow)
    rr2.grow (1);

  std::list<LwRectangle> grown_rr;
  grown_rr.push_back (rr1);
  grown_rr.push_back (rr2);

  if (!only_maptiles)
    objects = GameMap::getInstance ()->copyObjects (grown_rr);
  maptiles = GameMap::getInstance ()->copyMaptiles (grown_rr);
}

SmallmapEditorAction_ChangeMap::~SmallmapEditorAction_ChangeMap ()
{
  for (auto m : maptiles)
    delete m;
  for (auto o : objects)
    delete o;
}

//-----------------------------------------------------------------------------
//SmallmapEditorAction_Terrain

Glib::ustring SmallmapEditorAction_Terrain::getActionName () const
{
  Tileset *ts = GameMap::getTileset ();
  int idx = ts->getIndex (d_tile);
  if (idx < 0)
    return "";
  return (*ts)[idx]->getName ();
}
