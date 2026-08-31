//  Copyright (C) 2026 Ben Asselstine
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
#ifndef FIGHT_RESULT_H
#define FIGHT_RESULT_H

#include "army.h"
#include "hero.h"

//! The result of a fight
/*
 * this is used to see who won, and to track who gets medals and levels up.
 */
class FightResult
{
    public:

        //! The three possibilities how a fight can end
        enum Outcome
          {
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

        FightResult ()
          {
          }

        ~FightResult()
          {
          }

        FightResult::Outcome get_outcome () const
          {
            return m_fight_outcome;
          }

        std::list<Army*> get_medalists (int medal_kind) const
          {
            switch (medal_kind)
              {
              case 0:
                return m_merciless_medalists;
              case 1:
                return m_defender_medalists;
              case 2:
                return m_veteran_medalists;
              }
            std::list<Army*> empty = {};
            return empty;
          }

        std::list<Hero*> get_advancing_heroes () const
          {
            return m_adv_heroes;
          }

        void set_outcome (FightResult::Outcome outcome)
          {
            m_fight_outcome = outcome;
          }

        void add_medalist (Army *a, int medal_kind)
          {
            switch (medal_kind)
              {
              case 0:
                m_merciless_medalists.push_back (a);
                break;
              case 1:
                m_defender_medalists.push_back (a);
                break;
              case 2:
                m_veteran_medalists.push_back (a);
                break;
              }
          }

        void add_advancing_hero (Hero *h)
          {
            m_adv_heroes.push_back (h);
          }

    private:
        FightResult::Outcome m_fight_outcome;
        std::list<Army*> m_merciless_medalists;
        std::list<Army*> m_defender_medalists;
        std::list<Army*> m_veteran_medalists;
        std::list<Hero*> m_adv_heroes;
};

#endif
