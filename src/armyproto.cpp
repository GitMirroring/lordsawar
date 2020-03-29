// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2014, 2015, 2020 Ben Asselstine
// Copyright (C) 2007, 2008 Ole Laursen
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
#include "armyproto.h"
#include "xmlhelper.h"
#include "armyset.h"
#include "gui/image-helpers.h"
#include "Tile.h"
#include "tarhelper.h"
#include "File.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)
Glib::ustring ArmyProto::d_tag = "armyproto";

ArmyProto::ArmyProto(const ArmyProto& a)
    :ArmyProtoBase(a), d_id(a.d_id), d_defends_ruins(a.d_defends_ruins), 
     d_awardable(a.d_awardable), d_image_name(a.d_image_name),
     d_gender(a.d_gender)
{
  for (unsigned int c = Shield::WHITE; c <= Shield::NEUTRAL; c++)
    {
      d_image_name[c] = a.d_image_name[c];
      if (a.d_image[c])
        d_image[c] = a.d_image[c]->copy();
      else
        d_image[c] = NULL;
      if (a.d_mask[c])
        d_mask[c] = a.d_mask[c]->copy();
      else
        d_mask[c] = NULL;
    }
}

ArmyProto::ArmyProto()
  :ArmyProtoBase(), d_id(0), d_defends_ruins(false), d_awardable(false), 
    d_gender(Hero::NONE)
{
  for (unsigned int c = Shield::WHITE; c <= Shield::NEUTRAL; c++)
    {
      d_image_name[c] = "";
      d_image[c] = NULL;
      d_mask[c] = NULL;
    }
}

ArmyProto::ArmyProto(XML_Helper* helper)
  :ArmyProtoBase(helper), d_defends_ruins(false), d_awardable(false)
{
  helper->getData(d_id, "id");
  helper->getData(d_image_name[Shield::WHITE], "image_white");
  File::add_png_if_no_ext (d_image_name[Shield::WHITE]);
  helper->getData(d_image_name[Shield::GREEN], "image_green");
  File::add_png_if_no_ext (d_image_name[Shield::GREEN]);
  helper->getData(d_image_name[Shield::YELLOW], "image_yellow");
  File::add_png_if_no_ext (d_image_name[Shield::YELLOW]);
  helper->getData(d_image_name[Shield::LIGHT_BLUE], "image_light_blue");
  File::add_png_if_no_ext (d_image_name[Shield::LIGHT_BLUE]);
  helper->getData(d_image_name[Shield::RED], "image_red");
  File::add_png_if_no_ext (d_image_name[Shield::RED]);
  helper->getData(d_image_name[Shield::DARK_BLUE], "image_dark_blue");
  File::add_png_if_no_ext (d_image_name[Shield::DARK_BLUE]);
  helper->getData(d_image_name[Shield::ORANGE], "image_orange");
  File::add_png_if_no_ext (d_image_name[Shield::ORANGE]);
  helper->getData(d_image_name[Shield::BLACK], "image_black");
  File::add_png_if_no_ext (d_image_name[Shield::BLACK]);
  helper->getData(d_image_name[Shield::NEUTRAL], "image_neutral");
  File::add_png_if_no_ext (d_image_name[Shield::NEUTRAL]);
  helper->getData(d_defends_ruins,"defends_ruins");
  helper->getData(d_awardable,"awardable");
  Glib::ustring gender_str;
  if (!helper->getData(gender_str, "gender"))
    d_gender = Hero::NONE;
  else
    d_gender = Hero::genderFromString(gender_str);
  for (unsigned int c = Shield::WHITE; c <= Shield::NEUTRAL; c++)
    {
      d_image[c] = NULL;
      d_mask[c] = NULL;
    }
}

ArmyProto::~ArmyProto()
{
}

bool ArmyProto::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag(d_tag);

  retval &= saveData(helper);

  retval &= helper->closeTag();

  return retval;
}

bool ArmyProto::saveData(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("id", d_id);
  retval &= ArmyProtoBase::saveData(helper);
  retval &= helper->saveData("image_white", d_image_name[Shield::WHITE]);
  retval &= helper->saveData("image_green", d_image_name[Shield::GREEN]);
  retval &= helper->saveData("image_yellow", d_image_name[Shield::YELLOW]);
  retval &= helper->saveData("image_light_blue", 
			     d_image_name[Shield::LIGHT_BLUE]);
  retval &= helper->saveData("image_red", d_image_name[Shield::RED]);
  retval &= helper->saveData("image_dark_blue", 
			     d_image_name[Shield::DARK_BLUE]);
  retval &= helper->saveData("image_orange", d_image_name[Shield::ORANGE]);
  retval &= helper->saveData("image_black", d_image_name[Shield::BLACK]);
  retval &= helper->saveData("image_neutral", d_image_name[Shield::NEUTRAL]);
  retval &= helper->saveData("awardable", d_awardable);
  retval &= helper->saveData("defends_ruins", d_defends_ruins);
  Glib::ustring gender_str = Hero::genderToString(Hero::Gender(d_gender));
  retval &= helper->saveData("gender", gender_str);

  return retval;
}

void ArmyProto::loadImage(int tilesize, Shield::Colour c, Glib::ustring image_filename, bool scale, bool &broken)
{
  Glib::ustring s;

  if (image_filename == "")
    {
      broken = true;
      return;
    }
  // load the army picture. This is done here to avoid confusion
  // since the armies are used as prototypes as well as actual units in the
  // game.
  // The army image consists of two halves. On the left is the army image, 
  // on the right the mask.
  std::vector<PixMask*> half;
  half = disassemble_row(image_filename, 2, broken);
  if (!broken)
    {
      if (scale)
        {
          PixMask::scale(half[0], tilesize, tilesize);
          PixMask::scale(half[1], tilesize, tilesize);
        }

      setImage(c, half[0]);
      setMask(c, half[1]);
    }

  return;
}

void ArmyProto::instantiateImages(guint32 tilesize, Tar_Helper *t, bool scale,
                                  bool &broken)
{
  broken = false;
  for (unsigned int c = Shield::WHITE; c <= Shield::NEUTRAL; c++)
    {
      Glib::ustring file = "";
      if (getImageName(Shield::Colour(c)).empty() == false)
	file = t->getFile(getImageName(Shield::Colour(c)), broken);
      if (!broken && file.empty() == false)
        loadImage(tilesize, Shield::Colour(c), file, scale, broken);
      if (file.empty() == false)
        File::erase(file);
    }
}

void ArmyProto::uninstantiateImages()
{
  for (unsigned int c = Shield::WHITE; c <= Shield::NEUTRAL; c++)
    {
      if (d_image[c] != NULL)
        delete d_image[c];
      if (d_mask[c] != NULL)
        delete d_mask[c];
      d_image[c] = NULL;
      d_mask[c] = NULL;
    }
}

ArmyProto * ArmyProto::createScout()
{
  ArmyProto *basearmy = new ArmyProto(); 
  basearmy->setMoveBonus(Tile::FOREST | Tile::HILLS);
  basearmy->setMaxMoves(50);
  return basearmy;
}

ArmyProto * ArmyProto::createBat()
{
  //oh no, it's the bat!
  ArmyProto *basearmy = new ArmyProto(); 
  basearmy->setMoveBonus(Tile::FOREST | Tile::HILLS | Tile::SWAMP | 
			 Tile::WATER | Tile::MOUNTAIN);
  basearmy->setMaxMoves(50);
  return basearmy;
}

void ArmyProto::clearImage (Shield::Colour col, bool clear_name)
{
  if (clear_name)
    setImageName (col, "");

  PixMask *p = getImage (col);
  delete p;
  setImage (col, NULL);

  p = getMask (col);
  delete p;
  setMask (col, NULL);
}

bool ArmyProto::instantiateImage (Glib::ustring cfgfile, guint32 ts, 
                                  Shield::Colour col)
{
  bool broken = false;
  Tar_Helper t(cfgfile, std::ios::in, broken);
  if (broken)
    return broken;
  Glib::ustring imgname = getImageName(col);
  if (imgname.empty() == false)
    {
      Glib::ustring filename = t.getFile(imgname, broken);
      if (!broken)
        {
          clearImage (col, false);
          loadImage (ts, col, filename, false, broken);
        }
    }
  return broken;
}
