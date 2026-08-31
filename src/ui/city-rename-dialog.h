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
#ifndef CITY_RENAME_DIALOG_H
#define CITY_RENAME_DIALOG_H
class CityRenameDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "city-rename.ui";
      }

    CityRenameDialog (BaseObjectType* o,
                      const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_accept_button = load <Gtk::Button> ("accept_button");
        m_name_entry = load <Gtk::Entry> ("name_entry");
        m_label = load <Gtk::Label> ("label");
      }

    bool is_valid_name (Glib::ustring n)
      {
        if (n == "")
          return false;
        return true;
      }

    void setup (City *city)
      {
        Glib::ustring s = _("Type the new name for this city:");
        m_label->set_text (s);

        m_name_entry->set_text (city->getName ());

        set_response (m_close_button, Gtk::ResponseType::CLOSE);
        set_response (m_accept_button, Gtk::ResponseType::ACCEPT);

        signal_response ().connect
          ([this, city](Gtk::ResponseType resp)
           {
             auto name = String::utrim (m_name_entry->get_text ());
             switch (resp)
               {
               case Gtk::ResponseType::ACCEPT:
                 if (is_valid_name (name))
                   Playerlist::getActiveplayer ()->cityRename (city, name);
                 break;
               default:
                 break;
               }

             hide ();
           });
      }
private:
    Gtk::Button *m_close_button;
    Gtk::Button *m_accept_button;
    Gtk::Entry *m_name_entry;
    Gtk::Label *m_label;
};
#endif
