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
#include "lw-dialog-base.h"
#ifndef TREACHERY_DIALOG_H
#define TREACHERY_DIALOG_H
class TreacheryDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "treachery.ui";
      }

    TreacheryDialog (BaseObjectType* o,
                     const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_accept_button = load <Gtk::Button> ("accept_button");
        m_label = load <Gtk::Label> ("label");
      }

    void setup (Player *enemy)
      {
        set_response (m_close_button, Gtk::ResponseType::CLOSE);
        set_response (m_accept_button, Gtk::ResponseType::ACCEPT);

        Glib::ustring s =
          String::ucompose (_("Are you sure you want to attack %1?\n"
                              "Other players may not like this!"),
                            enemy->getName ());
        m_label->set_text (s);

        signal_response ().connect
          ([this](Gtk::ResponseType)
           {
             hide ();
           });
      }
private:
    Gtk::Button *m_close_button;
    Gtk::Button *m_accept_button;
    Gtk::Label *m_label;
};
#endif
