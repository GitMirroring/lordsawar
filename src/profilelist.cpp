// Copyright (C) 2011, 2014 Ben Asselstine
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

#include <limits.h>
#include <fstream>
#include <iostream>
#include "xmlhelper.h"
#include "Configuration.h"
#include "defs.h"
#include "File.h"
#include "file-compat.h"
#include "profile.h"
#include "profilelist.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Glib::ustring Profilelist::d_tag = "profilelist";

Profilelist* Profilelist::s_instance = 0;

Profilelist* Profilelist::getInstance()
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
  return saveToFile(File::getSaveFile(PROFILE_LIST));
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
  return loadFromFile(File::getSaveFile (PROFILE_LIST));
}

bool Profilelist::loadFromFile(Glib::ustring filename)
{
  std::ifstream in(filename.c_str());
  if (in)
    {
      XML_Helper helper(filename.c_str(), std::ios::in);
      helper.registerTag(Profile::d_tag, 
                         sigc::mem_fun(this, &Profilelist::load_tag));
      bool retval = helper.parseXML();
      helper.close();
      if (retval == false)
	File::erase(filename);
      return retval;
    }
  return true;
}

Profilelist* Profilelist::getInstance(XML_Helper* helper)
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
  helper->registerTag(Profile::d_tag, 
                      sigc::mem_fun(this, &Profilelist::load_tag));
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
  retval &= helper->openTag(Profilelist::d_tag);

  for (const_iterator it = begin(); it != end(); ++it)
    (*it)->save(helper);

  retval &= helper->closeTag();

  return retval;
}

bool Profilelist::load_tag(Glib::ustring tag, XML_Helper* helper)
{
  if (helper->getVersion() != LORDSAWAR_PROFILES_VERSION)
    {
      return false;
    }
  if (tag == Profile::d_tag)
    {
      Profile *p = Profile::handle_load(helper);
      push_back(p);
      return true;
    }
  return false;
}

Profile *Profilelist::findLastPlayedProfileForUser(Glib::ustring user) const
{
  Profile *p = NULL;
  Glib::TimeVal latest = Glib::TimeVal(0,0);
  for (Profilelist::const_iterator i = begin(); i != end(); ++i)
    {
      if ((*i)->getUserName() == user)
        {
          if ((*i)->getLastPlayedOn() > latest)
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
  return FileCompat::getInstance()->upgrade(filename, old_version, new_version,
                                            FileCompat::PROFILELIST, 
                                            d_tag);
}

void Profilelist::support_backward_compatibility()
{
  FileCompat::getInstance()->support_type
    (FileCompat::PROFILELIST, 
     File::get_extension(File::getUserProfilesDescription()), d_tag, false);
  FileCompat::getInstance()->support_version
    (FileCompat::PROFILELIST, "0.2.0", LORDSAWAR_PROFILES_VERSION,
     sigc::ptr_fun(&Profilelist::upgrade));
}

// End of file
