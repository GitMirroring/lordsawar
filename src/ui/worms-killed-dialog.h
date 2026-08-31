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
#ifndef WORMS_KILLED_DIALOG_H
#define WORMS_KILLED_DIALOG_H
class WormsKilledDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "worms-killed.ui";
      }

    WormsKilledDialog (BaseObjectType* o,
                       const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_continue_button = load <Gtk::Button> ("continue_button");
        m_label = load <Gtk::Label> ("label");
      }

    void setup (Hero *hero, Glib::ustring name, guint32 num_killed)
      {
        Glib::ustring s =
          String::ucompose (ngettext ("%1 unit of %2 was banished by %3!",
                                      "%1 units of %2 were banished by %3!",
                                      num_killed), num_killed, name,
                            hero->getName ());
        m_label->set_text (s);

        set_response (m_continue_button, Gtk::ResponseType::ACCEPT);

        signal_response ().connect
          ([this](Gtk::ResponseType)
           {
             hide ();
           });
      }
private:
    Gtk::Button *m_continue_button;
    Gtk::Label *m_label;
};
#endif
