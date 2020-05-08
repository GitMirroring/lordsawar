//  Copyright (C) 2008, 2014 Ben Asselstine
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

#include "past-chooser.h"

PastChooser * PastChooser::s_instance = 0;

PastChooser* PastChooser::getInstance()
{
  if (s_instance == 0)
    s_instance = new PastChooser();

  return s_instance;
}

void PastChooser::deleteInstance()
{
  if (s_instance)
    delete s_instance;

  s_instance = 0;
}

PastChooser::PastChooser()
 : pattern_dir (std::map<Glib::ustring,Glib::ustring>())
{
}
    
void PastChooser::set_dir(Glib::RefPtr<Gtk::FileFilter> f, Glib::ustring dir)
{
  pattern_dir[f->get_name()] = dir;
}

Glib::ustring PastChooser::get_dir(Glib::RefPtr<Gtk::FileFilter> filter)
{
  if (!filter)
    return "";
  std::map<Glib::ustring,Glib::ustring>::iterator it = 
    pattern_dir.find(filter->get_name());
  if (it == pattern_dir.end())
    return "";
  return (*it).second;
}

void PastChooser::set_dir(Gtk::FileChooser *filechooser)
{
  if (filechooser->get_filter())
    set_dir (filechooser->get_filter(), filechooser->get_current_folder());
}

Glib::ustring PastChooser::get_dir(Gtk::FileChooser *filechooser)
{
  return get_dir(filechooser->get_filter());
}
