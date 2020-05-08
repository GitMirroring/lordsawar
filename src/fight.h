// Copyright (C) 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004 Bryan Duff
// Copyright (C) 2006 Andrea Paternesi
// Copyright (C) 2007, 2008, 2011, 2014, 2020 Ben Asselstine
// Copyright (C) 2008 Ole Laursen
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
#ifndef FIGHT_H
#define FIGHT_H

#include <gtkmm.h>
#include <list>
#include <vector>
#include <map>
#include "vector.h"
#include "LocationBox.h"
#include "Tile.h"

class Stack;
class Fighter;
class Hero;
class Army;
class Maptile;

//! A description of a round of casualties during a Fight.
/** 
 * This is the structure that describes the events of the fight.  It is 
 * played back by a fight dialog to reconstruct and show what transpired.
 */
struct FightItem
{
  //! The round number of the battle.
  int turn;
  //! The id of the army who was attacked in this event.
  guint32 id;
  //! The amount of damage that the army sustained.
  int damage;
};


//! Calculate the outcome of a battle.
/** 
 * This class is solely responsible for the _calculation_ of the fight.
 * It gets the participating stacks and damages the units within according
 * to the calculation. Furthermore, it creates a history of the fight, which
 * can later be used by the fight dialog to reconstruct the fight or be sent
 * over the network. For the graphical display, see the FightDialog class.
 *
 * Two things should be noted. First, the fight can include more than the
 * initial two stacks, since all stacks around the defender are considered
 * as potential "contributors". Second, irrespective of that, a fight is
 * always considered as "won", if the defending stack was destroyed and
 * "lost" if the attacking stack was crushed.
 */
class Fight
{
    public:
        //! The three possibilities how a fight can end
        enum Result {
	  //! There was no winner.
	  /**
	   * Although it is in the enumeration, every fight should always
	   * have a winner.  No draws allowed because MAX_ROUNDS is 0.
	   */
	  DRAW = 0,

	  //! The attacking list of stacks won the battle.
	  ATTACKER_WON = 1,

	  //! The defending list of stacks won the battle.
	  DEFENDER_WON = 2
	};

	//! The kind of fight.  Whether the outcome is realized or not.
        enum FightType {
	  //! The fight doesn't mean anything, it's just to see who would win.
	  /**
	   * @note This value is used to assist in the implementation of the
	   *       `Miltary Advisor' feature.
	   */
	  FOR_KICKS = 0,

	  //! The fight is real.  If an army dies, it stays dead.
	  FOR_KEEPS = 1
	};

	//! Make a new fight between two lists of stacks.
        /**
         * @param attacker         The list of attacking stacks.
         * @param defender         The list of defending stacks
	 * @param type             Optionally heal all stacks afterwards.
         */
        Fight(Stack* attacker, Stack* defender, FightType type = FOR_KEEPS);

        // construct from serialized action
        Fight(const std::list<Stack*> &attackers,
              const std::list<Stack*> &defenders,
              const std::list<FightItem> &history);

        // construct for the battle calculator
        Fight(const std::list<Stack*> &attackers, const std::list<Stack*> &defenders, bool city, Tile::Type terrain, FightType type = FOR_KEEPS);

	//! Destructor.
        ~Fight();

        //! Determine the outcome of the fight.
	/**
	 * This method fills out a set of FightItem events in d_actions.
	 *
	 * @param intense   Whether or not to Use 24 sided dice instead of 
	 *                  20 sided dice.  Makes battles harder to win when 
	 *                  set to True.
	 */
        void battle(bool intense);

        Result battleFromHistory();

        //! Returns the result of the fight.
        Result getResult() const {return d_result;}

        //! Returns the list of things that happened in chronological order.
        std::list<FightItem> getCourseOfEvents() const {return d_actions;};

        //! Returns the participating attacker stacks.
        std::list<Stack*> getAttackers() const {return d_attackers;}

        //! Returns the participating defender stacks.
        std::list<Stack*> getDefenders() const {return d_defenders;}

        //! Returns the participating attacker armies.
        std::list<Fighter*> getAttackerFighters() const { return d_initial_att_close;};
        //! Returns the participating defender armies.
        std::list<Fighter*> getDefenderFighters() const { return d_initial_def_close;};
	//! Get the modified strength bonus of the given Army unit.
	guint32 getModifiedStrengthBonus(Army *a);

        // CONSTANTS
        //! The number of rounds the fight lasts.
	/**
	 * @note If this is 0, then there is no maximum.
	 */
        static const int MAX_ROUNDS = 0;

	//! Turn a list of stacks into an ordered list of armies.
	/**
	 * @note This is used for calculation and display purposes.
	 */
        static void orderArmies(const std::list<Stack*> &stacks,
				std::vector<Army*> &armies);

        std::map<guint32, guint32> getInitialHPs() { return initial_hps; }

	static LocationBox calculateFightBox(Fight &fight);

        Glib::ustring getStrongestLivingHeroName(std::vector<Army *> s) const;
    private:
	//! Calculates one round of the fight.
        /** 
         * @return false if the maximum number of fight rounds has been
         * exceeded or one side has lost.
         */
        bool doRound();

        //! Calculates the attack/defense bonus of the armies.
        void calculateBonus(Maptile *mtile);

	//! Calculates the base strength of the armies fighting in the battle.
        void calculateBaseStrength(std::list<Fighter*> fighters);

	//! Add the bonuses provided by terrain.
        void calculateTerrainModifiers(std::list<Fighter*> fighters, Maptile *mtile, bool defender);

	//! Add the bonuses by opponents.
        void calculateModifiedStrengths (std::list<Fighter*>friendly,
                                         std::list<Fighter*>enemy,
                                         bool friendlyIsDefending,
                                         Hero *strongestHero,
                                         Maptile *mtile);

	//! Subtract stack bonuses of the opponent.
        void calculateFinalStrengths (std::list<Fighter*> friendly,
				      std::list<Fighter*> enemy);

        /** 
	 * This function just has two armies fight against each other. It
         * applies the bonuses and several special bonuses to attacker and 
	 * defender and calculates the result.
         *
         * @param attacker     The attacking army.
         * @param defender     The defending army.
         */
        void fightArmies(Fighter* attacker, Fighter* defender);

        //! Removes an army from the fight.
        void remove(Fighter* f);

        void fillInInitialHPs();

        void setupFight(const std::list<Stack*> &attackers, const std::list<Stack*> &defenders, bool city, Tile::Type terrain, FightType type);

        Army *findArmyById(const std::list<Stack *> &l, guint32 id);

        // DATA

	//! The attackers.
        std::list<Stack*> d_attackers;

	//! The defenders.
        std::list<Stack*> d_defenders;

	//!The attackers in the fight, afterwards.
        std::list<Fighter*> d_att_close;
	//!The attackers in the fight, beforehand.
        std::list<Fighter*> d_initial_att_close;

	//! The defenders in the fight, afterwards.
        std::list<Fighter*> d_def_close;
	//! The defenders in the fight, beforehand.
        std::list<Fighter*> d_initial_def_close;

        std::map<guint32, guint32> initial_hps;

	//! The list of fight events that gets calculated.
        std::list<FightItem> d_actions;

	//! The round of the fight.
        int d_turn;

	//! The result of the fight.
        Result d_result;

	//! The kind of fight.
	FightType d_type;

	//! Whether or not we're rolling 24-sided dice or 20 sided dice.
	bool d_intense_combat;
};

// Helper class; the single units participating in the fight are saved with
// additional information. This should be a struct, but I don't know how to
// forward the declaration properly.
//! A particpant in a Fight.
class Fighter
{
    public:
        Fighter(Army* a, Vector<int> p);
        Fighter(const Fighter &f);

        Army* army;
        Vector<int> pos;       // location on the map (needed to calculate boni)
        int terrain_strength;
	//! needed for sorting
        //bool operator() ( const Fighter* f1, const Fighter* f2 );

};


#endif // FIGHT_H

