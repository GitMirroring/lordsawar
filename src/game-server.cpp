//  Copyright (C) 2008 Ole Laursen
//  Copyright (C) 2008, 2011, 2014, 2015, 2017, 2020, 2021, 2026 Ben Asselstine
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
#include <list>
#include "lw.h"
#include "game-parameters.h"
#include "game-server.h"

#include "file.h"
#include "network-server.h"
#include "game.h"
#include "xml-helper.h"
#include "game-scenario.h"
#include "player-list.h"
#include "player.h"
#include "network-action.h"
#include "network-history.h"
#include "configuration.h"
#include "network-player.h"
#include "real-player.h"
#include "game-scenario-options.h"
#include "ucompose.hpp"
#include "army-set-list.h"
#include "lobby.h"
#include "network-connection.h"

class NetworkAction;

//! A helper class for GameServer.  A connected user in a network game.
struct Participant
{
  void *conn;
  std::list<GameParameters::Player> players;
  std::map<guint32, bool> id_end_turn;
  std::list<NetworkAction *> actions;
  std::list<NetworkHistory *> histories;
  Glib::ustring nickname;
  bool departed;
  Glib::ustring profile_id;
  Glib::DateTime connect_time;
  Glib::DateTime disconnect_time;
  bool ready;
};

GameServer * GameServer::s_instance = 0;

GameServer* GameServer::instance ()
{
  if (s_instance == 0)
    s_instance = new GameServer ();

  return s_instance;
}

void GameServer::deleteInstance ()
{
  if (s_instance)
    delete s_instance;

  s_instance = 0;
}

GameServer::GameServer ()
{
  d_game_has_begun = false;

  m_remote_player_moved.connect
    (sigc::mem_fun (*this, &GameServer::on_player_finished_turn));
  m_local_player_moved.connect
    (sigc::mem_fun (*this, &GameServer::on_player_finished_turn));
  d_stop = false;
  d_save_messages = "";
}

void GameServer::notifyRoundOver ()
{
  for (auto &i: participants)
    send (i->conn, MESSAGE_TYPE_ROUND_OVER, "");
}

bool GameServer::check_end_of_round ()
{
  if (Playerlist::getNeutral ()->hasAlreadyEndedTurn ())
    {
      Playerlist::getNeutral ()->clearActionlist ();
      notifyRoundOver ();
      sendRoundStart ();
      m_round_ends.emit ();
      return true;
    }
  return false;
}

void GameServer::on_player_finished_turn (Player *player)
{
  if (check_end_of_round () == false)
    {
      //if the end of turn is asynchronous, start a new turn from here
      //otherwise, just fall through back to the nextTurn method, and it's
      //inner loop.
      if (player->getType () == Player::HUMAN ||
          player->getType () == Player::NETWORKED)
        {
          if (nextTurn ())
            on_player_finished_turn (Playerlist::getNeutral ());
        }
    }
}

void GameServer::remove_all_participants ()
{
  stopListeningForLocalEvents ();
  //say goodbye to all participants
  for (auto &i: participants)
    send (i->conn, MESSAGE_TYPE_SERVER_DISCONNECT, "bye");
  for (auto &i: participants)
    delete i;
  participants.clear ();
  for (auto &i: disconnected_participants)
    delete i;
  disconnected_participants.clear ();
  players_seated_locally.clear ();
}

GameServer::~GameServer ()
{
  if (network_server.get () != NULL)
    {
      if (network_server->isRunning ())
        network_server->stop ();
    }
}

bool GameServer::isRunning ()
{
  if (network_server.get () != NULL)
    return network_server->isRunning ();
  else
    return false;
}

void GameServer::start (GameScenario *game_scenario, int port,
                        Glib::ustring profile_id, Glib::ustring nick)
{
  setGameScenario (game_scenario);
  setProfileId (profile_id);
  set_nickname (nick);

  if (network_server.get () != NULL && network_server->isRunning ())
    return;
  network_server.reset (new NetworkServer ());
  network_server->port_in_use.connect
    (sigc::mem_fun (m_port_in_use, &sigc::signal<void(int)>::emit));
  network_server->got_message.connect
    (sigc::mem_fun (*this, &GameServer::onGotMessage));
  network_server->connection_lost.connect
    (sigc::mem_fun (*this, &GameServer::onConnectionLost));
  network_server->connection_made.connect
    (sigc::mem_fun (*this, &GameServer::onConnectionMade));

  network_server->startListening (port);

  listenForLocalEvents (Playerlist::getNeutral ());
  for (auto &it: *Playerlist::instance ())
    if (it->getType () == Player::NETWORKED)
      listenForLocalEvents (it);
}

bool GameServer::sendNextPlayer ()
{
  Glib::ustring s =
    String::ucompose ("%1", Playerlist::getActiveplayer ()->get_shield ());
  //now we can send the start round message, and begin the round ourselves.
  for (auto &i: participants)
    send (i->conn, MESSAGE_TYPE_NEXT_PLAYER, s);
  Participant *part = findParticipantByPlayerId
    (Playerlist::getActiveplayer ()->getId ());
  if (!part)
    return false;
  return true;
}

bool GameServer::nextTurn ()
{
  while (1)
    {
      Player *p = m_get_next_player.emit ();
      if (p)
        {
          if (p->getType () == Player::NETWORKED)
            {
              sendNextPlayer ();
              m_start_player_turn.emit (p);
              break; //now it goes to on_player_finished_turn if player avail.
            }
          else
            {
              sendNextPlayer ();
              if (p->getType () == Player::HUMAN)
                {
                  m_start_player_turn.emit (p);
                  break;
                }
              else if (p->getType () == Player::AI_DUMMY)
                {
                  m_start_player_turn.emit (p);
                  return true;
                }
              else
                m_start_player_turn.emit (p);
            }
        }
      else
        break;
    }
  return false;
}

bool GameServer::sendRoundStart ()
{
  sendTurnOrder ();
  //now we can send the start round message, and begin the round ourselves.
  for (auto &i: participants)
    send (i->conn, MESSAGE_TYPE_ROUND_START, "");
  m_round_begins.emit ();
  Playerlist::instance ()->setActiveplayer (NULL);
  return nextTurn ();
}

void GameServer::gotChat (void *conn, Glib::ustring message)
{
  Participant *part = findParticipantByConn (conn);
  if (part)
    {
      got_chat_message (part->profile_id, message);
      for (auto &i: participants)
        send (i->conn, MESSAGE_TYPE_CHATTED, part->profile_id + " " + message);
    }
  return;
}

void GameServer::saveMessages (Glib::ustring f)
{
  d_save_messages = f;
  std::ofstream logfile;
  logfile.open (d_save_messages);
  logfile.close ();
}

bool GameServer::onGotMessage (void *conn, int type, Glib::ustring payload)
{
  if (d_save_messages.empty () == false)
    {
      std::ofstream logfile;
      logfile.open (d_save_messages, std::ios_base::app);
      Participant *part = findParticipantByConn (conn);
      if (part)
        logfile << "<!-- received message type " << type << " from " <<
          getPeerHostName (conn) << part->profile_id << " -->" << std::endl;
      else
        logfile << "<!-- received message type " << type << " from" <<
          getPeerHostName (conn) << " -->" << std::endl;
      logfile << payload << std::endl;
      logfile.close ();
    }

  if (Lw::app->m_network_debug && type != MESSAGE_TYPE_PING)
    std::cerr <<
      String::ucompose (_("%1: got message of type %2"), Lw::get_prgname (),
                        NetworkConnection::typeToString (type)) << std::endl;

  switch (MessageType (type))
    {
    case MESSAGE_TYPE_PING:
      send (conn, MESSAGE_TYPE_PONG, "");
      break;

    case MESSAGE_TYPE_PONG:
      break;

    case MESSAGE_TYPE_SENDING_ACTIONS:
      gotRemoteActions (conn, payload);
      break;

    case MESSAGE_TYPE_SENDING_HISTORY:
      gotRemoteHistory (conn, payload);
      break;

    case MESSAGE_TYPE_PARTICIPANT_CONNECT:
      join (conn, payload);
      break;

    case MESSAGE_TYPE_REQUEST_SEAT_MANIFEST:
      sendChatRoster (conn);
      sendSeats (conn);
      if (gameHasBegun ())
        send (conn, MESSAGE_TYPE_GAME_BEGIN, "");
      break;

    case MESSAGE_TYPE_PARTICIPANT_DISCONNECT:
      depart (conn);
      break;

    case MESSAGE_TYPE_PARTICIPANT_CONNECTED:
      break;

    case MESSAGE_TYPE_CHAT:
      gotChat (conn, payload);
      break;

    case MESSAGE_TYPE_ROUND_OVER:
      //what do we do now?
      break;

    case MESSAGE_TYPE_LOBBY_ACTIVITY:
        {
          Shield::Color shield;
          Glib::ustring profile_id;
          gint32 action;
          bool reported;
          Glib::ustring data;
          bool success =
            get_message_lobby_activity (payload, shield, profile_id, action,
                                        reported, data);

          if (Lw::app->m_network_debug)
            std::cerr <<
              String::ucompose
              (_("%1: lobby activity message is of type %2"),
               Lw::get_prgname (),
               NetworkConnection::lobbyActionTypeToString (action)) <<
              std::endl;

          if (success)
            {
              if (reported == false) //player is /reporting/
                {
                  switch (action)
                    {
                    case LOBBY_MESSAGE_TYPE_SIT:
                      sit (conn,
                           Playerlist::instance ()->get (shield), data);
                      break;
                    case LOBBY_MESSAGE_TYPE_STAND:
                      stand (conn,
                             Playerlist::instance ()->get (shield), data);
                      break;
                    case LOBBY_MESSAGE_TYPE_CHANGE_NAME:
                      break;
                    case LOBBY_MESSAGE_TYPE_CHANGE_TYPE:
                      change_type (conn,
                                   Playerlist::instance ()->get (shield),
                                   atoi (data.c_str ()));
                      break;
                    default:
                      break;
                    }
                }
            }
        }
      break;

    case MESSAGE_TYPE_PARTICIPANT_DISCONNECTED:
      break;

    case MESSAGE_TYPE_KICK:
        {
          Participant *part = findParticipantByConn (conn);
          if (part)
            {
              if (Lobby::instance ()->user_is_mod (part->profile_id))
                kick (payload);
            }
        }
      break;

    case MESSAGE_TYPE_READY:
      record_client_ready (conn);
      sendWaitingForReady ();
      if (all_ready ())
        {
          sendGameBegin ();
          m_countdown_secs = m_total_secs_to_countdown;
          Glib::signal_timeout ().connect
            ([this] ()
             {
               m_countdown_secs--;
               sendCountdown ();
               return m_countdown_secs > 0;
             }, 1000);
        }
      break;

    case MESSAGE_TYPE_COUNTDOWN:
    case MESSAGE_TYPE_SERVER_DISCONNECT:
    case MESSAGE_TYPE_CHATTED:
    case MESSAGE_TYPE_SYSMSG:
    case MESSAGE_TYPE_TURN_ORDER:
    case MESSAGE_TYPE_KILL_PLAYER:
    case MESSAGE_TYPE_ROUND_START:
    case MESSAGE_TYPE_CHANGE_NICKNAME:
    case MESSAGE_TYPE_GAME_BEGIN:
    case MESSAGE_TYPE_OFF_PLAYER:
    case MESSAGE_TYPE_NEXT_PLAYER:
    case MESSAGE_TYPE_SAME_PROFILE_ID:
    case MESSAGE_TYPE_MOD_ID:
    case MESSAGE_TYPE_SENDING_MAP:
    case MESSAGE_TYPE_GAME_CAN_BEGIN:
    case MESSAGE_TYPE_WAITING_FOR_READY:
      break;

      //faulty client
      break;
    }
  return true;
}

void GameServer::onConnectionMade (void *conn)
{
  (void) conn;
  m_remote_participant_connected.emit ();
}

void GameServer::onConnectionLost (void *conn)
{
  if (Lw::app->m_network_debug)
    std::cerr <<
      String::ucompose (_("%1: connection lost"),
                        Lw::get_prgname ()) << std::endl;

  Participant *part = findParticipantByConn (conn);
  if (part)
    {
      std::list<GameParameters::Player> players_to_stand = part->players;

      depart (conn);
      participants.remove (part);

      //reserve seats if we stayed long enough
      remove_disconnected_participant (part->profile_id);
      part->disconnect_time = Glib::DateTime::create_now_local ();
      auto diff =
        part->disconnect_time.difference (part->connect_time) /
        G_TIME_SPAN_SECOND;
      if (diff >= m_min_join_time_to_reserve_seat &&
          part->players.empty () == false)
        disconnected_participants.push_back (part);
      else
        delete part;

      //tell everybode else that we've just stood up.
      for (auto &i: players_to_stand)
	notifyStand (Playerlist::instance ()->get (i.id), getProfileId (),
                     m_nickname);
      m_remote_participant_disconnected.emit ();
    }
  network_server->onConnectionLost (conn);
}

void GameServer::remove_disconnected_participant (Glib::ustring profile_id)
{
  disconnected_participants.remove_if
    ([profile_id](Participant* p)
     {
       if (p->profile_id == profile_id)
         {
           delete p;
           return true;
         }
       return false;
     });
}

Participant *GameServer::findParticipantByConn (void *conn)
{
  for (auto &i: participants)
    if (i->conn == conn)
      return i;

  return NULL;
}

void GameServer::onLocalNonNetworkedActionDone (NetworkAction *action)
{
  if (Lw::app->m_network_debug)
    {
      Glib::ustring desc = action->toString ();
      std::cerr <<
        String::ucompose (_("%1: Game Server got action: %2 description: %3"),
                            Lw::get_prgname (), Action::actionTypeToString
                            (action->getAction ()->getType ()), desc) <<
        std::endl;

    }
  if (action->getAction ()->getType () == Action::END_TURN)
    m_local_player_moved.emit (action->getOwner ());
  if (action->getAction ()->getType () == Action::INIT_TURN)
    m_local_player_starts_move.emit (action->getOwner ());

  for (auto &i: participants)
    {
      i->actions.push_back (new NetworkAction (action->getAction (),
                                               action->getOwnerId ()));
      sendActions (i);
    }
  for (auto &i: participants)
    clearNetworkActionlist (i->actions);

  delete action;
}

void GameServer::onActionDone (Action *a, guint32 id)
{
  if (d_stop)
    return;
  NetworkAction *action = new NetworkAction (a, id);
  Player *p = Playerlist::instance ()->get (action->getOwnerId ());
  if (p->getType () != Player::NETWORKED)
    onLocalNonNetworkedActionDone (action);

}

void GameServer::onLocalNonNetworkedHistoryDone (NetworkHistory *history)
{
  if (Lw::app->m_network_debug)
    {
      Glib::ustring desc = history->toString ();
      std::cerr <<
        String::ucompose (_("%1: Game Server got history: %2 %3"),
                          Lw::get_prgname (),
                          History::historyTypeToString
                          (history->getHistory ()->getType ()), desc) <<
        std::endl;
    }

  if (history->getHistory ()->getType () == History::PLAYER_VANQUISHED)
    m_local_player_died.emit (history->getOwner ());

  for (auto &i: participants)
    {
      i->histories.push_back (new NetworkHistory (history->getHistory (),
                                                  history->getOwnerId ()));
      sendHistories (i);
      clearNetworkHistorylist (i->histories);
    }
  delete history;
}

void GameServer::onLocalNetworkedHistoryDone (NetworkHistory *history)
{
  //okay we only care about two locally generated history events.
  Glib::ustring desc = history->toString ();

  for (auto &i: participants)
    {
      i->histories.push_back (new NetworkHistory (history->getHistory (),
                                                  history->getOwnerId ()));
      if (history->getHistory ()->getType () == History::GOLD_TOTAL ||
          history->getHistory ()->getType () == History::SCORE)
        {
          if (Lw::app->m_network_debug)
            std::cerr <<
              String::ucompose
              (_("%1: Game Server got locally generated networked history event: %2"),
               Lw::get_prgname (), desc) << std::endl;
          sendHistories (i);
        }
      else
        {
          if (Lw::app->m_network_debug)
            std::cerr <<
              String::ucompose
              (_("%1: Game Server got locally generated networked history event but not sending: %2"),
               Lw::get_prgname (), desc) << std::endl;
        }

      clearNetworkHistorylist (i->histories);
    }
  delete history;
}

void GameServer::onHistoryDone (History *h, guint32 id)
{
  NetworkHistory *history = new NetworkHistory (h, id);
  Player *p = Playerlist::instance ()->get (history->getOwnerId ());
  if (p->getType () != Player::NETWORKED)
    onLocalNonNetworkedHistoryDone (history);
  else
    onLocalNetworkedHistoryDone (history);
}

void GameServer::notifyJoin (Glib::ustring profile_id, Glib::ustring nickname)
{
  Lobby::instance ()->join (profile_id, nickname);
  m_remote_participant_joins.emit (profile_id, true);
  for (auto &i: participants)
    {
      send (i->conn, MESSAGE_TYPE_PARTICIPANT_CONNECTED,
            "1 " + profile_id + " " + nickname);
      send (i->conn, MESSAGE_TYPE_SYSMSG,
            getProfileId () + " " +
            String::ucompose (_("%1 connected."), nickname));
    }
  got_system_message
    (getProfileId (),
     String::ucompose (_("%1 connected."), nickname));

  reseat_disconnected_client (profile_id);
}

Participant * GameServer::find_disconnected_profile (Glib::ustring profile_id)
{
  for (auto &i: disconnected_participants)
    if (i->profile_id == profile_id)
      return i;

  return NULL;
}

void GameServer::notifyDepart (void *conn, Glib::ustring profile_id,
                               Glib::ustring nickname)
{
  m_remote_participant_departs.emit (profile_id);
  for (auto &i: participants)
    {
      if (i->conn == conn)
	continue;
      send (i->conn, MESSAGE_TYPE_PARTICIPANT_DISCONNECTED,
            profile_id + " " + nickname);
      send (i->conn, MESSAGE_TYPE_SYSMSG,
            getProfileId () + " " +
            String::ucompose (_("%1 disconnected."), nickname));
    }
  got_system_message
    (getProfileId (),
     String::ucompose (_("%1 disconnected."), nickname));
  Lobby::instance ()->depart (profile_id);
}

void GameServer::notifySit (Player *player, Glib::ustring profile_id,
                            Glib::ustring nickname)
{
  if (!player)
    return;
  Glib::ustring payload =
    String::ucompose
    ("%1 %2 %3 %4 %5", player->get_shield (), profile_id,
     LOBBY_MESSAGE_TYPE_SIT, 1, nickname);
  m_player_sits.emit (player->get_shield (), profile_id);

  for (auto &i: participants)
    {
      send (i->conn, MESSAGE_TYPE_LOBBY_ACTIVITY, payload);
      send (i->conn, MESSAGE_TYPE_SYSMSG,
            getProfileId () + " " +
            String::ucompose (_("%1 takes control of %2."), nickname,
                              player->getName ()));
    }
  got_system_message
    (getProfileId (),
     String::ucompose (_("%1 takes control of %2."),
                       nickname, player->getName ()));
  Lobby::instance ()->sit (player->get_shield (), profile_id);

  if (all_are_seated ())
    {
      sendGameCanBegin ();
      sendWaitingForReady ();
    }
}

void GameServer::notifyTypeChange (Player *player, Glib::ustring profile_id, int type)
{
  if (!player)
    return;
  Glib::ustring payload =
    String::ucompose ("%1 %2 %3 %4 %5", player->get_shield (), profile_id,
                      LOBBY_MESSAGE_TYPE_CHANGE_TYPE, 1, type);
  m_player_changes_type.emit (player, type);

  for (auto &i: participants)
    send (i->conn, MESSAGE_TYPE_LOBBY_ACTIVITY, payload);
}

Participant *GameServer::findParticipantByPlayerId (guint32 id)
{
  for (auto &i: participants)
    for (auto &j: i->players)
      if (j.id == id)
        return i;

  return NULL;
}

Participant *GameServer::findParticipantByNick (Glib::ustring nickname)
{
  for (auto &i: participants)
    if (i->nickname == nickname)
      return i;

  return NULL;
}

Glib::ustring GameServer::make_nickname_unique (Glib::ustring nickname)
{
  Glib::ustring new_nickname;
  int count = 0;
  //okay, does this nickname appear twice?
  for (auto &i: participants)
    if (i->nickname == nickname)
      count++;
  if (nickname == m_nickname)
    count++;
  if (count <= 1)
    return nickname;
  count = 2;
  while (1)
    {
      new_nickname = String::ucompose ("%1-%2", nickname, count);
      Participant *part = findParticipantByNick (new_nickname);
      if (!part)
        break;
      count++;
      if (count == 1000)
        break;
    }
  if (count == 1000)
    return "";
  return new_nickname;
}

void GameServer::join (void *conn, Glib::ustring profile_id_and_nickname)
{
  bool new_participant = false;

  if (Lw::app->m_network_debug)
    std::cerr << String::ucompose (_("%1: JOIN: %2"),
                                   Lw::get_prgname (),
                                   profile_id_and_nickname) << std::endl;

  size_t pos;
  pos = profile_id_and_nickname.rfind (' ');
  if (pos == Glib::ustring::npos)
    return;
  Glib::ustring profile_id = profile_id_and_nickname.substr (0, pos);
  Glib::ustring nickname = profile_id_and_nickname.substr (pos + 1);
  if (Lobby::instance ()->user_in_lobby (profile_id))
    {
      send (conn, MESSAGE_TYPE_SAME_PROFILE_ID, "");
      network_server->onConnectionLost (conn);
      return;
    }

  Participant *part = findParticipantByConn (conn);
  if (!part)
    {
      part = new Participant;
      part->conn = conn;
      part->nickname = nickname;
      part->profile_id = profile_id;
      participants.push_back (part);
      part->departed = false;
      part->connect_time = Glib::DateTime::create_now_local ();
      part->disconnect_time = {};
      part->ready = false;
      new_participant = true;
    }
  if (new_participant)
    sendMap (part);

  Glib::ustring new_nickname = make_nickname_unique (nickname);
  if (new_nickname != "")
    {
      notifyJoin (profile_id, new_nickname);
      if (new_nickname != nickname)
        {
          part->nickname = new_nickname;
          send (conn, MESSAGE_TYPE_CHANGE_NICKNAME,
                profile_id + " " + new_nickname);
          Lobby::instance ()->change_name (profile_id, new_nickname);
        }
    }
}

void GameServer::depart (void *conn)
{
  Participant *part = findParticipantByConn (conn);
  if (part && part->departed == false)
    {
      if (Lw::app->m_network_debug)
        std::cerr << String::ucompose (_("%1: DEPART: %2 %3"),
                                       Lw::get_prgname (), part->profile_id,
                                       part->nickname) <<std::endl;
      notifyDepart (conn, part->profile_id, part->nickname);
      part->departed = true;
    }
  //we don't delete the participant, it gets deleted when it disconnects.
  //see onConnectionLost
}

bool GameServer::player_already_sitting (Player *p)
{
  //check if the player p is already sitting down as a participant.
  for (auto &i: participants)
    for (auto &j: i->players)
      if (p->get_shield () == j.id)
        return true;
  return false;
}

void GameServer::sit (void *conn, Player *player, Glib::ustring nickname)
{
  if (!player || !conn)
    return;
  Participant *part = findParticipantByConn (conn);
  if (!part)
    return;

  if (Lw::app->m_network_debug)
    {
      auto shield = player->get_shield ();
      std::cerr << String::ucompose (_("%1: SIT: %2 %3"),
                                     Lw::get_prgname (),
                                     Shield::colorToString (shield),
                                     part->profile_id) << std::endl;
    }

  if (player_already_sitting (player) == true)
    return;

  if (seat_is_reserved (player->get_shield ()))
    {
      send_reserved_seat_message (conn, player);
      return;
    }

  //is this player already locally instantiated as an ai or human player?
  if (player->getType () != Player::NETWORKED)
    return;

  add_to_player_list
    (part->players, player->get_shield (), player->getName (),
     GameParameters::player_type_to_player_param (player->getType ()));

  if (player)
    dynamic_cast<NetworkPlayer*>(player)->setConnected (true);

  notifySit (player, part->profile_id, nickname);
}

void GameServer::change_type (void *conn, Player *player, int type)
{
  if (!player || !conn)
    return;
  Participant *part = findParticipantByConn (conn);
  if (!part)
    return;

  if (Lw::app->m_network_debug)
    {
      auto shield = player->get_shield ();
      std::cerr <<
        String::ucompose
        (_("%1: CHANGE TYPE: %2 %3 to %4"),
         Lw::get_prgname (), Shield::colorToString (shield), part->profile_id,
         GameParameters::player_param_to_string (type)) << std::endl;
    }

  update_player_type (part->players, player->get_shield (), (guint32) type);
  notifyTypeChange (player, part->profile_id, type);
}

void GameServer::notifyStand (Player *player, Glib::ustring profile_id,
                              Glib::ustring nickname)
{
  if (!player)
    return;
  Glib::ustring payload =
    String::ucompose ("%1 %2 %3 %4 %5", player->get_shield (),  profile_id,
                      LOBBY_MESSAGE_TYPE_STAND, 1, nickname);
  m_player_sits.emit (player->get_shield (), profile_id);

  for (auto &i: participants)
    {
      send (i->conn, MESSAGE_TYPE_LOBBY_ACTIVITY, payload);
      send (i->conn, MESSAGE_TYPE_SYSMSG,
            getProfileId () + " " +
            String::ucompose (_("%1 releases control of %2."), nickname,
                              player->getName ()));
    }
  got_system_message
    (getProfileId (),
     String::ucompose (_("%1 releases control of %2."), nickname,
                       player->getName ()));
  Lobby::instance ()->stand (player->get_shield ());
}

bool GameServer::update_player_type (std::list<GameParameters::Player> &list,
                                     guint32 id, guint32 type)
{
  bool found = false;
  for (auto &i: list)
    {
      if (i.id == id)
        {
          found = true;
          i.type = GameParameters::Player::Type (type);

          //special case, someone has turned off a player that had someone
          //sitting
          if (type == GameParameters::Player::OFF)
            Lobby::instance ()->stand (Shield::Color (i.id));
          break;
        }
    }
  return found;
}

bool GameServer::update_player_name (std::list<GameParameters::Player> &list,
                                     guint32 id, Glib::ustring name)
{
  bool found = false;
  for (auto &i: list)
    {
      if (i.id == id)
	{
	  found = true;
          i.name = name;
	  break;
	}
    }
  return found;
}

bool GameServer::add_to_player_list (std::list<GameParameters::Player> &list,
                                     guint32 id, Glib::ustring name,
                                     guint32 type)
{
  bool found = false;
  for (auto &i: list)
    {
      if (i.id == id)
	{
	  found = true;
          i.type = GameParameters::Player::Type (type);
          i.name = name;
	  break;
	}
    }
  if (found == false)
    {
      GameParameters::Player p = GameParameters::Player ();
      p.id = id;
      p.type = GameParameters::Player::Type (type);
      p.name = name;
      list.push_back (p);
    }
  return found;
}

bool GameServer::remove_from_player_list (std::list<GameParameters::Player> &list,
                                          guint32 id)
{
  //remove player id from part.
  for (auto i = list.begin (); i != list.end (); ++i)
    {
      if ((*i).id == id)
	{
	  list.erase (i);
	  return true;
	}
    }
  return false;
}

void GameServer::stand (void *conn, Player *player, Glib::ustring nickname)
{
  if (!player || !conn)
    return;

  Participant *part = findParticipantByConn (conn);
  if (!part)
    return;

  if (Lw::app->m_network_debug)
    {
      auto shield = player->get_shield ();
      std::cerr << String::ucompose (_("%1: STAND: %2 %3"),
                                     Lw::get_prgname (),
                                     Shield::colorToString (shield),
                                     part->profile_id) << std::endl;
    }
  //remove player id from part.
  bool found = remove_from_player_list (part->players, player->get_shield ());

  if (!found)
    //okay somebody's trying to boot another player.
    return;

  if (player && player->getType () == Player::NETWORKED)
    dynamic_cast<NetworkPlayer*>(player)->setConnected (false);
  notifyStand (player, part->profile_id, nickname);
}

void GameServer::gotRemoteActions (void *conn, const Glib::ustring &payload)
{
  gotActions (payload);
  for (auto &i: participants)
    if (i->conn != conn)
      send (i->conn, MESSAGE_TYPE_SENDING_ACTIONS, payload);
}

void GameServer::gotRemoteHistory (void *conn, const Glib::ustring &payload)
{
  gotHistories (payload);
  for (auto &i: participants)
    if (i->conn != conn)
      send (i->conn, MESSAGE_TYPE_SENDING_HISTORY, payload);
}

void GameServer::sendMap (Participant *part)
{
  // first hack the players so the player type we serialize is right
  std::vector<Player*> players;
  for (auto &i: *Playerlist::instance ())
    {
      bool connected = false;
      players.push_back (i);
      if (i->isComputer () == true)
	connected = true;
      NetworkPlayer *new_p = new NetworkPlayer (*i, true);
      new_p->setConnected (connected);
      Playerlist::instance ()->swap (i, new_p);
    }

  // send the map, and save it to a file somewhere temporarily
  Glib::ustring tmpfile = File::get_tmp_file ();
  File::erase (tmpfile);
  tmpfile += SAVE_EXT;

  d_game_scenario->saveGame (tmpfile);

  if (Lw::app->m_network_debug)
    std::cerr << String::ucompose (_("%1: sending map"),
                                   Lw::get_prgname ()) << std::endl;
  network_server->sendFile (part->conn, MESSAGE_TYPE_SENDING_MAP, tmpfile);

  //file gets erased in NetworkConnection::sendFileMessage

  // unhack the players
  std::vector<Player*> deletables;
  for (auto i : players)
    {
      Player *p = Playerlist::instance ()->get (i->getId ());
      deletables.push_back (p);
      Playerlist::instance ()->swap (p, i);
    }

  for (auto i : deletables)
    {
      NetworkPlayer *p = dynamic_cast<NetworkPlayer*>(i);
      delete p;
    }
}

void GameServer::sendActions (Participant *part)
{
  std::ostringstream os;
  XML_Helper helper (&os);

  helper.begin (String::ucompose ("%1", MESSAGE_PROTOCOL_VERSION));
  helper.open_tag ("actions");

  for (auto &i: part->actions)
    (*i).save (&helper);

  helper.close_tag ();

  send (part->conn, MESSAGE_TYPE_SENDING_ACTIONS, os.str ());
}

void GameServer::sendHistories (Participant *part)
{
  std::ostringstream os;
  XML_Helper helper (&os);

  helper.begin (String::ucompose ("%1", MESSAGE_PROTOCOL_VERSION));
  helper.open_tag ("histories");

  for (auto &i: part->histories)
    {
      if (Lw::app->m_network_debug)
        std::cerr <<
          String::ucompose
          (_("%1: sending history %2 from person %3 %4 to person %5"),
           Lw::get_prgname (),
           History::historyTypeToString (i->getHistory ()->getType ()),
           m_nickname, Playerlist::instance ()->get
           (i->getOwnerId ())->getName (), part->nickname) << std::endl;
      (*i).save (&helper);
    }

  helper.close_tag ();

  send (part->conn, MESSAGE_TYPE_SENDING_HISTORY, os.str ());
}

bool GameServer::dumpActionsAndHistories (XML_Helper *helper, Player *player)
{
  Participant *part = NULL;
  for (auto &i: participants)
    {
      bool found = false;
      for (auto &it: i->players)
	{
	  if (it.id == player->getId ())
	    {
	      found = true;
	      break;
	    }
	}

      if (found)
	{
	  part = i;
	  break;
	}
    }
  if (part == NULL)
    return false;
  for (auto &i: part->histories)
    (*i).save (helper);
  for (auto &i: part->actions)
    (*i).save (helper);
  return true;
}

bool GameServer::dumpActionsAndHistories (XML_Helper *helper)
{
  Player *player = Playerlist::getActiveplayer ();
  return dumpActionsAndHistories (helper, player);
}

void GameServer::sit_down (Shield::Color shield, Glib::ustring profile_id)
{
  Player *player = Playerlist::instance ()->get (shield);
  if (!player)
    return;
  if (player->getType () == Player::NETWORKED)
    {
      //alright, we want to sit down as this player
      //convert the network player to a human player
      dynamic_cast<NetworkPlayer*>(player)->setConnected (true);
      RealPlayer *new_p = new RealPlayer (*player, true);
      Playerlist::instance ()->swap (player, new_p);
      stopListeningForLocalEvents (player);
      listenForLocalEvents (new_p);
      delete player;
      player = new_p;
      add_to_player_list
        (players_seated_locally, new_p->get_shield (), new_p->getName (),
         GameParameters::player_type_to_player_param (new_p->getType ()));
      notifySit (new_p, profile_id, m_nickname);
    }
  else if (player->getType () == Player::HUMAN)
    {
      //alright, we want to sit down as this player
      //wait, we're already human.
      // do nothing
    }
  else // an ai player
    {
      stopListeningForLocalEvents (player);
      listenForLocalEvents (player);
      add_to_player_list
        (players_seated_locally, player->get_shield (), player->getName (),
         GameParameters::player_type_to_player_param (player->getType ()));
      notifySit (player, profile_id, m_nickname);
    }
  notifyTypeChange (player, profile_id, player->getType ());
}

void GameServer::stand_up (Shield::Color shield, Glib::ustring profile_id)
{
  Player *player = Playerlist::instance ()->get (shield);
  if (!player)
    return;
  if (player->getType () == Player::HUMAN)
    {
      //alright, we want to stand up as this player
      //convert the player from a human player back to a network player

      NetworkPlayer *new_p = new NetworkPlayer (*player, true);
      Playerlist::instance ()->swap (player, new_p);
      stopListeningForLocalEvents (player);
      delete player;
      listenForLocalEvents (new_p);
      player = new_p;
      new_p->setConnected (false);
      notifyStand (new_p, profile_id, m_nickname);
      remove_from_player_list (players_seated_locally, new_p->get_shield ());
    }
  else if (player->getType () == Player::NETWORKED)
    {
      //this is the forcibly booted, made to stand-up case. .
      Participant *part = findParticipantByPlayerId (player->getId ());
      stand (part->conn, player, part->nickname);
    }
  else // an ai player
    {
      stopListeningForLocalEvents (player);
      remove_from_player_list (players_seated_locally, player->get_shield ());
      notifyStand (player, profile_id, m_nickname);
    }
  notifyTypeChange (player, profile_id, GameParameters::Player::NETWORKED);
}

void GameServer::type_change (Player *player, int type)
{
  if (!player)
    return;
  update_player_type (players_seated_locally, player->get_shield (), (guint32) type);
  Glib::ustring profile_id = "";
  for (auto &i: participants)
    {
      for (auto p : i->players)
        {
          if (p.id == player->get_shield ())
            {
              profile_id = i->profile_id;
              break;
            }
        }
      if (profile_id != "")
        break;
    }
  notifyTypeChange (player, profile_id, type);
  m_player_changes_type.emit (player, type);
}

void GameServer::chat (Glib::ustring message)
{
  notifyChat (message);
}

void GameServer::notifyChat (Glib::ustring message)
{
  got_chat_message (getProfileId (), message);
  for (auto &i: participants)
    send (i->conn, MESSAGE_TYPE_CHATTED, getProfileId () + " " + message);
}

void GameServer::sendSeat (void *conn, GameParameters::Player player,
                           Glib::ustring profile_id)
{
  Glib::ustring payload =
    String::ucompose ("%1 %2 %3 %4 %5", player.id, profile_id,
                      LOBBY_MESSAGE_TYPE_SIT, 1,
                      Lobby::instance ()->get_name (profile_id));
  send (conn, MESSAGE_TYPE_LOBBY_ACTIVITY, payload);

  payload = String::ucompose ("%1 %2 %3 %4 %5", player.id, profile_id,
                              LOBBY_MESSAGE_TYPE_CHANGE_TYPE, 1, player.type);
  send (conn, MESSAGE_TYPE_LOBBY_ACTIVITY, payload);

  payload = String::ucompose ("%1 %2 %3 %4 %5", player.id, profile_id,
                              LOBBY_MESSAGE_TYPE_CHANGE_NAME, 1,
                              player.name);
  send (conn, MESSAGE_TYPE_LOBBY_ACTIVITY, payload);
}

void GameServer::sendSeats (void *conn)
{
  Participant *part = findParticipantByConn (conn);
  if (!part)
    return;
  //send seatedness info for remote participants

  for (auto &i: participants)
    {
      if (i->conn == part->conn)
	continue;

      for (auto &j: i->players)
        sendSeat (conn, j, i->profile_id);
    }
  //send out seatedness info for local server
  for (auto &j: players_seated_locally)
    sendSeat (conn, j, getProfileId ());
}

void GameServer::sendChatRoster (void *conn)
{
  Participant *part = findParticipantByConn (conn);
  if (!part)
    return;
  for (auto &i: participants)
    {
      if (i->conn == part->conn)
	continue;
      send (part->conn, MESSAGE_TYPE_PARTICIPANT_CONNECTED,
            "0 " + i->profile_id + " " + i->nickname);
    }

  send (part->conn, MESSAGE_TYPE_PARTICIPANT_CONNECTED,
        "1 " + getProfileId () + " " + m_nickname);

  // we're a mod for sure
  send (part->conn, MESSAGE_TYPE_MOD_ID,
        getProfileId ());

  // and a participant might also be a mod
  for (auto &i: participants)
    {
      if (i->conn == part->conn)
	continue;
      if (Lobby::instance ()->user_is_mod (i->profile_id))
        send (part->conn, MESSAGE_TYPE_MOD_ID, i->profile_id);
    }
}

void GameServer::sendOffPlayer (Player *p)
{
  std::stringstream player;
  player << (int)p->get_shield ();
  for (auto &i: participants)
    send (i->conn, MESSAGE_TYPE_OFF_PLAYER, player.str ());

  m_remote_player_died.emit (p);
}

void GameServer::sendKillPlayer (Player *p)
{
  std::stringstream player;
  player << (int)p->get_shield ();
  for (auto &i: participants)
    send (i->conn, MESSAGE_TYPE_KILL_PLAYER, player.str ());

  m_remote_player_died.emit (p);
}

void GameServer::sendTurnOrder ()
{
  std::list<guint32> ids;
  std::stringstream players;
  for (auto &it: *Playerlist::instance ())
    {
      players << (int) it->get_shield () << " ";
      ids.push_back (it->get_shield ());
    }
  for (auto &i: participants)
    send (i->conn, MESSAGE_TYPE_TURN_ORDER, players.str ());
  m_playerlist_reorder_received.emit ();
}

bool GameServer::gameHasBegun ()
{
  return d_game_has_begun;
}

void GameServer::sendGameCanBegin ()
{
  for (auto &i: participants)
    send (i->conn, MESSAGE_TYPE_GAME_CAN_BEGIN, "");
  m_game_can_begin.emit ();
}

void GameServer::sendGameBegin ()
{
  d_game_has_begun = true;
  syncLocalPlayers ();
  for (auto &i: participants)
    send (i->conn, MESSAGE_TYPE_GAME_BEGIN, "");
}

void GameServer::syncLocalPlayers ()
{
  std::list<guint32> ids;
  for (auto &j: players_seated_locally)
    {
      Player *p = Playerlist::instance ()->get (j.id);
      auto as = Armysetlist::instance ()->get (p->getArmyset ());
      if (j.type == GameParameters::Player::OFF)
        {
          stopListeningForLocalEvents (p);
          m_player_gets_turned_off.emit (p);
          sendOffPlayer (p);
          ids.push_back (p->getId ());
          Playerlist::instance ()->syncPlayer (j, as->getBaseName ());
        }
      else
        {
          stopListeningForLocalEvents (p);
          Playerlist::instance ()->syncPlayer (j, as->getBaseName ());
          listenForLocalEvents (Playerlist::instance ()->get (j.id));
        }
    }
  for (auto &i: ids)
    remove_from_player_list (players_seated_locally, i);
}

void GameServer::on_turn_aborted ()
{
  d_stop = true;
  remove_all_participants ();
}

void GameServer::send (void *conn, int type, Glib::ustring payload)
{
  if (d_save_messages.empty () == false)
    {
      std::ofstream logfile;
      logfile.open (d_save_messages, std::ios_base::app);
      Participant *part = findParticipantByConn (conn);
      if (part)
        logfile << "<!-- sent message type " << type << " to " <<
          getPeerHostName (conn) << part->profile_id << " -->" << std::endl;
      else
        logfile << "<!-- sent message type " << type << " to " <<
          getPeerHostName (conn) << " -->" << std::endl;
      logfile << payload << std::endl;
      logfile.close ();
    }
  network_server->send (conn, type, payload);
}

Glib::ustring GameServer::getPeerHostName (void *conn)
{
  return network_server->get_hostname (conn);
}

void GameServer::add_mod (Glib::ustring id)
{
  Lobby::instance ()->add_mod (id);
  for (auto &i: participants)
    send (i->conn, MESSAGE_TYPE_MOD_ID, id);
}

void GameServer::kick (Glib::ustring profile_id)
{
  for (auto &i: participants)
    {
      if (i->profile_id == profile_id &&
          Lobby::instance ()->user_is_mod (profile_id) == false)
        {
          NetworkConnection *c = reinterpret_cast<NetworkConnection*>(i->conn);
          c->disconnect ();
          onConnectionLost (i->conn);
          remove_disconnected_participant (profile_id);
          break;
        }
    }
}

void GameServer::reseat_disconnected_client (Glib::ustring profile_id)
{
  auto part = find_disconnected_profile (profile_id);
  if (part)
    {
      Glib::DateTime now = Glib::DateTime::create_now_local ();
      auto diff = now.difference (part->disconnect_time) / G_TIME_SPAN_SECOND;
      if (diff <= m_time_to_hold_reserved_seat)
        {
          auto oldplayers = part->players;
          part->players.clear ();
          for (auto i : oldplayers)
            {
              Shield::Color shield = Shield::Color (i.id);
              if (Lobby::instance ()->get_profile_id (shield) == "")
                {
                  part->players.push_back (i);
                  part->connect_time = Glib::DateTime::create_now_local ();
                  part->disconnect_time = {};

                  auto player = Playerlist::instance ()->get (shield);
                  if (player)
                    dynamic_cast<NetworkPlayer*>(player)->setConnected (true);
                  notifySit (player, part->profile_id, part->nickname);
                }
            }
        }
      remove_disconnected_participant (part->profile_id);
    }
}

bool GameServer::seat_is_reserved (Shield::Color shield)
{
  for (auto &i: disconnected_participants)
    {
      for (auto &j: i->players)
        {
          if (j.id == (int)shield)
            {
              Glib::DateTime now = Glib::DateTime::create_now_local ();
              auto limit = m_time_to_hold_reserved_seat;
              auto seconds = now.difference (i->disconnect_time) / G_TIME_SPAN_SECOND;
              if (seconds <= limit)
                return true;
            }
        }
    }
  return false;
}
      
void GameServer::send_reserved_seat_message (void *conn, Player *player)
{
  int secs = get_reservation_duration (player);
  send
    (conn, MESSAGE_TYPE_CHATTED,
     String::ucompose
     (ngettext
      ("%1 Cannot take control of %2 because it's reserved for %3 more second.",
       "%1 Cannot take control of %2 because it's reserved for %3 more seconds.",
       secs),
      getProfileId (), player->getName (), secs));
}

int GameServer::get_reservation_duration (Player *player)
{
  auto shield = player->get_shield ();
  for (auto &i: disconnected_participants)
    {
      for (auto &j: i->players)
        {
          if (j.id == (int)shield)
            {
              Glib::DateTime now = Glib::DateTime::create_now_local ();
              auto limit = m_time_to_hold_reserved_seat;
              auto seconds = now.difference (i->disconnect_time) / G_TIME_SPAN_SECOND;
              return limit - seconds;
            }
        }
    }
  return 0;
}

bool GameServer::all_are_seated ()
{
  for (auto p : *Playerlist::instance ())
    {
      if (p == Playerlist::getNeutral ())
        continue;
      if (player_already_sitting (p) == false)
        return false;
    }
  return true;
}

void GameServer::sendWaitingForReady ()
{
  guint32 num_waiting = get_num_waiting ();
  Glib::ustring payload = String::ucompose ("%1", num_waiting);
  for (auto &i: participants)
    send (i->conn, MESSAGE_TYPE_WAITING_FOR_READY, payload);
  m_waiting_for_ready.emit (num_waiting);
}

void GameServer::sendCountdown ()
{
  Glib::ustring payload = String::ucompose ("%1", m_countdown_secs);
  for (auto &i: participants)
    send (i->conn, MESSAGE_TYPE_COUNTDOWN, payload);
  m_countdown.emit (m_countdown_secs);
}

guint32 GameServer::get_num_waiting ()
{
  guint32 count = 0;
  for (auto &i: participants)
    {
      if (i->players.empty () == false &&
          i->ready == false)
        count++;
    }
  return count;
}

void GameServer::record_client_ready (void *conn)
{
  Participant *part = findParticipantByConn (conn);
  if (part)
    part->ready = true;
}

bool GameServer::all_ready ()
{
  return get_num_waiting () == 0;
}

bool GameServer::player_is_ready (Player *p)
{
  for (auto &i: participants)
    {
      if (i->ready)
        {
          for (auto &j: i->players)
            if (p->get_shield () == j.id)
              return true;
        }
    }
  return false;
}
