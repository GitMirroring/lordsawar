//  Copyright (C) 2017, 2020, 2021, 2026 Ben Asselstine
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

#include "scenario-media.h"
#include "defs.h"
#include "image-helpers.h"
#include "xml-helper.h"
#include "file.h"
#include "snd.h"
#include "tar-file.h"
#include "tar-file-masked-image.h"
#include "tar-file-image.h"
#include "tar-file-sound.h"

ScenarioMedia* ScenarioMedia::d_instance = 0;

Glib::ustring ScenarioMedia::d_tag = "media";

ScenarioMedia* ScenarioMedia::instance()
{
  if (!d_instance)
    d_instance = new ScenarioMedia();

  return d_instance;
}

ScenarioMedia* ScenarioMedia::instance(XML_Helper* helper)
{
  if (d_instance)
    deleteInstance();

  d_instance = new ScenarioMedia(helper);
  return d_instance;
}

void ScenarioMedia::deleteInstance()
{
  if (d_instance != 0)
    delete d_instance;

  d_instance = 0;
}

ScenarioMedia::ScenarioMedia()
{
  d_hero_newlevel[0] =
    new TarFileMaskedImage (TarFileMaskedImage::VERTICAL_MASK,
                            PixMask::DIMENSION_ANY);
  d_hero_newlevel[1] =
    new TarFileMaskedImage (TarFileMaskedImage::VERTICAL_MASK,
                            PixMask::DIMENSION_ANY);
  d_next_turn = new TarFileImage (1, PixMask::DIMENSION_ANY);
  d_city_defeated = new TarFileImage (1, PixMask::DIMENSION_ANY);
  d_winning = new TarFileImage (1, PixMask::DIMENSION_ANY);
  d_hero[0] = new TarFileImage (1, PixMask::DIMENSION_ANY);
  d_hero[1] = new TarFileImage (1, PixMask::DIMENSION_ANY);
  d_ruin_success = new TarFileImage (1, PixMask::DIMENSION_ANY);
  d_ruin_defeat = new TarFileImage (1, PixMask::DIMENSION_ANY);
  d_parley_offered = new TarFileImage (1, PixMask::DIMENSION_ANY);
  d_parley_refused = new TarFileImage (1, PixMask::DIMENSION_ANY);
  d_medal[0] =
    new TarFileImage (MEDAL_TYPES,
                      PixMask::DIMENSION_WIDTH_IS_MULTIPLE_OF_HEIGHT);
  d_medal[1] =
    new TarFileImage (MEDAL_TYPES,
                      PixMask::DIMENSION_WIDTH_IS_MULTIPLE_OF_HEIGHT);
  d_commentator = new TarFileImage (1, PixMask::DIMENSION_ANY);
  d_bless_sound = new TarFileSound ();
  d_hero_sound = new TarFileSound ();
  d_battle_sound = new TarFileSound ();
  d_defeat_sound = new TarFileSound ();
  d_victory_sound = new TarFileSound ();
  d_back_sound = new TarFileSound ();
}

ScenarioMedia::ScenarioMedia(const ScenarioMedia &m)
 : d_next_turn (new TarFileImage (*m.d_next_turn)),
    d_city_defeated (new TarFileImage (*m.d_city_defeated)),
    d_winning (new TarFileImage (*m.d_winning)),
    d_ruin_success (new TarFileImage (*m.d_ruin_success)),
    d_ruin_defeat (new TarFileImage (*m.d_ruin_defeat)),
    d_parley_offered (new TarFileImage (*m.d_parley_offered)),
    d_parley_refused (new TarFileImage (*m.d_parley_refused)),
    d_commentator (new TarFileImage (*m.d_commentator)),
    d_bless_sound (new TarFileSound (*m.d_bless_sound)),
    d_hero_sound (new TarFileSound (*m.d_hero_sound)),
    d_battle_sound (new TarFileSound (*m.d_battle_sound)),
    d_defeat_sound (new TarFileSound (*m.d_defeat_sound)),
    d_victory_sound (new TarFileSound (*m.d_victory_sound)),
    d_back_sound (new TarFileSound (*m.d_back_sound)),
    d_musicMap (std::map<Glib::ustring, MusicItem*>()),
    d_bgMap (std::vector<Glib::ustring>())
{
  d_hero_newlevel[0] = new TarFileMaskedImage (*m.d_hero_newlevel[0]);
  d_hero_newlevel[1] = new TarFileMaskedImage (*m.d_hero_newlevel[1]);
  d_hero[0] = new TarFileImage (*m.d_hero[0]);
  d_hero[1] = new TarFileImage (*m.d_hero[1]);
  d_medal[0] = new TarFileImage (*m.d_medal[0]);
  d_medal[1] = new TarFileImage (*m.d_medal[1]);

  for (std::map<Glib::ustring, MusicItem*>::const_iterator
       i = m.d_musicMap.begin (); i != m.d_musicMap.end (); ++i)
    d_musicMap[(*i).first] = new MusicItem (*(*i).second);
  d_bgMap = m.d_bgMap;
}

ScenarioMedia::ScenarioMedia(XML_Helper *helper)
{
  d_hero_newlevel[0] =
    new TarFileMaskedImage (TarFileMaskedImage::VERTICAL_MASK,
                            PixMask::DIMENSION_HEIGHT_IS_MARKED);
  d_hero_newlevel[1] =
    new TarFileMaskedImage (TarFileMaskedImage::VERTICAL_MASK,
                            PixMask::DIMENSION_HEIGHT_IS_MARKED);
  d_next_turn = new TarFileImage (1, PixMask::DIMENSION_ANY);
  d_city_defeated = new TarFileImage (1, PixMask::DIMENSION_ANY);
  d_winning = new TarFileImage (1, PixMask::DIMENSION_ANY);
  d_hero[0] = new TarFileImage (1, PixMask::DIMENSION_ANY);
  d_hero[1] = new TarFileImage (1, PixMask::DIMENSION_ANY);
  d_ruin_success = new TarFileImage (1, PixMask::DIMENSION_ANY);
  d_ruin_defeat = new TarFileImage (1, PixMask::DIMENSION_ANY);
  d_parley_offered = new TarFileImage (1, PixMask::DIMENSION_ANY);
  d_parley_refused = new TarFileImage (1, PixMask::DIMENSION_ANY);
  d_medal[0] =
    new TarFileImage (MEDAL_TYPES,
                      PixMask::DIMENSION_WIDTH_IS_MULTIPLE_OF_HEIGHT);
  d_medal[1] =
    new TarFileImage (MEDAL_TYPES,
                      PixMask::DIMENSION_WIDTH_IS_MULTIPLE_OF_HEIGHT);
  d_commentator = new TarFileImage (1, PixMask::DIMENSION_ANY);
  d_bless_sound = new TarFileSound ();
  d_hero_sound = new TarFileSound ();
  d_battle_sound = new TarFileSound ();
  d_defeat_sound = new TarFileSound ();
  d_victory_sound = new TarFileSound ();
  d_back_sound = new TarFileSound ();
  d_next_turn->load_name (helper, "next_turn_image");
  d_city_defeated->load_name (helper, "city_defeated_image");
  d_winning->load_name (helper, "winning_image");
  d_hero[0]->load_name (helper, "male_hero_image");
  d_hero[1]->load_name (helper, "female_hero_image");
  d_ruin_success->load_name (helper, "ruin_success_image");
  d_ruin_defeat->load_name (helper, "ruin_defeat_image");
  d_parley_offered->load_name (helper, "parley_offered_image");
  d_parley_refused->load_name (helper, "parley_refused_image");
  d_hero_newlevel[0]->load (helper, "hero_newlevel_male_image");
  d_hero_newlevel[1]->load (helper, "hero_newlevel_female_image");
  d_medal[0]->load_name (helper, "small_medals_image");
  d_medal[1]->load_name (helper, "big_medals_image");
  d_commentator->load_name (helper, "commentator_image");

  d_bless_sound->load_name (helper, "bless_sound");
  d_hero_sound->load_name (helper, "hero_sound");
  d_battle_sound->load_name (helper, "battle_sound");
  d_defeat_sound->load_name (helper, "defeat_sound");
  d_victory_sound->load_name (helper, "victory_sound");
  d_back_sound->load_name (helper, "back_sound");
}

bool ScenarioMedia::anyValueSet() const
{
  if (d_next_turn->getName () != "" ||
      d_city_defeated->getName () != "" ||
      d_winning->getName () != "" ||
      d_hero[0]->getName () != "" ||
      d_hero[1]->getName () != "" ||
      d_ruin_success->getName () != "" ||
      d_ruin_defeat->getName () != "" ||
      d_parley_offered->getName () != "" ||
      d_parley_refused->getName () != "" ||
      d_hero_newlevel[0]->getName () != "" ||
      d_hero_newlevel[1]->getName () != "" ||
      d_medal[0]->getName () != "" ||
      d_medal[1]->getName () != "" ||
      d_commentator->getName () != "" ||
      d_bless_sound->get_name () != "" ||
      d_hero_sound->get_name () != "" ||
      d_battle_sound->get_name () != "" ||
      d_defeat_sound->get_name () != "" ||
      d_victory_sound->get_name () != "" ||
      d_back_sound->get_name () != "")
    return true;
  return false;
}

bool ScenarioMedia::save(XML_Helper* helper) const
{
  if (!anyValueSet())
    return true;
  bool retval = true;
  retval &= helper->open_tag(ScenarioMedia::d_tag);
  retval &= helper->save("next_turn_image", d_next_turn->getName ());
  retval &= helper->save("city_defeated_image", d_city_defeated->getName ());
  retval &= helper->save("winning_image", d_winning->getName ());
  retval &= helper->save("male_hero_image", d_hero[0]->getName ());
  retval &= helper->save("female_hero_image", d_hero[1]->getName ());
  retval &= helper->save("ruin_success_image", d_ruin_success->getName ());
  retval &= helper->save("ruin_defeat_image", d_ruin_defeat->getName ());
  retval &= helper->save("parley_offered_image", d_parley_offered->getName ());
  retval &= helper->save("parley_refused_image", d_parley_refused->getName ());
  retval &= d_hero_newlevel[0]->save (helper, "hero_newlevel_male_image");
  retval &= d_hero_newlevel[1]->save (helper, "hero_newlevel_female_image");
  retval &= helper->save("small_medals_image", d_medal[0]->getName ());
  retval &= helper->save("big_medals_image", d_medal[1]->getName ());
  retval &= helper->save("commentator_image", d_commentator->getName ());
  retval &= helper->save("bless_sound", d_bless_sound->get_name ());
  retval &= helper->save("hero_name", d_hero_sound->get_name ());
  retval &= helper->save("battle_name", d_battle_sound->get_name ());
  retval &= helper->save("defeat_name", d_defeat_sound->get_name ());
  retval &= helper->save("victory_name", d_victory_sound->get_name ());
  retval &= helper->save("back_name", d_back_sound->get_name ());
  retval &= helper->close_tag();
  return retval;
}

ScenarioMedia::~ScenarioMedia()
{
  uninstantiateImages();
  for (auto i: d_musicMap)
    {
      MusicItem *m = dynamic_cast<MusicItem*>(i.second);
      if (m)
        {
          File::erase (m->file);
          delete m;
        }
    }
  for (auto i : getMaskedImages ())
    delete i;
  for (auto i : getImages ())
    delete i;

  delete d_bless_sound;
  delete d_hero_sound;
  delete d_battle_sound;
  delete d_defeat_sound;
  delete d_victory_sound;
  delete d_back_sound;
}

void ScenarioMedia::uninstantiateImages()
{
  for (auto i : getImages ())
    i->uninstantiateImages ();
  for (auto i : getMaskedImages ())
    i->uninstantiateImages ();
}

void ScenarioMedia::copySound(Tar_Helper &t, TarFileSound *s, Glib::ustring piece, bool &broken)
{
  std::string name = s->get_name ();
  if (name.empty ())
    return;
  Glib::ustring n = t.getFile (name, broken);
  if (!broken && n != "")
    {
      Glib::ustring tmpfile = File::get_tmp_file(".ogg");
      File::copy (n, tmpfile);
      MusicItem *item = new MusicItem();
      if (item)
        {
          item->file = tmpfile;
          item->background = false;
          item->alias = "";
          d_musicMap[piece] = item;
        }
    }
}

void ScenarioMedia::copySounds(Tar_Helper &t, bool &broken)
{
  copySound(t, d_bless_sound, "bless", broken);
  copySound(t, d_hero_sound, "hero", broken);
  copySound(t, d_battle_sound, "battle", broken);
  copySound(t, d_defeat_sound, "defeat", broken);
  copySound(t, d_victory_sound, "victory", broken);

  copySound(t, d_back_sound, "back", broken);

  MusicItem *back = getSoundEffect("back");
  if (back)
    {
      back->background = true;
      d_bgMap.push_back("back");
    }
}

std::vector<TarFileImage*> ScenarioMedia::getImages()
{
  std::vector<TarFileImage*> i;
  i.push_back (d_next_turn);
  i.push_back (d_city_defeated);
  i.push_back (d_winning);
  i.push_back (d_hero[0]);
  i.push_back (d_hero[1]);
  i.push_back (d_ruin_success);
  i.push_back (d_ruin_defeat);
  i.push_back (d_parley_offered);
  i.push_back (d_parley_refused);
  i.push_back (d_medal[0]);
  i.push_back (d_medal[1]);
  i.push_back (d_commentator);
  return i;
}

std::vector<TarFileMaskedImage*> ScenarioMedia::getMaskedImages ()
{
  std::vector<TarFileMaskedImage*> i;
  i.push_back (d_hero_newlevel[0]);
  i.push_back (d_hero_newlevel[1]);
  return i;
}

void ScenarioMedia::instantiateImages(Tar_Helper &t, bool &broken)
{
  for (auto i : getImages ())
    if (i->getName ().empty () == false)
      {
        broken = i->load (&t);
        if (broken)
          break;
        i->instantiateImages ();
      }
  for (auto i : getMaskedImages ())
    if (i->getName ().empty () == false)
      {
        broken = i->load (&t);
        if (broken)
          break;
        i->instantiateImages ();
      }
}

MusicItem* ScenarioMedia::getSoundEffect(Glib::ustring n)
{
  auto it = d_musicMap.find (n);
  if (it == d_musicMap.end ())
    return NULL;
  return d_musicMap[n];
}

void ScenarioMedia::getFilenames(std::list<Glib::ustring> &files)
{
  for (auto i : getImages ())
    if (i->getName ().empty () == false)
      files.push_back (i->getName ());

  for (auto i : getMaskedImages ())
    if (i->getName ().empty () == false)
      files.push_back (i->getName ());

  if (getBlessSound ()->get_name() != "")
    files.push_back (getBlessSound ()->get_name());
  if (getHeroSound ()->get_name () != "")
    files.push_back (getHeroSound ()->get_name());
  if (getBattleSound ()->get_name () != "")
    files.push_back (getBattleSound ()->get_name());
  if (getDefeatSound ()->get_name () != "")
    files.push_back (getDefeatSound ()->get_name());
  if (getVictorySound ()->get_name () != "")
    files.push_back (getVictorySound ()->get_name());
  if (getBackSound ()->get_name () != "")
    files.push_back (getBackSound ()->get_name());
}

void ScenarioMedia::uninstantiateSameNamedImages (Glib::ustring name)
{
  TarFileImage::uninstantiate (name, getImages ());
  TarFileMaskedImage::uninstantiate (name, getMaskedImages ());
}

void ScenarioMedia::reset (ScenarioMedia *m)
{
  delete d_instance;
  d_instance = m;
}
