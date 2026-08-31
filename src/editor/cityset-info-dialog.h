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
#ifndef CITYSET_INFO_DIALOG_H
#define CITYSET_INFO_DIALOG_H
#include "undo-mgr.h"
#include "cityset-info-undo-actions.h"

class CitySetInfoDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "cityset-info.ui";
      }

    CitySetInfoDialog (BaseObjectType* o,
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
        m_size_spinbutton = load <Gtk::SpinButton> ("size_spinbutton");
        m_images_label = load <Gtk::Label> ("images_label");
      }

    ~CitySetInfoDialog ()
      {
        disconnect_signals ();
        delete m_umgr;
      }

    void setup (Cityset *c)
      {
        m_cityset = c;
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        setup_undo ();

        m_name = m_cityset->getName ();
        m_description = m_cityset->getInfo ();
        m_copyright = m_cityset->getCopyright ();
        m_license = m_cityset->getLicense ();
        m_tilesize = m_cityset->getTileSize ();

        m_orig_name = m_name;
        m_orig_description = m_description;
        m_orig_copyright = m_copyright;
        m_orig_license = m_license;
        m_orig_tilesize = m_tilesize;

        m_images_label->set_text
          (String::ucompose ("%1", m_cityset->countImages ()));
        m_location_label->set_label
          (m_cityset->getDirectory ().empty () ? "" :
           m_cityset->getConfigurationFile (true));

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
          m_orig_name != m_name ||
          m_orig_tilesize != m_tilesize;
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

    guint32 get_tile_size () const
      {
        return m_tilesize;
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
    Gtk::SpinButton *m_size_spinbutton;
    Gtk::Label *m_images_label;

    UndoMgr *m_umgr;
    std::list<sigc::connection> m_connections;

    Glib::ustring m_name;
    Glib::ustring m_description;
    Glib::ustring m_copyright;
    Glib::ustring m_license;
    guint32 m_tilesize;
    Glib::ustring m_orig_name;
    Glib::ustring m_orig_description;
    Glib::ustring m_orig_copyright;
    Glib::ustring m_orig_license;
    guint32 m_orig_tilesize;

    Cityset *m_cityset;

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
                (new CitySetInfoUndoAction_Description
                 (m_description, m_umgr, m_description_textview));
              m_description =
                m_description_textview->get_buffer ()->get_text ();
            }));

        add_connection
          (m_copyright_textview->get_buffer ()->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new CitySetInfoUndoAction_Copyright (m_copyright, m_umgr,
                                                      m_copyright_textview));
              m_copyright = m_copyright_textview->get_buffer ()->get_text ();
            }));

        add_connection
          (m_license_textview->get_buffer ()->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new CitySetInfoUndoAction_License (m_license, m_umgr,
                                                    m_license_textview));
              m_license = m_license_textview->get_buffer ()->get_text ();
            }));

        add_connection
          (m_size_spinbutton->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new CitySetInfoUndoAction_TileSize (m_tilesize));
              m_tilesize = m_size_spinbutton->get_value ();
              update_name ();
            }));

        add_connection
          (m_name_entry->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new CitySetInfoUndoAction_Name (m_name, m_umgr, m_name_entry));
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
          (sigc::mem_fun (*this, &CitySetInfoDialog::execute_action));

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
        auto action = dynamic_cast<CitySetInfoUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case CitySetInfoUndoAction::DESCRIPTION:
              {
                auto a = dynamic_cast<CitySetInfoUndoAction_Description*>(action);
                out =
                  new CitySetInfoUndoAction_Description
                  (m_description, m_umgr, m_description_textview);
                m_description = a->get_message ();
              }
            break;

          case CitySetInfoUndoAction::COPYRIGHT:
              {
                auto a = dynamic_cast<CitySetInfoUndoAction_Copyright*>(action);
                out =
                  new CitySetInfoUndoAction_Copyright (m_copyright, m_umgr,
                                                       m_copyright_textview);
                m_copyright = a->get_message ();
              }
            break;

          case CitySetInfoUndoAction::LICENSE:
              {
                auto a = dynamic_cast<CitySetInfoUndoAction_License*>(action);
                out = new CitySetInfoUndoAction_License (m_license, m_umgr,
                                                         m_license_textview);
                m_license = a->get_message ();
              }
            break;

          case CitySetInfoUndoAction::NAME:
              {
                auto a = dynamic_cast<CitySetInfoUndoAction_Name*>(action);
                out = new CitySetInfoUndoAction_Name (m_name, m_umgr,
                                                      m_name_entry);
                m_name = a->get_name ();
              }
            break;

          case CitySetInfoUndoAction::TILE_SIZE:
              {
                auto a = dynamic_cast<CitySetInfoUndoAction_TileSize*>(action);
                out = new CitySetInfoUndoAction_TileSize (m_tilesize);
                m_tilesize = a->get_tile_size ();
                update_name ();
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
        m_size_spinbutton->set_value (m_tilesize);
        m_umgr->set_cursors ();
        connect_signals ();

      }

    void update_name ()
      {
        Glib::ustring oldname = m_cityset->getName ();
        guint32 oldsize = m_cityset->getTileSize ();
        m_cityset->setName (String::utrim (m_name_entry->get_text ()));
        m_cityset->setTileSize (m_tilesize);
        m_close_button->set_sensitive
          (File::sanify (m_cityset->getName ()) != "");

        Glib::ustring file =
          Citysetlist::instance ()->lookupConfigurationFileByName (m_cityset);
        if (file != "" && file != m_cityset->getConfigurationFile (true))
          m_status_label->set_text (_("That name is already in use."));
        else
          m_status_label->set_text ("");
        m_cityset->setName (oldname);
        m_cityset->setTileSize (oldsize);
        m_name = String::utrim (m_name_entry->get_text ());
      }
};
#endif
