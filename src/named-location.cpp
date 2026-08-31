//  Copyright (C) 2008, 2021, 2026 Ben Asselstine
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

#include "named-location.h"

#include "xml-helper.h"

NamedLocation::NamedLocation(Vector<int> pos, guint32 size, Glib::ustring name,
			     Glib::ustring desc)
  :Location(pos, size), Renamable(name), d_description(desc)
{
}

NamedLocation::NamedLocation(const NamedLocation& object, bool sync_id)
  :Location(object, sync_id), Renamable(object),
    d_description(object.d_description)
{
}

NamedLocation::NamedLocation(const NamedLocation& object, Vector<int> pos)
  :Location(object, pos), Renamable(object), d_description(object.d_description)
{
}

NamedLocation::NamedLocation(XML_Helper* helper, guint32 size)
  :Location(helper, size), Renamable(helper)
{
  helper->get(d_description, "description");
}
