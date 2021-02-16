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

HeroTemplates* HeroTemplates::d_instance = 0;

Glib::ustring HeroTemplates::d_tag = "herotemplates";
Glib::ustring HeroTemplates::d_child_tag = "herotemplate";

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

HeroTemplates::HeroTemplates()
{
  XML_Helper helper(File::getMiscFile("heronames.xml"), std::ios::in);

  loadHeroTemplates(&helper);
  if (!helper.parseXML())
    std::cerr << String::ucompose(_("Error!  can't load heronames file `%1'.  Exiting."), File::getMiscFile("heronames.xml")) << std::endl;
  helper.close();
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
}

HeroTemplates::HeroTemplates(XML_Helper *helper)
{
  loadHeroTemplates(helper);
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

void HeroTemplates::loadHeroTemplates(XML_Helper *helper)
{
  loadHeroesFromArmysets ();

  helper->registerTag(HeroTemplates::d_child_tag,
                      sigc::mem_fun((*this), &HeroTemplates::load));

  return;
}

bool HeroTemplates::load(Glib::ustring tag, XML_Helper *helper)
{
  if (tag == "herotemplate")
    {
      guint32 id;
      helper->getData (id, "hero_id");
      Glib::ustring name;
      helper->getData(name, "name");
      guint32 owner;
      helper->getData(owner, "owner");
      Glib::ustring gender_str;
      if (owner >= (int) MAX_PLAYERS)
	return false;
      helper->getData(gender_str, "gender");
      Hero::Gender gender;
      gender = Hero::genderFromString(gender_str);

      const ArmyProto *herotype = NULL;
      if (gender == Hero::MALE)
	{
	  if (d_male_heroes.size() > 0)
	    herotype = d_male_heroes[Rnd::rand() % d_male_heroes.size()];
	}
      else if (gender == Hero::FEMALE)
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
      if (herotype == NULL)
	return false;
      HeroProto *newhero = new HeroProto (*herotype);
      newhero->setOwnerId(owner);
      newhero->setHeroId (id);

      newhero->setName (_(name.c_str()));
      d_herotemplates[owner].push_back (newhero);
    }
  return true;
}

std::vector<HeroProto*> HeroTemplates::getHeroes (int player_id)
{
  std::vector<HeroProto*> out;
  for (auto h : d_herotemplates[player_id])
    out.push_back (new HeroProto (*h));
  return out;
}

void HeroTemplates::replaceHeroes (int player_id, std::vector<HeroProto*> he)
{
  for (guint32 i = 0; i < d_herotemplates[player_id].size (); i++)
    delete d_herotemplates[player_id][i];
  d_herotemplates[player_id].clear ();
  for (guint32 i = 0; i < he.size (); i++)
    d_herotemplates[player_id].push_back (he[i]);
  updateHeroIds ();
}

bool HeroTemplates::isDefault() const
{
  bool same = true;
  HeroTemplates *def = new HeroTemplates ();

  for (guint32 i = 0; i < MAX_PLAYERS; i++)
    {
      std::vector<HeroProto*> h1 = d_herotemplates[i];
      std::vector<HeroProto*> h2 = def->d_herotemplates[i];
      if (h1.size () != h2.size ())
        {
          same = false;
          break;
        }
      for (guint32 j = 0; j < h1.size (); j++)
        {
          if (h1[j]->getName () != h2[j]->getName ())
            {
              same = false;
              break;
            }
          if (h1[j]->getOwnerId () != h2[j]->getOwnerId ())
            {
              same = false;
              break;
            }
          if (h1[j]->getGender () != h2[j]->getGender ())
            {
              same = false;
              break;
            }
        }
    }
  delete def;
  return same;
}

bool HeroTemplates::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(HeroTemplates::d_tag);

    for (guint32 i = 0; i < MAX_PLAYERS; i++)
      {
        for (guint32 j = 0; j < d_herotemplates[i].size (); j++)
          {
            HeroProto *h = d_herotemplates[i][j];
            retval &= helper->openTag(HeroTemplates::d_child_tag);
            retval &= helper->saveData("name", h->getName ());
            Glib::ustring gender_str =
              Hero::genderToString(Hero::Gender(h->getGender ()));
            retval &= helper->saveData("gender", gender_str);
            retval &= helper->saveData("hero_id", h->getHeroId ());
            OwnerId o = *h;
            retval &= o.save (helper);
            retval &= helper->closeTag();
          }
      }

    retval &= helper->closeTag();

    return retval;
}

void HeroTemplates::reset (HeroTemplates *h)
{
  delete d_instance;
  d_instance = h;
}

void HeroTemplates::updateHeroIds()
{
  guint32 id = 0;
  for (guint32 i = 0; i < MAX_PLAYERS; i++)
    {
      for (guint32 j = 0; j < d_herotemplates[i].size (); j++)
        {
          HeroProto *h = d_herotemplates[i][j];
          h->setHeroId (id);
        }
    }
  return;
}
        
HeroProto *HeroTemplates::getHeroProtoById (int player_id, guint32 hero_id)
{
  //let's just do a slow lookup, there's only about 10 to search through.
  if (player_id >= (int) MAX_PLAYERS)
    return NULL;
  for (guint32 j = 0; j < d_herotemplates[player_id].size (); j++)
    {
      HeroProto *h = d_herotemplates[player_id][j];
      if (h->getHeroId () == hero_id)
        return h;
    }
  return NULL;
}
