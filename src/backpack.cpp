//  Copyright (C) 2008, 2010, 2014, 2015, 2021 Ben Asselstine
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
#include <sigc++/functors/mem_fun.h>

#include "Backpack.h"

#include "xmlhelper.h"

#include "Item.h"

Glib::ustring Backpack::d_tag = "backpack";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<flush;}
#define debug(x)


Backpack::Backpack()
{
    debug("Backpack()");
}

Backpack::Backpack(XML_Helper* helper)
{
  helper->registerTag(Item::d_tag, sigc::mem_fun(this, &Backpack::loadItem));
}

Backpack::Backpack(const Backpack& backpack)
 : std::list<Item*>()
{
  for (const_iterator it = backpack.begin(); it != backpack.end(); ++it)
    push_back(new Item(**it));
}

Backpack::~Backpack()
{
  for (iterator it = begin(); it != end(); ++it)
    delete (*it);
}

bool Backpack::saveData(XML_Helper* helper) const
{
  bool retval = true;
  for (const_iterator it = begin(); it != end(); ++it)
    retval &= (*it)->save(helper);
  return true;
}

bool Backpack::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(Backpack::d_tag);
    retval &= saveData(helper);
    retval &= helper->closeTag();

    return retval;
}

bool Backpack::loadItem(Glib::ustring tag, XML_Helper* helper)
{
  if (tag == Backpack::d_tag)
    return true;

  if (tag == Item::d_tag)
    {
      Item* item = new Item(helper);
      push_back(item);
      return true;
    }

  return false;
}

guint32 Backpack::countStrengthBonuses()
{
  guint32 bonus = 0;
  for (iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getBonus(Item::ADD1STR))
	bonus += 1;
      if ((*it)->getBonus(Item::ADD2STR))
	bonus += 2;
      if ((*it)->getBonus(Item::ADD3STR))
	bonus += 3;
    }
  return bonus;
}

guint32 Backpack::countStackStrengthBonuses()
{
  guint32 bonus = 0;
  for (iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getBonus(Item::ADD1STACK))
	bonus += 1;
      if ((*it)->getBonus(Item::ADD2STACK))
	bonus += 2;
      if ((*it)->getBonus(Item::ADD3STACK))
	bonus += 3;
    }
  return bonus;
}

guint32 Backpack::countGoldBonuses()
{
  guint32 bonus = 0;
  for (iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getBonus(Item::ADD2GOLDPERCITY))
	bonus += 2;
      if ((*it)->getBonus(Item::ADD3GOLDPERCITY))
	bonus += 3;
      if ((*it)->getBonus(Item::ADD4GOLDPERCITY))
	bonus += 4;
      if ((*it)->getBonus(Item::ADD5GOLDPERCITY))
	bonus += 5;
    }
  return bonus;
}

guint32 Backpack::countMovementDoublers()
{
  guint32 bonus = 0;
  for (iterator it = begin(); it != end(); ++it)
    if ((*it)->getBonus(Item::DOUBLEMOVESTACK))
      bonus++;
  return bonus;
}

guint32 Backpack::countStackFlightGivers()
{
  guint32 bonus = 0;
  for (iterator it = begin(); it != end(); ++it)
    if ((*it)->getBonus(Item::FLYSTACK))
      bonus++;
  return bonus;
}

guint32 Backpack::countPlantableItems()
{
  guint32 count = 0;
  for (iterator it = begin(); it != end(); ++it)
    if ((*it)->isPlantable())
      count++;
  return count;
}

Item *Backpack::getPlantableItem(Player *player)
{
  for (iterator it = begin(); it != end(); ++it)
    if ((*it)->isPlantable() && (*it)->getPlantableOwner() == player)
      return *it;
  return NULL;
}
	
Item *Backpack::getItemById(guint32 id)
{
  for (iterator it = begin(); it != end(); ++it)
    if ((*it)->getId() == id)
      return *it;
  return NULL;
}

bool Backpack::addToBackpack(Item* item, int position)
{
  iterator it = begin();
  for (; position > 0; position--, ++it);
  insert(it, item);
  return true;
}

bool Backpack::addToBackpack(Item* item)
{
  iterator it = end();
  insert(it, item);
  return true;
}

bool Backpack::removeFromBackpack(Item* item)
{
  for (iterator it = begin(); it != end(); ++it)
    if ((*it) == item)
      {
	//FIXME: delete the item?
	erase(it);
	return true;
      }

  return false;
}
	
void Backpack::removeAllFromBackpack()
{
  while (!empty())
    removeFromBackpack(front());
}
	
void Backpack::add(Backpack *backpack)
{
  for (Backpack::iterator it = backpack->begin(); it != backpack->end(); ++it)
    addToBackpack(new Item(**it));
}

bool Backpack::hasUsableItem() const
{
  for (Backpack::const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->isUsable())
        return true;
    }
  return false;
}

void Backpack::getUsableItems(std::list<Item*> &items) const
{
  for (Backpack::const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->isUsable())
        items.push_back(*it);
    }
  return;
}

bool Backpack::useItem(Item *item)
{
  if (item->isUsable() && item->use())
    return removeFromBackpack(item);
  return false;
}

// End of file
