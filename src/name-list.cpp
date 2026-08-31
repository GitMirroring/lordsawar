//  Copyright (C) 2009, 2021, 2026 Ben Asselstine
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

#include <iostream>
#include <sigc++/functors/mem_fun.h>

#include "name-list.h"
#include "defs.h"
#include "file.h"
#include "ucompose.hpp"
#include "xml-helper.h"
#include "rnd.h"

#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
//#define debug(x)

NameList::NameList(Glib::ustring filename, Glib::ustring item_tag)
 : d_filename (filename), d_item_tag (item_tag)
{
  XML_Helper helper (File::getMiscFile (d_filename), std::ios::in);

  helper.register_tag (d_item_tag, sigc::mem_fun ((*this), &NameList::load));

  if (!helper.parse_XML ())
    {
      std::cerr << String::ucompose (_("Error can't load namelist `%1'"),
                                     d_filename) << std::endl;
      exit (-1);
    }
  helper.close ();
  return;
}

NameList::NameList (const NameList &l)
 : std::vector<Glib::ustring> (), sigc::trackable ()
{
  for (const_iterator i = l.begin (); i != l.end (); ++i)
    push_back (*i);
}

bool NameList::load(Glib::ustring tag, XML_Helper *helper)
{
  if (tag == d_item_tag)
    {
      Glib::ustring name;
      helper->get(name, "name");
      push_back(name); 
    }
  return true;
}

bool NameList::repopulate (std::list<Glib::ustring> names)
{
  clear ();
  XML_Helper helper(File::getMiscFile (d_filename), std::ios::in);

  helper.register_tag(d_item_tag, sigc::mem_fun ((*this), &NameList::load));

  if (!helper.parse_XML ())
    {
      std::cerr << String::ucompose (_("Error can't load namelist `%1'"),
                                     d_filename) << std::endl;
      exit (-1);
    }
  helper.close ();
  for (auto name : names)
    {
      NameList::iterator it = std::find (begin (), end (), name);
      if (it != end ())
        erase (it);
    }
  return empty () == false;
}

Glib::ustring NameList::popRandomName()
{
  Glib::ustring name;
  if (empty())
    return "";
  int randno = Rnd::rand() % size();
  name = (*this)[randno];

  NameList::iterator it = std::find(begin(), end(), name);
  if (it != end())
    erase(it);

  return name;
}
