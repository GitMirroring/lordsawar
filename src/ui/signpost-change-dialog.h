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
#ifndef SIGNPOST_CHANGE_DIALOG_H
#define SIGNPOST_CHANGE_DIALOG_H
#include "signpost.h"
class SignpostChangeDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "signpost-change.ui";
      }

    SignpostChangeDialog (BaseObjectType* o,
                          const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_accept_button = load <Gtk::Button> ("accept_button");
        m_message_entry = load <Gtk::Entry> ("message_entry");
        m_label = load <Gtk::Label> ("label");
      }

    Glib::ustring get_message ()
      {
        return m_message_entry->get_text ();
      }

    void setup (Signpost *sign)
      {
        Glib::ustring s = _("Change the message on this sign:");
        m_label->set_text (s);

        m_message_entry->set_text (sign->getName ());

        set_response (m_close_button, Gtk::ResponseType::CLOSE);
        set_response (m_accept_button, Gtk::ResponseType::ACCEPT);
      }
private:
    Gtk::Button *m_close_button;
    Gtk::Button *m_accept_button;
    Gtk::Entry *m_message_entry;
    Gtk::Label *m_label;
};
#endif
