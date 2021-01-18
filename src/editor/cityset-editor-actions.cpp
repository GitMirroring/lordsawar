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

#include "cityset-editor-actions.h"
#include "cityset.h"
#include "File.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<< x << std::endl<<std::flush;}
#define debug(x)

CitySetEditorAction::CitySetEditorAction(Type type)
    :d_type(type)
{
}

//-----------------------------------------------------------------------------
//CitySetEditorAction_Properties

CitySetEditorAction_Properties::CitySetEditorAction_Properties(Glib::ustring n, Glib::ustring d, Glib::ustring c, Glib::ustring l, guint32 ts)
 :CitySetEditorAction(CitySetEditorAction::CHANGE_PROPERTIES), d_name (n),
    d_desc (d), d_copyright (c), d_license (l), d_tile_size (ts)
{
}

//-----------------------------------------------------------------------------
//CitySetEditorAction_Save

CitySetEditorAction_Save::CitySetEditorAction_Save(Cityset *c, Type t)
 :CitySetEditorAction (t)
{
  d_cityset = new Cityset (*c);

  d_filename = File::get_tmp_file () + CITYSET_EXT;
  c->save (d_filename, CITYSET_EXT);
}

CitySetEditorAction_Save::~CitySetEditorAction_Save ()
{
  File::erase (d_filename);
  delete d_cityset;
}

//-----------------------------------------------------------------------------
//CitySetEditorAction_AddImage

CitySetEditorAction_AddImage::CitySetEditorAction_AddImage(Cityset *c)
 :CitySetEditorAction_Save (c, CitySetEditorAction::ADD_IMAGE)
{
}

//-----------------------------------------------------------------------------
//CitySetEditorAction_ClearImage

CitySetEditorAction_ClearImage::CitySetEditorAction_ClearImage(Cityset *c)
 :CitySetEditorAction_Save (c, CitySetEditorAction::CLEAR_IMAGE)
{
}

//-----------------------------------------------------------------------------
//CitySetEditorAction_CityWidth

CitySetEditorAction_CityWidth::CitySetEditorAction_CityWidth(guint32 w)
 :CitySetEditorAction (CitySetEditorAction::CITY_TILE_WIDTH), d_city_width (w)
{
}

//-----------------------------------------------------------------------------
//CitySetEditorAction_RuinWidth

CitySetEditorAction_RuinWidth::CitySetEditorAction_RuinWidth(guint32 w)
 :CitySetEditorAction (CitySetEditorAction::RUIN_TILE_WIDTH), d_ruin_width (w)
{
}

//-----------------------------------------------------------------------------
//CitySetEditorAction_TempleWidth

CitySetEditorAction_TempleWidth::CitySetEditorAction_TempleWidth(guint32 w)
 :CitySetEditorAction (CitySetEditorAction::TEMPLE_TILE_WIDTH),
    d_temple_width (w)
{
}
