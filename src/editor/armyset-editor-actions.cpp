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

#include "armyset-editor-actions.h"
#include "armyset.h"
#include "File.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<< x << std::endl<<std::flush;}
#define debug(x)

ArmySetEditorAction::ArmySetEditorAction(Type type)
    :d_type(type)
{
}

//-----------------------------------------------------------------------------
//ArmySetEditorAction_Properties

ArmySetEditorAction_Properties::ArmySetEditorAction_Properties(Glib::ustring n, Glib::ustring d, Glib::ustring c, Glib::ustring l, guint32 ts)
 :ArmySetEditorAction(ArmySetEditorAction::CHANGE_PROPERTIES), d_name (n),
    d_desc (d), d_copyright (c), d_license (l), d_tile_size (ts)
{
}

//-----------------------------------------------------------------------------
//ArmySetEditorAction_Save

ArmySetEditorAction_Save::ArmySetEditorAction_Save(Armyset *a, Type t)
 :ArmySetEditorAction (t)
{
  d_filename = File::get_tmp_file () + ARMYSET_EXT;
  a->save (d_filename, ARMYSET_EXT);
}

ArmySetEditorAction_Save::~ArmySetEditorAction_Save ()
{
  File::erase (d_filename);
}
