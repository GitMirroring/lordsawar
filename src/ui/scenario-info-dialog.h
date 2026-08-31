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
#ifndef SCENARIO_INFO_DIALOG_H
#define SCENARIO_INFO_DIALOG_H

class ScenarioInfoDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "scenario-info.ui";
      }

    ScenarioInfoDialog (BaseObjectType* o,
                        const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_name_label = load <Gtk::Label> ("name_label");
        m_description_label = load <Gtk::Label> ("description_label");
      }

    ~ScenarioInfoDialog ()
      {
      }

    void setup (GameScenario *s)
      {
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        if (String::utrim (s->getName ()) == "")
          {
            m_name_label->add_css_class ("status-label");
            m_name_label->set_text (_("(No scenario name given.)"));
          }
        else
          m_name_label->set_text (s->getName ());

        if (s->getComment () == "")
          {
            m_description_label->add_css_class ("status-label");
            m_description_label->set_text (_("(No description.)"));
          }
        else
          m_description_label->set_text (s->getComment ());
      }

private:
    Gtk::Button *m_close_button;
    Gtk::Label *m_name_label;
    Gtk::Label *m_description_label;
};
#endif
