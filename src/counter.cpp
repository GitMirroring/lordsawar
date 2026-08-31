//  Copyright (C) 2003 Michael Bartl
//  Copyright (C) 2003, 2005 Ulf Lorenz
//  Copyright (C) 2007, 2008, 2014, 2021, 2026 Ben Asselstine
//  Copyright (C) 2008 Ole Laursen
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

#include "counter.h"

#include "xml-helper.h"

#include "defs.h"
Glib::ustring ID_Counter::d_tag = "counter";

ID_Counter* id_counter;

ID_Counter::ID_Counter ()
{
  //we reserve the first bunch of ids for players
  //and those ids equate to what we see in shield.h for Shield::Color.
  //8 is neutral, so that means 9 is our first id
  m_current_id = MAX_PLAYERS + 1;
}

ID_Counter::ID_Counter (const ID_Counter &c)
 : sigc::trackable (c), m_current_id (c.m_current_id)
{
}

ID_Counter::ID_Counter (XML_Helper* helper)
{
  helper->get (m_current_id, "curID");
}

void ID_Counter::sync_to_id (guint32 id)
{
  if (id > m_current_id)
    m_current_id = id;
}

guint32 ID_Counter::get_next_id ()
{
  guint32 ret = m_current_id;
  m_current_id++;
  return ret;
}

bool ID_Counter::save (XML_Helper* helper)
{
  bool retval =true;

  retval &= helper->open_tag (ID_Counter::d_tag);
  retval &= helper->save ("curID", m_current_id);
  retval &= helper->close_tag ();

  return retval;
}

void ID_Counter::reset (ID_Counter *f)
{
  delete id_counter;
  id_counter = f;
}
