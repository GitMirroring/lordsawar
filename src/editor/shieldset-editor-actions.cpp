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

#include "shieldset-editor-actions.h"
#include "shieldset.h"
#include "File.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<< x << std::endl<<std::flush;}
#define debug(x)

ShieldSetEditorAction::ShieldSetEditorAction(Type type)
    :d_type(type)
{
}

ShieldSetEditorAction::ShieldSetEditorAction(const ShieldSetEditorAction &action)
:d_type(action.d_type)
{

}

//-----------------------------------------------------------------------------
//ShieldSetEditorAction_Color

ShieldSetEditorAction_Color::ShieldSetEditorAction_Color(guint32 id, Gdk::RGBA c)
 :ShieldSetEditorAction(ShieldSetEditorAction::CHANGE_COLOR), d_player_id (id),
    d_color (c)
{
}

ShieldSetEditorAction_Color::ShieldSetEditorAction_Color (const ShieldSetEditorAction_Color &a)
:ShieldSetEditorAction(a), d_player_id(a.d_player_id), d_color(a.d_color)
{
}

//-----------------------------------------------------------------------------
//ShieldSetEditorAction_Properties

ShieldSetEditorAction_Properties::ShieldSetEditorAction_Properties(Glib::ustring n, Glib::ustring d, Glib::ustring c, Glib::ustring l)
 :ShieldSetEditorAction(ShieldSetEditorAction::CHANGE_PROPERTIES), d_name (n),
    d_desc (d), d_copyright (c), d_license (l)
{
}

ShieldSetEditorAction_Properties::ShieldSetEditorAction_Properties (const ShieldSetEditorAction_Properties &a)
:ShieldSetEditorAction(a), d_name (a.d_name), d_desc(a.d_desc),
    d_copyright(a.d_copyright), d_license (a.d_license)
{
}

//-----------------------------------------------------------------------------
//ShieldSetEditorAction_Save

ShieldSetEditorAction_Save::ShieldSetEditorAction_Save(Shieldset *s, Type t)
 :ShieldSetEditorAction (t)
{
  d_filename = File::get_tmp_file () + SHIELDSET_EXT;
  s->save (d_filename, SHIELDSET_EXT);
}

ShieldSetEditorAction_Save::ShieldSetEditorAction_Save (const ShieldSetEditorAction_Save &a)
:ShieldSetEditorAction(a)
{
  d_filename = File::get_tmp_file () + SHIELDSET_EXT;
  File::copy (a.d_filename, d_filename);
}

ShieldSetEditorAction_Save::~ShieldSetEditorAction_Save ()
{
  File::erase (d_filename);
}

//-----------------------------------------------------------------------------
//ShieldSetEditorAction_WhiteDown

ShieldSetEditorAction_WhiteDown::ShieldSetEditorAction_WhiteDown(Shieldset *s)
 :ShieldSetEditorAction_Save(s, ShieldSetEditorAction::COPY_WHITE_DOWN)
{
}

ShieldSetEditorAction_WhiteDown::ShieldSetEditorAction_WhiteDown (const ShieldSetEditorAction_WhiteDown &a)
:ShieldSetEditorAction_Save(a)
{
}

//-----------------------------------------------------------------------------
//ShieldSetEditorAction_AddImage

ShieldSetEditorAction_AddImage::ShieldSetEditorAction_AddImage(Shieldset *s)
 :ShieldSetEditorAction_Save(s, ShieldSetEditorAction::ADD_IMAGE)
{
}

ShieldSetEditorAction_AddImage::ShieldSetEditorAction_AddImage (const ShieldSetEditorAction_AddImage &a)
:ShieldSetEditorAction_Save(a)
{
}

//-----------------------------------------------------------------------------
//ShieldSetEditorAction_ClearImage

ShieldSetEditorAction_ClearImage::ShieldSetEditorAction_ClearImage(Shieldset *s)
 :ShieldSetEditorAction_Save(s, ShieldSetEditorAction::CLEAR_IMAGE)
{
}

ShieldSetEditorAction_ClearImage::ShieldSetEditorAction_ClearImage (const ShieldSetEditorAction_ClearImage &a)
:ShieldSetEditorAction_Save(a)
{
}
