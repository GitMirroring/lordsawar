//  Copyright (C) 2009, 2014, 2021 Ben Asselstine
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

#include "OwnerId.h"
#include "playerlist.h"
#include "xmlhelper.h"
#include "player.h"

OwnerId::OwnerId()
 : d_owner_id (0), owner_id_set (false)
{
}

OwnerId::OwnerId(Player *p)
  :d_owner_id(0), owner_id_set (false)
{
  if (p)
    {
      d_owner_id = p->getId ();
      owner_id_set = true;
    }
}

OwnerId::OwnerId(guint32 owner)
  :d_owner_id(owner), owner_id_set (true)
{
}

OwnerId::OwnerId(const OwnerId& own)
  :d_owner_id(own.d_owner_id), owner_id_set(own.owner_id_set)
{
}

OwnerId OwnerId::load(XML_Helper *helper)
{
  OwnerId result;
  int i = -1;
  helper->getData(i, "owner");
  if (i == -1)
    result.setOwnerId(0);
  else
    result.setOwnerId(i);
  return result;
}

OwnerId::OwnerId(XML_Helper* helper)
{
  if (!helper)
    return;
  OwnerId result = load(helper);
  d_owner_id = result.d_owner_id;
  owner_id_set = true;
}

Player * OwnerId::getOwner() const
{
  if (owner_id_set == false)
    return NULL;

 return Playerlist::getInstance()->getPlayer(d_owner_id); 
}
  
bool OwnerId::save(XML_Helper *helper) const
{
  return helper->saveData("owner", d_owner_id);
}

bool OwnerId::isFriend(Player *opponent) const
{
  return getOwner () == opponent;
}

void OwnerId::setOwner (Player *p)
{
  if (p)
    {
      d_owner_id = p->getId ();
      owner_id_set = true;
    }
}
