//  Copyright (C) 2007, 2008, 2009, 2014, 2021 Ben Asselstine
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

#include <assert.h>
#include <sigc++/functors/mem_fun.h>

#include "vectoredunitlist.h"
#include "vectoredunit.h"
#include "city.h"
#include "xmlhelper.h"
#include "player.h"
#include "GameMap.h"

Glib::ustring VectoredUnitlist::d_tag = "vectoredunitlist";
//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

VectoredUnitlist* VectoredUnitlist::s_instance = 0;

VectoredUnitlist* VectoredUnitlist::getInstance()
{
    if (s_instance == 0)
        s_instance = new VectoredUnitlist();

    return s_instance;
}

VectoredUnitlist* VectoredUnitlist::getInstance(XML_Helper* helper)
{
    if (s_instance)
        deleteInstance();

    s_instance = new VectoredUnitlist(helper);
    return s_instance;
}

void VectoredUnitlist::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

VectoredUnitlist::VectoredUnitlist()
{
}

VectoredUnitlist::VectoredUnitlist (const VectoredUnitlist &v)
 : std::list<VectoredUnit*> (), sigc::trackable (v)
{
  for (auto vectoredunit: v)
    push_back (new VectoredUnit (*vectoredunit));
}

VectoredUnitlist::~VectoredUnitlist()
{
  for (VectoredUnitlist::iterator it = begin(); it != end(); ++it)
    delete *it;
}

VectoredUnitlist::VectoredUnitlist(XML_Helper* helper)
{
    helper->registerTag(VectoredUnit::d_tag, sigc::mem_fun(this, &VectoredUnitlist::load));
    helper->registerTag(ArmyProdBase::d_tag, sigc::mem_fun(this, &VectoredUnitlist::load));
}

bool VectoredUnitlist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(VectoredUnitlist::d_tag);

    for (VectoredUnitlist::const_iterator it = begin(); it != end(); ++it)
        retval &= (*it)->save(helper);
    
    retval &= helper->closeTag();

    return retval;
}

bool VectoredUnitlist::load(Glib::ustring tag, XML_Helper* helper)
{
  if (tag == VectoredUnit::d_tag)
    {
      VectoredUnit *r = new VectoredUnit(helper);
      push_back(r);
      return true;
    }

  if (tag == ArmyProdBase::d_tag)
    {
      VectoredUnit *vectoredunit = back();
      vectoredunit->setArmy(new ArmyProdBase (helper));
      return true;
    }

    return false;
}

void VectoredUnitlist::nextTurn(Player* p)
{
  debug("next_turn(" <<p->getName() <<")");

  for (VectoredUnitlist::iterator it = begin(); it != end(); ++it)
    {
      City *c = GameMap::getCity((*it)->getPos());
      if (c)
	{
	  if (c->getOwner() == p)
	    (*it)->nextTurn();
	}
      else //must be a standard
	(*it)->nextTurn();
    }

  for (VectoredUnitlist::iterator it = begin(); it != end();)
    {
      if ((*it)->getDuration() <= 0)
	{
	  it = flErase(it);
	  continue;
	}
      ++it;
    }
}

bool VectoredUnitlist::removeVectoredUnitsGoingTo(Vector<int> pos)
{
  bool found = false;
  printf ("trying to remove vectored units going to %d,%d\n", pos.x, pos.y);
  for (VectoredUnitlist::iterator it = begin(); it != end();)
    {
      if ((*it)->getDestination() == pos)
	{
	  found = true;
	  it = flErase(it);
	  continue;
	}
      ++it;
    }
  return found;
}

bool VectoredUnitlist::removeVectoredUnitsComingFrom(Vector<int> pos)
{
  bool found = false;
  for (VectoredUnitlist::iterator it = begin(); it != end();)
    {
      if ((*it)->getPos() == pos)
	{
	  found = true;
	  it = flErase(it);
	  continue;
	}
      ++it;
    }
  return found;
}

bool VectoredUnitlist::removeVectoredUnitsGoingTo(City *c)
{
  int count = 0;
  int counter = 0;
  bool found = false;
  for (VectoredUnitlist::iterator it = begin(); it != end();)
    {
      if (c->contains((*it)->getDestination()))
	{
	  found = true;
	  it = flErase(it);
	  counter++;
	  continue;
	}
      ++it;
    }
  if (counter != count)
    {
      counter = 0;
  for (VectoredUnitlist::iterator it = begin(); it != end();)
    {
      if (c->contains((*it)->getDestination()))
	{
	  printf ("crap!  we found another one on the second try\n");
	  found = true;
	  it = flErase(it);
	  counter++;
	  continue;
	}
      ++it;
    }
  if (counter)
    {
  printf ("got another %d\n", counter);
    exit(0);
    }
    }
  return found;
}

bool VectoredUnitlist::removeVectoredUnitsComingFrom(City *c)
{
  bool found = false;
  for (VectoredUnitlist::iterator it = begin(); it != end();)
    {
      if (c->contains((*it)->getPos()))
	{
	  found = true;
	  it = flErase(it);
	  continue;
	}
      ++it;
    }
  return found;
}

void VectoredUnitlist::getVectoredUnitsGoingTo(City *c, std::list<VectoredUnit*>& vectored) const
{
  for (VectoredUnitlist::const_iterator it = begin(); it != end(); ++it)
    {
      if (c->contains((*it)->getDestination()))
	{
	  vectored.push_back(*it);
	}
    }
}
void VectoredUnitlist::getVectoredUnitsGoingTo(Vector<int> pos, std::list<VectoredUnit*>& vectored) const
{
  for (VectoredUnitlist::const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getDestination() == pos)
	{
	  vectored.push_back(*it);
	}
    }
}
void VectoredUnitlist::getVectoredUnitsComingFrom(Vector<int> pos, std::list<VectoredUnit*>& vectored) const
{
  for (VectoredUnitlist::const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getPos() == pos)
	{
	  vectored.push_back(*it);
	}
    }
}

guint32 VectoredUnitlist::getNumberOfVectoredUnitsGoingTo(Vector<int> pos) const
{
  guint32 count = 0;
  for (VectoredUnitlist::const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getDestination() == pos)
	{
	  count++;
	}
    }
  return count;
}

bool VectoredUnitlist::changeDestination(City *c, Vector<int> new_dest)
{
  bool found = false;
  for (VectoredUnitlist::iterator it = begin(); it != end(); ++it)
    {
      if (c->contains((*it)->getPos()))
	{
	  assert (c->getOwner() == (*it)->getOwner());
	  (*it)->setDestination(new_dest);
	  found = true;
	}
    }
  return found;
}

VectoredUnitlist::iterator VectoredUnitlist::flErase(iterator object)
{
  delete(*object);
  return erase (object);
}

void VectoredUnitlist::changeOwnership(Player *old_owner, Player *new_owner)
{
  for (iterator it = begin(); it != end(); ++it)
    if ((*it)->getOwner() == old_owner)
      (*it)->setOwner(new_owner);
}

void VectoredUnitlist::reset (VectoredUnitlist *v)
{
  delete s_instance;
  s_instance = v;
}
