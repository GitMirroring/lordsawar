//  Copyright (C) 2008, 2014, 2026 Ben Asselstine
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

#include "movable.h"

#include "xml-helper.h"

Movable::Movable(Vector<int> pos)
  :Positioned(pos)
{
}

Movable::Movable(const Movable& pos)
  :Positioned(pos)
{
}

Movable::Movable(XML_Helper* helper)
  :Positioned(helper)
{
}
