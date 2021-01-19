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
