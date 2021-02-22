//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2009, 2011, 2014, 2021 Ben Asselstine
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

#include "image-helpers.h"

std::vector<PixMask*>
disassemble_row(const Glib::ustring &file, int no, bool &broken)
{
  Glib::RefPtr<Gdk::Pixbuf> row;
  try
    {
      if (getenv ("LORDSAWAR_HEADLESS") == NULL)
        row = Gdk::Pixbuf::create_from_file(file);
      else
        broken = true;
    }
  catch (const Glib::Exception &ex)
    {
      broken = true;
    }

  if (broken || !row)
    {
      std::vector<PixMask*> empty;
      return  empty;
    }

    std::vector<Glib::RefPtr<Gdk::Pixbuf> > images;
    images.reserve(no);
  
    int h = row->get_height();
    int w = row->get_width() / no;

    // disassemble row
    for (int x = 0; x < no; ++x) 
      {
        Glib::RefPtr<Gdk::Pixbuf> buf
          = Gdk::Pixbuf::create(row->get_colorspace(),
                                row->get_has_alpha(),
                                row->get_bits_per_sample(),
                                w, h);

        row->copy_area(x * w, 0, w, h, buf, 0, 0);

        images.push_back(buf);
      }
    
    std::vector<PixMask*> pixmasks;
    for (unsigned int i = 0; i < images.size(); i++)
      pixmasks.push_back(PixMask::create(images[i]));

    return pixmasks;
}

std::vector<PixMask*>
disassemble_row(Glib::RefPtr<Gdk::Pixbuf> row, int no, bool first_half_height)
{
    std::vector<Glib::RefPtr<Gdk::Pixbuf> > images;
    images.reserve(no);
  
    int h = row->get_height() / 2;
    int w = row->get_width() / no;

    int s = 0;
    if (first_half_height == false)
      s = h;
    // disassemble row
    for (int x = 0; x < no; ++x) 
      {
        Glib::RefPtr<Gdk::Pixbuf> buf
          = Gdk::Pixbuf::create(row->get_colorspace(),
                                row->get_has_alpha(),
                                row->get_bits_per_sample(),
                                w, h);

        row->copy_area(x * w, s, w, h, buf, 0, 0);

        images.push_back(buf);
      }
    
    std::vector<PixMask*> pixmasks;
    for (unsigned int i = 0; i < images.size(); i++)
      pixmasks.push_back(PixMask::create(images[i]));
    return pixmasks;
}

std::vector<PixMask*>
disassemble_row(const Glib::ustring &file, int no, bool first_half_height, bool &broken)
{
    Glib::RefPtr<Gdk::Pixbuf> row;
    try
      {
        row = Gdk::Pixbuf::create_from_file(file);
      }
  catch (const Glib::Exception &ex)
    {
      broken = true;
    }

  if (broken || !row)
    {
      std::vector<PixMask*> empty;
      return  empty;
    }
  return disassemble_row(row, no, first_half_height);
}

bool image_width_is_multiple_of_image_height(const Glib::ustring file)
{
  Glib::RefPtr<Gdk::Pixbuf> row;
  try
    {
      row = Gdk::Pixbuf::create_from_file(file);
    }
  catch (const Glib::Exception &ex)
    {
      return false;
    }

  guint32 width = row->get_width();
  guint32 height = row->get_height();

  if ((width % height) != 0)
    return false;
  return true;
}

void get_image_width_and_height (const Glib::ustring &file, guint32 &width, guint32 &height, bool &broken)
{
  Glib::RefPtr<Gdk::Pixbuf> row;
  try
    {
      row = Gdk::Pixbuf::create_from_file(file);
    }
  catch (const Glib::Exception &ex)
    {
      broken = true;
      return;
    }
  width = row->get_width();
  height = row->get_height();
}
