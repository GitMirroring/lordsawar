//  Copyright (C) 2007, 2008, 2009, 2014, 2021, 2026 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>

#include "road-list.h"
#include "road.h"
#include "game-map.h"
#include "xml-helper.h"

Glib::ustring Roadlist::d_tag = "roadlist";
//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Roadlist* Roadlist::s_instance=0;

Roadlist* Roadlist::instance()
{
    if (s_instance == 0)
        s_instance = new Roadlist();

    return s_instance;
}

Roadlist* Roadlist::instance(XML_Helper* helper)
{
    if (s_instance)
        deleteInstance();

    s_instance = new Roadlist(helper);
    return s_instance;
}

void Roadlist::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

Roadlist::Roadlist()
{
}

Roadlist::~Roadlist()
{
  for (iterator it = begin(); it != end(); ++it)
    delete *it;
  d_object.clear();
  d_id.clear();
}

Roadlist::Roadlist (const Roadlist &r, bool sync_ids)
 : LocationList<Road*> (), sigc::trackable (r)
{
  for (auto road : r)
    add (new Road (*road, sync_ids));
}

Roadlist::Roadlist(XML_Helper* helper)
{
    helper->register_tag(Road::d_tag, sigc::mem_fun(*this, &Roadlist::load));
}

bool Roadlist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->open_tag(Roadlist::d_tag);

    for (const_iterator it = begin(); it != end(); ++it)
        retval &= (*it)->save(helper);
    
    retval &= helper->close_tag();

    return retval;
}

bool Roadlist::load(Glib::ustring tag, XML_Helper* helper)
{
    if (tag != Road::d_tag)
    //what has happened?
        return false;
    
    add(new Road(helper));

    return true;
}

Road::Type Roadlist::calculateType (Vector<int> t) const
{
    // examine neighbour tiles to discover whether there's a road on them
    bool u = false; //up
    bool b = false; //bottom
    bool l = false; //left
    bool r = false; //right

    if (t.y > 0)
      u = getObjectAt(t + Vector<int>(0, -1));
    if (t.y < GameMap::getHeight() - 1)
      b = getObjectAt(t + Vector<int>(0, 1));
    if (t.x > 0)
      l = getObjectAt(t + Vector<int>(-1, 0));
    if (t.x < GameMap::getWidth() - 1)
    r = getObjectAt(t + Vector<int>(1, 0));

    // then translate this to the type
    auto type = Road::CONNECTS_ALL_DIRECTIONS;
    //show road type 2 when no other road tiles are around
    if (!u && !b && !l && !r)
      type = Road::CONNECTS_ALL_DIRECTIONS;
    else if (u && b && l && r)
      type = Road::CONNECTS_ALL_DIRECTIONS;
    else if (!u && b && l && r)
      type = Road::CONNECTS_EAST_WEST_AND_SOUTH;
    else if (u && !b && l && r)
      type = Road::CONNECTS_EAST_WEST_AND_NORTH;
    else if (u && b && !l && r)
      type = Road::CONNECTS_NORTH_AND_SOUTH_AND_EAST;
    else if (u && b && l && !r)
      type = Road::CONNECTS_NORTH_SOUTH_AND_WEST;
    else if (u && b && !l && !r)
      type = Road::CONNECTS_NORTH_AND_SOUTH;
    else if (!u && !b && l && r)
      type = Road::CONNECTS_EAST_AND_WEST;
    else if (u && !b && l && !r)
      type = Road::CONNECTS_NORTH_AND_WEST;
    else if (u && !b && !l && r)
      type = Road::CONNECTS_NORTH_AND_EAST;
    else if (!u && b && l && !r)
      type = Road::CONNECTS_WEST_AND_SOUTH;
    else if (!u && b && !l && r)
      type = Road::CONNECTS_SOUTH_AND_EAST;
    else if (u && !b && !l && !r)
      type = Road::CONNECTS_NORTH;
    else if (!u && b && !l && !r)
      type = Road::CONNECTS_SOUTH;
    else if (!u && !b && l && !r)
      type = Road::CONNECTS_WEST;
    else if (!u && !b && !l && r)
      type = Road::CONNECTS_EAST;
    return type;
}

void Roadlist::reset (Roadlist *r)
{
  delete s_instance;
  s_instance = r;
}
