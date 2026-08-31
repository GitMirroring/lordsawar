//  Copyright (C) 2011, 2014, 2026 Ben Asselstine
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

//#include <iostream>
#include <fstream>
#include <sstream>
#include "profile.h"
#include "xml-helper.h"
#include "ucompose.hpp"
#include "game-scenario.h"

Glib::ustring Profile::d_tag = "profile";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Profile::Profile(Glib::ustring nickname)
 : d_id (GameScenario::generate_guid ()), d_nickname (nickname),
    d_user (Glib::get_user_name ()), d_creation_date (Glib::DateTime ()),
    d_last_played_date (Glib::DateTime ())
{
  d_creation_date = Glib::DateTime::create_now_local ();
  d_last_played_date = Glib::DateTime::create_now_local ();
}

Profile::Profile (XML_Helper* helper)
{
  helper->get (d_id, "id");
  helper->get (d_nickname, "nickname");
  helper->get (d_user, "user");
  Glib::ustring s;
  helper->get (s, "created_on");
  if (s != "")
    d_creation_date = Glib::DateTime::create_from_iso8601 (s);
  else
    d_creation_date = Glib::DateTime::create_now_local ();
  helper->get (s, "last_played_on");
  if (s != "")
    d_last_played_date = Glib::DateTime::create_from_iso8601 (s);
  else
    d_last_played_date = Glib::DateTime::create_now_local ();
}
        
Profile::Profile(const Profile &orig)
  : d_id(orig.d_id), d_nickname(orig.d_nickname), d_user(orig.d_user),
    d_creation_date(orig.d_creation_date), 
    d_last_played_date(orig.d_last_played_date)
{
}

bool Profile::saveContents(XML_Helper *helper) const
{
  bool retval = true;
  retval &= helper->save("id", d_id);
  retval &= helper->save("nickname", d_nickname);
  retval &= helper->save("user", d_user);
  Glib::ustring s = d_creation_date.format_iso8601();
  retval &= helper->save("created_on", s);
  s = d_last_played_date.format_iso8601();
  retval &= helper->save("last_played_on", s);
  return retval;
}

Profile* Profile::handle_load(XML_Helper *helper)
{
  return new Profile(helper);
}

bool Profile::save(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->open_tag(Profile::d_tag);
  retval &= saveContents(helper);
  retval &= helper->close_tag();
  return retval;
}

void Profile::play()
{
  d_last_played_date = Glib::DateTime::create_now_local();
}

