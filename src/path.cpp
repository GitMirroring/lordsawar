// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2004 John Farrell
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2014, 2015 Ben Asselstine
// Copyright (C) 2008 Ole Laursen
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
#include <sstream>
#include <queue>

#include "PathCalculator.h"
#include "path.h"
#include "army.h"
#include "GameMap.h"
#include "city.h"
#include "stacklist.h"
#include "xmlhelper.h"
#include "stack.h"
#include "player.h"

Glib::ustring Path::d_tag = "path";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Path::Path()
 : std::list<Vector<int> > ()
{
  clear();
  d_moves_exhausted_at_point = 0;
}

Path::Path(const Path& p)
 : std::list<Vector<int> >(),
    d_moves_exhausted_at_point(p.d_moves_exhausted_at_point)
{
  for (const_iterator it = p.begin(); it != p.end(); ++it)
    push_back(Vector<int>((*it).x, (*it).y));
}

Path::Path(XML_Helper* helper)
{
    int i;
    std::istringstream sx, sy;
    Glib::ustring s;

    helper->getData(d_moves_exhausted_at_point, "moves_exhausted_at_point");
    helper->getData(i, "size");

    helper->getData(s, "x");
    sx.str(s);
    helper->getData(s, "y");
    sy.str(s);

    for (; i > 0; i--)
    {
        Vector<int> p = Vector<int>(-1,-1);

        sx >> p.x;
        sy >> p.y;
        push_back(p);
    }
}

Path::~Path()
{
    clear();
}

bool Path::save(XML_Helper* helper) const
{
    bool retval = true;

    std::stringstream sx, sy;
    for (const_iterator it = begin(); it != end(); ++it)
    {
        sx <<(*it).x <<" ";
        sy <<(*it).y <<" ";
    }

    retval &= helper->openTag(Path::d_tag);
    retval &= helper->saveData("size", (guint32) size());
    retval &= helper->saveData("moves_exhausted_at_point", 
                               d_moves_exhausted_at_point);
    retval &= helper->saveData("x", sx.str());
    retval &= helper->saveData("y", sy.str());
    retval &= helper->closeTag();

    return retval;
}

bool Path::checkPath(Stack* s, int enemy_city_avoidance, int enemy_stack_avoidance)
{
    if (empty())
        return true;
    bool valid = true;
    if (size() > 1)
      {
	iterator secondlast = end();
	--secondlast;
	for (iterator it = begin(); it != secondlast; ++it)
	  {
	    if (PathCalculator::isBlocked(s, *it, enemy_city_avoidance, 
					  enemy_stack_avoidance) == false)
	      {
		valid = false;
		break;
	      }
	  }
      }

    return valid;
}

//this is used to update the moves_exhausted_at_point variable
void Path::recalculate (Stack* s)
{
  if (size() == 0)
    return;

  // be careful to not go into cities that are now owned by the enemy
  reverse_iterator it = rbegin();
  for (; it != rend(); ++it)
    {
      Vector<int> dest = *it;
      City *c = GameMap::getCity(dest);
      if (c && c->getOwner() != s->getOwner())
	continue;
      else
	break;
    }
  if (it == rend())
    {
      //well, it looks like all of our points were in enemy cities
      setMovesExhaustedAtPoint(0);
      clear();
    }
  else
    {
      Vector<int> dest = *it;
      calculate(s, dest);
    }
  return;
}

guint32 Path::calculateToCity (Stack *s, City *c, bool zigzag)
{
  int min_dist = -1;
  Vector<int> shortest = c->getPos();
  bool checkJoin = s->getOwner() == c->getOwner();

  for (unsigned int i = 0; i < c->getSize(); i++)
    for (unsigned int j = 0; j < c->getSize(); j++)
      {
	if (checkJoin == true)
	  {
	    Stack *other_stack = GameMap::getStack(c->getPos() + Vector<int>(i,j));
	    if (other_stack && GameMap::canJoin(s,other_stack) == false)
	      continue;
	  }
	int distance = dist (s->getPos(), c->getPos() + Vector<int>(i, j));
	if (distance > 0)
	  {
	    if (distance < min_dist || min_dist == -1)
	      {
		min_dist = distance;
		shortest = c->getPos() + Vector<int>(i, j);
	      }
	  }
      }
  int mp = calculate(s, shortest, zigzag);
  if (mp <= 0)
    {
      //okay.. try really hard
      min_dist = -1;
      for (unsigned int i = 0; i < c->getSize(); i++)
	for (unsigned int j = 0; j < c->getSize(); j++)
	  {
	    if (checkJoin == true)
	      {
		Stack *other_stack = GameMap::getStack(c->getPos() + Vector<int>(i,j));
		if (other_stack && GameMap::canJoin(s,other_stack) == false)
		  continue;
	      }
	    int dist = calculate(s, c->getPos() + Vector<int>(i, j), zigzag);
	    if (dist > 0)
	      {
		if (dist < min_dist || min_dist == -1)
		  {
		    min_dist = dist;
		    shortest = c->getPos() + Vector<int>(i, j);
		  }
	      }
	  }
      mp = calculate(s, shortest, zigzag);
    }
  return mp;
}

void Path::calculate (Stack* s, Vector<int> dest, guint32 &moves, guint32 &turns, guint32 &left, bool zigzag)
{
  //int mp;
  //Vector<int> start = s->getPos();
  debug("path from "<<start.x<<","<<start.y<<" to "<<dest.x<<","<<dest.y)

  clear();

  // Some notes concerning the path finding algorithm. The algorithm
  // uses two different lists. There is a nodes array, which contains how
  // many MP one needs to get to the location (x,y), and a process queue that
  // tells you at what point the number of movement points is calculated next.
  //
  // What we basically do is to start at the stack's position and calculate
  // the distance (i.e. MP needed to get there) in circles around the starting
  // position with the circles becoming increasingly larger. In detail, we
  // start by appending the starting position in the queue of positions to be
  // processed. Then, recursively, we take the first point in the queue and
  // recalculate the distance for all bordering tiles, assuming that we go
  // over this point and overwriting their current value if it is larger
  // than what we find now. Then, we append each modified tile to the queue of
  // tiles to be processed and continue. We stop when there are no more tiles
  // to process.
  //
  // Finally, all that is left is finding the minimum distance way from start
  // point to destination.

    
  int enemy_city_avoidance = -1;
  int enemy_stack_avoidance = -1;
  if (s->getOwner() && s->getOwner()->isComputer())
    {
      //If we're a computer player we don't let enemy stacks and cities
      //prevent us from reaching our destination.  When we encounter them
      //we'll fight, but we try not to encounter them.
      enemy_city_avoidance = 10;
      enemy_stack_avoidance = 10;
    }
  PathCalculator pc = PathCalculator(s, zigzag, enemy_city_avoidance, 
				     enemy_stack_avoidance);

  Path *calculated_path = pc.calculate(dest, moves, turns, left, zigzag);
  if (calculated_path->size())
    {
      for(Path::iterator it = calculated_path->begin(); it!= calculated_path->end(); ++it)
	push_back(*it);
    }

  //calculate when the waypoints show no more movement possible
  guint32 pathcount = 0;
  guint32 moves_left = s->getMoves();
  for (iterator it = begin(); it != end(); ++it)
    {
      guint32 tile_moves = s->calculateTileMovementCost(*it);
      if (moves_left >= tile_moves)
	moves_left -= tile_moves;
      else
	break;
      pathcount++;
    }
  setMovesExhaustedAtPoint(pathcount);
  delete calculated_path;

  return;
}

guint32 Path::calculate (Stack* s, Vector<int> dest, guint32 &turns, bool zigzag)
{
  guint32 moves = 0, left = 0;
  calculate(s, dest, moves, turns, left, zigzag);
  return moves;
}

guint32 Path::calculate (Stack* s, Vector<int> dest, bool zigzag)
{
  guint32 mp = 0, turns = 0, left = 0;
  calculate(s, dest, mp, turns, left, zigzag);
  return mp;
}

void Path::eraseFirstPoint()
{
  erase(begin());

  if (getMovesExhaustedAtPoint() > 0)
    setMovesExhaustedAtPoint(getMovesExhaustedAtPoint()-1);
}

// End of file
