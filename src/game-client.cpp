//  Copyright (C) 2008 Ole Laursen
//  Copyright (C) 2011, 2014, 2015, 2017, 2021, 2026 Ben Asselstine
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
#include <sstream>
#include <fstream>

#include "lw.h"
#include "network-common.h"
#include "game-client.h"

#include "network-connection.h"
#include "file.h"
#include "action.h"
#include "network-action.h"
#include "network-history.h"
#include "player-list.h"
#include "xml-helper.h"
#include "ucompose.hpp"
#include "real-player.h"
#include "network-player.h"
#include "connection-manager.h"
#include "army-set-list.h"
#include "lobby.h"

GameClient * GameClient::s_instance = 0;

GameClient* GameClient::instance ()
{
  if (s_instance == 0)
    s_instance = new GameClient ();

  return s_instance;
}

void GameClient::deleteInstance ()
{
  if (s_instance)
    delete s_instance;

  s_instance = 0;
}

GameClient::GameClient ()
{
  d_connected = false;
  player_id = -1;
  network_connection = NULL;
  first_ping = true;
}

GameClient::~GameClient ()
{
}

void GameClient::start (Glib::ustring host, guint32 port,
                        Glib::ustring profile_id, Glib::ustring nick)
{
  d_host = host;
  d_port = port;
  player_id = -1;
  set_nickname (nick);
  setProfileId (profile_id);
  if (network_connection)
    network_connection->tear_down_connection ();
  network_connection = ConnectionManager::create_connection ();
  network_connection->torn_down.connect
    (sigc::mem_fun (*this, &GameClient::on_torn_down));
  network_connection->connected.connect
    (sigc::mem_fun (*this, &GameClient::onConnected));
  network_connection->connection_lost.connect
    (sigc::mem_fun (*this, &GameClient::onConnectionLost));
  network_connection->got_message.connect
    (sigc::mem_fun (*this, &GameClient::onGotMessage));
  network_connection->connection_failed.connect
    (sigc::mem_fun (m_client_could_not_connect, &sigc::signal<void()>::emit));
  network_connection->payload_progress.connect
    (sigc::mem_fun (m_payload_progress, &sigc::signal<void(int,int)>::emit));
  network_connection->connectToHost (host, port);
}

void GameClient::onConnected ()
{
  d_connected = true;

  d_ping_timer =
    Glib::signal_timeout ().connect
    (sigc::mem_fun (*this, &GameClient::on_ping_timeout), 5000);
  network_connection->send (MESSAGE_TYPE_PING, "");

  m_client_connected.emit ();

  Lobby::instance ()->join (getProfileId (), get_nickname ());
}

bool GameClient::on_ping_timeout ()
{
  network_connection->send (MESSAGE_TYPE_PING, "");

  return true;
}

void GameClient::onConnectionLost ()
{
  d_ping_timer.disconnect ();
  if (d_connected)
    m_client_forcibly_disconnected.emit ();
  else
    m_client_could_not_connect.emit ();
}

void GameClient::sat_down (Shield::Color shield, Glib::ustring profile_id)
{
  auto player = Playerlist::instance ()->get (shield);
  if (!player)
    return;
  if (player->getType () == Player::NETWORKED)
    dynamic_cast<NetworkPlayer*>(player)->setConnected (true);
  Lobby::instance ()->sit (shield, profile_id);
  m_player_sits.emit (shield, profile_id);
}

void GameClient::type_changed (Player *player, int type)
{
  if (!player)
    return;

  //special case, someone has turned off a player that had someone sitting
  if (type == GameParameters::Player::OFF)
    Lobby::instance ()->stand (player->get_shield ());
  //we don't change the type of player here.
  //the player is networked on the remote side, and not networked
  //(e.g. ai_smart) on the local side.
  //we show the "ai smart" in the game lobby, but the player object is still
  //networked.
  m_player_changes_type.emit (player, type);
}

void GameClient::stood_up (Shield::Color shield, Glib::ustring profile_id)
{
  auto player = Playerlist::instance ()->get (shield);
  if (!player)
    return;
  if (player->getType () == Player::HUMAN)
    {
      //this covers the "boot", or forcibly-stand case.
      NetworkPlayer *new_p = new NetworkPlayer (*player);
      Playerlist::instance ()->swap (player, new_p);
      delete player;
      player = new_p;
      new_p->setConnected (false);
    }
  else if (player->getType () == Player::NETWORKED)
    dynamic_cast<NetworkPlayer*>(player)->setConnected (false);
  Lobby::instance ()->stand (shield);
  m_player_stands.emit (shield, profile_id);
}

bool GameClient::onGotMessage (int type, Glib::ustring payload)
{
  if (Lw::app->m_network_debug && type != MESSAGE_TYPE_PONG)
    std::cerr <<
      String::ucompose (_("%1: got message of type %2"),
                        Lw::get_prgname (),
                        NetworkConnection::typeToString (type)) << std::endl;

  switch (MessageType (type))
    {
    case MESSAGE_TYPE_PING:
      network_connection->send (MESSAGE_TYPE_PONG, "PONGOGOGO");
      break;

    case MESSAGE_TYPE_PONG:
      if (first_ping)
        {
          network_connection->send (MESSAGE_TYPE_PARTICIPANT_CONNECT,
                                    getProfileId () + " " + get_nickname ());
          first_ping = false;
        }
      break;

    case MESSAGE_TYPE_SAME_PROFILE_ID:
      m_logged_in_as_same_profile_id.emit ();
      disconnect ();
      break;

    case MESSAGE_TYPE_SENDING_ACTIONS:
      gotActions (payload);
      break;

    case MESSAGE_TYPE_SENDING_MAP:
      m_game_scenario_received.emit (payload);
      break;

    case MESSAGE_TYPE_PARTICIPANT_CONNECTED:
        {
          if (Lw::app->m_network_debug)
            std::cerr <<
              String::ucompose (_("%1: message: %2 has data: %3"),
                                Lw::get_prgname (),
                                NetworkConnection::typeToString (type),
                                payload) << std::endl;

          auto pos = payload.find (' ');
          if (pos != Glib::ustring::npos)
            {
              auto update = payload.substr (0, pos);
              auto profile_id_and_nickname = payload.substr (pos + 1);
              auto ppos = profile_id_and_nickname.find (' ');
              if (ppos != Glib::ustring::npos)
                {
                  Glib::ustring nick = profile_id_and_nickname.substr (ppos + 1);
                  Glib::ustring profile_id =
                    profile_id_and_nickname.substr (0, ppos);
                  Lobby::instance ()->join (profile_id, nick);
                  m_remote_participant_joins.emit (profile_id, update == "1");
                }
            }
        }
      break;

    case MESSAGE_TYPE_PARTICIPANT_DISCONNECTED:
        {
          if (Lw::app->m_network_debug)
            std::cerr <<
              String::ucompose (_("%1: message: %2 has data: %3"),
                                Lw::get_prgname (),
                                NetworkConnection::typeToString (type),
                                payload) << std::endl;

          Glib::ustring profile_id_and_nickname = payload;
          auto pos = profile_id_and_nickname.find (' ');
          if (pos != Glib::ustring::npos)
            {
              Glib::ustring nick = profile_id_and_nickname.substr (pos + 1);
              Glib::ustring profile_id = profile_id_and_nickname.substr (0, pos);
              Lobby::instance ()->depart (profile_id);
              m_remote_participant_departs.emit (profile_id);
            }
        }
      break;

    case MESSAGE_TYPE_MOD_ID:
      Lobby::instance ()->add_mod (payload);
      m_moderator_assigned.emit (payload);
      break;

    case MESSAGE_TYPE_ROUND_OVER:
      m_round_ends.emit ();
      break;

    case MESSAGE_TYPE_GAME_CAN_BEGIN:
  
      got_chat_message
        ("",
         _("Press play to begin the game."));

      m_game_can_begin.emit ();
      break;

    case MESSAGE_TYPE_WAITING_FOR_READY:
      m_waiting_for_ready.emit (atoi (payload.c_str ()));
      break;

    case MESSAGE_TYPE_REQUEST_SEAT_MANIFEST:
    case MESSAGE_TYPE_PARTICIPANT_CONNECT:
    case MESSAGE_TYPE_PARTICIPANT_DISCONNECT:
    case MESSAGE_TYPE_CHAT:
    case MESSAGE_TYPE_KICK:
    case MESSAGE_TYPE_READY:
      //FIXME: faulty server.
      break;

    case MESSAGE_TYPE_CHATTED:
        {
          auto pos = payload.find (' ');
          if (pos != Glib::ustring::npos)
            {
              auto profile_id = payload.substr (0, pos);
              auto message = payload.substr (pos + 1);
              got_chat_message (profile_id, message);
            }
        }
      break;

    case MESSAGE_TYPE_SYSMSG:
        {
          auto pos = payload.find (' ');
          if (pos != Glib::ustring::npos)
            {
              auto profile_id = payload.substr (0, pos);
              auto message = payload.substr (pos + 1);
              got_system_message (profile_id, message);
            }
        }
      break;

    case MESSAGE_TYPE_SENDING_HISTORY:
      gotHistories (payload);
      break;

    case MESSAGE_TYPE_SERVER_DISCONNECT:
      return false;
      break;

    case MESSAGE_TYPE_TURN_ORDER:
      gotTurnOrder (payload);
      break;

    case MESSAGE_TYPE_KILL_PLAYER:
      gotKillPlayer (Playerlist::instance ()->get (atoi (payload.c_str ())));
      break;

    case MESSAGE_TYPE_NEXT_PLAYER:
        {
          auto shield = Shield::Color (atoi (payload.c_str ()));
          auto p = Playerlist::instance ()->get (shield);
          m_start_player_turn.emit (p);
        }
      break;

    case MESSAGE_TYPE_ROUND_START:
      m_round_begins.emit ();
      break;

    case MESSAGE_TYPE_COUNTDOWN:
      m_countdown.emit (atoi (payload.c_str ()));
      break;

    case MESSAGE_TYPE_CHANGE_NICKNAME:
        {
          Glib::ustring profile_id_and_nickname = payload;
          auto pos = profile_id_and_nickname.find (' ');
          if (pos != Glib::ustring::npos)
            {
              Glib::ustring new_nick = profile_id_and_nickname.substr (pos + 1);
              Glib::ustring profile_id = profile_id_and_nickname.substr (0,
                                                                         pos);
              m_nickname = new_nick;
              Lobby::instance ()->change_name (profile_id, new_nick);
              m_nickname_changed.emit (profile_id, new_nick);
            }
        }
      break;

    case MESSAGE_TYPE_GAME_BEGIN:
      m_game_begin.emit ();
      break;

    case MESSAGE_TYPE_OFF_PLAYER:
      gotOffPlayer (Playerlist::instance ()->get (atoi (payload.c_str ())));
      break;

    case MESSAGE_TYPE_LOBBY_ACTIVITY:
        {
          Shield::Color shield;
          Glib::ustring profile_id;
          gint32 action;
          bool reported;
          Glib::ustring data;
          bool success = get_message_lobby_activity (payload, shield, profile_id,
                                                     action, reported, data);
          if (Lw::app->m_network_debug)
            std::cerr <<
              String::ucompose
              (_("%1: lobby activity message is of type %2 (%3)"),
               Lw::get_prgname (),
               NetworkConnection::lobbyActionTypeToString (action), action) <<
              std::endl;

          if (success)
            {
              if (reported)
                {
                  switch (action)
                    {
                    case LOBBY_MESSAGE_TYPE_SIT:
                      sat_down (shield, profile_id);
                      break;
                    case LOBBY_MESSAGE_TYPE_STAND:
                      stood_up (shield, profile_id);
                      break;
                    case LOBBY_MESSAGE_TYPE_CHANGE_NAME:
                      break;
                    case LOBBY_MESSAGE_TYPE_CHANGE_TYPE:
                      type_changed (Playerlist::instance ()->get (shield),
                                    atoi (data.c_str ()));
                      break;
                    default:
                      break;
                    }
                }
            }
        }
      break;

    }
  return true;
}

void GameClient::kick (Glib::ustring profile_id)
{
  network_connection->send (MESSAGE_TYPE_KICK, profile_id);
}

void GameClient::sendReady ()
{
  network_connection->send (MESSAGE_TYPE_READY, "");
}

void GameClient::gotOffPlayer (Player *player)
{
  m_player_gets_turned_off.emit (player);
  GameParameters::Player p;
  p.id = player->getId ();
  p.type = GameParameters::Player::OFF;
  p.name = player->getName ();
  auto as = Armysetlist::instance ()->get (player->getArmyset ());
  if (as)
    Playerlist::instance ()->syncPlayer (p, as->getBaseName ());
  else
    Playerlist::instance ()->syncPlayer (p, "default");
}

void GameClient::gotKillPlayer (Player *player)
{
  player->kill (false);
}

void GameClient::onHistoryDone (History *h, guint32 id)
{
  NetworkHistory *history = new NetworkHistory (h, id);
  Glib::ustring desc = history->toString ();

  if (history->getHistory ()->getType () == History::PLAYER_VANQUISHED)
    m_local_player_died.emit (history->getOwner ());

  histories.push_back (history);
  sendHistories ();
  clearNetworkHistorylist (histories);
}

void GameClient::onActionDone (Action *a, guint32 id)
{
  NetworkAction *action = new NetworkAction (a, id);
  Glib::ustring desc = action->toString ();

  if (action->getAction ()->getType () == Action::END_TURN)
    m_local_player_moved.emit (action->getOwner ());
  if (action->getAction ()->getType () == Action::INIT_TURN)
    m_local_player_starts_move.emit (action->getOwner ());

  actions.push_back (action);
  sendActions ();
  clearNetworkActionlist (actions);
}

void GameClient::sendActions ()
{
  std::ostringstream os;
  XML_Helper helper (&os);

  helper.begin (String::ucompose ("%1", MESSAGE_PROTOCOL_VERSION));
  helper.open_tag ("actions");

  for (auto i = actions.begin (), end = actions.end (); i != end; ++i)
    (*i)->save (&helper);

  helper.close_tag ();

  if (Lw::app->m_network_debug)
    std::cerr <<
      String::ucompose (_("%1: sending actions"),
                        Lw::get_prgname ()) << std::endl;
  network_connection->send (MESSAGE_TYPE_SENDING_ACTIONS, os.str ());
}

void GameClient::sendHistories ()
{
  std::ostringstream os;
  XML_Helper helper (&os);

  helper.begin (String::ucompose ("%1", MESSAGE_PROTOCOL_VERSION));
  helper.open_tag ("histories");

  for (auto i = histories.begin (), end = histories.end (); i != end; ++i)
    (**i).save (&helper);

  helper.close_tag ();

  network_connection->send (MESSAGE_TYPE_SENDING_HISTORY, os.str ());
}

void GameClient::sit_or_stand (Shield::Color shield, bool sit)
{
  auto player = Playerlist::instance ()->get (shield);
  if (!player)
    return;
  Glib::ustring payload =
    String::ucompose ("%1 %2 %3 %4 %5", (int)shield, getProfileId (),
                      sit ? LOBBY_MESSAGE_TYPE_SIT : LOBBY_MESSAGE_TYPE_STAND,
                      0, m_nickname);
  network_connection->send (MESSAGE_TYPE_LOBBY_ACTIVITY, payload);
  if (sit)
    {
      RealPlayer *new_p = new RealPlayer (*player, true);
      Playerlist::instance ()->swap (player, new_p);
      stopListeningForLocalEvents (player);
      listenForLocalEvents (new_p);
      delete player;
      payload =
        String::ucompose ("%1 %2 %3 %4 %5", new_p->getId (), getProfileId (),
                          LOBBY_MESSAGE_TYPE_CHANGE_TYPE, 0, new_p->getType ());
      network_connection->send (MESSAGE_TYPE_LOBBY_ACTIVITY, payload);
      //Lobby::instance ()->sit (shield, getProfileId ());
    }
  else
    {
      NetworkPlayer *new_p = new NetworkPlayer (*player);
      Playerlist::instance ()->swap (player, new_p);
      stopListeningForLocalEvents (player);
      delete player;
      new_p->setConnected (false);
      Lobby::instance ()->stand (shield);
    }

}

void GameClient::change_type (Player *player, int type)
{
  Glib::ustring payload =
    String::ucompose ("%1 %2 %3 %4 %5", player->get_shield (), getProfileId (),
                      LOBBY_MESSAGE_TYPE_CHANGE_TYPE, 0, type);
  network_connection->send (MESSAGE_TYPE_LOBBY_ACTIVITY, payload);
}

void GameClient::sit_down (Shield::Color shield)
{
  sit_or_stand (shield, true);
}

void GameClient::stand_up (Shield::Color shield)
{
  sit_or_stand (shield, false);
}

void GameClient::chat (Glib::ustring message)
{
  network_connection->send (MESSAGE_TYPE_CHAT, message);
  Lobby::instance ()->add_chat_message (getProfileId (), message);
}

void GameClient::request_seat_manifest ()
{
  network_connection->send (MESSAGE_TYPE_REQUEST_SEAT_MANIFEST, "");
}

void GameClient::gotTurnOrder (Glib::ustring payload)
{
  std::list<guint32> player_ids;
  std::stringstream players;
  players.str (payload);

  int ival;
  while (players.eof () == false)
    {
      ival = -1;
      players >> ival;
      if (ival != -1)
        player_ids.push_back (ival);
    }
  Playerlist::instance ()->reorder (player_ids);
  m_playerlist_reorder_received.emit ();
}

void GameClient::disconnect ()
{
  d_connected = false;
  if (network_connection)
    {
      network_connection->send
        (MESSAGE_TYPE_PARTICIPANT_DISCONNECT, m_nickname);
      network_connection->tear_down_connection ();
    }
}

void GameClient::on_torn_down ()
{
  network_connection = NULL;
}
