//  Copyright (C) 2017, 2020, 2021 Ben Asselstine
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

//#include <iostream>
#include <fstream>
#include <sstream>
#include "tartan.h"
#include "xmlhelper.h"
#include "ucompose.hpp"
#include "shieldset.h"
#include "tarhelper.h"
#include "gui/image-helpers.h"
#include "TarFileMaskedImage.h"

Glib::ustring Tartan::d_tartan_tag = "tartan";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Tartan::Tartan(XML_Helper* helper)
{
  d_left_mimage =
    new TarFileMaskedImage (TarFileMaskedImage::HORIZONTAL_MASK,
                            PixMask::DIMENSION_ANY);
  d_center_mimage =
    new TarFileMaskedImage (TarFileMaskedImage::HORIZONTAL_MASK,
                            PixMask::DIMENSION_ANY);
  d_right_mimage =
    new TarFileMaskedImage (TarFileMaskedImage::HORIZONTAL_MASK,
                            PixMask::DIMENSION_ANY);

  d_left_mimage->load (helper, "left_image", "left_image_num_masks");
  d_center_mimage->load (helper, "center_image", "center_image_num_masks");
  d_right_mimage->load (helper, "right_image", "right_image_num_masks");
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
    new TarFileMaskedImage (TarFileMaskedImage::HORIZONTAL_MASK,
                            PixMask::DIMENSION_ANY);
  d_center_mimage =
    new TarFileMaskedImage (TarFileMaskedImage::HORIZONTAL_MASK,
                            PixMask::DIMENSION_ANY);
  d_right_mimage =
    new TarFileMaskedImage (TarFileMaskedImage::HORIZONTAL_MASK,
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

  retval &= helper->openTag(d_tartan_tag);
  retval &= d_left_mimage->save (helper, "left_image", "left_image_num_masks");
  retval &= d_center_mimage->save (helper, "center_image", "center_image_num_masks");
  retval &= d_right_mimage->save (helper, "right_image", "right_image_num_masks");
  retval &= helper->closeTag();
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
