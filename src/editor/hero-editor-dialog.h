//  Copyright (C) 2009, 2014, 2020, 2021, 2026 Ben Asselstine
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
#ifndef HERO_EDITOR_DIALOG_H
#define HERO_EDITOR_DIALOG_H
#include "item.h"
#include "hero-editor-undo-actions.h"
#include "backpack-editor-dialog.h"
#include "select-character-dialog.h"
#include "button-label.h"
class HeroEditorDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "hero-editor.ui";
      }

    HeroEditorDialog (BaseObjectType* o,
                      const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_name_entry = load <Gtk::Entry> ("name_entry");
        m_close_button = load <Gtk::Button> ("close_button");
        m_edit_backpack_button = load <Gtk::Button> ("edit_backpack_button");
        m_edit_character_button = load <Gtk::Button> ("edit_character_button");
        m_gender_combobox = load <Gtk::Box> ("gender_combobox");
      }

    ~HeroEditorDialog ()
      {
        disconnect_signals ();
        delete m_umgr;
      }

    void setup (Hero *hero)
      {
        m_hero = hero;
        fill_gender_combo ();
        m_edit_backpack_button_label =
          Gtk::make_managed <ButtonLabel> (m_edit_backpack_button);
        m_edit_character_button_label =
          Gtk::make_managed <ButtonLabel> (m_edit_character_button);
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
    Gtk::Button *m_close_button;
    Gtk::Button *m_edit_backpack_button;
    ButtonLabel *m_edit_backpack_button_label;
    Gtk::Button *m_edit_character_button;
    ButtonLabel *m_edit_character_button_label;
    Gtk::Box *m_gender_combobox;
    LwCombo *m_gender_combo;

    Hero *m_hero;
    UndoMgr *m_umgr;
    std::list<sigc::connection> m_connections;

    void add_connection (sigc::connection c)
      {
        m_connections.push_back (c);
      }

    void update ()
      {
        disconnect_signals ();

        m_edit_backpack_button_label->set_label
          (String::ucompose (ngettext ("%1 item",
                                       "%1 items",
                                       m_hero->getBackpack ()->size ()),
                             m_hero->getBackpack ()->size ()));

        if (m_hero->getOwner () == Playerlist::getNeutral ())
          {
            m_edit_character_button_label->set_label (_("No character set"));
            m_edit_character_button->set_sensitive (false);
          }
        else
          {
            Character *c =
              HeroTemplates::instance ()->getCharacterById
              (m_hero->getCharacterId ());

            if (c)
              m_edit_character_button_label->set_label (c->get_name ());
            else
              m_edit_character_button_label->set_label (_("No character set"));
          }
        m_name_entry->set_text (m_hero->getName ());
        m_gender_combo->set_active ((int)m_hero->getGender ());
        connect_signals ();
      }

    void connect_signals ()
      {
        add_connection
          (m_name_entry->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new HeroEditorUndoAction_Name (m_hero->getName (), m_umgr,
                                                m_name_entry));
              m_hero->setName (m_name_entry->get_text ());
            }));

        add_connection
          (m_gender_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new HeroEditorUndoAction_Gender
                 (Hero::Gender (m_hero->getGender ())));
              m_hero->setGender
                (Hero::Gender (m_gender_combo->get_active_row_number ()));
            }));

        add_connection
          (m_edit_backpack_button->signal_clicked ().connect
           ([this] ()
            {
              auto d = LwDialog::build<BackpackEditorDialog> (this);
              d->setup (m_hero->getBackpack (),
                        m_hero->getOwner ()->get_shield ());
              d->signal_response ().connect
                ([this, d] (Gtk::ResponseType)
                 {
                   if (d->is_changed ())
                     {
                       m_umgr->add
                         (new HeroEditorUndoAction_Backpack (m_hero->getBackpack ()));
                       m_hero->getBackpack ()->removeAllFromBackpack ();
                       m_hero->getBackpack ()->add (d->get_backpack ());
                       update ();
                     }
                   delete d;
                 });
            }));

        add_connection
          (m_edit_character_button->signal_clicked ().connect
           ([this] ()
            {
              auto d = LwDialog::build<SelectCharacterDialog> (this);
              d->setup (m_hero->getCharacterId (),
                        m_hero->getOwner ()->get_shield ());
              d->signal_response ().connect
                ([this, d] (Gtk::ResponseType resp)
                 {
                   switch (resp)
                     {
                     case Gtk::ResponseType::ACCEPT:
                       m_umgr->add
                         (new HeroEditorUndoAction_Character
                          (m_hero->getCharacterId ()));
                       m_hero->setCharacterId
                         (d->get_selected_character ());
                       update ();
                       break;

                     default:
                       break;
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
        auto action = dynamic_cast<HeroEditorUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case HeroEditorUndoAction::NAME:
              {
                auto a = dynamic_cast<HeroEditorUndoAction_Name*>(action);
                out = new HeroEditorUndoAction_Name (m_hero->getName (), m_umgr,
                                                     m_name_entry);

                m_hero->setName (a->get_name ());
              } 
            break;

          case HeroEditorUndoAction::GENDER:
              {
                auto a = dynamic_cast<HeroEditorUndoAction_Gender*>(action);
                out = new HeroEditorUndoAction_Gender
                  (Hero::Gender (m_hero->getGender ()));

                m_hero->setGender (a->get_gender ());
              }
            break;

          case HeroEditorUndoAction::BACKPACK:
              {
                auto a = dynamic_cast<HeroEditorUndoAction_Backpack*>(action);
                out = new HeroEditorUndoAction_Backpack (m_hero->getBackpack ());

                m_hero->getBackpack ()->removeAllFromBackpack ();
                m_hero->getBackpack ()->add (a->get_backpack ());
              }
            break;

          case HeroEditorUndoAction::CHARACTER:
              {
                auto a = dynamic_cast<HeroEditorUndoAction_Character*>(action);
                out =
                  new HeroEditorUndoAction_Character
                  (m_hero->getCharacterId ());

                m_hero->setCharacterId (a->get_character_id ());
              }
            break;
          }
        return out;
      }

    void setup_undo ()
      {
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &HeroEditorDialog::execute_action));

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

    void fill_gender_combo ()
      {
        m_gender_combo = Gtk::make_managed<LwCombo> ();
        m_gender_combo->append (_("None"));
        m_gender_combo->append (_("Male"));
        m_gender_combo->append (_("Female"));
        m_gender_combo->set_hexpand (false);
        m_gender_combobox->append (*m_gender_combo);
      }
};
#endif
