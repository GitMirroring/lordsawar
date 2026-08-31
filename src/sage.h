// Copyright (C) 2009, 2014, 2017 Ben Asselstine
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
#ifndef SAGE_H
#define SAGE_H

#include <gtkmm.h>
#include <list>
#include "reward.h"

//! A sage is a provider of rewards.
/** 
 * 
 * Sages provide rewards but with the following limits:
 * They do not provide items or allies, they provide a location of where they
 * can be found.
 * They can also provide gold and maps (if we're doing that) immediately.
 *
 * The rewardlist already has hidden ruins in it, thanks to GameScenario.
 * If there aren't any, it means we don't have any hidden ruins (left) to 
 * give.  hidden ruins are ruins that are only visible to a single player.
 *
 * The hidden ruins don't normally have a reward set, unless it was set
 * in the editor.
 *
 * We don't want populate a hidden ruin with a reward unless it is the
 * reward that the player has selected.
 *
 * We want to provide a single site that has allies, up to two sites that
 * have items, a gold reward, and a map if we're doing that.
 * Not all of these will always be available, but in the worst scenario we
 * can always offer a gold reward.
 *
 */

class Sage: public std::list<Reward*>
{
    public:

	//! Creates a new Sage from scratch.
        Sage();

        //! Destructor.
        ~Sage();

        Reward *getSelectedReward() const {return d_reward;};
        void selectReward(Reward *r) {d_reward = r;};

    private:

        //! The selected reward.
        Reward *d_reward;

        //! The gold reward that we always create.
        Reward_Gold *d_gold_reward;

        //! The allies reward we put into an empty ruin.
        Reward_Allies *d_allies_reward;

        //! The map reward that we make if we're doing that.
        Reward_Map *d_map_reward;

        //! The item reward we put into an empty ruin.
        Reward_Item *d_item_reward;

        //! The empty ruin we put the allies in.
        Reward_Ruin *d_allies_ruin;

        //! The empty ruin we put the item in.
        Reward_Ruin *d_item_ruin;

        bool d_item_ruin_popped;
        bool d_allies_ruin_popped;
};

#endif //SAGE_H
