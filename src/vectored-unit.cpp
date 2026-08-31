//  Copyright (C) 2007, 2008, 2009, 2014, 2015, 2021, 2026 Ben Asselstine
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

#include "vectored-unit.h"
#include <xml-helper.h>

#include "army-set-list.h"
#include "player-list.h"
#include "army.h"
#include "city.h"
#include "game-map.h"
#include "action.h"
#include "map-backpack.h"
#include "game-scenario.h"

Glib::ustring VectoredUnit::d_tag = "vectoredunit";

VectoredUnit::VectoredUnit(Vector<int> pos, Vector<int> dest, ArmyProdBase *army, int duration, Player *player)
    :OwnerId(player), LocationBox(pos), d_destination(dest), 
    d_duration(duration)
{
  if (army)
    d_army = new ArmyProdBase(*army);
  else
    d_army = NULL;
}

VectoredUnit::VectoredUnit(const VectoredUnit& v)
    :OwnerId(v), LocationBox(v), sigc::trackable(v),
    d_destination(v.d_destination), d_duration(v.d_duration)
{
  if (v.d_army)
    d_army = new ArmyProdBase(*v.d_army);
  else
    d_army = NULL;
}

VectoredUnit::VectoredUnit(XML_Helper* helper)
    :OwnerId(helper), LocationBox(helper), d_army(NULL)
{
    helper->get(d_duration, "duration");
    helper->get(d_destination.x, "dest_x");
    helper->get(d_destination.y, "dest_y");
    //army is loaded via callback in vectoredunitlist
}

VectoredUnit::~VectoredUnit()
{
  if (d_army)
    delete d_army;
}

bool VectoredUnit::save(XML_Helper* helper) const
{
    bool retval = true;
    Glib::ustring name = "";

    retval &= helper->open_tag(VectoredUnit::d_tag);
    retval &= helper->save("x", getPos().x);
    retval &= helper->save("y", getPos().y);
    retval &= helper->save("name", name);
    retval &= helper->save("duration", d_duration);
    retval &= helper->save("dest_x", d_destination.x);
    retval &= helper->save("dest_y", d_destination.y);
    if (getOwner ())
        retval &= helper->save("owner", d_owner_id);
    else
        retval &= helper->save("owner", -1);
    retval &= d_army->save(helper);
    retval &= helper->close_tag();

    return retval;
}

Army *VectoredUnit::armyArrives(Stack *& stack) const
{
  City *dest;
  // drop it in the destination city!
  dest = GameMap::getCity(d_destination);
  if (!dest)
    {
      if (d_destination == Vector<int>(-1,-1))
	{
	  printf ("destination is -1,-1??? why?\n");
	return NULL;
	}
      printf ("uhh... no city at %d,%d?\n", d_destination.x, d_destination.y);
      Maptile *tile = GameMap::instance()->getTile(d_destination);
      if (tile)
	{
	  if (tile->getBackpack()->getPlantedItem(getOwner ()))
	    {
	      //army arrives on a planted standard
	      Army *a = new Army(*d_army, getOwner ());
	      LocationBox loc = LocationBox(d_destination);
              stack = GameMap::instance()->addArmy(d_destination, a);
	      return a;
	    }
	}
    }
  else
    {
      if (!dest->isBurnt() && dest->getOwner() == getOwner ())
	{
	  //army arrives in a city
	  Army *a = new Army(*d_army, getOwner ());
          stack = GameMap::instance()->addArmy(d_destination, a);
	  return a;
	}
      printf ("destination city is owned by `%s', but the vectored unit is owned by `%s'\n", dest->getOwner()->getName().c_str(), getOwner ()->getName().c_str());
    }
  return NULL;
}

bool VectoredUnit::nextTurn()
{
  d_duration--;
  if (d_duration == 0)
    return getOwner ()->vectoredUnitArrives(this);
  return false;
}

int VectoredUnit::get_travel_turns (Vector<int> src, Vector<int> dest)
{
  int turns = MAX_TURNS_FOR_VECTORING;
  switch (GameScenario::s_vectoring_mode)
    {
    case GameParameters::VECTORING_ALWAYS_TWO_TURNS:
      turns = MAX_TURNS_FOR_VECTORING;
      break;
    case GameParameters::VECTORING_VARIABLE_TURNS:
      int d = dist(dest, src);
      double r = (double) d /
        (double)std::max(GameMap::getWidth(), GameMap::getHeight());
      turns = (4 * r) + 1;
      break;
    }
  return turns;
}
