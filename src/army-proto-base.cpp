// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2007, 2008, 2014, 2021 Ben Asselstine
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
#include "armyprotobase.h"
#include "xmlhelper.h"
#include "defs.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

ArmyProtoBase::ArmyProtoBase(const ArmyProtoBase& a)
    :ArmyBase(a), d_name(a.d_name), 
     d_description(a.d_description), d_production_cost(a.d_production_cost),
     d_new_production_cost(a.d_new_production_cost),
     d_production(a.d_production), d_armyset(a.d_armyset)
{
}

ArmyProtoBase::ArmyProtoBase()
  :ArmyBase(), d_name(_("Untitled")),
    d_description(""), d_production_cost(0), d_new_production_cost(0),
    d_production(0), d_armyset(0)
{
}

ArmyProtoBase::ArmyProtoBase(XML_Helper* helper)
  :ArmyBase(helper), d_name(""), 
    d_description(""), d_production_cost(0), d_new_production_cost(0),
    d_production(0), d_armyset(0)
{

  helper->getData(d_name, "name");
  helper->getData(d_production_cost, "production_cost");
  helper->getData(d_new_production_cost, "new_production_cost");
  helper->getData(d_production, "production");
  helper->getData(d_description, "description");
}

bool ArmyProtoBase::saveData(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("name", d_name);
  retval &= helper->saveData("description", d_description);
  retval &= helper->saveData("production_cost", d_production_cost);
  retval &= helper->saveData("new_production_cost", d_new_production_cost);
  retval &= helper->saveData("production", d_production);
  retval &= ArmyBase::saveData(helper);
  return retval;
}

void ArmyProtoBase::setArmyBonus (guint32 b, bool flag)
{
  if ((d_army_bonus & b) != 0)
    {
      if (!flag)
        d_army_bonus ^= b;
    }
  else
    {
      if (flag)
        d_army_bonus |= b;
    }
}
