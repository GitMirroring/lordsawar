//  Copyright (C) 2007, 2008, 2009, 2012, 2014, 2015, 2017, 2026 Ben Asselstine
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
#ifndef QUEST_REPORT_DIALOG_H
#define QUEST_REPORT_DIALOG_H
#include "quest.h"
#include "quest-map.h"
#include "quest-manager.h"

class QuestReportDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "quest-report.ui";
      }

    QuestReportDialog (BaseObjectType* o,
                       const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_next_quest_button = load <Gtk::Button> ("next_quest_button");
        m_previous_quest_button = load <Gtk::Button> ("previous_quest_button");
        m_label = load <Gtk::Label> ("label");
        m_map_drawing_area = load<Gtk::DrawingArea> ("map_drawing_area");
      }

    ~QuestReportDialog ()
      {
        m_questmap_changed.disconnect ();
      }

    void setup (const std::vector<Quest *>quests, Hero *hero)
      {
        for (auto q : quests)
          m_quests.push_back (q);

        if (hero)
          {
            for (m_quest = m_quests.begin (); m_quest != m_quests.end ();
                 ++m_quest)
              if ((*m_quest)->getHero () == hero)
                break;
          }
        else
          m_quest = m_quests.begin ();

        set_response (m_close_button, Gtk::ResponseType::CLOSE);

        m_questmap = new QuestMap (quests);
        m_questmap_changed = m_questmap->map_changed.connect
          ([this] (Cairo::RefPtr<Cairo::Surface> map)
           {
             cairo_surface_to_drawing_area (map, m_map_drawing_area);
           });

        if (m_quests.empty () == false)
          m_questmap->set_quest (*m_quest);

        m_questmap->resize ();
        m_questmap->draw ();

        m_next_quest_button->set_sensitive (m_quests.size () > 1);
        m_previous_quest_button->set_sensitive (m_quests.size () > 1);

        m_next_quest_button->signal_clicked ().connect
          ([this] ()
           {
             m_quest++;
             if (m_quest == m_quests.end ())
               m_quest = m_quests.begin ();
             m_questmap->set_quest (*m_quest);
             m_questmap->draw ();
             fill_quest_info ((*m_quest));
           });

        m_previous_quest_button->signal_clicked ().connect
          ([this] ()
           {
             if (m_quest == m_quests.begin ())
               m_quest = std::prev (m_quests.end ());
             else
               --m_quest;
             m_questmap->set_quest (*m_quest);
             m_questmap->draw ();
             fill_quest_info ((*m_quest));
           });

        auto click = Gtk::GestureClick::create ();
        click->set_button (1);
        click->signal_pressed ().connect
          ([this] (int n, double x, double y)
           {
             struct MouseButtonEvent ev = to_input_event (n, x, y, true);
             m_questmap->mouse_button_event (ev);
             if (m_questmap->get_quest ())
               {
                 set_quest (m_questmap->get_quest ());
                 m_questmap->draw ();
                 fill_quest_info (*m_quest);
               }
           });
        m_map_drawing_area->add_controller (click);

        auto motion = Gtk::EventControllerMotion::create ();
        motion->signal_motion ().connect
          ([this](double x, double y)
           {
             auto c = m_questmap->get_cursor (x, y);
             static ImageCache::CursorType prev_cursor = ImageCache::SHIP;
             if (c != prev_cursor)
               {
                 auto hotspot = ImageCache::get_hotspot (c);
                 auto im = ImageCache::instance ()->getCursorPic (c);
                 auto cursor = Gdk::Cursor::create (im->to_texture (),
                                                    hotspot.x, hotspot.y);
                 m_map_drawing_area->set_cursor (cursor);
               }
             prev_cursor = c;
           });
        m_map_drawing_area->add_controller (motion);

        if (m_quests.empty ())
          fill_quest_info (NULL);
        else
          fill_quest_info (*m_quest);
      }

private:
    Gtk::Button *m_close_button = NULL;
    Gtk::Button *m_next_quest_button = NULL;
    Gtk::Button *m_previous_quest_button = NULL;
    Gtk::Label *m_label = NULL;
    Gtk::DrawingArea *m_map_drawing_area = NULL;
    QuestMap *m_questmap;
    sigc::connection m_questmap_changed;
    std::vector<Quest*> m_quests;
    std::vector<Quest*>::iterator m_quest;

    void fill_quest_info (Quest *q)
      {
        if (q)
          {
            set_title
              (String::ucompose (_("Quest for %1"), q->getHero ()->getName ()));

            m_questmap->set_quest (q);
            m_questmap->draw ();

            Glib::ustring s = q->getDescription ();
            s += "\n\n";
            s += q->getProgress ();
            m_label->set_text (s);
          }
        else
          {
            set_title (_("No Quest"));
            int num = Rnd::rand () % 3;
            Glib::ustring s = "";
            switch (num)
              {
              case 0:
                s = _("Seek a quest in a temple!");
                break;

              case 1:
                s = _("Quest?  What Quest?");
                break;

              default:
                s = _("Thou hast no quests!");
                break;
              }
            m_label->set_text (s);
          }
      }
        
    void set_quest (Quest *q)
      {
        for (m_quest = m_quests.begin (); m_quest != m_quests.end ();
             ++m_quest)
          if ((*m_quest) == q)
            break;
      }
};
#endif
