//  Copyright (C) 2007, 2008, 2009, 2010, 2011, 2014, 2015, 2020 Ben Asselstine
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
#ifndef ARMYSET_H
#define ARMYSET_H

#include <gtkmm.h>
#include <map>
#include <vector>
#include <sigc++/trackable.h>

#include "armyproto.h"
#include "set.h"
#include "hero.h"

//! A collection of Army prototype objects.
/**
 * An Armyset is a complete set of Army prototype objects.  An Army prototype
 * is a kind of Army, as opposed to an Army unit instance (e.g. on the game
 * map).  See the Army class for more information about what an Army prototype
 * is.  The Armyset describes the size of the graphic tiles that each Army 
 * graphic occupies on the screen (Army::d_tilesize).
 * There special images are kept with the Armyset:  the ship picture, the
 * planted standard picture, and the bag of items picture.
 *
 * The ship picture is what the Stack looks like when it is in a boat.
 * The planted standard is what the player's standard looks like when it has
 * been planted in the ground.  The bag of items picture is what it looks like
 * when a hero has dropped an item on the ground.
 *
 * Armysets are most often referred to by their Id (Armyset::d_id), but may 
 * sometimes be referred to by their name (Armyset::d_name) or basename 
 * name (Armyset::d_basename).
 *
 * Armyset objects are loaded from an armyset configuration file.
 *
 * Armyset objects are created by the armyset editor.
 *
 * Every Player has an Armyset that dictates the characteristics of the
 * player's forces, but in practise there is only one Armyset per scenario.
 *
 * The armyset configuration file is a tar file that contains an XML file, 
 * and a set of png files.  Filenames have the following form:
 * army/${Armyset::d_basename}.lwa.
 */
class Armyset: public std::list<ArmyProto *>, public sigc::trackable, public Set
{
    public:

	//! The xml tag of this object in an armyset configuration file.
	static Glib::ustring d_tag; 
	static Glib::ustring file_extension; 

	//! Default constructor.
	/**
	 * Make a new Armyset.
	 *
	 * @param id    The unique Id of this Armyset among all other Armyset
	 *              objects.  Must be more than 0.  
	 * @param name  The name of the Armyset.  Analagous to Armyset::d_name.
	 */
	Armyset(guint32 id, Glib::ustring name);
	//! Loading constructor.
	/**
	 * Load armyset XML entities from armyset configuration files.
	 */
        Armyset(XML_Helper* helper, Glib::ustring directory);

        //! Copy constructor.
        Armyset(const Armyset& armyset);

	static Armyset *create(Glib::ustring filename, bool &unsupported);

        static Armyset *copy (const Armyset *orig);

	//! Destructor.
        ~Armyset();

	/**
	 * @param helper  An opened armyset configuration file.
	 */
	//! Save the Armyset to an Armyset configuration file.
	bool save(XML_Helper* helper) const;
        
        bool save(Glib::ustring filename, Glib::ustring ext) const;

	//! Get the image of the stack in a ship (minus the mask).
        std::vector<PixMask*> getShipPics() const {return d_ship;}

	//! Sets the basename of the file containing the big selector images for the given player.
	void setLargeSelectorFilename(Shield::Colour c, Glib::ustring p);

	//! Sets the basename of the file containing the small selector images for the given player.
	void setSmallSelectorFilename(Shield::Colour c, Glib::ustring p);

	//! Sets a big selector image for the given player.
	void setSelectorImage(Shield::Colour c, guint32 i, PixMask *p);

	//! Sets a big selector mask for the given player.
	void setSelectorMask(Shield::Colour c, guint32 i, PixMask *p);

	//! Sets a small selector image for the given player.
	void setSmallSelectorImage(Shield::Colour c, guint32 i, PixMask *p);

	//! Sets a small selector mask for the given player.
	void setSmallSelectorMask(Shield::Colour c, guint32 i, PixMask *p);

	//! Sets the number of animation frames in the big selector for the given player.
	void setNumberOfSelectorFrames(Shield::Colour c, guint32 s);

	//! Sets the number of animation frames in the small selector for the given player.
	void setNumberOfSmallSelectorFrames(Shield::Colour c, guint32 s);

        //!Get rid of the small selector image for the given players
        void clearSmallSelectorImage (Shield::Colour c, bool clear_name = true);

        //!Get rid of the large selector image for the given players
        void clearLargeSelectorImage (Shield::Colour c, bool clear_name = true);

        bool instantiateSmallSelectorImages ();
        bool instantiateSmallSelectorImages(Shield::Colour c);
        bool instantiateLargeSelectorImages ();
        bool instantiateLargeSelectorImages(Shield::Colour c);

	//! Set the image of the stack in a ship
	void setShipImages(std::vector<PixMask*> ship) {d_ship = ship;};

	//! Get the mask portion of the image of the stack in a ship.
        std::vector<PixMask*> getShipMasks() const {return d_shipmask;}

	//! Set the mask portion of the image of the stack in a ship.
	void setShipMasks(std::vector<PixMask*> shipmask) {d_shipmask = shipmask;};

        //! Clear the ship name, pic, and mask
        void clearShipImage (bool clear_name = true);

        //! Instantiate the ship image by loading it from the lwa file.
        bool instantiateShipImage ();

	//! Get the image of the bag.
	PixMask* getBagPic() const {return d_bag;}

	//! Set the image of the bag.
	void setBagPic(PixMask* s) {d_bag = s;};

        //! Clear the bag name and pic 
        void clearBagImage (bool clear_name = true);

        //! Instantiate the bag image by loading it from the lwa file.
        bool instantiateBagImage ();

	//! Get the image of the planted standard (minus the mask).
        std::vector<PixMask*> getStandardPics() const {return d_standard;}

	//! Set the image of the planted standard (minus the mask).
	void setStandardPics(std::vector<PixMask*> s) {d_standard = s;};

	//! Get the mask portion of the image of the planted standard.
        std::vector<PixMask*> getStandardMasks() const {return d_standard_mask;}

	//! Set the mask portion of the image of the planted standard.
	void setStandardMasks(std::vector<PixMask*> s) {d_standard_mask = s;};

        //! Clear the standard (hero's flag) name, pic and mask
        void clearStandardImage (bool clear_name = true);
        
        //! Instantiate the standard image by loading it from the lwa file.
        bool instantiateStandardImage ();

	//! Set the name of the file holding the image of the stack in a boat.
	void setShipImageName(Glib::ustring n) {d_stackship_name = n;};

	//! Get the name of the file holding the image of the stack in a boat.
	Glib::ustring getShipImageName() {return d_stackship_name;};

	//! Set the name of the file holding the image of the hero's flag.
	void setStandardImageName(Glib::ustring n) {d_standard_name = n;};

	//! Get the name of the file holding the image of the hero's flag.
	Glib::ustring getStandardImageName() {return d_standard_name;};

	//! Set the name of the file holding the image of the bag.
	void setBagImageName(Glib::ustring n) {d_bag_name = n;};

	//! Get the name of the file holding the image of the bag.
	Glib::ustring getBagImageName() {return d_bag_name;};

        //! Find the type id with the highest value and return it.
        guint32 getMaxId() const;

	//! Returns the basename of the file containing big selector images for the given player.
	Glib::ustring getLargeSelectorFilename(Shield::Colour c) const;

	//! Returns the basename of the file containing small selector images for the given player.
	Glib::ustring getSmallSelectorFilename(Shield::Colour c) const;

	//! Get the big selector image for the given player.  Pass in the index.
	PixMask *getSelectorImage(Shield::Colour c, guint32 i) const;

	//! Get the big selector mask for the given player.  Pass in the index.
	PixMask *getSelectorMask(Shield::Colour c, guint32 i) const;

	//! Get the small selector image for the given player.  Pass in the index.
	PixMask *getSmallSelectorImage(Shield::Colour c, guint32 i) const;

	//! Get the small selector mask for the given player.  Pass in the index.
	PixMask *getSmallSelectorMask(Shield::Colour c, guint32 i) const;

	//! Get the number of animation frames in the big selector image for the given player.
	guint32 getNumberOfSelectorFrames(Shield::Colour c) const;

	//! Get the number of animation frames in the small selector image for the given player.
	guint32 getNumberOfSmallSelectorFrames(Shield::Colour c) const;

	//! Find an army with a type in this armyset.
	/**
	 * Scan the Army prototype objects in this Armyset and return it.
	 *
	 * @note This is only used for the editor.  Most callers should use 
	 * Armysetlist::getArmy instead.
	 *
	 * @param army_type  The army type id of the Army prototype object
	 *                   to search for in this Armyset.
	 *
	 * @return The Army with the given army type id, or NULL if none
	 *         could be found.
	 */
	ArmyProto * lookupArmyByType(guint32 army_type) const;

	ArmyProto * lookupArmyByName(Glib::ustring name) const;

	ArmyProto * lookupArmyByStrengthAndTurns(guint32 str, guint32 turns) const;

	ArmyProto * lookupArmyByGender(Hero::Gender gender) const;

	ArmyProto * lookupSimilarArmy(ArmyProto *army) const;
        
        ArmyProto * lookupWeakestQuickestArmy() const;

	//! can this armyset be used within the game?
	bool validate();
	bool validateHero();
	bool validatePurchasables();
	bool validateRuinDefenders();
	bool validateAwardables();
	bool validateShip();
	bool validateStandard();
	bool validateBag();
	bool validateArmyUnitImages();
	bool validateArmyUnitImage(ArmyProto *a, Shield::Colour &c);
	bool validateArmyUnitNames();
	bool validateArmyUnitName(ArmyProto *a);
	bool validateArmyTypeIds();

        //! Load the images associated with this armyset.
        /**
         * Go get the image files from the armyset file and create the
         * various pixmask objects.
         *
         * @param scale   The images are clamped to the tile size or not.
         * @param broken  True when things went wrong reading the armyset file.
         */
	void instantiateImages(bool scale, bool &broken);
	void uninstantiateImages();
        void uninstantiateSameNamedImages (Glib::ustring name);

	void loadStandardPic(Glib::ustring image_filename, bool scale, bool &broken);
	void loadShipPic(Glib::ustring image_filename, bool scale, bool &broken);
	void loadBagPic(Glib::ustring image_filename, bool &broken);
        bool loadSelectorPics (Tar_Helper *t);

	static void switchArmyset(Army *army, const Armyset *armyset);
	static void switchArmyset(ArmyProdBase *army, const Armyset *armyset);
	static void switchArmysetForRuinKeeper(Army *army, const Armyset *armyset);
	const ArmyProto * getRandomRuinKeeper() const;
	const ArmyProto *getRandomAwardableAlly() const;

        //! Load the armyset again.
        void reload(bool &broken);
        bool calculate_preferred_tile_size(guint32 &ts) const;

        //! callback to upgrade old files.
        static bool upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version);
        static void support_backward_compatibility();

        static guint32 get_default_tile_size ();
    private:

        //! Callback function for the army tag (see XML_Helper)
        bool loadArmyProto(Glib::ustring tag, XML_Helper* helper);

        void read_selector_name (XML_Helper *helper, Shield::Colour c, bool large);
        void write_selector_name (XML_Helper *helper, Shield::Colour c, bool large) const;
        void clear_vectors ();
        
	//! The unshaded picture of the stack when it's in a boat.
        std::vector<PixMask*> d_ship;

	//! The mask of what to shade with the player's colour on the boat.
        std::vector<PixMask*> d_shipmask;

	//! The unshaded picture of the planted standard.
        std::vector<PixMask*> d_standard;

	//! The mask of what to shade with the player's colour on the standard.
        std::vector<PixMask*> d_standard_mask;

	//! The picture of an item when it's lying on the ground.
	PixMask *d_bag;

	//! The name of the file that holds the picture of the hero's flag.
	Glib::ustring d_standard_name;

	//! The name of the file that holds the picture of stack on water.
	Glib::ustring d_stackship_name;

	//! The name of the file that holds the picture of the sack of items.
	Glib::ustring d_bag_name;

	//! The basename of the small selector image, white player.
	Glib::ustring d_small_white_selector;

	//! The basename of the small selector image, green player.
	Glib::ustring d_small_green_selector;

	//! The basename of the small selector image, yellow player.
	Glib::ustring d_small_yellow_selector;

	//! The basename of the small selector image, light blue player.
	Glib::ustring d_small_light_blue_selector;

	//! The basename of the small selector image, orange player.
	Glib::ustring d_small_orange_selector;

	//! The basename of the small selector image, dark blue player.
	Glib::ustring d_small_dark_blue_selector;

	//! The basename of the small selector image, red player.
	Glib::ustring d_small_red_selector;

	//! The basename of the small selector image, black player.
	Glib::ustring d_small_black_selector;

	//! the basename of the large selector image, white player.
	Glib::ustring d_large_white_selector;

	//! The basename of the large selector image, green player.
	Glib::ustring d_large_green_selector;

	//! The basename of the large selector image, yellow player.
	Glib::ustring d_large_yellow_selector;

	//! The basename of the large selector image, light blue player.
	Glib::ustring d_large_light_blue_selector;

	//! The basename of the large selector image, orange player.
	Glib::ustring d_large_orange_selector;

	//! The basename of the large selector image, dark blue player.
	Glib::ustring d_large_dark_blue_selector;

	//! The basename of the large selector image, red player.
	Glib::ustring d_large_red_selector;

	//! The basename of the large selector image, black player.
	Glib::ustring d_large_black_selector;

        //! The number of animation frames in the big selector, white player.
        guint32 number_of_white_selector_frames;

        //! The number of animation frames in the big selector, green player.
        guint32 number_of_green_selector_frames;

        //! The number of animation frames in the big selector, yellow player.
        guint32 number_of_yellow_selector_frames;

        //! The number of animation frames in the big selector, light blue player.
        guint32 number_of_light_blue_selector_frames;

        //! The number of animation frames in the big selector, orange player.
        guint32 number_of_orange_selector_frames;

        //! The number of animation frames in the big selector, dark blue player.
        guint32 number_of_dark_blue_selector_frames;

        //! The number of animation frames in the big selector, red player.
        guint32 number_of_red_selector_frames;

        //! The number of animation frames in the big selector, black player.
        guint32 number_of_black_selector_frames;

        //! The image frames in the big selector, white player.
        std::vector<PixMask* > white_selector;

        //! The image frames in the big selector, green player.
        std::vector<PixMask* > green_selector;

        //! The image frames in the big selector, yellow player.
        std::vector<PixMask* > yellow_selector;

        //! The image frames in the big selector, light blue player.
        std::vector<PixMask* > light_blue_selector;

        //! The image frames in the big selector, orange player.
        std::vector<PixMask* > orange_selector;

        //! The image frames in the big selector, dark blue player.
        std::vector<PixMask* > dark_blue_selector;

        //! The image frames in the big selector, red player.
        std::vector<PixMask* > red_selector;

        //! The image frames in the big selector, black player.
        std::vector<PixMask* > black_selector;

        //! The mask frames of the big selector, white player.
        std::vector<PixMask* > white_selectormask;

        //! The mask frames of the big selector, green player.
        std::vector<PixMask* > green_selectormask;

        //! The mask frames of the big selector, yellow player.
        std::vector<PixMask* > yellow_selectormask;

        //! The mask frames of the big selector, light blue player.
        std::vector<PixMask* > light_blue_selectormask;

        //! The mask frames of the big selector, orange player.
        std::vector<PixMask* > orange_selectormask;

        //! The mask frames of the big selector, dark blue player.
        std::vector<PixMask* > dark_blue_selectormask;

        //! The mask frames of the big selector, red player.
        std::vector<PixMask* > red_selectormask;

        //! The mask frames of the big selector, black player.
        std::vector<PixMask* > black_selectormask;

        //! The number of animation frames in the small selector, white player.
        guint32 number_of_white_small_selector_frames;

        //! The number of animation frames in the small selector, green player.
        guint32 number_of_green_small_selector_frames;

        //! The number of animation frames in the small selector, yellow player.
        guint32 number_of_yellow_small_selector_frames;

        //! The number of animation frames in the small selector, light blue player.
        guint32 number_of_light_blue_small_selector_frames;

        //! The number of animation frames in the small selector, orange player.
        guint32 number_of_orange_small_selector_frames;

        //! The number of animation frames in the small selector, dark blue player.
        guint32 number_of_dark_blue_small_selector_frames;

        //! The number of animation frames in the small selector, red player.
        guint32 number_of_red_small_selector_frames;

        //! The number of animation frames in the small selector, black player.
        guint32 number_of_black_small_selector_frames;

        //! The image frames of the small selector, white player.
        std::vector<PixMask* > white_smallselector;

        //! The image frames of the small selector, green player.
        std::vector<PixMask* > green_smallselector;

        //! The image frames of the small selector, yellow player.
        std::vector<PixMask* > yellow_smallselector;

        //! The image frames of the small selector, light blue player.
        std::vector<PixMask* > light_blue_smallselector;

        //! The image frames of the small selector, orange player.
        std::vector<PixMask* > orange_smallselector;

        //! The image frames of the small selector, dark blue player.
        std::vector<PixMask* > dark_blue_smallselector;

        //! The image frames of the small selector, red player.
        std::vector<PixMask* > red_smallselector;

        //! The image frames of the small selector, black player.
        std::vector<PixMask* > black_smallselector;

        //! The mask frames of the small selector, white player.
        std::vector<PixMask* > white_smallselectormask;

        //! The mask frames of the small selector, green player.
        std::vector<PixMask* > green_smallselectormask;

        //! The mask frames of the small selector, yellow player.
        std::vector<PixMask* > yellow_smallselectormask;

        //! The mask frames of the small selector, light blue player.
        std::vector<PixMask* > light_blue_smallselectormask;

        //! The mask frames of the small selector, orange player.
        std::vector<PixMask* > orange_smallselectormask;

        //! The mask frames of the small selector, dark blue player.
        std::vector<PixMask* > dark_blue_smallselectormask;

        //! The mask frames of the small selector, red player.
        std::vector<PixMask* > red_smallselectormask;

        //! The mask frames of the small selector, black player.
        std::vector<PixMask* > black_smallselectormask;
};

bool weakest_quickest (const ArmyProto* first, const ArmyProto* second);
#endif // ARMYSET_H

