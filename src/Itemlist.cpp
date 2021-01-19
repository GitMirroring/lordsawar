// Copyright (C) 2004, 2005 Ulf Lorenz
// Copyright (C) 2007, 2008, 2011, 2014, 2015, 2021 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>

#include <iostream>
#include "Itemlist.h"

#include "File.h"
#include "defs.h"
#include "file-compat.h"
#include "ucompose.hpp"
#include "xmlhelper.h"

Glib::ustring Itemlist::d_tag = "itemlist";

Itemlist* Itemlist::d_instance = 0;

Itemlist* Itemlist::getInstance()
{
    if (!d_instance)
        d_instance = new Itemlist();

    return d_instance;
}

Itemlist* Itemlist::getInstance(XML_Helper *helper)
{
    if (d_instance)
      delete d_instance;

    d_instance = new Itemlist(helper);
    return d_instance;
}

void Itemlist::createStandardInstance()
{
    deleteInstance();

    XML_Helper helper(File::getItemDescription(), std::ios::in);
    d_instance = new Itemlist(&helper);

    if (!helper.parseXML())
    {
      std::cerr << String::ucompose(_("Could not parse item description file `%1'.  Exiting."), File::getItemDescription()) << std::endl;
        exit(-1);
    }
    helper.close();
}

void Itemlist::deleteInstance()
{
    if (d_instance != 0)
        delete d_instance;

    d_instance = 0;
}


Itemlist::Itemlist(XML_Helper* helper)
{
    helper->registerTag(ItemProto::d_itemproto_tag, sigc::mem_fun(*this, &Itemlist::loadItemProto));
}

Itemlist::Itemlist()
{
}

Itemlist::Itemlist (const Itemlist &ilist)
 : std::map<guint32, ItemProto*> (), sigc::trackable (ilist)
{
  for (auto i : ilist)
    (*this)[i.first] = new ItemProto (*i.second);
}

Itemlist::~Itemlist()
{
    flClear();
}

bool Itemlist::loadItemProto(Glib::ustring tag, XML_Helper* helper)
{
    if (tag != ItemProto::d_itemproto_tag)
        return false;

    ItemProto* i = new ItemProto(helper);
    (*this)[(*this).size()] = i;

    return true;
}

void Itemlist::flErase(iterator it)
{
    delete (*it).second;
    erase(it);
}

void Itemlist::flClear()
{
    while (!empty())
        flErase(begin());
}

bool Itemlist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(d_tag);

    for (const_iterator it = begin(); it != end(); ++it)
      (*it).second->save(helper);
    
    retval &= helper->closeTag();

    return retval;
}

void Itemlist::remove(ItemProto *itemproto)
{
  guint32 index = 0;
  for (iterator it = begin(); it != end(); ++it)
    {
      if ((*it).second == itemproto)
	{
	  erase(index);
	  break;
	}
      index++;
    }
}

void Itemlist::add(ItemProto *itemproto)
{
  (*this)[size()] = itemproto;
}

bool Itemlist::upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version)
{
  return FileCompat::getInstance()->upgrade(filename, old_version, new_version,
                                            FileCompat::ITEMLIST, 
                                            d_tag);
}

void Itemlist::support_backward_compatibility()
{
  FileCompat::getInstance()->support_type
    (FileCompat::ITEMLIST, File::get_extension(File::getItemDescription()), 
     d_tag, false);
  FileCompat::getInstance()->support_version
    (FileCompat::ITEMLIST, "0.2.0", LORDSAWAR_ITEMS_VERSION,
     sigc::ptr_fun(&Itemlist::upgrade));
}

void Itemlist::create()
{
    deleteInstance();
    getInstance();
}

void Itemlist::reset (Itemlist *i)
{
  delete d_instance;
  d_instance = i;
}
