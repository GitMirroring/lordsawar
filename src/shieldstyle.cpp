//  Copyright (C) 2008, 2009, 2011, 2014, 2015 Ben Asselstine
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
#include <sstream>
#include "shieldstyle.h"
#include "xmlhelper.h"
#include "File.h"
#include "shieldset.h"
#include "gui/image-helpers.h"

Glib::ustring ShieldStyle::d_tag = "shieldstyle";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

ShieldStyle::ShieldStyle(ShieldStyle::Type type)
{
  d_type = type;
  d_image_name = "";
  d_image = NULL;
  d_mask = NULL;
}
        
ShieldStyle::~ShieldStyle()
{
  uninstantiateImages();
}

ShieldStyle::ShieldStyle(const ShieldStyle &s)
 : sigc::trackable(s)
{
  d_type = s.d_type;
  d_image_name = s.d_image_name;
  if (s.d_image != NULL)
    d_image = s.d_image->copy();
  else
    d_image = NULL;
  if (s.d_mask != NULL)
    d_mask = s.d_mask->copy();
  else
    d_mask = NULL;
}

ShieldStyle::ShieldStyle(XML_Helper* helper)
  :d_image(0), d_mask(0)
{
  Glib::ustring type_str;
  helper->getData(type_str, "type");
  d_type = shieldStyleTypeFromString(type_str);
  helper->getData(d_image_name, "image");
  File::add_png_if_no_ext (d_image_name);
}

Glib::ustring ShieldStyle::shieldStyleTypeToString(const ShieldStyle::Type type)
{
  switch (type)
    {
      case ShieldStyle::SMALL: return "ShieldStyle::SMALL";
      case ShieldStyle::MEDIUM: return "ShieldStyle::MEDIUM";
      case ShieldStyle::LARGE: return "ShieldStyle::LARGE";
    }
  return "ShieldStyle::SMALL";
}

Glib::ustring ShieldStyle::shieldStyleTypeToFriendlyName(const ShieldStyle::Type type)
{
  switch (type)
    {
      case ShieldStyle::SMALL: return _("Small");
      case ShieldStyle::MEDIUM: return _("Medium");
      case ShieldStyle::LARGE: return _("Large");
    }
  return _("Small");
}

ShieldStyle::Type ShieldStyle::shieldStyleTypeFromString(const Glib::ustring str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return ShieldStyle::Type(atoi(str.c_str()));
  if (str == "ShieldStyle::SMALL") return ShieldStyle::SMALL;
  else if (str == "ShieldStyle::MEDIUM") return ShieldStyle::MEDIUM;
  else if (str == "ShieldStyle::LARGE") return ShieldStyle::LARGE;
  return ShieldStyle::SMALL;
}

bool ShieldStyle::save(XML_Helper *helper) const
{
  bool retval = true;

  retval &= helper->openTag(d_tag);
  Glib::ustring s = shieldStyleTypeToString(ShieldStyle::Type(d_type));
  retval &= helper->saveData("type", s);
  retval &= helper->saveData("image", d_image_name);
  retval &= helper->closeTag();
  return retval;
}

void ShieldStyle::instantiateImages(Glib::ustring filename, Shieldset *s, bool scale, bool &broken)
{
  if (filename.empty() == true)
    return;
  // The shield image consists of two halves. On the left is the shield 
  // image, on the right the mask.
  debug("loading shield file: " << filename);
  std::vector<PixMask* > half = disassemble_row(filename, 2, broken);
  if (broken)
    return;

  int xsize = 0;
  int ysize = 0;
  switch (getType())
    {
    case ShieldStyle::SMALL:
      xsize = s->getSmallWidth(); ysize = s->getSmallHeight(); break;
    case ShieldStyle::MEDIUM:
      xsize = s->getMediumWidth(); ysize = s->getMediumHeight(); break;
    case ShieldStyle::LARGE:
      xsize = s->getLargeWidth(); ysize = s->getLargeHeight(); break;
    }
  if (xsize == 0 || ysize == 0)
    {
      xsize = half[0]->get_unscaled_width();
      ysize = half[0]->get_unscaled_height();
    }
  if (xsize > 0 && ysize > 0)
    {
      if (scale)
        {
          PixMask::scale(half[0], xsize, ysize);
          PixMask::scale(half[1], xsize, ysize);
        }
      setImage(half[0]);
      setMask(half[1]);
    }
}

void ShieldStyle::uninstantiateImages()
{
  if (d_image != NULL)
    delete d_image;
  if (d_mask != NULL)
    delete d_mask;
  d_mask = NULL;
  d_image = NULL;
}

