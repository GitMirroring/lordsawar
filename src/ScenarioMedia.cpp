// Copyright (C) 2017 Ben Asselstine
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
 : d_next_turn_name(""), d_city_defeated_name(""), d_winning_name(""),
    d_male_hero_name(""), d_female_hero_name(""), d_ruin_success_name(""),
    d_ruin_defeat_name(""), d_parley_offered_name(""),
    d_parley_refused_name(""), d_hero_newlevel_male_name(""),
    d_hero_newlevel_female_name(""), d_small_medals_name(""),
    d_big_medals_name(""), d_bless_name(""), d_hero_name(""), d_battle_name(""),
    d_defeat_name(""), d_victory_name(""), d_back_name (""),
    d_next_turn_image(0), d_city_defeated_image(0), d_winning_image(0),
    d_male_hero_image(0), d_female_hero_image(0),
    d_ruin_success_image(0), d_ruin_defeat_image(0), d_parley_offered_image(0),
    d_parley_refused_image(0), d_hero_newlevel_male_image(0),
    d_hero_newlevel_male_mask(0), d_hero_newlevel_female_image(0),
    d_hero_newlevel_female_mask(0)
{
}

ScenarioMedia::ScenarioMedia(XML_Helper *helper)
 : d_next_turn_name(""), d_city_defeated_name(""), d_winning_name(""),
    d_male_hero_name(""), d_female_hero_name(""), d_ruin_success_name(""),
    d_ruin_defeat_name(""), d_parley_offered_name(""),
    d_parley_refused_name(""), d_hero_newlevel_male_name(""),
    d_hero_newlevel_female_name(""), d_small_medals_name(""),
    d_big_medals_name(""), d_bless_name(""), d_hero_name(""), d_battle_name(""),
    d_defeat_name(""), d_victory_name(""), d_back_name (""),
    d_next_turn_image(0), d_city_defeated_image(0), d_winning_image(0),
    d_male_hero_image(0), d_female_hero_image(0),
    d_ruin_success_image(0), d_ruin_defeat_image(0), d_parley_offered_image(0),
    d_parley_refused_image(0), d_hero_newlevel_male_image(0),
    d_hero_newlevel_male_mask(0), d_hero_newlevel_female_image(0),
    d_hero_newlevel_female_mask(0)
{
  helper->getData(d_next_turn_name, "next_turn_image");
  helper->getData(d_city_defeated_name, "city_defeated_image");
  helper->getData(d_winning_name, "winning_image");
  helper->getData(d_male_hero_name, "male_hero_image");
  helper->getData(d_female_hero_name, "female_hero_image");
  helper->getData(d_ruin_success_name, "ruin_success_image");
  helper->getData(d_ruin_defeat_name, "ruin_defeat_image");
  helper->getData(d_parley_offered_name, "parley_offered_image");
  helper->getData(d_parley_refused_name, "parley_refused_image");
  helper->getData(d_hero_newlevel_male_name, "hero_newlevel_male_image");
  helper->getData(d_hero_newlevel_female_name,"hero_newlevel_female_image");
  helper->getData(d_small_medals_name, "small_medals_image");
  helper->getData(d_big_medals_name, "big_medals_image");
  helper->getData(d_bless_name, "bless_sound");
  helper->getData(d_hero_name,"d_hero_name");
  helper->getData(d_battle_name,"d_battle_name");
  helper->getData(d_defeat_name,"d_defeat_name");
  helper->getData(d_victory_name,"d_victory_name");
  helper->getData(d_back_name,"d_back_name");
}

bool ScenarioMedia::anyValueSet() const
{
  if (d_next_turn_name != "" ||
      d_city_defeated_name != "" ||
      d_winning_name != "" ||
      d_male_hero_name != "" ||
      d_female_hero_name != "" ||
      d_ruin_success_name != "" ||
      d_ruin_defeat_name != "" ||
      d_parley_offered_name != "" ||
      d_parley_refused_name != "" ||
      d_hero_newlevel_male_name != "" ||
      d_hero_newlevel_female_name != "" ||
      d_small_medals_name != "" ||
      d_big_medals_name != "" ||
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
  retval &= helper->saveData("next_turn_image", d_next_turn_name);
  retval &= helper->saveData("city_defeated_image", d_city_defeated_name);
  retval &= helper->saveData("winning_image", d_winning_name);
  retval &= helper->saveData("male_hero_image", d_male_hero_name);
  retval &= helper->saveData("female_hero_image", d_female_hero_name);
  retval &= helper->saveData("ruin_success_image", d_ruin_success_name);
  retval &= helper->saveData("ruin_defeat_image", d_ruin_defeat_name);
  retval &= helper->saveData("parley_offered_image", d_parley_offered_name);
  retval &= helper->saveData("parley_refused_image", d_parley_refused_name);
  retval &= helper->saveData("hero_newlevel_male_image",
                             d_hero_newlevel_male_name);
  retval &= helper->saveData("hero_newlevel_female_image",
                             d_hero_newlevel_female_name);
  retval &= helper->saveData("small_medals_image", d_small_medals_name);
  retval &= helper->saveData("big_medals_image", d_big_medals_name);
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
}

void ScenarioMedia::uninstantiateImages()
{
  if (d_next_turn_image)
    delete d_next_turn_image;
  d_next_turn_image = NULL;
  if (d_city_defeated_image)
    delete d_city_defeated_image;
  d_city_defeated_image = NULL;
  if (d_winning_image)
    delete d_winning_image;
  d_winning_image = NULL;
  if (d_male_hero_image)
    delete d_male_hero_image;
  d_male_hero_image = NULL;
  if (d_female_hero_image)
    delete d_female_hero_image;
  d_female_hero_image = NULL;
  if (d_ruin_success_image)
    delete d_ruin_success_image;
  d_ruin_success_image = NULL;
  if (d_ruin_defeat_image)
    delete d_ruin_defeat_image;
  d_ruin_defeat_image = NULL;
  if (d_parley_offered_image)
    delete d_parley_offered_image;
  d_parley_offered_image = NULL;
  if (d_parley_refused_image)
    delete d_parley_refused_image;
  d_parley_refused_image = NULL;
  if (d_hero_newlevel_male_image)
    delete d_hero_newlevel_male_image;
  d_hero_newlevel_male_image = NULL;
  if (d_hero_newlevel_male_mask)
    delete d_hero_newlevel_male_mask;
  d_hero_newlevel_male_mask = NULL;
  if (d_hero_newlevel_female_image)
    delete d_hero_newlevel_female_image;
  d_hero_newlevel_female_image = NULL;
  if (d_hero_newlevel_female_mask)
    delete d_hero_newlevel_female_mask;
  d_hero_newlevel_female_mask = NULL;
  for (auto i : d_small_medal_images)
    delete i;
  d_small_medal_images.clear();
  for (auto i : d_big_medal_images)
    delete i;
  d_big_medal_images.clear();
}

void ScenarioMedia::instantiateImageRow(Tar_Helper &t, Glib::ustring name, int num, std::vector<PixMask *>&images, bool &broken)
{
  if (name != "")
    {
      Glib::ustring n = t.getFile (name + ".png", broken);
      if (!broken)
        images = disassemble_row(n, num, broken);
    }
}

void ScenarioMedia::instantiateMaskedImage(Tar_Helper &t, Glib::ustring name, PixMask **image, PixMask **mask, bool &broken)
{
  if (name != "")
    {
      Glib::ustring n = t.getFile (name + ".png", broken);
      if (!broken)
        {
          std::vector<PixMask* > half = disassemble_row(n, 2, broken);
          if (!broken)
            {
              *image = half[0];
              *mask = half[1];
            }
        }
      if (broken)
        return;
    }
}

void ScenarioMedia::instantiateImage(Tar_Helper &t, Glib::ustring name, PixMask **image, bool &broken)
{
  if (name != "")
    {
      Glib::ustring n = t.getFile (name + ".png", broken);
      if (!broken)
        {
          PixMask *i = PixMask::create (n, broken);
          if (!broken)
            *image = i;
        }
      if (broken)
        return;
    }
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

void ScenarioMedia::instantiateImages(Tar_Helper &t, bool &broken)
{
  instantiateImage (t, d_next_turn_name, &d_next_turn_image, broken);
  if (!broken)
    instantiateImage (t, d_city_defeated_name, &d_city_defeated_image, broken);
  if (!broken)
    instantiateImage (t, d_winning_name, &d_winning_image, broken);
  if (!broken)
    instantiateImage (t, d_male_hero_name, &d_male_hero_image, broken);
  if (!broken)
    instantiateImage (t, d_female_hero_name, &d_female_hero_image, broken);
  if (!broken)
    instantiateImage (t, d_ruin_success_name, &d_ruin_success_image, broken);
  if (!broken)
    instantiateImage (t, d_ruin_defeat_name, &d_ruin_defeat_image, broken);
  if (!broken)
    instantiateImage (t, d_parley_offered_name, &d_parley_offered_image,
                      broken);
  if (!broken)
    instantiateImage (t, d_parley_refused_name, &d_parley_refused_image,
                      broken);
  if (!broken)
    instantiateMaskedImage (t, d_hero_newlevel_male_name, &d_hero_newlevel_male_image, &d_hero_newlevel_male_mask, broken);
  if (!broken)
    instantiateMaskedImage (t, d_hero_newlevel_female_name, &d_hero_newlevel_female_image, &d_hero_newlevel_female_mask, broken);
  if (!broken)
    instantiateImageRow (t, d_small_medals_name, MEDAL_TYPES, d_small_medal_images, broken);
  if (!broken)
    instantiateImageRow (t, d_big_medals_name, MEDAL_TYPES, d_big_medal_images, broken);
}
        
MusicItem* ScenarioMedia::getSoundEffect(Glib::ustring n)
{
  return d_musicMap[n];
}

void ScenarioMedia::getFilenames(std::list<Glib::ustring> &files)
{
  if (getNextTurnImageName() != "")
    files.push_back (getNextTurnImageName() + ".png");
  if (getCityDefeatedImageName() != "")
    files.push_back (getCityDefeatedImageName() + ".png");
  if (getWinningImageName() != "")
    files.push_back (getWinningImageName() + ".png");
  if (getMaleHeroImageName() != "")
    files.push_back (getMaleHeroImageName() + ".png");
  if (getFemaleHeroImageName() != "")
    files.push_back (getFemaleHeroImageName() + ".png");
  if (getRuinSuccessImageName() != "")
    files.push_back (getRuinSuccessImageName() + ".png");
  if (getRuinDefeatImageName() != "")
    files.push_back (getRuinDefeatImageName() + ".png");
  if (getParleyOfferedImageName() != "")
    files.push_back (getParleyOfferedImageName() + ".png");
  if (getParleyRefusedImageName() != "")
    files.push_back (getParleyRefusedImageName() + ".png");
  if (getHeroNewLevelMaleImageName() != "")
    files.push_back (getHeroNewLevelMaleImageName() + ".png");
  if (getHeroNewLevelFemaleImageName() != "")
    files.push_back (getHeroNewLevelFemaleImageName() + ".png");
  if (getSmallMedalsImageName() != "")
    files.push_back (getSmallMedalsImageName() + ".png");
  if (getBigMedalsImageName() != "")
    files.push_back (getBigMedalsImageName() + ".png");
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
