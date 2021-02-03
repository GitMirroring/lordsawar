//  Copyright (C) 2007-2009, 2011, 2014, 2015, 2017, 2020, 2021 Ben Asselstine
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
#include <vector>
#include <sigc++/functors/mem_fun.h>

#include "SightMap.h"
#include "reward.h"
#include "army.h"
#include "armysetlist.h"
#include "playerlist.h"
#include "ruinlist.h"
#include "ruin.h"
#include "rewardlist.h"
#include "Itemlist.h"
#include "Item.h"
#include "GameMap.h"
#include "ucompose.hpp"
#include "stackreflist.h"
#include "xmlhelper.h"
#include "rnd.h"
#include "GameScenarioOptions.h"

Glib::ustring Reward::d_tag = "reward";

Reward::Reward(Type type, Glib::ustring name)
    :d_type(type), d_name(name)
{
}

Reward::Reward(XML_Helper *helper)
{
  Glib::ustring type_str;
  helper->getData(type_str, "type");
  d_type = rewardTypeFromString(type_str);
  helper->getData(d_name, "name");
}

Reward::Reward (const Reward& orig, bool sync_id)
	:d_type(orig.d_type), d_name(orig.d_name)
{
  (void)sync_id;
}

Reward* Reward::handle_load(XML_Helper* helper)
{
    guint32 t;
    Glib::ustring type_str;
    helper->getData(type_str, "type");
    t = rewardTypeFromString(type_str);
    switch (t)
      {
      case Reward::GOLD:
        return (new Reward_Gold(helper));
      case Reward::ALLIES:
        return (new Reward_Allies(helper));
      case Reward::ITEM:
        return (new Reward_Item(helper));
      case Reward::RUIN:
        return (new Reward_Ruin(helper));
      case Reward::MAP:
        return (new Reward_Map(helper));
      }
    return 0;
}

Reward::Type Reward::getRandomRewardType(bool exclude_ruins)
{
  std::vector<Reward::Type> types;
  types.push_back(Reward::GOLD);
  types.push_back(Reward::ALLIES);
  types.push_back(Reward::ITEM);
  if (!exclude_ruins)
    types.push_back(Reward::RUIN);
  if (GameScenarioOptions::s_hidden_map)
    types.push_back(Reward::MAP);
  return types[Rnd::rand() % types.size()];
}

Reward* Reward::createRandomReward(bool take_from_list, bool exclude_ruins)
{
  Reward::Type reward_type = Reward::getRandomRewardType (exclude_ruins);
  switch (reward_type)
    {
    case Reward::GOLD:
        {
          Reward *goldReward;
          if (take_from_list)
            {
              goldReward = Rewardlist::getInstance()->pop (Reward::GOLD);
              if (goldReward)
                return goldReward;
            }
          goldReward = Reward_Gold::createRandomReward();
          return goldReward;
        }
      break;
    case Reward::ALLIES:
        {
          Reward *alliesReward;
          if (take_from_list)
            {
              alliesReward = Rewardlist::getInstance()->pop (Reward::ALLIES);
              if (alliesReward)
                return alliesReward;
            }
          alliesReward = Reward_Allies::createRandomReward();
          return alliesReward;
        }
      break;
    case Reward::ITEM:
        {
          Reward *itemReward;
          if (take_from_list)
            {
              itemReward = Rewardlist::getInstance()->pop(Reward::ITEM);
              if (itemReward)
                return itemReward;
            }
          itemReward = Reward_Gold::createRandomReward();
          return itemReward;
        }
      break;
    case Reward::RUIN:
        {
          Reward *ruinReward;
          if (take_from_list)
            {
              ruinReward = Rewardlist::getInstance()->pop (Reward::RUIN);
              if (ruinReward)
                return ruinReward;
            }
          ruinReward = Reward_Gold::createRandomReward();
          return ruinReward;
        }
      break;
    case Reward::MAP:
        {
          Reward *mapReward;
          if (take_from_list)
            {
              mapReward = Rewardlist::getInstance()->pop (Reward::MAP);
              if (mapReward)
                return mapReward;
            }
          mapReward = Reward_Map::createRandomReward();
          return mapReward;
        }
      break;
    }
  return NULL;
}

Reward_Gold::Reward_Gold(guint32 gold)
    :Reward(Reward::GOLD), d_gold(gold)
{
}

Reward_Gold::Reward_Gold(XML_Helper* helper)
    :Reward(helper)
{
  d_type = Reward::GOLD;
  helper->getData(d_gold, "gold");
}

Reward_Gold::Reward_Gold (const Reward_Gold & orig, bool sync_id)
	:Reward(orig, sync_id), d_gold(orig.d_gold)
{
}

bool Reward_Gold::save(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->openTag(Reward::d_tag);
  Glib::ustring type_str = rewardTypeToString(Reward::Type(d_type));
  retval &= helper->saveData("type", type_str);
  retval &= helper->saveData("name", d_name);
  retval &= helper->saveData("gold", d_gold);
  retval &= helper->closeTag();
  return retval;
}

Reward_Gold::~Reward_Gold()
{
}

Reward_Gold *Reward_Gold::createRandomReward()
{
  int gold = Reward_Gold::getRandomGoldPieces();
  return new Reward_Gold(gold);
}

guint32 Reward_Gold::getRandomGoldPieces()
{
  return 310 + (Rnd::rand() % 1000);
}

guint32 Reward_Gold::getRandomSageGoldPieces()
{
  return 600 + (Rnd::rand() % 1500);
}

Reward_Allies::Reward_Allies()
    :Reward(Reward::ALLIES), 
    d_army (NULL), d_army_set (0), d_army_type (0), d_count(1)
{
}

Reward_Allies::Reward_Allies(guint32 army_type, guint32 army_set, guint32 count)
    :Reward(Reward::ALLIES), 
    d_army (Armysetlist::getInstance()->getArmy (army_set, army_type)),
    d_army_set (army_set), d_army_type (army_type), d_count(count)
{
}

Reward_Allies::Reward_Allies(const ArmyProto *army, guint32 count)
    :Reward(Reward::ALLIES), d_army (army), d_army_set (army->getArmyset ()),
    d_army_type (army->getId ()), d_count(count)
{
}

Reward_Allies::Reward_Allies(XML_Helper* helper)
    :Reward(helper)
{
  d_type = Reward::ALLIES;
  helper->getData(d_count, "num_allies");
  helper->getData(d_army_type, "ally_type");
  helper->getData(d_army_set, "ally_armyset");
  d_army = Armysetlist::getInstance()->getArmy (d_army_set, d_army_type);
}

Reward_Allies::Reward_Allies (const Reward_Allies& orig, bool sync_id)
	:Reward(orig, sync_id), d_army(orig.d_army),
        d_army_set(orig.d_army_set), d_army_type(orig.d_army_type),
        d_count(orig.d_count)
{
}

bool Reward_Allies::save(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->openTag(Reward::d_tag);
  Glib::ustring type_str = rewardTypeToString(Reward::Type(d_type));
  retval &= helper->saveData("type", type_str);
  retval &= helper->saveData("name", d_name);
  retval &= helper->saveData("num_allies", d_count);
  retval &= helper->saveData("ally_type", d_army_type);
  retval &= helper->saveData("ally_armyset", d_army_set);
  retval &= helper->closeTag();
  return retval;
}
	
guint32 Reward_Allies::getRandomAmountOfAllies()
{
  int percent = Rnd::rand() % 100;
  if (percent < 30)
    return 1;
  else if (percent < 50)
    return 2;
  else if (percent < 70)
    return 3;
  else if (percent < 80)
    return 4;
  else if (percent < 85)
    return 5;
  else if (percent < 90)
    return 6;
  else if (percent < 95)
    return 7;
  else if (percent < 100)
    return 8;
  else
    return 1;
}

const ArmyProto* Reward_Allies::randomArmyAlly()
{
  Player *p = Playerlist::getInstance()->getActiveplayer();
  if (!p)
    p = Playerlist::getInstance()->getNeutral();
  return Armysetlist::getInstance()->get(p->getArmyset())->getRandomAwardableAlly();
}

bool Reward_Allies::addAllies(Player *p, Vector<int> pos, const ArmyProto *army, guint32 alliesCount, StackReflist *stacks)
{
  for (unsigned int i = 0; i < alliesCount; i++)
    {
      Army* ally = new Army(*army, p);
      ally->setUpkeep(0);
      Stack *s = GameMap::getInstance()->addArmy(pos, ally);
      if (s == NULL)
        return false;
      else if (stacks)
        {
          if (stacks->contains(s->getId()) == false)
            stacks->addStack(s);
        }
    }
  return true;
}

bool Reward_Allies::addAllies(Player *p, Location *l, const Army *army, guint32 alliesCount, StackReflist *stacks)
{
  for (unsigned int i = 0; i < alliesCount; i++)
    {
      Army* ally = new Army(*army, p);
      ally->setUpkeep(0);
      Stack *s = GameMap::getInstance()->addArmy(l, ally);
      if (s == NULL)
        return false;
      else if (stacks)
        {
          if (stacks->contains(s->getId()) == false)
            stacks->addStack(s);
        }
    }
  return true;
}

void Reward_Allies::clearAllies ()
{
  d_count = 0;
  d_army = NULL;
  d_army_type = 0;
  d_army_set = 0;
}

void Reward_Allies::setArmy (const ArmyProto *a)
{
  d_army = a;
}

Reward_Allies::~Reward_Allies()
{
}

Reward_Allies * Reward_Allies::createRandomReward()
{
  int num = (Rnd::rand() % 8) + 1;
  const ArmyProto *a = Reward_Allies::randomArmyAlly();
  Reward_Allies *reward = new Reward_Allies(a, num);
  return reward;
}

Reward_Item::Reward_Item(Item *item)
    :Reward(Reward::ITEM)
{
  if (item)
    d_item = new Item (*item);
  else
    d_item = NULL;
}

Reward_Item::Reward_Item()
    :Reward(Reward::ITEM), d_item (NULL)
{
}

bool Reward_Item::loadItem(Glib::ustring tag, XML_Helper* helper)
{
  if (tag == Item::d_tag)
    {
      d_item = new Item(helper);
      return true;
    }

  return false;
}

Reward_Item::Reward_Item(XML_Helper* helper)
 : Reward(helper)
{
  d_type = Reward::ITEM;
  helper->registerTag(Item::d_tag, sigc::mem_fun(this, &Reward_Item::loadItem));
}

Reward_Item::Reward_Item (const Reward_Item& orig, bool sync_id)
 : Reward(orig, sync_id)
{
  if (orig.d_item)
    d_item = new Item(*orig.d_item, sync_id);
  else
    d_item = NULL;
}

bool Reward_Item::save(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->openTag(Reward::d_tag);
  Glib::ustring type_str = rewardTypeToString(Reward::Type(d_type));
  retval &= helper->saveData("type", type_str);
  retval &= helper->saveData("name", d_name);
  retval &= d_item->save(helper);
  retval &= helper->closeTag();
  return retval;
}

Item *Reward_Item::getRandomItem()
{
  Itemlist::iterator it = Itemlist::getInstance()->begin();
  guint32 id = Rnd::rand() % Itemlist::getInstance()->size();
  std::advance(it, id);
  ItemProto *i = it->second;
  return new Item(*i, id);
}

Reward_Item::~Reward_Item()
{
  if (d_item)
    delete d_item;
}

Reward_Ruin::Reward_Ruin(Ruin *ruin)
    :Reward(Reward::RUIN)
{
  if (ruin)
    d_ruin_pos = ruin->getPos ();
  else
    d_ruin_pos = Vector<int>(-1, -1);
}

Reward_Ruin::Reward_Ruin()
    :Reward(Reward::RUIN), d_ruin_pos (Vector<int>(-1,-1))
{
}

Reward_Ruin::Reward_Ruin(XML_Helper* helper)
    :Reward(helper)
{
  d_type = Reward::RUIN;
  guint32 x;
  guint32 y;
  helper->getData(x, "x");
  helper->getData(y, "y");
  d_ruin_pos = Vector<int>(x,y);
}

Reward_Ruin::Reward_Ruin (const Reward_Ruin& orig, bool sync_id)
	:Reward(orig, sync_id), d_ruin_pos(orig.d_ruin_pos)
{
}

bool Reward_Ruin::save(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->openTag(Reward::d_tag);
  Glib::ustring type_str = rewardTypeToString(Reward::Type(d_type));
  retval &= helper->saveData("type", type_str);
  retval &= helper->saveData("name", d_name);
  retval &= helper->saveData("x", getRuin()->getPos().x);
  retval &= helper->saveData("y", getRuin()->getPos().y);
  retval &= helper->closeTag();
  return retval;
}

Ruin *Reward_Ruin::getRandomHiddenRuin()
{
  std::vector<Ruin *>hidden_ruins;
  for (auto it: *Ruinlist::getInstance())
    {
      if (it->isHidden())
	if (it->getOwner() == NULL || 
	    it->getOwner() == Playerlist::getInstance()->getNeutral())
	  {
	    //is it already being pointed to by a reward in the rewardlist?
	    bool found = false;
	    for (auto i: *Rewardlist::getInstance())
	      {
		if (i->getType() == Reward::RUIN)
		  {
		    Ruin *r = static_cast<Reward_Ruin*>(i)->getRuin();
		    if (r)
		      {
			if (r->getPos() == it->getPos())
			  {
			    found = true;
			    break;
			  }
		      }
		  }
	      }
	    if (found == false)
	      hidden_ruins.push_back(it);
	  }
    }
 if (hidden_ruins.empty())
   return NULL;
 return hidden_ruins[Rnd::rand() % hidden_ruins.size()];
}

Reward_Ruin::~Reward_Ruin()
{
}

Reward_Map::Reward_Map(Vector<int> pos, Glib::ustring name, 
		       guint32 height, guint32 width)
    :Reward(Reward::MAP, name)
{
  d_sightmap = new SightMap(name, pos, height, width);
}

bool Reward_Map::loadMap(Glib::ustring tag, XML_Helper* helper)
{
  if (tag == SightMap::d_tag)
    {
      d_sightmap = new SightMap(helper);
      return true;
    }

  return false;
}

Reward_Map::Reward_Map(XML_Helper* helper)
    :Reward(helper)
{
  helper->registerTag(SightMap::d_tag, sigc::mem_fun(this, &Reward_Map::loadMap));
}

Reward_Map::Reward_Map (const Reward_Map& orig, bool sync_id)
	:Reward(orig, sync_id)
{
  d_sightmap = new SightMap(*orig.d_sightmap);
}

bool Reward_Map::save(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->openTag(Reward::d_tag);
  Glib::ustring type_str = rewardTypeToString(Reward::Type(d_type));
  retval &= helper->saveData("type", type_str);
  retval &= helper->saveData("name", d_sightmap->getName());
  retval &= d_sightmap->save(helper);
  retval &= helper->closeTag();
  return retval;
}

void Reward_Map::getRandomMap(int *x, int *y, int *width, int *height)
{
  int map_width = GameMap::getInstance()->getWidth();
  *x = Rnd::rand() % (map_width - (map_width / 10));
  int map_height = GameMap::getInstance()->getHeight();
  *y = Rnd::rand() % (map_height - (map_height / 10));
  *width = ((Rnd::rand() % (map_width - *x)) + (map_width / 10));
  *height = ((Rnd::rand() % (map_height - *y)) + (map_height / 10));
}

Reward_Map::~Reward_Map()
{
  if (d_sightmap)
    delete d_sightmap;
}

Reward_Map *Reward_Map::createRandomReward()
{
  int x = 0, y = 0, width = 0, height = 0;
  Reward_Map::getRandomMap(&x, &y, &width, &height);
  return new Reward_Map(Vector<int>(x,y),
                        Reward_Map::getRandomName(), height, width);
}

Glib::ustring Reward::getDescription() const
{
  Glib::ustring s = "";
  switch (getType())
    {
    case Reward::GOLD:
	{
	  const Reward_Gold *g = dynamic_cast<const Reward_Gold*>(this);
	  s += String::ucompose(ngettext("%1 Gold Piece", "%1 Gold Pieces", 
					 g->getGold()), g->getGold());
	  return s;
	}
    case Reward::ALLIES:
	{
	  const Reward_Allies *a = dynamic_cast<const Reward_Allies *>(this);
	  if (a->getArmy())
	    s += String::ucompose(_("%1 x %2"), a->getArmy()->getName(),
				  a->getNoOfAllies());
	  return s;
	}
    case Reward::ITEM:
	{
	  const Reward_Item *i = dynamic_cast<const Reward_Item *>(this);
	  if (i->getItem())
	    s += i->getItem()->getName();
	  return s;
	}
    case Reward::RUIN:
	{
	  const Reward_Ruin *r = dynamic_cast<const Reward_Ruin *>(this);
	  if (r->getRuin())
	    s += r->getRuin()->getName();
	  return s;
	}
    case Reward::MAP:
	{
	  const Reward_Map *m = dynamic_cast<const Reward_Map *>(this);
	  s += String::ucompose(_("Map: %1,%2 %3x%4"), 
				  m->getLocation().x, m->getLocation().y,
				  m->getSightMap()->h, 
                                  m->getSightMap()->w);
	  return s;
	}
    }
  return s;
}

Glib::ustring Reward::rewardTypeToString(const Reward::Type type)
{
  switch (type)
    {
      case Reward::GOLD: return "Reward::GOLD";
      case Reward::ALLIES: return "Reward::ALLIES";
      case Reward::ITEM: return "Reward::ITEM";
      case Reward::RUIN: return "Reward::RUIN";
      case Reward::MAP: return "Reward::MAP";
    }
  return "Reward::GOLD";
}

Reward::Type Reward::rewardTypeFromString(const Glib::ustring str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return Reward::Type(atoi(str.c_str()));
  if (str == "Reward::GOLD") return Reward::GOLD;
  else if (str == "Reward::ALLIES") return Reward::ALLIES;
  else if (str == "Reward::ITEM") return Reward::ITEM;
  else if (str == "Reward::RUIN") return Reward::RUIN;
  else if (str == "Reward::MAP") return Reward::MAP;
  return Reward::GOLD;
}

Reward* Reward::copy(const Reward* r)
{
  switch(r->getType())
    {
    case  Reward::GOLD:
      return (new Reward_Gold(*dynamic_cast<const Reward_Gold*>(r)));
    case  Reward::ALLIES:
      return (new Reward_Allies(*dynamic_cast<const Reward_Allies*>(r)));
    case Reward::ITEM:
      return (new Reward_Item(*dynamic_cast<const Reward_Item*>(r)));
    case Reward::RUIN:
      return (new Reward_Ruin(*dynamic_cast<const Reward_Ruin*>(r)));
    case Reward::MAP:
      return (new Reward_Map(*dynamic_cast<const Reward_Map*>(r)));
    }
  return 0;
}

Glib::ustring Reward_Map::getMapName() const
{
  return d_sightmap->getName();
}

SightMap * Reward_Map::getSightMap() const
{
  return d_sightmap;
}

Vector<int> Reward_Map::getLocation() const
{
  return d_sightmap->pos;
}

void Reward_Map::setMapName(Glib::ustring name)
{
  d_sightmap->setName(name);
}

Glib::ustring Reward_Map::getRandomName()
{
  std::vector<Glib::ustring> names;
  names.push_back(_("old map"));
  names.push_back(_("old dusty map"));
  names.push_back(_("parchment map"));
  names.push_back(_("vellum map"));
  names.push_back(_("paper map"));
  names.push_back(_("torn paper map"));
  names.push_back(_("dusty map"));
  names.push_back(_("blood-stained map"));
  return names[Rnd::rand() % names.size()];
}
