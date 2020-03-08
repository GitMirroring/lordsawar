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

#include <sigc++/functors/mem_fun.h>

#include <iostream>
#include "font-size.h"

FontSize* FontSize::s_instance = 0;

FontSize* FontSize::getInstance()
{
  if (s_instance == 0)
    s_instance = new FontSize();

  return s_instance;
}

void FontSize::deleteInstance()
{
  if (s_instance)
    delete s_instance;

  s_instance = 0;
}

FontSize::FontSize()
{
  calculate_default_font_height_and_width ();
}

bool FontSize::recalculate ()
{
  double height = d_height, width = d_width;
  calculate_default_font_height_and_width ();
  return d_height != height || d_width != width;
}

void FontSize::calculate_default_font_height_and_width ()
{
  Gtk::HBox *box = new Gtk::HBox ();
  Glib::RefPtr <Pango::Context> cr = box->create_pango_context ();
  Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create (cr);
  layout->set_text ("A");
  Pango::Rectangle ink, logical;
  layout->get_pixel_extents (ink, logical);
  d_height = ink.get_height ();
  d_width = ink.get_width ();
  if (d_height == 0)
    d_height = 11.0;
  if (d_width == 0)
    d_width = 7.0;
}
// End of file
