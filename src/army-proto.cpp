// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2014, 2015, 2020,
// 2021 Ben Asselstine
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
#include "TarFileMaskedImage.h"
#include "File.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)
Glib::ustring ArmyProto::d_tag = "armyproto";

ArmyProto::ArmyProto(const ArmyProto& a)
    :ArmyProtoBase(a), d_id(a.d_id), d_defends_ruins(a.d_defends_ruins), 
     d_awardable(a.d_awardable), d_gender(a.d_gender)
{
  for (unsigned int c = Shield::WHITE; c <= Shield::NEUTRAL; c++)
    d_mimage[c] = new TarFileMaskedImage(*a.d_mimage[c]);
}

ArmyProto::ArmyProto()
  :ArmyProtoBase(), d_id(0), d_defends_ruins(false), d_awardable(false), 
    d_gender(Hero::NONE)
{
  for (unsigned int c = Shield::WHITE; c <= Shield::NEUTRAL; c++)
    d_mimage[c] =
      new TarFileMaskedImage
      (TarFileMaskedImage::HORIZONTAL_MASK,
       PixMask::DIMENSION_WIDTH_IS_MULTIPLE_OF_HEIGHT);
}

ArmyProto::ArmyProto(XML_Helper* helper)
  :ArmyProtoBase(helper), d_defends_ruins(false), d_awardable(false)
{
  for (unsigned int c = Shield::WHITE; c <= Shield::NEUTRAL; c++)
    d_mimage[c] =
      new TarFileMaskedImage
      (TarFileMaskedImage::HORIZONTAL_MASK,
       PixMask::DIMENSION_WIDTH_IS_MULTIPLE_OF_HEIGHT);
  helper->getData(d_id, "id");
  d_mimage[Shield::WHITE]->load (helper, "image_white", "image_white_num_masks");
  d_mimage[Shield::GREEN]->load (helper, "image_green", "image_green_num_masks");
  d_mimage[Shield::YELLOW]->load (helper, "image_yellow", "image_yellow_num_masks");
  d_mimage[Shield::LIGHT_BLUE]->load (helper, "image_light_blue", "image_red_num_masks");
  d_mimage[Shield::RED]->load (helper, "image_red", "image_red_num_masks");
  d_mimage[Shield::DARK_BLUE]->load (helper, "image_dark_blue", "image_dark_blue_num_masks");
  d_mimage[Shield::ORANGE]->load (helper, "image_orange", "image_orange_num_masks");
  d_mimage[Shield::BLACK]->load (helper, "image_black", "image_black_num_masks");
  d_mimage[Shield::NEUTRAL]->load (helper, "image_neutral", "image_neutral_num_masks");
  helper->getData(d_defends_ruins,"defends_ruins");
  helper->getData(d_awardable,"awardable");
  Glib::ustring gender_str;
  if (!helper->getData(gender_str, "gender"))
    d_gender = Hero::NONE;
  else
    d_gender = Hero::genderFromString(gender_str);
}

ArmyProto::~ArmyProto()
{
  for (unsigned int c = Shield::WHITE; c <= Shield::NEUTRAL; c++)
    delete d_mimage[c];
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
  retval &= d_mimage[Shield::WHITE]->save (helper, "image_white", "image_white_num_masks");
  retval &= d_mimage[Shield::GREEN]->save (helper, "image_green", "image_green_num_masks");
  retval &= d_mimage[Shield::GREEN]->save (helper, "image_yellow", "image_yellow_num_masks");
  retval &= d_mimage[Shield::GREEN]->save (helper, "image_light_blue", "image_light_blue_num_masks");
  retval &= d_mimage[Shield::GREEN]->save (helper, "image_red", "image_red_num_masks");
  retval &= d_mimage[Shield::GREEN]->save (helper, "image_dark_blue", "image_dark_blue_num_masks");
  retval &= d_mimage[Shield::GREEN]->save (helper, "image_orange", "image_orange_num_masks");
  retval &= d_mimage[Shield::GREEN]->save (helper, "image_black", "image_black_num_masks");
  retval &= d_mimage[Shield::GREEN]->save (helper, "image_neutral", "image_neutral_num_masks");
  retval &= helper->saveData("awardable", d_awardable);
  retval &= helper->saveData("defends_ruins", d_defends_ruins);
  Glib::ustring gender_str = Hero::genderToString(Hero::Gender(d_gender));
  retval &= helper->saveData("gender", gender_str);

  return retval;
}

void ArmyProto::instantiateImages(guint32 tilesize, Tar_Helper *t, bool scale,
                                  bool &broken)
{
  broken = false;
  Vector<int> dim = Vector<int>(tilesize,tilesize);

  for (unsigned int c = Shield::WHITE; c <= Shield::NEUTRAL; c++)
    {
      broken = d_mimage[c]->load (t);
      if (broken)
        break;
      d_mimage[c]->instantiateImages (scale ? dim : Vector<int>(-1,-1));
    }
}

void ArmyProto::uninstantiateImages()
{
  for (unsigned int c = Shield::WHITE; c <= Shield::NEUTRAL; c++)
    d_mimage[c]->uninstantiateImages ();
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

bool ArmyProto::instantiateImage (Glib::ustring cfgfile, Shield::Color col)
{
  bool broken = false;
  Tar_Helper t(cfgfile, std::ios::in, broken);
  if (broken)
    return broken;
  Glib::ustring imgname = getMaskedImage(col)->getName ();
  if (imgname.empty() == false)
    {
      getMaskedImage (col)->clear (false);
      broken = getMaskedImage (col)->load (&t);
      getMaskedImage (col)->instantiateImages();
    }
  return broken;
}
