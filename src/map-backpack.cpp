//  Copyright (C) 2008, 2021, 2026 Ben Asselstine
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

#include "map-backpack.h"
#include "item.h"
#include "xml-helper.h"

Glib::ustring MapBackpack::d_mapbackpack_tag = "itemstack";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

MapBackpack::MapBackpack(Vector<int> pos, Player *p)
  :Backpack(), OwnerId (p), Immovable(pos), UniquelyIdentified((guint32)0)
{
}

MapBackpack::MapBackpack(const MapBackpack& object, bool sync_id)
  :Backpack(object), OwnerId (object), Immovable(object),
    UniquelyIdentified(object, sync_id)
{
}

MapBackpack::MapBackpack(XML_Helper* helper)
  :OwnerId (helper), Immovable(helper), UniquelyIdentified((guint32)0)
{
  helper->register_tag(Backpack::d_tag, sigc::mem_fun(*this, &MapBackpack::loadItem));
  helper->register_tag(Item::d_tag, sigc::mem_fun(*this, &MapBackpack::loadItem));
}

bool MapBackpack::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->open_tag(MapBackpack::d_mapbackpack_tag);
  retval &= OwnerId::save (helper);
  retval &= helper->save("x", getPos().x);
  retval &= helper->save("y", getPos().y);
  retval &= Backpack::save(helper);
  retval &= helper->close_tag();

  return retval;
}

Item *MapBackpack::getFirstPlantedItem()
{
  for (MapBackpack::iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getPlanted() == true)
	return *it;
    }
  return NULL;
}

Item *MapBackpack::getPlantedItem(Player *player)
{
  for (MapBackpack::iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getPlanted() == true &&
	  (*it)->getPlantableOwner() == player)
	return *it;
    }
  return NULL;
}

Item *MapBackpack::getPlantedItem()
{
  for (MapBackpack::iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getPlanted() == true)
	return *it;
    }
  return NULL;
}
