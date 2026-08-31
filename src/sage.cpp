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

#include "Sage.h"
#include "rewardlist.h"
#include "rnd.h"
#include "GameScenarioOptions.h"

Sage::Sage()
{
  d_reward = NULL;
  d_gold_reward = NULL;
  d_allies_reward = NULL;
  d_map_reward = NULL;
  d_item_reward = NULL;
  d_allies_ruin = NULL;
  d_allies_ruin_popped = false;
  d_item_ruin = NULL;
  d_item_ruin_popped = false;
  std::vector<Reward_Ruin *> allies_ruins;
  std::vector<Reward_Ruin *> item_ruins;
  std::vector<Reward_Ruin *> empty_ruins;
  std::vector<Reward_Ruin *> more_empty_ruins;
  std::vector<Reward_Ruin *> other_ruins;

  //collect up the hidden ruins that have items, allies, and are just empty.
  while (1)
    {
      Reward_Ruin *r = 
        dynamic_cast<Reward_Ruin*>(Rewardlist::getInstance()->pop(Reward::RUIN));
      if (!r)
        break;
      if (r->getRuin()->isHidden() == false)
        continue;
      if (r->getRuin()->isSearched() == true)
        continue;
      if (r->getRuin()->getReward())
        {
          switch (r->getRuin()->getReward()->getType())
            {
            case Reward::ITEM:
              item_ruins.push_back(r);
              break;
            case Reward::ALLIES:
              allies_ruins.push_back(r);
              break;
            default:
              other_ruins.push_back(r);
              break;
            }
        }
      else
        empty_ruins.push_back (r);
    }

  //definitely put the ones back we don't care about
  for (auto o : other_ruins)
    Rewardlist::getInstance()->push_back(o);

  std::random_shuffle(empty_ruins.begin(), empty_ruins.end());
  std::random_shuffle(item_ruins.begin(), item_ruins.end());
  std::random_shuffle(allies_ruins.begin(), allies_ruins.end());

  if (allies_ruins.size())
    {
      Reward_Ruin *r = allies_ruins.back();
      allies_ruins.pop_back();
      //put the rest back
      for (auto o : allies_ruins)
        Rewardlist::getInstance()->push_back(o);
      push_back(r);
    }
  else if (empty_ruins.size())
    {
      d_allies_reward =
        dynamic_cast<Reward_Allies*>(Reward_Allies::createRandomReward());
      if (d_allies_reward)
        {
          d_allies_ruin = empty_ruins.back();
          d_allies_ruin_popped = true;
          empty_ruins.pop_back();
          d_allies_ruin->getRuin()->setReward(d_allies_reward);
          push_back(d_allies_ruin);
        }
    }

  if (item_ruins.size())
    {
      Reward_Ruin *r = item_ruins.back();
      item_ruins.pop_back();
      //put the rest back
      for (auto o : item_ruins)
        Rewardlist::getInstance()->push_back(o);
      push_back(r);
    }
  else if (empty_ruins.size())
    {
      d_item_reward = 
        dynamic_cast<Reward_Item*>(Rewardlist::getInstance()->pop(Reward::ITEM));
      if (d_item_reward)
        {
          d_item_ruin = empty_ruins.back();
          d_item_ruin_popped = true;
          empty_ruins.pop_back();
          d_item_ruin->getRuin()->setReward(d_item_reward);
          push_back (d_item_ruin);
        }
    }
  //put the rest of the empty ruins back
  for (auto o: empty_ruins)
    Rewardlist::getInstance()->push_back(o);

  //okay, we've handled the custom scenario case,
  //now we handle the more normal case
  for (auto r: *Ruinlist::getInstance())
    {
      if (r->isSearched() == true)
        continue;
      if (r->isHidden() == false)
        continue;
      if (r->getReward() == NULL)
        more_empty_ruins.push_back(new Reward_Ruin (r));
    }
  std::random_shuffle(more_empty_ruins.begin(), more_empty_ruins.end());
  if (!d_allies_reward && more_empty_ruins.size())
    {
      d_allies_reward =
        dynamic_cast<Reward_Allies*>(Reward_Allies::createRandomReward());
      if (d_allies_reward)
        {
          d_allies_ruin = more_empty_ruins.back();
          more_empty_ruins.pop_back();
          d_allies_ruin->getRuin()->setReward(d_allies_reward);
          push_back(d_allies_ruin);
        }
    }

  if (!d_item_reward && more_empty_ruins.size())
    {
      d_item_reward = 
        dynamic_cast<Reward_Item*>(Rewardlist::getInstance()->pop(Reward::ITEM));
      if (d_item_reward)
        {
          d_item_ruin = more_empty_ruins.back();
          more_empty_ruins.pop_back();
          d_item_ruin->getRuin()->setReward(d_item_reward);
          push_back (d_item_ruin);
        }
    }
  for (auto r : more_empty_ruins)
    delete r;

  //and now come the rewards that aren't associated with a ruin:
  //i.e. instead of being pointed to a place where there is a reward,
  //we just give these rewards right now.

  //there's always money in the banana stand.
  d_gold_reward = new Reward_Gold(Reward_Gold::getRandomSageGoldPieces());
  push_back(d_gold_reward);

  //there may or may not be a map, depending on if we're playing hidden map.
  if (GameScenarioOptions::s_hidden_map)
    {
      d_map_reward =
        dynamic_cast<Reward_Map*>(Reward_Map::createRandomReward());
      if (d_map_reward)
        push_back(d_map_reward);
    }
}

Sage::~Sage ()
{
  //first put back the ones we didn't make, or select
  for (iterator i = begin(); i != end(); ++i)
    {
      if (*i != d_gold_reward  &&
          *i != d_allies_ruin &&
          *i != d_item_ruin &&
          *i != d_map_reward &&
          *i != d_reward)
        Rewardlist::getInstance()->push_back(*i);
    }

  //get rid of the allies reward we made, unless it's selected
  if (d_allies_ruin && d_reward != d_allies_ruin)
    {
      Reward *reward = d_allies_ruin->getRuin()->takeReward();
      delete reward;
      if (d_allies_ruin_popped)
        Rewardlist::getInstance()->push_back(d_allies_ruin);
    }

  if (d_item_ruin && d_reward != d_item_ruin)
    {
      Reward *reward = d_item_ruin->getRuin()->takeReward();
      Rewardlist::getInstance()->push_back(reward);
      if (d_item_ruin_popped)
        Rewardlist::getInstance()->push_back(d_item_ruin);
    }

  if (d_gold_reward && d_reward != d_gold_reward)
    delete d_gold_reward;

  if (d_map_reward && d_reward != d_map_reward)
    delete d_map_reward;
}
