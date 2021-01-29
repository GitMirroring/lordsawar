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

#pragma once
#ifndef TILESET_SMALLMAP_BUILDING_COLORS_ACTIONS_H
#define TILESET_SMALLMAP_BUILDING_COLORS_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"

class TileSetSmallmapBuildingColorsAction: public UndoAction
{
    public:

        enum Type {
                CHANGE_COLOR = 1,
        };

	//! Default constructor.
        TileSetSmallmapBuildingColorsAction(Type type,
                              UndoAction::AggregateType a = UndoAction::AGGREGATE_NONE)
          : UndoAction (a), d_type(type) {}

        Type getType() const {return d_type;}

    protected:

        Type d_type;
};

class TileSetSmallmapBuildingColorsAction_Color: public TileSetSmallmapBuildingColorsAction
{
    public:
        TileSetSmallmapBuildingColorsAction_Color (guint32 id, Gdk::RGBA c)
          : TileSetSmallmapBuildingColorsAction(TileSetSmallmapBuildingColorsAction::CHANGE_COLOR),
          d_bldg_type_id (id), d_color (c) {}
        ~TileSetSmallmapBuildingColorsAction_Color () {}

        Glib::ustring getActionName () const {return "Color";}

        guint32 getBuildingTypeId () const {return d_bldg_type_id;}
        Gdk::RGBA getColor () const {return d_color;}

    private:
        guint32 d_bldg_type_id;
        Gdk::RGBA d_color;
};

#endif //TILESET_SMALLMAP_BUILDING_COLORS_ACTIONS_H
