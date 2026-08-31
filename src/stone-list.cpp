//  Copyright (C) 2017, 2021 Ben Asselstine
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

#include "stonelist.h"
#include "stone.h"
#include "GameMap.h"
#include "xmlhelper.h"

Glib::ustring Stonelist::d_tag = "stonelist";
//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Stonelist* Stonelist::s_instance=0;

Stonelist* Stonelist::getInstance()
{
    if (s_instance == 0)
        s_instance = new Stonelist();

    return s_instance;
}

Stonelist* Stonelist::getInstance(XML_Helper* helper)
{
    if (s_instance)
        deleteInstance();

    s_instance = new Stonelist(helper);
    return s_instance;
}

void Stonelist::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

Stonelist::Stonelist()
{
}

Stonelist::~Stonelist()
{
  for (iterator it = begin(); it != end(); ++it)
    delete *it;
  d_object.clear();
  d_id.clear();
}
Stonelist::Stonelist (const Stonelist &s, bool sync_ids)
 : LocationList<Stone*> (), sigc::trackable (s)
{
  for (auto stone : s)
    add (new Stone (*stone, sync_ids));
}

Stonelist::Stonelist(XML_Helper* helper)
{
    helper->registerTag(Stone::d_tag, sigc::mem_fun(this, &Stonelist::load));
}

bool Stonelist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(Stonelist::d_tag);

    for (const_iterator it = begin(); it != end(); ++it)
        retval &= (*it)->save(helper);
    
    retval &= helper->closeTag();

    return retval;
}

bool Stonelist::load(Glib::ustring tag, XML_Helper* helper)
{
    if (tag != Stone::d_tag)
    //what has happened?
        return false;
    
    add(new Stone(helper));

    return true;
}

void Stonelist::reset (Stonelist *s)
{
  delete s_instance;
  s_instance = s;
}
