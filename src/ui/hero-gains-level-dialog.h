//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2011, 2014, 2020, 2026 Ben Asselstine
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
#ifndef HERO_GAINS_LEVEL_DIALOG_H
#define HERO_GAINS_LEVEL_DIALOG_H
#include "image-cache.h"
#include "hero.h"
class HeroGainsLevelDialog: public LwDialogBase
{
public:

    struct StatItem
      {
        Army::Stat stat;
        Glib::ustring desc;
        Gtk::ToggleButton *button;
      };

    static std::string get_resource_name ()
      {
        return "hero-gains-level.ui";
      }

    HeroGainsLevelDialog (BaseObjectType* o,
                          const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_choose_button = load <Gtk::Button> ("choose_button");
        m_stats_vbox = load <Gtk::Box> ("stats_vbox");
        m_hero_picture = load <Gtk::Picture> ("hero_picture");
        m_hero_icon = load <Gtk::Image> ("hero_icon");
        m_label = load <Gtk::Label> ("label");
      }

    void setup (Hero *hero, bool show_sight_stat)
      {
        set_response (m_choose_button, Gtk::ResponseType::ACCEPT);

        m_hero = hero;
        m_show_sight_stat = show_sight_stat;

        set_title
          (String::ucompose (_("%1 has advanced to level %2!"),
                             hero->getName (),
                             hero->getLevel () + 1));

        Glib::ustring s = "\n\n";
        s += _("Choose an attribute to improve:");
        m_label->set_text (s);

        Player *p = Playerlist::getActiveplayer ();
        auto im =
          ImageCache::instance ()->getNewLevelPic (p->get_shield (),
                                                   hero->getGender ());
        m_hero_picture->set_paintable (im->to_texture ());

        m_hero_icon->set_pixel_size (LW_BUTTON_SIZE);
        m_hero_icon->set 
          (ImageCache::instance ()->getCircledArmyPic
           (hero, false, Shield::NEUTRAL, true,
            Lw::get_dark ())->to_pixbuf ());

        add_toggle (Army::MOVES, _("Moves: %1"));
        if (show_sight_stat == true)
          add_toggle (Army::SIGHT, _("Sight: %1"));
        if (hero->getStat(Army::STRENGTH, false) < MAX_ARMY_STRENGTH)
          add_toggle (Army::STRENGTH, _("Strength: %1"));

        fill_in_descriptions ();

        m_choose_button->set_sensitive (false);
        set_default_widget (*m_choose_button);
        signal_response ().connect
          ([this](Gtk::ResponseType)
           {
             hide ();
           });
      }

    Army::Stat get_selected_stat ()
      {
        return m_selected_stat;
      }

    void setup ()
      {
      }

    void add_toggle (Army::Stat stat, Glib::ustring desc)
      {
        StatItem item;
        item.stat = stat;
        item.desc = desc;
        if (m_stat_toggles.empty ())
          item.button = Gtk::make_managed<Gtk::ToggleButton> ();
        else
          {
            item.button = Gtk::make_managed<Gtk::ToggleButton> ();
            item.button->set_group (*m_stat_toggles.front ().button);
          }

        m_stat_toggles.push_back (item);

        item.button->signal_toggled ().connect
          ([this] ()
           {
             for (unsigned int i = 0; i < m_stat_toggles.size (); ++i)
               if (m_stat_toggles[i].button->get_active ())
                 {
                   m_selected_stat = m_stat_toggles[i].stat;
                   m_choose_button->set_sensitive (true);
                   break;
                 }

             fill_in_descriptions ();
           });

        m_stats_vbox->append (*item.button);
      }

    void fill_in_descriptions ()
      {
        for (unsigned int i = 0; i < m_stat_toggles.size (); ++i)
          {
            StatItem &item = m_stat_toggles[i];

            int v = m_hero->getStat (item.stat, false);

            if (item.button->get_active ())
              v += m_hero->computeLevelGain (item.stat);

            item.button->set_label (String::ucompose (item.desc, v));
          }
      }
private:
    Gtk::Button *m_choose_button = NULL;
    Gtk::Box *m_stats_vbox = NULL;
    Gtk::Picture *m_hero_picture = NULL;
    Gtk::Image *m_hero_icon = NULL;
    Gtk::Label *m_label = NULL;
    Army::Stat m_selected_stat = Army::Stat::MOVES;
    Hero *m_hero = NULL;
    bool m_show_sight_stat = false;
    std::vector<StatItem> m_stat_toggles = {};
};
#endif
