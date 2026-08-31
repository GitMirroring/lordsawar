//  Copyright (C) 2008, 2009, 2011, 2014, 2015, 2020, 2021, 2026 Ben Asselstine
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
#include <sstream>
#include "shield-style.h"
#include "xml-helper.h"
#include "file.h"
#include "shield-set.h"
#include "tar-helper.h"
#include "image-helpers.h"
#include "tar-file-masked-image.h"

Glib::ustring ShieldStyle::d_tag = "shieldstyle";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

ShieldStyle::ShieldStyle(ShieldStyle::Type type)
 : d_type (type)
{
  d_mimage =
    new TarFileMaskedImage (TarFileMaskedImage::VERTICAL_MASK,
                            PixMask::DIMENSION_HEIGHT_IS_MARKED);
}
        
ShieldStyle::~ShieldStyle()
{
  delete d_mimage;
}

ShieldStyle::ShieldStyle(const ShieldStyle &s)
 : sigc::trackable(s), d_type (s.d_type)
{
  d_mimage = new TarFileMaskedImage (*s.d_mimage);
}

ShieldStyle::ShieldStyle(XML_Helper* helper)
{
  d_mimage =
    new TarFileMaskedImage (TarFileMaskedImage::VERTICAL_MASK,
                            PixMask::DIMENSION_HEIGHT_IS_MARKED);
  Glib::ustring type_str;
  helper->get(type_str, "type");
  d_type = shieldStyleTypeFromString(type_str);
  d_mimage->load (helper, "image");
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

  retval &= helper->open_tag(d_tag);
  Glib::ustring s = shieldStyleTypeToString(ShieldStyle::Type(d_type));
  retval &= helper->save("type", s);
  retval &= getMaskedImage ()->save (helper, "image");
  retval &= helper->close_tag();
  return retval;
}
