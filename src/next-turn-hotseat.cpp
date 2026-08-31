// Copyright (C) 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2007, 2008, 2009, 2011, 2014 Ben Asselstine
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

#include "NextTurnHotseat.h"

#include "playerlist.h"
#include "citylist.h"
#include "ruinlist.h"
#include "stacklist.h"
#include "armysetlist.h"
#include "hero.h"
#include "vectoredunitlist.h"
#include "FogMap.h"
#include "history.h"
#include "QuestsManager.h"
#include "GameScenarioOptions.h"

#include "path.h"

#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::flush<<std::endl;}
//#define debug(x)

NextTurnHotseat::NextTurnHotseat()
    :NextTurn()
{
  continuing_turn = false;
  
  for (auto p: *Playerlist::getInstance())
    p->ending_turn.connect(sigc::mem_fun(this, &NextTurn::endTurn));
}

void NextTurnHotseat::start()
{
    //We need the playerlist a lot, so maintain a copy of it.

    //set first player as active if no active player exists
    if (!Playerlist::getActiveplayer())
      nextPlayer();
	
    while (!d_stop)
    {
      supdating.emit();

        // do various start-up tasks
        if (continuing_turn)
	  {
	    continuing_turn = false;
	    return;
	  }

	startTurn();
       
	// inform everyone about the next turn 
	snextTurn.emit(Playerlist::getActiveplayer());
    
	splayerStart.emit(Playerlist::getActiveplayer());

	// let the player do his or her duties...
	bool continue_loop = Playerlist::getActiveplayer()->startTurn();
	if (!continue_loop || d_stop)
	  return;
	
	//Now do some cleanup at the end of the turn.
	finishTurn();

        //...and initiate the next one.
	nextPlayer();
        
        //if it is the first player's turn now, a new round has started
        if (Playerlist::getInstance()->getActiveplayer() == 
	    Playerlist::getInstance()->getFirstLiving())
        
	  {
	    if (Playerlist::getInstance()->checkPlayers() == true)
	      {
		if (Playerlist::getInstance()->getNoOfPlayers() <= 1)
		  break;
		if (Playerlist::getActiveplayer()->isDead())
		  nextPlayer();
	      }
	    finishRound();
	    snextRound.emit();
	  }
    }
}

void NextTurnHotseat::endTurn()
{
  // Finish off the player and transfers the control to the start function
  // again.
  finishTurn();
  if (Playerlist::getInstance()->checkPlayers() == true)
    {
      if (d_stop)
	return;
      if (Playerlist::getInstance()->getNoOfPlayers() <= 1)
	return;
    }
  nextPlayer();

  //this is problematic when the first player dies.
  //end the turn if all living players have ended their turn.
  if (Playerlist::getInstance()->isEndOfRound() == true)
    {
      finishRound();
      snextRound.emit();
    }

  start();
}

void NextTurnHotseat::startTurn()
{
  //this function is called before a player starts his turn. Some
  //items you could imagine to be placed here: healing/building
  //units, check for joining heroes...

  //a shortcut
  Player* p = Playerlist::getActiveplayer();

  p->initTurn();

  //calculate upkeep and income
  p->calculateUpkeep();
  p->calculateIncome();

  QuestsManager::getInstance()->nextTurn(p);
}

void NextTurnHotseat::finishTurn()
{
  //Put everything that has to be done before the next player starts
  //his turn here. E.g. one could clear some caches.
  Player *p = Playerlist::getActiveplayer();
  p->getFogMap()->smooth();
  p->endTurn();
}

void NextTurnHotseat::finishRound()
{
  //Put everything that has to be done when a new round starts in here.
  //E.g. increase the round number in GameScenario. (this is done with
  //the snextRound signal, but useful for an example).

  for (auto it: *Playerlist::getInstance())
    {
      if (it->isDead())
        continue;

      it->collectTaxesAndPayUpkeep();

      //reset, and heal armies
      it->stacksReset();

      //vector armies (needs to preceed city's next turn)
      VectoredUnitlist::getInstance()->nextTurn(it);

      //produce new armies
      Citylist::getInstance()->nextTurn(it);
    }

  // heal the stacks in the ruins
  Playerlist::getInstance()->getNeutral()->ruinsReset();
    
  if (GameScenarioOptions::s_random_turns)
    {
      Playerlist::getInstance()->randomizeOrder();
      nextPlayer();
    }

}
