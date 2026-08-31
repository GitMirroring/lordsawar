//  Copyright (C) 2020, 2021, 2026 Ben Asselstine
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
#include "tar-file-image.h"
#include "pixmask.h"
#include "tar-helper.h"
#include "xml-helper.h"
#include "tar-file.h"
#include "image-helpers.h"
#include "file.h"

TarFileImage::TarFileImage (guint32 n, PixMask::DimensionType d)
 : tarfile (NULL), name (""), file_on_disk (""),
    dimension (Vector<int>(-1,-1)),
    dimension_type (d), image (NULL), number_of_frames (n), frames ({})
{
}

TarFileImage::TarFileImage (const TarFileImage &i)
 : tarfile (i.tarfile), name (i.name), file_on_disk (i.file_on_disk),
    dimension (i.dimension),
    dimension_type (i.dimension_type), image (NULL),
    number_of_frames (i.number_of_frames)
{
  if (i.image)
    image = i.image->copy ();
  else
    return;
  frames.clear ();
  for (auto f : i.frames)
    frames.push_back (f->copy ());
}

bool TarFileImage::load (TarFile *ta, Glib::ustring bname)
{
  if (bname.empty () == true)
    return false;
  setName (bname);
  bool broken = false;
  Tar_Helper t(ta->getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return broken;
  tarfile = &t;
  broken = load ();
  tarfile = NULL;
  t.Close ();
  return broken;
}

bool TarFileImage::load (Tar_Helper *t, Glib::ustring bname)
{
  tarfile = t;
  return load (bname);
}

bool TarFileImage::load ()
{
  if (name.empty () == true)
    return false;
  return load (name);
}

bool TarFileImage::load (Tar_Helper *t)
{
  tarfile = t;
  return load (name);
}

bool TarFileImage::load (Glib::ustring bname)
{
  bool broken = false;
  if (name.empty () == true)
    return broken;
  Glib::ustring filename = tarfile->getFile(bname, broken);
  if (!broken)
    broken = loadFromFile (filename);
  return broken;
}

bool TarFileImage::loadFromFile (Glib::ustring filename)
{
  bool broken = false;
  if (filename.empty () == true)
    return false;
  PixMask *p = PixMask::create (filename, broken);
  if (!broken)
    {
      if (image)
        delete image;
      image = p;
      file_on_disk = filename;
      int size =
        p->get_width () / number_of_frames;
      frames.resize (number_of_frames);
      dimension = Vector<int>(size,size);
    }

  return broken;
}

bool TarFileImage::instantiateImages ()
{
  if (!image)
    return false;
  uninstantiateImages ();

  int h = image->get_height ();
  int w = image->get_width () / number_of_frames;

  frames.clear ();
  Glib::RefPtr<Gdk::Pixbuf> row = image->to_pixbuf ();
  for (guint32 i = 0; i < number_of_frames; ++i)
    {
      Glib::RefPtr<Gdk::Pixbuf> buf = Gdk::Pixbuf::create (Gdk::Colorspace::RGB,
                                                           true, 8, w, h);
      row->copy_area (i * w, 0, w, h, buf, 0, 0);

      PixMask *p = PixMask::create (buf);
      frames.push_back (p);
    }
  return true;
}

void TarFileImage::uninstantiateImages ()
{
  guint32 oldsize = frames.size ();
  for (guint32 i = 0; i < frames.size (); i++)
    {
      PixMask *p = frames[i];
      if (p)
        {
          delete p;
          frames[i] = NULL;
        }
    }
  frames.clear ();
  frames.resize (oldsize);
}

void TarFileImage::clear (bool clear_name)
{
  if (clear_name)
    name = "";

  dropBackingImage ();
  uninstantiateImages ();
}

void TarFileImage::dropBackingImage ()
{
  if (image)
    delete image;
  image = NULL;
}

TarFileImage::~TarFileImage ()
{
  dropBackingImage ();
  uninstantiateImages ();
}

void TarFileImage::load_name (XML_Helper *helper, Glib::ustring data_tag)
{
  Glib::ustring n;
  helper->get(n, data_tag);
  std::string na = n;
  File::add_png_if_no_ext (na);
  setName (na);
}

void TarFileImage::uninstantiate (Glib::ustring name, std::vector<TarFileImage*> images)
{
  for (auto i : images)
    if (i->getName () == name)
      i->clear ();
}

bool TarFileImage::checkDimension (Glib::ustring f)
{
  return PixMask::checkDimension (f, dimension_type);
}
