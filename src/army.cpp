// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2007, 2008, 2011, 2014, 2015, 2017, 2021 Ben Asselstine
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
#include "army.h"
#include "armyprodbase.h"
#include "armyproto.h"
#include "armysetlist.h"
#include "counter.h"
#include "xmlhelper.h"
#include "stacklist.h"
#include "templelist.h"
#include "ucompose.hpp"
#include "Tile.h"
#include "player.h"

Glib::ustring Army::d_tag = "army";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

sigc::signal<void, Army*> Army::sdying;

Army::Army(const Army& a, bool sync_id, Player *owner)
    : ArmyBase(a), UniquelyIdentified(a, sync_id), OwnerId(owner),
    sigc::trackable(a), d_type_id(a.d_type_id), d_armyset(a.d_armyset),
    d_max_hp(a.d_max_hp), d_max_moves_multiplier(a.d_max_moves_multiplier),
    d_max_moves_rest_bonus(a.d_max_moves_rest_bonus),
    d_ship(a.d_ship), d_hp(a.d_hp), d_moves(a.d_moves), d_xp(a.d_xp),
    d_level(a.d_level), d_battles_number(a.d_battles_number), 
    d_number_hashit(a.d_number_hashit), 
    d_number_hasbeenhit(a.d_number_hasbeenhit), 
    d_visitedTemples(a.d_visitedTemples)
{
  if (!owner)
    setOwnerId (a.getOwnerId ());
  for(int i = 0; i < 3; i++)
    d_medal_bonus[i] = a.d_medal_bonus[i];
}

Army::Army(const ArmyProto& a, Player* p)
    :ArmyBase(a), UniquelyIdentified(), OwnerId(p), 
    d_type_id(a.getId()), d_armyset(a.getArmyset()), 
    d_max_hp(2), d_max_moves_multiplier(1), d_max_moves_rest_bonus(0),
    d_ship(false), d_hp(2), d_moves(a.getMaxMoves()), d_xp(0), d_level(0),
    d_battles_number(0), d_number_hashit(0), d_number_hasbeenhit(0)
{
  for(int i = 0; i < 3; i++)
    d_medal_bonus[i] = 0;
  d_visitedTemples.clear();
}

Army::Army(const ArmyProto& a, guint32 id, Player *p)
    :ArmyBase(a), UniquelyIdentified(id), OwnerId(p), 
    d_type_id(a.getId()), d_armyset(a.getArmyset()), 
    d_max_hp(2), d_max_moves_multiplier(1), d_max_moves_rest_bonus(0),
    d_ship(false), d_hp(2), d_moves(a.getMaxMoves()), d_xp(0), d_level(0),
    d_battles_number(0), d_number_hashit(0), d_number_hasbeenhit(0)
{
  for(int i = 0; i < 3; i++)
    d_medal_bonus[i] = 0;
  d_visitedTemples.clear();
}

Army::Army(const ArmyProdBase& a, guint32 id, Player *p)
    :ArmyBase(a), UniquelyIdentified(id), OwnerId(p), 
    d_type_id(a.getTypeId()), d_armyset(a.getArmyset()), 
    d_max_hp(2), d_max_moves_multiplier(1), d_max_moves_rest_bonus(0),
    d_ship(false), d_hp(2), d_moves(a.getMaxMoves()), d_xp(0), d_level(0),
    d_battles_number(0), d_number_hashit(0), d_number_hasbeenhit(0)
{
  for(int i = 0; i < 3; i++)
    d_medal_bonus[i] = 0;
  d_visitedTemples.clear();
}

Army* Army::createNonUniqueArmy(const ArmyProto& a, Player *player)
{
  return new Army(a, (guint32) 0, player);
}

Army* Army::createNonUniqueArmy(const ArmyProdBase& a, Player *player)
{
  return new Army(a, (guint32) 0, player);
}

Army::Army(const ArmyProdBase& a, Player* p)
 : ArmyBase(a), UniquelyIdentified(), OwnerId(p), d_type_id(a.getTypeId()),
    d_armyset(a.getArmyset()), d_max_hp(2), d_max_moves_multiplier(1),
    d_max_moves_rest_bonus(0), d_ship(false), d_hp(2), d_moves(a.getMaxMoves()),
    d_xp(0), d_level(0), d_medal_bonus {false}, d_battles_number(0),
    d_number_hashit(0), d_number_hasbeenhit(0)
{
  d_visitedTemples.clear();
}

Army::Army()
 : ArmyBase(), UniquelyIdentified(), OwnerId((Player *)0), d_type_id(0),
    d_armyset(0), d_max_hp(2), d_max_moves_multiplier(1),
    d_max_moves_rest_bonus(0), d_ship(false), d_hp(2), d_moves(0), d_xp(0),
    d_level(0), d_medal_bonus {false},  d_battles_number(0), d_number_hashit(0),
    d_number_hasbeenhit(0)
{
  d_visitedTemples.clear();
}

Army::Army(XML_Helper* helper)
 : ArmyBase(helper), UniquelyIdentified(helper), OwnerId((XML_Helper*) 0),
    d_type_id(0), d_armyset(0), d_max_hp(2), d_max_moves_multiplier(1), 
    d_max_moves_rest_bonus(0), d_ship(false), d_hp(2), d_moves(0), d_xp(0),
    d_level(0), d_medal_bonus {false}, d_battles_number(0), d_number_hashit(0),
    d_number_hasbeenhit(0)
{
  //d_owner_id is not read in here.  it is set to the owner of the stack
  //in stack.cpp
  d_visitedTemples.clear();

  int ival = -1;
  //get the information which army we are
  helper->getData(d_type_id, "type");
  helper->getData(d_armyset, "armyset");

  helper->getData(d_hp, "hp");
  helper->getData(d_ship, "ship");
  helper->getData(d_moves, "moves");
  helper->getData(d_max_moves_multiplier, "max_moves_multiplier");
  helper->getData(d_xp, "xp");
  helper->getData(d_level, "level");

  Glib::ustring medals;
  std::stringstream smedals;
  bool val;

  helper->getData(medals, "medals");
  smedals.str(medals);

  for(int i=0;i<3;i++)
    {
      smedals >> val;
      d_medal_bonus[i]=val;
      debug("ARMY-XML-CONSTRUCTOR medalsbonus[" << i << "]=" << d_medal_bonus[i])
    }

  helper->getData(d_battles_number, "battlesnumber");    

  Glib::ustring temples;
  std::stringstream stemples;
  helper->getData(temples, "visited_temples");
  stemples.str(temples);

  while (stemples.eof() == false)
    {
      ival = -1;
      stemples >> ival;
      if (ival != -1)
	d_visitedTemples.push_front(ival);
    }
}

Army::~Army()
{
  if (d_unique)
    sdying.emit(this);
}

void Army::setStat(Army::Stat stat, guint32 value)
{
  switch (stat)
    {
    case STRENGTH:  
      d_strength = value;
      if (d_strength > MAX_ARMY_STRENGTH)
	d_strength = MAX_ARMY_STRENGTH;
      break;
    case HP:        
      d_max_hp = value;
      if (d_hp > d_max_hp)
	d_hp = value;
      break;
    case MOVES:     
      d_max_moves = value;
      if (d_moves > d_max_moves)
	d_moves = value;
      break;
    case MOVES_MULTIPLIER:
      d_max_moves_multiplier = value;
      break;
    case MOVE_BONUS:    d_move_bonus = value;
			break;
    case ARMY_BONUS:    d_army_bonus = value;
			break;
    case SIGHT:         d_sight = value;
			break;
    case SHIP:          value == 0 ? d_ship = false : d_ship = true;
			break;
    case BOAT_STRENGTH:
                        break;
    }
}

guint32 Army::getStat(Stat stat, bool modified) const
{
  switch (stat)
    {
    case STRENGTH:
      return d_strength;
    case HP:
      return d_max_hp;
    case MOVES:
	{
	  if (modified)
	    return (d_max_moves + d_max_moves_rest_bonus) * d_max_moves_multiplier;
	  else
	    return d_max_moves;
	}
    case MOVE_BONUS:
      return d_move_bonus;
    case ARMY_BONUS:
      return d_army_bonus;
    case SIGHT:
      return d_sight;
    case SHIP:
      return d_ship;
    case MOVES_MULTIPLIER:
      return d_max_moves_multiplier;
    case BOAT_STRENGTH:
        {
          if (d_ship)
            {
              if (d_strength >= MAX_BOAT_STRENGTH)
                return MAX_BOAT_STRENGTH;
              else
                return d_strength;
            }
          else
            return d_strength;
        }
    }

  // should never come to this
  return 0;
}

void Army::resetMoves()
{
  switch (d_moves)
    {
    case 0: d_max_moves_rest_bonus = 0; break;
    case 1: d_max_moves_rest_bonus = 1; break;
    case 2: d_max_moves_rest_bonus = 2; break;
    default: d_max_moves_rest_bonus = 2; break;
    }
  if (d_ship)
    d_moves = MAX_BOAT_MOVES;
  else
    d_moves = getStat(MOVES);
}

/* is this temple one we've already visited? */
bool Army::bless(Temple *temple)
{
  bool visited = false;

  if (!temple)
    return false;

  for (auto it: d_visitedTemples)
    if (it == temple->getId())
      visited = true;

  if (visited == false)  /* no?  increase strength */
    {
      d_visitedTemples.push_back(temple->getId());
      setStat(STRENGTH, d_strength + 1);
    }
  return !visited;
}


void Army::heal(guint32 hp)
{
  if (hp == 0)
    {
      // if no hp are specified, we assume that the healing at the end of
      // the turn takes place. In this case the algorithm is: Heal 10%
      // plus 1HP for each point of vitality above 5 (or one less for each
      // point below 5), heal a minimum of 1 HP per turn
      hp = getStat(HP)/10;
      if (hp <= 5)
	hp = 1;
      else
	hp += 5;
    }

  d_hp += hp;
  if (d_hp > getStat(HP))
    d_hp = getStat(HP);
}

bool Army::damage(guint32 damageDone)
{
  if (damageDone >= d_hp)
    d_hp = 0;
  else
    d_hp -= damageDone;
  return (d_hp == 0);
}

void Army::decrementMoves(guint32 moves)
{
  if (moves >= d_moves)
    d_moves = 0;
  else
    d_moves -= moves;
}

void Army::incrementMoves(guint32 moves)
{
  d_moves += moves;
}

void Army::gainXp(double n)
{
  d_xp += n;
}

bool Army::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag(Army::d_tag);
  retval &= saveData(helper);
  retval &= helper->closeTag();

  return retval;
}

bool Army::saveData(XML_Helper* helper) const
{
  bool retval = true;

  retval &= ArmyBase::saveData(helper);
  retval &= helper->saveData("id", d_id);
  retval &= helper->saveData("armyset", d_armyset);
  retval &= helper->saveData("type", d_type_id);
  retval &= helper->saveData("hp", d_hp);
  retval &= helper->saveData("ship", d_ship);
  retval &= helper->saveData("moves", d_moves);
  retval &= helper->saveData("xp", d_xp);
  retval &= helper->saveData("max_moves_multiplier", 
			     d_max_moves_multiplier);
  retval &= helper->saveData("level", d_level);

  std::stringstream medals;
  for (int i=0;i<3;i++)
    {
      medals << d_medal_bonus[i] << " ";
    }
  retval &= helper->saveData("medals", medals.str());
  retval &= helper->saveData("battlesnumber",d_battles_number);    

  std::stringstream temples;
  for (auto it: d_visitedTemples)
    temples << it << " ";
  retval &= helper->saveData("visited_temples", temples.str());

  return retval;
}

void Army::setInShip (bool s)
{
  if (s == true && isFlyer() == true)
    s = false;
  d_ship = s;
}

bool Army::isFlyer() const
{
  return d_move_bonus == Tile::isFlying();
}

//! Sets this army as being fortified (+1 to stack)
void Army::setFortified (bool f)
{
  if (getFortified() == true && f == false)
    d_army_bonus ^= Army::FORTIFY;
  else if (getFortified() == false && f == true)
    d_army_bonus |= Army::FORTIFY;
}

//! get the fortify flag for this army
bool Army::getFortified () const
{
  return (d_army_bonus & Army::FORTIFY) == Army::FORTIFY;
}

bool Army::blessedAtTemple(guint32 temple_id) const
{
  unsigned int id = temple_id;
  if (std::find (d_visitedTemples.begin(), d_visitedTemples.end(), id) ==
      d_visitedTemples.end())
    return false;

  return true;
}

bool Army::getDefendsRuins() const
{
  ArmyProto *a = Armysetlist::getInstance()->getArmy(d_armyset, d_type_id);
  if (a)
    return a->getDefendsRuins();
  else
    return false;
}

bool Army::getAwardable() const
{
  ArmyProto *a = Armysetlist::getInstance()->getArmy(d_armyset, d_type_id);
  if (a)
    return a->getAwardable();
  else
    return false;
}

Glib::ustring Army::getName() const
{
  ArmyProto *a = Armysetlist::getInstance()->getArmy(d_armyset, d_type_id);
  if (a)
    return a->getName();
  else
    return "";
}

void Army::morph(const ArmyProto *army)
{
  setStat(Army::STRENGTH, army->getStrength());
  setStat(Army::MOVES, army->getMaxMoves());
  setStat(Army::MOVE_BONUS, army->getMoveBonus());
  setStat(Army::ARMY_BONUS, army->getArmyBonus());
  d_type_id = army->getId();
  d_armyset = army->getArmyset();
}
