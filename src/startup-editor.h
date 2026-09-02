//  Copyright (C) 2026 Ben Asselstine
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
//  Foundation, Inc., 31 Milk Street #960789, Boston, MA 02196, USA.

#ifndef STARTUP_EDITOR_H
#define STARTUP_EDITOR_H
#include "lw.h"

class Cityset;
class Shieldset;
class Tileset;
class Armyset;

class StartupEditor
{
  friend class Startup;
public:


  StartupEditor ()
    {
    }

  ~StartupEditor ()
    {
    }

  void editor (std::string filename);
  void cityset_editor (std::string filename);
  void shieldset_editor (std::string filename);
  void armyset_editor (std::string shieldset_theme, std::string filename);
  void tileset_editor (std::string shieldset_theme, std::string filename);

private:

  void cityset_editor (Cityset *cityset);
  void shieldset_editor (Shieldset *shieldset);
  void tileset_editor (Shieldset *shieldset, Tileset *tileset);
  void armyset_editor (Shieldset *shieldset, Armyset *armyset);

  Shieldset *load_shieldset_from_option (std::string shieldset_theme);
};
#endif
