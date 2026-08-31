//  Copyright (C) 2008 Ole Laursen
//  Copyright (C) 2008, 2011, 2014, 2021, 2026 Ben Asselstine
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
#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include <config.h>

#include <memory>
#include <list>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>

#include "game-station.h"

class NetworkServer;
class Participant;
class Action;
class History;
class Player;
class XML_Helper;
class GameScenario;
class GameParameters;

//! A networked game server.  Talks to GameClient objects.
class GameServer: public GameStation
{
public:

  /* a client needs to be joined for 5 minutes before a seat is reserved for
   * them should they disconnect. */
  static const int m_min_join_time_to_reserve_seat = 5 * 60;
  /* a grace period of 1 hour is given for the disconnected client to return
   * to their seat. */
  static const int m_time_to_hold_reserved_seat = 60 * 60;
  static const int m_total_secs_to_countdown = 2;

  //! Returns the singleton instance.  Creates a new one if neccessary.
  static GameServer * instance();

  //! Deletes the singleton instance.
  static void deleteInstance();

  bool isRunning();
  void start(GameScenario *game_scenario, int port, Glib::ustring profile_id, Glib::ustring nick);

  void sit_down (Shield::Color shield, Glib::ustring profile_id);
  void stand_up (Shield::Color shield, Glib::ustring profile_id);
  void type_change (Player *player, int type);
  void chat(Glib::ustring message);
  void sendTurnOrder();
  void sendKillPlayer(Player *player);
  void sendOffPlayer(Player *player);
  void sendGameBegin ();
  void sendGameCanBegin ();
  void notifyRoundOver();
  sigc::signal<void()> signal_remote_participant_connected ()
    {
      return m_remote_participant_connected;
    }
  sigc::signal<void()> signal_remote_participant_disconnected ()
    {
      return m_remote_participant_disconnected;
    }
  sigc::signal<Player*()> signal_get_next_player ()
    {
      return m_get_next_player;
    }
  sigc::signal<void(int)> signal_port_in_use ()
    {
      return m_port_in_use;
    }

  void setGameScenario(GameScenario *scenario) {d_game_scenario = scenario;};

  bool sendRoundStart();
  bool sendNextPlayer();
  bool gameHasBegun();

  void on_player_finished_turn(Player *player);
  void on_turn_aborted();
  bool check_end_of_round();
  void saveMessages (Glib::ustring f);

  void add_mod (Glib::ustring id);
  void kick (Glib::ustring id);
protected:
  GameServer();
  ~GameServer();

protected:
  sigc::signal<void()> m_remote_participant_connected;
  sigc::signal<void()> m_remote_participant_disconnected;
  sigc::signal<Player*()> m_get_next_player;
  sigc::signal<void(int)> m_port_in_use;
private:
  GameScenario *d_game_scenario;
  bool d_game_has_begun;
  guint32 m_countdown_secs;
  void onActionDone(Action *action, guint32 id);
  void onHistoryDone(History *history, guint32 id);

  void send (void *conn, int type, Glib::ustring payload);
  void join(void *conn, Glib::ustring payload);
  void notifyJoin (Glib::ustring profile_id, Glib::ustring nickname);
  void depart(void *conn);
  void notifyDepart (void *conn, Glib::ustring profile_id, Glib::ustring nickname);
  void sit(void *conn, Player *player, Glib::ustring nickname);
  void notifySit(Player *player, Glib::ustring profile_id, Glib::ustring nickname);
  void stand(void *conn, Player *player, Glib::ustring nickname);
  void notifyStand(Player *player, Glib::ustring profile_id, Glib::ustring nickname);
  void change_type(void *conn, Player *player, int type);
  void notifyTypeChange(Player *player, Glib::ustring profile_id, int type);
  void gotRemoteActions(void *conn, const Glib::ustring &payload);
  void gotRemoteHistory(void *conn, const Glib::ustring &payload);
  void notifyChat(Glib::ustring message);

  void sendMap(Participant *part);
  void sendSeats(void *conn);
  void sendSeat(void *conn, GameParameters::Player player,
                Glib::ustring profile_id);
  void sendChatRoster(void *conn);

  void sendActions(Participant *part);
  void sendHistories(Participant *part);

  std::unique_ptr<NetworkServer> network_server;

  std::list<Participant *> participants;
  std::list<Participant *> disconnected_participants;
  std::list<GameParameters::Player> players_seated_locally;
  std::map<guint32, bool> id_end_turn; //whether local players ended their turn

  Participant * play_by_mail_participant;

  Participant *findParticipantByConn(void *conn);
  Participant *findParticipantByNick(Glib::ustring nickname);
  Participant *findParticipantByPlayerId(guint32 id);

  bool onGotMessage(void *conn, int type, Glib::ustring message);
  void onConnectionLost(void *conn);
  void onConnectionMade(void *conn);
  bool dumpActionsAndHistories(XML_Helper *helper);
  bool dumpActionsAndHistories(XML_Helper *helper, Player *player);

  void gotChat(void *conn, Glib::ustring message);

  bool player_already_sitting(Player *p);

  bool add_to_player_list(std::list<GameParameters::Player> &list, guint32 id,
                          Glib::ustring name, guint32 type);
  bool remove_from_player_list(std::list<GameParameters::Player> &list, guint32 id);
  bool update_player_type (std::list<GameParameters::Player> &list, guint32 id, guint32 type);
  bool update_player_name (std::list<GameParameters::Player> &list, guint32 id, Glib::ustring name);

  void syncLocalPlayers();

  Glib::ustring make_nickname_unique(Glib::ustring nickname);

  void onLocalNonNetworkedActionDone(NetworkAction *action);
  void onLocalNonNetworkedHistoryDone(NetworkHistory *history);
  void onLocalNetworkedHistoryDone(NetworkHistory *history);

  bool nextTurn();

  Glib::ustring getPeerHostName (void *conn);
  void remove_all_participants();

  void remove_disconnected_participant (Glib::ustring profile_id);
  void reseat_disconnected_client (Glib::ustring profile_id);
  Participant * find_disconnected_profile (Glib::ustring profile_id);
  bool seat_is_reserved (Shield::Color shield);
  void send_reserved_seat_message (void *conn, Player *player);
  int get_reservation_duration (Player *player);
  bool all_are_seated ();
  void record_client_ready (void *conn);
  bool all_ready ();
  bool player_is_ready (Player *p);
  guint32 get_num_waiting ();
  void sendWaitingForReady ();
  void sendCountdown ();

  bool d_stop;
  Glib::ustring d_save_messages;
  //! A static pointer for the singleton instance.
  static GameServer * s_instance;
};

#endif
