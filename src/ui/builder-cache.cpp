//  Copyright (C) 2015 Ben Asselstine
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

#include <iostream>
#include "builder-cache.h"
#include "File.h"

BuilderCache* BuilderCache::s_instance = 0;

BuilderCache* BuilderCache::getInstance()
{
  if (s_instance == 0)
    s_instance = new BuilderCache();

  return s_instance;
}

void BuilderCache::deleteInstance()
{
  if (s_instance)
    delete s_instance;

  s_instance = 0;
}

void BuilderCache::preloadAllBuilders(Glib::ustring dir)
{
  std::list<Glib::ustring> f = File::scanForFiles(dir, ".ui");
  for (std::list<Glib::ustring>::iterator i = f.begin(); i != f.end(); ++i)
    {
      //for some reason when we load about-dialog.ui, it gets shown.
      if (File::get_basename ((*i)) == "about-dialog")
        continue;
      try 
        {
          Glib::RefPtr<Gtk::Builder> xml = Gtk::Builder::create_from_file(*i);
          (*this)[File::get_basename(*i, true)] = Glib::RefPtr<Gtk::Builder>(xml);
        } catch (const Glib::Error &ex)
      {
        fprintf(stderr, "Error, couldn't load builder file `%s' (%s)\n", (*i).c_str(), ex.what().c_str());
        exit (1);
      }
    }
}

BuilderCache::BuilderCache()
{
  preloadAllBuilders(File::add_slash_if_necessary(File::getMiscFile("glade")));
}

Glib::RefPtr<Gtk::Builder> BuilderCache::editor_get(Glib::ustring f)
{
  return Gtk::Builder::create_from_file(File::getEditorGladeFile(f));
}

Glib::RefPtr<Gtk::Builder> BuilderCache::get(Glib::ustring f)
{
  //Crapola.  the builder files are being destroyed somehow.
  return Gtk::Builder::create_from_file(File::getGladeFile(f));
  BuilderCache *b = getInstance();
  Glib::ustring k = File::get_basename(f, true);
  std::map<Glib::ustring,Glib::RefPtr<Gtk::Builder> >::iterator i = b->find(k);
  if (i == b->end())
    {
      Glib::RefPtr<Gtk::Builder> xml = Gtk::Builder::create_from_file(File::getGladeFile(f));
      if (xml)
        {
          (*getInstance())[k] = Glib::RefPtr<Gtk::Builder>(xml);
          return (*getInstance())[k];
        }
      else
        fprintf(stderr, "Error, couldn't load builder file `%s'\n", f.c_str());
      Glib::RefPtr<Gtk::Builder> none;
      return none;
    }
  return Glib::RefPtr<Gtk::Builder>((*getInstance())[k]);
}
// End of file
