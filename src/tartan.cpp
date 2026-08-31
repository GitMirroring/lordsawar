//  Copyright (C) 2017, 2020, 2021, 2026 Ben Asselstine
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

#include <fstream>
#include <sstream>
#include "tartan.h"
#include "xml-helper.h"
#include "ucompose.hpp"
#include "shield-set.h"
#include "tar-helper.h"
#include "image-helpers.h"
#include "tar-file-masked-image.h"

Glib::ustring Tartan::d_tartan_tag = "tartan";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Tartan::Tartan(XML_Helper* helper)
{
  d_left_mimage =
    new TarFileMaskedImage (TarFileMaskedImage::VERTICAL_MASK,
                            PixMask::DIMENSION_ANY);
  d_center_mimage =
    new TarFileMaskedImage (TarFileMaskedImage::VERTICAL_MASK,
                            PixMask::DIMENSION_ANY);
  d_right_mimage =
    new TarFileMaskedImage (TarFileMaskedImage::VERTICAL_MASK,
                            PixMask::DIMENSION_ANY);

  d_left_mimage->load (helper, "left_image");
  d_center_mimage->load (helper, "center_image");
  d_right_mimage->load (helper, "right_image");
}

Tartan::Tartan(const Tartan& t)
{
  d_left_mimage = new TarFileMaskedImage (*t.d_left_mimage);
  d_center_mimage = new TarFileMaskedImage (*t.d_center_mimage);
  d_right_mimage = new TarFileMaskedImage (*t.d_right_mimage);
}

Tartan::Tartan()
{
  d_left_mimage =
    new TarFileMaskedImage (TarFileMaskedImage::VERTICAL_MASK,
                            PixMask::DIMENSION_ANY);
  d_center_mimage =
    new TarFileMaskedImage (TarFileMaskedImage::VERTICAL_MASK,
                            PixMask::DIMENSION_ANY);
  d_right_mimage =
    new TarFileMaskedImage (TarFileMaskedImage::VERTICAL_MASK,
                            PixMask::DIMENSION_ANY);
}

Tartan::~Tartan()
{
  delete d_left_mimage;
  delete d_center_mimage;
  delete d_right_mimage;
}

bool Tartan::saveTartan(XML_Helper *helper) const
{
  bool retval = true;

  retval &= helper->open_tag(d_tartan_tag);
  retval &= d_left_mimage->save (helper, "left_image");
  retval &= d_center_mimage->save (helper, "center_image");
  retval &= d_right_mimage->save (helper, "right_image");
  retval &= helper->close_tag();
  return retval;
}

TarFileMaskedImage * Tartan::getTartanMaskedImage (Type t) const
{
  switch (t)
    {
    case Tartan::LEFT: return d_left_mimage;
    case Tartan::CENTER: return d_center_mimage;
    case Tartan::RIGHT: return d_right_mimage;
    }
  return d_left_mimage;
}

Glib::ustring Tartan::tartanTypeToFriendlyName(const Tartan::Type type)
{
  switch (type)
    {
      case Tartan::LEFT: return _("Left");
      case Tartan::CENTER: return _("Center");
      case Tartan::RIGHT: return _("Right");
    }
  return _("Left");
}

int Tartan::get_tallest_tartan_component () const
{
  int height = 0;
  if (d_left_mimage)
    {
      if (d_left_mimage->getImage ())
        {
          if (d_left_mimage->getImage ()->get_height () > height)
            height = d_left_mimage->getImage ()->get_height ();
        }
    }
  if (d_center_mimage)
    {
      if (d_center_mimage->getImage ())
        {
          if (d_center_mimage->getImage ()->get_height () > height)
            height = d_center_mimage->getImage ()->get_height ();
        }
    }
  if (d_right_mimage)
    {
      if (d_right_mimage->getImage ())
        {
          if (d_right_mimage->getImage ()->get_height () > height)
            height = d_right_mimage->getImage ()->get_height ();
        }
    }
  return height;
}
