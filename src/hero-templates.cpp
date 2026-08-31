//  Copyright (C) 2007, 2008, 2009, 2014, 2015, 2020, 2021 Ben Asselstine
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

#include "herotemplates.h"

#include "File.h"
#include "armysetlist.h"
#include "playerlist.h"
#include "hero.h"
#include "heroproto.h"
#include "xmlhelper.h"
#include "ucompose.hpp"
#include "rnd.h"
#include "hero-strategy.h"

HeroTemplates* HeroTemplates::d_instance = 0;

Glib::ustring HeroTemplates::d_tag = "herotemplates";

HeroTemplates* HeroTemplates::getInstance()
{
  if (!d_instance)
    d_instance = new HeroTemplates();

  return d_instance;
}

HeroTemplates* HeroTemplates::getInstance(XML_Helper *helper)
{
  if (d_instance)
    deleteInstance();

  d_instance = new HeroTemplates (helper);
  return d_instance;
}

void HeroTemplates::deleteInstance()
{
  if (d_instance != 0)
    delete d_instance;

  d_instance = 0;
}

void HeroTemplates::populateHeroProtos ()
{
  for (unsigned int i = 0; i < MAX_PLAYERS; ++i)
    {
      for (std::vector<HeroProto *>::iterator j = d_herotemplates[i].begin();
           j != d_herotemplates[i].end(); ++j)
        delete *j;
      d_herotemplates[i].clear();
    }
  //take our characters and fill our hero proto arrays
  for (auto c : d_characters)
    {
      const ArmyProto *herotype = NULL;
      if (c->gender == Hero::MALE)
        {
          if (d_male_heroes.size() > 0)
            herotype = d_male_heroes[Rnd::rand() % d_male_heroes.size()];
        }
      else if (c->gender == Hero::FEMALE)
        {
          if (d_female_heroes.size() > 0)
            herotype = d_female_heroes[Rnd::rand() % d_female_heroes.size()];
        }
      if (herotype == NULL)
        {
          if (d_male_heroes.size() > 0)
            herotype = d_male_heroes[Rnd::rand() % d_male_heroes.size()];
          else if (d_female_heroes.size() > 0)
            herotype = d_female_heroes[Rnd::rand() % d_female_heroes.size()];
        }
      if (herotype &&
          c->owner != Playerlist::getInstance ()->getNeutral ()->getId ())
        {
          HeroProto *newhero = new HeroProto (*herotype);
          newhero->setOwnerId(c->owner);
          newhero->setHeroId (c->id);

          newhero->setName (_(c->name.c_str()));
          newhero->setDescription (_(c->description.c_str()));
          d_herotemplates[c->owner].push_back (newhero);
        }
    }
}

HeroTemplates::HeroTemplates()
{
  loadHeroesFromArmysets ();
  CharacterLoader loader (File::getMiscFile("heronames.xml"));
  for (auto c : loader.characters)
    d_characters.push_back (Character::copy (c));
  populateHeroProtos ();
}

HeroTemplates::HeroTemplates(const HeroTemplates &h)
{
  for (guint32 i = 0; i < MAX_PLAYERS; i++)
    {
      d_herotemplates[i] = std::vector<HeroProto*>();
      for (guint32 j = 0; j < h.d_herotemplates[i].size (); j++)
        d_herotemplates[i].push_back
          (new HeroProto(*h.d_herotemplates[i][j]));
    }

  for (guint32 i = 0; i < h.d_male_heroes.size (); i++)
    d_male_heroes.push_back (new ArmyProto (*h.d_male_heroes[i]));

  for (guint32 i = 0; i < h.d_female_heroes.size (); i++)
    d_female_heroes.push_back (new ArmyProto (*h.d_female_heroes[i]));
  for (auto c: h.d_characters)
    d_characters.push_back (Character::copy (c));
}

HeroTemplates::HeroTemplates(XML_Helper *helper)
{
  loadHeroesFromArmysets ();

  helper->registerTag
    (HeroStrategy::d_tag,
     sigc::bind (sigc::ptr_fun (&Character::load), &d_characters));
  helper->registerTag
    (Character::d_tag,
     sigc::bind (sigc::ptr_fun (&Character::load), &d_characters));
}

HeroTemplates::~HeroTemplates()
{
  for (unsigned int i = 0; i < MAX_PLAYERS; ++i)
    {
      for (std::vector<HeroProto *>::iterator j = d_herotemplates[i].begin();
           j != d_herotemplates[i].end(); ++j)
        delete *j;
      d_herotemplates[i].clear();
    }
  for (unsigned int i = 0; i < d_male_heroes.size(); i++)
    delete d_male_heroes[i];
  d_male_heroes.clear();
  for (unsigned int i = 0; i < d_female_heroes.size(); i++)
    delete d_female_heroes[i];
  d_female_heroes.clear();
  for (auto c : d_characters)
    delete c;
  d_characters.clear ();
}

HeroProto *HeroTemplates::getRandomHero(Hero::Gender gender, int player_id)
{
  std::vector<HeroProto*> heroes;
  for (unsigned int i = 0; i < d_herotemplates[player_id].size(); i++)
    {
      if (Hero::Gender(d_herotemplates[player_id][i]->getGender()) == gender)
	heroes.push_back (d_herotemplates[player_id][i]);
    }
  if (heroes.size() == 0)
    return getRandomHero(player_id);

  int num = Rnd::rand() % heroes.size();
  return heroes[num];
}

HeroProto *HeroTemplates::getRandomHero(int player_id)
{
  int num = Rnd::rand() % d_herotemplates[player_id].size();
  return d_herotemplates[player_id][num];
}

void HeroTemplates::loadHeroesFromArmysets ()
{
  d_male_heroes.clear();
  d_female_heroes.clear();

  // list all the army types that are heroes.
  Player *p = Playerlist::getInstance()->getNeutral();
  Armyset *as = Armysetlist::getInstance()->get(p->getArmyset());
  for (Armyset::iterator j = as->begin(); j != as->end(); ++j)
    {
      const ArmyProto *a =
        Armysetlist::getInstance()->getArmy (p->getArmyset(), (*j)->getId());
      if (a->isHero())
	{
	  if (a->getGender() == Hero::FEMALE)
	    d_female_heroes.push_back(new ArmyProto(*a));
	  else
	    d_male_heroes.push_back(new ArmyProto(*a));
	}
    }
  if (d_female_heroes.size() == 0 && d_male_heroes.size() > 0)
    {
      //add a female hero if there isn't one in the armyset.
      ArmyProto *female_hero = new ArmyProto(*(*d_male_heroes.begin()));
      female_hero->setGender(Hero::FEMALE);
      d_female_heroes.push_back(female_hero);
    }
}

std::vector<Character*> HeroTemplates::getHeroes (guint32 player_id)
{
  std::vector<Character*> out;
  for (auto c : d_characters)
    if (c->owner == player_id)
      out.push_back (Character::copy (c));
  return out;
}

void HeroTemplates::replaceHeroes (guint32 player_id, std::vector<Character*> he)
{
  std::list<Character*> to_delete;
  for (auto c : d_characters)
    {
      if (c->owner == player_id)
        to_delete.push_back (c);
    }

  for (auto c : to_delete)
    {
      d_characters.remove (c);
      delete c;
    }

  for (auto c : he)
    d_characters.push_back (c);
  populateHeroProtos ();
}

bool HeroTemplates::isDefault() const
{
  bool same = true;
  HeroTemplates *def = new HeroTemplates ();

  if (def->d_characters.size () != d_characters.size ())
    {
      delete def;
      return false;
    }
  auto i = def->d_characters.begin ();
  auto j = d_characters.begin ();

  for (; i != def->d_characters.end (); ++i, ++j)
    {
      Character *l = *i;
      Character *r = *j;
      if (l->name != r->name)
        {
          same = false;
          break;
        }
      if (l->description != r->description)
        {
          same = false;
          break;
        }
      if (l->item_ids != r->item_ids)
        {
          same = false;
          break;
        }
      if (l->owner != r->owner)
        {
          same = false;
          break;
        }
      if (l->gender != r->gender)
        {
          same = false;
          break;
        }
      if (HeroStrategy::compare (l->strategy, r->strategy) == false)
        {
          same = false;
          break;
        }
    }

  delete def;
  return same;
}

bool HeroTemplates::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(HeroTemplates::d_tag);

    for (auto c : d_characters)
      retval &= Character::save (helper, c);

    retval &= helper->closeTag();

    return retval;
}

void HeroTemplates::reset (HeroTemplates *h)
{
  delete d_instance;
  d_instance = h;
}

bool HeroTemplates::removeItemAffectsStartingItemIds (guint32 idx)
{
  for (auto c : d_characters)
    {
          for (auto k : c->item_ids)
            if (k >= idx)
              return true;
    }
  return false;
}

Character *HeroTemplates::getCharacterById (guint32 hero_id)
{
  for (auto c : d_characters)
    if (c->id == hero_id)
      return c;
  return NULL;
}

guint32 HeroTemplates::getNextAvailableId () const
{
  std::list<guint32> ids;
  for (auto c : d_characters)
    ids.push_back (c->id);
  ids.sort ();

  guint32 new_id = 0;
  for (auto id : ids)
    {
      if (id != new_id)
        return new_id;
      new_id++;
    }
  return ids.size ();
}
