//  Copyright (C) 2007, 2008, 2009, 2011, 2014, 2021 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>

#include "bridgelist.h"
#include "bridge.h"
#include "xmlhelper.h"

Glib::ustring Bridgelist::d_tag = "bridgelist";
//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Bridgelist* Bridgelist::s_instance=0;

Bridgelist* Bridgelist::getInstance()
{
    if (s_instance == 0)
        s_instance = new Bridgelist();

    return s_instance;
}

Bridgelist* Bridgelist::getInstance(XML_Helper* helper)
{
    if (s_instance)
        deleteInstance();

    s_instance = new Bridgelist(helper);
    return s_instance;
}

void Bridgelist::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

Bridgelist::Bridgelist()
{
}

Bridgelist::~Bridgelist()
{
  for (iterator it = begin(); it != end(); ++it)
    delete *it;
  d_object.clear();
  d_id.clear();
}

Bridgelist::Bridgelist (const Bridgelist &b, bool sync_ids)
 : LocationList<Bridge*> (), sigc::trackable (b)
{
  for (auto bridge : b)
    add (new Bridge (*bridge, sync_ids));
}

Bridgelist::Bridgelist(XML_Helper* helper)
{
    helper->registerTag(Bridge::d_tag, sigc::mem_fun(this, &Bridgelist::load));
}

bool Bridgelist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(Bridgelist::d_tag);

    for (const_iterator it = begin(); it != end(); ++it)
        retval &= (*it)->save(helper);
    
    retval &= helper->closeTag();

    return retval;
}

bool Bridgelist::load(Glib::ustring tag, XML_Helper* helper)
{
    if (tag != Bridge::d_tag)
    //what has happened?
        return false;
    
    add(new Bridge(helper));

    return true;
}

int Bridgelist::calculateType(Vector<int> t) const
{
    // examine neighbour tiles to discover whether there's a bridge on them
    bool u = getObjectAt(t + Vector<int>(0, -1));
    bool b = getObjectAt(t + Vector<int>(0, 1));
    bool l = getObjectAt(t + Vector<int>(-1, 0));
    bool r = getObjectAt(t + Vector<int>(1, 0));

    if (u)
      return Bridge::CONNECTS_TO_NORTH;
    if (b)
      return Bridge::CONNECTS_TO_SOUTH;
    if (r)
      return Bridge::CONNECTS_TO_EAST;
    if (l)
      return Bridge::CONNECTS_TO_WEST;
    return Bridge::CONNECTS_TO_NORTH;
}

Bridge* Bridgelist::getOtherSide(Bridge *bridge)
{
  switch (bridge->getType())
    {
    case Bridge::CONNECTS_TO_NORTH:
      return getObjectAt(bridge->getPos() + Vector<int>(0, -1));
    case Bridge::CONNECTS_TO_SOUTH:
      return getObjectAt(bridge->getPos() + Vector<int>(0, 1));
    case Bridge::CONNECTS_TO_EAST:
      return getObjectAt(bridge->getPos() + Vector<int>(1, 0));
    case Bridge::CONNECTS_TO_WEST:
      return getObjectAt(bridge->getPos() + Vector<int>(-1, 0));
    }
  return NULL;
}

std::list<Vector<int> > Bridgelist::getRoadEntryPoints(Bridge *bridge)
{
  std::list<Vector<int> > points;
  points.push_back(bridge->getRoadEntryPoint());
  Bridge *other = getOtherSide(bridge);
  if (other)
    points.push_back(other->getRoadEntryPoint());
  return points;
}

void Bridgelist::reset (Bridgelist *b)
{
  delete s_instance;
  s_instance = b;
}
