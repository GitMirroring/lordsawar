// Copyright (C) 2008, 2014, 2015, 2021 Ben Asselstine
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
#include "heroproto.h"
#include "xmlhelper.h"

Glib::ustring HeroProto::d_heroproto_tag = "heroproto";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

HeroProto::HeroProto(const HeroProto& a)
    :ArmyProto(a), OwnerId(a), d_gender(a.d_gender), d_hero_id (a.d_hero_id),
     d_strategy (HeroStrategy::copy (a.d_strategy)),
     d_starting_items (a.d_starting_items)
{
}

HeroProto::HeroProto(const ArmyProto& a)
    :ArmyProto(a), OwnerId()
{
  d_gender = a.getGender();
  if (d_gender == Hero::NONE)
    d_gender = Hero::MALE;
  d_hero_id = 0;
  d_strategy = new HeroStrategy_None ();
}

HeroProto::HeroProto()
  :ArmyProto(), OwnerId(), d_gender(Hero::FEMALE), d_hero_id (0),
    d_strategy (new HeroStrategy_None ())
{
}

HeroProto::HeroProto(XML_Helper* helper)
  :ArmyProto(helper), OwnerId(helper)
{
  helper->registerTag (HeroStrategy::d_tag,
                       sigc::mem_fun (this, &HeroProto::load));
  helper->getData(d_hero_id, "hero_id");
  Glib::ustring gender_str;
  if (!helper->getData(gender_str, "gender"))
    d_gender = Hero::NONE;
  else
    d_gender = Hero::genderFromString(gender_str);

  helper->getData(d_armyset, "armyset");

  Glib::ustring items;
  std::stringstream sitems;
  helper->getData(items, "starting_items");
  sitems.str(items);

  while (sitems.eof() == false)
    {
      int ival = -1;
      sitems >> ival;
      if (ival != -1)
	d_starting_items.push_back ((guint32)ival);
    }
}

bool HeroProto::load (Glib::ustring tag, XML_Helper* helper)
{
  if (tag == HeroStrategy::d_tag)
    {
      HeroStrategy *s = HeroStrategy::handle_load (helper);
      d_strategy = s;
      return true;
    }

  return false;
}

HeroProto::~HeroProto()
{
  uninstantiateImages();
  delete d_strategy;
}

bool HeroProto::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag(HeroProto::d_heroproto_tag);

  retval &= helper->saveData("hero_id", d_hero_id);
  retval &= ArmyProto::saveData(helper);
  Glib::ustring gender_str = Hero::genderToString(Hero::Gender(d_gender));
  retval &= helper->saveData("gender", gender_str);
  retval &= OwnerId::save(helper);
  retval &= helper->saveData("armyset", d_armyset);
  std::stringstream items;
  for (auto it: d_starting_items)
    items << it << " ";
  retval &= helper->saveData("starting_items", items.str());
  retval &= d_strategy->save (helper);
  retval &= helper->closeTag();

  return retval;
}
