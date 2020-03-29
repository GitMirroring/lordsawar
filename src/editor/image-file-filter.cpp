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

#include <config.h>
#include <ostream>
#include <iostream>
#include <sstream>

#include "defs.h"
#include "File.h"
#include "image-file-filter.h"

ImageFileFilter* ImageFileFilter::s_instance = 0;

void ImageFileFilter::add (Gtk::FileChooserDialog *d)
{
  Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
  filter->set_name(_("Image files (*.png, *.svg)"));
  filter->add_pattern("*.png");
  filter->add_pattern("*.svg");
  d->add_filter(filter);
}

ImageFileFilter * ImageFileFilter::getInstance()
{
    if (s_instance == 0)
        s_instance = new ImageFileFilter ();

    return s_instance;
}

void ImageFileFilter::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

bool ImageFileFilter::hasInvalidExt (Glib::ustring filename)
{
  if (filename == "")
    return true;
  if (File::nameEndsWith(filename, ".png") ||
      File::nameEndsWith(filename, ".svg"))
    return false;
  return true;
}
          
void ImageFileFilter::showErrorDialog(Gtk::Dialog *d)
{
  Gtk::MessageDialog e(*d, _("Only PNG and SVG files can be used as images."));
  e.run();
  e.hide();
}
