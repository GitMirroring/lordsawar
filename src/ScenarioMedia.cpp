// Copyright (C) 2017, 2020 Ben Asselstine
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

#include "ScenarioMedia.h"
#include "defs.h"
#include "gui/image-helpers.h"
#include "xmlhelper.h"
#include "File.h"
#include "snd.h"
#include "tarfile.h"
#include "TarFileMaskedImage.h"
#include "TarFileImage.h"

ScenarioMedia* ScenarioMedia::d_instance = 0;

Glib::ustring ScenarioMedia::d_tag = "media";

ScenarioMedia* ScenarioMedia::getInstance()
{
  if (!d_instance)
    d_instance = new ScenarioMedia();

  return d_instance;
}

ScenarioMedia* ScenarioMedia::getInstance(XML_Helper* helper)
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
 : d_bless_name(""), d_hero_name(""), d_battle_name(""), d_defeat_name(""),
    d_victory_name(""), d_back_name ("")
{
  d_hero_newlevel[0] =
    new TarFileMaskedImage (TarFileMaskedImage::HORIZONTAL_MASK);
  d_hero_newlevel[1] =
    new TarFileMaskedImage (TarFileMaskedImage::HORIZONTAL_MASK);
  d_next_turn = new TarFileImage (1);
  d_city_defeated = new TarFileImage (1);
  d_winning = new TarFileImage (1);
  d_hero[0] = new TarFileImage (1);
  d_hero[1] = new TarFileImage (1);
  d_ruin_success = new TarFileImage (1);
  d_ruin_defeat = new TarFileImage (1);
  d_parley_offered = new TarFileImage (1);
  d_parley_refused = new TarFileImage (1);
  d_medal[0] = new TarFileImage (MEDAL_TYPES);
  d_medal[1] = new TarFileImage (MEDAL_TYPES);
  d_commentator = new TarFileImage (1);
}

ScenarioMedia::ScenarioMedia(XML_Helper *helper)
 : d_bless_name(""), d_hero_name(""), d_battle_name(""), d_defeat_name(""),
    d_victory_name(""), d_back_name ("")
{
  d_hero_newlevel[0] =
    new TarFileMaskedImage (TarFileMaskedImage::HORIZONTAL_MASK);
  d_hero_newlevel[1] =
    new TarFileMaskedImage (TarFileMaskedImage::HORIZONTAL_MASK);
  d_next_turn = new TarFileImage (1);
  d_city_defeated = new TarFileImage (1);
  d_winning = new TarFileImage (1);
  d_hero[0] = new TarFileImage (1);
  d_hero[1] = new TarFileImage (1);
  d_ruin_success = new TarFileImage (1);
  d_ruin_defeat = new TarFileImage (1);
  d_parley_offered = new TarFileImage (1);
  d_parley_refused = new TarFileImage (1);
  d_medal[0] = new TarFileImage (3);
  d_medal[1] = new TarFileImage (3);
  d_commentator = new TarFileImage (1);
  d_next_turn->load_name (helper, "next_turn_image");
  d_city_defeated->load_name (helper, "city_defeated_image");
  d_winning->load_name (helper, "winning_image");
  d_hero[0]->load_name (helper, "male_hero_image");
  d_hero[1]->load_name (helper, "female_hero_image");
  d_ruin_success->load_name (helper, "ruin_success_image");
  d_ruin_defeat->load_name (helper, "ruin_defeat_image");
  d_parley_offered->load_name (helper, "parley_offered_image");
  d_parley_refused->load_name (helper, "parley_refused_image");
  d_hero_newlevel[0]->load_name (helper, "hero_newlevel_male_image");
  d_hero_newlevel[1]->load_name (helper, "hero_newlevel_female_image");
  d_medal[0]->load_name (helper, "small_medals_image");
  d_medal[1]->load_name (helper, "big_medals_image");
  d_commentator->load_name (helper, "commentator_image");
  helper->getData(d_bless_name, "bless_sound");
  helper->getData(d_hero_name,"d_hero_name");
  helper->getData(d_battle_name,"d_battle_name");
  helper->getData(d_defeat_name,"d_defeat_name");
  helper->getData(d_victory_name,"d_victory_name");
  helper->getData(d_back_name,"d_back_name");
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
      d_bless_name != "" ||
      d_hero_name != "" ||
      d_battle_name != "" ||
      d_defeat_name != "" ||
      d_victory_name != "" ||
      d_back_name != "")
    return true;
  return false;
}

bool ScenarioMedia::save(XML_Helper* helper) const
{
  if (!anyValueSet())
    return true;
  bool retval = true;
  retval &= helper->openTag(ScenarioMedia::d_tag);
  retval &= helper->saveData("next_turn_image", d_next_turn->getName ());
  retval &= helper->saveData("city_defeated_image", d_city_defeated->getName ());
  retval &= helper->saveData("winning_image", d_winning->getName ());
  retval &= helper->saveData("male_hero_image", d_hero[0]->getName ());
  retval &= helper->saveData("female_hero_image", d_hero[1]->getName ());
  retval &= helper->saveData("ruin_success_image", d_ruin_success->getName ());
  retval &= helper->saveData("ruin_defeat_image", d_ruin_defeat->getName ());
  retval &= helper->saveData("parley_offered_image", d_parley_offered->getName ());
  retval &= helper->saveData("parley_refused_image", d_parley_refused->getName ());
  retval &= helper->saveData("hero_newlevel_male_image",
                             d_hero_newlevel[0]->getName ());
  retval &= helper->saveData("hero_newlevel_female_image",
                             d_hero_newlevel[0]->getName ());
  retval &= helper->saveData("small_medals_image", d_medal[0]->getName ());
  retval &= helper->saveData("big_medals_image", d_medal[1]->getName ());
  retval &= helper->saveData("commentator_image", d_commentator->getName ());
  retval &= helper->saveData("bless_sound", d_bless_name);
  retval &= helper->saveData("d_hero_name", d_hero_name);
  retval &= helper->saveData("d_battle_name", d_battle_name);
  retval &= helper->saveData("d_defeat_name", d_defeat_name);
  retval &= helper->saveData("d_victory_name", d_victory_name);
  retval &= helper->saveData("d_back_name", d_back_name);
  retval &= helper->closeTag();
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
  delete d_hero_newlevel[0];
  delete d_hero_newlevel[1];
}

void ScenarioMedia::uninstantiateImages()
{
  for (auto i : getTarFileImages ())
    i->uninstantiateImages ();
  d_hero_newlevel[0]->uninstantiateImages ();
  d_hero_newlevel[1]->uninstantiateImages ();
}

void ScenarioMedia::copySound(Tar_Helper &t, Glib::ustring name, Glib::ustring piece, bool &broken)
{
  Glib::ustring n = t.getFile (name + ".ogg", broken);
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
  Glib::ustring f = "";
  copySound(t, d_bless_name, "bless", broken);
  copySound(t, d_hero_name, "hero", broken);
  copySound(t, d_battle_name, "battle", broken);
  copySound(t, d_defeat_name, "defeat", broken);
  copySound(t, d_victory_name, "victory", broken);

  copySound(t, d_back_name, "back", broken);

  MusicItem *back = getSoundEffect("back");
  if (back)
    {
      back->background = true;
      d_bgMap.push_back("back");
    }
}

std::vector<TarFileImage*> ScenarioMedia::getTarFileImages()
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

void ScenarioMedia::instantiateImages(Tar_Helper &t, bool &broken)
{
  for (auto i : getTarFileImages ())
    {
      if (i->getName ().empty () == false)
        {
          broken = i->load (&t);
          if (broken)
            break;
          i->instantiateImages ();
        }
    }

  if (!broken)
    {
      if (d_hero_newlevel[0]->getName().empty () == false)
        {
          broken = d_hero_newlevel[0]->load (&t);
          if (!broken)
            d_hero_newlevel[0]->instantiateImages ();
        }
    }
  if (!broken)
    {
      if (d_hero_newlevel[1]->getName().empty () == false)
        {
          broken = d_hero_newlevel[1]->load (&t);
          if (!broken)
            d_hero_newlevel[1]->instantiateImages ();
        }
    }
}

MusicItem* ScenarioMedia::getSoundEffect(Glib::ustring n)
{
  return d_musicMap[n];
}

void ScenarioMedia::getFilenames(std::list<Glib::ustring> &files)
{
  for (auto i : getTarFileImages ())
    if (i->getName ().empty () == false)
      files.push_back (i->getName ());

  if (d_hero_newlevel[0]->getName() != "")
    files.push_back (d_hero_newlevel[0]->getName ());
  if (d_hero_newlevel[1]->getName() != "")
    files.push_back (d_hero_newlevel[1]->getName ());
  if (getBlessSoundName() != "")
    files.push_back (getBlessSoundName() + ".ogg");
  if (getHeroSoundName() != "")
    files.push_back (getHeroSoundName() + ".ogg");
  if (getBattleSoundName() != "")
    files.push_back (getBattleSoundName() + ".ogg");
  if (getDefeatSoundName() != "")
    files.push_back (getDefeatSoundName() + ".ogg");
  if (getVictorySoundName() != "")
    files.push_back (getVictorySoundName() + ".ogg");
  if (getBackSoundName() != "")
    files.push_back (getBackSoundName() + ".ogg");
}

Glib::ustring ScenarioMedia::getDefaultNextTurnImageFilename()
{
  return File::getVariousFile("ship.png");
}

Glib::ustring ScenarioMedia::getDefaultCityDefeatedImageFilename()
{
  return File::getVariousFile("city_occupied.png");
}

Glib::ustring ScenarioMedia::getDefaultWinningImageFilename()
{
  return File::getVariousFile("win.png");
}

Glib::ustring ScenarioMedia::getDefaultMaleHeroImageFilename()
{
  return File::getVariousFile("recruit_male.png");
}

Glib::ustring ScenarioMedia::getDefaultFemaleHeroImageFilename()
{
  return File::getVariousFile("recruit_female.png");
}

Glib::ustring ScenarioMedia::getDefaultRuinSuccessImageFilename()
{
  return File::getVariousFile("ruin_2.png");
}

Glib::ustring ScenarioMedia::getDefaultRuinDefeatImageFilename()
{
  return File::getVariousFile("ruin_1.png");
}

Glib::ustring ScenarioMedia::getDefaultParleyOfferedImageFilename()
{
  return File::getVariousFile("parley_offered.png");
}

Glib::ustring ScenarioMedia::getDefaultParleyRefusedImageFilename()
{
  return File::getVariousFile("parley_refused.png");
}

Glib::ustring ScenarioMedia::getDefaultHeroNewLevelMaleImageFilename()
{
  return File::getVariousFile("hero-newlevel-male.png");
}

Glib::ustring ScenarioMedia::getDefaultHeroNewLevelFemaleImageFilename()
{
  return File::getVariousFile("hero-newlevel-female.png");
}

Glib::ustring ScenarioMedia::getDefaultSmallMedalsImageFilename()
{
  return File::getVariousFile("medals_mask.png");
}

Glib::ustring ScenarioMedia::getDefaultBigMedalsImageFilename()
{
  return File::getVariousFile("bigmedals.png");
}

Glib::ustring ScenarioMedia::getDefaultCommentatorImageFilename()
{
  return File::getVariousFile("commentator.png");
}

Glib::ustring ScenarioMedia::getDefaultBlessSoundFilename()
{
  return Snd::getInstance()->getFile("bless");
}

Glib::ustring ScenarioMedia::getDefaultHeroSoundFilename()
{
  return Snd::getInstance()->getFile("hero");
}

Glib::ustring ScenarioMedia::getDefaultBattleSoundFilename()
{
  return Snd::getInstance()->getFile("battle");
}

Glib::ustring ScenarioMedia::getDefaultDefeatSoundFilename()
{
  return Snd::getInstance()->getFile("defeat");
}

Glib::ustring ScenarioMedia::getDefaultVictorySoundFilename()
{
  return Snd::getInstance()->getFile("victory");
}

Glib::ustring ScenarioMedia::getDefaultBackSoundFilename()
{
  return Snd::getInstance()->getFile("back");
}
