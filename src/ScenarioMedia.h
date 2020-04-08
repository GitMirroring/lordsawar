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

        Glib::ustring getNextTurnImageName() {return d_next_turn_name;}
        Glib::ustring getCityDefeatedImageName() {return d_city_defeated_name;}
        Glib::ustring getWinningImageName() {return d_winning_name;}
        Glib::ustring getMaleHeroImageName() {return d_male_hero_name;}
        Glib::ustring getFemaleHeroImageName() {return d_female_hero_name;}
        Glib::ustring getRuinSuccessImageName() {return d_ruin_success_name;}
        Glib::ustring getRuinDefeatImageName() {return d_ruin_defeat_name;}
        Glib::ustring getParleyOfferedImageName() {return d_parley_offered_name;}
        Glib::ustring getParleyRefusedImageName() {return d_parley_refused_name;}
        Glib::ustring getHeroNewLevelMaleImageName() {return d_hero_newlevel_male_name;}
        Glib::ustring getHeroNewLevelFemaleImageName() {return d_hero_newlevel_female_name;}
        Glib::ustring getSmallMedalsImageName() {return d_small_medals_name;}
        Glib::ustring getBigMedalsImageName() {return d_big_medals_name;}
        Glib::ustring getCommentatorImageName() {return d_commentator_name;}
        Glib::ustring getBlessSoundName() {return d_bless_name;}
        Glib::ustring getHeroSoundName() {return d_hero_name;}
        Glib::ustring getBattleSoundName() {return d_battle_name;}
        Glib::ustring getDefeatSoundName() {return d_defeat_name;}
        Glib::ustring getVictorySoundName() {return d_victory_name;}
        Glib::ustring getBackSoundName() {return d_back_name;}

        PixMask *getNextTurnImage() {return d_next_turn_image;}
        PixMask *getCityDefeatedImage() {return d_city_defeated_image;}
        PixMask *getWinningImage() {return d_winning_image;}
        PixMask *getMaleHeroImage() {return d_male_hero_image;}
        PixMask *getFemaleHeroImage() {return d_female_hero_image;}
        PixMask *getRuinSuccessImage() {return d_ruin_success_image;}
        PixMask *getRuinDefeatImage() {return d_ruin_defeat_image;}
        PixMask *getParleyOfferedImage() {return d_parley_offered_image;}
        PixMask *getParleyRefusedImage() {return d_parley_refused_image;}
        PixMask *getHeroNewLevelMaleImage() {return d_hero_newlevel_male_image;}
        PixMask *getHeroNewLevelMaleMask() {return d_hero_newlevel_male_mask;}
        PixMask *getHeroNewLevelFemaleImage() {return d_hero_newlevel_female_image;}
        PixMask *getHeroNewLevelFemaleMask() {return d_hero_newlevel_female_mask;}
        PixMask *getSmallMedalImage(guint32 i)
          {return d_small_medal_images.size () > i ? d_small_medal_images[i] : NULL;}
        PixMask *getBigMedalImage(guint32 i)
          {return d_big_medal_images.size () > i ? d_big_medal_images[i] : NULL;}
        PixMask *getCommentatorImage() {return d_commentator_image;}

        void clearNextTurnImage(bool clear_name = true);
        void clearCityDefeatedImage(bool clear_name = true);
        void clearWinningImage(bool clear_name = true);
        void clearMaleHeroImage(bool clear_name = true);
        void clearFemaleHeroImage(bool clear_name = true);
        void clearRuinSuccessImage(bool clear_name = true);
        void clearRuinDefeatImage(bool clear_name = true);
        void clearParleyOfferedImage(bool clear_name = true);
        void clearParleyRefusedImage(bool clear_name = true);
        void clearSmallMedalImage(bool clear_name = true);
        void clearBigMedalImage(bool clear_name = true);
        void clearHeroNewLevelMaleImage (bool clear_name = true);
        void clearHeroNewLevelFemaleImage (bool clear_name = true);
        void clearCommentatorImage (bool clear_name = true);

        bool instantiateNextTurnImage(TarFile *t);
        bool instantiateCityDefeatedImage(TarFile *t);
        bool instantiateWinningImage(TarFile *t);
        bool instantiateMaleHeroImage(TarFile *t);
        bool instantiateFemaleHeroImage(TarFile *t);
        bool instantiateRuinSuccessImage(TarFile *t);
        bool instantiateRuinDefeatImage(TarFile *t);
        bool instantiateParleyOfferedImage(TarFile *t);
        bool instantiateParleyRefusedImage(TarFile *t);
        bool instantiateSmallMedalImage(TarFile *t);
        bool instantiateBigMedalImage(TarFile *t);
        bool instantiateHeroNewLevelMaleImage (TarFile *t);
        bool instantiateHeroNewLevelFemaleImage (TarFile *t);
        bool instantiateCommentatorImage (TarFile *t);

        MusicItem* getSoundEffect(Glib::ustring n);
        std::vector<Glib::ustring> getBackgroundMusic() const {return d_bgMap;}
        std::map<Glib::ustring, MusicItem*> getSounds() const {return d_musicMap;}
        //Set methods

        void setNextTurnImageName(Glib::ustring n) {d_next_turn_name = n;}
        void setCityDefeatedImageName(Glib::ustring n) {d_city_defeated_name=n;}
        void setWinningImageName(Glib::ustring n) {d_winning_name = n;}
        void setMaleHeroImageName(Glib::ustring n) {d_male_hero_name = n;}
        void setFemaleHeroImageName(Glib::ustring n) {d_female_hero_name = n;}
        void setRuinSuccessImageName(Glib::ustring n) {d_ruin_success_name = n;}
        void setRuinDefeatImageName(Glib::ustring n) {d_ruin_defeat_name = n;}
        void setParleyOfferedImageName(Glib::ustring n) {d_parley_offered_name = n;}
        void setParleyRefusedImageName(Glib::ustring n) {d_parley_refused_name = n;}
        void setHeroNewLevelMaleImageName(Glib::ustring n) {d_hero_newlevel_male_name = n;}
        void setHeroNewLevelFemaleImageName(Glib::ustring n) {d_hero_newlevel_female_name = n;}
        void setSmallMedalsImageName(Glib::ustring n) {d_small_medals_name = n;}
        void setBigMedalsImageName(Glib::ustring n) {d_big_medals_name = n;}
        void setCommentatorImageName(Glib::ustring n) {d_commentator_name = n;}

        void setNextTurnImage(PixMask *i) {d_next_turn_image = i;}
        void setCityDefeatedImage(PixMask *i) {d_city_defeated_image = i;}
        void setWinningImage(PixMask *i) {d_winning_image = i;}
        void setMaleHeroImage(PixMask *i) {d_male_hero_image = i;}
        void setFemaleHeroImage(PixMask *i) {d_female_hero_image = i;}
        void setRuinSuccessImage(PixMask *i) {d_ruin_success_image = i;}
        void setRuinDefeatImage(PixMask *i) {d_ruin_defeat_image = i;}
        void setParleyOfferedImage(PixMask *i) {d_parley_offered_image = i;}
        void setParleyRefusedImage(PixMask *i) {d_parley_refused_image = i;}
        void setHeroNewLevelMaleImage(PixMask *i)
          {d_hero_newlevel_male_image=i;}
        void setHeroNewLevelMaleMask(PixMask *m) {d_hero_newlevel_male_mask=m;}
        void setHeroNewLevelFemaleImage(PixMask *i)
          {d_hero_newlevel_female_image=i;}
        void setHeroNewLevelFemaleMask(PixMask *m)
          {d_hero_newlevel_female_mask=m;}
        void setSmallMedalsImage(guint32 n, PixMask *i)
          { if (n < d_small_medal_images.size ()) d_small_medal_images[n] = i;}
        void setBigMedalsImage(guint32 n, PixMask *i)
          {if (n < d_big_medal_images.size ()) d_big_medal_images[n] = i;}
        void setCommentatorImage (PixMask *i) {d_commentator_image = i;}

        void setBlessSoundName(Glib::ustring n) {d_bless_name = n;}
        void setHeroSoundName(Glib::ustring n) {d_hero_name = n;}
        void setBattleSoundName(Glib::ustring n) {d_battle_name = n;}
        void setDefeatSoundName(Glib::ustring n) {d_defeat_name = n;}
        void setVictorySoundName(Glib::ustring n) {d_victory_name = n;}
        void setBackSoundName(Glib::ustring n) {d_back_name = n;}

	// Methods that operate on class data and modify the class.

        void instantiateImages(Tar_Helper &t, bool &broken);
        void copySounds(Tar_Helper &t, bool &broken);

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
        Glib::ustring d_next_turn_name;
        Glib::ustring d_city_defeated_name;
        Glib::ustring d_winning_name;
        Glib::ustring d_male_hero_name;
        Glib::ustring d_female_hero_name;
        Glib::ustring d_ruin_success_name;
        Glib::ustring d_ruin_defeat_name;
        Glib::ustring d_parley_offered_name;
        Glib::ustring d_parley_refused_name;
        Glib::ustring d_hero_newlevel_male_name;
        Glib::ustring d_hero_newlevel_female_name;
        Glib::ustring d_small_medals_name;
        Glib::ustring d_big_medals_name;
        Glib::ustring d_commentator_name;
        Glib::ustring d_bless_name;
        Glib::ustring d_hero_name;
        Glib::ustring d_battle_name;
        Glib::ustring d_defeat_name;
        Glib::ustring d_victory_name;
        Glib::ustring d_back_name;

        std::map<Glib::ustring, MusicItem*> d_musicMap;
        std::vector<Glib::ustring> d_bgMap;

        PixMask *d_next_turn_image;
        PixMask *d_city_defeated_image;
        PixMask *d_winning_image;
        PixMask *d_male_hero_image;
        PixMask *d_female_hero_image;
        PixMask *d_ruin_success_image;
        PixMask *d_ruin_defeat_image;
        PixMask *d_parley_offered_image;
        PixMask *d_parley_refused_image;
        PixMask *d_hero_newlevel_male_image;
        PixMask *d_hero_newlevel_male_mask;
        PixMask *d_hero_newlevel_female_image;
        PixMask *d_hero_newlevel_female_mask;
        std::vector<PixMask *> d_small_medal_images;
        std::vector<PixMask *> d_big_medal_images;
        PixMask *d_commentator_image;

        //helpers
        void instantiateMaskedImage(Tar_Helper &t, Glib::ustring name, PixMask **image, PixMask **mask, bool &broken);
        void instantiateImage(Tar_Helper &t, Glib::ustring name, PixMask **image, bool &broken);
        void instantiateImageRow(Tar_Helper &t, Glib::ustring name, int num, std::vector<PixMask *>&images, bool &broken);
        void uninstantiateImages();
        bool anyValueSet() const;
        void copySound(Tar_Helper &t, Glib::ustring name, Glib::ustring piece, bool &broken);
};

#endif //SCENARIO_MEDIA_H
