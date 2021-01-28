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
                            PixMask::DIMENSION_WIDTH_IS_TWO_HEIGHT);
  d_center_mimage =
    new TarFileMaskedImage (TarFileMaskedImage::HORIZONTAL_MASK,
                            PixMask::DIMENSION_WIDTH_IS_TWO_HEIGHT);
  d_right_mimage =
    new TarFileMaskedImage (TarFileMaskedImage::HORIZONTAL_MASK,
                            PixMask::DIMENSION_WIDTH_IS_TWO_HEIGHT);

  d_left_mimage->load_name (helper, "left_image");
  d_center_mimage->load_name (helper, "center_image");
  d_right_mimage->load_name (helper, "right_image");
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
                            PixMask::DIMENSION_WIDTH_IS_TWO_HEIGHT);
  d_center_mimage =
    new TarFileMaskedImage (TarFileMaskedImage::HORIZONTAL_MASK,
                            PixMask::DIMENSION_WIDTH_IS_TWO_HEIGHT);
  d_right_mimage =
    new TarFileMaskedImage (TarFileMaskedImage::HORIZONTAL_MASK,
                            PixMask::DIMENSION_WIDTH_IS_TWO_HEIGHT);
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
  retval &= helper->saveData("left_image", d_left_mimage->getName ());
  retval &= helper->saveData("center_image", d_center_mimage->getName ());
  retval &= helper->saveData("right_image", d_right_mimage->getName ());
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
