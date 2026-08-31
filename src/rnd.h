//  Copyright (C) 2015, 2026 Ben Asselstine
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

#pragma once
#ifndef RND_H
#define RND_H

#include <random>
#include <glibmm.h>
//! A simple random number provider
/**
  */

class Rnd
{
public:
    static Rnd* instance ();

    static void set_seed (uint32_t seed)
      {
        instance ()->engine.seed (seed);
      }

    static uint32_t rand ()
      {
        static std::uniform_int_distribution<uint32_t> dist;
        return dist (instance ()->engine);
      }

    static std::mt19937& gen () { return instance ()->engine; }
private:
    Rnd () : engine(std::random_device{}()) {}
    Rnd (const Rnd&) = delete;
    Rnd& operator=(const Rnd&) = delete;
    ~Rnd () = default;

    static Rnd* s_instance;

    std::mt19937 engine;
};

#endif
