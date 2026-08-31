//  Copyright (C) 2007, 2008, 2014, 2021, 2026 Ben Asselstine
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

#include "port.h"
#include "game-map.h"
#include "xml-helper.h"

Glib::ustring Port::d_tag = "port";

Port::Port(Vector<int> pos)
  :Location(pos)
{
    //mark the location on the game map as occupied by a port
    GameMap::instance()->getTile(getPos())->setBuilding(Maptile::PORT);
}

Port::Port(XML_Helper* helper)
    :Location(helper)
{
    //mark the location on the game map as occupied by a port
    GameMap::instance()->getTile(getPos())->setBuilding(Maptile::PORT);
}

Port::Port(const Port& s, bool sync_id)
  :Location(s, sync_id)
{
}

Port::Port(const Port& s, Vector<int> pos)
  :Location(s, pos)
{
}

bool Port::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->open_tag(Port::d_tag);
    retval &= helper->save("id", d_id);
    retval &= helper->save("x", getPos().x);
    retval &= helper->save("y", getPos().y);
    retval &= helper->close_tag();
    
    return retval;
}
