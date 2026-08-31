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

#pragma once
#ifndef GAME_CLIENT_DECODER_H
#define GAME_CLIENT_DECODER_H

#include <config.h>
#include "chat-client.h"

#include <list>
#include <memory>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include "xml-helper.h"
#include "network-action.h"
#include "network-history.h"

class Player;

//! Helper class for sending NetworkAction objects to the NetworkPlayer.
class GameClientDecoder: public ChatClient
{
public:
  GameClientDecoder ();
  ~GameClientDecoder ();

  sigc::signal<void(std::string)> signal_game_scenario_received ()
    {
      return m_game_scenario_received;
    }

  sigc::signal<void(Player *)> signal_remote_player_moved ()
    {
      return m_remote_player_moved;
    }

  sigc::signal<void(Player *)> signal_remote_player_starts_move ()
    {
      return m_remote_player_starts_move;
    }

  sigc::signal<void(Player *)> signal_remote_player_named ()
    {
      return m_remote_player_named;
    }

  sigc::signal<void(Player *)> signal_remote_player_died ()
    {
      return m_remote_player_died;
    }

protected:
  class ActionLoader 
    {
  public:
      bool loadAction(Glib::ustring tag, XML_Helper* helper)
	{
	  if (tag == Action::d_tag)
	    {
	      NetworkAction *action = &*actions.back();
	      action->setAction(Action::handle_load(helper));
	      return true;
	    }
	  if (tag == NetworkAction::d_tag) 
	    {
	      NetworkAction * action = new NetworkAction(helper);
	      actions.push_back(action);
	      return true;
	    }
	  return false;
	}

      std::list<NetworkAction *> actions;
    };
  class HistoryLoader 
    {
  public:
      bool loadHistory(Glib::ustring tag, XML_Helper* helper)
	{
	  if (tag == History::d_tag)
	    {
	      NetworkHistory *history = &*histories.back();
	      history->setHistory(History::handle_load(helper));
	      return true;
	    }
	  if (tag == NetworkHistory::d_tag) 
	    {
	      NetworkHistory* history = new NetworkHistory(helper);
	      histories.push_back(history);
	      return true;
	    }
	  return false;
	}

      std::list<NetworkHistory *> histories;
    };


protected:
  void gotActions(const Glib::ustring &payload);
  void gotHistories(const Glib::ustring &payload);
  int decodeActions(std::list<NetworkAction*> actions);
  int decodeHistories(std::list<NetworkHistory*> histories);

  sigc::signal<void(std::string)> m_game_scenario_received;
  sigc::signal<void(Player *)> m_remote_player_moved;
  sigc::signal<void(Player *)> m_remote_player_starts_move;
  sigc::signal<void(Player *)> m_remote_player_named;
  sigc::signal<void(Player *)> m_remote_player_died;
};

#endif
