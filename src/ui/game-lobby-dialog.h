//  Copyright (C) 2008, 2009, 2011, 2014, 2015, 2017, 2020, 2021,
//  2026 Ben Asselstine
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
#ifndef GAME_LOBBY_DIALOG_H
#define GAME_LOBBY_DIALOG_H

#include "lw-dialog-base.h"
#include "game-scenario.h"
#include "next-turn-networked.h"
#include "game-station.h"
#include "game-options-dialog.h"
#include "lobby.h"
#include "city-map.h"
#include "game-server.h"
#include "game-client.h"
#include "ruin-list.h"
#include "image-helpers.h"
#include "player-list.h"

class ChatPersonRow: public Glib::Object
{
public:
    Glib::ustring m_profile_id;

    static Glib::RefPtr<ChatPersonRow> create (Glib::ustring id)
      {
        return
          Glib::make_refptr_for_instance<ChatPersonRow>
          (new ChatPersonRow (id));
      }

protected:
    ChatPersonRow (Glib::ustring id)
      : m_profile_id (id)
      {
      }
};

class PlayerRow: public Glib::Object
{
public:
    Shield::Color m_shield;
    GameParameters::Player::Type m_type;

    static Glib::RefPtr<PlayerRow> create (Shield::Color shield)
      {
        return
          Glib::make_refptr_for_instance<PlayerRow>
          (new PlayerRow (shield));
      }

    sigc::signal<void()> signal_changed ()
      {
        return m_signal_changed;
      }

    void changed ()
      {
        m_signal_changed.emit ();
      }

    void update_type ()
      {
        auto p = Playerlist::instance ()->get (m_shield);
        if (!p)
          {
            m_type = GameParameters::Player::OFF;
            return;
          }

        switch (p->getType ())
          {
          case Player::HUMAN:
            m_type = GameParameters::Player::HUMAN;
            break;

          case Player::AI_FAST:
            m_type = GameParameters::Player::EASY;
            break;

          case Player::AI_SMART:
            m_type = GameParameters::Player::HARD;
            break;

          case Player::NETWORKED:
            m_type = GameParameters::Player::HUMAN;
            break;

          default:
            break;
          }
      }

protected:
    PlayerRow (Shield::Color shield)
      : m_shield (shield), m_type (GameParameters::Player::OFF)
      {
        update_type ();
      }

private:
    sigc::signal<void()> m_signal_changed;
};


class GameLobbyDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "game-lobby.ui";
      }

    GameLobbyDialog (BaseObjectType* o,
                     const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_play_button = load <Gtk::Button> ("play_button");
        m_cancel_button = load <Gtk::Button> ("cancel_button");
        m_chat_entry = load <Gtk::Entry> ("chat_entry");
        m_people_treeview = load <Gtk::ColumnView> ("people_treeview");
        m_player_treeview = load <Gtk::ColumnView> ("player_treeview");
        m_chat_textview = load <Gtk::TextView> ("chat_textview");
        m_chat_scrolledwindow =
          load <Gtk::ScrolledWindow> ("chat_scrolledwindow");
        m_map_drawing_area = load <Gtk::DrawingArea> ("map_drawing_area");
        m_show_options_button = load <Gtk::Button> ("show_options_button");
        m_take_control_button = load <Gtk::Button> ("take_control_button");
        m_release_button = load <Gtk::Button> ("release_button");
        m_control_all_button = load <Gtk::Button> ("control_all_button");

        m_citymap = NULL;
      }

    void setup (GameScenario *game_scenario, bool is_server)
      {
        m_is_server = is_server;
        m_game_scenario = game_scenario;

        m_play_button->set_sensitive (false);
        m_reported_as_ready = false;
        m_play_has_started = false;

        m_control_all_button->signal_clicked ().connect
          ([this] ()
           {
             for (guint i = 0; i < m_player_store->get_n_items (); ++i)
               {
                 m_player_selection_model->set_selected (i);
                 m_take_control_action->activate ();
               }
           });

        if (m_is_server)
          m_game_station = GameServer::instance ();
        else
          m_game_station = GameClient::instance ();

        m_profile_id = m_game_station->getProfileId ();

        auto actions = Gio::SimpleActionGroup::create();

        auto key_controller = Gtk::EventControllerKey::create ();
        key_controller->signal_key_released ().connect
          ([this] (guint keyval, guint, Gdk::ModifierType)
           {
             if (keyval == GDK_KEY_Return)
               {
                 if (m_is_server)
                   GameServer::instance ()->chat (m_chat_entry->get_text ());
                 else
                   GameClient::instance ()->chat (m_chat_entry->get_text ());
                 m_chat_entry->set_text("");
               }

             return;
           }, false);
        m_chat_entry->add_controller (key_controller);

        m_take_control_action = actions->add_action
          ("take-control",
           ([this] ()
            {
              auto row = get_selected_player ();
              if (row)
                {
                  sit_down (row->m_shield);
                  row->changed ();
                  update_buttons ();
                  m_game_station->hosted_player_sits (row->m_shield);
                }
            }));

        m_release_action = actions->add_action
          ("release",
           ([this] ()
            {
              auto row = get_selected_player ();
              if (row)
                {
                  stand_up (row->m_shield);
                  row->changed ();
                  update_buttons ();
                  m_game_station->hosted_player_stands (row->m_shield);
                }
            }));

        m_mute_action = actions->add_action
          ("mute",
           ([this] ()
            {
              auto row = get_selected_person ();
              if (row)
                Lobby::instance ()->toggle_mute (row->m_profile_id);
            }));

        m_unmute_action = actions->add_action
          ("unmute",
           ([this] ()
            {
              auto row = get_selected_person ();
              if (row)
                Lobby::instance ()->toggle_mute (row->m_profile_id);
            }));

        m_kick_action = actions->add_action
          ("kick",
           ([this] ()
            {
              auto row = get_selected_person ();
              if (row)
                {
                  if (m_is_server)
                    GameServer::instance ()->kick (row->m_profile_id);
                  else
                    GameClient::instance ()->kick (row->m_profile_id);
                }
            }));

        insert_action_group ("win", actions);

        m_take_control_button->set_action_name ("win.take-control");
        m_release_button->set_action_name ("win.release");

        set_response (m_cancel_button, Gtk::ResponseType::CANCEL);

        setup_player_treeview ();
        setup_people_treeview ();
        fill_chat ();
        update ();
      }

    void start_game ()
      {
        // we switch from being a modal dialog on splash window
        // to a non-modal dialog on game window
        //
        // 1. desensitize the type combos
        // 2. get rid of the play button
        // 3. and rename the cancel button to close
        m_play_has_started = true;
        m_cancel_button->set_label (_("Close"));
        m_play_button->set_visible (false);
      }
private:
    Gtk::Button *m_play_button;
    Gtk::Button *m_cancel_button;
    Gtk::Button *m_take_control_button;
    Gtk::Button *m_control_all_button;
    Gtk::Button *m_release_button;
    Gtk::Entry *m_chat_entry;
    Gtk::ColumnView *m_people_treeview;
    Gtk::ColumnView *m_player_treeview;
    Gtk::TextView *m_chat_textview;
    Gtk::ScrolledWindow *m_chat_scrolledwindow;
    Gtk::DrawingArea *m_map_drawing_area;
    Gtk::Button *m_show_options_button;
    Gtk::PopoverMenu *m_people_popover_menu;
    Gtk::PopoverMenu *m_player_popover_menu;

    Glib::RefPtr<Gtk::SingleSelection> m_player_selection_model;
    Glib::RefPtr<Gio::ListStore<PlayerRow>> m_player_store;

    Glib::RefPtr<Gtk::SingleSelection> m_people_selection_model;
    Glib::RefPtr<Gio::ListStore<ChatPersonRow>> m_people_store;

    GameScenario *m_game_scenario;
    bool m_is_server;
    Glib::ustring m_profile_id;
    CityMap *m_citymap;
    GameStation *m_game_station;
    bool m_play_has_started;
    bool m_reported_as_ready;
    Glib::RefPtr<Gio::SimpleAction> m_take_control_action;
    Glib::RefPtr<Gio::SimpleAction> m_release_action;
    Glib::RefPtr<Gio::SimpleAction> m_mute_action;
    Glib::RefPtr<Gio::SimpleAction> m_unmute_action;
    Glib::RefPtr<Gio::SimpleAction> m_kick_action;

    std::list<sigc::connection> m_connections;

    void add_connection (sigc::connection c)
      {
        m_connections.push_back (c);
      }

    void disconnect_signals ()
      {
        for (auto c : m_connections)
          c.disconnect ();
        m_connections.clear ();
      }

    void connect_signals ()
      {
        add_connection
          (m_show_options_button->signal_clicked ().connect
           ([this] ()
            {
              auto d = LwDialog::build<GameOptionsDialog>(this);
              d->setup (true);
              d->signal_response ().connect
                ([d] (Gtk::ResponseType)
                 {
                   delete d;
                 });
            }));

        add_connection
          (m_game_station->signal_remote_participant_joins ().connect
           ([this] (Glib::ustring, bool update)
            {
              if (update)
                fill_people_treeview ();
            }));

        add_connection
          (m_game_station->signal_moderator_assigned ().connect
           ([this] (Glib::ustring)
            {
              fill_people_treeview ();
              for (auto p : *Playerlist::instance ())
                player_row_changed (p->get_shield ());
            }));

        add_connection
          (m_game_station->signal_nickname_changed ().connect
           ([this] (Glib::ustring, Glib::ustring)
            {
              fill_people_treeview ();
              fill_chat ();
            }));

        add_connection
          (m_game_station->signal_remote_participant_departs ().connect
           ([this] (Glib::ustring /*profile_id*/)
            {
              fill_people_treeview ();
            }));

        add_connection
          (m_game_station->signal_chat_message_received ().connect
           ([this] (Glib::ustring profile_id, Glib::ustring text)
            {
              add_to_chat (profile_id,  text);
            }));

        add_connection
          (m_game_station->signal_system_message_received ().connect
           ([this] (Glib::ustring text)
            {
              add_to_chat ("",  text);
            }));

        add_connection
          (m_game_station->signal_player_sits ().connect
           ([this] (Shield::Color shield, Glib::ustring)
            {
              player_row_changed (shield);
            }));

        add_connection
          (m_game_station->signal_player_stands ().connect
           ([this] (Shield::Color shield, Glib::ustring)
            {
              player_row_changed (shield);
            }));

        add_connection
          (m_game_station->signal_player_changes_type ().connect
           ([this] (Player *p, int t)
            {
              for (guint i = 0; i < m_player_store->get_n_items (); i++)
                {
                  auto item = m_player_store->get_item (i);
                  auto row = std::dynamic_pointer_cast<PlayerRow>(item);
                  if (row->m_shield == p->get_shield ())
                    {
                      row->m_type = (enum GameParameters::Player::Type) t;
                      break;
                    }
                }
              player_row_changed (p->get_shield ());
            }));

        add_connection
          (m_game_station->signal_player_gets_turned_off ().connect
           ([this] (Player *p)
            {
              // this is for when a game begins and we want to remove the off
              // players from the list, bc we can't have a mod turning them
              // on mid-game.
              for (guint i = 0; i < m_player_store->get_n_items (); ++i)
                {
                  auto row = m_player_store->get_item (i);

                  if (row->m_shield == p->get_shield ())
                    {
                      m_player_store->remove(i);
                      break;
                    }
                }
            }));

        add_connection
          (m_play_button->signal_clicked ().connect
           ([this] ()
            {
               GameClient::instance ()->sendReady ();
               m_play_button->set_sensitive (false);
               m_reported_as_ready = true;
            }));

        add_connection
          (m_game_station->signal_nickname_changed ().connect
           ([this] (Glib::ustring /*profile_id*/, Glib::ustring /*nickname*/)
            {
              fill_people_treeview ();
            }));

        add_connection
          (m_game_station->signal_waiting_for_ready ().connect
           ([this] (int waiting_for_num_clients)
            {
              if (m_reported_as_ready)
                m_play_button->set_label
                  (String::ucompose
                   (ngettext ("Waiting for %1 player…",
                              "Waiting for %1 players…",
                              waiting_for_num_clients),
                    waiting_for_num_clients));
            }));

        add_connection
          (m_game_station->signal_game_can_begin ().connect
           ([this] ()
            {
              m_play_button->set_sensitive (true);
            }));

        add_connection
          (m_game_station->signal_local_player_starts_move ().connect
           ([this] (Player *)
            {
              update ();
            }));

        add_connection
          (m_game_station->signal_remote_player_starts_move ().connect
           ([this] (Player *)
            {
              update ();
            }));

        add_connection
          (m_game_station->signal_local_player_moved ().connect
           ([this] (Player *)
            {
              update ();
            }));

        add_connection
          (m_game_station->signal_remote_player_moved ().connect
           ([this] (Player *)
            {
              update ();
            }));
      }

    void update ()
      {
        disconnect_signals ();
        update_city_map ();
        fill_players_treeview ();
        fill_people_treeview ();

        update_buttons ();
        connect_signals ();
      }

    void setup_player_treeview ()
      {
        m_player_store = Gio::ListStore<PlayerRow>::create ();
        m_player_selection_model =
          Gtk::SingleSelection::create (m_player_store);
        m_player_treeview->set_model (m_player_selection_model);
        m_player_selection_model->signal_selection_changed ().connect
          ([this] (const guint &, const guint &)
           {
             update_buttons ();
           });
        setup_shield_column ();
        setup_name_column ();
        setup_type_column ();
        setup_player_context_menu ();
      }

    void setup_shield_column ()
      {
        auto n = Playerlist::getNeutral ();
        auto pi = ImageCache::instance ()->getShieldPic (1, n, false);
        auto w = pi->get_width ();
        auto h = pi->get_height ();

        auto factory = Gtk::SignalListItemFactory::create ();

        factory->signal_setup ().connect
          ([w, h] (const Glib::RefPtr<Gtk::ListItem>& item)
           {
             auto picture = Gtk::make_managed<Gtk::Picture> ();
             picture->set_valign (Gtk::Align::CENTER);
             picture->set_can_shrink (false);
             picture->set_size_request (w, h);
             picture->set_content_fit (Gtk::ContentFit::SCALE_DOWN);
             item->set_child (*picture);
             item->set_data ("shield", picture);
           });

        factory->signal_bind ().connect
          ([](const Glib::RefPtr<Gtk::ListItem>& item)
           {
             auto row = std::dynamic_pointer_cast<PlayerRow>(item->get_item ());
             auto picture =
               static_cast<Gtk::Picture*>(item->get_data ("shield"));

             if (row && picture)
               {
                 auto p = Playerlist::instance ()->get (row->m_shield);
                 auto pp = ImageCache::instance ()->getShieldPic (1, p, false);
                 if (Playerlist::getActiveplayer () == p)
                   {
                     auto padding = 8;
                     auto size =
                       std::max (pp->get_width (), pp->get_height ()) + padding;
                     if (size % 2 == 1)
                       size++;
                     auto box = PixMask::create (size);
                     auto cr = box->get_gc ();
                     cr->set_source_rgba (0, 0, 0, 1);
                     cr->rectangle (0, 0, size, size);
                     cr->fill ();

                     pp->blit_centered (box->get_pixmap (),
                                        Vector<int>(size / 2, size / 2));
                     picture->set_paintable (box->to_texture ());
                     delete box;
                   }
                 else
                   picture->set_paintable (pp->to_texture ());
               }
           });

        auto column = Gtk::ColumnViewColumn::create ("", factory);

        m_player_treeview->append_column (column);
      }

    void setup_name_column ()
      {
        auto quark = "name_label";
        auto factory = Gtk::SignalListItemFactory::create ();

        factory->signal_setup ().connect
          ([this, quark] (const Glib::RefPtr<Gtk::ListItem>& item)
           {
             auto label = Gtk::make_managed<Gtk::Label>();
             label->set_halign (Gtk::Align::START);
             label->set_ellipsize (Pango::EllipsizeMode::END);
             label->set_justify (Gtk::Justification::LEFT);
             item->set_child (*label);
             item->set_data (quark, label);
           });

        factory->signal_bind () .connect
          ([this, quark]
           (const Glib::RefPtr<Gtk::ListItem>& item)
           {
             auto row =
               std::dynamic_pointer_cast<PlayerRow>(item->get_item ());
             auto label = static_cast<Gtk::Label*>(item->get_data (quark));

             if (label && row)
               {
                 row->signal_changed ().connect
                   ([label, row] ()
                    {
                      auto p = Playerlist::instance ()->get (row->m_shield);
                      auto profile_id =
                        Lobby::instance ()->get_profile_id (row->m_shield);
                      Glib::ustring text = "";
                      if (profile_id != "")
                        {
                          Glib::ustring nick =
                            Lobby::instance ()->get_name (profile_id);
                          text = String::ucompose ("%1 (%2)", p->getName (), nick);
                        }
                      else
                        text = String::ucompose ("%1 [%2]", p->getName (),
                                                 _("Available"));
                      label->set_text (text);
                    });
                 row->changed ();
               }
           });

        auto column = Gtk::ColumnViewColumn::create (_("Name"), factory);
        column->set_expand (true);
        m_player_treeview->append_column (column);
      }
                 
    void setup_people_treeview ()
      {
        m_people_store = Gio::ListStore<ChatPersonRow>::create ();
        m_people_selection_model =
          Gtk::SingleSelection::create (m_people_store);
        m_people_treeview->set_model (m_people_selection_model);
        m_people_selection_model->signal_selection_changed ().connect
          ([this] (const guint &, const guint &)
           {
             update_buttons ();
           });

        auto quark = "pname_label";
        auto factory = Gtk::SignalListItemFactory::create ();

        factory->signal_setup ().connect
          ([this, quark] (const Glib::RefPtr<Gtk::ListItem>& item)
           {
             auto label = Gtk::make_managed<Gtk::Label>();
             label->set_halign (Gtk::Align::START);
             label->set_ellipsize (Pango::EllipsizeMode::END);
             label->set_justify (Gtk::Justification::LEFT);
             item->set_child (*label);

             item->set_data (quark, label);

           });

        factory->signal_bind () .connect
          ([this, quark]
           (const Glib::RefPtr<Gtk::ListItem>& item)
           {
             auto row =
               std::dynamic_pointer_cast<ChatPersonRow>(item->get_item ());
             auto label = static_cast<Gtk::Label*>(item->get_data (quark));

             if (label && row)
               {
                 auto user = Lobby::instance ()->get_name (row->m_profile_id);
                 if (Lobby::instance ()->user_is_mod (row->m_profile_id))
                   label->set_text (String::ucompose ("[%1]", user));
                 else
                   label->set_text (user);
                 label->set_tooltip_text (user);

                 auto click = Gtk::GestureClick::create();
                 click->set_button(GDK_BUTTON_SECONDARY);
                 click->signal_pressed ().connect
                   ([this, item, label](int, double x, double y)
                    {

                      auto pos = item->get_position ();
                      if (pos == GTK_INVALID_LIST_POSITION)
                        return;

                      if (!m_people_selection_model->is_selected (pos))
                        {
                          m_people_selection_model->unselect_all ();
                          m_people_selection_model->select_item (pos, false);
                        }

                      double nx, ny;
                      label->translate_coordinates (*m_people_treeview, x, y,
                                                    nx, ny);

                      Gdk::Rectangle rect (nx, ny, 1, 1);
                      m_people_popover_menu->set_pointing_to (rect);
                      m_people_popover_menu->popup ();
                    });

                 label->add_controller (click);
               }
           });

        auto column = Gtk::ColumnViewColumn::create (_("People"), factory);
        column->set_expand (true);
        m_people_treeview->append_column (column);

        setup_chat_person_context_menu ();
      }

    void update_city_map ()
      {
        if (m_game_scenario->s_hidden_map == false)
          {
            if (m_citymap)
              delete m_citymap;
            m_citymap = new CityMap ();
            m_citymap->map_changed.connect
              ([this] (Cairo::RefPtr<Cairo::Surface> map)
               {
                 Vector<int> dim = calculate_size_of_map ();
                 auto pp = PixMask::create (map);
                 auto p = pp->scale (dim.x, dim.y);
                 cairo_surface_to_drawing_area (p->get_pixmap (),
                                                m_map_drawing_area);
                 delete pp;
                 delete p;
               });
            // we can't show the map prior to selecting players because the
            // capital cities are already chosen and having first pick is too
            // much of an advantage.
            if (m_game_scenario->getRound () > 1)
              {
                m_citymap->resize ();
                m_citymap->draw ();
              }
            else
              {
                Vector<int> dim = calculate_size_of_map ();
                auto surface = Cairo::ImageSurface::create
                  (Cairo::Surface::Format::ARGB32, dim.x, dim.y);
                auto cr = Cairo::Context::create (surface);
                cr->set_source_rgb (0.0, 0.0, 0.0);
                cr->rectangle (0, 0, dim.x, dim.y);
                cr->fill ();

                cr->set_source_rgb (1, 1, 1);
                cr->select_font_face ("Sans",
                                      Cairo::ToyFontFace::Slant::ITALIC,
                                      Cairo::ToyFontFace::Weight::NORMAL);
                cr->set_font_size (11);

                const std::string text = "(Map hidden until game begins.)";

                Cairo::TextExtents extents;
                cr->get_text_extents (text, extents);

                double x = (dim.x - extents.width) / 2.0 - extents.x_bearing;
                double y = (dim.y - extents.height) / 2.0 - extents.y_bearing;
                cr->move_to (x, y);
                cr->show_text (text);

                cairo_surface_to_drawing_area (surface, m_map_drawing_area);
              }
          }
        else
          {
            static bool set_draw  = false;
            if (!set_draw)
              {
                m_map_drawing_area->set_draw_func
                  ([] (const Cairo::RefPtr<Cairo::Context>& cr, int, int)
                   {
                     Vector<int> dim = OverviewMap::calculate_smallmap_size ();
                     cr->set_source_rgb (0.0, 0.0, 0.0);
                     cr->rectangle (0, 0, dim.x, dim.y);
                     cr->fill ();
                   });
              }
            set_draw = true;
          }
        m_map_drawing_area->queue_draw ();
      }

    void setup_type_column ()
      {
        auto factory = Gtk::SignalListItemFactory::create ();

        factory->signal_setup ().connect
          ([] (const Glib::RefPtr<Gtk::ListItem>& item) mutable
           {
             auto combo = Gtk::make_managed<LwCombo> ();
             combo->append (HUMAN_PLAYER_TYPE);
             combo->append (EASY_PLAYER_TYPE);
             combo->append (HARD_PLAYER_TYPE);
             combo->append (NO_PLAYER_TYPE);
             combo->set_valign (Gtk::Align::CENTER);
             item->set_child (*combo);
             item->set_data ("typecombo", combo);

           });

        factory->signal_bind ().connect
          ([this](const Glib::RefPtr<Gtk::ListItem>& item) mutable
           {
             auto row = std::dynamic_pointer_cast<PlayerRow>(item->get_item ());
             auto combo = static_cast<LwCombo*>(item->get_data ("typecombo"));

             if (row && combo)
               {
                 auto p = Playerlist::instance ()->get (row->m_shield);
                 auto conn = combo->signal_changed ().connect
                   ([this, p, row, combo] () mutable
                    {
                      auto t = GameParameters::Player::OFF;
                      switch (combo->get_active_row_number ())
                        {
                        case 0:
                          t = GameParameters::Player::HUMAN;
                          break;

                        case 1:
                          t = GameParameters::Player::EASY;
                          break;

                        case 2:
                          t = GameParameters::Player::HARD;
                          break;

                        case 3:
                          t = GameParameters::Player::OFF;
                          break;
                        }
                      row->m_type = t;
                      // our game server and client keep a list of players so we
                      // have to update it through one or the other.
                      if (m_is_server)
                        GameServer::instance ()->type_change (p, (int)row->m_type);
                      else
                        GameClient::instance ()->change_type (p, (int)row->m_type);
                    });

                 row->signal_changed ().connect
                   ([this, p, combo, conn, row] () mutable
                    {
                      conn.block ();
                      combo->set_active (row->m_type);
                      conn.unblock ();

                      bool active = 
                        Lobby::instance ()->user_is_mod (m_profile_id) &&
                        !m_play_has_started;

                      combo->set_sensitive (active);
                    });
                 row->changed ();
               }
           });

        auto column = Gtk::ColumnViewColumn::create (_("Type"), factory);

        m_player_treeview->append_column (column);
      }

    void update_buttons ()
      {
        auto row = get_selected_player ();
        if (!row)
          {
            m_take_control_action->set_enabled (false);
            m_release_action->set_enabled (false);
            return;
          }
        else if (row->m_type == GameParameters::Player::OFF)
          {
            m_take_control_action->set_enabled (false);
            m_release_action->set_enabled (false);
            return;
          }
        Shield::Color shield = get_selected_player ()->m_shield;
        auto profile_id = Lobby::instance ()->get_profile_id (shield);
        m_take_control_action->set_enabled (profile_id == "");
        m_release_action->set_enabled (profile_id == m_profile_id);

        auto prow = get_selected_person ();
        if (!prow)
          {
            m_mute_action->set_enabled (false);
            m_unmute_action->set_enabled (false);
            m_kick_action->set_enabled (false);
          }
        else
          {
            m_mute_action->set_enabled
              (Lobby::instance ()->can_mute (prow->m_profile_id));
            m_unmute_action->set_enabled 
              (Lobby::instance ()->user_is_muted (prow->m_profile_id));
            m_kick_action->set_enabled 
              (!Lobby::instance ()->user_is_mod (prow->m_profile_id));
          }
      }

    std::shared_ptr<PlayerRow> get_selected_player ()
      {
        auto single_selection =
          std::dynamic_pointer_cast<Gtk::SingleSelection>
          (m_player_selection_model);

        auto item = single_selection->get_selected_item ();

        if (item)
          {
            auto row = std::dynamic_pointer_cast<PlayerRow> (item);
            return row;
          }
        return NULL;
      }

    void fill_players_treeview ()
      {
        m_player_store->remove_all ();
        for (auto p : *Playerlist::instance ())
          {
            if (p == Playerlist::getNeutral ())
              continue;
            m_player_store->append (PlayerRow::create (p->get_shield ()));
          }
      }

    void fill_people_treeview ()
      {
        m_people_store->remove_all ();
        auto roster = Lobby::instance ()->get_roster ();
        for (auto p : roster)
          m_people_store->append (ChatPersonRow::create (p));
      }

    void stand_up (Shield::Color shield)
      {
        auto id = Lobby::instance ()->get_profile_id (shield);
        if (m_is_server)
          GameServer::instance ()->stand_up (shield, id);
        else
          GameClient::instance ()->stand_up (shield);
      }

    void sit_down (Shield::Color shield)
      {
        auto id = Lobby::instance ()->get_profile_id (shield);
        if (m_is_server)
          GameServer::instance ()->sit_down (shield, id);
        else
          GameClient::instance ()->sit_down (shield);

      }

    void fill_chat ()
      {
        m_chat_textview->get_buffer ()->set_text ("");
        for (auto row : Lobby::instance ()->get_chat_log ())
          add_to_chat (row.get_profile_id (), row.get_message ());
      }

    void add_to_chat (Glib::ustring id, Glib::ustring line)
      {
        if (id == "")
          {
            auto text = m_chat_textview->get_buffer ()->get_text ();
            m_chat_textview->get_buffer ()->set_text
              (text + line + "\n");
          }
        else
          {
            auto nickname = Lobby::instance ()->get_name (id);
            auto text = m_chat_textview->get_buffer ()->get_text ();
            m_chat_textview->get_buffer ()->set_text
              (text + nickname + ": " + line + "\n");
          }
        scroll_chat_to_bottom ();
      }

    Vector<int> calculate_size_of_map ()
      {
        double max_height = 260;
        Vector<int> dim = OverviewMap::calculate_smallmap_size ();
        double ch = max_height / dim.y;
        dim.y = max_height;
        dim.x = dim.x * ch;
        return dim;
      }

    std::shared_ptr<ChatPersonRow> get_selected_person ()
      {
        auto single_selection =
          std::dynamic_pointer_cast<Gtk::SingleSelection>
          (m_people_selection_model);

        auto item = single_selection->get_selected_item ();

        if (item)
          {
            auto row = std::dynamic_pointer_cast<ChatPersonRow> (item);
            return row;
          }
        return NULL;
      }

    void scroll_chat_to_bottom ()
      {
        Glib::signal_idle ().connect_once
          ([this] ()
           {
             auto vadj = m_chat_scrolledwindow->get_vadjustment ();
             if (vadj)
               vadj->set_value (vadj->get_upper () - vadj->get_page_size ());
           });

       }

    void setup_chat_person_context_menu ()
      {
        auto menu = Gio::Menu::create();
        menu->append (_("Mute"), "win.mute");
        menu->append (_("Unmute"), "win.unmute");
        menu->append (_("Kick"), "win.kick");
        m_people_popover_menu = Gtk::make_managed<Gtk::PopoverMenu> ();
        m_people_popover_menu->set_menu_model (menu);
        m_people_popover_menu->set_has_arrow (false);
        m_people_popover_menu->set_parent (*m_people_treeview);
      }

    void setup_player_context_menu ()
      {
        auto menu = Gio::Menu::create();
        menu->append (_("Take Control"), "win.take-control");
        menu->append (_("Release"), "win.release");
        m_player_popover_menu = Gtk::make_managed<Gtk::PopoverMenu> ();
        m_player_popover_menu->set_menu_model (menu);
        m_player_popover_menu->set_has_arrow (false);
        m_player_popover_menu->set_parent (*m_player_treeview);
      }

    void player_row_changed (Shield::Color shield)
      {
        for (guint i = 0; i < m_player_store->get_n_items (); i++)
          {
            auto item = m_player_store->get_item (i);
            auto row = std::dynamic_pointer_cast<PlayerRow>(item);
            if (row->m_shield == shield)
              {
                row->changed ();
                break;
              }
          }
        update_buttons ();
      }
};
#endif
