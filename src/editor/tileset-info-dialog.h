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
#ifndef TILESET_INFO_DIALOG_H
#define TILESET_INFO_DIALOG_H
#include "undo-mgr.h"
#include "tileset-info-undo-actions.h"
#include "tile-set-list.h"

class TileSetInfoDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "tileset-info.ui";
      }

    TileSetInfoDialog (BaseObjectType* o,
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
        m_tiles_label = load <Gtk::Label> ("tiles_label");
        m_tilestyles_label = load <Gtk::Label> ("tilestyles_label");
        m_selector_label = load <Gtk::Label> ("selector_label");
        m_explosion_label = load <Gtk::Label> ("explosion_label");
        m_roads_label = load <Gtk::Label> ("roads_label");
        m_bridges_label = load <Gtk::Label> ("bridges_label");
        m_stones_label = load <Gtk::Label> ("stones_label");
        m_fog_label = load <Gtk::Label> ("fog_label");
        m_flags_label = load <Gtk::Label> ("flags_label");
        m_move_bonus_label = load <Gtk::Label> ("move_bonus_label");
      }

    ~TileSetInfoDialog ()
      {
        disconnect_signals ();
        delete m_umgr;
      }

    void setup (Tileset *c)
      {
        m_tileset = c;
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        setup_undo ();

        m_tiles_label->set_text (String::ucompose ("%1", m_tileset->size ()));
        m_tilestyles_label->set_text (String::ucompose ("%1", m_tileset->countTileStyles ()));
        m_selector_label->set_text
          (m_tileset->getSelector (false)->getName () == "" ||
           m_tileset->getSelector (true)->getName () == "" ?
           _("Not present") : _("Present"));
        m_explosion_label->set_text
          (m_tileset->getExplosion ()->getName () == "" ?
           _("Not present") : _("Present"));
        m_roads_label->set_text (m_tileset->getRoad ()->getName () == "" ?
                                 _("Not present") : _("Present"));
        m_bridges_label->set_text (m_tileset->getBridge ()->getName () == "" ?
                                   _("Not present") : _("Present"));
        m_stones_label->set_text (m_tileset->getStone ()->getName () == "" ?
                                  _("Not present") : _("Present"));
        m_fog_label->set_text (m_tileset->getFog ()->getName () == "" ?
                               _("Not present") : _("Present"));

        guint32 count = 0;
        for (guint32 i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
          {
            Shield::Color shield = Shield::Color (i);
            if (m_tileset->getFlags (shield)->getName () != "")
              count++;
          }
        m_flags_label->set_text
          (String::ucompose (_("%1 of %2"), count, MAX_PLAYERS + 1));

        m_move_bonus_label->set_text
          (String::ucompose ("%1 of 6", m_tileset->countMoveBonusImages ()));

        m_name = m_tileset->getName ();
        m_description = m_tileset->getInfo ();
        m_copyright = m_tileset->getCopyright ();
        m_license = m_tileset->getLicense ();
        m_tilesize = m_tileset->getTileSize ();

        m_orig_name = m_name;
        m_orig_description = m_description;
        m_orig_copyright = m_copyright;
        m_orig_license = m_license;
        m_orig_tilesize = m_tilesize;

        m_images_label->set_text
          (String::ucompose ("%1", m_tileset->countImages ()));
        m_location_label->set_label
          (m_tileset->getDirectory ().empty () ? "" :
           m_tileset->getConfigurationFile (true));

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
    Gtk::Label *m_tiles_label;
    Gtk::Label *m_tilestyles_label;
    Gtk::Label *m_selector_label;
    Gtk::Label *m_explosion_label;
    Gtk::Label *m_roads_label;
    Gtk::Label *m_bridges_label;
    Gtk::Label *m_stones_label;
    Gtk::Label *m_fog_label;
    Gtk::Label *m_flags_label;
    Gtk::Label *m_move_bonus_label;

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

    Tileset *m_tileset;

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
                (new TileSetInfoUndoAction_Description
                 (m_description, m_umgr, m_description_textview));
              m_description =
                m_description_textview->get_buffer ()->get_text ();
            }));

        add_connection
          (m_copyright_textview->get_buffer ()->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new TileSetInfoUndoAction_Copyright (m_copyright, m_umgr,
                                                      m_copyright_textview));
              m_copyright = m_copyright_textview->get_buffer ()->get_text ();
            }));

        add_connection
          (m_license_textview->get_buffer ()->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new TileSetInfoUndoAction_License (m_license, m_umgr,
                                                    m_license_textview));
              m_license = m_license_textview->get_buffer ()->get_text ();
            }));

        add_connection
          (m_size_spinbutton->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new TileSetInfoUndoAction_TileSize (m_tilesize));
              m_tilesize = m_size_spinbutton->get_value ();
              update_name ();
            }));

        add_connection
          (m_name_entry->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new TileSetInfoUndoAction_Name (m_name, m_umgr, m_name_entry));
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
          (sigc::mem_fun (*this, &TileSetInfoDialog::execute_action));

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
        auto action = dynamic_cast<TileSetInfoUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case TileSetInfoUndoAction::DESCRIPTION:
              {
                auto a = dynamic_cast<TileSetInfoUndoAction_Description*>(action);
                out =
                  new TileSetInfoUndoAction_Description
                  (m_description, m_umgr, m_description_textview);
                m_description = a->get_message ();
              }
            break;

          case TileSetInfoUndoAction::COPYRIGHT:
              {
                auto a = dynamic_cast<TileSetInfoUndoAction_Copyright*>(action);
                out =
                  new TileSetInfoUndoAction_Copyright (m_copyright, m_umgr,
                                                       m_copyright_textview);
                m_copyright = a->get_message ();
              }
            break;

          case TileSetInfoUndoAction::LICENSE:
              {
                auto a = dynamic_cast<TileSetInfoUndoAction_License*>(action);
                out = new TileSetInfoUndoAction_License (m_license, m_umgr,
                                                         m_license_textview);
                m_license = a->get_message ();
              }
            break;

          case TileSetInfoUndoAction::NAME:
              {
                auto a = dynamic_cast<TileSetInfoUndoAction_Name*>(action);
                out = new TileSetInfoUndoAction_Name (m_name, m_umgr,
                                                      m_name_entry);
                m_name = a->get_name ();
              }
            break;

          case TileSetInfoUndoAction::TILE_SIZE:
              {
                auto a = dynamic_cast<TileSetInfoUndoAction_TileSize*>(action);
                out = new TileSetInfoUndoAction_TileSize (m_tilesize);
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
        Glib::ustring oldname = m_tileset->getName ();
        guint32 oldsize = m_tileset->getTileSize ();
        m_tileset->setName (String::utrim (m_name_entry->get_text ()));
        m_tileset->setTileSize (m_tilesize);
        m_close_button->set_sensitive
          (File::sanify (m_tileset->getName ()) != "");

        Glib::ustring file =
          Tilesetlist::instance ()->lookupConfigurationFileByName (m_tileset);
        if (file != "" && file != m_tileset->getConfigurationFile (true))
          m_status_label->set_text (_("That name is already in use."));
        else
          m_status_label->set_text ("");
        m_tileset->setName (oldname);
        m_tileset->setTileSize (oldsize);
        m_name = String::utrim (m_name_entry->get_text ());
      }
};
#endif
