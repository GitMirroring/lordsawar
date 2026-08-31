//  Copyright (C) 2000, 2001, 2003 Michael Bartl
//  Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
//  Copyright (C) 2002 Mark L. Amidon
//  Copyright (C) 2005 Andrea Paternesi
//  Copyright (C) 2006, 2007, 2008, 2014, 2015 Ben Asselstine
//  Copyright (C) 2008 Ole Laursen
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

#include <sstream>
#include "prodslotlist.h"
#include "path.h"
#include "army.h"
#include "armyprodbase.h"
#include "hero.h"
#include "stacklist.h"
#include "stack.h"
#include "playerlist.h"
#include "armysetlist.h"
#include "citylist.h"
#include "GameMap.h"
#include "vectoredunitlist.h"
#include "vectoredunit.h"
#include "action.h"
#include "xmlhelper.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

ProdSlotlist::ProdSlotlist(guint32 numslots)
     : d_active_production_slot(-1), d_duration(-1)
{
  // Initialise armytypes
  for (unsigned int i = 0; i < numslots; i++)
    push_back(new ProdSlot());

}

ProdSlotlist::ProdSlotlist(XML_Helper* helper)
{
  helper->getData(d_active_production_slot, "active_production_slot");
  helper->getData(d_duration, "duration");
  helper->registerTag(ProdSlot::d_tag, 
		      sigc::mem_fun(this, &ProdSlotlist::load));
}

bool ProdSlotlist::load(Glib::ustring tag, XML_Helper *helper)
{
  if (tag == ProdSlot::d_tag)
    {
      push_back(new ProdSlot(helper));
      return true;
    }
  return false;
}

ProdSlotlist::ProdSlotlist(const ProdSlotlist& c)
    :std::vector<ProdSlot*>(), 
    d_active_production_slot(c.d_active_production_slot), 
    d_duration(c.d_duration)
{
  for (std::vector<ProdSlot*>::const_iterator it = c.begin(); 
       it != c.end(); ++it)
    push_back(new ProdSlot(*(*it)));
}

bool ProdSlotlist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("active_production_slot", 
			       d_active_production_slot);
    retval &= helper->saveData("duration", d_duration);

    for (unsigned int i = 0; i < size(); i++)
      {
	if ((*this)[i])
	  retval &= (*this)[i]->save(helper);
      }
    return retval;
}

ProdSlotlist::~ProdSlotlist()
{
  for (unsigned int i = 0; i < size(); i++)
    delete (*this)[i];
  clear();
}

guint32 ProdSlotlist::getNoOfProductionBases() const
{
  unsigned int max = 0;
  for (unsigned int i = 0; i < getMaxNoOfProductionBases(); i++)
    {
      if (getProductionBase(i))
        max++;
    }
  return max;
}

void ProdSlotlist::setActiveProductionSlot(int index)
{
    if (index == -1)
    {
        d_active_production_slot = index;
        d_duration = -1;
        return;
    }

    // return on wrong data
    if (((index >= (int)size())) || 
	(index >= 0 && getArmytype(index) == -1))
        return;

    d_active_production_slot = index;
    const ArmyProdBase* a = getProductionBase(index);

    // set the duration to produce this armytype
    if (a)
        d_duration = a->getProduction(); 
}

int ProdSlotlist::getFreeSlot()  const
{
     int index=-1;

     debug(getName()<< " BASIC SLOTS=" << size()) 
     for (unsigned int i = 0; i < size(); i++)
     {
         debug(getName()<< " Index Value=" << (*this)[i])
         if ((*this)[i]->getArmyProdBase() == NULL)
         {
             index=i;
             return i;
         }         
     }

     return index;
}

bool ProdSlotlist::hasProductionBase(const ArmyProto * army) const
{
  return hasProductionBase(army->getId());
}

void ProdSlotlist::addProductionBase(int idx, ArmyProdBase *army)
{
    if (idx < 0)
    {
        // try to find an unoccupied production slot. If there is none, pick 
        // the slot with the highest index.
        for (unsigned int i = 0; i < size(); i++)
            if ((*this)[i]->getArmyProdBase() == NULL)
            {
                idx = i;
                break;
            }

        if (idx < 0)
        {
            idx = size() - 1;
        }
    }
    if (idx >= (int)size())
      return;
    
    if ((*this)[idx]->getArmyProdBase())
      {
	bool restore_production = false;
	if (d_active_production_slot == idx)
	  restore_production = true;
	removeProductionBase(idx);
	(*this)[idx]->setArmyProdBase(army);
	if (restore_production)
	  setActiveProductionSlot(idx);
      }
    else
      (*this)[idx]->setArmyProdBase(army);
}

void ProdSlotlist::removeProductionBase(int idx)
{
    if ((idx < 0) || (idx > (int)(getMaxNoOfProductionBases() - 1)))
        return;

    if ((*this)[idx]->getArmyProdBase() != NULL)
      (*this)[idx]->clear();

    if (d_active_production_slot == idx)
        setActiveProductionSlot(-1);
}

bool ProdSlotlist::hasProductionBase(int type) const
{
  if (type < 0)
    return false;
  for (unsigned int i = 0; i < size(); i++)
    {
      if ((*this)[i]->getArmyProdBase() == NULL)
	continue;
      if ((*this)[i]->getArmyProdBase()->getTypeId() == (unsigned int) type)
	return true;
    }

  return false;
}

int ProdSlotlist::getArmytype(int slot) const
{
  if (slot < 0)
    return -1;

  if (slot >= (int)size())
    return -1;
  if ((*this)[slot]->getArmyProdBase() == NULL)
    return -1;
  return (*this)[slot]->getArmyProdBase()->getTypeId();
}

const ArmyProdBase * ProdSlotlist::getProductionBase(int slot) const
{
  if (getArmytype(slot) == -1)
    return 0;
  return (*this)[slot]->getArmyProdBase();
}

const ArmyProdBase *ProdSlotlist::getActiveProductionBase() const
{
  return getProductionBase(d_active_production_slot);
}

const ArmyProdBase *ProdSlotlist::getProductionBaseBelongingTo(const Army *army) const
{
  if (!army)
    return NULL;
  for (unsigned int i = 0; i < this->getMaxNoOfProductionBases(); i++)
    {
      const ArmyProdBase* armyprodbase = this->getProductionBase(i);
      if (armyprodbase == NULL)
	continue;
      if (army->getArmyset() == armyprodbase->getArmyset() &&
	  army->getTypeId() == armyprodbase->getTypeId())
	return armyprodbase;
    }
  return NULL;
}
        
bool ProdSlotlist::removeArmyProdBasesWithoutAType(guint32 armyset)
{
  bool removed = false;
  for (unsigned int i = 0; i < size(); i++)
    {
      const ArmyProdBase* armyprodbase = this->getProductionBase(i);
      if (armyprodbase == NULL)
	continue;
      ArmyProto *a = Armysetlist::getInstance()->getArmy (armyset, armyprodbase->getTypeId());
      if (!a)
        removeProductionBase(i);
      //XXX XXX XXX should we squeeze out the empty spaces?
    }
  return removed;
}
// End of file
