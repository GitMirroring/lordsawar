//  Copyright (C) 2015 Ben Asselstine
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

#include "rnd.h"

Rnd *Rnd::s_instance = 0;

Rnd* Rnd::instance()
{
  if (s_instance == NULL)
    s_instance = new Rnd();
  return s_instance;
}

Rnd::Rnd()
{
  rnd = new Glib::Rand();
}

Rnd::Rnd (const Rnd &k)
{
  rnd = new Glib::Rand ();
  rnd->set_seed (k.rnd->get_int ());
}
  
guint32 Rnd::rand()
{
  guint32 num = instance()->rnd->get_int();
  return num;
}

Rnd::~Rnd()
{
  delete rnd;
}
