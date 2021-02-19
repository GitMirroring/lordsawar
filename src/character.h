//  Copyright (C) 2021 Ben Asselstine
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

#pragma once
#ifndef CHARACTER_H
#define CHARACTER_H

#include <gtkmm.h>
#include <list>
#include "tarhelper.h"
#include "xmlhelper.h"
#include "File.h"
#include "hero.h"
#include "hero-strategy.h"

//! Some essential hero details
/**
 * Characters are the named heroes that can show up in a city.
 * They can have a name, a description, a list of starting items and so on.
 *
 * Later on they get melded with an ArmyProto to make a HeroProto so that Hero
 * objects can have strength, moves and so on, but can also refer backward to
 * their type (this Character) to see the ai strategy, etc.
 *
 * The HeroTemplates singleton holds a list of these Character objects.
 *
 * We have two loaders here because we might be loading characters from the
 * default heronames.xml file, or alternatively we might be loading from a
 * saved-game file or map.  The latter is in a tar file so it has to be
 * unpacked.
 *
 */
class Character
{
public:
    static Glib::ustring d_tag;

    Character (guint32 o, Glib::ustring n, Glib::ustring d, guint32 i,
               Hero::Gender g, std::list<guint32> ids, HeroStrategy *s)
      : owner (o), name (n), description (d), id (i), gender (g),
      item_ids (ids), strategy (s) {}
    ~Character () {if (strategy) delete strategy;}
    static Character *copy (Character *chr)
      {
        return new
          Character (chr->owner, chr->name, chr->description,
                     chr->id, chr->gender, chr->item_ids,
                     chr->strategy ?  HeroStrategy::copy (chr->strategy) : NULL);
      }
    static bool save (XML_Helper *helper, Character *c)
      {
        bool retval = true;
        retval &= helper->openTag(Character::d_tag);
        retval &= helper->saveData("name", c->name);
        Glib::ustring gender_str = Hero::genderToString(c->gender);
        retval &= helper->saveData("gender", gender_str);
        retval &= helper->saveData("hero_id", c->id);
        retval &= helper->saveData("owner", c->owner);
        std::stringstream items;
        for (auto it: c->item_ids)
          items << it << " ";
        retval &= helper->saveData("starting_items", items.str());
        retval &= helper->saveData("description", c->description);
        if (c->strategy)
          retval &= c->strategy->save (helper);
        retval &= helper->closeTag();
        return retval;
      }
    static bool load(Glib::ustring tag, XML_Helper* helper,
                     std::list<Character*> *chrs)
      {
	if (tag == Character::d_tag)
          {
            guint32 owner;
            helper->getData(owner, "owner");
            Glib::ustring name;
            helper->getData(name, "name");
            Glib::ustring desc;
            helper->getData(desc, "description");
            guint32 id;
            helper->getData (id, "hero_id");

            Glib::ustring gender_str;
            helper->getData(gender_str, "gender");
            Hero::Gender gender;
            gender = Hero::genderFromString(gender_str);

            Glib::ustring items;
            std::stringstream sitems;
            helper->getData(items, "starting_items");
            sitems.str(items);

            std::list<guint32> item_ids;
            while (sitems.eof() == false)
              {
                int ival = -1;
                sitems >> ival;
                if (ival != -1)
                  item_ids.push_back ((guint32)ival);
              }
            chrs->push_back (new Character (owner, name, desc, id, gender,
                                            item_ids, NULL));
            return true;
          }
	if (tag == HeroStrategy::d_tag)
          {
            HeroStrategy *s = HeroStrategy::handle_load (helper);
            if (chrs->empty () == false)
              chrs->back ()->strategy = s;
            else
              delete s;
            return true;
          }
	return false;
      }
    guint32 owner;
    Glib::ustring name;
    Glib::ustring description;
    guint32 id;
    Hero::Gender gender;
    std::list<guint32> item_ids;
    HeroStrategy *strategy;
};

class CharacterLoader
{
public:
    CharacterLoader(Glib::ustring filename) {
      XML_Helper helper(filename, std::ios::in);
      helper.registerTag
        (Character::d_tag, 
         sigc::bind (sigc::ptr_fun(&Character::load),
                     &characters));
      helper.registerTag
        (HeroStrategy::d_tag,
         sigc::bind (sigc::ptr_fun (&Character::load),
                     &characters));
      helper.parseXML();
      helper.close();
    }
    ~CharacterLoader ()
      {
        for (auto c : characters)
          delete c;
        characters.clear ();
      }

    std::list<Character*> characters;
};

class ScenarioCharacterLoader
{
public:
    ScenarioCharacterLoader(Glib::ustring filename, bool &broken){
      Tar_Helper t(filename, std::ios::in, broken);
      if (broken)
        return;
      std::list<Glib::ustring> ext;
      ext.push_back(MAP_EXT);
      ext.push_back(SAVE_EXT);
      Glib::ustring tmpfile = t.getFirstFile(ext, broken);
      if (!broken)
        {
          CharacterLoader loader (tmpfile);
          for (auto c : loader.characters)
            characters.push_back (Character::copy (c));
        }
      File::erase(tmpfile);
    }
    ~ScenarioCharacterLoader ()
      {
        for (auto c : characters)
          delete c;
        characters.clear ();
      }
    std::list<Character*> characters;
};

#endif
