//  Copyright (C) 2026 Ben Asselstine
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
#ifndef NEW_CONNECTION_DIALOG_H
#define NEW_CONNECTION_DIALOG_H
#include "lw-dialog-base.h"
class NewConnectionDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "new-connection.ui";
      }

    NewConnectionDialog (BaseObjectType* o,
                         const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_connect_button = load <Gtk::Button> ("connect_button");
        m_cancel_button = load <Gtk::Button> ("cancel_button");
        m_host_entry = load <Gtk::Entry> ("hostname_entry");
        m_port_spinbutton = load <Gtk::SpinButton> ("port_spinbutton");
      }

    void setup ()
      {
        set_response (m_connect_button, Gtk::ResponseType::ACCEPT);
        set_response (m_cancel_button, Gtk::ResponseType::CANCEL);

        m_port_spinbutton->set_value (LORDSAWAR_PORT);

        m_host_entry->signal_changed ().connect
          ([this] ()
           {
             if (String::utrim (m_host_entry->get_text ()) != "")
               m_connect_button->set_sensitive (true);
           });

        m_connect_button->set_sensitive (false);

        signal_response ().connect
          ([this](Gtk::ResponseType resp)
           {
             hide ();
             switch (resp)
               {
               case Gtk::ResponseType::ACCEPT: //connect
                 m_game_selected.emit
                   (String::utrim (m_host_entry->get_text ()),
                    m_port_spinbutton->get_value_as_int ());
                 break;
               default:
                 break;
               }
           });
        m_host_entry->set_text ("localhost"); //removeme XXX XXX XXX
      }

    sigc::signal<void (Glib::ustring, unsigned short)> signal_game_selected ()
      {
        return m_game_selected;
      }

private:
    Gtk::Entry *m_host_entry;
    Gtk::SpinButton *m_port_spinbutton;
    Gtk::Button *m_cancel_button;
    Gtk::Button *m_connect_button;
    sigc::signal<void (Glib::ustring /*ip*/, unsigned short /*port*/)> m_game_selected;
};
#endif
