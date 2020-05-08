// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2007, 2008, 2014 Ben Asselstine
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
#include <algorithm>
#include "armyprodbase.h"
#include "armyprotobase.h"
#include "xmlhelper.h"
#include "armysetlist.h"

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
  helper->getData(d_armyset, "armyset");
  helper->getData(d_type_id, "type");
}

bool ArmyProdBase::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag(ArmyProdBase::d_tag);

  ArmyProtoBase::saveData(helper);

  retval &= helper->saveData("type", d_type_id);
  retval &= helper->saveData("armyset", d_armyset);

  retval &= helper->closeTag();

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
