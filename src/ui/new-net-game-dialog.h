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

#pragma once
#ifndef NEW_NETWORK_GAME_DIALOG_H
#define NEW_NETWORK_GAME_DIALOG_H

#include <gtkmm.h>

#include "ucompose.hpp"
#include "lw-dialog.h"
class Profile;
// dialog for choosing between a hosted game and a game we connect to
class NewNetworkGameDialog: public LwDialog
{
 public:
    NewNetworkGameDialog(Gtk::Window &parent, bool force_server);
    ~NewNetworkGameDialog() {};

    Profile* getProfile() const {return d_profile;};
    bool isClient() {return client_radiobutton->get_active();};
    bool isAdvertised() {return advertise_checkbutton->get_active();};
    bool isRemotelyHosted() {return remote_checkbutton->get_active();};
    void hide() {dialog->hide();};
    bool run();
    
 private:
  Gtk::RadioButton *client_radiobutton;
  Gtk::RadioButton *server_radiobutton;
  Gtk::Button *accept_button;
  Gtk::Button *add_button;
  Gtk::Button *remove_button;
  Gtk::CheckButton *advertise_checkbutton;
  Gtk::CheckButton *remote_checkbutton;
  Gtk::TreeView *profiles_treeview;
    class ProfilesColumns: public Gtk::TreeModelColumnRecord {
    public:
	ProfilesColumns() 
        { add(nickname); add(profile);}
	
	Gtk::TreeModelColumn<Glib::ustring> nickname;
	Gtk::TreeModelColumn<Profile*> profile;
    };
    const ProfilesColumns profiles_columns;
    Glib::RefPtr<Gtk::ListStore> profiles_list;
    Profile *d_profile;

  void update_buttons();
  void add_profile(Profile *profile);
  void on_remove_button_clicked();
  void on_add_button_clicked();
  void select_preferred_profile(Glib::ustring user);
  void on_profile_selected();
  void on_client_radiobutton_toggled();
  void on_remote_checkbutton_toggled();
  void on_profile_activated();
};

#endif
