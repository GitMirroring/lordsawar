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
#ifndef TEMPLE_EDITOR_DIALOG_H
#define TEMPLE_EDITOR_DIALOG_H
#include "temple-editor-undo-actions.h"
class TempleEditorDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "temple-editor.ui";
      }

    TempleEditorDialog (BaseObjectType* o,
                        const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_name_entry = load <Gtk::Entry> ("name_entry");
        m_close_button = load <Gtk::Button> ("close_button");
        m_description_entry = load <Gtk::Entry> ("description_entry");
        m_type_spinbutton = load <Gtk::SpinButton> ("type_spinbutton");
        m_randomize_name_button = load <Gtk::Button> ("randomize_name_button");
        m_temple_picture = load <Gtk::Picture> ("temple_picture");
      }

    ~TempleEditorDialog ()
      {
        delete m_temple;
        disconnect_signals ();
        delete m_umgr;
      }

    void setup (Temple *temple, CreateScenarioRandomize *randomize)
      {
        m_randomizer = randomize;
        m_temple = new Temple (*temple, true);
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);
        setup_undo ();
        update ();
      }

    Temple *get_temple ()
      {
        return m_temple;
      }

    bool is_changed ()
      {
        return m_umgr->undo_empty () == false;
      }
private:
    Gtk::Entry *m_name_entry;;
    Gtk::Button *m_close_button;;
    Gtk::Entry *m_description_entry;;
    Gtk::SpinButton *m_type_spinbutton;;
    Gtk::Button *m_randomize_name_button;;
    Gtk::Picture *m_temple_picture;

    Temple *m_temple;
    CreateScenarioRandomize *m_randomizer;
    UndoMgr *m_umgr;
    std::list<sigc::connection> m_connections;

    void add_connection (sigc::connection c)
      {
        m_connections.push_back (c);
      }

    void update ()
      {
        disconnect_signals ();

        m_name_entry->set_text (m_temple->getName ());
        m_description_entry->set_text (m_temple->getDescription ());
        m_type_spinbutton->set_value (m_temple->getType ());

        m_temple_picture->set_paintable
          (ImageCache::instance ()->getTemplePic
           (m_temple->getType ())->to_texture ());

        connect_signals ();
      }

    void connect_signals ()
      {
        add_connection
          (m_randomize_name_button->signal_clicked ().connect
           ([this] ()
            {
              m_umgr->add
                (new TempleEditorUndoAction_RandomizeName
                 (m_temple->getName ()));

              Glib::ustring existing_name = m_name_entry->get_text();
              if (existing_name == Temple::getDefaultName ())
                m_name_entry->set_text (m_randomizer->popRandomTempleName ());
              else
                {
                  m_name_entry->set_text (m_randomizer->popRandomTempleName());
                  m_randomizer->pushRandomTempleName (existing_name);
                }
            }));

        add_connection
          (m_name_entry->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new TempleEditorUndoAction_Name (m_temple->getName (), m_umgr,
                                                  m_name_entry));
              m_temple->setName (m_name_entry->get_text ());
            }));

        add_connection
          (m_type_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new TempleEditorUndoAction_Type
                 (Temple::Type (m_temple->getType ())));
              m_temple->setType (Temple::Type (m_type_spinbutton->get_value_as_int ()));
              update ();
            }));

        add_connection
          (m_description_entry->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new TempleEditorUndoAction_Description
                 (m_temple->getDescription (), m_umgr,
                  m_description_entry));
              m_temple->setDescription (m_description_entry->get_text ());
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
        auto action = dynamic_cast<TempleEditorUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case TempleEditorUndoAction::NAME:
              {
                auto a = dynamic_cast<TempleEditorUndoAction_Name*>(action);
                out = new TempleEditorUndoAction_Name (m_temple->getName (),
                                                       m_umgr, m_name_entry);

                m_temple->setName (a->get_name ());
              }
            break;

          case TempleEditorUndoAction::DESCRIPTION:
              {
                auto a =
                  dynamic_cast<TempleEditorUndoAction_Description*>(action);
                out = new TempleEditorUndoAction_Name
                  (m_temple->getDescription (), m_umgr, m_description_entry);

                m_temple->setDescription (a->get_description ());
              }
            break;

          case TempleEditorUndoAction::TYPE:
              {
                auto a = dynamic_cast<TempleEditorUndoAction_Type*>(action);
                out = new TempleEditorUndoAction_Type
                  (Temple::Type (m_temple->getType ()));

                m_temple->setType (Temple::Type (a->get_temple_type ()));
              }
            break;

          case TempleEditorUndoAction::RANDOMIZE_NAME:
              {
                auto a =
                  dynamic_cast<TempleEditorUndoAction_RandomizeName*>(action);
                out =
                  new TempleEditorUndoAction_RandomizeName
                  (m_temple->getName ());

                m_temple->setName (a->get_name ());
              }
            break;

          }
        return out;
      }

    void setup_undo ()
      {
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &TempleEditorDialog::execute_action));

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
        m_umgr->add_cursor (m_description_entry);
      }
};
#endif
