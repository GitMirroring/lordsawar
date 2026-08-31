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
#ifndef PLANTED_STANDARD_EDITOR_DIALOG_H
#define PLANTED_STANDARD_EDITOR_DIALOG_H
#include "item.h"
#include "planted-standard-undo-actions.h"
class PlantedStandardEditorDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "planted-standard-editor.ui";
      }

    PlantedStandardEditorDialog (BaseObjectType* o,
                                 const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_new_planted_standard = false;
        m_close_button = load <Gtk::Button> ("close_button");
        m_owner_combobox = load <Gtk::Box> ("owner_combobox");
        m_original_owner_combobox = load <Gtk::Box> ("original_owner_combobox");
        m_name_entry = load <Gtk::Entry> ("name_entry");
        m_bonus_label = load <Gtk::Label> ("bonus_label");
      }

    ~PlantedStandardEditorDialog ()
      {
        disconnect_signals ();
        delete m_item;
        delete m_umgr;
      }

    void setup (Vector<int> pos)
      {
        auto backpack = GameMap::getBackpack (pos);
        auto flag = backpack->getFirstPlantedItem ();
        if (!flag)
          {
            auto p = Playerlist::getActiveplayer ();
            Glib::ustring name =
              String::ucompose (_("%1 Standard"),  p->getName ());
            m_item = new Item (name, true, p);
            m_item->addBonus (Item::ADD1STACK);
            m_item->setPlanted (true);
            m_item->setPlantableOwnerId (p->getId ());
            m_item->setPlantableOriginalOwnerId (p->getId ());
            m_new_planted_standard = true;
          }
        else
          m_item = new Item (*flag, true);
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);
        fill_owner_combobox ();
        fill_original_owner_combobox ();
        setup_undo ();
        update ();
      }

    Item *get_item () const
      {
        return m_item;
      }

    bool is_changed ()
      {
        return m_new_planted_standard || m_umgr->undo_empty () == false;
      }

private:
    Gtk::Box *m_owner_combobox;
    LwCombo *m_owner_combo;
    Gtk::Box *m_original_owner_combobox;
    LwCombo *m_original_owner_combo;
    Gtk::Entry *m_name_entry;
    Gtk::Label *m_bonus_label;
    Gtk::Button *m_close_button;
    Item *m_item;
    UndoMgr *m_umgr;
    std::list<sigc::connection> m_connections;
    bool m_new_planted_standard;

    void add_connection (sigc::connection c)
      {
        m_connections.push_back (c);
      }

    void update ()
      {
        disconnect_signals ();

        m_name_entry->set_text (m_item->getName ());

        int i = player_to_row_number (m_item->getPlantableOwner ());
        if (i >= 0)
          m_owner_combo->set_active (i);

        i = player_to_row_number (m_item->getPlantableOriginalOwner ());
        if (i >= 0)
          m_original_owner_combo->set_active (i);

        m_bonus_label->set_text (m_item->getBonusDescription ());

        connect_signals ();
      }

    void connect_signals ()
      {
        add_connection
          (m_owner_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new PlantedStandardUndoAction_OrigOwner
                 (m_item->getPlantableOriginalOwner ()->getId ()));
              int row = m_owner_combo->get_active_row_number ();
              auto it = Playerlist::instance ()->begin ();
              std::advance (it, row);
              m_item->setPlantableOwnerId ((*it)->getId ());
            }));

        add_connection
          (m_original_owner_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new PlantedStandardUndoAction_OrigOwner
                 (m_item->getPlantableOwner ()->getId ()));
              int row = m_original_owner_combo->get_active_row_number ();
              auto it = Playerlist::instance ()->begin ();
              std::advance (it, row);
              m_item->setPlantableOriginalOwnerId ((*it)->getId ());
            }));

        add_connection
          (m_name_entry->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new PlantedStandardUndoAction_Name (m_item->getName (),
                                                     m_umgr, m_name_entry));
              m_item->setName (m_name_entry->get_text ());
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
        auto action = dynamic_cast<PlantedStandardUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case PlantedStandardUndoAction::OWNER:
              {
                auto a = dynamic_cast<PlantedStandardUndoAction_Owner*>(action);
                out = new PlantedStandardUndoAction_Owner
                  (m_item->getPlantableOwner ()->getId ());

                m_item->setPlantableOwnerId (a->get_owner_id ());
              }
            break;

          case PlantedStandardUndoAction::ORIG_OWNER:
              {
                auto a =
                  dynamic_cast<PlantedStandardUndoAction_OrigOwner*>(action);
                out = new PlantedStandardUndoAction_OrigOwner
                  (m_item->getPlantableOriginalOwner ()->getId ());

                m_item->setPlantableOriginalOwnerId (a->get_owner_id ());
              }
            break;

          case PlantedStandardUndoAction::NAME:
              {
                auto *a = dynamic_cast<PlantedStandardUndoAction_Name*>(action);
                out = new PlantedStandardUndoAction_Name (m_item->getName (),
                                                          m_umgr, m_name_entry);

                m_item->setName (a->get_name ());
              }
            break;
          }
        return out;
      }

    void setup_undo ()
      {
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &PlantedStandardEditorDialog::execute_action));

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

    void fill_owner_combobox ()
      {
        m_owner_combo = Gtk::make_managed<LwCombo> ();
        int i = 0;
        int found = -1;
        for (auto p : *Playerlist::instance ())
          {
            m_owner_combo->append (p->getName ());
            if (m_item->getPlantableOwner ()->getId () == p->getId ())
              found = i;
            i++;
          }
        if (found >= 0)
          m_owner_combo->set_active (found);
        m_owner_combobox->append (*m_owner_combo);
      }

    void fill_original_owner_combobox ()
      {
        m_original_owner_combo = Gtk::make_managed<LwCombo> ();
        int i = 0;
        int found = -1;
        for (auto p : *Playerlist::instance ())
          {
            m_original_owner_combo->append (p->getName ());
            if (m_item->getPlantableOriginalOwner ()->getId () == p->getId ())
              found = i;
            i++;
          }
        if (found >= 0)
          m_original_owner_combo->set_active (found);
        m_original_owner_combobox->append (*m_original_owner_combo);
      }

    int player_to_row_number (Player *match)
      {
        int i = 0;
        for (auto p : *Playerlist::instance ())
          {
            if (p->getId () == match->getId ())
              return i;
            i++;
          }

        return -1;
      }
};
#endif
