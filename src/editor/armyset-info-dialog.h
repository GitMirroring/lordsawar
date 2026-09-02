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
#ifndef ARMYSET_INFO_DIALOG_H
#define ARMYSET_INFO_DIALOG_H
#include "undo-mgr.h"
#include "armyset-info-undo-actions.h"
#include "army-set-list.h"

class ArmySetInfoDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "armyset-info.ui";
      }

    ArmySetInfoDialog (BaseObjectType* o,
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
        m_armies_label = load <Gtk::Label> ("armies_label");
        m_selectors_label = load <Gtk::Label> ("selectors_label");
        m_ship_label = load <Gtk::Label> ("ship_label");
        m_bag_label = load <Gtk::Label> ("bag_label");
        m_flag_label = load <Gtk::Label> ("flag_label");
      }

    ~ArmySetInfoDialog ()
      {
        disconnect_signals ();
        delete m_umgr;
      }

    void setup (Armyset *c)
      {
        m_armyset = c;
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        setup_undo ();

        m_armies_label->set_text (String::ucompose ("%1", m_armyset->size ()));
        m_selectors_label->set_text
          (String::ucompose ("%1", m_armyset->countSelectors ()));
        m_bag_label->set_text (m_armyset->getBag ()->getName () == "" ?
                               _("Not present") : _("Present"));
        m_ship_label->set_text (m_armyset->getShip ()->getName () == "" ?
                                _("Not present") : _("Present"));
        m_flag_label->set_text (m_armyset->getStandard ()->getName () == "" ?
                                _("Not present") : _("Present"));

        m_name = m_armyset->getName ();
        m_description = m_armyset->getInfo ();
        m_copyright = m_armyset->getCopyright ();
        m_license = m_armyset->getLicense ();
        m_tilesize = m_armyset->getTileSize ();

        m_orig_name = m_name;
        m_orig_description = m_description;
        m_orig_copyright = m_copyright;
        m_orig_license = m_license;
        m_orig_tilesize = m_tilesize;

        m_images_label->set_text
          (String::ucompose ("%1", m_armyset->countImages ()));
        m_location_label->set_label
          (m_armyset->getDirectory ().empty () ? "" :
           m_armyset->getConfigurationFile (true));

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
    Gtk::Label *m_armies_label;
    Gtk::Label *m_selectors_label;
    Gtk::Label *m_bag_label;
    Gtk::Label *m_ship_label;
    Gtk::Label *m_flag_label;

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

    Armyset *m_armyset;

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
                (new ArmySetInfoUndoAction_Description
                 (m_description, m_umgr, m_description_textview));
              m_description =
                m_description_textview->get_buffer ()->get_text ();
            }));

        add_connection
          (m_copyright_textview->get_buffer ()->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ArmySetInfoUndoAction_Copyright (m_copyright, m_umgr,
                                                      m_copyright_textview));
              m_copyright = m_copyright_textview->get_buffer ()->get_text ();
            }));

        add_connection
          (m_license_textview->get_buffer ()->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ArmySetInfoUndoAction_License (m_license, m_umgr,
                                                    m_license_textview));
              m_license = m_license_textview->get_buffer ()->get_text ();
            }));

        add_connection
          (m_size_spinbutton->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new ArmySetInfoUndoAction_TileSize (m_tilesize));
              m_tilesize = m_size_spinbutton->get_value ();
              update_name ();
            }));

        add_connection
          (m_name_entry->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ArmySetInfoUndoAction_Name (m_name, m_umgr, m_name_entry));
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
          (sigc::mem_fun (*this, &ArmySetInfoDialog::execute_action));

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
        auto action = dynamic_cast<ArmySetInfoUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case ArmySetInfoUndoAction::DESCRIPTION:
              {
                auto a = dynamic_cast<ArmySetInfoUndoAction_Description*>(action);
                out =
                  new ArmySetInfoUndoAction_Description
                  (m_description, m_umgr, m_description_textview);
                m_description = a->get_message ();
              }
            break;

          case ArmySetInfoUndoAction::COPYRIGHT:
              {
                auto a = dynamic_cast<ArmySetInfoUndoAction_Copyright*>(action);
                out =
                  new ArmySetInfoUndoAction_Copyright (m_copyright, m_umgr,
                                                       m_copyright_textview);
                m_copyright = a->get_message ();
              }
            break;

          case ArmySetInfoUndoAction::LICENSE:
              {
                auto a = dynamic_cast<ArmySetInfoUndoAction_License*>(action);
                out = new ArmySetInfoUndoAction_License (m_license, m_umgr,
                                                         m_license_textview);
                m_license = a->get_message ();
              }
            break;

          case ArmySetInfoUndoAction::NAME:
              {
                auto a = dynamic_cast<ArmySetInfoUndoAction_Name*>(action);
                out = new ArmySetInfoUndoAction_Name (m_name, m_umgr,
                                                      m_name_entry);
                m_name = a->get_name ();
              }
            break;

          case ArmySetInfoUndoAction::TILE_SIZE:
              {
                auto a = dynamic_cast<ArmySetInfoUndoAction_TileSize*>(action);
                out = new ArmySetInfoUndoAction_TileSize (m_tilesize);
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
        Glib::ustring oldname = m_armyset->getName ();
        guint32 oldsize = m_armyset->getTileSize ();
        m_armyset->setName (String::utrim (m_name_entry->get_text ()));
        m_armyset->setTileSize (m_tilesize);
        m_close_button->set_sensitive
          (File::sanify (m_armyset->getName ()) != "");

        Glib::ustring file =
          Armysetlist::instance ()->lookupConfigurationFileByName (m_armyset);
        if (file != "" && file != m_armyset->getConfigurationFile (true))
          m_status_label->set_text (_("That name is already in use."));
        else
          m_status_label->set_text ("");
        m_armyset->setName (oldname);
        m_armyset->setTileSize (oldsize);
        m_name = String::utrim (m_name_entry->get_text ());
      }
};
#endif
