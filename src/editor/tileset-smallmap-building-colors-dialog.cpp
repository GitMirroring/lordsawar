//  Copyright (C) 2010, 2014, 2020, 2021 Ben Asselstine
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

#include <config.h>

#include <sigc++/functors/mem_fun.h>

#include "tileset-smallmap-building-colors-dialog.h"

#include "ucompose.hpp"
#include "File.h"
#include "defs.h"
#include "tileset.h"
#include "tileset-smallmap-building-colors-actions.h"

#define method(x) sigc::mem_fun(*this, &TilesetSmallmapBuildingColorsDialog::x)

TilesetSmallmapBuildingColorsDialog::TilesetSmallmapBuildingColorsDialog(Gtk::Window &parent, Tileset *tileset)
 : LwEditorDialog(parent, "tileset-smallmap-building-colors-dialog.ui")
{
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));
  d_tileset = tileset;

  xml->get_widget("road_colorbutton", road_colorbutton);
  xml->get_widget("ruin_colorbutton", ruin_colorbutton);
  xml->get_widget("temple_colorbutton", temple_colorbutton);
  xml->get_widget("undo_button", undo_button);
  undo_button->signal_activate ().connect (method (on_undo_activated));
  xml->get_widget("redo_button", redo_button);
  redo_button->signal_activate ().connect (method (on_redo_activated));

  connect_signals ();
  update ();
}

TilesetSmallmapBuildingColorsDialog::~TilesetSmallmapBuildingColorsDialog ()
{
  delete umgr;
}

void TilesetSmallmapBuildingColorsDialog::on_road_color_chosen()
{
  TileSetSmallmapBuildingColorsAction_Color *action = 
    new TileSetSmallmapBuildingColorsAction_Color
    (0, d_tileset->getRoadColor ());
  umgr->add (action);
  d_tileset->setRoadColor(road_colorbutton->get_rgba());
  update ();
  d_changed = true;
}

void TilesetSmallmapBuildingColorsDialog::on_ruin_color_chosen()
{
  TileSetSmallmapBuildingColorsAction_Color *action = 
    new TileSetSmallmapBuildingColorsAction_Color
    (1, d_tileset->getRuinColor ());
  umgr->add (action);
  d_tileset->setRuinColor(ruin_colorbutton->get_rgba());
  update ();
  d_changed = true;
}

void TilesetSmallmapBuildingColorsDialog::on_temple_color_chosen()
{
  TileSetSmallmapBuildingColorsAction_Color *action = 
    new TileSetSmallmapBuildingColorsAction_Color
    (2, d_tileset->getTempleColor ());
  umgr->add (action);
  d_tileset->setTempleColor(temple_colorbutton->get_rgba());
  update ();
  d_changed = true;
}

void TilesetSmallmapBuildingColorsDialog::on_undo_activated ()
{
  umgr->undo ();
  if (umgr->undoEmpty ())
    d_changed = false;
  update ();
  return;
}

void TilesetSmallmapBuildingColorsDialog::on_redo_activated ()
{
  umgr->redo ();
  d_changed = true;
  update ();
}

void TilesetSmallmapBuildingColorsDialog::update ()
{
  disconnect_signals ();
  road_colorbutton->set_rgba (d_tileset->getRoadColor ());
  ruin_colorbutton->set_rgba (d_tileset->getRuinColor ());
  temple_colorbutton->set_rgba (d_tileset->getTempleColor ());
  connect_signals ();
}

void TilesetSmallmapBuildingColorsDialog::connect_signals ()
{
  road_colorbutton->signal_color_set().connect (method(on_road_color_chosen));
  temple_colorbutton->signal_color_set().connect (method(on_temple_color_chosen));
  ruin_colorbutton->signal_color_set().connect (method(on_ruin_color_chosen));
}

void TilesetSmallmapBuildingColorsDialog::disconnect_signals ()
{
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}

UndoAction *TilesetSmallmapBuildingColorsDialog::executeAction (UndoAction *action2)
{
  TileSetSmallmapBuildingColorsAction *action =
    dynamic_cast<TileSetSmallmapBuildingColorsAction*>(action2);
  UndoAction *out = NULL;

  switch (action->getType ())
    {
    case TileSetSmallmapBuildingColorsAction::CHANGE_COLOR:
        {
          TileSetSmallmapBuildingColorsAction_Color *a =
            dynamic_cast<TileSetSmallmapBuildingColorsAction_Color*>(action);
          Gdk::RGBA color;
          switch (a->getBuildingTypeId ())
            {
            case 0: color = road_colorbutton->get_rgba ();
              break;
            case 1: color = ruin_colorbutton->get_rgba ();
              break;
            default:
            case 2: color = temple_colorbutton->get_rgba ();
              break;
            }
          out = new TileSetSmallmapBuildingColorsAction_Color
            (a->getBuildingTypeId (), color);
          switch (a->getBuildingTypeId ())
            {
            case 0: d_tileset->setRoadColor (a->getColor());
              break;
            case 1: d_tileset->setRuinColor (a->getColor ());
              break;
            default:
            case 2: d_tileset->setTempleColor (a->getColor ());
              break;
            }
        }
      break;
    }
  return out;
}
