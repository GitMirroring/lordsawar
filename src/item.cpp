//  Copyright (C) 2004, 2005, 2006 Ulf Lorenz
//  Copyright (C) 2004 Andrea Paternesi
//  Copyright (C) 2007, 2008, 2009, 2010, 2014, 2021, 2026 Ben Asselstine
//  Copyright (C) 2008 Ole Laursen
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

#include <sstream>
#include <map>
#include "item.h"
#include "item-proto.h"
#include "file.h"
#include "player-list.h"
#include "ucompose.hpp"
#include "map-tile.h"
#include "xml-helper.h"

Glib::ustring Item::d_tag = "item";

Item::Item(XML_Helper* helper)
	: ItemProto(helper), UniquelyIdentified(helper)
{
    
    helper->get(d_plantable, "plantable");
    if (d_plantable)
      {
        helper->get(d_plantable_owner_id, "plantable_owner");
        helper->get(d_planted, "planted");
        helper->get(d_plantable_orig_owner_id, "plantable_orig_owner");
      }
    else
      {
	d_plantable_owner_id = MAX_PLAYERS;
	d_plantable_orig_owner_id = d_plantable_owner_id;
	d_planted = false;
      }

    helper->get(d_type, "type");

}

Item::Item(Glib::ustring name, bool plantable, Player *plantable_owner)
 : ItemProto(name), UniquelyIdentified(), d_plantable (plantable),
    d_plantable_owner_id (MAX_PLAYERS), d_planted (false),
    d_plantable_orig_owner_id (d_plantable_owner_id), d_type (0)
{
  if (d_plantable)
    d_bonus = ItemProto::PLANT_TO_VECTOR;
  else
    d_bonus = 0;

  if (plantable_owner)
    d_plantable_owner_id = plantable_owner->getId();
  //std::cerr << "item created with id " << d_id << std::endl;
}

Item::Item(const Item& orig, bool sync_id)
:ItemProto(orig), UniquelyIdentified(orig, sync_id), 
    d_plantable(orig.d_plantable), 
    d_plantable_owner_id(orig.d_plantable_owner_id), d_planted(orig.d_planted),
    d_plantable_orig_owner_id(orig.d_plantable_orig_owner_id),
    d_type(orig.d_type)
{
}

Item::Item(const ItemProto &proto, guint32 type_id)
: ItemProto(proto), UniquelyIdentified(),
    d_plantable ((d_bonus & ItemProto::PLANT_TO_VECTOR) == ItemProto::PLANT_TO_VECTOR),
    d_plantable_owner_id (MAX_PLAYERS), d_planted (false),
    d_plantable_orig_owner_id (d_plantable_owner_id), d_type (type_id)
{
}

Item::~Item()
{
  if (d_unique)
    sdying.emit(this);
}

bool Item::save(XML_Helper* helper) const
{
  bool retval = true;

  // A template is never saved, so we assume this class is a real-life item
  retval &= helper->open_tag(Item::d_tag);
  retval &= saveContents(helper);
  retval &= helper->save("plantable", d_plantable);
  if (d_plantable)
    {
      retval &= helper->save("plantable_owner", d_plantable_owner_id);
      retval &= helper->save("planted", d_planted);
      retval &= helper->save("plantable_orig_owner", d_plantable_orig_owner_id);
    }
  retval &= helper->save("id", d_id);
  retval &= helper->save("type", d_type);
  retval &= helper->close_tag();

  return retval;
}

bool Item::use()
{
  if (d_uses_left)
    {
      d_uses_left--;
      if (d_uses_left)
        return false;
      else
        return true;
    }
  return true;
}
	
Player *Item::getPlantableOwner() const
{
  return Playerlist::instance()->get (d_plantable_owner_id);
}

Player *Item::getPlantableOriginalOwner() const
{
  return Playerlist::instance()->get (d_plantable_orig_owner_id);
}
