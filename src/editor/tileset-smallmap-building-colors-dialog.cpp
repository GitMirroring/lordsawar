//  Copyright (C) 2010, 2014, 2020 Ben Asselstine
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

#define method(x) sigc::mem_fun(*this, &TilesetSmallmapBuildingColorsDialog::x)

TilesetSmallmapBuildingColorsDialog::TilesetSmallmapBuildingColorsDialog(Gtk::Window &parent, Tileset *tileset)
 : LwEditorDialog(parent, "tileset-smallmap-building-colors-dialog.ui")
{
  d_tileset = tileset;

  xml->get_widget("road_colorbutton", road_colorbutton);
  road_colorbutton->set_rgba(tileset->getRoadColor());
  road_colorbutton->signal_color_set().connect (method(on_road_color_chosen));
  xml->get_widget("ruin_colorbutton", ruin_colorbutton);
  ruin_colorbutton->set_rgba(tileset->getRuinColor());
  ruin_colorbutton->signal_color_set().connect (method(on_ruin_color_chosen));
  xml->get_widget("temple_colorbutton", temple_colorbutton);
  temple_colorbutton->set_rgba(tileset->getTempleColor());
  temple_colorbutton->signal_color_set().connect (method(on_temple_color_chosen));
}

void TilesetSmallmapBuildingColorsDialog::on_road_color_chosen()
{
  d_tileset->setRoadColor(road_colorbutton->get_rgba());
  d_changed = true;
}

void TilesetSmallmapBuildingColorsDialog::on_ruin_color_chosen()
{
  d_tileset->setRuinColor(ruin_colorbutton->get_rgba());
  d_changed = true;
}

void TilesetSmallmapBuildingColorsDialog::on_temple_color_chosen()
{
  d_tileset->setTempleColor(temple_colorbutton->get_rgba());
  d_changed = true;
}
