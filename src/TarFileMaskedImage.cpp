// Copyright (C) 2020, 2021 Ben Asselstine
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

#include <iostream>
#include "TarFileMaskedImage.h"
#include "PixMask.h"
#include "tarhelper.h"
#include "xmlhelper.h"
#include "tarfile.h"
#include "gui/image-helpers.h"
#include "File.h"
#include "player.h"
#include "ucompose.hpp"

TarFileMaskedImage::TarFileMaskedImage (MaskOrientation o)
 : orientation (o), tarfile (NULL), name (""), file_on_disk (""),
    scale_dimension (Vector<int>(-1,-1)), dimension (Vector<int>(-1,-1)),
    image (NULL), calculated_number_of_frames (0)
{
}

TarFileMaskedImage::TarFileMaskedImage (const TarFileMaskedImage &i)
 : orientation (i.orientation), tarfile (i.tarfile), name (i.name),
    file_on_disk (i.file_on_disk), scale_dimension (i.scale_dimension),
    dimension (i.dimension), image (NULL),
    calculated_number_of_frames (i.calculated_number_of_frames)
{
  if (i.image)
    image = i.image->copy ();
  frames.clear ();
  for (auto f : i.frames)
    frames.push_back (std::make_pair (f.first->copy (), f.second->copy ()));
}

bool TarFileMaskedImage::load (TarFile *ta, Glib::ustring bname)
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

bool TarFileMaskedImage::load (Tar_Helper *t, Glib::ustring bname)
{
  tarfile = t;
  return load (bname);
}

bool TarFileMaskedImage::load ()
{
  if (name.empty () == true)
    return false;
  return load (name);
}

bool TarFileMaskedImage::load (Tar_Helper *t)
{
  tarfile = t;
  return load (name);
}

bool TarFileMaskedImage::load (Glib::ustring bname)
{
  bool broken = false;
  if (name.empty () == true)
    return broken;
  Glib::ustring filename = tarfile->getFile(bname, broken);
  if (!broken)
    broken = loadFromFile (filename);
  return broken;
}

bool TarFileMaskedImage::loadFromFile (Glib::ustring filename)
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
      int size = 0;
      if (orientation == HORIZONTAL_MASK)
        {
          size = p->get_unscaled_width () / 2;
          calculated_number_of_frames = 1;
          frames.resize (calculated_number_of_frames);
        }
      else if (orientation == VERTICAL_MASK)
        {
          size = p->get_unscaled_height () / 2;
          calculated_number_of_frames = p->get_unscaled_width () / size;
          frames.resize (calculated_number_of_frames);
        }
      dimension = Vector<int>(size,size);
    }

  return broken;
}

void TarFileMaskedImage::instantiateImages (Vector<int> scale_to_dimension)
{
  uninstantiateImages ();
  scale_dimension = scale_to_dimension;
  if (orientation == HORIZONTAL_MASK)
    instantiateHorizontal ();
  else
    instantiateVertical ();
}

void TarFileMaskedImage::instantiateHorizontal ()
{
  if (frames.empty () == false)
    {
      if (frames[0].first)
        delete frames[0].first;
      if (frames[0].second)
        delete frames[0].second;
      frames.clear ();
    }
  frames.push_back (std::make_pair (image->cropLeftHalf (), image->cropRightHalf ()));
}

void TarFileMaskedImage::instantiateVertical ()
{
  guint32 s = calculated_number_of_frames;
  if (s == 0)
    return;

  bool scale =
    scale_dimension != Vector<int>(-1,-1) &&
    scale_dimension != dimension;

  std::vector<PixMask *> images =
    disassemble_row(image->to_pixbuf (), s, true);
  if (scale)
    for (guint32 i = 0; i < images.size (); i++)
      PixMask::scale(images[i], scale_dimension.x, scale_dimension.y);

  std::vector<PixMask *> masks = disassemble_row(image->to_pixbuf (), s, false);
  if (scale)
    for (guint32 i = 0; i < masks.size (); i++)
      PixMask::scale(masks[i], scale_dimension.x, scale_dimension.y);

  frames.clear ();
  for (guint32 i = 0; i < images.size (); i++)
    frames.push_back (std::make_pair (images[i], masks[i]));
  return;
}

void TarFileMaskedImage::uninstantiateImages ()
{
  guint32 oldsize = frames.size ();
  for (guint32 i = 0; i < frames.size (); i++)
    {
      PixMask *p = frames[i].first;
      if (p)
        {
          delete p;
          frames[i].first = NULL;
        }
      p = frames[i].second;
      if (p)
        {
          delete p;
          frames[i].second = NULL;
        }
    }
  frames.clear ();
  frames.resize (oldsize);
}

void TarFileMaskedImage::clear (bool clear_name)
{
  if (clear_name)
    name = "";

  uninstantiateImages ();
}

void TarFileMaskedImage::dropBackingImage ()
{
  if (image)
    delete image;
  image = NULL;
}

TarFileMaskedImage::~TarFileMaskedImage ()
{
  dropBackingImage ();
  uninstantiateImages ();
}

void TarFileMaskedImage::load_name (XML_Helper *helper, Glib::ustring data_tag)
{
  Glib::ustring n;
  helper->getData(n, data_tag);
  File::add_png_if_no_ext (n);
  setName (n);
}

PixMask *TarFileMaskedImage::applyMask (Gdk::RGBA colour) const
{
  return applyMask(frames[0].first, frames[0].second, colour);
}

PixMask *TarFileMaskedImage::applyMask (Player *p) const
{
  return applyMask (p->getColor ());
}

PixMask *TarFileMaskedImage::applyMask (guint32 i, Gdk::RGBA colour) const
{
  return applyMask(frames[i].first, frames[i].second, colour);
}

PixMask *TarFileMaskedImage::applyMask (guint32 i, Player *p) const
{
  return applyMask (i, p->getColor ());
}

PixMask* TarFileMaskedImage::applyMask(PixMask* im, PixMask* ma, Gdk::RGBA colour) const
{
  int width = im->get_width();
  int height = im->get_height();
  PixMask* result = PixMask::create(im->get_pixmap(), ma->get_pixmap());
  if (!result)
    return NULL;
  if (ma->get_width() != width || (ma->get_height()) != height)
    {
      std::cerr <<"Warning: mask and original image do not match\n";
      return NULL;
    }
  Glib::RefPtr<Gdk::Pixbuf> maskbuf = ma->to_pixbuf();

  guint8 *data = maskbuf->get_pixels();
  guint8 *copy = (guint8*)  malloc (height * width * 4 * sizeof(guint8));
  memcpy(copy, data, height * width * 4 * sizeof(guint8));
  for (int i = 0; i < width; i++)
    for (int j = 0; j < height; j++)
      {
	const int base = (j * 4) + (i * height * 4);

	if (copy[base+3] != 0)
	  {
	    copy[base+0] = colour.get_red() *copy[base+0];
	    copy[base+1] = colour.get_green() * copy[base+1];
	    copy[base+2] = colour.get_blue() * copy[base+2];
	  }
      }
  Glib::RefPtr<Gdk::Pixbuf> colouredmask =
    Gdk::Pixbuf::create_from_data(copy, Gdk::COLORSPACE_RGB, true, 8,
				  width, height, width * 4);
  result->draw_pixbuf(colouredmask, 0, 0, 0, 0, width, height);
  free(copy);

  return result;
}

bool TarFileMaskedImage::copy (TarFile *t, TarFileMaskedImage *dest)
{
  bool success = false;
  Glib::ustring newname;
  if (getName ().empty () == true)
    return false;
  Glib::ustring filename = t->getFileFromConfigurationFile (getName ());
  /*
   * we have to copy the file out of the way because there are
   * intermediate Close operations on the tarfile which deletes it.
   */
  Glib::ustring tmp_dir = File::get_tmp_file ();
  File::create_dir (tmp_dir);
  Glib::ustring bname = getName ();
  Glib::ustring destfile = String::ucompose ("%1/%2", tmp_dir, bname);
  File::copy (filename, destfile);

  if (dest->getName ().empty () == true)
    success = t->addFileInCfgFile (destfile, newname);
  else
    success = t->replaceFileInCfgFile (dest->getName (), destfile, newname);

  if (success)
    {
      dest->load (t, newname);
      dest->instantiateImages ();
    }
  File::erase (destfile);
  File::erase_dir (tmp_dir);
  return success;
}

void TarFileMaskedImage::uninstantiate (Glib::ustring name, std::vector<TarFileMaskedImage*> images)
{
  for (auto i : images)
    if (i->getName () == name)
      i->clear ();
}
