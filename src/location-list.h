// Copyright (C) 2001, 2003 Michael Bartl
// Copyright (C) 2004 Ulf Lorenz
// Copyright (C) 2005, 2006 Andrea Paternesi
// Copyright (C) 2007, 2008, 2009, 2012, 2014, 2015, 2021 Ben Asselstine
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

#pragma once
#ifndef LOCATIONLIST_H
#define LOCATIONLIST_H

#include <gtkmm.h>
#include <algorithm>
#include <list>
#include <map>
#include "PathCalculator.h"
#include "vector.h"
#include "stack.h"
#include "maptile.h"
#include "Location.h"

//! A template for holding and operatig on lists of Location objects.
/** A list for object instances
  * 
  * This class extends the stl lists by adding the functions getObjectAt()
  * which return the object at position (x,y). Necessary for such things as
  * the city list.
  */

template<class T> class LocationList : public std::list<T>
{
 public:
  
  LocationList(){};  
  ~LocationList() {}

  void add(T t)
    {
      this->push_back(t);
      d_id[t->getId()] = t;
      int s = t->getSize();
      for (int i = 0; i < s; i++)
	for (int j = 0; j < s; j++)
	  {
	    Vector<int> pos = t->getPos() + Vector<int>(i,j);
	    d_object[pos] = t;
	  }
    }
  void replace (T o, T n)
    {
      subtract (o);
      add (n);
    }
  void subtract(T t)
    {
      this->erase(std::find(this->begin(), this->end(), t));
      d_id.erase(d_id.find(t->getId()));
      int s = t->getSize();
      for (int i = 0; i < s; i++)
	for (int j = 0; j < s; j++)
	  {
	    Vector<int> pos = t->getPos() + Vector<int>(i,j);
            if (d_object.find(pos) != d_object.end())
              d_object.erase(d_object.find(pos));
	  }
      delete t;
    }

  //! Returns the object at position (x,y).  
  T getObjectAt(int x, int y) const
    {
      Vector<int> pos = Vector<int>(x,y);
  
      if (d_object.find(pos) == d_object.end())
	return NULL;
      else
	return (*d_object.find(pos)).second;
    }

  //! Returns the object at position pos.  
  T getObjectAt(const Vector<int>& pos) const
    {
      return getObjectAt(pos.x, pos.y);
    }

void resizeLocations(Maptile::Building building_type, guint32 tile_width, guint32 old_tile_width, void (*func1)(Location*, Maptile::Building, guint32), void (*func2)(Location*, Maptile::Building, guint32))
{
  if (old_tile_width > tile_width)
    {
      for (typename LocationList<T>::const_iterator it = this->begin(); it != this->end(); ++it)
        func1(*it, building_type, old_tile_width);
    }
  std::list<T> objs;
  for (typename LocationList<T>::iterator it = this->begin(); it != this->end(); ++it)
    objs.push_back(*it);

  for (typename std::list<T>::iterator i = objs.begin(); i != objs.end(); ++i)
  //for (typename LocationList<T>::iterator it = this->begin(); it != this->end(); ++it)
    func2((*i), building_type, tile_width);
}

  T getNearestObjectInDir(const Vector<int> &pos, const Vector<int> dir) const
    {
      int diff = -1;
      typename LocationList<T>::const_iterator diffit;
      for (typename LocationList<T>::const_iterator it = this->begin(); it != this->end(); ++it)
        {
          Vector<int> p = (*it)->getPos();
          int delta = abs(p.x - pos.x) + abs(p.y - pos.y);
	  //if dir is -1, then the difference between pos.x and p.x should be positive
	  //if dir is +1, then the difference between pos.x and p.x should be negative
	  //if looking west, and the object is to the east
	  if (dir.x < 0 && (pos.x - p.x) <= 0)
	    continue;
	  //if looking east , and the object is to the west
	  if (dir.x > 0 && (pos.x - p.x) >= 0)
	    continue;
	  //if looking north, and the object is to the south
	  if (dir.y < 0 && (pos.y - p.y) <= 0)
	    continue;
	  //if looking south, and the object is to the north
	  if (dir.y > 0 && (pos.y - p.y) >= 0)
	    continue;

          if ((diff > delta) || (diff == -1))
            {
              diff = delta;
              diffit = it;
            }
        }
      if (diff == -1) return 0;
      return (*diffit);
    }

  T getClosestObject (const Stack *stack, std::list<bool (*)(void*)> *filters) const
    {
      int diff = -1;
      typename LocationList<T>::const_iterator diffit;
      PathCalculator pc(stack, true, 0, 0);
      for (typename LocationList<T>::const_iterator it = this->begin(); it != this->end(); ++it)
        {
          int delta = pc.calculate((*it)->getPos());
          if (delta <= 0)
            continue;
	  if (filters)
	    {
	      bool filtered = false;
	      for (std::list<bool (*)(void*)>::iterator fit = filters->begin();
                   fit != filters->end(); ++fit)
	        {
	          if ((*fit)(*it) == true)
	            {
		      filtered = true;
		      break;
	            }
	            
	        }
	      if (filtered)
	        continue;
	    }

          if ((diff > delta) || (diff == -1))
            {
              diff = delta;
              diffit = it;
            }
        }
      if (diff == -1) return 0;
      return (*diffit);
    }

  T getClosestObject (const Stack *stack) const
    {
      return getClosestObject (stack, NULL);
    }

  T getNearestObject (const Vector<int>& pos, std::list<bool (*)(void*)> *filters) const
    {
      int diff = -1;
      typename LocationList<T>::const_iterator diffit;
      for (typename LocationList<T>::const_iterator it = this->begin(); it != this->end(); ++it)
        {
          Vector<int> p = (*it)->getPos();
          int delta = abs(p.x - pos.x) + abs(p.y - pos.y);
	  if (filters)
	    {
	      bool filtered = false;
	      for (std::list<bool (*)(void*)>::iterator fit = filters->begin();
                   fit != filters->end(); ++fit)
	        {
	          if ((*fit)(*it) == true)
	            {
		      filtered = true;
		      break;
	            }
	            
	        }
	      if (filtered)
	        continue;
	    }

          if ((diff > delta) || (diff == -1))
            {
              diff = delta;
              diffit = it;
            }
        }
      if (diff == -1) return 0;
      return (*diffit);
    }

  T getNearestObject (const Vector<int>& pos) const
    {
      return getNearestObject (pos, NULL);
    }

  T getNearestObjectBefore (const Vector<int>& pos, int dist) const
    {
      T t = getNearestObject(pos);
      if (!t)
	return NULL;
      if (t->getPos().x <= pos.x + dist && t->getPos().x >= pos.x - dist &&
          t->getPos().y <= pos.y + dist && t->getPos().y >= pos.y - dist)
        return t;
      return NULL;
    }

  T getNearestObjectAfter(const Vector<int>& pos, int dist, 
			   std::list<bool (*)(void*)> *filters) const
    {
      int diff = -1;
      typename LocationList<T>::const_iterator diffit;

      for (typename LocationList<T>::const_iterator it = this->begin(); it != this->end(); ++it)
        {
	  if (filters)
	    {
	      bool filtered = false;
	      for (std::list<bool (*)(void*)>::iterator fit = filters->begin();
                   fit != filters->end(); ++fit)
	        {
	          if ((*fit)(*it) == true)
	            {
		      filtered = true;
		      break;
	            }
	            
	        }
	      if (filtered)
	        continue;
	    }
          
            Vector<int> p = (*it)->getPos();
            int delta = abs(p.x - pos.x);
            if (delta < abs(p.y - pos.y))
                delta = abs(p.y - pos.y);
          
            if ((diff > delta && delta >= dist) || (diff == -1))
              {
                diff = delta;
                diffit = it;
              }
        }
    
      if (diff == -1) return 0;
      return (*diffit);
    }

  T getById(guint32 id)
  {
      if (d_id.find(id) == d_id.end())
	return NULL;
      else
	return (*d_id.find(id)).second;
    return 0;
  }
	
 protected:
  typedef std::map<Vector<int>, T> PositionMap;
  typedef std::map<guint32, T> IdMap;
  PositionMap d_object;
  IdMap d_id;

};

#endif // LOCATIONLIST_H

// End of file
