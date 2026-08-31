//  Copyright (C) 2003, 2004, 2005 Ulf Lorenz
//  Copyright (C) 2007, 2008, 2014, 2026 Ben Asselstine
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

#include "next-turn.h"
#include "player-list.h"
#include "player.h"

#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::flush<<std::endl;}
//#define debug(x)

NextTurn::NextTurn()
    :d_stop(false)
{
  continuing_turn = false;

  Player *active = Playerlist::getActiveplayer();
  game_abort = srequestAbort.connect(sigc::mem_fun(*active, &Player::abortTurn));
}

void NextTurn::stop()
{
  Player *active = Playerlist::getActiveplayer();
  game_abort = srequestAbort.connect(sigc::mem_fun(*active, &Player::abortTurn));
  d_stop = true;
  srequestAbort.emit();
}

void NextTurn::nextPlayer()
{
  Playerlist::instance()->nextPlayer();
  Player *active = Playerlist::getActiveplayer();

  game_abort.disconnect();
  game_abort = srequestAbort.connect(sigc::mem_fun(*active, &Player::abortTurn));
}
