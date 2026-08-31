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
#ifndef ITEM_EDITOR_DIALOG_H
#define ITEM_EDITOR_DIALOG_H
#include "item.h"
#include "item-editor-undo-actions.h"
#include "item-bonus-editor-dialog.h"
class ItemEditorDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "item-editor.ui";
      }

    ItemEditorDialog (BaseObjectType* o,
                      const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_name_entry = load <Gtk::Entry> ("name_entry");
        m_uses_spinbutton = load <Gtk::SpinButton> ("uses_spinbutton");
        m_bonus_label = load <Gtk::Label> ("bonus_label");
        m_close_button = load <Gtk::Button> ("close_button");
        m_edit_button = load <Gtk::Button> ("edit_button");
      }

    ~ItemEditorDialog ()
      {
        disconnect_signals ();
        delete m_umgr;
      }

    void setup (Item *item, Shield::Color shield)
      {
        m_item = item;
        m_shield = shield;
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);
        setup_undo ();
        update ();
      }

    bool is_changed ()
      {
        return m_umgr->undo_empty () == false;
      }
private:
    Gtk::Entry *m_name_entry;
    Gtk::SpinButton *m_uses_spinbutton;
    Gtk::Label *m_bonus_label;
    Gtk::Button *m_edit_button;
    Gtk::Button *m_close_button;

    Item *m_item;
    Shield::Color m_shield;

    UndoMgr *m_umgr;
    std::list<sigc::connection> m_connections;

    void add_connection (sigc::connection c)
      {
        m_connections.push_back (c);
      }

    void update ()
      {
        disconnect_signals ();
        Glib::ustring bonus = m_item->getBonusDescription ();
        if (bonus == "")
          bonus = _("No Bonus");
        m_bonus_label->set_text (bonus);
        m_uses_spinbutton->set_value (m_item->getNumberOfUsesLeft ());
        m_name_entry->set_text (m_item->getName ());
        connect_signals ();
      }

    void connect_signals ()
      {
        add_connection
          (m_name_entry->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemEditorUndoAction_Name (m_item->getName (), m_umgr,
                                                m_name_entry));
              m_item->setName (m_name_entry->get_text ());
            }));

        add_connection
          (m_uses_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemEditorUndoAction_Uses
                 (m_item->getNumberOfUsesLeft ()));
  
              m_item->setNumberOfUsesLeft (m_uses_spinbutton->get_value ());
            }));

        add_connection
          (m_edit_button->signal_clicked ().connect
           ([this] ()
            {
              auto d = LwDialog::build<ItemBonusEditorDialog> (this);
              d->setup (m_item, m_shield);
              d->signal_response ().connect
                ([this, d] (Gtk::ResponseType)
                 {
                   if (d->is_changed ())
                     {
                       m_umgr->add
                         (new ItemEditorUndoAction_Bonus (m_item));
                       delete m_item;
                       m_item = new Item (*d->get_item (), true);
                       update ();
                     }
                   delete d;
                 });
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
        auto action = dynamic_cast<ItemEditorUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case ItemEditorUndoAction::USES:
              {
                auto a = dynamic_cast<ItemEditorUndoAction_Uses*>(action);
                out =
                  new ItemEditorUndoAction_Uses
                  (m_item->getNumberOfUsesLeft ());

                m_item->setNumberOfUsesLeft (a->get_uses ());
              }
            break;

          case ItemEditorUndoAction::NAME:
              {
                auto a = dynamic_cast<ItemEditorUndoAction_Name*>(action);
                out = new ItemEditorUndoAction_Name (m_item->getName (), m_umgr,
                                                     m_name_entry);

                m_item->setName (a->get_name ());
              } 
            break;

          case ItemEditorUndoAction::BONUS:
              {
                auto a = dynamic_cast<ItemEditorUndoAction_Bonus*>(action);
                out = new ItemEditorUndoAction_Bonus (m_item);

                delete m_item;
                m_item = new Item (*a->get_item (), true);
              }
            break;
          }
        return out;
      }

    void setup_undo ()
      {
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &ItemEditorDialog::execute_action));

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
