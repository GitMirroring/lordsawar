//  Copyright (C) 2026 Ben Asselstine
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

#include <gtkmm.h>
#ifndef LOBBY_H
#define LOBBY_H
#include "shield.h"

#include <map>
#include <vector>
class ChatRecord
{
public:
    ChatRecord (Glib::ustring profile_id, Glib::ustring message)
      : m_timestamp (Glib::DateTime::create_now_local ()),
      m_profile_id (profile_id), m_message (message)
  {

  }

    ~ChatRecord ()
      {
      }

    Glib::DateTime get_timestamp () const
      {
        return m_timestamp;
      }

    Glib::ustring get_profile_id () const
      {
        return m_profile_id;
      }

    Glib::ustring get_message () const
      {
        return m_message;
      }
private:
    Glib::DateTime m_timestamp;
    Glib::ustring m_profile_id;
    Glib::ustring m_message;
};

class Lobby
{
public:
    static Lobby* instance ()
      {
        if (s_instance == 0)
          s_instance = new Lobby ();

        return s_instance;
      }

    static void deleteInstance ()
      {
        if (s_instance)
          delete s_instance;

        s_instance = 0;
      }

    void join (Glib::ustring profile_id, Glib::ustring name)
      {
        auto it = m_id_name.find (profile_id);
        if (it == m_id_name.end ())
          {
            m_id_name[profile_id] = name;
            m_chat_participants.push_back (profile_id);
          }
      }

    void depart (Glib::ustring profile_id)
      {
        auto it = m_id_name.find (profile_id);
        if (it != m_id_name.end ())
          m_id_name.erase (it);

        auto &v = m_chat_participants;
        v.erase (std::remove (v.begin (), v.end (), profile_id), v.end ());

        stand (profile_id);
      }

    void sit (Shield::Color shield, Glib::ustring profile_id)
      {
        auto it = m_shield_id.find (shield);
        if (it == m_shield_id.end ())
          m_shield_id[shield] = profile_id;
      }

    void change_name (Glib::ustring profile_id, Glib::ustring new_name)
      {
        for (auto &idn : m_id_name)
          {
            if (idn.first == profile_id)
              idn.second = new_name;
          }
      }

    void stand (Shield::Color shield)
      {
        m_shield_id.erase(shield);
      }

    void add_chat_message (Glib::ustring profile_id, Glib::ustring message)
      {
        if (user_is_muted (profile_id))
          return;
        m_chat_log.push_back (ChatRecord (profile_id, message));
      }

    void dump_chat ()
      {
        for (auto r : m_chat_log)
          {
            printf ("%s: %s\n", get_name (r.get_profile_id ()).c_str (),
                    r.get_message ().c_str ());
          }
      }

    std::vector<ChatRecord> get_chat_log ()
      {
        return m_chat_log;
      }

    void clear_chat_log ()
      {
        m_chat_log.clear ();
      }

    void set_profile (Glib::ustring id)
      {
        m_profile_id = id;
      }

    Glib::ustring get_profile_id (Shield::Color shield)
      {
        auto it = m_shield_id.find (shield);
        if (it != m_shield_id.end ())
          return m_shield_id[shield];
        return "";
      }

    Glib::ustring get_name (Glib::ustring profile_id)
      {
        auto it = m_id_name.find (profile_id);
        if (it == m_id_name.end ())
          return "";
        else
          return (*it).second;
      }

    bool user_in_lobby (Glib::ustring id)
      {
        return
          std::find
          (m_chat_participants.begin (), m_chat_participants.end (), id) !=
          m_chat_participants.end ();
      }

    bool user_is_muted (Glib::ustring id)
      {
        return
          std::find
          (m_muted_participants.begin (), m_muted_participants.end (), id) !=
          m_muted_participants.end ();
      }

    bool user_is_mod (Glib::ustring id)
      {
        return
          std::find
          (m_mods.begin (), m_mods.end (), id) != m_mods.end ();
      }

    guint32 count_mods ()
      {
        return m_mods.size ();
      }

    void add_mod (Glib::ustring profile_id)
      {
        if (user_is_mod (profile_id))
          return;
        m_mods.push_back (profile_id);
      }

    bool can_mute (Glib::ustring profile_id)
      {
        if (profile_id == m_profile_id)
          return false;
        if (user_is_mod (profile_id))
          return false;
        if (user_is_muted (profile_id))
          return false;
        return true;
      }

    void toggle_mute (Glib::ustring profile_id)
      {
        //we can't mute ourselves
        if (profile_id == m_profile_id)
          return;
        //we can't mute mods
        if (user_is_mod (profile_id))
          return;
        if (user_is_muted (profile_id))
          {
            auto &v = m_muted_participants;
            v.erase (std::remove (v.begin (), v.end (), profile_id), v.end ());
          }
        else
          m_muted_participants.push_back (profile_id);
      }

    std::vector<Glib::ustring> get_roster ()
      {
        std::vector<Glib::ustring> roster = m_chat_participants;
        /*
         // remove muted participants from roster, but then we can't unmute
        for (auto profile_id : m_muted_participants)
          {
            auto &v = roster;
            v.erase (std::remove (v.begin (), v.end (), profile_id), v.end ());
          }
          */
        std::sort (roster.begin (), roster.end (),
          [this] (const Glib::ustring& a, const Glib::ustring& b)
          {
            bool a_mod = user_is_mod (a);
            bool b_mod = user_is_mod (b);

            if (a_mod != b_mod)
              return a_mod > b_mod;

            return get_name (a).raw () < get_name (b).raw ();
          });
        return roster;
      }

protected:
    Lobby ()
      {
      }

    ~Lobby ()
      {
      }

private:
    std::vector<ChatRecord> m_chat_log;

    //shield::color to profile id mapping
    std::map<Shield::Color, Glib::ustring> m_shield_id;

    //profile id to person name mappping
    std::map<Glib::ustring, Glib::ustring> m_id_name;

    //who's in the chat roster
    std::vector<Glib::ustring> m_chat_participants;

    //mute list, we don't see their messages
    std::vector<Glib::ustring> m_muted_participants;

    std::vector<Glib::ustring> m_mods;

    //it's our own profile id
    Glib::ustring m_profile_id;

    inline static Lobby *s_instance = 0;

    void stand (Glib::ustring profile_id)
      {
        for (auto sn : m_shield_id)
          {
            if (sn.second == profile_id)
              {
                stand (sn.first);
                break;
              }
          }
      }
};
#endif
