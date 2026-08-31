// Copyright (C) 2002, 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2006 Andrea Paternesi
// Copyright (C) 2007, 2008, 2009, 2010, 2014, 2015, 2021 Ben Asselstine
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

#include "ai_dummy.h"
#include "playerlist.h"
#include "armysetlist.h"
#include "stacklist.h"
#include "citylist.h"
#include "city.h"
#include <fstream>
#include "path.h"
#include "action.h"
#include "xmlhelper.h"
#include "history.h"
#include "GameScenarioOptions.h"
#include "SightMap.h"
#include "Sage.h"
#include "rnd.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

AI_Dummy::AI_Dummy(Glib::ustring name, guint32 armyset,
                   std::vector<Gdk::RGBA> colors, int width, int height,
                   int player_no)
    :RealPlayer(name, armyset, colors, width, height, Player::AI_DUMMY, player_no)
{
}

AI_Dummy::AI_Dummy(const Player& player, bool sync_ids)
    :RealPlayer(player, sync_ids)
{
    d_type = AI_DUMMY;
}

AI_Dummy::AI_Dummy(XML_Helper* helper)
    :RealPlayer(helper)
{
}

void AI_Dummy::abortTurn()
{
  abort_requested = true;
  if (surrendered)
    aborted_turn.emit();
  else if (Playerlist::getInstance()->countPlayersAlive() == 1)
    aborted_turn.emit();
}

void AI_Dummy::setDefensiveProduction(City *city)
{
  if (city->getActiveProductionSlot() == -1 &&
      city->countDefenders() < MAX_ARMIES_PRODUCED_IN_NEUTRAL_CITY)
    {
      if (Rnd::rand() % 2 == 0)
	{
	  int idx = Rnd::rand() % city->getMaxNoOfProductionBases();
	  cityChangeProduction(city, idx);
	}
    }
  else
    {
      std::list<Action_Produce *> actions = getUnitsProducedThisTurn();
      for (std::list<Action_Produce *>::iterator it = actions.begin();
           it != actions.end(); ++it)
	{
	  if ((*it)->getCityId() == city->getId())
	    {
	      cityChangeProduction(city, -1);
	      break;
	    }
	}
    }
      //if production is stopped, then start it.
      //if an army arrived this turn, stop production.
}
void AI_Dummy::examineCities()
{
    debug("Examinating Cities to see what we can do")
    for (auto city: *Citylist::getInstance())
      {
        if ((city->isFriend(this)) && (city->isBurnt()==false))
	  setDefensiveProduction(city);
      }
}

bool AI_Dummy::startTurn()
{
      
  if (GameScenarioOptions::s_neutral_cities == GameParameters::DEFENSIVE)
    {
      //setup production defensive style.
      if (d_gold > 100)
	examineCities();
      else
	{
	  // stop the presses.
          for (auto city: *Citylist::getInstance())
	    {
	      if ((city->isFriend(this)) && (city->isBurnt()==false))
		city->setActiveProductionSlot(-1);
	    }
	}

    }
  //this is a dummy AI (neutral player) so there is not much point in
  //doing anything
  if (abort_requested)
    aborted_turn.emit();
  return !(Playerlist::getInstance()->getNoOfPlayers() <= 1);
}

void AI_Dummy::invadeCity(City* c)
{
  //dummy ai player should never invade an enemy city, but if it happens, we
  //make sure there is no inconsistency
  cityOccupy(c);
}

void AI_Dummy::heroGainsLevel(Hero * a)
{
  Army::Stat stat = Army::STRENGTH;
  doHeroGainsLevel(a, stat);
  addAction(new Action_Level(a, stat));
}

bool AI_Dummy::chooseHero(HeroProto *hero, City *city, int gold)
{
  (void) hero;
  (void) city;
  (void) gold;
  //neutral players never accept heroes.
  return false;
}
        
Reward *AI_Dummy::chooseReward(Ruin *ruin, Sage *sage, Stack *stack)
{
  (void) ruin;
  (void) stack;
  //neutrals don't search ruins, but let's not return null.
  return sage->front();
}

bool AI_Dummy::chooseTreachery (Stack *stack, Player *player, Vector <int> pos)
{
  (void) stack;
  (void) player;
  (void) pos;
  //neutrals don't leave the castle.
  return true;
}

Army::Stat AI_Dummy::chooseStat(Hero *hero)
{
  (void) hero;
  //neutrals don't have heroes.
  return Army::STRENGTH;
}

bool AI_Dummy::chooseQuest(Hero *hero)
{
  (void) hero;
  //neutrals don't have heroes.
  return true;
}

bool AI_Dummy::computerChooseVisitRuin(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) stack;
  (void) dest;
  (void) moves;
  (void) turns;
  //neutrals don't have heroes.
  return true;
}

bool AI_Dummy::computerChoosePickupBag(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) stack;
  (void) dest;
  (void) moves;
  (void) turns;
  //neutrals don't have heroes.
  return true;
}

bool AI_Dummy::computerChooseVisitTempleForBlessing(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) stack;
  (void) dest;
  (void) moves;
  (void) turns;
  //neutral army units don't go afield.
  return true;
}

bool AI_Dummy::computerChooseVisitTempleForQuest(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) stack;
  (void) dest;
  (void) moves;
  (void) turns;
  //neutrals don't have heroes
  return true;
}

bool AI_Dummy::computerChooseContinueQuest(Stack *stack, Quest *quest, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) stack;
  (void) quest;
  (void) moves;
  (void) dest;
  (void) turns;
  //neutrals don't have heroes
  return true;
}
// End of file
