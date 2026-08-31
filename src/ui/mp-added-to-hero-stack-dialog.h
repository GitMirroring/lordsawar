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
#ifndef MP_ADDED_TO_HERO_STACK_DIALOG_H
#define MP_ADDED_TO_HERO_STACK_DIALOG_H
class MPAddedToHeroStackDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "mp-added-to-hero-stack.ui";
      }

    MPAddedToHeroStackDialog (BaseObjectType* o,
                      const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_continue_button = load <Gtk::Button> ("continue_button");
        m_label = load <Gtk::Label> ("label");
      }

    void setup (Hero *hero, guint32 mp)
      {
        Glib::ustring s =
          String::ucompose
          (ngettext
           ("%1 movement point was added to %2 and accompanying units!",
            "%1 movement points were added to %2 and accompanying units!",
            mp), mp, hero->getName ());

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
