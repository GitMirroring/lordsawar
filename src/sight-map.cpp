//  Copyright (C) 2008, 2014, 2015, 2020, 2026 Ben Asselstine
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

#include <sstream>
#include "sight-map.h"
#include "xml-helper.h"

Glib::ustring SightMap::d_tag = "sightmap";

SightMap::SightMap(XML_Helper* helper)
	:Renamable(helper)
{
    helper->get(x, "x");
    helper->get(x, "y");
    helper->get(w, "width");
    helper->get(h, "height");
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

  retval &= helper->open_tag(SightMap::d_tag);
  retval &= helper->save("name", getName());
  retval &= helper->save("x", x);
  retval &= helper->save("y", y);
  retval &= helper->save("width", w);
  retval &= helper->save("height", h);
  retval &= helper->close_tag();

  return retval;
}
