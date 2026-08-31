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

#include <sigc++/functors/mem_fun.h>

#include <limits.h>
#include <fstream>
#include <iostream>
#include "xml-helper.h"
#include "configuration.h"
#include "defs.h"
#include "file.h"
#include "file-compat.h"
#include "profile.h"
#include "profile-list.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Glib::ustring Profilelist::d_tag = "profilelist";

Profilelist* Profilelist::s_instance = 0;

Profilelist* Profilelist::instance()
{
  if (s_instance == 0)
    {
      s_instance = new Profilelist();
      s_instance->load();
    }

  return s_instance;
}

bool Profilelist::save() const
{
  return saveToFile(File::getUserProfilesDescription());
}

bool Profilelist::saveToFile(Glib::ustring filename) const
{
  bool retval = true;
  XML_Helper helper(filename, std::ios::out);
  retval &= save(&helper);
  helper.close();
  return retval;
}

bool Profilelist::load()
{
  return loadFromFile(File::getUserProfilesDescription ());
}

bool Profilelist::loadFromFile (Glib::ustring filename)
{
  std::ifstream in (filename.c_str ());
  if (in)
    {
      XML_Helper helper (filename.c_str (), std::ios::in);
      instance (&helper);
      bool retval = helper.parse_XML ();
      helper.close ();
      if (retval == false)
	File::erase (filename);
      return retval;
    }
  else
    {
      // file not found? create our defaults
      createDefaultProfile ();
      createAdminProfile ();
      save ();
    }
  return true;
}

Profilelist* Profilelist::instance(XML_Helper* helper)
{
  if (s_instance)
    deleteInstance();

  s_instance = new Profilelist(helper);
  return s_instance;
}

void Profilelist::deleteInstance()
{
  if (s_instance)
    delete s_instance;

  s_instance = 0;
}

Profilelist::Profilelist()
{
}

Profilelist::Profilelist(XML_Helper* helper)
{
  helper->register_tag(Profile::d_tag, 
                      sigc::mem_fun(*this, &Profilelist::load_tag));
  helper->register_tag(Profilelist::d_tag, 
                      sigc::mem_fun(*this, &Profilelist::load_tag));
}

Profilelist::~Profilelist()
{
  for (Profilelist::iterator it = begin(); it != end(); ++it)
    delete *it;
}

bool Profilelist::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->begin(LORDSAWAR_PROFILES_VERSION);
  retval &= helper->open_tag(Profilelist::d_tag);

  helper->save ("default", d_default);

  for (const_iterator it = begin(); it != end(); ++it)
    (*it)->save(helper);

  retval &= helper->close_tag();

  return retval;
}

bool Profilelist::load_tag(Glib::ustring tag, XML_Helper* helper)
{
  if (helper->get_version() != LORDSAWAR_PROFILES_VERSION)
    {
      return false;
    }
  if (tag == Profile::d_tag)
    {
      Profile *p = Profile::handle_load(helper);
      push_back(p);
      return true;
    }
  else if (tag == Profilelist::d_tag)
    {
      helper->get (d_default, "default");
      return true;
    }
  return false;
}

Profile *Profilelist::findLastPlayedProfileForUser(Glib::ustring user) const
{
  Profile *p = NULL;
  Glib::DateTime latest =
    Glib::DateTime::create_local (1900, 1, 1, 0, 0, 0);
  for (Profilelist::const_iterator i = begin(); i != end(); ++i)
    {
      if ((*i)->getUserName() == user)
        {
          if ((*i)->getLastPlayedOn().to_unix () > latest.to_unix ())
            {
              p = (*i);
              latest = (*i)->getLastPlayedOn();
            }
        }
    }
  return p;
}
        
Profile *Profilelist::findProfileById(Glib::ustring id) const
{
  for (Profilelist::const_iterator i = begin(); i != end(); ++i)
    {
      if ((*i)->getId() == id)
        return *i;
    }
  return NULL;
}

bool Profilelist::upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version)
{
  return FileCompat::instance()->upgrade(filename, old_version, new_version,
                                            FileCompat::PROFILELIST, 
                                            d_tag);
}

void Profilelist::support_backward_compatibility()
{
  FileCompat::instance()->support_type
    (FileCompat::PROFILELIST, 
     File::get_extension(File::getUserProfilesDescription()), d_tag, false);
  FileCompat::instance()->support_version
    (FileCompat::PROFILELIST, "0.2.0", "0.3.0",
     sigc::ptr_fun(&Profilelist::upgrade));
  FileCompat::instance()->support_version
    (FileCompat::PROFILELIST, "0.3.0", "0.4.0",
     sigc::ptr_fun(&Profilelist::upgrade));
}
        
Profile *Profilelist::getDefaultProfile () const
{
  if (d_default.empty ())
    return NULL;
  auto p = findProfileById (d_default);
  return p;
}
