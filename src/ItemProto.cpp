// Copyright (C) 2008, 2010, 2011, 2014, 2020 Ben Asselstine
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

#include <vector>
#include "ItemProto.h"
#include "ucompose.hpp"
#include "defs.h"
#include "maptile.h"
#include "playerlist.h"
#include "armysetlist.h"
#include "armyproto.h"
#include "xmlhelper.h"

Glib::ustring ItemProto::d_itemproto_tag = "itemproto";

ItemProto::ItemProto(XML_Helper* helper)
	: Renamable (helper)
{
    
    // Loading of items is a bit complicated, so i'd better loose some words.
    // In general, items can be loaded from the items description file or
    // from a savegame. They both differ a bit, more on that when we encounter
    // such a situation. First, let us deal with the common things.

    Glib::ustring bonus_str;
    helper->getData(bonus_str, "bonus");
    d_bonus = bonusFlagsFromString(bonus_str);

    d_has_army_type_to_summon = false;
    d_has_army_type_to_raise = false;
    d_has_army_type_to_kill = false;
    d_uses_left = 0;
    d_steal_gold_percent = 0.0;
    d_army_type_to_kill = 0;
    d_army_type_to_summon = 0;
    d_building_type_to_summon_on = 0;
    d_percent_armies_to_kill = 0.0;
    d_mp_to_add = 0;
    d_army_type_to_raise = 0;
    d_num_armies_to_raise = 0;

    if (isUsable())
      {
        helper->getData(d_uses_left, "uses_left");
        if (d_bonus & ItemProto::STEAL_GOLD)
          helper->getData(d_steal_gold_percent, "steal_gold_percent");
        if (d_bonus & ItemProto::BANISH_WORMS)
          {
            helper->getData(d_army_type_to_kill, "army_type_to_kill");
            d_has_army_type_to_kill = true;
          }
        if (d_bonus & ItemProto::SUMMON_MONSTER)
          {
            helper->getData(d_army_type_to_summon, "army_type_to_summon");
            Glib::ustring str;
            helper->getData(str, "building_type_to_summon_on");
            d_building_type_to_summon_on = Maptile::buildingFromString(str);
            d_has_army_type_to_summon = true;
          }
        if (d_bonus & ItemProto::DISEASE_CITY)
          helper->getData(d_percent_armies_to_kill, "percent_armies_to_kill");
        if (d_bonus & ItemProto::ADD_2MP_STACK)
          helper->getData(d_mp_to_add, "mp_to_add");
        if (d_bonus & ItemProto::RAISE_DEFENDERS)
          {
            helper->getData(d_army_type_to_raise, "army_type_to_raise");
            helper->getData(d_num_armies_to_raise, "num_armies_to_raise");
            d_has_army_type_to_raise = true;
          }
      }
}

ItemProto::ItemProto(Glib::ustring name)
 : Renamable(name), d_bonus (0), d_uses_left (0), d_army_type_to_kill (0),
    d_steal_gold_percent (0.0), d_army_type_to_summon (0),
    d_building_type_to_summon_on (0), d_percent_armies_to_kill (0.0),
    d_mp_to_add (0), d_army_type_to_raise (0), d_num_armies_to_raise (0),
    d_has_army_type_to_kill (false), d_has_army_type_to_summon (false),
    d_has_army_type_to_raise (false)
{
}

ItemProto::ItemProto(const ItemProto& orig)
:Renamable(orig), d_bonus(orig.d_bonus), d_uses_left(orig.d_uses_left),
    d_army_type_to_kill(orig.d_army_type_to_kill),
    d_steal_gold_percent(orig.d_steal_gold_percent),
    d_army_type_to_summon(orig.d_army_type_to_summon),
    d_building_type_to_summon_on(orig.d_building_type_to_summon_on),
    d_percent_armies_to_kill(orig.d_percent_armies_to_kill),
    d_mp_to_add(orig.d_mp_to_add),
    d_army_type_to_raise(orig.d_army_type_to_raise),
    d_num_armies_to_raise(orig.d_num_armies_to_raise),
    d_has_army_type_to_kill (orig.d_has_army_type_to_kill),
    d_has_army_type_to_summon (orig.d_has_army_type_to_summon),
    d_has_army_type_to_raise (orig.d_has_army_type_to_raise)
{
}

bool ItemProto::saveContents(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("name", getName(false));
  Glib::ustring bonus_str = bonusFlagsToString(d_bonus);
  retval &= helper->saveData("bonus", bonus_str);
  if (isUsable())
      {
        retval &= helper->saveData("uses_left", d_uses_left);
        if (d_bonus & ItemProto::STEAL_GOLD)
          retval &= helper->saveData("steal_gold_percent", 
                                     d_steal_gold_percent);
        if (d_bonus & ItemProto::BANISH_WORMS)
          retval &= helper->saveData("army_type_to_kill", d_army_type_to_kill);
        if (d_bonus & ItemProto::SUMMON_MONSTER)
          {
            retval &= helper->saveData("army_type_to_summon", 
                                       d_army_type_to_summon);
            Glib::ustring type_str = 
              Maptile::buildingToString
              (Maptile::Building(d_building_type_to_summon_on));
            retval &= helper->saveData("building_type_to_summon_on", type_str);
          }
        if (d_bonus & ItemProto::DISEASE_CITY)
          retval &= helper->saveData("percent_armies_to_kill", 
                                     d_percent_armies_to_kill);
        if (d_bonus & ItemProto::ADD_2MP_STACK)
          retval &= helper->saveData("mp_to_add", d_mp_to_add);
        if (d_bonus & ItemProto::RAISE_DEFENDERS)
          {
            retval &= helper->saveData("army_type_to_raise", 
                                       d_army_type_to_raise);
            retval &= helper->saveData("num_armies_to_raise", 
                                       d_num_armies_to_raise);
          }
      }

  return retval;
}

bool ItemProto::save(XML_Helper* helper) const
{
  bool retval = true;
  
  retval &= helper->openTag(d_itemproto_tag);
  retval &= saveContents(helper);
  retval &= helper->closeTag();

  return retval;
}

bool ItemProto::getBonus(ItemProto::Bonus bonus) const
{
  return (d_bonus & bonus) == 0 ? false : true;
}

void ItemProto::addBonus(ItemProto::Bonus bonus)
{
  d_bonus |= bonus;
}

void ItemProto::removeBonus(ItemProto::Bonus bonus)
{
  d_bonus ^= bonus;
}

Glib::ustring ItemProto::getBonusDescription() const
{
  guint32 battle = 0;
  guint32 command = 0;
  guint32 goldpercity = 0;
  // the attributes column
  std::vector<Glib::ustring> s;
  if (getBonus(ItemProto::ADD1STR))
    battle++;
  if (getBonus(ItemProto::ADD2STR))
    battle+=2;
  if (getBonus(ItemProto::ADD3STR))
    battle+=3;
  if (getBonus(ItemProto::ADD1STACK))
    command++;
  if (getBonus(ItemProto::ADD2STACK))
    command+=2;
  if (getBonus(ItemProto::ADD3STACK))
    command+=3;
  if (getBonus(ItemProto::FLYSTACK))
    s.push_back(_("Allows Flight"));
  if (getBonus(ItemProto::DOUBLEMOVESTACK))
    s.push_back(_("Doubles Movement"));
  if (getBonus(ItemProto::ADD2GOLDPERCITY))
    goldpercity+=2;
  if (getBonus(ItemProto::ADD3GOLDPERCITY))
    goldpercity+=3;
  if (getBonus(ItemProto::ADD4GOLDPERCITY))
    goldpercity+=4;
  if (getBonus(ItemProto::ADD5GOLDPERCITY))
    goldpercity+=5;
  if (getBonus(ItemProto::STEAL_GOLD))
    s.push_back(_("Steals Gold"));
  if (getBonus(ItemProto::SINK_SHIPS))
    s.push_back(_("Sink Ships"));
  if (getBonus(ItemProto::PICK_UP_BAGS))
    s.push_back(_("Picks Up Bags"));
  if (getBonus(ItemProto::ADD_2MP_STACK))
    s.push_back(_("+2 MP to stack"));
  if (getBonus(ItemProto::BANISH_WORMS))
    {
      ArmyProto *a = Armysetlist::getInstance()->getArmy(Playerlist::getActiveplayer()->getArmyset(), d_army_type_to_kill);
      s.push_back(String::ucompose(_("Kills all %1"), a->getName()));
    }
  if (getBonus(ItemProto::BURN_BRIDGE))
    s.push_back(_("Destroys a Bridge"));
  if (getBonus(ItemProto::CAPTURE_KEEPER))
    s.push_back(_("Removes Monster from Ruin"));
  if (getBonus(ItemProto::DISEASE_CITY))
    s.push_back(_("Kills Defenders in a City"));
  if (getBonus(ItemProto::SUMMON_MONSTER))
    {
      ArmyProto *a = Armysetlist::getInstance()->getArmy(Playerlist::getActiveplayer()->getArmyset(), d_army_type_to_summon);
      if (d_building_type_to_summon_on != Maptile::NONE)
        s.push_back(String::ucompose(_("Summons %1 at a %2"), a->getName(),
                                     Maptile::buildingToFriendlyName(d_building_type_to_summon_on)));
      else
        s.push_back(String::ucompose(_("Summons %1"), a->getName()));
    }
  if (getBonus(ItemProto::RAISE_DEFENDERS))
    s.push_back(_("Add Defenders to a City"));
  if (getBonus(ItemProto::PERSUADE_NEUTRALS))
    s.push_back(_("Take a Neutral City"));
  if (getBonus(ItemProto::TELEPORT_TO_CITY))
    s.push_back(_("Teleport Stack to a City"));
  if (getBonus(ItemProto::PLANT_TO_VECTOR))
    s.push_back(_("Army units can be Vectored when Planted"));

  if (battle > 0)
    s.push_back(String::ucompose(_("+%1 Battle"), battle));
  if (command > 0)
    s.push_back(String::ucompose(_("+%1 Command"), command));
  if (goldpercity > 0)
    s.push_back(String::ucompose(_("+%1 Gold per City"), goldpercity));


  Glib::ustring str;
  bool first = true;
  for (std::vector<Glib::ustring>::iterator i = s.begin(), end = s.end();
       i != end; ++i)
    {
      if (first)
	first = false;
      else
	str += "\n";
      str += *i;
    }
  return str;
}

Glib::ustring ItemProto::bonusFlagToString(ItemProto::Bonus bonus)
{
  switch (bonus)
    {
    case ItemProto::ADD1STR: return "ItemProto::ADD1STR";
    case ItemProto::ADD2STR: return "ItemProto::ADD2STR";
    case ItemProto::ADD3STR: return "ItemProto::ADD3STR";
    case ItemProto::ADD1STACK: return "ItemProto::ADD1STACK";
    case ItemProto::ADD2STACK: return "ItemProto::ADD2STACK";
    case ItemProto::ADD3STACK: return "ItemProto::ADD3STACK";
    case ItemProto::FLYSTACK: return "ItemProto::FLYSTACK";
    case ItemProto::DOUBLEMOVESTACK: return "ItemProto::DOUBLEMOVESTACK";
    case ItemProto::ADD2GOLDPERCITY: return "ItemProto::ADD2GOLDPERCITY";
    case ItemProto::ADD3GOLDPERCITY: return "ADD3GOLDPERCITY";
    case ItemProto::ADD4GOLDPERCITY: return "ItemProto::ADD4GOLDPERCITY";
    case ItemProto::ADD5GOLDPERCITY: return "ItemProto::ADD5GOLDPERCITY";
    case ItemProto::STEAL_GOLD: return "ItemProto::STEAL_GOLD";
    case ItemProto::SINK_SHIPS: return "ItemProto::SINK_SHIPS";
    case ItemProto::PICK_UP_BAGS: return "ItemProto::PICK_UP_BAGS";
    case ItemProto::ADD_2MP_STACK: return "ItemProto::ADD_2MP_STACK";
    case ItemProto::BANISH_WORMS: return "ItemProto::BANISH_WORMS";
    case ItemProto::BURN_BRIDGE: return "ItemProto::BURN_BRIDGE";
    case ItemProto::CAPTURE_KEEPER: return "ItemProto::CAPTURE_KEEPER";
    case ItemProto::SUMMON_MONSTER: return "ItemProto::SUMMON_MONSTER";
    case ItemProto::DISEASE_CITY: return "ItemProto::DISEASE_CITY";
    case ItemProto::RAISE_DEFENDERS: return "ItemProto::RAISE_DEFENDERS";
    case ItemProto::PERSUADE_NEUTRALS: return "ItemProto::PERSUADE_NEUTRALS";
    case ItemProto::TELEPORT_TO_CITY: return "ItemProto::TELEPORT_TO_CITY";
    case ItemProto::PLANT_TO_VECTOR: return "ItemProto::PLANT_TO_VECTOR";
    }
  return "ItemProto::ADD1STR";
}

Glib::ustring ItemProto::bonusFlagsToString(guint32 bonus)
{
  Glib::ustring bonuses;
  if (bonus & ItemProto::ADD1STR)
    bonuses += " " + bonusFlagToString(ItemProto::ADD1STR);
  if (bonus & ItemProto::ADD2STR)
    bonuses += " " + bonusFlagToString(ItemProto::ADD2STR);
  if (bonus & ItemProto::ADD3STR)
    bonuses += " " + bonusFlagToString(ItemProto::ADD3STR);
  if (bonus & ItemProto::ADD1STACK)
    bonuses += " " + bonusFlagToString(ItemProto::ADD1STACK);
  if (bonus & ItemProto::ADD2STACK)
    bonuses += " " + bonusFlagToString(ItemProto::ADD2STACK);
  if (bonus & ItemProto::ADD3STACK)
    bonuses += " " + bonusFlagToString(ItemProto::ADD3STACK);
  if (bonus & ItemProto::FLYSTACK)
    bonuses += " " + bonusFlagToString(ItemProto::FLYSTACK);
  if (bonus & ItemProto::DOUBLEMOVESTACK)
    bonuses += " " + bonusFlagToString(ItemProto::DOUBLEMOVESTACK);
  if (bonus & ItemProto::ADD2GOLDPERCITY)
    bonuses += " " + bonusFlagToString(ItemProto::ADD2GOLDPERCITY);
  if (bonus & ItemProto::ADD3GOLDPERCITY)
    bonuses += " " + bonusFlagToString(ItemProto::ADD3GOLDPERCITY);
  if (bonus & ItemProto::ADD4GOLDPERCITY)
    bonuses += " " + bonusFlagToString(ItemProto::ADD4GOLDPERCITY);
  if (bonus & ItemProto::ADD5GOLDPERCITY)
    bonuses += " " + bonusFlagToString(ItemProto::ADD5GOLDPERCITY);
  if (bonus & ItemProto::STEAL_GOLD)
    bonuses += " " + bonusFlagToString(ItemProto::STEAL_GOLD);
  if (bonus & ItemProto::SINK_SHIPS)
    bonuses += " " + bonusFlagToString(ItemProto::SINK_SHIPS);
  if (bonus & ItemProto::PICK_UP_BAGS)
    bonuses += " " + bonusFlagToString(ItemProto::PICK_UP_BAGS);
  if (bonus & ItemProto::ADD_2MP_STACK)
    bonuses += " " + bonusFlagToString(ItemProto::ADD_2MP_STACK);
  if (bonus & ItemProto::BANISH_WORMS)
    bonuses += " " + bonusFlagToString(ItemProto::BANISH_WORMS);
  if (bonus & ItemProto::BURN_BRIDGE)
    bonuses += " " + bonusFlagToString(ItemProto::BURN_BRIDGE);
  if (bonus & ItemProto::CAPTURE_KEEPER)
    bonuses += " " + bonusFlagToString(ItemProto::CAPTURE_KEEPER);
  if (bonus & ItemProto::SUMMON_MONSTER)
    bonuses += " " + bonusFlagToString(ItemProto::SUMMON_MONSTER);
  if (bonus & ItemProto::DISEASE_CITY)
    bonuses += " " + bonusFlagToString(ItemProto::DISEASE_CITY);
  if (bonus & ItemProto::RAISE_DEFENDERS)
    bonuses += " " + bonusFlagToString(ItemProto::RAISE_DEFENDERS);
  if (bonus & ItemProto::PERSUADE_NEUTRALS)
    bonuses += " " + bonusFlagToString(ItemProto::PERSUADE_NEUTRALS);
  if (bonus & ItemProto::TELEPORT_TO_CITY)
    bonuses += " " + bonusFlagToString(ItemProto::TELEPORT_TO_CITY);
  if (bonus & ItemProto::PLANT_TO_VECTOR)
    bonuses += " " + bonusFlagToString(ItemProto::PLANT_TO_VECTOR);
  return bonuses;
}

guint32 ItemProto::bonusFlagsFromString(Glib::ustring str)
{
  return XML_Helper::flagsFromString(str, bonusFlagFromString);
}

guint32 ItemProto::bonusFlagFromString(Glib::ustring str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return ItemProto::Bonus(atoi(str.c_str()));
  if (str == "ItemProto::ADD1STR") return ItemProto::ADD1STR;
  else if (str == "ItemProto::ADD2STR") return ItemProto::ADD2STR;
  else if (str == "ItemProto::ADD3STR") return ItemProto::ADD3STR;
  else if (str == "ItemProto::ADD1STACK") return ItemProto::ADD1STACK;
  else if (str == "ItemProto::ADD2STACK") return ItemProto::ADD2STACK;
  else if (str == "ItemProto::ADD3STACK") return ItemProto::ADD3STACK;
  else if (str == "ItemProto::FLYSTACK") return ItemProto::FLYSTACK;
  else if (str == "ItemProto::DOUBLEMOVESTACK") return ItemProto::DOUBLEMOVESTACK;
  else if (str == "ItemProto::ADD2GOLDPERCITY") return ItemProto::ADD2GOLDPERCITY;
  else if (str == "ItemProto::ADD3GOLDPERCITY") return ItemProto::ADD3GOLDPERCITY;
  else if (str == "ItemProto::ADD4GOLDPERCITY") return ItemProto::ADD4GOLDPERCITY;
  else if (str == "ItemProto::ADD5GOLDPERCITY") return ItemProto::ADD5GOLDPERCITY;
  else if (str == "ItemProto::STEAL_GOLD") return ItemProto::STEAL_GOLD;
  else if (str == "ItemProto::SINK_SHIPS") return ItemProto::SINK_SHIPS;
  else if (str == "ItemProto::PICK_UP_BAGS") return ItemProto::PICK_UP_BAGS;
  else if (str == "ItemProto::ADD_2MP_STACK") return ItemProto::ADD_2MP_STACK;
  else if (str == "ItemProto::BANISH_WORMS") return ItemProto::BANISH_WORMS;
  else if (str == "ItemProto::BURN_BRIDGE") return ItemProto::BURN_BRIDGE;
  else if (str == "ItemProto::CAPTURE_KEEPER") return ItemProto::CAPTURE_KEEPER;
  else if (str == "ItemProto::SUMMON_MONSTER") return ItemProto::SUMMON_MONSTER;
  else if (str == "ItemProto::DISEASE_CITY") return ItemProto::DISEASE_CITY;
  else if (str == "ItemProto::RAISE_DEFENDERS") return ItemProto::RAISE_DEFENDERS;
  else if (str == "ItemProto::PERSUADE_NEUTRALS") return ItemProto::PERSUADE_NEUTRALS;
  else if (str == "ItemProto::TELEPORT_TO_CITY") return ItemProto::TELEPORT_TO_CITY;
  else if (str == "ItemProto::PLANT_TO_VECTOR") return ItemProto::PLANT_TO_VECTOR;
  return ItemProto::ADD1STR;
}

bool ItemProto::isCurrentlyUsable(guint32 building, bool bags_on_map, bool victims_left, bool ruin_has_occupant, bool friendly_cities_present, bool enemy_cities_present, bool neutral_cities_present)
{
  bool usable = false;
  if (d_bonus & ItemProto::BURN_BRIDGE && building == Maptile::BRIDGE)
    usable = true;
  if (d_bonus & ItemProto::SUMMON_MONSTER)
    {
      if (getBuildingTypeToSummonOn() == Maptile::NONE ||
          getBuildingTypeToSummonOn() == building)
        usable = true;
    }
  if (d_bonus & ItemProto::PICK_UP_BAGS && bags_on_map)
    usable = true;
  if (d_bonus & ItemProto::CAPTURE_KEEPER && building == Maptile::RUIN &&
      ruin_has_occupant)
    usable = true;
        
  if (usableOnVictimPlayer() && victims_left)
    usable = true;
  if (usableOnFriendlyCity() && friendly_cities_present)
    usable = true;
  if (usableOnEnemyCity() && enemy_cities_present)
    usable = true;
  if (usableOnNeutralCity() && neutral_cities_present)
    usable = true;
  if (usableOnAnyCity())
    usable = true;

  return usable;
}
