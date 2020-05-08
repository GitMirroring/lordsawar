// Copyright (C) 2007, 2008, 2009, 2010, 2014, 2015, 2020 Ben Asselstine
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
#include <iomanip>


#include "tilestyle.h"
#include "ucompose.hpp"
#include "defs.h"
#include "xmlhelper.h"

Glib::ustring TileStyle::d_tag = "tilestyle";

TileStyle::TileStyle()
  : d_image(0), d_type (LONE), d_id (0)
{
}
        
TileStyle::TileStyle(const TileStyle& t)
{
  if (t.d_image != NULL)
    d_image = t.d_image->copy();
  else
    d_image = NULL;

  d_type = t.d_type;

  d_id = t.d_id;
}

TileStyle::TileStyle(guint32 id, TileStyle::Type type)
 : d_image(0), d_type(type), d_id(id)
{
}

TileStyle::~TileStyle()
{
  uninstantiateImage();
}

TileStyle::TileStyle(XML_Helper* helper)
  : d_image(0)
{
  int i;
  char *end = NULL;
  Glib::ustring idstr;

  helper->getData(idstr, "id");
  unsigned long int val = 0;
  val = strtoul (idstr.c_str(), &end, 0);
  d_id = (guint32) val;
  helper->getData(i, "type");
  d_type = static_cast<TileStyle::Type>(i);

}
    
bool TileStyle::save(XML_Helper *helper)
{
  bool retval = true;

  retval &= helper->openTag(d_tag);
  Glib::ustring idstr;
  
  idstr = String::ucompose ("0x%1", idToString(d_id));
  retval &= helper->saveData("id", idstr);
  retval &= helper->saveData("type", d_type);
  retval &= helper->closeTag();

  return retval;
}

Glib::ustring TileStyle::getTypeName() const
{
  return getTypeName(d_type);
}

void TileStyle::uninstantiateImage()
{
  if (d_image != NULL)
    delete d_image;
  d_image = NULL;
}

Glib::ustring TileStyle::getTypeName(Type type)
{
  switch (type)
    {
    case LONE: return "Lone";
    case OUTERTOPLEFT: return "Outer Top-Left";
    case OUTERTOPCENTER: return "Outer Top-Centre";
    case OUTERTOPRIGHT: return "Outer Top-Right";
    case OUTERBOTTOMLEFT: return "Outer Bottom-Left";
    case OUTERBOTTOMCENTER: return "Outer Bottom-Centre";
    case OUTERBOTTOMRIGHT: return "Outer Bottom-Right";
    case OUTERMIDDLELEFT: return "Outer Middle-Left";
    case INNERMIDDLECENTER: return "Outer Middle-Centre";
    case OUTERMIDDLERIGHT: return "Outer Middle-Right";
    case INNERTOPLEFT: return "Inner Top-Left";
    case INNERTOPRIGHT: return "Inner Top-Right";
    case INNERBOTTOMLEFT: return "Inner Bottom-Left";
    case INNERBOTTOMRIGHT: return "Inner Bottom-Right";
    case TOPLEFTTOBOTTOMRIGHTDIAGONAL: return "Top-Left To Bottom-Right Diagonal";
    case BOTTOMLEFTTOTOPRIGHTDIAGONAL: return "Bottom-Left to Top-Right Diagonal";
    case OTHER: return "Other";
    case UNKNOWN: return "Unknown";
    default:
      return "Unknown";
    }
}

guint32 TileStyle::calculateHexDigits(guint32 id)
{
  if (id < 256)
    return 2;
  else if (id < 4096)
    return 3;
  else if (id < 65536)
    return 4;
  else
    return 5;
}

Glib::ustring TileStyle::idToString(guint32 id, guint32 digits)
{
  guint32 num_digits;
  if (digits != 0)
    num_digits = digits;
  else
    num_digits = calculateHexDigits(id);

  return String::ucompose ("%1", Glib::ustring::format(std::hex, std::setfill(L'0'), std::setw(num_digits), id));
}
// End of file
