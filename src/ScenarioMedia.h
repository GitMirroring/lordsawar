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

#pragma once
#ifndef SCENARIO_MEDIA_H
#define SCENARIO_MEDIA_H

#include <gtkmm.h>
#include <vector>
#include "PixMask.h"
#include "tarhelper.h"
#include "snd.h"

class XML_Helper;
class TarFile;
class TarFileMaskedImage;
class TarFileImage;

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

        //! Returns the singleton instance.
	static ScenarioMedia* getInstance();

        //! Returns the singleton instance by loading it from a save-file.
	static ScenarioMedia* getInstance(XML_Helper *helper);

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

        Glib::ustring getBlessSoundName() {return d_bless_name;}
        Glib::ustring getHeroSoundName() {return d_hero_name;}
        Glib::ustring getBattleSoundName() {return d_battle_name;}
        Glib::ustring getDefeatSoundName() {return d_defeat_name;}
        Glib::ustring getVictorySoundName() {return d_victory_name;}
        Glib::ustring getBackSoundName() {return d_back_name;}

        MusicItem* getSoundEffect(Glib::ustring n);
        std::vector<Glib::ustring> getBackgroundMusic() const {return d_bgMap;}
        std::map<Glib::ustring, MusicItem*> getSounds() const {return d_musicMap;}
        //Set methods

        void setBlessSoundName(Glib::ustring n) {d_bless_name = n;}
        void setHeroSoundName(Glib::ustring n) {d_hero_name = n;}
        void setBattleSoundName(Glib::ustring n) {d_battle_name = n;}
        void setDefeatSoundName(Glib::ustring n) {d_defeat_name = n;}
        void setVictorySoundName(Glib::ustring n) {d_victory_name = n;}
        void setBackSoundName(Glib::ustring n) {d_back_name = n;}

	// Methods that operate on class data and modify the class.

        void instantiateImages(Tar_Helper &t, bool &broken);
        void copySounds(Tar_Helper &t, bool &broken);
        void uninstantiateSameNamedImages (Glib::ustring name);

	// Methods that operate on class data and do not modify the class.

        //! Saves the scenario-media data to an opened saved-game file.
        bool save(XML_Helper* helper) const;

	void getFilenames(std::list<Glib::ustring> &files);

	// Static Methods

        static Glib::ustring getDefaultNextTurnImageFilename();
        static Glib::ustring getDefaultCityDefeatedImageFilename();
        static Glib::ustring getDefaultWinningImageFilename();
        static Glib::ustring getDefaultMaleHeroImageFilename();
        static Glib::ustring getDefaultFemaleHeroImageFilename();
        static Glib::ustring getDefaultRuinSuccessImageFilename();
        static Glib::ustring getDefaultRuinDefeatImageFilename();
        static Glib::ustring getDefaultParleyOfferedImageFilename();
        static Glib::ustring getDefaultParleyRefusedImageFilename();
        static Glib::ustring getDefaultHeroNewLevelMaleImageFilename();
        static Glib::ustring getDefaultHeroNewLevelFemaleImageFilename();
        static Glib::ustring getDefaultSmallMedalsImageFilename();
        static Glib::ustring getDefaultBigMedalsImageFilename();
        static Glib::ustring getDefaultCommentatorImageFilename();
        static Glib::ustring getDefaultBlessSoundFilename();
        static Glib::ustring getDefaultHeroSoundFilename();
        static Glib::ustring getDefaultBattleSoundFilename();
        static Glib::ustring getDefaultDefeatSoundFilename();
        static Glib::ustring getDefaultVictorySoundFilename();
        static Glib::ustring getDefaultBackSoundFilename();
    protected:

	//! Creates a new ScenarioMedia object from scratch.
        ScenarioMedia();

	//! Creates a new ScenarioMedia object from an opened save-file.
        ScenarioMedia(XML_Helper *helper);

        //! Destructor.
        ~ScenarioMedia();

    private:

        //data
        static ScenarioMedia* d_instance;

        //! The image shown when the hero levels up.  0 is male, 1 is female
        TarFileMaskedImage *d_hero_newlevel[2];

        TarFileImage *d_next_turn;
        TarFileImage *d_city_defeated;
        TarFileImage *d_winning;
        TarFileImage *d_hero[2]; //male is 0, female is 1
        TarFileImage *d_ruin_success;
        TarFileImage *d_ruin_defeat;
        TarFileImage *d_parley_offered;
        TarFileImage *d_parley_refused;
        TarFileImage *d_medal[2]; //small is 0, big is 1
        TarFileImage *d_commentator;

        Glib::ustring d_bless_name;
        Glib::ustring d_hero_name;
        Glib::ustring d_battle_name;
        Glib::ustring d_defeat_name;
        Glib::ustring d_victory_name;
        Glib::ustring d_back_name;

        std::map<Glib::ustring, MusicItem*> d_musicMap;
        std::vector<Glib::ustring> d_bgMap;

        //helpers
        void uninstantiateImages();
        bool anyValueSet() const;
        void copySound(Tar_Helper &t, Glib::ustring name, Glib::ustring piece, bool &broken);
        std::vector<TarFileImage*> getImages();
        std::vector<TarFileMaskedImage*> getMaskedImages();
};

#endif //SCENARIO_MEDIA_H
