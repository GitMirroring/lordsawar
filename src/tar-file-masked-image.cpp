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
#include <numeric>
#include "tar-file-masked-image.h"
#include "pixmask.h"
#include "tar-helper.h"
#include "xml-helper.h"
#include "tar-file.h"
#include "image-helpers.h"
#include "file.h"
#include "player.h"
#include "game-map.h"
#include "shield-set.h"
#include "ucompose.hpp"

TarFileMaskedImage::TarFileMaskedImage (MaskOrientation o, PixMask::DimensionType d)
 : orientation (o), tarfile (NULL), name (""), file_on_disk (""),
    dimension (Vector<int>(-1,-1)),
    dimension_type (d), image (NULL), calculated_number_of_frames (0),
    maskcount(1)
{
}

TarFileMaskedImage::TarFileMaskedImage (const TarFileMaskedImage &i)
 : orientation (i.orientation), tarfile (i.tarfile), name (i.name),
    file_on_disk (i.file_on_disk),
    dimension (i.dimension), dimension_type (i.dimension_type), image (NULL),
    calculated_number_of_frames (i.calculated_number_of_frames),
    maskcount(i.maskcount)
{
  frames.clear ();
  if (i.image)
    image = i.image->copy ();
  else
    return;
  for (auto frame : i.frames)
    {
      std::vector<PixMask*> newframe;
      for (auto f : frame)
        newframe.push_back (f->copy ());
      frames.push_back (newframe);
    }
}

bool TarFileMaskedImage::load (TarFile *ta, std::string bname)
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

bool TarFileMaskedImage::load (Tar_Helper *t, std::string bname)
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

bool TarFileMaskedImage::load (std::string bname)
{
  bool broken = false;
  if (name.empty () == true)
    return broken;
  std::string filename = tarfile->getFile(bname, broken);
  if (!broken)
    broken = loadFromFile (filename);
  return broken;
}

bool TarFileMaskedImage::loadFromFile (std::string filename)
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
      if (orientation == VERTICAL_MASK)
        {
          calculateNumberOfMasks (filename, broken);
          size = p->get_height () / (maskcount + 1);
          calculated_number_of_frames = p->get_width () / size;
          //some images aren't square, so we can sometimes end up with 0
          if (calculated_number_of_frames == 0)
            calculated_number_of_frames = 1;
        }
      dimension = Vector<int>(size,size);
    }

  return broken;
}

void TarFileMaskedImage::instantiateImages ()
{
  uninstantiateImages ();
  if (image == NULL)
    return;
  if (orientation == VERTICAL_MASK)
    instantiateVertical ();
}

std::vector<PixMask*> TarFileMaskedImage::disassemble_row(int cols)
{
  Glib::RefPtr<Gdk::Pixbuf> row = image->to_pixbuf ();

  std::vector<Glib::RefPtr<Gdk::Pixbuf> > images;
  images.reserve(cols);

  int h = row->get_height();
  int w = row->get_width() / cols;

  // disassemble row
  for (int x = 0; x < cols; ++x) 
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

std::vector<std::vector<PixMask*> > TarFileMaskedImage::disassemble_grid (int rows, int cols)
{
  std::vector<std::vector<PixMask*> > result;
  guint32 h = image->get_height () / rows;
  guint32 w = image->get_width () / cols;
  Glib::RefPtr<Gdk::Pixbuf> im = image->to_pixbuf ();
  for (int i = 0; i < cols; i++)
    {
      std::vector<PixMask*> frame;
      for (int j = 0; j < rows; j++) 
        {
          Glib::RefPtr<Gdk::Pixbuf> buf
            = Gdk::Pixbuf::create(im->get_colorspace(),
                                  im->get_has_alpha(),
                                  im->get_bits_per_sample(),
                                  w, h);

          im->copy_area(i * w, j * h, w, h, buf, 0, 0);

          frame.push_back (PixMask::create (buf));
        }
      result.push_back (frame);
    }
  return result;
}

void TarFileMaskedImage::instantiateVertical ()
{
  guint32 s = calculated_number_of_frames;
  if (s == 0)
    return;

  frames = disassemble_grid (maskcount + 1, s);
  return;
}

void TarFileMaskedImage::uninstantiateImages ()
{
  for (guint32 i = 0; i < frames.size (); i++)
    {
      for (auto f : frames[i])
        delete f;
    }
  frames.clear ();
}

void TarFileMaskedImage::clear (bool clear_name)
{
  if (clear_name)
    name = "";

  dropBackingImage ();
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

bool TarFileMaskedImage::save (XML_Helper *helper, std::string name_tag)
{
  bool retval = true;
  retval &= helper->save(name_tag, getName ());
  return retval;
}

void TarFileMaskedImage::load (XML_Helper *helper, std::string name_tag)
{
  Glib::ustring n;
  helper->get(n, name_tag);
  std::string na = n;
  File::add_png_if_no_ext (na);
  setName (na);
}

PixMask *TarFileMaskedImage::applyMask (std::vector<Gdk::RGBA> colors) const
{
  return applyMask(frames[0], colors);
}

PixMask *TarFileMaskedImage::applyMask (Shieldset *s, guint32 shield) const
{
  return applyMask (s->getColors (shield));
}

PixMask *TarFileMaskedImage::applyMask (guint32 shield) const
{
  return applyMask (GameMap::getShieldset ()->getColors (shield));
}

PixMask *TarFileMaskedImage::applyMask (guint32 i, std::vector<Gdk::RGBA> colors) const
{
  return applyMask(frames[i], colors);
}

PixMask *TarFileMaskedImage::applyMask (guint32 i, guint32 shield) const
{
  return applyMask (i, GameMap::getShieldset ()->getColors (shield));
}

PixMask* TarFileMaskedImage::applyMask(std::vector<PixMask*> frame, std::vector<Gdk::RGBA> colors) const
{
  if (frame.size () <= 1)
    {
      printf ("masked image '%s' reports only 1 frame when we need at least 2\n", name.c_str ());
      exit (0);
    }
  PixMask *im = frame[0];
  PixMask *ma = frame[1];
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
  auto it = frame.begin ();
  it++;
  //we have our colors coming in, and the number of masks in this image could
  //be more than than or less.
  for (auto color : colors)
    {
      ma = *it;

      Glib::RefPtr<Gdk::Pixbuf> maskbuf = ma->to_pixbuf();

      guint8 *data = maskbuf->get_pixels();
      guint8 *copy = (guint8*)  malloc (height * width * 4 * sizeof(guint8));
      for (int i = 0; i < height * width * 4; i++)
        copy[i] = data[i];
      for (int i = 0; i < width; i++)
        for (int j = 0; j < height; j++)
          {
            const int base = (j * 4) + (i * height * 4);

            if (copy[base+3] != 0)
              {
                copy[base+0] = color.get_red() *copy[base+0];
                copy[base+1] = color.get_green() * copy[base+1];
                copy[base+2] = color.get_blue() * copy[base+2];
              }
          }

      Glib::RefPtr<Gdk::Pixbuf> coloredmask =
        Gdk::Pixbuf::create_from_data(copy, Gdk::Colorspace::RGB, true, 8,
                                      width, height, width * 4);
      result->draw_pixbuf(coloredmask, 0, 0, 0, 0, width, height);
      free(copy);
      it++;
      if (it == frame.end ()) //no more masks, we're done
        break;
    }

  return result;
}

bool TarFileMaskedImage::copy (TarFile *t, TarFileMaskedImage *dest, Glib::ustring &err)
{
  bool success = false;
  std::string newname;
  if (getName ().empty () == true)
    return false;
  std::string filename = t->getFileFromConfigurationFile (getName ());
  /*
   * we have to copy the file out of the way because there are
   * intermediate Close operations on the tarfile which deletes it.
   */
  std::string tmp_dir = File::get_tmp_file ();
  File::create_dir (tmp_dir);
  std::string bname = getName ();
  std::string destfile = String::ucompose ("%1/%2", tmp_dir, bname);
  bool copied = File::copy (filename, destfile);
  if (!copied)
    return false;

  if (dest->getName ().empty () == true)
    success = t->addFileInCfgFile (destfile, newname, err);
  else
    success = t->replaceFileInCfgFile (dest->getName (), destfile, newname, err);

  if (success)
    {
      dest->setNumMasks (getNumMasks ());
      dest->load (t, newname);
      dest->instantiateImages ();
    }
  File::erase (destfile);
  File::erase_dir (tmp_dir);
  return success;
}

void TarFileMaskedImage::uninstantiate (std::string name, std::vector<TarFileMaskedImage*> images)
{
  for (auto i : images)
    if (i->getName () == name)
      i->clear ();
}

void TarFileMaskedImage::copyFrames (TarFileMaskedImage *dst)
{
  dst->clear (true);
  dst->orientation = orientation;
  dst->name = name;
  dst->file_on_disk = file_on_disk;
  dst->dimension = dimension;
  if (image)
    dst->image = image->copy ();
  else
    dst->image = NULL;

  dst->calculated_number_of_frames = calculated_number_of_frames;

  for (guint32 i = 0; i < frames.size (); i++)
    {
      std::vector<PixMask*> frame;
      for (guint32 j = 0; j < frames[i].size (); j++)
        frame.push_back (frames[i][j]->copy ());
      dst->frames[i] = frame;
    }
}

bool TarFileMaskedImage::checkDimension (std::string f)
{
  return PixMask::checkDimension (f, dimension_type, getNumMasks ());
}

bool TarFileMaskedImage::calculateNumberOfMasks (std::string f, bool &bad_dimension)
{
  if (dimension_type == PixMask::DIMENSION_WIDTH_IS_MULTIPLE_OF_ROW_HEIGHT &&
      orientation == VERTICAL_MASK)
    {
      if (checkDimension (f))
        {
          bool broken;
          PixMask *p = PixMask::create (f, broken);
          for (int i = 2; i < 5; i++)
            {
              int h = p->get_height () / i;
              if (p->get_width () % h == 0)
                {
                  maskcount = (p->get_height () / h) - 1;
                  break;
                }
            }
          delete p;
        }
      else
        bad_dimension = true;
    }
  else if (dimension_type == PixMask::DIMENSION_HEIGHT_IS_MULTIPLE_OF_WIDTH &&
      orientation == VERTICAL_MASK)
    {
      if (checkDimension (f))
        {
          bool broken = false;
          PixMask *p = PixMask::create (f, broken);
          maskcount =
            (p->get_height () / p->get_width ()) - 1;
          delete p;
          return true;
        }
      else
        bad_dimension = true;
    }
  else if (dimension_type == PixMask::DIMENSION_WIDTH_IS_FIXED_MAX_PLAYERS &&
           orientation == VERTICAL_MASK)
    {
      if (checkDimension (f))
        {
          bool broken = false;
          PixMask *p = PixMask::create (f, broken);
          guint32 ts = p->get_width () / MAX_PLAYERS;
          maskcount = (p->get_height () / ts) - 1;
          delete p;
          return true;
        }
      else
        bad_dimension = true;
    }
  else if (dimension_type == PixMask::DIMENSION_HEIGHT_IS_MARKED &&
           orientation == VERTICAL_MASK)
    {
      auto pixbuf = Gdk::Pixbuf::create_from_file (f);
      if (!pixbuf)
        return false;

      int height = pixbuf->get_height ();
      int stride = pixbuf->get_rowstride ();

      const guint8* pixels = pixbuf->get_pixels ();

      bool found = false;
      int row = -1;
      for (int y = 0; y < height; y++)
        {
          const guint8* p = pixels + y * stride;

          guint8 r = p[0];
          guint8 g = p[1];
          guint8 b = p[2];

          if (r == 255 && g == 87 && b == 204)
            {
              found = true;
              row = y;
              break;
            }
        }

      if (!found)
        bad_dimension = true;
      else
        maskcount = (pixbuf->get_height () / row) - 1;
      return found;
    }

  return false;
}
