//  Copyright (C) 2017, 2021 Ben Asselstine
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
#ifndef STONE_H
#define STONE_H

#include "Location.h"
#include "road.h"

//! A single tile on the map that has a standing stone on it.
/**
 * Standing stones are purely cosmetic.  They can only be on grassy tiles
 * without buildings, except stones.
 */
class Stone: public Location
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

        enum Type {
          ROAD_E_AND_W_STONE_N = 0,
          ROAD_E_AND_W_STONE_S = 1,
          ROAD_E_AND_W_STONES_N_AND_S = 2,
          ROAD_N_AND_S_STONE_W = 3,
          ROAD_N_AND_S_STONE_E = 4,
          ROAD_N_AND_S_STONES_W_AND_E = 5,
          ROAD_ALL_DIRECTIONS_STONE_NW = 6,
          ROAD_ALL_DIRECTIONS_STONE_NE = 7,
          ROAD_ALL_DIRECTIONS_STONE_SW = 8,
          ROAD_ALL_DIRECTIONS_STONE_SE = 9,
          ROAD_ALL_DIRECTIONS_STONES_NW_NE = 10,
          ROAD_ALL_DIRECTIONS_STONES_SW_SE = 11,
          ROAD_ALL_DIRECTIONS_STONES_NW_SW = 12,
          ROAD_ALL_DIRECTIONS_STONES_NE_SE = 13,
          ROAD_ALL_DIRECTIONS_STONES_NW_SE = 14,
          ROAD_ALL_DIRECTIONS_STONES_NE_SW = 15,
          ROAD_ALL_DIRECTIONS_STONES_NE_SW_SE = 16,
          ROAD_ALL_DIRECTIONS_STONES_NW_SW_SE = 17,
          ROAD_ALL_DIRECTIONS_STONES_NW_NE_SE = 18,
          ROAD_ALL_DIRECTIONS_STONES_NW_NE_SW = 19,
          ROAD_ALL_DIRECTIONS_STONES_NW_NE_SW_SE = 20,
          ROAD_N_AND_W_STONE_NW = 21,
          ROAD_N_AND_W_STONE_SE = 22,
          ROAD_N_AND_W_STONES_NW_SE = 23,
          ROAD_N_AND_E_STONE_NE = 24,
          ROAD_N_AND_E_STONE_SW = 25,
          ROAD_N_AND_E_STONES_NE_SW = 26,
          ROAD_S_AND_E_STONE_NW = 27,
          ROAD_S_AND_E_STONE_SE = 28,
          ROAD_S_AND_E_STONES_NW_SE = 29,
          ROAD_W_AND_S_STONE_SW = 30,
          ROAD_W_AND_S_STONE_NE = 31,
          ROAD_W_AND_S_STONES_SW_NE = 32,
          ROAD_N_AND_S_AND_E_STONE_NE = 33,
          ROAD_N_AND_S_AND_E_STONE_SE = 34,
          ROAD_N_AND_S_AND_E_STONE_W = 35,
          ROAD_N_AND_S_AND_E_STONES_SE_W = 36,
          ROAD_N_AND_S_AND_E_STONES_NE_W = 37,
          ROAD_N_AND_S_AND_E_STONES_NE_SE = 38,
          ROAD_N_AND_S_AND_E_STONES_NE_SE_W = 39,
          ROAD_E_W_AND_N_STONE_NW = 40,
          ROAD_E_W_AND_N_STONE_NE = 41,
          ROAD_E_W_AND_N_STONE_S = 42,
          ROAD_E_W_AND_N_STONES_NE_S = 43,
          ROAD_E_W_AND_N_STONES_NW_S = 44,
          ROAD_E_W_AND_N_STONES_NW_NE = 45,
          ROAD_E_W_AND_N_STONES_NW_NE_S = 46,
          ROAD_E_W_AND_S_STONE_SW = 47,
          ROAD_E_W_AND_S_STONE_SE = 48,
          ROAD_E_W_AND_S_STONE_N = 49,
          ROAD_E_W_AND_S_STONES_SE_N = 50,
          ROAD_E_W_AND_S_STONES_SW_N = 51,
          ROAD_E_W_AND_S_STONES_SW_SE = 52,
          ROAD_E_W_AND_S_STONES_SW_SE_N = 53,
          ROAD_N_S_AND_W_STONE_NW = 54,
          ROAD_N_S_AND_W_STONE_SW = 55,
          ROAD_N_S_AND_W_STONE_E = 56,
          ROAD_N_S_AND_W_STONES_SW_E = 57,
          ROAD_N_S_AND_W_STONES_NW_E = 58,
          ROAD_N_S_AND_W_STONES_NW_SW = 59,
          ROAD_N_S_AND_W_STONES_NW_SW_E = 60,
          ROAD_W_STONE_N = 61,
          ROAD_W_STONE_S = 62,
          ROAD_W_STONE_E = 63,
          ROAD_W_STONES_S_E = 64,
          ROAD_W_STONES_N_E = 65,
          ROAD_W_STONES_N_S = 66,
          ROAD_W_STONES_N_S_E = 67,
          ROAD_N_STONE_W = 68,
          ROAD_N_STONE_E = 69,
          ROAD_N_STONE_S = 70,
          ROAD_N_STONES_E_S = 71,
          ROAD_N_STONES_W_S = 72,
          ROAD_N_STONES_W_E = 73,
          ROAD_N_STONES_W_E_S = 74,
          ROAD_E_STONE_N = 75,
          ROAD_E_STONE_S = 76,
          ROAD_E_STONE_W = 77,
          ROAD_E_STONES_S_W = 78,
          ROAD_E_STONES_N_W = 79,
          ROAD_E_STONES_N_S = 80,
          ROAD_E_STONES_N_S_W = 81,
          ROAD_S_STONE_W = 82 ,
          ROAD_S_STONE_E = 83,
          ROAD_S_STONE_N = 84,
          ROAD_S_STONES_E_W = 85,
          ROAD_S_STONES_N_W = 86,
          ROAD_S_STONES_N_E = 87,
          ROAD_S_STONES_N_E_W = 88,
        };

	//! Default constructor.
        /**
          * @param pos          The location of the stone.
          * @param type 	The type of stone.
          */
        Stone(Vector<int> pos, int type = ROAD_E_AND_W_STONE_N);

	//! Copy constructor.
        Stone(const Stone&, bool sync_id = false);

	//! Alternative copy constructor that changes the stone's position.
        Stone(const Stone&, Vector<int> pos);

        //! Loading constructor.
	/**
	 * Make a new stone object by reading lordsawar.stonelist.stone XML 
	 * entities from the saved-game file.
	 *
	 * @param helper  The opened saved-game file to load the stone from.
	 */
        Stone(XML_Helper* helper);

	//! Destructor.
        ~Stone() {};


	// Get Methods

        //! Returns the type of the stone.
        int getType() const {return d_type;};

	// Set Methods

        //! Sets the type of the stone.
        void setType(int type) {d_type = type;};


	// Methods that operate on class data but do not modify the class

        //! Save the stone data to an opened saved-game file.
        bool save(XML_Helper* helper) const;


	// Static Methods

	//! Convert a Stone::Type enumerated value to a string.
	static Glib::ustring stoneTypeToString(const Stone::Type type);

	//! Convert a string containing a Stone::Type to it's enumerated value.
	static Stone::Type stoneTypeFromString(const Glib::ustring str);

        //! Returns a random suitable stone type for a given road type.
        static int getRandomType (const Road::Type type);

        //! Returns a random suitable stone type for the field.
        static int getRandomType ();

        //! Returns which stone types are suitable for a given road type.
        static std::vector<Stone::Type> getSuitableTypes (const Road::Type type);
        //! Returns which stone types are suitable for being in the field.
        static std::vector<Stone::Type> getSuitableTypes ();

        static std::vector<Stone::Type> getTypes ();
    protected:

	// DATA

	//! The type of the stone.
	/**
	 * The type of stone refers to the look of the stone on the map.  It 
	 * can be any one of the values found in Stone::Type.
	 *
	 */
	int d_type;

};

#endif // STONE_H
