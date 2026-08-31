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

#pragma once
#ifndef SCENARIO_MEDIA_H
#define SCENARIO_MEDIA_H

#include <gtkmm.h>
#include <vector>
#include "tar-helper.h"
#include "snd.h"
#include "file.h"

class XML_Helper;
class TarFileMaskedImage;
class TarFileImage;
class TarFileSound;

//! Scenario Media provides images/sounds/music for the scenario
/**
 *
 * Usually we use the default images, but ScenarioMedia lets us override
 * those defaults.
 *
 * It lives in the savefile at lordsawar/media.
 */

class ScenarioMedia
{
    public:

	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag;

        //! Makes a copy of the scenario media.
        ScenarioMedia * copy () {return new ScenarioMedia (*this);}

        //! Returns the singleton instance.
	static ScenarioMedia* instance();

        //! Returns the singleton instance by loading it from a save-file.
	static ScenarioMedia* instance(XML_Helper *helper);

        //! Explicitly deletes the singleton instance.
        static void deleteInstance();


        //Get methods

        TarFileMaskedImage *getHeroNewLevelMaskedImage (bool female)
          {return d_hero_newlevel[female ? 1 : 0];}
        TarFileImage *getNextTurnImage () {return d_next_turn;}
        TarFileImage *getCityDefeatedImage () {return d_city_defeated;}
        TarFileImage *getWinningImage () {return d_winning;}
        TarFileImage *getHeroOfferedImage (bool female)
          {return female ? d_hero[1] : d_hero[0];}
        TarFileImage *getRuinSuccessImage () {return d_ruin_success;}
        TarFileImage *getRuinDefeatImage () {return d_ruin_defeat;}
        TarFileImage *getParleyOfferedImage () {return d_parley_offered;}
        TarFileImage *getParleyRefusedImage () {return d_parley_refused;}
        TarFileImage *getMedalImage(bool large)
          {return large ? d_medal[1] : d_medal[0];}
        TarFileImage *getCommentatorImage () {return d_commentator;}

        TarFileSound *getBlessSound() {return d_bless_sound;}
        TarFileSound *getHeroSound() {return d_hero_sound;}
        TarFileSound *getBattleSound() {return d_battle_sound;}
        TarFileSound *getDefeatSound() {return d_defeat_sound;}
        TarFileSound *getVictorySound() {return d_victory_sound;}
        TarFileSound *getBackSound() {return d_back_sound;}

        MusicItem* getSoundEffect(Glib::ustring n);
        const std::vector<Glib::ustring>& getBackgroundMusic() const {return d_bgMap;}
        std::map<Glib::ustring, MusicItem*> getSounds() const {return d_musicMap;}

	// Methods that operate on class data and modify the class.

        void instantiateImages(Tar_Helper &t, bool &broken);
        void copySounds(Tar_Helper &t, bool &broken);
        void uninstantiateSameNamedImages (Glib::ustring name);

	// Methods that operate on class data and do not modify the class.

        //! Saves the scenario-media data to an opened saved-game file.
        bool save(XML_Helper* helper) const;

	void getFilenames(std::list<Glib::ustring> &files);

	// Static Methods

        static Glib::ustring getDefaultNextTurnImageFilename ()
          {return File::getVariousFile ("ship.png"); }
        static Glib::ustring getDefaultCityDefeatedImageFilename ()
          {return File::getVariousFile ("city_occupied.png");}
        static Glib::ustring getDefaultWinningImageFilename ()
          {return File::getVariousFile ("win.png");}
        static Glib::ustring getDefaultMaleHeroImageFilename ()
          {return File::getVariousFile ("recruit_male.png");}
        static Glib::ustring getDefaultFemaleHeroImageFilename ()
          {return File::getVariousFile ("recruit_female.png");}
        static Glib::ustring getDefaultRuinSuccessImageFilename ()
          {return File::getVariousFile ("ruin_2.png");}
        static Glib::ustring getDefaultRuinDefeatImageFilename ()
          {return File::getVariousFile ("ruin_1.png");}
        static Glib::ustring getDefaultParleyOfferedImageFilename ()
          {return File::getVariousFile ("parley_offered.png");}
        static Glib::ustring getDefaultParleyRefusedImageFilename ()
          {return File::getVariousFile ("parley_refused.png");}
        static Glib::ustring getDefaultHeroNewLevelMaleImageFilename ()
          {return File::getVariousFile ("hero-newlevel-male.png");}
        static Glib::ustring getDefaultHeroNewLevelFemaleImageFilename ()
          {return File::getVariousFile ("hero-newlevel-female.png");}
        static Glib::ustring getDefaultSmallMedalsImageFilename ()
          {return File::getVariousFile ("medals_mask.png");}
        static Glib::ustring getDefaultBigMedalsImageFilename ()
          {return File::getVariousFile ("bigmedals.png");}
        static Glib::ustring getDefaultCommentatorImageFilename ()
          {return File::getVariousFile ("commentator.png");}
        static Glib::ustring getDefaultBlessSoundFilename()
          {return Snd::instance ()->getFile ("bless.ogg");}
        static Glib::ustring getDefaultHeroSoundFilename()
          {return Snd::instance ()->getFile ("hero.ogg");}
        static Glib::ustring getDefaultBattleSoundFilename ()
          {return Snd::instance ()->getFile ("battle.ogg");}
        static Glib::ustring getDefaultDefeatSoundFilename ()
          {return Snd::instance ()->getFile ("defeat.ogg");}
        static Glib::ustring getDefaultVictorySoundFilename ()
          {return Snd::instance ()->getFile ("victory.ogg");}
        static Glib::ustring getDefaultBackSoundFilename ()
          {return Snd::instance ()->getFile ("back.ogg");}
        //! Replace the current scenario media with another.
        static void reset (ScenarioMedia *m);

        //! Destructor.
        ~ScenarioMedia();

    protected:

	//! Creates a new ScenarioMedia object from scratch.
        ScenarioMedia();

        //! Creates a ScenarioMedia object from another one.
        ScenarioMedia(const ScenarioMedia &sm);

	//! Creates a new ScenarioMedia object from an opened save-file.
        ScenarioMedia(XML_Helper *helper);

    private:

        //data
        static ScenarioMedia* d_instance;

        //! The image shown when the hero levels up.  0 is male, 1 is female

        TarFileImage *d_next_turn;
        TarFileImage *d_city_defeated;
        TarFileImage *d_winning;
        TarFileImage *d_ruin_success;
        TarFileImage *d_ruin_defeat;
        TarFileImage *d_parley_offered;
        TarFileImage *d_parley_refused;
        TarFileImage *d_commentator;

        TarFileSound *d_bless_sound;
        TarFileSound *d_hero_sound;
        TarFileSound *d_battle_sound;
        TarFileSound *d_defeat_sound;
        TarFileSound *d_victory_sound;
        TarFileSound *d_back_sound;

        TarFileMaskedImage *d_hero_newlevel[2];
        TarFileImage *d_hero[2]; //male is 0, female is 1
        TarFileImage *d_medal[2]; //small is 0, big is 1

        std::map<Glib::ustring, MusicItem*> d_musicMap;
        std::vector<Glib::ustring> d_bgMap;

        //helpers
        void uninstantiateImages();
        bool anyValueSet() const;
        void copySound(Tar_Helper &t, TarFileSound *s, Glib::ustring piece, bool &broken);
        std::vector<TarFileImage*> getImages();
        std::vector<TarFileMaskedImage*> getMaskedImages();
};

#endif
