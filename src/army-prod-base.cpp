//  Copyright (C) 2000, 2001, 2003 Michael Bartl
//  Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
//  Copyright (C) 2004, 2005 Andrea Paternesi
//  Copyright (C) 2007, 2008, 2014, 2026 Ben Asselstine
//  Copyright (C) 2007, 2008 Ole Laursen
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
#include <algorithm>
#include "army-prod-base.h"
#include "army-proto-base.h"
#include "xml-helper.h"
#include "army-set-list.h"

Glib::ustring ArmyProdBase::d_tag = "armyprodbase";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

ArmyProdBase::ArmyProdBase(const ArmyProdBase& a)
    :ArmyProtoBase(a), d_type_id(a.d_type_id)
{
}

ArmyProdBase::ArmyProdBase(const ArmyProto& a)
    :ArmyProtoBase(a), d_type_id (a.getId())
{
}

ArmyProdBase::ArmyProdBase(XML_Helper* helper)
  :ArmyProtoBase(helper)
{
  helper->get(d_armyset, "armyset");
  helper->get(d_type_id, "type");
}

bool ArmyProdBase::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->open_tag(ArmyProdBase::d_tag);

  ArmyProtoBase::save(helper);

  retval &= helper->save("type", d_type_id);
  retval &= helper->save("armyset", d_armyset);

  retval &= helper->close_tag();

  return retval;
}
	
void ArmyProdBase::morph(const ArmyProto *army)
{
  setStrength(army->getStrength());
  setMaxMoves(army->getMaxMoves());
  setMoveBonus(army->getMoveBonus());
  setArmyBonus(army->getArmyBonus());
  setTypeId(army->getId());
  setArmyset(army->getArmyset());
}
