//  Copyright (C) 2000, 2001, 2003 Michael Bartl
//  Copyright (C) 2000, 2001, 2002, 2004, 2005 Ulf Lorenz
//  Copyright (C) 2006 Andrea Paternesi
//  Copyright (C) 2006, 2007, 2008, 2014, 2021, 2026 Ben Asselstine
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

#include "location.h"
#include "army.h"
#include "player.h"
#include "player-list.h"
#include "stack-list.h"
#include "fog-map.h"

#include "xml-helper.h"

Location::Location(Vector<int> pos, guint32 size)
    :UniquelyIdentified(), LocationBox(pos, size)
{
}

Location::Location(const Location& loc, bool sync_id)
  :UniquelyIdentified(loc, sync_id), LocationBox(loc)
{
}

Location::Location(const Location& loc, Vector<int> pos)
  :UniquelyIdentified(loc), LocationBox(loc, pos)
{
}

Location::Location(XML_Helper* helper, guint32 size)
    :UniquelyIdentified(helper), LocationBox(helper)
{
    d_size = size;
}
