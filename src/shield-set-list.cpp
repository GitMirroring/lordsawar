//  Copyright (C) 2008, 2009, 2010, 2011, 2014, 2015, 2020, 2021,
//  2026 Ben Asselstine
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

#include <iostream>
#include <assert.h>
#include "rectangle.h"
#include <sigc++/functors/mem_fun.h>

#include "shield-set-list.h"
#include "shield-set.h"
#include "file.h"
#include "defs.h"
#include "ucompose.hpp"
#include "tar-helper.h"
#include "set-list.h"
#include "tar-file-masked-image.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Shieldsetlist* Shieldsetlist::s_instance = 0;

Shieldsetlist* Shieldsetlist::instance()
{
    if (!s_instance)
        s_instance = new Shieldsetlist();

    return s_instance;
}

void Shieldsetlist::deleteInstance()
{
    if (s_instance)
      delete s_instance;

    s_instance = 0;
}

Shieldsetlist::Shieldsetlist()
 : SetList(Shieldset::file_extension)
{
  loadSets(SetList::scan(Shieldset::file_extension));
  loadSets(SetList::scan(Shieldset::file_extension, false));
}

Shieldsetlist::~Shieldsetlist()
{
  uninstantiateImages();
  for (iterator it = begin(); it != end(); ++it)
    delete *it;
  clear();
}

std::list<guint32> Shieldsetlist::getValidIds() const
{
  std::vector<Shieldset*> objects;
  for (const_iterator it = begin(); it != end(); ++it)
    if ((*it)->validate () == true)
      objects.push_back (*it);

  std::sort
    (objects.begin(), objects.end (),
     [](const Shieldset* a, const Shieldset* b)
     {
       return (a->getName ().casefold () < b->getName ().casefold ());
     });

  std::list<guint32> ids;
  for (auto o : objects)
    ids.push_back (o->getId ());
  return ids;
}

std::vector<Gdk::RGBA> Shieldsetlist::getColors(guint32 shieldset, guint32 owner) const
{
  Shieldset *s = get(shieldset);
  if (!s)
    {
      std::vector<Gdk::RGBA> l;
      l.push_back (Gdk::RGBA("black"));
      return l;
    }
  return s->getColors(owner);
}

ShieldStyle *Shieldsetlist::getShield(guint32 shieldset, guint32 type, guint32 color) const
{
  Shieldset *s = get(shieldset);
  if (!s)
    return NULL;
  return s->lookupShieldByTypeAndColor(type, color);
}

void Shieldsetlist::instantiateImages(bool &broken)
{
  broken = false;
  for (iterator it = begin (); it != end (); ++it)
    if (!broken)
      if ((*it)->validate () == true)
        (*it)->instantiateImages (broken);
}

void Shieldsetlist::uninstantiateImages()
{
  for (iterator it = begin (); it != end (); ++it)
    (*it)->uninstantiateImages ();
}

TarFileMaskedImage *Shieldsetlist::getTartan(guint32 shieldset, guint32 color, Tartan::Type type) const
{
  Shieldset *s = get (shieldset);
  if (!s)
    return NULL;
  return s->lookupTartanImage (color, type);
}
