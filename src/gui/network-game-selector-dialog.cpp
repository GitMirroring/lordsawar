//  Copyright (C) 2008, 2009, 2011, 2014, 2015 Ben Asselstine
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#include <config.h>

#include <gtkmm.h>
#include <sigc++/functors/mem_fun.h>

#include "network-game-selector-dialog.h"

#include "recently-played-game-list.h"
#include "recently-played-game.h"
#include "Configuration.h"
#include "profile.h"
#include "gamelist-client.h"
#include "ucompose.hpp"

#define method(x) sigc::mem_fun(*this, &NetworkGameSelectorDialog::x)

NetworkGameSelectorDialog::NetworkGameSelectorDialog(Gtk::Window &parent, Profile *p)
 : LwDialog(parent, "pick-network-game-to-join-dialog.ui")
{
    profile = p;
    xml->get_widget("hostname_entry", hostname_entry);
    xml->get_widget("port_spinbutton", port_spinbutton);
    hostname_entry->set_activates_default(true);
    hostname_entry->signal_changed().connect (method(on_hostname_changed));
    xml->get_widget("clear_button", clear_button);
    clear_button->signal_clicked().connect (method(on_clear_clicked));
    xml->get_widget("refresh_button", refresh_button);
    refresh_button->signal_clicked().connect (method(on_refresh_clicked));
    xml->get_widget("connect_button", connect_button);
    connect_button->set_sensitive(false);
    recently_joined_games_list = 
      Gtk::ListStore::create(recently_joined_games_columns);
    xml->get_widget("recent_treeview", recent_treeview);
    recent_treeview->set_model(recently_joined_games_list);
    recent_treeview->append_column(_("Name"), recently_joined_games_columns.name);
    recent_treeview->append_column(_("Turn"), recently_joined_games_columns.turn);
    recent_treeview->append_column(_("Players"), recently_joined_games_columns.number_of_players);
    recent_treeview->append_column(_("Cities"), recently_joined_games_columns.number_of_cities);
    recent_treeview->append_column(_("Host"), recently_joined_games_columns.host);
    recent_treeview->append_column(_("Port"), recently_joined_games_columns.port);
    recent_treeview->set_headers_visible(true);
    recent_treeview->get_selection()->signal_changed().connect
          (method(on_recent_game_selected));
    recent_treeview->signal_row_activated().connect(sigc::hide(sigc::hide(method(on_recent_game_activated))));
    
    RecentlyPlayedGameList *rpgl = RecentlyPlayedGameList::getInstance();
    rpgl->pruneGames();
    fill_games(rpgl, recently_joined_games_list, recently_joined_games_columns,
               p);

    xml->get_widget("games_treeview", games_treeview);
    xml->get_widget("notebook1", notebook);
    games_list = Gtk::ListStore::create(games_columns);
    games_treeview->set_model(games_list);
    games_treeview->append_column(_("Name"), games_columns.name);
    games_treeview->append_column(_("Turn"), games_columns.turn);
    games_treeview->append_column(_("Players"), games_columns.number_of_players);
    games_treeview->append_column(_("Cities"), games_columns.number_of_cities);
    games_treeview->append_column(_("Host"), games_columns.host);
    games_treeview->append_column(_("Port"), games_columns.port);
    games_treeview->set_headers_visible(true);
    games_treeview->get_selection()->signal_changed().connect
          (method(on_game_selected));
  
    port_spinbutton->set_value(LORDSAWAR_PORT);

    select_first_game();
 
  if (Configuration::s_gamelist_server_hostname != "" &&
      Configuration::s_gamelist_server_port != 0)
    {
      GamelistClient::getInstance()->client_connected.connect
        (method(on_connected_to_gamelist_server));
      GamelistClient::getInstance()->client_could_not_connect.connect
        (method(on_could_not_connect_to_gamelist_server));

      GamelistClient::getInstance()->received_game_list.connect
        (method(on_game_list_received));

      GamelistClient::getInstance()->start 
        (Configuration::s_gamelist_server_hostname, 
         Configuration::s_gamelist_server_port, profile);
    }
    games_treeview->signal_row_activated().connect(sigc::hide(sigc::hide(method(on_hosted_game_activated))));
  notebook->set_current_page(1);
}

void NetworkGameSelectorDialog::on_could_not_connect_to_gamelist_server()
{
  ;
}

void NetworkGameSelectorDialog::on_connected_to_gamelist_server()
{
  refresh_button->set_sensitive(false);
  GamelistClient::getInstance()->request_game_list();
}

void NetworkGameSelectorDialog::on_game_list_received(RecentlyPlayedGameList *rpgl, Glib::ustring err)
{
  if (err == "")
    {
      fill_games(rpgl, games_list, games_columns, NULL);
      select_first_game();
    }

  delete rpgl;
  refresh_button->set_sensitive(true);
}

void NetworkGameSelectorDialog::select_first_game()
{
  if (games_treeview->get_model()->children().size() > 0)
    {
      Gtk::TreeModel::Row row;
      row = games_treeview->get_model()->children()[0];
      if (row)
        games_treeview->get_selection()->select(row);
    }
  else if (recent_treeview->get_model()->children().size() > 0)
    {
      Gtk::TreeModel::Row row;
      row = recent_treeview->get_model()->children()[0];
      if (row)
        recent_treeview->get_selection()->select(row);
    }
}

void NetworkGameSelectorDialog::fill_games(RecentlyPlayedGameList *rpgl, Glib::RefPtr<Gtk::ListStore> list, const GamesColumns &columns, Profile *p)
{
  for (RecentlyPlayedGameList::iterator it = rpgl->begin(); it != rpgl->end();
       ++it)
    {
      if ((*it)->getPlayMode() == GameScenario::NETWORKED)
        {
          RecentlyPlayedNetworkedGame *game;
          game = dynamic_cast<RecentlyPlayedNetworkedGame*>(*it);
          if (p)
            {
              if (game->getProfileId() == p->getId())
                {
                  addGame(list, columns, game);
                }
            }
          else
            addGame(list, columns, game);
        }
    }
}

NetworkGameSelectorDialog::~NetworkGameSelectorDialog()
{
  GamelistClient::getInstance()->disconnect();
  //GamelistClient::deleteInstance();
  notebook->property_show_tabs () = false;
}

void NetworkGameSelectorDialog::addGame(Glib::RefPtr<Gtk::ListStore> list, const GamesColumns &columns, RecentlyPlayedNetworkedGame*g)
{
    Gtk::TreeIter i = list->append();
    (*i)[columns.name] = g->getName();
    (*i)[columns.turn] = g->getRound();
    (*i)[columns.number_of_players] = g->getNumberOfPlayers();
    (*i)[columns.number_of_cities] = g->getNumberOfCities();
    (*i)[columns.host] = g->getHost();
    (*i)[columns.port] = g->getPort();
}


void NetworkGameSelectorDialog::on_hostname_changed()
{
  update_buttons();
}

bool NetworkGameSelectorDialog::run()
{
  int response = dialog->run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      hide();
      guint32 port = guint32(port_spinbutton->get_value());
      game_selected.emit(hostname_entry->get_text(), port);
      return true;
    }
  else
    return false;
}
          
void NetworkGameSelectorDialog::on_recent_game_selected()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = recent_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();
  Gtk::TreeModel::Row row = *iterrow;
  hostname_entry->set_text(row[recently_joined_games_columns.host]);
  port_spinbutton->set_value(row[recently_joined_games_columns.port]);
}

void NetworkGameSelectorDialog::on_game_selected()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = games_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();
  Gtk::TreeModel::Row row = *iterrow;
  hostname_entry->set_text(row[games_columns.host]);
  port_spinbutton->set_value(row[games_columns.port]);
}
      
void NetworkGameSelectorDialog::on_clear_clicked()
{
  RecentlyPlayedGameList *rpgl = RecentlyPlayedGameList::getInstance();
  rpgl->removeAllNetworkedGames();
  rpgl->save();
  recently_joined_games_list->clear();
  recently_joined_games_list.reset();
  recently_joined_games_list = 
    Gtk::ListStore::create(recently_joined_games_columns);
  recent_treeview->set_model(recently_joined_games_list);
  hostname_entry->set_text("");
  port_spinbutton->set_value(LORDSAWAR_PORT);
  update_buttons();
}

void NetworkGameSelectorDialog::update_buttons()
{
  if (recently_joined_games_list->children().size() == 0)
    clear_button->set_sensitive(false);
  else
    clear_button->set_sensitive(true);

  //validate the ip/hostname
  if (String::utrim(hostname_entry->get_text()).length() > 0)
    {
      //connect_button->grab_focus();
      connect_button->set_sensitive(true);
      connect_button->property_can_focus() = true;
      connect_button->property_can_default() = true;
      connect_button->property_has_default() = true;
      hostname_entry->property_activates_default() = true;
      connect_button->property_receives_default() = true;
    }
  else
    connect_button->set_sensitive(false);
}

void NetworkGameSelectorDialog::on_refresh_clicked()
{
  games_list->clear();
  on_connected_to_gamelist_server();
}

void NetworkGameSelectorDialog::on_recent_game_activated()
{
  connect_button->activate();
}

void NetworkGameSelectorDialog::on_hosted_game_activated()
{
  connect_button->activate();
}
