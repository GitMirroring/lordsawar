//  Copyright (C) 2011, 2014, 2015 Ben Asselstine
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
#include "new-network-game-dialog.h"
#include "Configuration.h"
#include "defs.h"
#include "ucompose.hpp"
#include "profile.h"
#include "profilelist.h"
#include "new-profile-dialog.h"

#define method(x) sigc::mem_fun(*this, &NewNetworkGameDialog::x)

NewNetworkGameDialog::NewNetworkGameDialog(Gtk::Window &parent, bool force_server)
 : LwDialog(parent, "new-network-game-dialog.ui")
{
  xml->get_widget("client_radiobutton", client_radiobutton);
  xml->get_widget("server_radiobutton", server_radiobutton);
  client_radiobutton->signal_toggled().connect(method(on_client_radiobutton_toggled));
  xml->get_widget("accept_button", accept_button);
  xml->get_widget("add_button", add_button);
  add_button->signal_clicked().connect(method(on_add_button_clicked));
  xml->get_widget("remove_button", remove_button);
  remove_button->signal_clicked().connect(method(on_remove_button_clicked));
  xml->get_widget("profiles_treeview", profiles_treeview);
  profiles_list = Gtk::ListStore::create(profiles_columns);
  profiles_treeview->set_model(profiles_list);
  profiles_treeview->append_column("", profiles_columns.nickname);
  profiles_treeview->signal_row_activated().connect(sigc::hide(sigc::hide(method(on_profile_activated))));

  if (Profilelist::getInstance()->empty() == true)
    Profilelist::getInstance()->push_back(new Profile(Glib::get_user_name()));

  for (Profilelist::iterator i = Profilelist::getInstance()->begin();
       i != Profilelist::getInstance()->end(); ++i)
    add_profile(*i);

  xml->get_widget("advertise_checkbutton", advertise_checkbutton);
  xml->get_widget("remote_checkbutton", remote_checkbutton);
  remote_checkbutton->signal_toggled().connect
    (method(on_remote_checkbutton_toggled));

  if (Configuration::s_gamelist_server_hostname == "" ||
      Configuration::s_gamelist_server_port == 0)
    advertise_checkbutton->set_label (_("List the game on a remote server."));
  else
    advertise_checkbutton->set_label
    (String::ucompose(_("List the game on %1."),
                      Configuration::s_gamelist_server_hostname));
  if (Configuration::s_gamehost_server_hostname == "" ||
      Configuration::s_gamehost_server_port == 0)
    remote_checkbutton->set_label
      (_("Host and list the game on a remote server."));
  else
    remote_checkbutton->set_label
      (String::ucompose(_("Host and list the game on %1."),
                        Configuration::s_gamehost_server_hostname));
  select_preferred_profile(Glib::get_user_name());
  update_buttons();
  profiles_treeview->get_selection()->signal_changed().connect
    (method(on_profile_selected));
  if (force_server)
    {
      server_radiobutton->set_active(true);
      server_radiobutton->set_sensitive(false);
      client_radiobutton->set_sensitive(false);
    }
}

void NewNetworkGameDialog::select_preferred_profile(Glib::ustring user)
{
  Profile *p = Profilelist::getInstance()->findLastPlayedProfileForUser(user);
  Gtk::TreeModel::Children kids = profiles_list->children();
  for (Gtk::TreeModel::Children::iterator i = kids.begin(); 
       i != kids.end(); ++i)
    {
      Gtk::TreeModel::Row row = *i;
      if (row[profiles_columns.profile] == p)
        {
          profiles_treeview->get_selection()->select(row);
          return;
        }
    }

  Gtk::TreeModel::Row row = profiles_treeview->get_model()->children()[0];
  if(row)
    profiles_treeview->get_selection()->select(row);
}

void NewNetworkGameDialog::add_profile(Profile *profile)
{
  Gtk::TreeIter i = profiles_list->append();
  (*i)[profiles_columns.nickname] = profile->getNickname();
  (*i)[profiles_columns.profile] = profile;
}
	    
void NewNetworkGameDialog::update_buttons()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = 
    profiles_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();
  if (iterrow) 
    accept_button->set_sensitive(true);
  else
    accept_button->set_sensitive(false);

  if (Configuration::s_gamelist_server_hostname != "" &&
      Configuration::s_gamelist_server_port != 0)
    advertise_checkbutton->set_sensitive(!client_radiobutton->get_active());
  else
    advertise_checkbutton->set_sensitive(false);

  if (Configuration::s_gamehost_server_hostname != "" &&
      Configuration::s_gamehost_server_port != 0)
    remote_checkbutton->set_sensitive(!client_radiobutton->get_active());
  else
    remote_checkbutton->set_sensitive(false);

  if (remote_checkbutton->get_active() && remote_checkbutton->property_sensitive())
    {
      advertise_checkbutton->set_sensitive(true);
      advertise_checkbutton->set_active(false);
      advertise_checkbutton->set_sensitive(false);
    }
}

bool NewNetworkGameDialog::run()
{
  int response = dialog->run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      Profilelist::getInstance()->save();
      Glib::RefPtr<Gtk::TreeSelection> selection = 
        profiles_treeview->get_selection();
      Gtk::TreeModel::iterator iterrow = selection->get_selected();
      if (iterrow) 
        {
          Gtk::TreeModel::Row row = *iterrow;
          d_profile = row[profiles_columns.profile];
          d_profile->play();
          Profilelist::getInstance()->save();
        }
      return true;
    }
  return false;
}

void NewNetworkGameDialog::on_add_button_clicked()
{
  NewProfileDialog d(*dialog);
  if (d.run_and_hide() == Gtk::RESPONSE_ACCEPT)
    {
      Profile *profile = new Profile (d.getNickname());
      Profilelist::getInstance()->push_back(profile);
      add_profile(profile);
      Gtk::TreeModel::Row row;
      int n = profiles_treeview->get_model()->children().size();
      row = profiles_treeview->get_model()->children()[n-1];
      if(row)
	profiles_treeview->get_selection()->select(row);
    }
  update_buttons();
}

void NewNetworkGameDialog::on_remove_button_clicked()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = 
    profiles_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();
  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Profile *profile = row[profiles_columns.profile];
      if (profile)
        Profilelist::getInstance()->remove(profile);
      profiles_list->erase(iterrow);
    }
  update_buttons();
}

void NewNetworkGameDialog::on_profile_selected()
{
  update_buttons();
}
  
void NewNetworkGameDialog::on_client_radiobutton_toggled()
{
  update_buttons();
}
    
void NewNetworkGameDialog::on_remote_checkbutton_toggled()
{
  update_buttons();
}

void NewNetworkGameDialog::on_profile_activated()
{
  accept_button->activate();
}
