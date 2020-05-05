// Copyright (C) 2008 Ole Laursen
// Copyright (C) 2008, 2014, 2015 Ben Asselstine
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

#include <iostream>
#include <fstream>

#include "game-client-decoder.h"

#include "action.h"
#include "network-action.h"
#include "network-history.h"
#include "network_player.h"
#include "playerlist.h"
#include "xmlhelper.h"
#include "GameScenario.h"
#include "ucompose.hpp"

GameClientDecoder::GameClientDecoder()
{
}

GameClientDecoder::~GameClientDecoder()
{
}

int GameClientDecoder::decodeActions(std::list<NetworkAction*> actions)
{
  int count = 0;
  for (std::list<NetworkAction *>::iterator i = actions.begin(),
       end = actions.end(); i != end; ++i)
  {
    NetworkAction *action = *i;
    Glib::ustring desc = action->toString();
    
    Player *p = action->getOwner();
    std::cerr << String::ucompose(_("decoding action: %1"), desc);
    NetworkPlayer *np = static_cast<NetworkPlayer *>(p);

    if (!np) {
      std::cerr << String::ucompose(_("warning, ignoring action for player %1"), p) << std::endl;
      continue;
    }

    np->decodeAction(action->getAction());
    if (action->getAction()->getType() == Action::END_TURN)
      remote_player_moved.emit((*actions.back()).getOwner());
    else if (action->getAction()->getType() == Action::INIT_TURN)
      remote_player_starts_move.emit((*actions.back()).getOwner());
    count++;
  }

  for (std::list<NetworkAction *>::iterator i = actions.begin(),
       end = actions.end(); i != end; ++i)
    delete *i;
  return count;
}

void GameClientDecoder::gotActions(const Glib::ustring &payload)
{
  std::istringstream is(payload);

  ActionLoader loader;
  
  XML_Helper helper(&is);
  helper.registerTag(Action::d_tag, sigc::mem_fun(loader, &ActionLoader::loadAction));
  helper.registerTag(NetworkAction::d_tag, sigc::mem_fun(loader, &ActionLoader::loadAction));
  helper.parseXML();
  helper.close();

  decodeActions(loader.actions);
}

int GameClientDecoder::decodeHistories(std::list<NetworkHistory *> histories)
{
  int count = 0;
  for (std::list<NetworkHistory *>::iterator i = histories.begin(),
       end = histories.end(); i != end; ++i)
  {
    NetworkHistory *history = *i;
    Glib::ustring desc = history->toString();
    std::cerr << String::ucompose(_("received history: %1"), desc) << std::endl;
    
    //just add it to the player's history list.
    Player *p = history->getOwner();
    p->getHistorylist()->push_back(History::copy(history->getHistory()));
    count++;
    if (history->getHistory()->getType() == History::PLAYER_VANQUISHED)
      remote_player_died.emit(history->getOwner());
  }

  for (std::list<NetworkHistory *>::iterator i = histories.begin(),
       end = histories.end(); i != end; ++i)
    delete *i;
  return count;
}

void GameClientDecoder::gotHistories(const Glib::ustring &payload)
{
  std::istringstream is(payload);

  HistoryLoader loader;
  
  XML_Helper helper(&is);
  helper.registerTag(History::d_tag, sigc::mem_fun(loader, &HistoryLoader::loadHistory));
  helper.registerTag(NetworkHistory::d_tag, sigc::mem_fun(loader, &HistoryLoader::loadHistory));
  helper.parseXML();
  helper.close();

  decodeHistories(loader.histories);
}
