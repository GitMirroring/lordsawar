// Copyright (C) 2001, 2003 Michael Bartl
// Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2006 Andrea Paternesi
// Copyright (C) 2007, 2008, 2009, 2014, 2015, 2021 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
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

#include "temple.h"
#include "GameMap.h"
#include "QuestsManager.h"
#include "stack.h"
#include "xmlhelper.h"

Glib::ustring Temple::d_tag = "temple";

Temple::Temple(Vector<int> pos, guint32 width, Glib::ustring name, int type)
:NamedLocation(pos, width, name,
	       name + _(" can bless your armies or give you quests.")), 
    d_type(type)
{
    //mark the location on the game map as occupied by a temple
    for (unsigned int i = 0; i < getSize(); i++)
      for (unsigned int j = 0; j < getSize(); j++)
	{
	  Vector<int> p = getPos() + Vector<int>(i, j);
	  GameMap::getInstance()->getTile(p)->setBuilding(Maptile::TEMPLE);
	}
}

Temple::Temple(XML_Helper* helper, guint32 width)
    :NamedLocation(helper, width)
{
    //mark the location on the game map as occupied by a temple
    helper->getData(d_type, "type");
    for (unsigned int i = 0; i < getSize(); i++)
      for (unsigned int j = 0; j < getSize(); j++)
	{
	  Vector<int> pos = getPos() + Vector<int>(i, j);
	  GameMap::getInstance()->getTile(pos)->setBuilding(Maptile::TEMPLE);
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

    retval &= helper->openTag(Temple::d_tag);
    retval &= helper->saveData("id", d_id);
    retval &= helper->saveData("x", getPos().x);
    retval &= helper->saveData("y", getPos().y);
    retval &= helper->saveData("name", getName(false));
    retval &= helper->saveData("description", getDescription());
    retval &= helper->saveData("type", d_type);
    retval &= helper->closeTag();
    
    return retval;
}
