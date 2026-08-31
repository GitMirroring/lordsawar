//  Copyright (C) 2026 Ben Asselstine
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
#include "tar-file-sound.h"
#include "tar-helper.h"
#include "xml-helper.h"
#include "tar-file.h"
#include "file.h"

TarFileSound::TarFileSound ()
 : m_tarfile (NULL), m_name (""), m_file_on_disk ("")
{
}

TarFileSound::TarFileSound (const TarFileSound &i)
 : m_tarfile (i.m_tarfile), m_name (i.m_name), m_file_on_disk (i.m_file_on_disk)
{
}

bool TarFileSound::load (TarFile *ta, Glib::ustring bname)
{
  if (bname.empty () == true)
    return false;
  set_name (bname);
  bool broken = false;
  Tar_Helper t(ta->getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return broken;
  m_tarfile = &t;
  broken = load ();
  m_tarfile = NULL;
  t.Close ();
  return broken;
}

bool TarFileSound::load (Tar_Helper *t, Glib::ustring bname)
{
  m_tarfile = t;
  return load (bname);
}


bool TarFileSound::load ()
{
  if (m_name.empty () == true)
    return false;
  return load (m_name);
}

bool TarFileSound::load (Tar_Helper *t)
{
  m_tarfile = t;
  return load (m_name);
}

bool TarFileSound::load (Glib::ustring bname)
{
  bool broken = false;
  if (m_name.empty () == true)
    return broken;
  Glib::ustring filename = m_tarfile->getFile (bname, broken);
  if (!broken)
    broken = loadFromFile (filename);
  return broken;
}

bool TarFileSound::loadFromFile (Glib::ustring filename)
{
  m_file_on_disk = filename;
  return false;
}

TarFileSound::~TarFileSound ()
{
}

void TarFileSound::load_name (XML_Helper *helper, Glib::ustring data_tag)
{
  Glib::ustring n;
  helper->get(n, data_tag);
  std::string na = n;
  set_name (na);
}

