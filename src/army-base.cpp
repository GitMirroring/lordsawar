// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2007, 2008, 2014, 2015, 2020, 2021 Ben Asselstine
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
#include "ucompose.hpp"
#include "armybase.h"
#include "xmlhelper.h"
#include "Tile.h"
#include "defs.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

ArmyBase::ArmyBase(const ArmyBase& a)
: d_upkeep(a.d_upkeep), d_strength(a.d_strength), d_max_moves(a.d_max_moves), 
    d_sight(a.d_sight), d_move_bonus(a.d_move_bonus),
    d_army_bonus(a.d_army_bonus), d_xp_value(a.d_xp_value)
{
}

ArmyBase::ArmyBase()
  : d_upkeep(0), 
    d_strength(0), d_max_moves(0), d_sight(0), 
    d_move_bonus(0), d_army_bonus(0), d_xp_value(0.0)
{
}

ArmyBase::ArmyBase(XML_Helper* helper)
{
  helper->getData(d_upkeep, "upkeep");
  Glib::ustring move_bonus_str;
  helper->getData(move_bonus_str, "move_bonus");
  d_move_bonus = moveFlagsFromString(move_bonus_str);
  Glib::ustring army_bonus_str;
  helper->getData(army_bonus_str, "army_bonus");
  d_army_bonus = bonusFlagsFromString(army_bonus_str);
  helper->getData(d_max_moves, "max_moves");
  helper->getData(d_strength, "strength");
  helper->getData(d_sight, "sight");
  helper->getData(d_xp_value, "expvalue");
}

bool ArmyBase::saveData(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->saveData("upkeep", d_upkeep);
  Glib::ustring move_bonus_str = moveFlagsToString(d_move_bonus);
  retval &= helper->saveData("move_bonus", move_bonus_str);
  Glib::ustring army_bonus_str = bonusFlagsToString(d_army_bonus);
  retval &= helper->saveData("army_bonus", army_bonus_str);
  retval &= helper->saveData("max_moves", d_max_moves);
  retval &= helper->saveData("strength", d_strength);
  retval &= helper->saveData("sight", d_sight);
  retval &= helper->saveData("expvalue", d_xp_value);
  return retval;
}

Glib::ustring ArmyBase::getArmyBonusDescription() const
{
  guint32 bonus = d_army_bonus;
  Glib::ustring s = "";
  if (bonus & ArmyBase::ADD1STRINOPEN && bonus & ArmyBase::ADD2STRINOPEN)
    s += String::ucompose("%1%2", s == "" ? " " : "& ",
			  _("+3 Str In Open"));
  else if (bonus & ArmyBase::ADD1STRINOPEN)
    s += String::ucompose("%1%2", s == "" ? " " : "& ",
			  bonusFlagToFriendlyName(ArmyBase::ADD1STRINOPEN));
  else if (bonus & ArmyBase::ADD2STRINOPEN)
    s += String::ucompose("%1%2", s == "" ? " " : "& ",
			  bonusFlagToFriendlyName(ArmyBase::ADD2STRINOPEN));
  if (bonus & ArmyBase::ADD1STRINFOREST && bonus & ArmyBase::ADD2STRINFOREST)
    s += String::ucompose("%1%2", s == "" ? " " : "& ",
			  _("+3 Str In Forest"));
  else if (bonus & ArmyBase::ADD1STRINFOREST)
    s += String::ucompose("%1%2", s == "" ? " " : "& ",
			  bonusFlagToFriendlyName(ArmyBase::ADD1STRINFOREST));
  else if (bonus & ArmyBase::ADD2STRINFOREST)
    s += String::ucompose("%1%2", s == "" ? " " : "& ",
			  bonusFlagToFriendlyName(ArmyBase::ADD2STRINFOREST));
  if (bonus & ArmyBase::ADD1STRINHILLS && bonus & ArmyBase::ADD2STRINHILLS)
    s += String::ucompose("%1%2", s == "" ? " " : " & ",
			  _("+3 Str In Hills"));
  else if (bonus & ArmyBase::ADD1STRINHILLS)
    s += String::ucompose("%1%2", s == "" ? " " : " & ",
			  bonusFlagToFriendlyName(ArmyBase::ADD1STRINHILLS));
  else if (bonus & ArmyBase::ADD2STRINHILLS)
    s += String::ucompose("%1%2", s == "" ? " " : " & ",
			  bonusFlagToFriendlyName(ArmyBase::ADD2STRINHILLS));

  if (bonus & ArmyBase::ADD1STRINCITY && bonus & ArmyBase::ADD2STRINCITY)
    s += String::ucompose("%1%2", s == "" ? " " : " & ",
			  _("+3 Str In City"));
  else if (bonus & ArmyBase::ADD1STRINCITY)
    s += String::ucompose("%1%2", s == "" ? " " : " & ",
			  bonusFlagToFriendlyName(ArmyBase::ADD1STRINCITY));
  else if (bonus & ArmyBase::ADD2STRINCITY)
    s += String::ucompose("%1%2", s == "" ? " " : " & ",
			  bonusFlagToFriendlyName(ArmyBase::ADD2STRINCITY));
  if (bonus & ArmyBase::ADD1STACKINHILLS)
    s += String::ucompose("%1%2", s == "" ? " " : " & ",
			  bonusFlagToFriendlyName(ArmyBase::ADD1STACKINHILLS));
  if (bonus & ArmyBase::SUBALLCITYBONUS)
    s += String::ucompose("%1%2", s == "" ? " " : " & ",
			  bonusFlagToFriendlyName(ArmyBase::SUBALLCITYBONUS));
  if (bonus & ArmyBase::SUB1ENEMYSTACK && bonus & ArmyBase::SUB2ENEMYSTACK)
    s += String::ucompose("%1%2", s == "" ? " " : " & ",
			  _("-3 Enemy Stack"));
  else if (bonus & ArmyBase::SUB1ENEMYSTACK)
    s += String::ucompose("%1%2", s == "" ? " " : " & ",
			  bonusFlagToFriendlyName(ArmyBase::SUB1ENEMYSTACK));
  else if (bonus & ArmyBase::SUB2ENEMYSTACK)
    s += String::ucompose("%1%2", s == "" ? " " : " & ",
			  bonusFlagToFriendlyName(ArmyBase::SUB2ENEMYSTACK));

  if (bonus & ArmyBase::ADD1STACK && bonus & ArmyBase::ADD2STACK)
    s += String::ucompose("%1%2", s == "" ? " " : " & ", _("+3 Stack"));
  else if (bonus & ArmyBase::ADD1STACK)
    s += String::ucompose("%1%2", s == "" ? " " : " & ", 
			  bonusFlagToFriendlyName(ArmyBase::ADD1STACK));
  else if (bonus & ArmyBase::ADD2STACK)
    s += String::ucompose("%1%2", s == "" ? " " : " & ", 
			  bonusFlagToFriendlyName(ArmyBase::ADD2STACK));
  if (bonus & ArmyBase::SUBALLNONHEROBONUS)
    s += String::ucompose("%1%2", s == "" ? " " : " & ",
			  bonusFlagToFriendlyName(ArmyBase::SUBALLNONHEROBONUS));
  if (bonus & ArmyBase::SUBALLHEROBONUS)
    s += String::ucompose("%1%2", s == "" ? " " : " & ",
			  bonusFlagToFriendlyName(ArmyBase::SUBALLHEROBONUS));
  if (bonus & ArmyBase::CONFER_MOVE_BONUS)
    s += String::ucompose("%1%2", s == "" ? " " : " & ",
			  bonusFlagToFriendlyName(ArmyBase::CONFER_MOVE_BONUS));
  return s;
}

Glib::ustring ArmyBase::moveFlagsToString(const guint32 bonus)
{
  Glib::ustring move_bonuses;
  //we don't add grass, because it's always implied.
  if (bonus & Tile::WATER)
    move_bonuses += " " + Tile::tileTypeToString(Tile::WATER);
  if (bonus & Tile::FOREST)
    move_bonuses += " " + Tile::tileTypeToString(Tile::FOREST);
  if (bonus & Tile::HILLS)
    move_bonuses += " " + Tile::tileTypeToString(Tile::HILLS);
  if (bonus & Tile::MOUNTAIN)
    move_bonuses += " " + Tile::tileTypeToString(Tile::MOUNTAIN);
  if (bonus & Tile::SWAMP)
    move_bonuses += " " + Tile::tileTypeToString(Tile::SWAMP);
  return move_bonuses;
}

guint32 ArmyBase::moveFlagsFromString(const Glib::ustring str)
{
  return XML_Helper::flagsFromString(str, Tile::tileTypeFromString);
}

Glib::ustring ArmyBase::bonusFlagToString(const ArmyBase::Bonus bonus)
{
  switch (bonus)
    {
    case ArmyBase::ADD1STRINOPEN: return "ArmyBase::ADD1STRINOPEN";
    case ArmyBase::ADD2STRINOPEN: return "ArmyBase::ADD2STRINOPEN";
    case ArmyBase::ADD1STRINFOREST: return "ArmyBase::ADD1STRINFOREST";
    case ArmyBase::ADD1STRINHILLS: return "ArmyBase::ADD1STRINHILLS";
    case ArmyBase::ADD1STRINCITY: return "ArmyBase::ADD1STRINCITY";
    case ArmyBase::ADD2STRINCITY: return "ArmyBase::ADD2STRINCITY";
    case ArmyBase::ADD1STACKINHILLS: return "ArmyBase::ADD1STACKINHILLS";
    case ArmyBase::SUBALLCITYBONUS: return "ArmyBase::SUBALLCITYBONUS";
    case ArmyBase::SUB1ENEMYSTACK: return "ArmyBase::SUB1ENEMYSTACK";
    case ArmyBase::ADD1STACK: return "ArmyBase::ADD1STACK";
    case ArmyBase::ADD2STACK: return "ArmyBase::ADD2STACK";
    case ArmyBase::SUBALLNONHEROBONUS: return "ArmyBase::SUBALLNONHEROBONUS";
    case ArmyBase::SUBALLHEROBONUS: return "ArmyBase::SUBALLHEROBONUS";
    case ArmyBase::FORTIFY: return "ArmyBase::FORTIFY";
    case ArmyBase::ADD2STRINFOREST: return "ArmyBase::ADD2STRINFOREST";
    case ArmyBase::ADD2STRINHILLS: return "ArmyBase::ADD2STRINHILLS";
    case ArmyBase::SUB2ENEMYSTACK: return "ArmyBase::SUB2ENEMYSTACK";
    case ArmyBase::CONFER_MOVE_BONUS: return "ArmyBase::CONFER_MOVE_BONUS";
    }
  return "";
}

Glib::ustring ArmyBase::bonusFlagToFriendlyName(const ArmyBase::Bonus bonus)
{
  switch (bonus)
    {
    case ArmyBase::ADD1STRINOPEN: return _("+1 Str In Open");
    case ArmyBase::ADD2STRINOPEN: return _("+2 Str In Open");
    case ArmyBase::ADD1STRINFOREST: return _("+1 Str In Forest");
    case ArmyBase::ADD1STRINHILLS: return _("+1 Str In Hills");
    case ArmyBase::ADD1STRINCITY: return _("+1 Str In City");
    case ArmyBase::ADD2STRINCITY: return _("+2 Str In City");
    case ArmyBase::ADD1STACKINHILLS: return _("+1 Stack In Hills");
    case ArmyBase::SUBALLCITYBONUS: return _("Cancel City Bonus");
    case ArmyBase::SUB1ENEMYSTACK: return _("-1 Enemy Stack)");
    case ArmyBase::ADD1STACK: return _("+1 Stack");
    case ArmyBase::ADD2STACK: return _("+2 Stack");
    case ArmyBase::SUBALLNONHEROBONUS: return _("Cancel Non-Hero");
    case ArmyBase::SUBALLHEROBONUS: return _("Cancel Hero");
    case ArmyBase::FORTIFY: return _("Fortify");
    case ArmyBase::ADD2STRINFOREST: return _("+2 Str In Forest");
    case ArmyBase::ADD2STRINHILLS: return _("+2 Str In Hills");
    case ArmyBase::SUB2ENEMYSTACK: return _("-2 Enemy Stack");
    case ArmyBase::CONFER_MOVE_BONUS: return _("Confers Move Bonus To Stack");
    }
  return "";
}

Glib::ustring ArmyBase::bonusFlagsToString(const guint32 bonus)
{
  Glib::ustring bonuses;
  if (bonus & ArmyBase::ADD1STRINOPEN)
    bonuses += " " + bonusFlagToString(ArmyBase::ADD1STRINOPEN);
  if (bonus & ArmyBase::ADD2STRINOPEN)
    bonuses += " " + bonusFlagToString(ArmyBase::ADD2STRINOPEN);
  if (bonus & ArmyBase::ADD1STRINFOREST)
    bonuses += " " + bonusFlagToString(ArmyBase::ADD1STRINFOREST);
  if (bonus & ArmyBase::ADD1STRINHILLS)
    bonuses += " " + bonusFlagToString(ArmyBase::ADD1STRINHILLS);
  if (bonus & ArmyBase::ADD1STRINCITY)
    bonuses += " " + bonusFlagToString(ArmyBase::ADD1STRINCITY);
  if (bonus & ArmyBase::ADD2STRINCITY)
    bonuses += " " + bonusFlagToString(ArmyBase::ADD2STRINCITY);
  if (bonus & ArmyBase::ADD1STACKINHILLS)
    bonuses += " " + bonusFlagToString(ArmyBase::ADD1STACKINHILLS);
  if (bonus & ArmyBase::SUBALLCITYBONUS)
    bonuses += " " + bonusFlagToString(ArmyBase::SUBALLCITYBONUS);
  if (bonus & ArmyBase::SUB1ENEMYSTACK)
    bonuses += " " + bonusFlagToString(ArmyBase::SUB1ENEMYSTACK);
  if (bonus & ArmyBase::ADD1STACK)
    bonuses += " " + bonusFlagToString(ArmyBase::ADD1STACK);
  if (bonus & ArmyBase::ADD2STACK)
    bonuses += " " + bonusFlagToString(ArmyBase::ADD2STACK);
  if (bonus & ArmyBase::SUBALLNONHEROBONUS)
    bonuses += " " + bonusFlagToString(ArmyBase::SUBALLNONHEROBONUS);
  if (bonus & ArmyBase::SUBALLHEROBONUS)
    bonuses += " " + bonusFlagToString(ArmyBase::SUBALLHEROBONUS);
  if (bonus & ArmyBase::FORTIFY)
    bonuses += " " + bonusFlagToString(ArmyBase::FORTIFY);
  if (bonus & ArmyBase::ADD2STRINFOREST)
    bonuses += " " + bonusFlagToString(ArmyBase::ADD2STRINFOREST);
  if (bonus & ArmyBase::ADD2STRINHILLS)
    bonuses += " " + bonusFlagToString(ArmyBase::ADD2STRINHILLS);
  if (bonus & ArmyBase::SUB2ENEMYSTACK)
    bonuses += " " + bonusFlagToString(ArmyBase::SUB2ENEMYSTACK);
  if (bonus & ArmyBase::CONFER_MOVE_BONUS)
    bonuses += " " + bonusFlagToString(ArmyBase::CONFER_MOVE_BONUS);
  return bonuses;
}

guint32 ArmyBase::bonusFlagsFromString(const Glib::ustring str)
{
  return XML_Helper::flagsFromString(str, bonusFlagFromString);
}

guint32 ArmyBase::bonusFlagFromString(const Glib::ustring str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return ArmyBase::Bonus(atoi(str.c_str()));
  if (str == "ArmyBase::ADD1STRINOPEN") return ArmyBase::ADD1STRINOPEN;
  else if (str == "ArmyBase::ADD2STRINOPEN") return ArmyBase::ADD2STRINOPEN;
  else if (str == "ArmyBase::ADD1STRINFOREST") return ArmyBase::ADD1STRINFOREST;
  else if (str == "ArmyBase::ADD1STRINHILLS") return ArmyBase::ADD1STRINHILLS;
  else if (str == "ArmyBase::ADD1STRINCITY") return ArmyBase::ADD1STRINCITY;
  else if (str == "ArmyBase::ADD2STRINCITY") return ArmyBase::ADD2STRINCITY;
  else if (str == "ArmyBase::ADD1STACKINHILLS") return ArmyBase::ADD1STACKINHILLS;
  else if (str == "ArmyBase::SUBALLCITYBONUS") return ArmyBase::SUBALLCITYBONUS;
  else if (str == "ArmyBase::SUB1ENEMYSTACK") return ArmyBase::SUB1ENEMYSTACK;
  else if (str == "ArmyBase::ADD1STACK") return ArmyBase::ADD1STACK;
  else if (str == "ArmyBase::ADD2STACK") return ArmyBase::ADD2STACK;
  else if (str == "ArmyBase::SUBALLNONHEROBONUS") return ArmyBase::SUBALLNONHEROBONUS;
  else if (str == "ArmyBase::SUBALLHEROBONUS") return ArmyBase::SUBALLHEROBONUS;
  else if (str == "ArmyBase::FORTIFY") return ArmyBase::FORTIFY;
  else if (str == "ArmyBase::ADD2STRINFOREST") return ArmyBase::ADD2STRINFOREST;
  else if (str == "ArmyBase::ADD2STRINHILLS") return ArmyBase::ADD2STRINHILLS;
  else if (str == "ArmyBase::SUB2ENEMYSTACK") return ArmyBase::SUB2ENEMYSTACK;
  else if (str == "ArmyBase::CONFER_MOVE_BONUS") return ArmyBase::CONFER_MOVE_BONUS;
  return ArmyBase::ADD1STRINOPEN;
}

Glib::ustring ArmyBase::getMoveBonusDescription() const
{
  guint32 bonus = getMoveBonus ();
  if (bonus == Tile::isFlying ())
    return _("Flies");
  else
    {
      if (bonus == Tile::GRASS)
        return "";
      else
        {
          Glib::ustring s = "";
          bool first = true;
          if (bonus & Tile::WATER)
            {
              s += (first ? " " : ", ") +
                Tile::tileTypeToFriendlyName(Tile::WATER);
              first = false;
            }
          if (bonus & Tile::FOREST)
            {
              s += (first ? " " : ", ") +
                Tile::tileTypeToFriendlyName(Tile::FOREST);
              first = false;
            }
          if (bonus & Tile::HILLS)
            {
              s += (first ? " " : ", ") +
                Tile::tileTypeToFriendlyName(Tile::HILLS);
              first = false;
            }
          if (bonus & Tile::MOUNTAIN)
            {
              s += (first ? " " : ", ") +
                Tile::tileTypeToFriendlyName(Tile::MOUNTAIN);
              first = false;
            }
          if (bonus & Tile::SWAMP)
            {
              s += (first ? " " : ", ") +
                Tile::tileTypeToFriendlyName(Tile::SWAMP);
              first = false;
            }
          return s;
        }
    }
  return "";
}
