//  Copyright (C) 2008, 2014, 2015, 2021, 2026 Ben Asselstine
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
#include "army-proto.h"
#include "hero-proto.h"
#include "xml-helper.h"

Glib::ustring HeroProto::d_heroproto_tag = "heroproto";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

HeroProto::HeroProto(const HeroProto& a)
    :ArmyProto(a), OwnerId(a), d_gender(a.d_gender),
    d_character_id (a.d_character_id)
{
}

HeroProto::HeroProto(const ArmyProto& a)
    :ArmyProto(a), OwnerId()
{
  d_gender = a.getGender();
  if (d_gender == Hero::NONE)
    d_gender = Hero::MALE;
  d_character_id = 0;
}

HeroProto::HeroProto()
  :ArmyProto(), OwnerId(), d_gender(Hero::FEMALE), d_character_id (0)
{
}

HeroProto::HeroProto(XML_Helper* helper)
  :OwnerId(helper)
{
  helper->get(d_character_id, "hero_id");
  Glib::ustring gender_str;
  helper->get(gender_str, "gender");
  d_gender = Hero::genderFromString(gender_str);
  helper->get(d_armyset, "armyset");

  helper->register_tag
    (ArmyProto::d_tag, 
    ([this] (Glib::ustring tag, XML_Helper *h) -> bool
     {
       if (tag == ArmyProto::d_tag)
         {
           auto armyproto = ArmyProto (h);
           *this = armyproto;
           return true;
         }
       return false;
     }));

}

HeroProto::~HeroProto()
{
  uninstantiateImages();
}

bool HeroProto::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->open_tag(HeroProto::d_heroproto_tag);

  retval &= helper->save("hero_id", d_character_id);
  Glib::ustring gender_str = Hero::genderToString(Hero::Gender(d_gender));
  retval &= helper->save("gender", gender_str);
  retval &= OwnerId::save(helper);
  retval &= helper->save("armyset", d_armyset);
  retval &= ArmyProto::save(helper);
  retval &= helper->close_tag();

  return retval;
}

HeroProto& HeroProto::operator=(const ArmyProto& a)
{
  setUpkeep (a.getUpkeep ());
  setStrength (a.getStrength ());
  setMaxMoves (a.getMaxMoves ());
  setSight (a.getSight ());
  setMoveBonus (a.getMoveBonus ());
  setArmyBonus (a.getArmyBonus ());
  setXpReward (a.getXpReward ());
  setName (a.getName ());
  setProductionCost (a.getProductionCost ());
  setNewProductionCost (a.getNewProductionCost ());
  setProduction (a.getProduction ());
  setArmyset (a.getArmyset ());
  setId (a.getId ());
  setDefendsRuins (a.getDefendsRuins ());
  setAwardable (a.getAwardable ());
  setGender (a.getGender ());
  setMaskedImages (getMaskedImages ());

  return *this;
}
