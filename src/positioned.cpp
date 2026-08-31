//  Copyright (C) 2008, Ben Asselstine
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

#include "Positioned.h"

#include "xmlhelper.h"

Positioned::Positioned(Vector<int> pos)
  :d_pos(pos)
{
}

Positioned::Positioned(const Positioned& pos)
  :d_pos(pos.d_pos)
{
}

Positioned::Positioned(XML_Helper* helper)
{
  if (!helper)
    return;
  helper->getData(d_pos.x, "x");
  helper->getData(d_pos.y, "y");
}
