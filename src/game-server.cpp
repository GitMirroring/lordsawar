// Copyright (C) 2008 Ole Laursen
// Copyright (C) 2008, 2011, 2014, 2015, 2017, 2020, 2021 Ben Asselstine
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
#include <fstream>
#include <list>
#include "game-parameters.h"
#include "game-server.h"

#include "File.h"
#include "network-server.h"
#include "game.h"
#include "xmlhelper.h"
#include "GameScenario.h"
#include "playerlist.h"
#include "player.h"
#include "network-action.h"
#include "network-history.h"
#include "Configuration.h"
#include "network_player.h"
#include "real_player.h"
#include "GameScenarioOptions.h"
#include "ucompose.hpp"

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
};

GameServer * GameServer::s_instance = 0;

GameServer* GameServer::getInstance()
{
    if (s_instance == 0)
        s_instance = new GameServer();

    return s_instance;
}

void GameServer::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}


GameServer::GameServer()
{
  d_game_has_begun = false;

  remote_player_moved.connect
    (sigc::mem_fun(*this, &GameServer::on_player_finished_turn));
  local_player_moved.connect
    (sigc::mem_fun(*this, &GameServer::on_player_finished_turn));
  d_stop = false;
  d_save_messages = "";
}

void GameServer::notifyRoundOver()
{
  for (auto &i: participants)
    send(i->conn, MESSAGE_TYPE_ROUND_OVER, "");
}

bool GameServer::check_end_of_round()
{
  if (Playerlist::getInstance()->getNeutral()->hasAlreadyEndedTurn())
    {
      Playerlist::getInstance()->getNeutral()->clearActionlist();
      notifyRoundOver();
      round_ends.emit();
      return true;
    }
  return false;
}

void GameServer::on_player_finished_turn(Player *player)
{
  if (check_end_of_round() == false)
    {
      //if the end of turn is asynchronous, start a new turn from here
      //otherwise, just fall through back to the nextTurn method, and it's 
      //inner loop.
      if (player->getType() == Player::HUMAN ||
          player->getType() == Player::NETWORKED)
        {
          if (nextTurn())
            on_player_finished_turn(Playerlist::getInstance()->getNeutral());
        }
    }
}

void GameServer::remove_all_participants()
{
  stopListeningForLocalEvents();
  //say goodbye to all participants
  for (auto &i: participants)
    send(i->conn, MESSAGE_TYPE_SERVER_DISCONNECT, "bye");
  for (auto &i: participants)
    delete i;
  participants.clear();
  players_seated_locally.clear();
}

GameServer::~GameServer()
{
  if (network_server.get() != NULL)
    {
      if (network_server->isListening())
        network_server->stop();
    }
}

bool GameServer::isListening()
{
  if (network_server.get() != NULL)
    return network_server->isListening();
  else
    return false;
}

void GameServer::start(GameScenario *game_scenario, int port, Glib::ustring profile_id, Glib::ustring nick)
{
  setGameScenario(game_scenario);
  setNickname(nick);
  setProfileId(profile_id);

  if (network_server.get() != NULL && network_server->isListening())
    return;
  network_server.reset(new NetworkServer());
  network_server->port_in_use.connect
    (sigc::mem_fun(port_in_use, &sigc::signal<void, int>::emit));
  network_server->got_message.connect
    (sigc::mem_fun(this, &GameServer::onGotMessage));
  network_server->connection_lost.connect
    (sigc::mem_fun(this, &GameServer::onConnectionLost));
  network_server->connection_made.connect
    (sigc::mem_fun(this, &GameServer::onConnectionMade));

  network_server->startListening(port);

  listenForLocalEvents(Playerlist::getInstance()->getNeutral());
  for (auto &it: *Playerlist::getInstance())
    if (it->getType() == Player::NETWORKED)
      listenForLocalEvents(it);
}

bool GameServer::sendNextPlayer()
{
  Glib::ustring s = 
    String::ucompose("%1", Playerlist::getActiveplayer()->getId());
  //now we can send the start round message, and begin the round ourselves.
  for (auto &i: participants)
    send(i->conn, MESSAGE_TYPE_NEXT_PLAYER, s);
  Participant *part = findParticipantByPlayerId
    (Playerlist::getActiveplayer()->getId());
  if (!part)
    return false;
  return true;
}

bool GameServer::nextTurn()
{
  while (1)
    {
      Player *p = get_next_player.emit();
      if (p)
        {
          if (p->getType() == Player::NETWORKED)
            {
              sendNextPlayer();
              start_player_turn.emit(p);
              break; //now it goes to on_player_finished_turn if player avail.
            }
          else
            {
              sendNextPlayer();
              if (p->getType() == Player::HUMAN)
                {
                  start_player_turn.emit(p);
                  break;
                }
              else if (p->getType() == Player::AI_DUMMY)
                {
                  start_player_turn.emit(p);
                  return true;
                }
              else
                start_player_turn.emit(p);
            }
        }
      else
        break;
    }
  return false;
}

bool GameServer::sendRoundStart()
{
  sendTurnOrder();
  //now we can send the start round message, and begin the round ourselves.
  for (auto &i: participants)
    send(i->conn, MESSAGE_TYPE_ROUND_START, "");
  round_begins.emit();
  Playerlist::getInstance()->setActiveplayer(NULL);
  return nextTurn();
}

void GameServer::gotChat(void *conn, Glib::ustring message)
{
  Participant *part = findParticipantByConn(conn);
  if (part)
    {
      gotChatMessage(part->nickname, message);
      for (auto &i: participants)
        send(i->conn, MESSAGE_TYPE_CHATTED, message);
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

bool GameServer::onGotMessage(void *conn, int type, Glib::ustring payload)
{
  if (d_save_messages.empty () == false)
    {
      std::ofstream logfile;
      logfile.open(d_save_messages, std::ios_base::app);
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
  //std::cerr << "got message of type " << type << std::endl;
  switch (MessageType(type))
    {
    case MESSAGE_TYPE_PING:
      //std::cerr << "sending pong" << std::endl;
      send (conn, MESSAGE_TYPE_PONG, "");
      break;

    case MESSAGE_TYPE_PONG:
      break;

    case MESSAGE_TYPE_SENDING_ACTIONS:
      gotRemoteActions (conn, payload);
      break;

    case MESSAGE_TYPE_SENDING_MAP:
      // should never occur
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
        send (conn, MESSAGE_TYPE_GAME_MAY_BEGIN, "");
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
          guint32 id;
          gint32 action;
          bool reported;
          Glib::ustring data;
          bool success = 
            get_message_lobby_activity (payload, id, action, reported, data);
          if (success)
            {
              if (reported == false) //player is /reporting/
                {
                  switch (action)
                    {
                    case LOBBY_MESSAGE_TYPE_SIT:
                      sit (conn,
                           Playerlist::getInstance ()->getPlayer (id), data);
                      break;
                    case LOBBY_MESSAGE_TYPE_STAND:
                      stand (conn,
                             Playerlist::getInstance ()->getPlayer (id), data);
                      break;
                    case LOBBY_MESSAGE_TYPE_CHANGE_NAME:
                      break;
                    case LOBBY_MESSAGE_TYPE_CHANGE_TYPE:
                      change_type (conn, 
                                   Playerlist::getInstance ()->getPlayer (id), 
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

    case MESSAGE_TYPE_SERVER_DISCONNECT:
    case MESSAGE_TYPE_CHATTED:
    case MESSAGE_TYPE_TURN_ORDER:
    case MESSAGE_TYPE_KILL_PLAYER:
    case MESSAGE_TYPE_ROUND_START:
    case MESSAGE_TYPE_CHANGE_NICKNAME:
    case MESSAGE_TYPE_GAME_MAY_BEGIN:
    case MESSAGE_TYPE_OFF_PLAYER:
    case MESSAGE_TYPE_NEXT_PLAYER:
      //faulty client
      break;
    }
  return true;
}

void GameServer::onConnectionMade(void *conn)
{
  (void) conn;
  remote_participant_connected.emit();
}

void GameServer::onConnectionLost(void *conn)
{
  std::cerr << "connection lost" << std::endl;

  Participant *part = findParticipantByConn(conn);
  if (part)
    {
      std::list<GameParameters::Player> players_to_stand = part->players;

      depart(conn);
      participants.remove(part);
      //tell everybode else that we've just stood up.
      for (auto &i: players_to_stand)
	notifyStand(Playerlist::getInstance()->getPlayer(i.id), d_nickname);
      remote_participant_disconnected.emit();
      delete part;
    }
  network_server->onConnectionLost(conn);
}

Participant *GameServer::findParticipantByConn(void *conn)
{
  for (auto &i: participants)
    if (i->conn == conn)
      return i;

  return NULL;
}

void GameServer::onLocalNonNetworkedActionDone(NetworkAction *action)
{
  Glib::ustring desc = action->toString();
  std::cerr << String::ucompose("Game Server got action: %1 description: %2", Action::actionTypeToString(action->getAction()->getType()), desc) << std::endl;

  if (action->getAction()->getType() == Action::END_TURN)
    local_player_moved.emit(action->getOwner());
  if (action->getAction()->getType() == Action::INIT_TURN)
    local_player_starts_move.emit(action->getOwner());

  for (auto &i: participants)
    {
      i->actions.push_back(new NetworkAction (action->getAction(),
                                              action->getOwnerId()));
      sendActions(i);
    }
  for (auto &i: participants)
    clearNetworkActionlist(i->actions);

  //do it here.
  delete action;
}

void GameServer::onActionDone(Action *a, guint32 id)
{
  if (d_stop)
    return;
  NetworkAction *action = new NetworkAction (a, id);
  Player *p = Playerlist::getInstance()->getPlayer(action->getOwnerId());
  if (p->getType() != Player::NETWORKED)
    onLocalNonNetworkedActionDone(action);

}

void GameServer::onLocalNonNetworkedHistoryDone(NetworkHistory *history)
{
  Glib::ustring desc = history->toString();
  std::cerr << String::ucompose("Game Server got history: %1 %2", History::historyTypeToString(history->getHistory()->getType()), desc) << std::endl;

  if (history->getHistory()->getType() == History::PLAYER_VANQUISHED)
    local_player_died(history->getOwner());

  for (auto &i: participants)
    {
      i->histories.push_back (new NetworkHistory (history->getHistory(), 
                                                  history->getOwnerId()));
      sendHistories(i);
      clearNetworkHistorylist(i->histories);
    }
  delete history;
}

void GameServer::onLocalNetworkedHistoryDone(NetworkHistory *history)
{
  //okay we only care about two locally generated history events.
  Glib::ustring desc = history->toString();

  for (auto &i: participants)
    {
      i->histories.push_back (new NetworkHistory (history->getHistory(), 
                                                  history->getOwnerId()));
      if (history->getHistory()->getType() == History::GOLD_TOTAL ||
          history->getHistory()->getType() == History::SCORE)
        {
          std::cerr << String::ucompose("Game Server got locally generated networked history event: %1", desc) << std::endl;
          sendHistories(i);
        }
      else
        {
          std::cerr << String::ucompose("Game Server got locally generated networked history event but not sending: %1", desc) << std::endl;
        }

      clearNetworkHistorylist(i->histories);
    }
  delete history;
}

void GameServer::onHistoryDone(History *h, guint32 id)
{
  NetworkHistory *history = new NetworkHistory(h, id);
  Player *p = Playerlist::getInstance()->getPlayer(history->getOwnerId());
  if (p->getType() != Player::NETWORKED)
    onLocalNonNetworkedHistoryDone(history);
  else
    onLocalNetworkedHistoryDone(history);
}

void GameServer::notifyJoin(Glib::ustring nickname)
{
  remote_participant_joins.emit(nickname);
  for (auto &i: participants)
    send(i->conn, MESSAGE_TYPE_PARTICIPANT_CONNECTED, nickname);
  gotChatMessage("[server]", String::ucompose (_("%1 connected."), nickname));
}

void GameServer::notifyDepart(void *conn, Glib::ustring nickname)
{
  remote_participant_departs.emit(nickname);
  for (auto &i: participants)
    {
      if (i->conn == conn)
	continue;
      send(i->conn, MESSAGE_TYPE_PARTICIPANT_DISCONNECTED, nickname);
      send(i->conn, MESSAGE_TYPE_CHATTED,
           String::ucompose (_("%1 disconnected."), nickname));
    }
  gotChatMessage("", String::ucompose (_("%1 disconnected"), nickname));
}

void GameServer::notifySit(Player *player, Glib::ustring nickname)
{
  if (!player)
    return;
  Glib::ustring payload = 
    String::ucompose("%1 %2 %3 %4", player->getId(), LOBBY_MESSAGE_TYPE_SIT, 
                     1, nickname);
  player_sits.emit(player, nickname);

  for (auto &i: participants)
    {
      send(i->conn, MESSAGE_TYPE_LOBBY_ACTIVITY, payload);
      send(i->conn, MESSAGE_TYPE_CHATTED, nickname + " assumes control of " + 
           player->getName() +".");
    }
  gotChatMessage("", nickname + " assumes control of " + 
		 player->getName() +".");
}

void GameServer::notifyTypeChange(Player *player, int type)
{
  if (!player)
    return;
  Glib::ustring payload = 
    String::ucompose("%1 %2 %3 %4", player->getId(), 
                     LOBBY_MESSAGE_TYPE_CHANGE_TYPE, 1, type);
  player_changes_type.emit(player, type);

  for (auto &i: participants)
    send(i->conn, MESSAGE_TYPE_LOBBY_ACTIVITY, payload);
}

Participant *GameServer::findParticipantByPlayerId(guint32 id)
{
  for (auto &i: participants)
    for (auto &j: i->players)
      if (j.id == id)
        return i;

  return NULL;
}

Participant *GameServer::findParticipantByNick(Glib::ustring nickname)
{
  for (auto &i: participants)
    if (i->nickname == nickname)
      return i;

  return NULL;
}

Glib::ustring GameServer::make_nickname_unique(Glib::ustring nickname)
{
  Glib::ustring new_nickname;
  int count = 0;
  //okay, does this nickname appear twice?
  for (auto &i: participants)
    if (i->nickname == nickname)
      count++;
  if (nickname == d_nickname)
    count++;
  if (count <= 1)
    return nickname;
  count = 2;
  while (1)
    {
      new_nickname = String::ucompose("%1-%2", nickname, count);
      Participant *part = findParticipantByNick(new_nickname);
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

void GameServer::join(void *conn, Glib::ustring nickname_and_profile_id)
{
  bool new_participant = false;
  std::cout << "JOIN: " << conn << std::endl;

  size_t pos;
  pos = nickname_and_profile_id.rfind(' ');
  if (pos == Glib::ustring::npos)
    return;
  Glib::ustring nickname = nickname_and_profile_id.substr(0, pos);
  Glib::ustring profile_id = nickname_and_profile_id.substr(pos + 1);
  Participant *part = findParticipantByConn(conn);
  if (!part) {
    part = new Participant;
    part->conn = conn;
    part->nickname = nickname;
    part->profile_id = profile_id;
    participants.push_back(part);
    part->departed = false;
    new_participant = true;
  }
  if (new_participant)
    sendMap(part);

  Glib::ustring new_nickname = make_nickname_unique(nickname);
  if (new_nickname != "")
    {
      if (new_nickname != nickname)
        {
          part->nickname = new_nickname;
          send(conn, MESSAGE_TYPE_CHANGE_NICKNAME, new_nickname);
        }
      notifyJoin(new_nickname);
    }
}

void GameServer::depart(void *conn)
{
  Participant *part = findParticipantByConn(conn);
  if (part && part->departed == false)
    {
      std::cout << "DEPART: " << conn << std::endl;
      notifyDepart(conn, part->nickname);
      part->departed = true;
    }
  //we don't delete the participant, it gets deleted when it disconnects.
  //see onConnectionLost
}

bool GameServer::player_already_sitting(Player *p)
{
  //check if the player p is already sitting down as a participant.
  for (auto &i: participants)
    for (auto &j: i->players)
      if (p->getId() == j.id)
        return true;
  return false;
}

void GameServer::sit(void *conn, Player *player, Glib::ustring nickname)
{
  std::cout << "SIT: " << conn << " " << player << std::endl;

  if (!player || !conn)
    return;
  Participant *part = findParticipantByConn(conn);
  if (!part) 
    return;

  if (player_already_sitting(player) == true)
    return;

  //is this player already locally instantiated as an ai or human player?
  if (player->getType() != Player::NETWORKED)
    return;

  add_to_player_list(part->players, player->getId(), player->getName(),
                     GameParameters::player_type_to_player_param(player->getType()));

  if (player)
    dynamic_cast<NetworkPlayer*>(player)->setConnected(true);

  notifySit(player, nickname);
}

void GameServer::change_type(void *conn, Player *player, int type)
{
  std::cout << "CHANGE TYPE: " << conn << " " << player << " " << type << std::endl;

  if (!player || !conn)
    return;
  Participant *part = findParticipantByConn(conn);
  if (!part) 
    return;

  update_player_type (part->players, player->getId(), (guint32) type);
  notifyTypeChange(player, type);
}

void GameServer::notifyStand(Player *player, Glib::ustring nickname)
{
  if (!player)
    return;
  Glib::ustring payload = 
    String::ucompose("%1 %2 %3 %4", player->getId(), 
                     LOBBY_MESSAGE_TYPE_STAND, 1, nickname);
  player_stands.emit(player, nickname);

  for (auto &i: participants)
    {
      send(i->conn, MESSAGE_TYPE_LOBBY_ACTIVITY, payload);
      send(i->conn, MESSAGE_TYPE_CHATTED, 
           nickname + " relinquishes control of " + player->getName() +".");
    }
  gotChatMessage("", nickname + " relinquishes control of " + 
		 player->getName() +".");
}

bool
GameServer::update_player_type (std::list<GameParameters::Player> &list, guint32 id, guint32 type)
{
  bool found = false;
  for (auto &i: list)
    {
      if (i.id == id)
	{
	  found = true;
          i.type = GameParameters::Player::Type(type);
	  break;
	}
    }
  return found;
}

bool
GameServer::update_player_name (std::list<GameParameters::Player> &list, guint32 id, Glib::ustring name)
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

bool
GameServer::add_to_player_list(std::list<GameParameters::Player> &list, guint32 id, Glib::ustring name, guint32 type)
{
  bool found = false;
  for (auto &i: list)
    {
      if (i.id == id)
	{
	  found = true;
          i.type = GameParameters::Player::Type(type);
          i.name = name;
	  break;
	}
    }
  if (found == false)
    {
      GameParameters::Player p = GameParameters::Player ();
      p.id = id;
      p.type = GameParameters::Player::Type(type);
      p.name = name;
      list.push_back(p);
    }
  return found;
}

bool
GameServer::remove_from_player_list(std::list<GameParameters::Player> &list, guint32 id)
{
  //remove player id from part.
  for (std::list<GameParameters::Player>::iterator i = list.begin(); i != list.end(); ++i)
    {
      if ((*i).id == id)
	{
	  list.erase (i);
	  return true;
	}
    }
  return false;
}

void GameServer::stand(void *conn, Player *player, Glib::ustring nickname)
{
  std::cout << "STAND: " << conn << " " << player << std::endl;
  if (!player || !conn)
    return;

  Participant *part = findParticipantByConn(conn);
  if (!part) 
    return;

  //remove player id from part.
  bool found = remove_from_player_list (part->players, player->getId());

  if (!found)
    //okay somebody's trying to boot another player.
    return;

  if (player && player->getType() == Player::NETWORKED)
    dynamic_cast<NetworkPlayer*>(player)->setConnected(false);
  notifyStand(player, nickname);
}

void GameServer::gotRemoteActions(void *conn, const Glib::ustring &payload)
{
  gotActions(payload);
  for (auto &i: participants)
    if (i->conn != conn)
      send(i->conn, MESSAGE_TYPE_SENDING_ACTIONS, payload);
}

void GameServer::gotRemoteHistory(void *conn, const Glib::ustring &payload)
{
  gotHistories(payload);
  for (auto &i: participants)
    if (i->conn != conn)
      send(i->conn, MESSAGE_TYPE_SENDING_HISTORY, payload);
}

void GameServer::sendMap(Participant *part)
{
  // first hack the players so the player type we serialize is right
  std::vector<Player*> players;
  for (auto &i: *Playerlist::getInstance())
    {
      bool connected = false;
      players.push_back(i);
      if (i->isComputer() == true)
	connected = true;
      NetworkPlayer *new_p = new NetworkPlayer(*i, true);
      new_p->setConnected(connected);
      Playerlist::getInstance()->swap(i, new_p);
    }

  // send the map, and save it to a file somewhere temporarily
  Glib::ustring tmpfile = File::get_tmp_file();
  File::erase(tmpfile);
  tmpfile += SAVE_EXT;

  d_game_scenario->saveGame(tmpfile);

  std::cerr << "sending map" << std::endl;
  network_server->sendFile(part->conn, MESSAGE_TYPE_SENDING_MAP, tmpfile);
  //file gets erased in NetworkConnection::sendFileMessage

  // unhack the players
  std::vector<Player*> deletables;
  for (auto i : players)
    {
      Player *p = Playerlist::getInstance()->getPlayer(i->getId());
      deletables.push_back(p);
      Playerlist::getInstance()->swap(p, i);
    }

  for (auto i : deletables)
    {
      NetworkPlayer *p = dynamic_cast<NetworkPlayer*>(i);
      delete p;
    }
}

void GameServer::sendActions(Participant *part)
{
  std::ostringstream os;
  XML_Helper helper(&os);

  helper.begin("1");
  helper.openTag("actions");

  for (auto &i: part->actions)
    (*i).save(&helper);

  helper.closeTag();

  send(part->conn, MESSAGE_TYPE_SENDING_ACTIONS, os.str());
}

void GameServer::sendHistories(Participant *part)
{
  std::ostringstream os;
  XML_Helper helper(&os);

  helper.begin("1");
  helper.openTag("histories");

  for (auto &i: part->histories)
    {
      std::cerr << String::ucompose("sending history %1 from person %2 %3 to person %4", History::historyTypeToString(i->getHistory()->getType()), d_nickname, Playerlist::getInstance()->getPlayer(i->getOwnerId())->getName(), part->nickname) << std::endl;
      (*i).save(&helper);
    }

  helper.closeTag();

  send(part->conn, MESSAGE_TYPE_SENDING_HISTORY, os.str());
}

bool GameServer::dumpActionsAndHistories(XML_Helper *helper, Player *player)
{
  Participant *part = NULL;
  for (auto &i: participants)
    {
      bool found = false;
      for (auto &it: i->players)
	{
	  if (it.id == player->getId())
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
    (*i).save(helper);
  for (auto &i: part->actions)
    (*i).save(helper);
  return true;
}

bool GameServer::dumpActionsAndHistories(XML_Helper *helper)
{
  Player *player = Playerlist::getActiveplayer();
  return dumpActionsAndHistories(helper, player);
}

void GameServer::sit_down (Player *player)
{
  if (!player)
    return;
  if (player->getType() == Player::NETWORKED)
    {
      //alright, we want to sit down as this player
      //convert the network player to a human player
      dynamic_cast<NetworkPlayer*>(player)->setConnected(true);
      RealPlayer *new_p = new RealPlayer (*player, true);
      Playerlist::getInstance()->swap(player, new_p);
      stopListeningForLocalEvents(player);
      listenForLocalEvents(new_p);
      delete player;
      player = new_p;
      add_to_player_list (players_seated_locally, new_p->getId(),
                          new_p->getName(), 
                          GameParameters::player_type_to_player_param(new_p->getType()));
      notifySit(new_p, d_nickname);
    }
  else if (player->getType() == Player::HUMAN)
    {
      //alright, we want to sit down as this player
      //wait, we're already human.
      // do nothing
    }
  else // an ai player
    {
      stopListeningForLocalEvents(player);
      listenForLocalEvents(player);
      add_to_player_list (players_seated_locally, player->getId(),
                          player->getName(),
                          GameParameters::player_type_to_player_param(player->getType()));
      notifySit(player, d_nickname);
    }
  notifyTypeChange(player, player->getType());
}

void GameServer::stand_up (Player *player)
{
  if (!player)
    return;
  if (player->getType() == Player::HUMAN)
    {
      //alright, we want to stand up as this player
      //convert the player from a human player back to a network player

      NetworkPlayer *new_p = new NetworkPlayer(*player, true);
      Playerlist::getInstance()->swap(player, new_p);
      stopListeningForLocalEvents(player);
      delete player;
      listenForLocalEvents(new_p);
      player = new_p;
      new_p->setConnected(false);
      notifyStand(new_p, d_nickname);
      remove_from_player_list (players_seated_locally, new_p->getId());
    }
  else if (player->getType() == Player::NETWORKED)
    {
      //this is the forcibly booted, made to stand-up case. .
      Participant *part = findParticipantByPlayerId(player->getId());
      stand(part->conn, player, part->nickname);
    }
  else // an ai player
    {
      stopListeningForLocalEvents(player);
      remove_from_player_list (players_seated_locally, player->getId());
      notifyStand(player, d_nickname);
    }
  notifyTypeChange(player, GameParameters::Player::NETWORKED);
}

void GameServer::type_change (Player *player, int type)
{
  if (!player)
    return;
  update_player_type (players_seated_locally, player->getId(), (guint32) type);
  notifyTypeChange(player, type);
  player_changes_type.emit(player, type);
}

void GameServer::chat (Glib::ustring message)
{
  notifyChat(d_nickname + ":" + message);
}

void GameServer::notifyChat(Glib::ustring message)
{
  gotChatMessage(d_nickname, message);
  for (auto &i: participants)
    send(i->conn, MESSAGE_TYPE_CHATTED, message);
}

void GameServer::sendSeat(void *conn, GameParameters::Player player, Glib::ustring nickname)
{
  Glib::ustring payload = String::ucompose ("%1 %2 %3 %4", player.id, 
                                            LOBBY_MESSAGE_TYPE_SIT, 1, 
                                            nickname);
  send(conn, MESSAGE_TYPE_LOBBY_ACTIVITY, payload);

  payload = String::ucompose ("%1 %2 %3 %4", player.id, 
                              LOBBY_MESSAGE_TYPE_CHANGE_TYPE, 1, player.type);
  send(conn, MESSAGE_TYPE_LOBBY_ACTIVITY, payload);

  payload = String::ucompose ("%1 %2 %3 %4", player.id, 
                              LOBBY_MESSAGE_TYPE_CHANGE_NAME, 1, 
                              player.name);
  send(conn, MESSAGE_TYPE_LOBBY_ACTIVITY, payload);
}

void GameServer::sendSeats(void *conn)
{
  Participant *part = findParticipantByConn(conn);
  if (!part)
    return;
  //send seatedness info for remote participants

  for (auto &i: participants)
    {
      if (i->conn == part->conn)
	continue;

      for (auto &j: i->players)
        sendSeat(conn, j, i->nickname);
    }
  //send out seatedness info for local server
  for (auto &j: players_seated_locally)
    sendSeat(conn, j, d_nickname);
}

void GameServer::sendChatRoster(void *conn)
{
  Participant *part = findParticipantByConn(conn);
  if (!part)
    return;
  for (auto &i: participants)
    {
      if (i->conn == part->conn)
	continue;
      send(part->conn, MESSAGE_TYPE_PARTICIPANT_CONNECTED, i->nickname);
    }
  send(part->conn, MESSAGE_TYPE_PARTICIPANT_CONNECTED, d_nickname);
}

void GameServer::sendOffPlayer(Player *p)
{
  std::stringstream player;
  player << p->getId();
  for (auto &i: participants)
    send(i->conn, MESSAGE_TYPE_OFF_PLAYER, player.str());

  remote_player_died.emit(p);
}

void GameServer::sendKillPlayer(Player *p)
{
  std::stringstream player;
  player << p->getId();
  for (auto &i: participants)
    send(i->conn, MESSAGE_TYPE_KILL_PLAYER, player.str());

  remote_player_died.emit(p);
}

void GameServer::sendTurnOrder()
{
  std::list<guint32> ids;
  std::stringstream players;
  for (auto &it: *Playerlist::getInstance())
    {
      players << it->getId() << " ";
      ids.push_back(it->getId());
    }
  for (auto &i: participants)
    send(i->conn, MESSAGE_TYPE_TURN_ORDER, players.str());
  playerlist_reorder_received.emit();
}

bool GameServer::gameHasBegun()
{
  return d_game_has_begun;
}

void GameServer::notifyClientsGameMayBeginNow()
{
  d_game_has_begun = true;
  syncLocalPlayers();
  //notify everyone that the game can finally start.
  for (auto &i: participants)
    send(i->conn, MESSAGE_TYPE_GAME_MAY_BEGIN, "");
}

void GameServer::syncLocalPlayers()
{
  std::list<guint32> ids;
  for (auto &j: players_seated_locally)
    {
      Player *p = Playerlist::getInstance()->getPlayer(j.id);
      if (j.type == GameParameters::Player::OFF)
        {
          stopListeningForLocalEvents(p);
          player_gets_turned_off.emit(p);
          sendOffPlayer(p);
          ids.push_back(p->getId());
          Playerlist::getInstance()->syncPlayer(j);
        }
      else
        {
          stopListeningForLocalEvents(p);
          Playerlist::getInstance()->syncPlayer(j);
          listenForLocalEvents(Playerlist::getInstance()->getPlayer(j.id));
        }
    }
  for (auto &i: ids)
    remove_from_player_list (players_seated_locally, i);
}

void GameServer::on_turn_aborted()
{
  d_stop = true;
  remove_all_participants();
}

void GameServer::send (void *conn, int type, Glib::ustring payload)
{
  if (d_save_messages.empty () == false)
    {
      std::ofstream logfile;
      logfile.open(d_save_messages, std::ios_base::app);
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
// End of file
