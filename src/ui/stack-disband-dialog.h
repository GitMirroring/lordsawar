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
#ifndef STACK_DISBAND_DIALOG_H
#define STACK_DISBAND_DIALOG_H
#include "stack.h"

class StackDisbandDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "disband-stack.ui";
      }

    StackDisbandDialog (BaseObjectType* o,
                        const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_cancel_button = load <Gtk::Button> ("cancel_button");
        m_accept_button = load <Gtk::Button> ("accept_button");
        m_label = load <Gtk::Label> ("label");
      }

    void setup (Stack *stack)
      {
        std::vector<guint32> heroes;
        stack->getHeroes (heroes);
        Glib::ustring s = _("Are you sure you want to disband this group?");
        if (heroes.size () > 0)
          {
            s += "\n";
            s += String::ucompose (ngettext ("(It contains %1 hero).",
                                             "(It contains %1 heroes).",
                                             heroes.size ()), heroes.size ());
          }

        m_label->set_text (s);

        set_response (m_cancel_button, Gtk::ResponseType::CANCEL);
        set_response (m_accept_button, Gtk::ResponseType::ACCEPT);
      }

private:
    Gtk::Button *m_cancel_button;
    Gtk::Button *m_accept_button;
    Gtk::Label *m_label;
};
#endif
