//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014, 2020, 2021, 2026 Ben Asselstine
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
#include "lw-dialog-base.h"
#ifndef SIGNPOST_EDITOR_DIALOG_H
#define SIGNPOST_EDITOR_DIALOG_H
#include "item.h"
#include "signpost-undo-actions.h"
class SignpostEditorDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "signpost-editor.ui";
      }

    SignpostEditorDialog (BaseObjectType* o,
                          const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_randomize_button = load <Gtk::Button> ("randomize_button");
        m_sign_textview = load <Gtk::TextView> ("sign_textview");
      }

    ~SignpostEditorDialog ()
      {
        disconnect_signals ();
        delete m_signpost;
        delete m_umgr;
      }

    void setup (Vector<int> pos, CreateScenarioRandomize *randomizer)
      {
        auto sign = GameMap::getSignpost (pos);
        m_signpost = new Signpost (*sign, true);

        m_sign_textview->get_buffer ()->set_text (m_signpost->getName ());
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        m_randomize_button->signal_clicked ().connect
          ([this, randomizer] ()
           {
             Glib::ustring existing_name =
               m_sign_textview->get_buffer ()->get_text ();
             bool dynamic =
               ((Rnd::rand () % randomizer->getNumSignposts ()) == 0);
             SignpostUndoAction_Message *action =
               new SignpostUndoAction_Message (m_signpost->getName (), m_umgr,
                                               m_sign_textview);
             m_umgr->add (action);
             if (existing_name == DEFAULT_SIGNPOST)
               {
                 if (dynamic)
                   m_signpost->setName
                     (randomizer->getDynamicSignpost (m_signpost));
                 else
                   m_signpost->setName (randomizer->popRandomSignpost ());
               }
             else
               {
                 if (dynamic)
                   m_signpost->setName
                     (randomizer->getDynamicSignpost (m_signpost));
                 else
                   {
                     m_signpost->setName (randomizer->popRandomSignpost ());
                     randomizer->pushRandomSignpost (existing_name);
                   }
               }
             update ();
           });

        setup_undo ();
        update ();
      }

    Signpost *get_signpost () const
      {
        return m_signpost;
      }

    bool is_changed ()
      {
        return m_umgr->undo_empty () == false;
      }

private:
    Gtk::TextView *m_sign_textview;
    Gtk::Button *m_close_button;
    Gtk::Button *m_randomize_button;
    Signpost *m_signpost;
    UndoMgr *m_umgr;
    std::list<sigc::connection> m_connections;

    void add_connection (sigc::connection c)
      {
        m_connections.push_back (c);
      }

    void update ()
      {
        disconnect_signals ();

        m_sign_textview->get_buffer ()->set_text (m_signpost->getName ());

        connect_signals ();
      }

    void connect_signals ()
      {
        add_connection
          (m_sign_textview->get_buffer ()->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new SignpostUndoAction_Message (m_signpost->getName (),
                                                 m_umgr, m_sign_textview));
              m_signpost->setName (m_sign_textview->get_buffer ()->get_text ());
            }));
      }

    void disconnect_signals ()
      {
        for (auto c : m_connections)
          c.disconnect ();
        m_connections.clear ();
      }

    UndoAction *execute_action (UndoAction *action2)
      {
        auto action = dynamic_cast<SignpostUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case SignpostUndoAction::MESSAGE:
              {
                auto a = dynamic_cast<SignpostUndoAction_Message*>(action);
                out = new SignpostUndoAction_Message
                  (m_signpost->getName (), m_umgr, m_sign_textview);
                m_signpost->setName (a->get_message ());
              }
            break;
          }
        return out;
      }

    void setup_undo ()
      {
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &SignpostEditorDialog::execute_action));

        setup_undo_and_redo ();

        signal_undo ().connect
           ([this] ()
            {
              m_umgr->undo ();
              update ();
            });

        signal_redo ().connect
           ([this] ()
            {
              m_umgr->redo ();
              update ();
            });

        m_umgr->add_cursor (m_sign_textview);
      }
};
#endif
