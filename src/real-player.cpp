//  Copyright (C) 2002, 2003 Michael Bartl
//  Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
//  Copyright (C) 2004, 2006 Andrea Paternesi
//  Copyright (C) 2004 John Farrell
//  Copyright (C) 2004 Bryan Duff
//  Copyright (C) 2006, 2007, 2008, 2009, 2014, 2015, 2021, 2026 Ben Asselstine
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

#include <fstream>

#include "real-player.h"
#include "action.h"
#include "history.h"
#include "player-list.h"
#include "stack-list.h"
#include "city-list.h"
#include "city.h"
#include "hero-templates.h"
#include "game.h"
#include "xml-helper.h"
#include "game-scenario-options.h"
#include "sight-map.h"
#include "sage.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

RealPlayer::RealPlayer(Glib::ustring name, guint32 armyset,
                       Shield::Color shield, int width, int height,
                       Player::Type type)
    :Player(name, armyset, shield, width, height, type),
    d_abort_requested(false)
{
}

RealPlayer::RealPlayer(const Player& player, bool sync_ids)
    :Player(player, sync_ids)
{
    d_type = HUMAN;
    d_abort_requested = false;
}

RealPlayer::RealPlayer(XML_Helper* helper)
    :Player(helper), d_abort_requested(false)
{
}

bool RealPlayer::save(XML_Helper* helper) const
{
    // This allows computer players to save additional data
    bool retval = true;
    retval &= helper->open_tag(Player::d_tag);
    retval &= Player::saveContents(helper);
    retval &= helper->close_tag();

    return retval;
}

void RealPlayer::abortTurn()
{
  aborted_turn.emit();
}

void RealPlayer::startTurn(sigc::slot<void(bool)> finish)
{
  finish (false);
}

void RealPlayer::endTurn()
{
  pruneActionlist();
  recordEndOfTurn();
  //this is where a lot of signals are piling up
}

void RealPlayer::invadeCity(City* c)
{
  (void) c;
    // For the realplayer, this function doesn't do a lot. However, an AI
    // player has to decide here what to do (occupy, raze, pillage)
}

bool RealPlayer::chooseHero(HeroProto *hero, City* c, int gold)
{
  (void) hero;
  (void) c;
  (void) gold;
    // For the realplayer, this function doesn't do a lot. However, an AI
    // player has to decide here what to do (accept/deny hero)
    return false;
}

Reward *RealPlayer::chooseReward(Ruin *ruin, Sage *sage, Stack *stack)
{
  (void) ruin;
  (void) sage;
  (void) stack;
    // For the realplayer, this function doesn't do a lot. However, an AI
    // player has to decide here what to do (pick a reward from sage)
    return NULL;
}

bool RealPlayer::chooseTreachery (Stack *stack, Player *player, Vector <int> pos)
{
  (void) stack;
  (void) player;
  (void) pos;
    // For the realplayer, this function doesn't do a lot. However, an AI
    // player has to decide here what to do (fight a friend or not)
  return true;
}

Army::Stat RealPlayer::chooseStat(Hero *hero)
{
  (void) hero;
    // For the realplayer, this function doesn't do a lot. However, an AI
    // player has to decide here what to do (pick strength/moves/sight stat)
  return Army::STRENGTH;
}

bool RealPlayer::chooseQuest(Hero *hero)
{
  (void) hero;
  //we decide interactively with the gui, not by this method.
  // For the realplayer, this function doesn't do a lot. However, an AI
  // player has to decide here what to do (get a quest for the hero or not)
  return true;
}

void RealPlayer::heroGainsLevel(Hero* a, Army::Stat stat)
{
    // the standard human player just asks the GUI what to do
    doHeroGainsLevel(a, stat);
    addAction(new Action_Level(a, stat));
}

CityDefeatedChoice RealPlayer::chooseCityDefeatedAction (City *c, Stack *s)
{
  (void) c;
  (void) s;
  //we don't do this, instead we go out to the gui to ask
  return CityDefeatedChoice::CITY_DEFEATED_OCCUPY;
}

bool RealPlayer::chooseVisitRuin(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) stack;
  (void) dest;
  (void) moves;
  (void) turns;
  //this decision callback is only for computer players
  return true;
}

bool RealPlayer::choosePickupBag(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) stack;
  (void) dest;
  (void) moves;
  (void) turns;
  //this decision callback is only for computer players
  return true;
}

bool RealPlayer::chooseVisitTempleForBlessing(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) stack;
  (void) dest;
  (void) moves;
  (void) turns;
  //this decision callback is only for computer players
  return true;
}

bool RealPlayer::chooseVisitTempleForQuest(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) stack;
  (void) dest;
  (void) moves;
  (void) turns;
  //this decision callback is only for computer players
  return true;
}

bool RealPlayer::chooseContinueQuest(Stack *stack, Quest *quest, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) stack;
  (void) quest;
  (void) dest;
  (void) moves;
  (void) turns;
  //this decision callback is only for computer players
  return true;
}
