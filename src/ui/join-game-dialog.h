//  Copyright (C) 2008, 2009, 2011, 2014, 2015, 2026 Ben Asselstine
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
#ifndef JOIN_GAME_DIALOG_H
#define JOIN_GAME_DIALOG_H
#include "lw-dialog-base.h"
#include "profile.h"
#include "recently-played-game-list.h"
#include "game-list-client.h"
#include "lw-column.h"
#include "new-connection-dialog.h"

class GameRow: public Glib::Object
{
public:
    Glib::ustring m_name;
    guint32 m_turn;
    guint32 m_num_players;
    guint32 m_num_cities;
    Glib::ustring m_host;
    guint32 m_port;

    static Glib::RefPtr<GameRow> create
      (Glib::ustring n, guint32 t, guint32 np, guint32 nc, Glib::ustring h,
       guint32 p)
      {
        return
          Glib::make_refptr_for_instance<GameRow> (new GameRow (n, t, np, nc,
                                                                h, p));
      }

protected:
    GameRow (Glib::ustring n, guint32 t, guint32 np, guint32 nc,
             Glib::ustring h, guint32 p)
      : m_name (n), m_turn (t), m_num_players (np), m_num_cities (nc),
      m_host (h), m_port (p)
      {
      }
};

class JoinGameDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "join-game.ui";
      }

    JoinGameDialog (BaseObjectType* o,
                    const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_connect_button = load <Gtk::Button> ("connect_button");
        m_new_button = load <Gtk::Button> ("new_button");
        m_cancel_button = load <Gtk::Button> ("cancel_button");
        m_clear_button = load <Gtk::Button> ("clear_button");
        m_treeview = load <Gtk::ColumnView> ("treeview");
      }

    void setup (Profile *profile)
      {
        m_profile = profile;
        m_have_recent = false;
        set_response (m_connect_button, Gtk::ResponseType::ACCEPT);
        set_response (m_cancel_button, Gtk::ResponseType::CANCEL);

        m_store = Gio::ListStore<GameRow>::create ();
        m_selection_model = Gtk::SingleSelection::create (m_store);
        m_treeview->set_model (m_selection_model);
        setup_columns (m_treeview);

        m_connect_button->set_sensitive (false);
        m_clear_button->set_sensitive (false);

        m_selection_model->signal_selection_changed ().connect
          ([this] (const guint &, const guint &)
           {
             auto row = m_selection_model->get_selected_item ();
             if (row)
               m_connect_button->set_sensitive (true);
           });

        m_treeview->signal_activate ().connect
          ([this](guint pos)
           {
             auto i = m_selection_model->get_model ()->get_object (pos);
             auto game = std::dynamic_pointer_cast<GameRow>(i);

             if (game)
               {
                 std::cout << "Activated: " << game->m_name << std::endl;
               }
           });

        m_clear_button->signal_clicked ().connect
          ([this] ()
           {
             auto rpgl = RecentlyPlayedGameList::instance ();
             rpgl->removeAllNetworkedGames ();
             rpgl->save ();
             fill_treeview ();
             update_buttons ();
           });

        m_new_button->signal_clicked ().connect
          ([this] ()
           {
              auto d = LwDialog::build<NewConnectionDialog>(this);
              d->setup ();
              d->signal_game_selected ().connect
                ([this, d] (Glib::ustring host, unsigned short port)
                 {
                   hide ();
                   m_game_selected.emit (host, port);
                 });

              d->signal_response ().connect
                ([d] (Gtk::ResponseType)
                 {
                   d->hide ();
                   delete d;
                 });
           });

        signal_response ().connect
          ([this](Gtk::ResponseType resp)
           {
             hide ();
             switch (resp)
               {
               case Gtk::ResponseType::ACCEPT: //connect
                   {
                     auto item = m_selection_model->get_selected_item ();
                     auto row = std::dynamic_pointer_cast<GameRow>(item);
                     m_game_selected.emit (row->m_host, row->m_port);
                   }
                 break;
               default:
                 break;
               }
           });
        fill_treeview ();
        update_buttons ();
      }

    sigc::signal<void (Glib::ustring, unsigned short)> signal_game_selected ()
      {
        return m_game_selected;
      }

private:
    Gtk::Button *m_clear_button;
    Gtk::ColumnView *m_treeview;
    Gtk::Button *m_cancel_button;
    Gtk::Button *m_connect_button;
    Gtk::Button *m_new_button;
    Glib::RefPtr<Gtk::SingleSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<GameRow>> m_store;
    Profile *m_profile;
    bool m_have_recent;
    sigc::signal<void (Glib::ustring /*ip*/, unsigned short /*port*/)> m_game_selected;

    void setup_columns (Gtk::ColumnView *column_view)
      {
        LwColumn::setup_text_column<GameRow>
          (column_view, "name_label", true, Gtk::Justification::LEFT,
           _("Name"),
           [] (const auto& row)
           {
             return row->m_name;
           });

        LwColumn::setup_text_column<GameRow>
          (column_view, "turn_label", true, Gtk::Justification::LEFT,
           _("Turn"),
           [] (const auto& row)
           {
             return String::ucompose ("%1", row->m_turn);
           });

        LwColumn::setup_text_column<GameRow>
          (column_view, "num_players_label", true, Gtk::Justification::LEFT,
           _("Players"),
           [] (const auto& row)
           {
             return String::ucompose ("%1", row->m_num_players);
           });

        LwColumn::setup_text_column<GameRow>
          (column_view, "num_cities_label", true, Gtk::Justification::LEFT,
           _("Cities"),
           [] (const auto& row)
           {
             return String::ucompose ("%1", row->m_num_cities);
           });

        LwColumn::setup_text_column<GameRow>
          (column_view, "host_label", true, Gtk::Justification::LEFT,
           _("Host"),
           [] (const auto& row)
           {
             return row->m_host;
           });

        LwColumn::setup_text_column<GameRow>
          (column_view, "port_label", true, Gtk::Justification::LEFT,
           _("Port"),
           [] (const auto& row)
           {
             return String::ucompose ("%1", row->m_port);
           });
      }

    void add_game (Glib::RefPtr<Gio::ListStore<GameRow>> list, RecentlyPlayedNetworkedGame*g)
      {
        list->append
          (GameRow::create
           (g->getName (),
            g->getRound (),
            g->getNumberOfPlayers (),
            g->getNumberOfCities (),
            g->getHost (),
            g->getPort ()));
      }

    void fill_games (RecentlyPlayedGameList *rpgl, Glib::RefPtr<Gio::ListStore<GameRow>> list, Profile *p)
      {
        for (auto it = rpgl->begin (); it != rpgl->end (); ++it)
          {
            if ((*it)->getPlayMode () == GameScenario::NETWORKED)
              {
                RecentlyPlayedNetworkedGame *game;
                game = dynamic_cast<RecentlyPlayedNetworkedGame*>(*it);
                if (p)
                  {
                    if (game->getProfileId () == p->getId ())
                      add_game (list, game);
                  }
                else
                  add_game (list, game);
              }
          }
      }

    void fill_treeview ()
      {
        m_store->remove_all ();
        if (Configuration::s_gamelist_server_hostname != "" &&
            Configuration::s_gamelist_server_port != 0)
          {
            GamelistClient::instance ()->client_connected.connect
              ([this] ()
               {
                 GamelistClient::instance ()->request_game_list ();
               });

            GamelistClient::instance ()->client_could_not_connect.connect
              ([] ()
               {
                 ;
               });

            GamelistClient::instance ()->received_game_list.connect
              ([this] (RecentlyPlayedGameList *rpgl, Glib::ustring err)
               {
                 if (err == "")
                   fill_games (rpgl, m_store, NULL);

                 delete rpgl;
                 GamelistClient::deleteInstance ();
               });

            GamelistClient::instance ()->start 
              (Configuration::s_gamelist_server_hostname, 
               Configuration::s_gamelist_server_port, m_profile);
          }

        auto rpgl = RecentlyPlayedGameList::instance ();
        m_have_recent = rpgl->empty () == false;
        rpgl->pruneGames ();
        fill_games (rpgl, m_store, m_profile);
      }

    void update_buttons ()
      {
        m_clear_button->set_sensitive (m_have_recent);
        auto row = m_selection_model->get_selected_item ();
        if (row)
          m_connect_button->set_sensitive (true);
        else
          m_connect_button->set_sensitive (false);
      }
};
#endif
