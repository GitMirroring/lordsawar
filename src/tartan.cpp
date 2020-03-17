//  Copyright (C) 2017, 2020 Ben Asselstine
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

Glib::ustring Tartan::d_tag = "tartan";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Tartan::Tartan(XML_Helper* helper)
 : d_left_tartan_image(0), d_left_tartan_mask(0),
    d_center_tartan_image(0), d_center_tartan_mask(0),
    d_right_tartan_image(0), d_right_tartan_mask(0)
{
  helper->getData(d_left_tartan_name, "left_image");
  helper->getData(d_center_tartan_name, "center_image");
  helper->getData(d_right_tartan_name, "right_image");
}

Tartan::Tartan(const Tartan& t)
  :d_left_tartan_name(t.d_left_tartan_name),
    d_center_tartan_name(t.d_center_tartan_name),
    d_right_tartan_name(t.d_right_tartan_name),
    d_left_tartan_image(0), d_left_tartan_mask(0),
    d_center_tartan_image(0), d_center_tartan_mask(0),
    d_right_tartan_image(0), d_right_tartan_mask(0)
{
  if (t.d_left_tartan_image)
    d_left_tartan_image = t.d_left_tartan_image->copy();
  if (t.d_left_tartan_mask)
    d_left_tartan_mask = t.d_left_tartan_mask->copy();
  if (t.d_center_tartan_image)
    d_center_tartan_image = t.d_center_tartan_image->copy();
  if (t.d_center_tartan_mask)
    d_center_tartan_mask = t.d_center_tartan_mask->copy();
  if (t.d_right_tartan_image)
    d_right_tartan_image = t.d_right_tartan_image->copy();
  if (t.d_right_tartan_mask)
    d_right_tartan_mask = t.d_right_tartan_mask->copy();
}

Tartan::Tartan()
  :d_left_tartan_name(""), d_center_tartan_name(""), d_right_tartan_name(""),
    d_left_tartan_image(0), d_left_tartan_mask(0),
    d_center_tartan_image(0), d_center_tartan_mask(0),
    d_right_tartan_image(0), d_right_tartan_mask(0)
{
}

Tartan::~Tartan()
{
  uninstantiateTartanImages();
}

bool Tartan::saveTartan(XML_Helper *helper) const
{
  bool retval = true;

  retval &= helper->openTag(d_tag);
  retval &= helper->saveData("left_image", d_left_tartan_name);
  retval &= helper->saveData("center_image", d_center_tartan_name);
  retval &= helper->saveData("right_image", d_right_tartan_name);
  retval &= helper->closeTag();
  return retval;
}

void Tartan::instantiateTartanImage(Tartan::Type type, Glib::ustring file, bool &broken)
{
  std::vector<PixMask* > half = disassemble_row(file, 2, broken);
  if (!broken)
    {
      switch (type)
        {
        case Tartan::LEFT:
          d_left_tartan_image = half[0];
          d_left_tartan_mask = half[1];
          break;
        case Tartan::RIGHT:
          d_right_tartan_image = half[0];
          d_right_tartan_mask = half[1];
          break;
        case Tartan::CENTER:
          d_center_tartan_image = half[0];
          d_center_tartan_mask = half[1];
          break;
        }
    }
}

void Tartan::instantiateTartanImages(Glib::ustring l, Glib::ustring c, Glib::ustring r, bool &broken)
{
  broken = false;
  instantiateTartanImage(Tartan::LEFT, l, broken);
  if (!broken)
    instantiateTartanImage(Tartan::CENTER, c, broken);
  if (!broken)
    instantiateTartanImage(Tartan::RIGHT, r, broken);
}

void Tartan::uninstantiateTartanImage(Tartan::Type type)
{
  switch (type)
    {
    case Tartan::LEFT:
      if (d_left_tartan_image)
        delete d_left_tartan_image;
      d_left_tartan_image = NULL;
      if (d_left_tartan_mask)
        delete d_left_tartan_mask;
      d_left_tartan_mask = NULL;
      break;
    case Tartan::RIGHT:
      if (d_right_tartan_image)
        delete d_right_tartan_image;
      d_right_tartan_image = NULL;
      if (d_right_tartan_mask)
        delete d_right_tartan_mask;
      d_right_tartan_mask = NULL;
      break;
    case Tartan::CENTER:
      if (d_center_tartan_image)
        delete d_center_tartan_image;
      d_center_tartan_image = NULL;
      if (d_center_tartan_mask)
        delete d_center_tartan_mask;
      d_center_tartan_mask = NULL;
      break;
    }
}

void Tartan::uninstantiateTartanImages()
{
  uninstantiateTartanImage (Tartan::LEFT);
  uninstantiateTartanImage (Tartan::CENTER);
  uninstantiateTartanImage (Tartan::RIGHT);
}
        
Glib::ustring Tartan::getName(Type t) const
{
  switch (t)
    {
    case Tartan::LEFT:
      return d_left_tartan_name;
    case Tartan::CENTER:
      return d_center_tartan_name;
    case Tartan::RIGHT:
      return d_right_tartan_name;
    }
  return d_left_tartan_name;
}

PixMask *Tartan::getImage(Type t) const
{
  switch (t)
    {
    case Tartan::LEFT:
      return d_left_tartan_image;
    case Tartan::CENTER:
      return d_center_tartan_image;
    case Tartan::RIGHT:
      return d_right_tartan_image;
    }
  return d_left_tartan_image;
}

PixMask *Tartan::getMask(Type t) const
{
  switch (t)
    {
    case Tartan::LEFT:
      return d_left_tartan_mask;
    case Tartan::CENTER:
      return d_center_tartan_mask;
    case Tartan::RIGHT:
      return d_right_tartan_mask;
    }
  return d_left_tartan_mask;
}

void Tartan::setName(Type t, Glib::ustring n)
{
  switch (t)
    {
    case Tartan::LEFT:
      d_left_tartan_name = n;
      break;
    case Tartan::CENTER:
      d_center_tartan_name = n;
      break;
    case Tartan::RIGHT:
      d_right_tartan_name = n;
      break;
    }
}

void Tartan::setImage(Type t, PixMask *i)
{
  switch (t)
    {
    case Tartan::LEFT:
      d_left_tartan_image = i;
      break;
    case Tartan::CENTER:
      d_center_tartan_image = i;
      break;
    case Tartan::RIGHT:
      d_right_tartan_image = i;
      break;
    }
}

void Tartan::setMask(Type t, PixMask *i)
{
  switch (t)
    {
    case Tartan::LEFT:
      d_left_tartan_mask = i;
      break;
    case Tartan::CENTER:
      d_center_tartan_mask = i;
      break;
    case Tartan::RIGHT:
      d_right_tartan_mask = i;
      break;
    }
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
    
Glib::ustring Tartan::getTartanImageName (Tartan::Type type) const
{
  switch (type)
    {
      case Tartan::LEFT: return d_left_tartan_name;
      case Tartan::CENTER: return d_center_tartan_name;
      case Tartan::RIGHT: return d_right_tartan_name;
    }
  return "";
}

void Tartan::setTartanImageName (Tartan::Type type, Glib::ustring name)
{
  switch (type)
    {
      case Tartan::LEFT: d_left_tartan_name = name; break;
      case Tartan::CENTER: d_center_tartan_name = name; break;
      case Tartan::RIGHT: d_right_tartan_name = name; break;
    }
}
