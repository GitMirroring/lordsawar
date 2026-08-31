//  Copyright (C) 2003, 2004, 2005 Ulf Lorenz
//  Copyright (C) 2007, 2008, 2009, 2011, 2014, 2026 Ben Asselstine
//  Copyright (C) 2007, 2008 Ole Laursen
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

#include "next-turn-hotseat.h"

#include "player-list.h"
#include "city-list.h"
#include "ruin-list.h"
#include "stack-list.h"
#include "army-set-list.h"
#include "hero.h"
#include "vectored-unit-list.h"
#include "fog-map.h"
#include "history.h"
#include "quest-manager.h"
#include "game-scenario-options.h"
#include "configuration.h"

#include "path.h"

#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::flush<<std::endl;}
//#define debug(x)

NextTurnHotseat::NextTurnHotseat()
    :NextTurn()
{
  continuing_turn = false;
  
  for (auto p: *Playerlist::instance())
    p->ending_turn.connect(sigc::mem_fun(*this, &NextTurn::endTurn));
}

void NextTurnHotseat::end_of_round_after_player_died ()
{
  if (Playerlist::instance ()->getNoOfPlayers () <= 1)
    m_signal_game_over.emit ();
  else
    {
      finishRound();
      snextRound.emit();
      start ();
    }
}

// this routine handles end of round and then starts the next player
void NextTurnHotseat::check_end_of_round ()
{
  //if it is the first player's turn now, a new round has started
  if (Playerlist::getActiveplayer () == Playerlist::getFirstLiving ())
    {
      // the end of round chain of dialogs
      auto dead = Playerlist::instance ()->getDeadPlayers ();

      if (dead.empty () == false)
        {
          auto finish = std::make_shared<sigc::slot<void()>>();

          *finish =
            [this, finish, dead] () mutable
              {
                auto player = dead.front ();
                player->kill ();

                //do the next if there is one, or end
                dead.erase (dead.begin ());
                if (dead.empty () == false)
                  m_signal_player_died.emit (dead.front (), finish);
                else
                  {
                    if (Playerlist::getActiveplayer ()->isDead ())
                      nextPlayer ();
                    end_of_round_after_player_died ();
                  }
              };
          m_signal_player_died.emit (dead.front (), finish);
        }
      else
        {
          end_of_round_after_player_died ();
        }
    }
  else
    start ();
}

void NextTurnHotseat::start ()
{
  //set first player as active if no active player exists
  if (!Playerlist::getActiveplayer ())
    nextPlayer ();

  supdating.emit ();

  // do various start-up tasks
  if (continuing_turn)
    {
      continuing_turn = false;
      return;
    }

  startTurn ();

  // inform everyone about the next turn 
  splayerStart.emit (Playerlist::getActiveplayer ());

  // let the player do his or her duties...
  Playerlist::getActiveplayer ()->startTurn
    ([this] (bool continue_loop)
     {
       if (!continue_loop)
         return;

       //Now do some cleanup at the end of the turn.
       finishTurn ();

       //...and initiate the next one.
       nextPlayer ();

       guint32 millisecs =  Configuration::s_displaySpeedDelay * 3;
       if (Playerlist::getActiveplayer () == Playerlist::getNeutral ())
         millisecs = 10;
       Glib::signal_timeout ().connect
         ([this] ()
          {
            check_end_of_round ();
            return false;
          }, millisecs);
     });
}

void NextTurnHotseat::endTurn()
{
  // Finish off the player and transfers the control to the start function
  // again.
  finishTurn();
       
  //...and initiate the next player.
  nextPlayer ();

  check_end_of_round ();
}

void NextTurnHotseat::startTurn()
{
  //this function is called before a player starts his turn. Some
  //items you could imagine to be placed here: healing/building
  //units, check for joining heroes...

  //a shortcut
  Player* p = Playerlist::getActiveplayer();

  p->initTurn();
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

  for (auto it: *Playerlist::instance())
    {
      if (it->isDead())
        continue;

      it->collectTaxesAndPayUpkeep();

      //reset, and heal armies
      it->stacksReset();

      //vector armies (needs to preceed city's next turn)
      VectoredUnitlist::instance()->nextTurn(it);

      //produce new armies
      Citylist::instance()->nextTurn(it);
    }

  // heal the stacks in the ruins
  Playerlist::getNeutral()->ruinsReset();
    
  if (GameScenarioOptions::s_random_turns)
    {
      Playerlist::instance()->randomizeOrder();
      nextPlayer();
    }

}
