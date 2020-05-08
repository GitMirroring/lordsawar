// Copyright (C) 2008, 2010, 2011, 2014, 2015, 2020 Ben Asselstine
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
#include <assert.h>
#include "rectangle.h"
#include <sigc++/functors/mem_fun.h>

#include "citysetlist.h"
#include "ucompose.hpp"
#include "File.h"
#include "defs.h"
#include "tarhelper.h"
#include "setlist.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Citysetlist* Citysetlist::s_instance = 0;

Citysetlist* Citysetlist::getInstance()
{
    if (!s_instance)
        s_instance = new Citysetlist();

    return s_instance;
}

void Citysetlist::deleteInstance()
{
    if (s_instance)
      delete s_instance;

    s_instance = 0;
}

Citysetlist::Citysetlist()
 : SetList(Cityset::file_extension)
{
  loadSets(SetList::scan(Cityset::file_extension));
  loadSets(SetList::scan(Cityset::file_extension, false));
}

Citysetlist::~Citysetlist()
{
  uninstantiateImages();
  for (iterator it = begin(); it != end(); ++it)
    delete *it;
  clear();
}

void Citysetlist::instantiateImages(bool &broken)
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

void Citysetlist::uninstantiateImages()
{
  for (iterator it = begin(); it != end(); ++it)
    (*it)->uninstantiateImages();
}

