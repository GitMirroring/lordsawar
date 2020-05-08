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

#pragma once
#ifndef RND_H
#define RND_H

#include <glibmm.h>
//! A simple random number provider
/**
  */
class Rnd
{
 public:
  static Rnd* instance();

  static void set_seed(guint32 seed) {instance()->rnd->set_seed(seed);}
  static guint32 rand();

 private:
    Rnd();
    Rnd(const Rnd &r);
    ~Rnd();
  static Rnd *s_instance;
  Glib::Rand *rnd;
};

#endif
