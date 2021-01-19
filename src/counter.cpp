// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2003, 2005 Ulf Lorenz
// Copyright (C) 2007, 2008, 2014, 2021 Ben Asselstine
// Copyright (C) 2008 Ole Laursen
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

#include "counter.h"

#include "xmlhelper.h"

Glib::ustring FL_Counter::d_tag = "counter";

FL_Counter* fl_counter;

FL_Counter::FL_Counter(guint32 start)
    :d_curID(start)
{
}

FL_Counter::FL_Counter (const FL_Counter &c)
 : sigc::trackable (c), d_curID (c.d_curID)
{
}

FL_Counter::FL_Counter(XML_Helper* helper)
{
    helper->getData(d_curID, "curID");
}

void FL_Counter::syncToId(guint32 id)
{
  if (id > d_curID)
    d_curID = id;
}

guint32 FL_Counter::getNextId()
{
  guint32 ret = d_curID;
  d_curID++;
  return ret;
}

bool FL_Counter::save(XML_Helper* helper)
{
    bool retval =true;

    retval &= helper->openTag(FL_Counter::d_tag);
    retval &= helper->saveData("curID", d_curID);
    retval &= helper->closeTag();

    return retval;
}

void FL_Counter::reset (FL_Counter *f)
{
  delete fl_counter;
  fl_counter = f;
}
