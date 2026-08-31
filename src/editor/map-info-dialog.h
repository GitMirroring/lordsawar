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
#ifndef MAP_INFO_DIALOG_H
#define MAP_INFO_DIALOG_H
#include "undo-mgr.h"
#include "map-info-undo-actions.h"
#include "city-list.h"
#include "ruin-list.h"
#include "temple-list.h"
#include "signpost-list.h"
#include "stone-list.h"
#include "port-list.h"
#include "road-list.h"
#include "bridge-list.h"
#include "player-list.h"
#include "stack-list.h"
#include "item-list.h"
#include "reward-list.h"
#include "game-map.h"

class MapInfoDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "map-info.ui";
      }

    MapInfoDialog (BaseObjectType* o,
                   const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_name_entry = load <Gtk::Entry> ("name_entry");
        m_status_label = load <Gtk::Label> ("status_label");
        m_description_textview = load <Gtk::TextView> ("description_textview");
        m_copyright_textview = load <Gtk::TextView> ("copyright_textview");
        m_license_textview = load <Gtk::TextView> ("license_textview");
        m_players_label = load <Gtk::Label> ("players_label");
        m_cities_label = load <Gtk::Label> ("cities_label");
        m_ruins_label = load <Gtk::Label> ("ruins_label");
        m_temples_label = load <Gtk::Label> ("temples_label");
        m_signs_label = load <Gtk::Label> ("signs_label");
        m_stones_label = load <Gtk::Label> ("stones_label");
        m_ports_label = load <Gtk::Label> ("ports_label");
        m_roads_label = load <Gtk::Label> ("roads_label");
        m_bridges_label = load <Gtk::Label> ("bridges_label");
        m_stacks_label = load <Gtk::Label> ("stacks_label");
        m_army_units_label = load <Gtk::Label> ("army_units_label");
        m_keepers_label = load <Gtk::Label> ("keepers_label");
        m_items_label = load <Gtk::Label> ("items_label");
        m_rewards_label = load <Gtk::Label> ("rewards_label");
        m_bags_label = load <Gtk::Label> ("bags_label");
      }

    ~MapInfoDialog ()
      {
        disconnect_signals ();
        delete m_umgr;
      }

    void setup (GameScenario *s)
      {
        m_scenario = s;
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        setup_undo ();

        m_name = m_scenario->getName ();
        m_description = m_scenario->getComment ();
        m_copyright = m_scenario->getCopyright ();
        m_license = m_scenario->getLicense ();

        m_orig_name = m_name;
        m_orig_description = m_description;
        m_orig_copyright = m_copyright;
        m_orig_license = m_license;

        m_name_entry->set_text (m_name);

        m_players_label->set_text
          (String::ucompose ("%1", Playerlist::instance ()->size ()));
        m_cities_label->set_text
          (String::ucompose ("%1", Citylist::instance ()->size ()));
        m_ruins_label->set_text
          (String::ucompose ("%1", Ruinlist::instance ()->size ()));
        m_temples_label->set_text
          (String::ucompose ("%1", Templelist::instance ()->size ()));
        m_signs_label->set_text
          (String::ucompose ("%1", Signpostlist::instance ()->size ()));
        m_stones_label->set_text
          (String::ucompose ("%1", Stonelist::instance ()->size ()));
        m_ports_label->set_text
          (String::ucompose ("%1", Portlist::instance ()->size ()));
        m_roads_label->set_text
          (String::ucompose ("%1", Roadlist::instance ()->size ()));
        m_bridges_label->set_text
          (String::ucompose ("%1", Bridgelist::instance ()->size ()));
        m_stacks_label->set_text
          (String::ucompose ("%1", Playerlist::instance ()->countAllStacks ()));
        m_army_units_label->set_text
          (String::ucompose ("%1", Stacklist::getNoOfArmies ()));
        guint32 num_empty = Ruinlist::instance ()->countEmptyKeepers ();
        if (!num_empty)
          m_keepers_label->set_text
            (String::ucompose ("%1", Ruinlist::instance ()->countKeepers ()));
        else
          m_keepers_label->set_text
            (String::ucompose (_("%1, %2 empty"),
                               Ruinlist::instance ()->countKeepers (),
                               num_empty));
        m_items_label->set_text
          (String::ucompose ("%1", Itemlist::instance ()->size ()));
        m_rewards_label->set_text
          (String::ucompose ("%1", Rewardlist::instance ()->size ()));
        m_bags_label->set_text
          (String::ucompose ("%1", GameMap::countBags ()));

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
    Gtk::Label *m_players_label;
    Gtk::Label *m_cities_label;
    Gtk::Label *m_ruins_label;
    Gtk::Label *m_temples_label;
    Gtk::Label *m_signs_label;
    Gtk::Label *m_stones_label;
    Gtk::Label *m_ports_label;
    Gtk::Label *m_roads_label;
    Gtk::Label *m_bridges_label;
    Gtk::Label *m_stacks_label;
    Gtk::Label *m_army_units_label;
    Gtk::Label *m_keepers_label;
    Gtk::Label *m_items_label;
    Gtk::Label *m_rewards_label;
    Gtk::Label *m_bags_label;

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

    GameScenario *m_scenario;

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
                (new MapInfoUndoAction_Description
                 (m_description, m_umgr, m_description_textview));
              m_description =
                m_description_textview->get_buffer ()->get_text ();
            }));

        add_connection
          (m_copyright_textview->get_buffer ()->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new MapInfoUndoAction_Copyright (m_copyright, m_umgr,
                                                      m_copyright_textview));
              m_copyright = m_copyright_textview->get_buffer ()->get_text ();
            }));

        add_connection
          (m_license_textview->get_buffer ()->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new MapInfoUndoAction_License (m_license, m_umgr,
                                                    m_license_textview));
              m_license = m_license_textview->get_buffer ()->get_text ();
            }));

        add_connection
          (m_name_entry->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new MapInfoUndoAction_Name (m_name, m_umgr, m_name_entry));
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
          (sigc::mem_fun (*this, &MapInfoDialog::execute_action));

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
        auto action = dynamic_cast<MapInfoUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case MapInfoUndoAction::DESCRIPTION:
              {
                auto a = dynamic_cast<MapInfoUndoAction_Description*>(action);
                out =
                  new MapInfoUndoAction_Description
                  (m_description, m_umgr, m_description_textview);
                m_description = a->get_message ();
              }
            break;

          case MapInfoUndoAction::COPYRIGHT:
              {
                auto a = dynamic_cast<MapInfoUndoAction_Copyright*>(action);
                out =
                  new MapInfoUndoAction_Copyright (m_copyright, m_umgr,
                                                   m_copyright_textview);
                m_copyright = a->get_message ();
              }
            break;

          case MapInfoUndoAction::LICENSE:
              {
                auto a = dynamic_cast<MapInfoUndoAction_License*>(action);
                out = new MapInfoUndoAction_License (m_license, m_umgr,
                                                     m_license_textview);
                m_license = a->get_message ();
              }
            break;

          case MapInfoUndoAction::NAME:
              {
                auto a = dynamic_cast<MapInfoUndoAction_Name*>(action);
                out = new MapInfoUndoAction_Name (m_name, m_umgr,
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
        Glib::ustring oldname = m_scenario->getName ();
        m_scenario->setName (String::utrim (m_name_entry->get_text ()));
        m_close_button->set_sensitive
          (File::sanify (m_scenario->getName ()) != "");

        m_scenario->setName (oldname);
        m_name = String::utrim (m_name_entry->get_text ());
      }
};
#endif
