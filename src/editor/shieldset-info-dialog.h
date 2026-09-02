//  Copyright (C) 2007, 2008, 2009, 2010, 2014, 2020, 2021, 2026 Ben Asselstine
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
#ifndef SHIELDSET_INFO_DIALOG_H
#define SHIELDSET_INFO_DIALOG_H
#include "undo-mgr.h"
#include "shieldset-info-undo-actions.h"
#include "shield-set-list.h"

class ShieldSetInfoDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "shieldset-info.ui";
      }

    ShieldSetInfoDialog (BaseObjectType* o,
                       const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_name_entry = load <Gtk::Entry> ("name_entry");
        m_status_label = load <Gtk::Label> ("status_label");
        m_location_label = load <Gtk::Label> ("location_label");
        m_description_textview = load <Gtk::TextView> ("description_textview");
        m_copyright_textview = load <Gtk::TextView> ("copyright_textview");
        m_license_textview = load <Gtk::TextView> ("license_textview");
        m_images_label = load <Gtk::Label> ("images_label");
      }

    ~ShieldSetInfoDialog ()
      {
        disconnect_signals ();
        delete m_umgr;
      }

    void setup (Shieldset *c)
      {
        m_shieldset = c;
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        setup_undo ();

        m_name = m_shieldset->getName ();
        m_description = m_shieldset->getInfo ();
        m_copyright = m_shieldset->getCopyright ();
        m_license = m_shieldset->getLicense ();

        m_orig_name = m_name;
        m_orig_description = m_description;
        m_orig_copyright = m_copyright;
        m_orig_license = m_license;

        m_images_label->set_text
          (String::ucompose ("%1", m_shieldset->countImages ()));
        m_location_label->set_label
          (m_shieldset->getDirectory ().empty () ? "" :
           m_shieldset->getConfigurationFile (true));

        m_name_entry->set_text (m_name);

        update_name ();
        update ();
      }

    bool is_changed ()
      {
        return
          m_orig_description != m_description ||
          m_orig_copyright != m_copyright ||
          m_orig_license != m_license ||
          m_orig_name != m_name;
      }

    Glib::ustring get_name () const
      {
        return m_name;
      }
      
    Glib::ustring get_description () const
      {
        return m_description;
      }

    Glib::ustring get_copyright () const
      {
        return m_copyright;
      }

    Glib::ustring get_license () const
      {
        return m_license;
      }

private:
    Glib::RefPtr<Gtk::Application> m_app;
    Gtk::Button *m_close_button;
    Gtk::Entry *m_name_entry;
    Gtk::Label *m_status_label;
    Gtk::Label *m_location_label;
    Gtk::TextView *m_description_textview;
    Gtk::TextView *m_copyright_textview;
    Gtk::TextView *m_license_textview;
    Gtk::Label *m_images_label;

    UndoMgr *m_umgr;
    std::list<sigc::connection> m_connections;

    Glib::ustring m_name;
    Glib::ustring m_description;
    Glib::ustring m_copyright;
    Glib::ustring m_license;

    Glib::ustring m_orig_name;
    Glib::ustring m_orig_description;
    Glib::ustring m_orig_copyright;
    Glib::ustring m_orig_license;

    Shieldset *m_shieldset;

    void add_connection (sigc::connection connection)
      {
        m_connections.push_back (connection);
      }

    void connect_signals ()
      {
        add_connection
          (m_description_textview->get_buffer ()->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ShieldSetInfoUndoAction_Description
                 (m_description, m_umgr, m_description_textview));
              m_description =
                m_description_textview->get_buffer ()->get_text ();
            }));

        add_connection
          (m_copyright_textview->get_buffer ()->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ShieldSetInfoUndoAction_Copyright (m_copyright, m_umgr,
                                                      m_copyright_textview));
              m_copyright = m_copyright_textview->get_buffer ()->get_text ();
            }));

        add_connection
          (m_license_textview->get_buffer ()->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ShieldSetInfoUndoAction_License (m_license, m_umgr,
                                                    m_license_textview));
              m_license = m_license_textview->get_buffer ()->get_text ();
            }));

        add_connection
          (m_name_entry->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ShieldSetInfoUndoAction_Name (m_name, m_umgr, m_name_entry));
              update_name ();
            }));
      }

    void disconnect_signals ()
      {
        for (auto c : m_connections)
          c.disconnect ();
        m_connections.clear ();
      }

    void setup_undo ()
      {
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &ShieldSetInfoDialog::execute_action));

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
        m_umgr->add_cursor (m_description_textview);
        m_umgr->add_cursor (m_copyright_textview);
        m_umgr->add_cursor (m_license_textview);
      }

    UndoAction* execute_action (UndoAction *action2)
      {
        auto action = dynamic_cast<ShieldSetInfoUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case ShieldSetInfoUndoAction::DESCRIPTION:
              {
                auto a = dynamic_cast<ShieldSetInfoUndoAction_Description*>(action);
                out =
                  new ShieldSetInfoUndoAction_Description
                  (m_description, m_umgr, m_description_textview);
                m_description = a->get_message ();
              }
            break;

          case ShieldSetInfoUndoAction::COPYRIGHT:
              {
                auto a = dynamic_cast<ShieldSetInfoUndoAction_Copyright*>(action);
                out =
                  new ShieldSetInfoUndoAction_Copyright (m_copyright, m_umgr,
                                                         m_copyright_textview);
                m_copyright = a->get_message ();
              }
            break;

          case ShieldSetInfoUndoAction::LICENSE:
              {
                auto a = dynamic_cast<ShieldSetInfoUndoAction_License*>(action);
                out = new ShieldSetInfoUndoAction_License (m_license, m_umgr,
                                                           m_license_textview);
                m_license = a->get_message ();
              }
            break;

          case ShieldSetInfoUndoAction::NAME:
              {
                auto a = dynamic_cast<ShieldSetInfoUndoAction_Name*>(action);
                out = new ShieldSetInfoUndoAction_Name (m_name, m_umgr,
                                                        m_name_entry);
                m_name = a->get_name ();
              }
            break;

          }
        return out;
      }

    void update ()
      {
        disconnect_signals ();
        m_description_textview->get_buffer ()->set_text (m_description);
        m_copyright_textview->get_buffer ()->set_text (m_copyright);
        m_license_textview->get_buffer ()->set_text (m_license);
        if (m_name_entry->get_text () != m_name)
          m_name_entry->set_text (m_name);
        m_umgr->set_cursors ();
        connect_signals ();

      }

    void update_name ()
      {
        Glib::ustring oldname = m_shieldset->getName ();
        guint32 oldsize = m_shieldset->getTileSize ();
        m_shieldset->setName (String::utrim (m_name_entry->get_text ()));
        m_close_button->set_sensitive
          (File::sanify (m_shieldset->getName ()) != "");

        Glib::ustring file =
          Shieldsetlist::instance ()->lookupConfigurationFileByName (m_shieldset);
        if (file != "" && file != m_shieldset->getConfigurationFile (true))
          m_status_label->set_text (_("That name is already in use."));
        else
          m_status_label->set_text ("");
        m_shieldset->setName (oldname);
        m_shieldset->setTileSize (oldsize);
        m_name = String::utrim (m_name_entry->get_text ());
      }
};
#endif
