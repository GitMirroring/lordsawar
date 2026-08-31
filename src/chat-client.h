//  Copyright (C) 2008, 2014, 2026 Ben Asselstine
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
#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H

#include <config.h>

#include <list>
#include <glibmm.h>
#include <memory>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>

//! An object representing a person in the game lobby chat window.
class ChatClient: public sigc::trackable
{
public:
  ChatClient (Glib::ustring nick = "guest");

  ~ChatClient()
    {
    }

  void got_chat_message (const Glib::ustring profile_id,
                         const Glib::ustring &payload);

  void got_system_message (const Glib::ustring profile_id,
                           const Glib::ustring &payload);

  void set_nickname (Glib::ustring nick)
    {
      m_nickname = nick;
    }

  Glib::ustring get_nickname () const
    {
      return m_nickname;
    }

  sigc::signal<void(Glib::ustring, Glib::ustring)>
    signal_chat_message_received ()
    {
      return m_chat_message_received;
    }

  sigc::signal<void(Glib::ustring)> signal_system_message_received ()
    {
      return m_system_message_received;
    }
protected:

  Glib::ustring m_nickname;

  /* profile id, message */
  sigc::signal<void(Glib::ustring, Glib::ustring)> m_chat_message_received;

  /* message */
  sigc::signal<void(Glib::ustring)> m_system_message_received;
};

#endif
