//  Copyright (C) 2021 Ben Asselstine
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
#ifndef UNDO_MGR_H
#define UNDO_MGR_H

#include <gtkmm.h>
#include "undo-action.h"

#include <sigc++/signal.h>
#include <sigc++/trackable.h>

class UndoCursor
{
public:
    UndoCursor (int pos, Gtk::Entry *e)
      :m_pos (pos), m_object (e)
      {
      }

    UndoCursor (int pos, Gtk::TextView *t)
      :m_pos (pos), m_object (t)
      {
      }

    ~UndoCursor ()
      {
      }

    int get_pos () const
      {
        return m_pos;
      }

    Gtk::Object *get_object () const
      {
        return m_object;
      }
private:
    int m_pos;
    Gtk::Object *m_object;
};

/**
 * The UndoMgr is handles the lists of undo and redo actions.
 * New'd undo actions are given to the manager, and then it later deletes them.
 * Callers connect to the execute signal for callback.
 */
class UndoMgr: public sigc::trackable
{
public:

    inline static const int LIMIT = 100;
    inline static const double DELAY = 0.6;

    //! Default constructor.
    UndoMgr (double delay, guint32 limit)
      : m_delay (delay), m_limit (limit)
      {
      }

    //! Destructor.
    ~UndoMgr ()
      {
        clear ();
      }

    void undo ()
      {
        std::list<UndoAction*> redo_group;
        std::list<UndoAction*> group = pop_group (&m_undos);
        for (auto action : group)
          {
            if (dynamic_cast<UndoCursor*>(action))
              {
                UndoCursor *c = dynamic_cast<UndoCursor*>(action);
                m_unwound_pos[c->get_object ()] = c->get_pos ();
              }
            UndoAction *redo = m_signal_execute.emit (action);
            redo->set_time (action->get_time ());
            redo_group.push_back (redo);
          }

        if (count_undo_blocks (m_redos, true) > m_limit)
          pop_last_group (&m_redos, true);
        for (auto action: redo_group)
          m_redos.push_front (action);

        for (auto action : group)
          delete action;
      }

    void redo ()
      {
        std::list<UndoAction*> undo_group;
        std::list<UndoAction*> group = pop_group (&m_redos, true);
        for (auto action : group)
          {
            if (dynamic_cast<UndoCursor*>(action))
              {
                UndoCursor *c = dynamic_cast<UndoCursor*>(action);
                m_unwound_pos[c->get_object ()] = c->get_pos ();
              }
            UndoAction *undo = m_signal_execute.emit (action);
            undo->set_time (action->get_time ());
            undo_group.push_back (undo);
          }

        if (count_undo_blocks (m_undos) > m_limit)
          pop_last_group (&m_undos);
        for (auto action: undo_group)
          m_undos.push_front (action);

        for (auto action : group)
          delete action;
      }

    void add (UndoAction *action)
      {
        m_undos.push_front (action);
        if (count_undo_blocks (m_undos) > m_limit)
          pop_last_group (&m_undos);
        m_signal_added_undo.emit ();
      }

    bool undo_empty ()
      {
        return m_undos.empty ();
      }

    bool redo_empty ()
      {
        return m_redos.empty ();
      }

    void set_delay (double d)
      {
        m_delay = d;
      }

    void set_limit (guint32 l)
      {
        m_limit = l;
      }

    Glib::ustring get_undo_name () const
      {
        return UndoMgr::get_action_name (m_undos);
      }

    std::list<Glib::ustring> get_undo_names () const
      {
        return UndoMgr::get_action_names (m_undos);
      }

    Glib::ustring get_redo_name () const
      {
        return UndoMgr::get_action_name (m_redos);
      }

    void update_actions (Glib::RefPtr<Gio::SimpleAction> undo,
                         Glib::RefPtr<Gio::SimpleAction> redo)
      {
        redo->set_enabled (redo_empty () == false);
        undo->set_enabled (undo_empty () == false);
      }

    void clear ()
      {
        for (auto a : m_redos)
          delete a;
        m_redos.clear ();
        for (auto a : m_undos)
          delete a;
        m_undos.clear ();
      }

    sigc::signal<UndoAction*(UndoAction*)> signal_execute ()
      {
        return m_signal_execute;
      }

    sigc::signal<void ()> signal_added_undo ()
      {
        return m_signal_added_undo;
      }

    void add_cursor (Gtk::Entry *entry)
      {
        m_entries[entry] = std::pair<int, int>(-1, -1);
      }

    void add_cursor (Gtk::TextView *textview)
      {
        m_textviews[textview] = std::pair<int, int>(-1, -1);
      }

    void connect_signals ()
      {
        for (auto l : m_entries)
          connect (l.first);
        for (auto l : m_textviews)
          connect (l.first);
      }

    void disconnect_signals ()
      {
        for (auto c : m_connections)
          c.disconnect ();
        m_connections.clear ();
      }

    int get_pos (Gtk::Entry *e)
      {
        auto it = m_entries.find (e);
        if (it == m_entries.end ())
          return 0;
        std::pair<int,int> p = (*it).second;
        int pos = 0;
        if (p.second != -1)
          pos = p.second;
        else if (p.first != -1)
          pos = p.first;
        return pos;
      }

    void set_pos (Gtk::Entry *e)
      {
        auto it = m_unwound_pos.find (e);
        if (it == m_unwound_pos.end ())
          return;
        e->set_position (m_unwound_pos[e]);
        update_entry (e);
        update_entry (e);
      }

    int get_pos (Gtk::TextView *t)
      {
        auto it = m_textviews.find (t);
        if (it == m_textviews.end ())
          return 0;
        std::pair<int,int> p = (*it).second;
        int pos = 0;
        if (p.second != -1)
          pos = p.second;
        else if (p.first != -1)
          pos = p.first;
        return pos;
      }

    void set_cursors ()
      {
        for (auto l : m_unwound_pos)
          {
            if (dynamic_cast<Gtk::Entry*>(l.first))
              set_pos (dynamic_cast<Gtk::Entry*>(l.first));
            else if (dynamic_cast<Gtk::TextView*>(l.first))
              set_pos (dynamic_cast<Gtk::TextView*>(l.first));
          }
      }

    void set_pos (Gtk::TextView *t)
      {
        auto it = m_unwound_pos.find (t);
        if (it == m_unwound_pos.end ())
          return;
        int p = m_unwound_pos[t];
        t->get_buffer ()->place_cursor
          (t->get_buffer ()->get_iter_at_offset (p));
        update_textview (t);
        update_textview (t);
      }

    static Glib::ustring get_action_name (std::list<UndoAction*> list)
      {
        for (auto l : list)
          {
            if (l->get_action_name () == "")
              continue;
            return l->get_action_name ();
          }
        return "";
      }

    static std::list<Glib::ustring> get_action_names (std::list<UndoAction*> list)
      {
        std::list<Glib::ustring> names;
        for (auto l : list)
          names.push_back (l->get_action_name ());
        return names;
      }

private:
    double m_delay;
    guint32 m_limit;
    std::list<UndoAction*> m_undos;
    std::list<UndoAction*> m_redos;
    sigc::signal<UndoAction*(UndoAction*)> m_signal_execute;
    sigc::signal<void ()> m_signal_added_undo;

    std::list<UndoAction*> pop_last_group
      (std::list<UndoAction*> *list, bool bottom = false)
        {
          std::list<UndoAction*> group;
          std::vector<UndoAction*> blanks;
          UndoAction *prev = NULL;
          for (auto it = list->rbegin (); it != list->rend (); it++)
            {
              UndoAction *l = *it;
              l->set_time (list->back ()->get_time ());
              if (is_grouped (prev, l, bottom))
                {
                  group.push_back (l);
                  if (l->get_action_name () == "")
                    blanks.push_back (l);
                }
              else
                break;
              prev = l;
            }
          for (auto g : group)
            g->set_time (group.front ()->get_time ());

          for (guint32 i = 0; i < group.size (); i++)
            list->pop_back ();

          if (blanks.size () > 1)
            {
              int i = 0;
              for (auto b : blanks)
                {
                  if (i > 0)
                    {
                      group.remove (b);
                      delete b;
                    }
                  i++;
                }
            }
          return group;
        }

    std::list<UndoAction*> pop_group
      (std::list<UndoAction*> *list, bool bottom = false)
        {
          std::list<UndoAction*> group;
          UndoAction *prev = NULL;
          std::list<UndoAction*> blanks;
          for (auto l : *list)
            {
              if (is_grouped (prev, l, bottom))
                {
                  group.push_back (l);
                  if (l->get_action_name () == "")
                    blanks.push_back (l);
                }
              else
                break;
              prev = l;
            }
          for (auto g : group)
            g->set_time (group.front ()->get_time ());
          for (guint32 i = 0; i < group.size (); i++)
            list->pop_front ();
          if (blanks.size () > 1)
            {
              int i = 0;
              for (auto b : blanks)
                {
                  if (i > 0)
                    {
                      group.remove (b);
                      delete b;
                    }
                  i++;
                }
            }
          return group;
        }

    bool is_grouped (UndoAction *l, UndoAction *r, bool bottom = false)
      {
        if (!l)
          return true;
        if (l->get_aggregate () != r->get_aggregate ())
          return false;

        if (l == NULL && r->get_aggregate () == UndoAction::AGGREGATE_NONE)
          return true;
        if (l->get_aggregate () == UndoAction::AGGREGATE_NONE && r == NULL)
          return true;
        if (l->get_aggregate () == UndoAction::AGGREGATE_NONE &&
            r->get_aggregate () == UndoAction::AGGREGATE_NONE)
          return false;

        Glib::ustring leftaction = l->get_action_name ();
        Glib::ustring rightaction = r->get_action_name ();

        if (l->get_aggregate () == UndoAction::AGGREGATE_BLANK)
          {
            if (bottom)
              {
                if (rightaction == "")
                  rightaction = leftaction;
              }
            else
              {
                if (leftaction == "")
                  leftaction = rightaction;
              }
          }

        if (leftaction != rightaction)
          return false;

        if (l->get_aggregate () == UndoAction::AGGREGATE_DELAY)
          {
            double diff = r->get_time ().to_unix () - l->get_time ().to_unix ();
            if (diff > 0 && diff > m_delay)
              return false;
            if (diff < 0 && diff < -m_delay)
              return false;
          }
        return true;
      }

    guint32 count_undo_blocks
      (std::list<UndoAction*> list, bool bottom = false)
        {
          guint32 count = 0;
          UndoAction *prev = NULL;
          for (auto u : list)
            {
              if (u->get_aggregate () == UndoAction::AGGREGATE_BLANK ||
                  u->get_aggregate () == UndoAction::AGGREGATE_DELAY)
                {
                  if (is_grouped (prev, u, bottom) == false)
                    count++;
                }
              else
                count++;
              prev = u;
            }
          return count;
        }

    void connect (Gtk::Entry *entry)
      {
        m_connections.push_back
          (entry->property_cursor_position ().signal_changed ().connect
           (sigc::bind (sigc::mem_fun (*this, &UndoMgr::update_entry),
                        entry)));
      }

    void update_entry (Gtk::Entry *entry)
      {
        auto it = m_entries.find (entry);
        if (it == m_entries.end ())
          return;
        std::pair<int,int> p = (*it).second;

        p.second = p.first;
        p.first = entry->get_position ();
        m_entries[entry] = p;
      }

    void connect (Gtk::TextView *textview)
      {
        m_connections.push_back
          (textview->get_buffer ()->property_cursor_position ().
           signal_changed (). connect
           (sigc::bind (sigc::mem_fun (*this, &UndoMgr::update_textview),
                        textview)));
      }

    void update_textview (Gtk::TextView *textview)
      {
        auto it = m_textviews.find (textview);
        if (it == m_textviews.end ())
          return;
        std::pair<int,int> p = (*it).second;

        p.second = p.first;
        p.first = 
          textview->get_buffer ()->property_cursor_position ().get_value ();
        m_textviews[textview] = p;
      }


    // the two previous positions, where second is farther back in time
    // than the first
    std::map<Gtk::Entry *, std::pair<int,int> > m_entries;
    std::map<Gtk::TextView *, std::pair<int,int> > m_textviews;
    std::map<Gtk::Object *, int> m_unwound_pos;
    std::list<sigc::connection> m_connections;
};

#endif
