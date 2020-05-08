// Copyright (C) 2009, 2014 Ben Asselstine
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

#include "set.h"
#include "tarhelper.h"

Set::Set(Glib::ustring ext, guint32 id, Glib::ustring name, guint32 ts)
  : TarFile("", "", ext), d_id(id), d_name(name), d_license(""), d_info(""),
    d_tileSize(ts), d_scale (1.0)
{
}

Set::Set(const Set &s)
  : TarFile(s), d_id(s.d_id), d_name(s.d_name), d_license(s.d_license),
    d_info(s.d_info), d_tileSize(s.d_tileSize), d_scale(s.d_scale)
{
}

Set::Set(Glib::ustring ext, XML_Helper* helper, Glib::ustring directory)
 :TarFile(directory, "", ext), d_scale(1.0)
{
  helper->getData(d_id, "id");
  helper->getData(d_name, "name");
  helper->getData(d_copyright, "copyright");
  helper->getData(d_license, "license");
  helper->getData(d_info, "info");
}

bool Set::save(XML_Helper *helper) const
{
  bool retval = true;
  retval &= helper->saveData("id", d_id);
  retval &= helper->saveData("name", d_name);
  retval &= helper->saveData("copyright", d_copyright);
  retval &= helper->saveData("license", d_license);
  retval &= helper->saveData("info", d_info);
  return retval;
}
