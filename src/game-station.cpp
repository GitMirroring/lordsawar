// Copyright (C) 2008, 2011, 2014 Ben Asselstine
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
#include <sstream>
#include <list>

#include "game-station.h"
#include "network-common.h"

#include "network-action.h"
#include "network-history.h"

GameStation::GameStation()
{
}

void GameStation::clearNetworkActionlist(std::list<NetworkAction*> &a)
{
  for (std::list<NetworkAction*>::iterator it = a.begin();
       it != a.end(); ++it)
    {
      delete (*it);
    }
  a.clear();
}

void GameStation::clearNetworkHistorylist(std::list<NetworkHistory*> &h)
{
  for (std::list<NetworkHistory*>::iterator it = h.begin(); 
       it != h.end(); ++it)
    {
      delete (*it);
    }
  h.clear();
}

void GameStation::listenForLocalEvents(Player *p)
{
  sigc::connection connection;
  connection = p->acting.connect(sigc::mem_fun(this, 
					       &GameStation::onActionDone));
  action_listeners[p->getId()] = connection;
  connection = p->history_written.connect
    (sigc::mem_fun(this, &GameStation::onHistoryDone));
  history_listeners[p->getId()] = connection;
}

void GameStation::stopListeningForLocalEvents()
{
  for (std::map<guint32, sigc::connection>::iterator i = action_listeners.begin();
       i != action_listeners.end(); ++i)
    (*i).second.disconnect();
  action_listeners.clear();
  for (std::map<guint32, sigc::connection>::iterator j = history_listeners.begin();
       j != history_listeners.end(); ++j)
    (*j).second.disconnect();
  history_listeners.clear();
}

void GameStation::stopListeningForLocalEvents(Player *p)
{
  sigc::connection connection;
  std::map<guint32, sigc::connection>::iterator it;
  it = action_listeners.find(p->getId());
  if (it != action_listeners.end())
    {
      connection = (*it).second;
      connection.disconnect();
      action_listeners.erase(it);
    }
  it = history_listeners.find(p->getId());
  if (it != history_listeners.end())
    {
      connection = (*it).second;
      connection.disconnect();
      history_listeners.erase(it);
    }
}

bool GameStation::get_message_lobby_activity (Glib::ustring payload, 
                                             guint32 &player_id, 
                                             gint32 &action, bool &reported,
                                             Glib::ustring &remainder)
{
  std::stringstream spayload;
  spayload.str(payload);
  spayload >> player_id;
  if (player_id >= MAX_PLAYERS + 1)
    return false;
  spayload >> action;
  switch (action)
    {
    case LOBBY_MESSAGE_TYPE_SIT:
    case LOBBY_MESSAGE_TYPE_CHANGE_NAME:
    case LOBBY_MESSAGE_TYPE_STAND:
    case LOBBY_MESSAGE_TYPE_CHANGE_TYPE:
      break;
    default:
      return false;
    }
  spayload >> reported;
  if (reported != 0 && reported != 1)
    return false;
  //okay, the rest of the stringstream is a nickname.
  char buffer[1024];
  memset (buffer, 0, sizeof (buffer));
  spayload.get();
  spayload.rdbuf()->sgetn(buffer, sizeof (buffer));
  remainder = Glib::ustring (buffer);
  return true;
}
// End of file
