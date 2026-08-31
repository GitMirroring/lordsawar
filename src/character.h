//  Copyright (C) 2021, 2026 Ben Asselstine
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

#pragma once
#ifndef CHARACTER_H
#define CHARACTER_H

#include <gtkmm.h>
#include <list>
#include "tar-helper.h"
#include "xml-helper.h"
#include "file.h"
#include "hero.h"
#include "hero-strategy.h"
#include "shield.h"
#include "ucompose.hpp"

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

    Character (Shield::Color o, Glib::ustring n, Glib::ustring d, guint32 i,
               Hero::Gender g, std::list<guint32> ids, HeroStrategy *s)
      : m_id (i), m_name (n), m_description (d), m_gender (g), m_shield (o),
      m_item_ids (ids), m_strategy (s)
  {
  }

    ~Character ()
      {
        if (m_strategy)
          delete m_strategy;
      }

    static Character *copy (Character *chr)
      {
        return new
          Character (chr->m_shield, chr->m_name, chr->m_description,
                     chr->m_id, chr->m_gender, chr->m_item_ids,
                     chr->m_strategy ? 
                     HeroStrategy::copy (chr->m_strategy) : NULL);
      }

    static bool save (XML_Helper *helper, Character *c)
      {
        bool retval = true;
        retval &= helper->open_tag (Character::d_tag);
        retval &= helper->save ("name", c->m_name);
        Glib::ustring gender_str = Hero::genderToString (c->m_gender);
        retval &= helper->save ("gender", gender_str);
        retval &= helper->save ("hero_id", c->m_id);
        retval &= helper->save ("owner", (guint32) c->m_shield);
        std::stringstream items;
        for (auto it: c->m_item_ids)
          items << String::ucompose ("%1", it) << " ";
        retval &= helper->save ("starting_items", items.str ());
        retval &= helper->save ("description", c->m_description);
        if (c->m_strategy)
          retval &= c->m_strategy->save (helper);
        retval &= helper->close_tag ();
        return retval;
      }

    static bool load (Glib::ustring tag, XML_Helper* helper,
                      std::list<Character*> *chrs)
      {
        if (tag == Character::d_tag)
          {
            guint32 owner;
            helper->get (owner, "owner");
            Glib::ustring name;
            helper->get (name, "name");
            Glib::ustring desc;
            helper->get (desc, "description");
            guint32 id;
            helper->get (id, "hero_id");

            Glib::ustring gender_str;
            helper->get (gender_str, "gender");
            Hero::Gender gender;
            gender = Hero::genderFromString (gender_str);

            Glib::ustring items;
            std::stringstream sitems;
            helper->get (items, "starting_items");
            sitems.str (items);

            std::list<guint32> item_ids;
            while (sitems.eof () == false)
              {
                int ival = -1;
                sitems >> ival;
                if (ival != -1)
                  item_ids.push_back ((guint32)ival);
              }
            chrs->push_back (new Character (Shield::Color (owner), name, desc,
                                            id, gender, item_ids, NULL));
            return true;
          }
        if (tag == HeroStrategy::d_tag)
          {
            HeroStrategy *s = HeroStrategy::handle_load (helper);
            if (chrs->empty () == false)
              chrs->back ()->m_strategy = s;
            else
              delete s;
            return true;
          }
        return false;
      }

    Glib::ustring get_name () const
      {
        return m_name;
      }

    Glib::ustring get_description () const
      {
        return m_description;
      }

    guint32 get_id () const
      {
        return m_id;
      }

    Hero::Gender get_gender () const
      {
        return m_gender;
      }

    Shield::Color get_shield () const
      {
        return m_shield;
      }

    std::list<guint32> get_starting_item_ids () const
      {
        return m_item_ids;
      }

    HeroStrategy* get_strategy () const
      {
        return m_strategy;
      }

    void set_name (Glib::ustring name)
      {
        m_name = name;
      }

    void set_description (Glib::ustring d)
      {
        m_description = d;
      }

    void set_gender (Hero::Gender g)
      {
        m_gender = g;
      }

    void set_shield (Shield::Color c)
      {
        m_shield = c;
      }

    void set_starting_item_ids (std::list<guint32> item_ids)
      {
        m_item_ids = item_ids;
      }

    void set_strategy (HeroStrategy *s)
      {
        if (m_strategy)
          delete m_strategy;
        m_strategy = s;
      }

private:
    guint32 m_id;
    Glib::ustring m_name;
    Glib::ustring m_description;
    Hero::Gender m_gender;
    Shield::Color m_shield;
    std::list<guint32> m_item_ids;
    HeroStrategy *m_strategy;
};

class CharacterLoader
{
public:
    CharacterLoader (std::string filename)
      {
        XML_Helper helper (filename, std::ios::in);
        helper.register_tag
          (Character::d_tag,
           sigc::bind (sigc::ptr_fun (&Character::load),
                       &characters));
        helper.register_tag
          (HeroStrategy::d_tag,
           sigc::bind (sigc::ptr_fun (&Character::load),
                       &characters));
        helper.parse_XML();
        helper.close ();
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
    ScenarioCharacterLoader (std::string filename, bool &broken)
      {
        Tar_Helper t (filename, std::ios::in, broken);
        if (broken)
          return;
        std::list<std::string> ext;
        ext.push_back (MAP_EXT);
        ext.push_back (SAVE_EXT);
        std::string tmpfile = t.getFirstFile (ext, broken);
        if (!broken)
          {
            CharacterLoader loader (tmpfile);
            for (auto c : loader.characters)
              characters.push_back (Character::copy (c));
          }
        File::erase (tmpfile);
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
