//  Copyright (C) 2008 Ole Laursen
//  Copyright (C) 2011, 2014, 2015, 2017, 2026 Ben Asselstine
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
#ifndef NETWORK_CONNECTION_H
#define NETWORK_CONNECTION_H

#include <config.h>

#include <queue>
#include <sigc++/signal.h>
#include <glibmm.h>
#include <giomm.h>
#include <gtkmm.h>
#include "network-common.h"
#include <mutex>
#include <condition_variable>

//! A simple network connection for sending messages, encapsulates the protocol
class NetworkConnection
{
public:

  NetworkConnection(const Glib::RefPtr<Gio::SocketConnection> &conn);
  NetworkConnection();
  ~NetworkConnection();

  void connectToHost(Glib::ustring host, int port);

  sigc::signal<void()> connected;
  sigc::signal<void()> connection_lost;
  sigc::signal<void()> connection_failed;
  sigc::signal<void()> connection_received_data;
  sigc::signal<bool(int, Glib::ustring)> got_message;
  sigc::signal<void()> queue_flushed;
  sigc::signal<void()> torn_down;
  sigc::signal<void(int, int)> payload_progress;

  void send(int type, const Glib::ustring &payload);
  void sendFile(int type, const Glib::ustring &filename);
  Glib::ustring get_peer_hostname();

  void tear_down_connection(bool lockit = false);
  void disconnect();
  Glib::ustring getHost() const {return d_host;};
  guint32 getPort() const {return d_port;};

  void send_queued_messages();

  static Glib::ustring lobbyActionTypeToString (int type)
    {
      enum LobbyActionType t = (enum LobbyActionType) type;
      switch  (t)
        {
        case LOBBY_MESSAGE_TYPE_SIT:
          return "Sit";

        case LOBBY_MESSAGE_TYPE_CHANGE_NAME:
          return "ChangeName";

        case LOBBY_MESSAGE_TYPE_STAND:
          return "Stand";

        case LOBBY_MESSAGE_TYPE_CHANGE_TYPE:
          return "ChangeType";

        default:
          return "Unknown";
        }
      return "Unknown";
    }

  static Glib::ustring typeToString (int type)
    {
      enum MessageType t = (enum MessageType) type;
      switch (t)
        {
        case MESSAGE_TYPE_PING:
          return "Ping";

        case MESSAGE_TYPE_PONG:
          return "Pong";

        case MESSAGE_TYPE_SENDING_MAP:
          return "SendingMap";

        case MESSAGE_TYPE_SENDING_ACTIONS:
          return "SendingActions";

        case MESSAGE_TYPE_SENDING_HISTORY:
          return "SendingHistory";

        case MESSAGE_TYPE_PARTICIPANT_CONNECT:
          return "ParticipantConnect";

        case MESSAGE_TYPE_PARTICIPANT_DISCONNECTED:
          return "ParticipantDisconnected";

        case MESSAGE_TYPE_PARTICIPANT_CONNECTED:
          return "ParticipantConnected";

        case MESSAGE_TYPE_PARTICIPANT_DISCONNECT:
          return "ParticipantDisconnect";

        case MESSAGE_TYPE_SERVER_DISCONNECT:
          return "ServerDisconnect";

        case MESSAGE_TYPE_CHAT:
          return "Chat";

        case MESSAGE_TYPE_CHATTED:
          return "Chatted";

        case MESSAGE_TYPE_REQUEST_SEAT_MANIFEST:
          return "RequestSeatManifest";

        case MESSAGE_TYPE_TURN_ORDER:
          return "TurnOrder";

        case MESSAGE_TYPE_KILL_PLAYER:
          return "KillPlayer";

        case MESSAGE_TYPE_ROUND_OVER:
          return "RoundOver";

        case MESSAGE_TYPE_ROUND_START:
          return "RoundStart";

        case MESSAGE_TYPE_LOBBY_ACTIVITY:
          return "LobbyActivity";

        case MESSAGE_TYPE_CHANGE_NICKNAME:
          return "ChangeNickname";

        case MESSAGE_TYPE_GAME_BEGIN:
          return "GameBegin";

        case MESSAGE_TYPE_OFF_PLAYER:
          return "OffPlayer";

        case MESSAGE_TYPE_NEXT_PLAYER:
          return "NextPlayer";

        case MESSAGE_TYPE_SAME_PROFILE_ID:
          return "SameProfileId";

        case MESSAGE_TYPE_MOD_ID:
          return "ModId";

        case MESSAGE_TYPE_SYSMSG:
          return "SysMsg";

        case MESSAGE_TYPE_KICK:
          return "Kick";

        case MESSAGE_TYPE_READY:
          return "Ready";

        case MESSAGE_TYPE_GAME_CAN_BEGIN:
          return "GameCanBegin";

        case MESSAGE_TYPE_WAITING_FOR_READY:
          return "WaitingForReady";

        case MESSAGE_TYPE_COUNTDOWN:
          return "Countdown";

        default:
          return "Unknown";
        }

      return "Unknown";
    }

private:
  Glib::RefPtr<Gio::SocketClient> client; //this is client-side connections.
  Glib::RefPtr<Gio::SocketConnection> conn;
  Glib::RefPtr<Gio::DataInputStream> in;
  Glib::RefPtr<Gio::DataOutputStream> out;
  Glib::RefPtr<Gio::SocketSource> source;
  sigc::connection d_connect_timer;
  sigc::connection d_in_cb;
  char *payload;
  int payload_left;
  int payload_size;
  char header[MESSAGE_SIZE_BYTES];
  int header_left;
  int header_size;
  Glib::ustring d_host;
  guint32 d_port;

  std::mutex mutex;
  std::condition_variable cond_push;
  std::condition_variable cond_pop;

  bool d_stop;
  bool d_bail;
  Glib::RefPtr<Gio::Cancellable> d_cancellable;

  struct Message
    {
      int type;
      Glib::ustring payload;
    };
  std::queue<struct Message> messages;

  void setup_connection();
  void on_connect_connected(Glib::RefPtr<Gio::AsyncResult> &result);
  gssize on_header_received(gssize len);
  gssize on_payload_received(gssize len);
  bool on_got_input(Glib::IOCondition cond);
  bool on_connect_timeout();

  void queue_message(int type, const Glib::ustring &payload);
  bool sendMessage(int type, const Glib::ustring &payload);
  void sendFileMessage(int type, Glib::ustring filename);

};

#endif
