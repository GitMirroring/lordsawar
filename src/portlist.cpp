// Copyright (C) 2007, 2008, 2009, 2014, 2021 Ben Asselstine
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

#include "portlist.h"
#include "port.h"
#include "xmlhelper.h"

Glib::ustring Portlist::d_tag = "portlist";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Portlist* Portlist::s_instance=0;

Portlist* Portlist::getInstance()
{
    if (s_instance == 0)
        s_instance = new Portlist();

    return s_instance;
}

Portlist* Portlist::getInstance(XML_Helper* helper)
{
    if (s_instance)
        deleteInstance();

    s_instance = new Portlist(helper);
    return s_instance;
}

void Portlist::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

Portlist::Portlist()
{
}

Portlist::~Portlist()
{
  for (iterator it = begin(); it != end(); ++it)
    delete *it;
  d_object.clear();
  d_id.clear();
}

Portlist::Portlist (const Portlist &p, bool sync_ids)
 : LocationList<Port*> (), sigc::trackable (p)
{
  for (auto port : p)
    add (new Port (*port, sync_ids));
}

Portlist::Portlist(XML_Helper* helper)
{
    helper->registerTag(Port::d_tag, sigc::mem_fun(this, &Portlist::load));
}

bool Portlist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(Portlist::d_tag);

    for (const_iterator it = begin(); it != end(); ++it)
        retval &= (*it)->save(helper);
    
    retval &= helper->closeTag();

    return retval;
}

bool Portlist::load(Glib::ustring tag, XML_Helper* helper)
{
    if (tag != Port::d_tag)
    //what has happened?
        return false;
    
    add(new Port(helper));

    return true;
}

void Portlist::reset (Portlist *p)
{
  delete s_instance;
  s_instance = p;
}
