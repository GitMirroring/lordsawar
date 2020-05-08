//  Copyright (C) 2020 Ben Asselstine
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

#include <assert.h>
#include <algorithm>
#include <sigc++/functors/mem_fun.h>

#include "scenario-list.h"
#include "scenario-details.h"
#include "File.h"

ScenarioList* ScenarioList::s_instance = 0;

ScenarioList* ScenarioList::getInstance()
{
  if (s_instance == 0)
    s_instance = new ScenarioList();

  return s_instance;
}

void ScenarioList::deleteInstance()
{
  if (s_instance)
    delete s_instance;

  s_instance = 0;
}

ScenarioList::ScenarioList()
{
  bool broken;
  for (auto i : File::scanMaps ())
    {
      broken = false;
      ScenarioDetails *scen =
        new ScenarioDetails (File::getMapFile (i), broken);
      if (!broken)
        push_back (scen);
      else
        delete scen;
    }
  for (auto i : File::scanUserMaps())
    {
      broken = false;
      ScenarioDetails *scen =
        new ScenarioDetails (File::getUserMapFile(i), broken);
      if (!broken)
        push_back (scen);
      else
        delete scen;
    }
  sort (compare);
}

bool ScenarioList::compare(const ScenarioDetails *lhs, const ScenarioDetails *rhs)
{
  return lhs->getName().compare (rhs->getName ()) < 0;
}

ScenarioList::~ScenarioList()
{
  for (ScenarioList::iterator it = begin(); it != end(); ++it)
    delete *it;
}

bool ScenarioList::add_file (Glib::ustring filename)
{
  /* this could be expanded by copying it into our user map dir
   * and deconflicting filenames */
  bool broken = false;
  ScenarioDetails *d = new ScenarioDetails (filename, broken);
  if (!broken)
    push_back (d);
  else
    delete d;
  return !broken;
}

bool ScenarioList::remove_file (Glib::ustring filename)
{
  std::list<ScenarioDetails*> del;
  for (iterator i = begin (); i != end (); ++i)
    if ((*i)->getFilename () == filename)
      del.push_back (*i);

  del.reverse ();

  bool success = true;
  for (auto f : del)
    {
      success = File::erase (f->getFilename ());
      if (success)
        remove (f);
      break;
    }
  return success;
}

Glib::ustring ScenarioList::findFreeName (Glib::ustring name)
{
  Glib::ustring new_name;
  guint32 max = 1000;
  for (unsigned int count = 1; count < max; count++)
    {
      new_name = String::ucompose("%1 %2", name, count);
      bool found = false;
      for (iterator i = begin (); i != end (); ++i)
        {
          if ((*i)->getName () == new_name)
            {
              found = true;
              break;
            }
        }

      if (found == false)
        return new_name;
      else
        new_name = "";
    }
  return "";
}
