//  Copyright (C) 2008, 2014 Ben Asselstine
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
#ifndef TRIUMPHS_H
#define TRIUMPHS_H

class XML_Helper;

#include "player.h"

//! Tallies of the kinds of army units that have been killed.
/**
 * 
 */
class Triumphs
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

	//! Every player keeps a tally of frags.
        enum TriumphType {
	  //! Kills we've made of an opponent's Hero army units.
	  TALLY_HERO = 0, 
	  //! Kills we've made of an opponent's awardable Army units.
	  TALLY_SPECIAL = 1, 
	  //! Kills we've made of an opponents other Army units.
	  TALLY_NORMAL = 2, 
	  //! Kills we've made of an opponent's Army units on the water.
	  TALLY_SHIP = 3, 
	  //! Kills we've made of opponent's Heroes who carry a standard Item.
	  TALLY_FLAG = 4
	};

        //! Standard constructor.
        Triumphs();

	//! Loading constructor.
	/**
	 * Load the triumph tallies from a file.
	 * Triumphs are stored in the saved-game file at:
	 * lordsawar.playerlist.player.triumphs.
	 *
	 * @param helper  The opened saved-game file to load the tallies from.
	 */
        Triumphs (XML_Helper* helper);

	//! Copy constructor.
	Triumphs(const Triumphs&);

	//! Destructor.
        ~Triumphs() {};


	// Methods that operate on the class data but do not modify the class.

	//! Save the triumph tallies to a file.
        /**
         * @param helper  The opened saved-game file to save the tallies to.
	 *
         * @return True if saving went well, false otherwise.
         */
        bool save(XML_Helper* helper) const;

	/**
	 * The player's triumphs are tallied as opponent's armies die.
	 * This method gets a tally for certain kind of triumph.  
	 * See TriumphsDialog for a caller of this method.
	 *
	 * @param player      The player to obtain a tally for.
	 * @param type        The kind of kills to tally (Player::TriumphType).
	 *
	 * @return Zero or more number of armies killed.
	 */
	//! Returns a number of armies killed.
	guint32 getTriumphTally(Player *player, TriumphType type) const
	  {return d_triumph[player->getId()][type];}


	// Methods the operate on the class data, and modify the class.
	
	//! Tally up a kill for the given player.
	void tallyTriumph(Player *p, TriumphType type);

    private:

	//! A set of tally statistics for frags of army units.
	/**
	 * 5 is max TriumphType + 1.
	 */ 
	guint32 d_triumph[MAX_PLAYERS][5]; 
};

#endif

// End of file
