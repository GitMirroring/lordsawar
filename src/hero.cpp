// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2007, 2008, 2014, 2017, 2020, 2021 Ben Asselstine
// Copyright (C) 2008 Ole Laursen
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
#include <fstream>
#include <sigc++/functors/mem_fun.h>

#include "hero.h"
#include "stacklist.h"
#include "templelist.h"
#include "heroproto.h"
#include "counter.h"
#include "Backpack.h"
#include "xmlhelper.h"
#include "playerlist.h"
#include "QuestsManager.h"
#include "herotemplates.h"

Glib::ustring Hero::d_hero_tag = "hero";

Hero::Hero(const HeroProto& a)
  : Army (dynamic_cast<const ArmyProto&>(a)), d_name(a.getName()),
    d_gender(Gender(a.getGender())), d_hero_type_id (a.getHeroId())
{
  d_level = 1;
  d_backpack = new Backpack();
  d_owner_id = a.getOwnerId();
}

Hero::Hero(Hero& h, bool sync_id)
  : Army(h, sync_id, h.getOwner ()), d_name(h.d_name), d_gender(h.d_gender),
    d_hero_type_id (h.d_hero_type_id)
{
  d_backpack = new Backpack(*h.d_backpack);
}

Hero::Hero(XML_Helper* helper)
    :Army(helper)
{
  helper->getData(d_name, "name");
  Glib::ustring gender_str;
  if (!helper->getData(gender_str, "gender"))
    d_gender = NONE;
  else
    d_gender = genderFromString(gender_str);
  helper->getData(d_hero_type_id, "hero_type");
  helper->registerTag(Backpack::d_tag, 
		      sigc::mem_fun(*this, &Hero::loadBackpack));
}

Hero::~Hero()
{
  delete d_backpack;
}

bool Hero::save(XML_Helper* helper) const
{
    bool retval = true;
    std::list<Item*>::const_iterator it;

    retval &= helper->openTag(Hero::d_hero_tag);

    retval &= helper->saveData("name", d_name);
    Glib::ustring gender_str = genderToString(Hero::Gender(d_gender));
    retval &= helper->saveData("gender", gender_str);
    retval &= helper->saveData("hero_type", d_hero_type_id);
    retval &= saveData(helper);

    // Now save the backpack
    retval &= d_backpack->save(helper);

    retval &= helper->closeTag();

    return retval;
}

bool Hero::loadBackpack(Glib::ustring tag, XML_Helper* helper)
{
  if (tag == Backpack::d_tag)
    {
      d_backpack = new Backpack(helper);
      return true;
    }
  return false;
}

guint32 Hero::getStat(Stat stat, bool modified) const
{
    guint32 bonus = 0;
    guint32 value = Army::getStat(stat, modified);

    if (!modified)
        return value;

    // Add item bonuses that affect only this hero
    if (stat == STRENGTH)
      bonus += d_backpack->countStrengthBonuses();

    return value + bonus;
}

guint32 Hero::calculateNaturalCommand()
{
  guint32 command = 0;
  guint32 strength = getStat(STRENGTH, true);
  if (strength == 9)
    command += 3;
  else if (strength > 6)
    command += 2;
  else if (strength > 3)
    command += 1;
  return command;
}

Glib::ustring Hero::genderToString(const Hero::Gender gender)
{
  switch (gender)
    {
    case Hero::NONE: return "Hero::NONE";
    case Hero::MALE: return "Hero::MALE";
    case Hero::FEMALE: return "Hero::FEMALE";
    }
  return "Hero::FEMALE";
}

Glib::ustring Hero::genderToFriendlyName (const Hero::Gender gender)
{
  switch (gender)
    {
    case Hero::NONE: return "NONE";
    case Hero::MALE: return _("Male");
    case Hero::FEMALE: return _("Female");
    }
  return _("Female");
}

Hero::Gender Hero::genderFromString(const Glib::ustring str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return Hero::Gender(atoi(str.c_str()));
  if (str == "Hero::MALE") return Hero::MALE;
  else if (str == "Hero::NONE") return Hero::NONE;
  else if (str == "Hero::FEMALE") return Hero::FEMALE;
  return Hero::FEMALE;
}

bool Hero::canGainLevel() const
{
  return getXP() >= getXpNeededForNextLevel();
}

guint32 Hero::getXpNeededForNextLevel() const
{
  return xp_per_level * getLevel();
}

int Hero::computeLevelGain(Stat stat) const
{
  if (stat == MOVE_BONUS || stat == ARMY_BONUS || stat == SHIP)
    return -1;

  switch (stat)
    {
    case STRENGTH:
    case SIGHT:
      return 1;
    case HP:
    case MOVES:
      return 4;
    default:
      return -1;
    }
}

int Hero::gainLevel(Stat stat)
{
  if (!canGainLevel())
    return -1;

  if (stat == MOVE_BONUS || stat == ARMY_BONUS || stat == SHIP ||
      stat == MOVES_MULTIPLIER)
    return -1;

  d_level++;
  d_xp_value *= 1.2;

  int delta = computeLevelGain(stat);
  switch (stat)
    {
    case STRENGTH:
      d_strength += delta;
      if (d_strength > MAX_ARMY_STRENGTH)
	d_strength = MAX_ARMY_STRENGTH;
      break;
    case HP:
      d_max_hp += delta;
      break;
    case MOVES:
      d_max_moves += delta;
      break;
    case SIGHT:
      d_sight += delta;
      break;
    default:
      break;
    }
  return delta;
}

bool Hero::hasQuest() const
{
  return QuestsManager::getInstance()->getHeroQuest(getId()) != NULL;
}

bool Hero::isFlyer() const
{
  bool flying = false;
  if (d_backpack)
    {
      if (d_backpack->countStackFlightGivers() > 0)
        flying = true;
    }
  return flying;
}
        
Glib::ustring Hero::getDescription () const
{
  Character *c =
    HeroTemplates::getInstance ()->getCharacterById (getHeroTypeId ());
  if (c)
    return c->description;
  else
    return "";
}
