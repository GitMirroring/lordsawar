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

#include "tileset-editor-actions.h"
#include "tileset.h"
#include "File.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<< x << std::endl<<std::flush;}
#define debug(x)

//-----------------------------------------------------------------------------
//TileSetEditorAction_Save

TileSetEditorAction_Save::TileSetEditorAction_Save(Tileset *s, Type t)
 :TileSetEditorAction (t)
{
  d_filename = File::get_tmp_file () + TILESET_EXT;
  s->save (d_filename, TILESET_EXT);
}

TileSetEditorAction_Save::~TileSetEditorAction_Save ()
{
  File::erase (d_filename);
}
