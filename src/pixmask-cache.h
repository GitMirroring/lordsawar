// Copyright (C) 2014, 2015 Ben Asselstine
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
#ifndef PIXMASKCACHE_H
#define PIXMASKCACHE_H

#include <list>
#include <map>
#include <sigc++/trackable.h>
#include <sigc++/slot.h>
#include "PixMask.h"

//! Template for caching PixMask objects along with a model of each object.
template<class T> class PixMaskCache: public std::list<T>, public sigc::trackable
{
public:
    typedef sigc::slot<PixMask*, T> Generator;

    PixMaskCache(Generator g): generate (g), cachesize (0) {}

    PixMaskCache(const PixMaskCache &c)
      : std::list<T>(), sigc::trackable (), generate (c.generate), cachesize (c.cachesize)
      {
        for (typename std::map<T, PixMask *>::const_iterator i = c.surfaces.begin ();
             i != c.surfaces.end (); ++i)
          {
            this->push_back (T((*i).first));
            surfaces[this->back ()] = (*i).second->copy ();
          }
      }

    ~PixMaskCache()
      {
        reset();
      }

    guint32 getCacheSize() const {return cachesize;}

    guint32 eraseLeastRecentlyUsed()
      {
        guint32 siz = 0;
        if (this->empty() == false)
          {
            T item = *(this->begin());
            this->erase(this->begin());
            typename std::map<T,PixMask*>::iterator i = surfaces.find(item);
            if (i != surfaces.end())
              {
                PixMask *s = (*i).second;
                surfaces.erase(i);
                if (s)
                  {
                    siz = s->get_depth()/8 * (s->get_width() * s->get_height());
                    cachesize -= siz;
                    delete s;
                  }
              }
          }
        return siz;
      }

    void reset()
      {
        while (this->empty() == false)
          this->eraseLeastRecentlyUsed();
        cachesize = 0;
      }

    PixMask* get(T &item, guint32 &size_added)
      {
        //see if we already made it.
        typename PixMaskCache<T>::iterator it = std::find(this->begin(), this->end(), item);
        if (it != this->end())
          {
            //looks like we made it.  barry manilow.
            //put the item in last place (last touched)
            this->erase(it);
            this->push_back(item);
            return surfaces[item];
          }
        else
          {
            //generate the image
            PixMask *s = (generate)(item);
            if (s)
              {
                surfaces[item] = s;
                this->push_back(item);
                size_added = (s->get_width() * s->get_height()) * 
                  (s->get_depth()/8);
                cachesize += size_added;
              }
            return s;
          }
      }

    guint32 discardHalf()
      {
        guint32 siz = 0;
        if (this->size() <= 1)
          //this is necessary so we don't delete the pixmask we just created
          return siz;
        if (this->empty() == false)
          {
            guint32 half = cachesize / 2;
            while (cachesize > half)
              {
                siz += this->eraseLeastRecentlyUsed();
                if (this->size () == 1)
                  break;
              }
          }
        return siz;
      }
private:
    std::map<T, PixMask *> surfaces;
    Generator generate;
    guint32 cachesize;
};
#endif
