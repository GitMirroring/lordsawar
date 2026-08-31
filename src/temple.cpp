//  Copyright (C) 2001, 2003 Michael Bartl
//  Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
//  Copyright (C) 2006 Andrea Paternesi
//  Copyright (C) 2007, 2008, 2009, 2014, 2015, 2021, 2026 Ben Asselstine
//  Copyright (C) 2007 Ole Laursen
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

#include "temple.h"
#include "game-map.h"
#include "quest-manager.h"
#include "stack.h"
#include "xml-helper.h"

Glib::ustring Temple::d_tag = "temple";

Temple::Temple(Vector<int> pos, guint32 width, Glib::ustring name, Type type)
:NamedLocation(pos, width, name,
	       name + _(" can bless your armies or give you quests.")), 
    d_type(type)
{
    //mark the location on the game map as occupied by a temple
    for (unsigned int i = 0; i < getSize(); i++)
      for (unsigned int j = 0; j < getSize(); j++)
	{
	  Vector<int> p = getPos() + Vector<int>(i, j);
	  GameMap::instance()->getTile(p)->setBuilding(Maptile::TEMPLE);
	}
}

Temple::Temple(XML_Helper* helper, guint32 width)
    :NamedLocation(helper, width)
{
  //mark the location on the game map as occupied by a temple
  Glib::ustring type_str;
  helper->get(type_str, "type");
  d_type = templeTypeFromString(type_str);

  for (unsigned int i = 0; i < getSize(); i++)
    for (unsigned int j = 0; j < getSize(); j++)
      {
        Vector<int> pos = getPos() + Vector<int>(i, j);
        GameMap::instance()->getTile(pos)->setBuilding(Maptile::TEMPLE);
      }
}

Temple::Temple(const Temple& t, bool sync_id)
  :NamedLocation(t, sync_id), d_type(t.d_type)
{
}

Temple::Temple(const Temple& t, Vector<int> pos)
  :NamedLocation(t, pos), d_type(t.d_type)
{
}

bool Temple::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->open_tag(Temple::d_tag);
    retval &= helper->save("id", d_id);
    retval &= helper->save("x", getPos().x);
    retval &= helper->save("y", getPos().y);
    retval &= helper->save("name", getName(false));
    retval &= helper->save("description", getDescription());
    Glib::ustring type_str = templeTypeToString(d_type);
    retval &= helper->save("type", type_str);
    retval &= helper->close_tag();
    
    return retval;
}

Glib::ustring Temple::templeTypeToString(const Temple::Type type)
{
  switch (type)
    {
      case Temple::TEMPLE: return "Temple::TEMPLE";
      case Temple::HENGE: return "Temple::HENGE";
    }
  return "Temple::TEMPLE";
}

Temple::Type Temple::templeTypeFromString(const Glib::ustring str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return Temple::Type(atoi(str.c_str()));
  if (str == "Temple::TEMPLE") return Temple::TEMPLE;
  else if (str == "Temple::HENGE") return Temple::HENGE;
  return Temple::TEMPLE;
}

