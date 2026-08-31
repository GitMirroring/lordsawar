//  Copyright (C) 2007, 2008, 2009, 2014, 2015, 2020, 2021, 2026 Ben Asselstine
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
//  Foundation, Inc., 31 Milk Street #960789, Boston, MA 02196, USA.

#include <sstream>

#include "hero-templates.h"

#include "file.h"
#include "army-set-list.h"
#include "player-list.h"
#include "hero.h"
#include "hero-proto.h"
#include "xml-helper.h"
#include "ucompose.hpp"
#include "rnd.h"
#include "hero-strategy.h"

HeroTemplates* HeroTemplates::d_instance = 0;

Glib::ustring HeroTemplates::d_tag = "herotemplates";

HeroTemplates* HeroTemplates::instance()
{
  if (!d_instance)
    d_instance = new HeroTemplates();

  return d_instance;
}

HeroTemplates* HeroTemplates::instance(XML_Helper *helper)
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
  for (auto j = d_herotemplates.begin (); j != d_herotemplates.end (); ++j)
    delete *j;
  d_herotemplates.clear ();
  //take our characters and fill our hero proto arrays
  //we marry up a character with a random hero from our collection of hero
  //army protos
  for (auto c : d_characters)
    {
      const ArmyProto *herotype = NULL;
      if (c->get_gender () == Hero::MALE)
        {
          if (d_male_heroes.size () > 0)
            herotype = d_male_heroes[Rnd::rand () % d_male_heroes.size ()];
        }
      else if (c->get_gender () == Hero::FEMALE)
        {
          if (d_female_heroes.size () > 0)
            herotype = d_female_heroes[Rnd::rand () % d_female_heroes.size ()];
        }
      if (herotype == NULL)
        {
          if (d_male_heroes.size () > 0)
            herotype = d_male_heroes[Rnd::rand () % d_male_heroes.size ()];
          else if (d_female_heroes.size () > 0)
            herotype = d_female_heroes[Rnd::rand () % d_female_heroes.size ()];
        }
      if (herotype && c->get_shield () != Shield::NEUTRAL)
        {
          HeroProto *newhero = new HeroProto (*herotype);
          newhero->setOwnerId ((guint32) c->get_shield ());
          newhero->setCharacterId (c->get_id ());

          newhero->setName (_(c->get_name ().c_str ()));
          d_herotemplates.push_back (newhero);
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

HeroTemplates::HeroTemplates (const HeroTemplates &h)
{
  for (guint32 j = 0; j < h.d_herotemplates.size (); j++)
    d_herotemplates.push_back (new HeroProto (*h.d_herotemplates[j]));

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

  helper->register_tag
    (HeroStrategy::d_tag,
     sigc::bind (sigc::ptr_fun (&Character::load), &d_characters));
  helper->register_tag
    (Character::d_tag,
     sigc::bind (sigc::ptr_fun (&Character::load), &d_characters));
}

HeroTemplates::~HeroTemplates()
{
  for (auto j = d_herotemplates.begin (); j != d_herotemplates.end (); ++j)
    delete *j;
  d_herotemplates.clear ();

  for (unsigned int i = 0; i < d_male_heroes.size (); i++)
    delete d_male_heroes[i];
  d_male_heroes.clear ();

  for (unsigned int i = 0; i < d_female_heroes.size (); i++)
    delete d_female_heroes[i];
  d_female_heroes.clear ();

  for (auto c : d_characters)
    delete c;
  d_characters.clear ();
}

HeroProto *HeroTemplates::getRandomHero(Shield::Color shield)
{
  std::vector<HeroProto*> heroes;
  for (auto hero : d_herotemplates)
    if (hero->getOwnerId () == (guint32) shield)
      heroes.push_back (hero);
  if (heroes.empty ())
    return NULL;
  int num = Rnd::rand() % heroes.size ();
  return heroes[num];
}

void HeroTemplates::loadHeroesFromArmysets ()
{
  // list all the army types that are heroes.
  std::vector<guint32> armyset_ids;
  for (auto p : *Playerlist::instance ())
    armyset_ids.push_back (p->getArmyset ());

  std::set<guint32> unique_armysets (armyset_ids.begin (), armyset_ids.end ());
  for (auto armyset_id : unique_armysets)
    {
      Armyset *as = Armysetlist::instance ()->get (armyset_id);
      for (Armyset::iterator j = as->begin (); j != as->end (); ++j)
        {
          const ArmyProto *a =
            Armysetlist::instance ()->getArmy (armyset_id, (*j)->getId ());
          if (a->isHero ())
            {
              if (a->getGender () == Hero::FEMALE)
                d_female_heroes.push_back (new ArmyProto (*a));
              else
                d_male_heroes.push_back (new ArmyProto (*a));
            }
        }
    }

  if (d_female_heroes.size () == 0 && d_male_heroes.size () > 0)
    {
      //add a female hero if there isn't one in the armyset.
      ArmyProto *female_hero = new ArmyProto (*(*d_male_heroes.begin ()));
      female_hero->setGender (Hero::FEMALE);
      d_female_heroes.push_back (female_hero);
    }
  if (d_male_heroes.size () == 0 && d_female_heroes.size () > 0)
    {
      //add a male hero if there isn't one in the armyset.
      ArmyProto *male_hero = new ArmyProto (*(*d_female_heroes.begin ()));
      male_hero->setGender (Hero::MALE);
      d_male_heroes.push_back (male_hero);
    }
}

std::vector<Character*> HeroTemplates::getHeroes (Shield::Color shield)
{
  std::vector<Character*> out;
  for (auto c : d_characters)
    if (c->get_shield () == shield)
      out.push_back (Character::copy (c));
  return out;
}

void HeroTemplates::replaceHeroes (Shield::Color shield, std::vector<Character*> he)
{
  std::list<Character*> to_delete;
  for (auto c : d_characters)
    {
      if (c->get_shield () == shield)
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
      if (l->get_name () != r->get_name ())
        {
          same = false;
          break;
        }
      if (l->get_description () != r->get_description ())
        {
          same = false;
          break;
        }
      if (l->get_starting_item_ids () != r->get_starting_item_ids ())
        {
          same = false;
          break;
        }
      if (l->get_shield () != r->get_shield ())
        {
          same = false;
          break;
        }
      if (l->get_gender () != r->get_gender ())
        {
          same = false;
          break;
        }
      if (HeroStrategy::compare (l->get_strategy (),
                                 r->get_strategy ()) == false)
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

    retval &= helper->open_tag(HeroTemplates::d_tag);

    for (auto c : d_characters)
      retval &= Character::save (helper, c);

    retval &= helper->close_tag();

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
          for (auto k : c->get_starting_item_ids ())
            if (k >= idx)
              return true;
    }
  return false;
}

Character *HeroTemplates::getCharacterById (guint32 hero_id)
{
  for (auto c : d_characters)
    if (c->get_id () == hero_id)
      return c;
  return NULL;
}

guint32 HeroTemplates::getNextAvailableId () const
{
  std::list<guint32> ids;
  for (auto c : d_characters)
    ids.push_back (c->get_id ());
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
