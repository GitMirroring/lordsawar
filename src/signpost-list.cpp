//  Copyright (C) 2007, 2008, 2009, 2014, 2020, 2021 Ben Asselstine
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

#include "signpostlist.h"
#include "xmlhelper.h"

Glib::ustring Signpostlist::d_tag = "signpostlist";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Signpostlist* Signpostlist::s_instance=0;

Signpostlist* Signpostlist::getInstance()
{
    if (s_instance == 0)
        s_instance = new Signpostlist();

    return s_instance;
}

Signpostlist* Signpostlist::getInstance(XML_Helper* helper)
{
    if (s_instance)
        deleteInstance();

    s_instance = new Signpostlist(helper);
    return s_instance;
}

void Signpostlist::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

Signpostlist::Signpostlist()
{
}

Signpostlist::~Signpostlist()
{
  for (iterator it = begin(); it != end(); ++it)
    delete *it;
  d_object.clear();
  d_id.clear();
}

Signpostlist::Signpostlist (const Signpostlist &s, bool sync_ids)
 : LocationList<Signpost*> (), sigc::trackable (s)
{
  for (auto signpost : s)
    add (new Signpost (*signpost, sync_ids));
}

Signpostlist::Signpostlist(XML_Helper* helper)
{
    helper->registerTag(Signpost::d_tag, sigc::mem_fun(this, &Signpostlist::load));
}

bool Signpostlist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(Signpostlist::d_tag);

    for (const_iterator it = begin(); it != end(); ++it)
        retval &= (*it)->save(helper);
    
    retval &= helper->closeTag();

    return retval;
}

bool Signpostlist::load(Glib::ustring tag, XML_Helper* helper)
{
    if (tag != Signpost::d_tag)
    //what has happened?
        return false;
    
    add(new Signpost(helper));

    return true;
}

guint32 Signpostlist::countUnamedSignposts () const
{
  guint32 count = 0;
  for (const_iterator it = begin (); it != end (); ++it)
    if ((*it)->getName () == DEFAULT_SIGNPOST)
      count++;
  return count;
}

void Signpostlist::reset (Signpostlist *s)
{
  delete s_instance;
  s_instance = s;
}
