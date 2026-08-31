//  Copyright (C) 2020, 2021, 2026 Ben Asselstine
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
#ifndef KEEPER_EDITOR_DIALOG_H
#define KEEPER_EDITOR_DIALOG_H
#include "army-type-label.h"
#include "keeper-undo-actions.h"

class KeeperEditorDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "keeper.ui";
      }

    KeeperEditorDialog (BaseObjectType* o,
                        const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_clear_button = load <Gtk::Button> ("clear_button");
        m_keeper_button = load <Gtk::Button> ("keeper_button");
        m_random_button = load <Gtk::Button> ("random_button");
        m_name_entry = load <Gtk::Entry> ("name_entry");
      }

    ~KeeperEditorDialog ()
      {
        disconnect_signals ();
        delete m_keeper;
        delete m_umgr;
      }

    void setup (Keeper *keeper, Vector<int> pos, CreateScenarioRandomize *r)
      {
        if (keeper)
          m_keeper = new Keeper (*keeper);
        else
          m_keeper = new Keeper (NULL, pos);
        m_pos = pos;
        m_randomize = r;
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);
        set_response (m_clear_button, Gtk::ResponseType::REJECT);

        m_keeper_army_type_label =
          Gtk::make_managed<ArmyTypeLabel>
          (this, m_keeper_button, Shield::NEUTRAL,
           SelectArmyDialog::SELECT_RUIN_DEFENDER);

        setup_undo ();

        update ();
      }

    bool is_changed ()
      {
        return m_umgr->undo_empty () == false;
      }

    Keeper *get_keeper ()
      {
        return m_keeper;
      }
private:
    Gtk::Button *m_close_button;
    Gtk::Button *m_random_button;
    Gtk::Entry *m_name_entry;
    Gtk::Button *m_keeper_button;
    Gtk::Button *m_clear_button;
    ArmyTypeLabel *m_keeper_army_type_label;
    std::list<sigc::connection> m_connections;

    Keeper *m_keeper;
    Vector<int> m_pos;
    CreateScenarioRandomize *m_randomize;

    UndoMgr *m_umgr;

    void
    add_connection (sigc::connection c)
      {
        m_connections.push_back (c);
      }

    void connect_signals ()
      {
        add_connection
          (m_name_entry->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new KeeperUndoAction_Name (m_keeper, m_umgr,
                                                      m_name_entry));
              m_keeper->setName (m_name_entry->get_text ());
            }));

        add_connection
          (m_keeper_army_type_label->signal_army_selected ().connect
           ([this] (int id)
            {
              m_umgr->add (new KeeperUndoAction_Keeper (m_keeper));

              if (id >= 0)
                {
                  Player *p = Playerlist::getNeutral ();
                  auto army_proto =
                    Armysetlist::instance ()->getArmy (p->getArmyset (), id);
                  m_keeper->add (army_proto, m_pos);
                  m_keeper->rename ();
                }
              else
                {
                  m_keeper->clearStack ();
                  m_keeper->rename ();
                }
              update ();
            }));

        add_connection
          (m_random_button->signal_clicked ().connect
           ([this] ()
            {
              Keeper *k = m_randomize->getRandomRuinKeeper (m_pos);
              if (!k)
                return;
              m_umgr->add (new KeeperUndoAction_Randomize (m_keeper));

              if (m_keeper)
                delete m_keeper;
              m_keeper = k;

              m_keeper_army_type_label->set_army_type (k->getTypeId ());
              m_keeper->rename ();
              update ();
            }));
      }

    void update ()
      {
        disconnect_signals ();
        m_name_entry->set_text (m_keeper->getName ());
        m_keeper_army_type_label->set_army_type (m_keeper->getTypeId ());

        connect_signals ();
      }

    void disconnect_signals ()
      {
        for (auto c : m_connections)
          c.disconnect ();
        m_connections.clear ();
      }

    UndoAction *execute_action (UndoAction *action2)
      {
        KeeperUndoAction *action = dynamic_cast<KeeperUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case KeeperUndoAction::NAME:
              {
                auto a = dynamic_cast<KeeperUndoAction_Name*>(action);
                out =
                  new KeeperUndoAction_Name (m_keeper, m_umgr, m_name_entry);
                m_keeper->setName (a->get_keeper ()->getName ());
              }
            break;

          case KeeperUndoAction::RANDOMIZE:
              {
                auto a = dynamic_cast<KeeperUndoAction_Randomize*>(action);
                out = new KeeperUndoAction_Randomize (m_keeper);
                delete m_keeper;
                m_keeper = new Keeper (*a->get_keeper ());
              }
            break;

          case KeeperUndoAction::KEEPER:
              {
                auto a = dynamic_cast<KeeperUndoAction_Keeper*>(action);
                out = new KeeperUndoAction_Keeper (m_keeper);
                delete m_keeper;
                m_keeper = new Keeper (*a->get_keeper ());
              }
            break;
          }
        return out;
      }

    void setup_undo ()
      {
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &KeeperEditorDialog::execute_action));

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

        m_umgr->add_cursor (m_name_entry);
      }
};
#endif
