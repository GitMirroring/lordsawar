// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2014, 2015, 2020 Ben Asselstine
// Copyright (C) 2007, 2008 Ole Laursen
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
#ifndef ARMY_PROTO_H
#define ARMY_PROTO_H

#include <gtkmm.h>
#include "PixMask.h"
#include "shield.h"
#include "armyprotobase.h"
#include "hero.h"

class XML_Helper;
class Tar_Helper;
class TarFileMaskedImage;

//! An army prototype object.  Appears in an armyset file.
class ArmyProto : public ArmyProtoBase
{
    public:
	//! The xml tag of this object in an armyset configuration file.
	static Glib::ustring d_tag; 

	//! Copy constructor.
        ArmyProto(const ArmyProto& armyproto);

	//! Loading constructor.
        ArmyProto(XML_Helper* helper);
        
	//! Default constructor.  Create an empty army prototype.
	ArmyProto();

	//! Destructor.
        ~ArmyProto();


        // Set Methods
        
        //! Sets the Type Id of the Army.
        void setId(guint32 id) {d_id = id;};

	//! Sets whether or not this Army prototype can found in a ruin.
	void setDefendsRuins(bool defends) {d_defends_ruins = defends; }

	/**
	 * Sets whether or not this Army prototype can be a reward for
	 * Quest, or if Army units of this kind can accompany a new
	 * Hero when one emerges in a City.
	 */
	//! Sets the awardable state of an Army prototype.
	void setAwardable (bool awardable) {d_awardable = awardable; }

	//! Sets the gender of the army prototype.
	void setGender(Hero::Gender g) {d_gender = g;};
	

        // Get Methods
        
        //! Returns the Type Id of this Army prototype.
        guint32 getId() const {return d_id;};

	//! Returns the army's masked image object
        TarFileMaskedImage *getMaskedImage (Shield::Color c) const
          {return d_mimage[c];}

	//! Gets whether or not this army type can found in a ruin.
	bool getDefendsRuins() const {return d_defends_ruins; }

	/**
	 * Gets whether or not this army can be a reward for completing a 
	 * Quest, or if an Army unit of this type can accompany a new
	 * Hero when one emerges in a City.
	 */
	//! Gets the awardable state of the Army.
	bool getAwardable() const {return d_awardable; }

	//! Returns whether or not the Army prototype is a Hero.
	bool isHero() const {return d_gender != Hero::NONE;};
	
	//! Returns the gender of the army prototype.
	Hero::Gender getGender() const {return d_gender;};

	// Methods that operate on class data and modify the class.

	//! Load the pictures associated with this ArmyProto object.
	void instantiateImages(guint32 tilesize, Tar_Helper *t, bool scale,
                               bool &broken);

        //! Instantiate the image for the given color from the lwa file.
        bool instantiateImage (Glib::ustring cfgfile, Shield::Color col);

	//! Destroy the images associated with this ArmyProto object.
	void uninstantiateImages();

	// Methods that operate on class data and do not modify the class.

        //! Saves the Army prototype to an opened armyset file.
        virtual bool save(XML_Helper* helper) const;
        
	// Static Methods
	
	//! Create an ArmyProto object that can walk well in hills and forest.
	static ArmyProto * createScout();

	//! Create an ArmyProto object that can fly.
	static ArmyProto * createBat();

    protected:

	//! Callback to read this object from an opened file.
	bool saveData(XML_Helper* helper) const;

    private:

        //! The Type Id of this Army prototype.
        guint32 d_id;

	//! Whether or not the Army prototype can defend a Ruin.
	/**
	 * Some Army unit can be the guardian of a Ruin.  Hero units fight
	 * a single Army unit of this kind when they search a Ruin.  
	 * d_defends_ruin indicates whether this Army unit can defend a Ruin 
	 * or not.
	 *
	 * This value does not change during gameplay.
	 */
	bool d_defends_ruins;

	//! The awardable status of the Army prototype.
	/**
	 * Whether or not this Army prototype can be a reward for a Quest, 
	 * or if Army units of this kind can accompany a new Hero when one 
	 * emerges in a City.
	 *
	 * This value does not change during gameplay.
	 */
	bool d_awardable;

	//! The basename of the file containing the image for this Army proto.
	/**
	 * This value does not contain a path, and does not contain an
	 * extension (e.g. .png).
	 *
	 * There is an image filename for each player, plus the neutral player.
	 */
	Glib::ustring d_image_name[MAX_PLAYERS + 1];

	//! The gender of this object.
	/**
	 * Heroes have genders, and regular armies do not.
	 */
	Hero::Gender d_gender;

        //! a set of masked images, one per player plus neutral.
        TarFileMaskedImage * d_mimage[MAX_PLAYERS + 1];
};

#endif // ARMY_PROTO_H
