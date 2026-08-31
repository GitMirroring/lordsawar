// Copyright (C) 2008, 2014, 2015, 2020 Ben Asselstine
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

#include <sstream>
#include "SightMap.h"
#include "xmlhelper.h"

Glib::ustring SightMap::d_tag = "sightmap";

SightMap::SightMap(XML_Helper* helper)
	:Renamable(helper)
{
    helper->getData(x, "x");
    helper->getData(x, "y");
    helper->getData(w, "width");
    helper->getData(h, "height");
}

SightMap::SightMap(Glib::ustring name, Vector<int> p, guint32 height, guint32 width)
:LwRectangle(p, Vector<int>(width, height)), Renamable(name)
{
}

SightMap::SightMap(const SightMap& orig)
:LwRectangle(orig), Renamable(orig)
{
}

bool SightMap::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag(SightMap::d_tag);
  retval &= helper->saveData("name", getName());
  retval &= helper->saveData("x", x);
  retval &= helper->saveData("y", y);
  retval &= helper->saveData("width", w);
  retval &= helper->saveData("height", h);
  retval &= helper->closeTag();

  return retval;
}
