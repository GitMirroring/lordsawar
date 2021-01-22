// Copyright (C) 2001, 2003 Michael Bartl
// Copyright (C) 2002, 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2007, 2008, 2009, 2014, 2015, 2017, 2020, 2021 Ben Asselstine
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

#include "ruin.h"
#include "playerlist.h"
#include "GameMap.h"
#include "rewardlist.h"
#include "Sage.h"
#include "keeper.h"
#include "xmlhelper.h"
#include "rnd.h"

Glib::ustring Ruin::d_tag = "ruin";

Ruin::Ruin(Vector<int> pos, guint32 width, Glib::ustring name, int type, Keeper* occupant, bool searched, bool hidden, Player *owner, bool sage)
:NamedLocation(pos, width, name,
	        name + _(" is inhabited by monsters and full of treasure!")), 
    d_searched(searched), 
    d_type(type), d_occupant(occupant), d_hidden(hidden), d_owner(owner), 
    d_sage(sage)
{
    d_owner = NULL;
    d_reward = NULL;
    //mark the location as being occupied by a ruin on the map
    for (unsigned int i = 0; i < getSize(); i++)
      for (unsigned int j = 0; j < getSize(); j++)
	{
	  Vector<int> p = getPos() + Vector<int>(i, j);
	  GameMap::getInstance()->getTile(p)->setBuilding(Maptile::RUIN);
	}
}

Ruin::Ruin(const Ruin& ruin, bool sync_id)
    :NamedLocation(ruin, sync_id), sigc::trackable(ruin),
    d_searched(ruin.d_searched), d_type(ruin.d_type), d_hidden(ruin.d_hidden),
    d_owner(ruin.d_owner), d_sage(ruin.d_sage)
{
  if (ruin.d_occupant)
    d_occupant = new Keeper(*ruin.d_occupant);
  else
    d_occupant = NULL;
  if (ruin.d_reward)
    d_reward = Reward::copy (ruin.d_reward);
  else
    d_reward = NULL;
}

Ruin::Ruin(const Ruin& ruin, Vector<int> pos)
    :NamedLocation(ruin, pos), d_searched(ruin.d_searched), 
    d_type(ruin.d_type), d_hidden(ruin.d_hidden), d_owner(ruin.d_owner),
    d_sage(ruin.d_sage)
{
  if (ruin.d_occupant)
    d_occupant = new Keeper(*ruin.d_occupant);
  else
    d_occupant = NULL;
  if (ruin.d_reward)
    d_reward = Reward::copy (ruin.d_reward);
  else
    d_reward = NULL;
}

Ruin::Ruin(XML_Helper* helper, guint32 width)
    :NamedLocation(helper, width), d_type(0), d_occupant(0), 
    d_hidden(0), d_owner(0), d_sage(0), d_reward(0)
{
  helper->registerTag(Keeper::d_tag, sigc::mem_fun(this, &Ruin::load));
  guint32 ui;
  Glib::ustring type_str;
  helper->getData(type_str, "type");
  d_type = ruinTypeFromString(type_str);
  helper->getData(d_searched, "searched");
  helper->getData(d_sage, "sage");
  helper->getData(d_hidden, "hidden");
  if (d_hidden || d_searched)
    {
      helper->getData(ui, "owner");
      if (ui != MAX_PLAYERS)
        d_owner = Playerlist::getInstance()->getPlayer(ui);
      else
        d_owner = NULL;
    }
  else
    d_owner = NULL;

  //mark the location as being occupied by a ruin on the map
  for (unsigned int i = 0; i < getSize(); i++)
    for (unsigned int j = 0; j < getSize(); j++)
      {
        Vector<int> pos = getPos() + Vector<int>(i, j);
        GameMap::getInstance()->getTile(pos)->setBuilding(Maptile::RUIN);
      }
}

Ruin::~Ruin()
{
  if (d_reward)
    delete d_reward;
  if (d_occupant)
    delete d_occupant;
}

bool Ruin::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag(Ruin::d_tag);
  retval &= helper->saveData("id", d_id);
  retval &= helper->saveData("x", getPos().x);
  retval &= helper->saveData("y", getPos().y);
  retval &= helper->saveData("name", getName(false));
  retval &= helper->saveData("description", getDescription());
  Glib::ustring type_str = ruinTypeToString(Ruin::Type(d_type));
  retval &= helper->saveData("type", type_str);
  retval &= helper->saveData("searched", d_searched);
  retval &= helper->saveData("sage", d_sage);
  retval &= helper->saveData("hidden", d_hidden);
  if (d_owner != NULL)
    retval &= helper->saveData("owner", d_owner->getId());
  else
    retval &= helper->saveData("owner", MAX_PLAYERS);
  if (d_occupant)
    retval &= d_occupant->save(helper);
  if (d_sage == false && d_reward)
    retval &= d_reward->save(helper);
  retval &= helper->closeTag();

  return retval;
}

bool Ruin::load(Glib::ustring tag, XML_Helper* helper)
{
  if (tag == Reward::d_tag)
    {
	guint32 t;
	Glib::ustring type_str;
	helper->getData(type_str, "type");
	t = Reward::rewardTypeFromString(type_str);
	switch (t)
	  {
	  case  Reward::GOLD:
	    d_reward = new Reward_Gold(helper); break;
	  case  Reward::ALLIES:
	    d_reward = new Reward_Allies(helper); break;
	  case Reward::ITEM:
	    d_reward = new Reward_Item(helper); break;
	  case Reward::RUIN:
	    d_reward = new Reward_Ruin(helper); break;
	  case Reward::MAP:
	    d_reward = new Reward_Map(helper); break;
	  }
	return true;
    }

  if (tag == Keeper::d_tag)
    {
      Keeper* k = new Keeper(helper);
      d_occupant = k;
      return true;
    }

  return false;
}

void Ruin::populateWithRandomReward()
{
  Reward *reward = Reward::createRandomReward(true, false);
  setReward (reward);
}

Glib::ustring Ruin::ruinTypeToString(const Ruin::Type type)
{
  switch (type)
    {
      case Ruin::RUIN: return "Ruin::RUIN";
      case Ruin::STRONGHOLD: return "Ruin::STRONGHOLD";
      case Ruin::SAGE: return "Ruin::SAGE";
    }
  return "Ruin::RUIN";
}

Ruin::Type Ruin::ruinTypeFromString(const Glib::ustring str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return Ruin::Type(atoi(str.c_str()));
  if (str == "Ruin::RUIN") return Ruin::RUIN;
  else if (str == "Ruin::STRONGHOLD") return Ruin::STRONGHOLD;
  else if (str == "Ruin::SAGE") return Ruin::SAGE;
  return Ruin::RUIN;
}

Sage* Ruin::generateSage() const
{
  return new Sage();
}
	
void Ruin::setSage(bool sage)
{
  d_sage = sage;
  if (sage)
    d_type = SAGE;
  else
    d_type = RUIN;
}

void Ruin::setOccupant(Keeper* occupant)
{
  if (d_occupant)
    delete d_occupant;
  d_occupant = occupant;
}

void Ruin::setReward (Reward *reward)
{
  if (d_reward)
    delete d_reward;
  d_reward = reward;
}

Reward *Ruin::takeReward()
{
  Reward *reward = d_reward;
  d_reward = NULL;
  return reward;
}

void Ruin::clearOccupant()
{
  //the idea here is that the occupant has been killed in a ruinfight.
  //in cleaning up from that fight, the stack gets deleted.
  //and now we need to make sure this pointer isn't hanging around.
  d_occupant = NULL;
}
        
// End of file
