//  Copyright (C) 2007, 2008, 2009, 2012, 2014, 2017, 2026 Ben Asselstine
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
#ifndef QUEST_ASSIGNED_DIALOG_H
#define QUEST_ASSIGNED_DIALOG_H
#include "image-helpers.h"
#include "quest-map.h"
#include "quest.h"
#include "hero.h"
#include "temple.h"
class QuestAssignedDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "quest-assigned.ui";
      }

    QuestAssignedDialog (BaseObjectType* o,
                         const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_continue_button = load <Gtk::Button> ("continue_button");
        m_label = load <Gtk::Label> ("label");
        m_map_drawing_area = load <Gtk::DrawingArea> ("map_drawing_area");
      }

    void setup (Hero *hero, Quest *quest)
      {
        set_title (String::ucompose (_("Quest for %1"), hero->getName ()));

        set_response (m_continue_button, Gtk::ResponseType::ACCEPT);

        m_quest_map = new QuestMap (quest);
        m_quest_map->map_changed.connect
          ([this] (Cairo::RefPtr<Cairo::Surface> map)
           {
             cairo_surface_to_drawing_area (map, m_map_drawing_area);
           });

        m_quest_map->resize ();
        m_quest_map->draw ();

        m_label->set_text (quest->getDescription ());

        signal_response ().connect
          ([this](Gtk::ResponseType)
           {
             hide ();
           });
      }
private:
    Gtk::Button *m_continue_button = NULL;
    Gtk::Label *m_label = NULL;
    Gtk::DrawingArea *m_map_drawing_area = NULL;
    QuestMap* m_quest_map = NULL;
};
#endif
