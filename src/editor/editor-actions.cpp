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

#include "editor-actions.h"
#include "File.h"
#include "Scenario.h"
#include "GameScenario.h"
#include "GameMap.h"
#include "tileset.h"
#include "cityset.h"
#include "shieldset.h"
#include "armyset.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<< x << std::endl<<std::flush;}
#define debug(x)


//-----------------------------------------------------------------------------
//EditorAction_Save

EditorAction_Save::EditorAction_Save (Type t, GameScenario *g)
 :EditorAction (t)
{
  d_scenario = new Scenario (g);

  d_filename = File::get_tmp_file () + SAVE_EXT;
  d_scenario->getGameScenario ()->saveGame (d_filename);
}

EditorAction_Save::~EditorAction_Save ()
{
  File::erase (d_filename);
  delete d_scenario;
}
        
//-----------------------------------------------------------------------------
//EditorAction_ChangeMap
EditorAction_ChangeMap::EditorAction_ChangeMap (Type t, LwRectangle r, bool grow, bool agg, bool only_maptiles)
: EditorAction (t, agg ? AGGREGATE_BLANK : AGGREGATE_NONE),
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

void EditorAction_ChangeMap::clearObjects ()
{
  objects = std::list<UniquelyIdentified*>();
}

EditorAction_ChangeMap::EditorAction_ChangeMap (Type t, LwRectangle r1, LwRectangle r2, bool grow, bool agg, bool only_maptiles)
: EditorAction (t, agg ? AGGREGATE_BLANK : AGGREGATE_NONE),
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

EditorAction_ChangeMap::~EditorAction_ChangeMap ()
{
  for (auto m : maptiles)
    delete m;
  for (auto o : objects)
    delete o;
}

//-----------------------------------------------------------------------------
//EditorAction_Terrain

Glib::ustring EditorAction_Terrain::getActionName () const
{
  Tileset *ts = GameMap::getTileset ();
  int idx = ts->getIndex (d_tile);
  if (idx < 0)
    return "";
  return (*ts)[idx]->getName ();
}
          
//-----------------------------------------------------------------------------
//EditorAction_TileSet

EditorAction_TileSet::EditorAction_TileSet (Tileset *t)
 :EditorAction (EditorAction::TILESET)
{
  d_tileset = new Tileset (*t);
  d_filename = File::get_tmp_file () + TILESET_EXT;
  t->save (d_filename, TILESET_EXT);
}

EditorAction_TileSet::~EditorAction_TileSet ()
{
  File::erase (d_filename);
  delete d_tileset;
}

//-----------------------------------------------------------------------------
//EditorAction_CitySet

EditorAction_CitySet::EditorAction_CitySet (Cityset *c)
 :EditorAction (EditorAction::CITYSET)
{
  d_cityset = new Cityset (*c);
  d_filename = File::get_tmp_file () + CITYSET_EXT;
  c->save (d_filename, CITYSET_EXT);
}

EditorAction_CitySet::~EditorAction_CitySet ()
{
  File::erase (d_filename);
  delete d_cityset;
}

//-----------------------------------------------------------------------------
//EditorAction_ArmySet

EditorAction_ArmySet::EditorAction_ArmySet (Armyset *a)
 :EditorAction (EditorAction::ARMYSET)
{
  d_armyset = new Armyset (*a);
  d_filename = File::get_tmp_file () + ARMYSET_EXT;
  a->save (d_filename, ARMYSET_EXT);
}

EditorAction_ArmySet::~EditorAction_ArmySet ()
{
  File::erase (d_filename);
  delete d_armyset;
}

//-----------------------------------------------------------------------------
//EditorAction_ShieldSet

EditorAction_ShieldSet::EditorAction_ShieldSet (Shieldset *s)
 :EditorAction (EditorAction::SHIELDSET)
{
  d_shieldset = new Shieldset (*s);
  d_filename = File::get_tmp_file () + SHIELDSET_EXT;
  s->save (d_filename, SHIELDSET_EXT);
}

EditorAction_ShieldSet::~EditorAction_ShieldSet ()
{
  File::erase (d_filename);
  delete d_shieldset;
}
