// Copyright (C) 2009, 2014, 2015, 2020 Ben Asselstine
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

#include "Commentator.h"
#include "player.h"
#include "defs.h"
#include "playerlist.h"
#include "stack.h"
#include "GameMap.h"
#include "stacklist.h"
#include "citylist.h"
#include "city.h"
#include "rnd.h"

Commentator* Commentator::d_instance = 0;

Commentator* Commentator::getInstance()
{
    if (!d_instance)
        d_instance = new Commentator();

    return d_instance;
}

void Commentator::deleteInstance()
{
    if (d_instance != 0)
        delete d_instance;

    d_instance = 0;
}

Commentator::Commentator()
{
}

bool Commentator::hasComment() const

{
  if ((Rnd::rand() % MAX_PLAYERS) == 0)
    return true;
  return false;
}

std::vector<Glib::ustring> Commentator::getComments(Player *player) const
{
  std::vector<Glib::ustring> comments;
  guint32 round = player->countEndTurnHistoryEntries();

  if (round < 2)
    return comments;

  if (player->getGold() < 100)
    comments.push_back(_("You are sadly in need of gold!"));
  else if (player->getGold() > 2500)
    {
      comments.push_back(_("Your wealth is greater than the mightiest of dragons!"));
      comments.push_back(_("All your gold must surely be a burden!"));
    }
  std::list<Hero*> heroes = player->getHeroes();
  if (heroes.size() == 0)
    comments.push_back(_("Will no hero defend your honour?"));
  else if (heroes.size() > 3)
    comments.push_back(_("I see heroes are flocking to your banner!"));
	
  if (round > 5 && player->getScore() < 10)
    {
      comments.push_back(_("Your enemies mock your feeble endeavours!"));
      comments.push_back(_("How much adversity can you endure?"));
      comments.push_back(_("Your enemies are beyond measure!"));
      comments.push_back(_("Your dreams of conquest confound you!"));
    }
  else if (round > 30 && player->getScore() < 10)
    comments.push_back(_("Your sorry efforts have come to nought!"));

  if (player->getScore() >= 40 && round >= 10)
    {
      comments.push_back(_("Victory is just beyond your reach!"));
      comments.push_back(_("Your destiny is forged in steel!"));
      comments.push_back(_("You stand at the crossroads of victory!"));
      comments.push_back(_("Attack is the best means of defence!"));
      comments.push_back(_("Do you feel the wolves snapping at your heels?"));
    }
  else if (player->getScore() >= 30 && round < 15)
    comments.push_back(_("Warlord!  Your progress is astounding!"));
  else if (player->getScore() >= 20 && round <= 20)
    {
      comments.push_back(_("So, Warlord, you show some merit!"));
      comments.push_back(_("You are doing well... ...so far!"));
    }

  if (player == Playerlist::getInstance()->getWinningPlayer())
    {
      comments.push_back(_("Beware!  Lest overconfidence consume you!"));
      comments.push_back(_("Your name evokes fear and loathing!"));
    }
  guint32 attacking_enemy_cities = 0;
  for (Stacklist::iterator it = player->getStacklist()->begin(); it != player->getStacklist()->end(); ++it)
    {
      Stack *stack = *it;
      if (stack->hasPath() && 
          GameMap::getEnemyCity(stack->getLastPointInPath()) != NULL)
        attacking_enemy_cities++;
    }
  if (attacking_enemy_cities > 0)
    comments.push_back(_("Ahh, the expectation of a coming battle!"));

  if (attacking_enemy_cities > 4)
    comments.push_back(_("Warlord... a mighty battle is brewing!"));

  City *capital_city = Citylist::getInstance()->getCapitalCity(player);
  if (capital_city && capital_city->getOwner() != player)
    comments.push_back(_("As your capital city has fallen, so shall you!"));
  return comments;
}
