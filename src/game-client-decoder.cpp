//  Copyright (C) 2008 Ole Laursen
//  Copyright (C) 2008, 2014, 2015, 2026 Ben Asselstine
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

#include <iostream>
#include <fstream>

#include "lw.h"
#include "game-client-decoder.h"

#include "action.h"
#include "network-action.h"
#include "network-history.h"
#include "network-player.h"
#include "player-list.h"
#include "xml-helper.h"
#include "game-scenario.h"
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
    if (Lw::app->m_network_debug)
      std::cerr << String::ucompose(_("%1: decoding action for %2 (%3): %4"),
                                    Lw::get_prgname (), 
                                    p->getName (),
                                    Player::playerTypeToString (Player::Type (p->getType ())),
                                    desc) << std::endl;
    NetworkPlayer *np = static_cast<NetworkPlayer *>(p);

    if (!np)
      {
        if (Lw::app->m_network_debug)
          std::cerr <<
            String::ucompose(_("%1: warning, ignoring action for player %2"),
                             Lw::get_prgname (), p) << std::endl;
        continue;
      }

    np->decodeAction(action->getAction());
    if (action->getAction()->getType() == Action::END_TURN)
      m_remote_player_moved.emit((*actions.back()).getOwner());
    else if (action->getAction()->getType() == Action::INIT_TURN)
      m_remote_player_starts_move.emit((*actions.back()).getOwner());
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
  helper.register_tag(Action::d_tag, sigc::mem_fun(loader, &ActionLoader::loadAction));
  helper.register_tag(NetworkAction::d_tag, sigc::mem_fun(loader, &ActionLoader::loadAction));
  helper.parse_XML();
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
    if (Lw::app->m_network_debug)
      std::cerr <<
        String::ucompose(_("%1: received history: %2"),
                         Lw::get_prgname (), desc) << std::endl;
    
    //just add it to the player's history list.
    Player *p = history->getOwner();
    p->getHistorylist()->push_back(History::copy(history->getHistory()));
    count++;
    if (history->getHistory()->getType() == History::PLAYER_VANQUISHED)
      m_remote_player_died.emit(history->getOwner());
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
  helper.register_tag(History::d_tag, sigc::mem_fun(loader, &HistoryLoader::loadHistory));
  helper.register_tag(NetworkHistory::d_tag, sigc::mem_fun(loader, &HistoryLoader::loadHistory));
  helper.parse_XML();
  helper.close();

  decodeHistories(loader.histories);
}
