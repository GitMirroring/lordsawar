// Copyright (C) 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2014, 2015, 2020 Ben Asselstine
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

#include <iostream>
#include <gtkmm.h>
#include <sigc++/functors/mem_fun.h>
#include <assert.h>

#include "rectangle.h"
#include "armysetlist.h"
#include "armyset.h"
#include "File.h"
#include "defs.h"
#include "ucompose.hpp"
#include "PixMask.h"
#include "tarhelper.h"
#include "setlist.h"
#include "TarFileMaskedImage.h"
#include "TarFileImage.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Armysetlist* Armysetlist::s_instance = 0;

Armysetlist* Armysetlist::getInstance()
{
  if (!s_instance)
    s_instance = new Armysetlist();

  return s_instance;
}

void Armysetlist::deleteInstance()
{
  if (s_instance)
    delete s_instance;

  s_instance = 0;
}

Armysetlist::Armysetlist()
 : SetList(Armyset::file_extension)
{
  signal_add().connect(sigc::mem_fun(this, &Armysetlist::on_armyset_added));
  signal_reload().connect(sigc::mem_fun(this, &Armysetlist::on_armyset_reloaded));
  loadSets(SetList::scan(Armyset::file_extension));
  loadSets(SetList::scan(Armyset::file_extension, false));
}

void Armysetlist::on_armyset_added(Armyset *armyset)
{
  for (Armyset::iterator ait = armyset->begin(); ait != armyset->end(); ++ait)
    d_armies[armyset->getId()][(*ait)->getId()] = (*ait);
}

void Armysetlist::on_armyset_reloaded(Armyset *armyset)
{
  d_armies[armyset->getId()].clear();
  for (Armyset::iterator ait = armyset->begin(); ait != armyset->end(); ++ait)
    d_armies[armyset->getId()][(*ait)->getId()] = (*ait);
}

Armysetlist::~Armysetlist()
{
  uninstantiateImages();
  for (iterator it = begin(); it != end(); ++it)
    delete *it;
  clear();
}

ArmyProto* Armysetlist::getArmy(guint32 id, guint32 type_id) const
{
  // always use ArmyProtoMap::find for searching, else a default entry is 
  // created, which can produce really bad results!!
  ArmyPrototypeMap::const_iterator it = d_armies.find(id);

  // armyset does not exist
  if (it == d_armies.end())
    return NULL;

  IdArmyPrototypeMap::const_iterator j = (*it).second.find(type_id);
  if (j == (*it).second.end())
    return NULL;
  return (*j).second;
}

ArmyProto* Armysetlist::lookupWeakestQuickestArmy(guint32 id) const
{
  Armyset *a = get(id);
  if (a)
    return a->lookupWeakestQuickestArmy();
  return NULL;
}

guint32 Armysetlist::getTileSize(guint32 id)
{
  for (iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getId() == id)
	return (*it)->getTileSize();
    }
  return 0;
}

TarFileImage* Armysetlist::getBag (guint32 id)
{
  for (iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getId() == id)
	return (*it)->getBag();
    }
  return NULL;
}

TarFileMaskedImage *Armysetlist::getStandardPic (guint32 id)
{
  for (iterator it = begin(); it != end(); ++it)
    if ((*it)->getId() == id)
      return (*it)->getStandard();
  return NULL;
}

void Armysetlist::instantiateImages(bool &broken)
{
  broken = false;
  for (iterator it = begin(); it != end(); ++it)
    {
      if (!broken)
        {
          if ((*it)->validate () == true)
            (*it)->instantiateImages(true, broken);
        }
    }
}

void Armysetlist::uninstantiateImages()
{
  for (iterator it = begin(); it != end(); ++it)
    (*it)->uninstantiateImages();
}

TarFileMaskedImage *Armysetlist::getShipPic (guint32 id)
{
  for (iterator it = begin(); it != end(); ++it)
    if ((*it)->getId() == id)
      return (*it)->getShip();
  return NULL;
}

