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
#ifndef RUIN_FIGHT_DIALOG_H
#define RUIN_FIGHT_DIALOG_H
#include "stack.h"
#include "keeper.h"
#include "fight-result.h"
#include "ruin-fight-finished-dialog.h"
#include "hero-gains-level-dialog.h"
#include "army-awarded-medal-dialog.h"
class RuinFightDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "ruinfight-started.ui";
      }

    RuinFightDialog (BaseObjectType* o,
                     const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_continue_button = load <Gtk::Button> ("continue_button");
        m_label = load <Gtk::Label> ("label");
      }

    void setup (Glib::ustring hero_name, Glib::ustring keeper_name, FightResult result)
      {
        m_result = result;
        set_title (_("Searching"));
        set_response (m_continue_button, Gtk::ResponseType::ACCEPT);

        Glib::ustring s =
          String::ucompose (_("%1 encounters %2..."), hero_name, keeper_name);
        m_label->set_text (s);

        signal_response ().connect
          ([this, result](Gtk::ResponseType)
           {
             hide ();
             auto d = LwDialog::build<RuinFightFinishedDialog> (this);
             d->setup (result.get_outcome ());
             d->signal_response ().connect
               ([d, result, this] (Gtk::ResponseType)
                {
                  d->hide ();
                  heroes_level_up (result.get_advancing_heroes ());
                  delete d;
                });
           });
      }

    void heroes_level_up (std::list<Hero*> hero_list)
      {
        m_next_hero = [this](std::list<Hero*> heroes) mutable
          {
            if (heroes.empty ())
              {
                // after we level up heroes we give out three different kinds
                // of medals
                int medal_type = 0;
                army_gets_medal (m_result.get_medalists (medal_type),
                                 medal_type);
                return;
              }
            auto d = LwDialog::build<HeroGainsLevelDialog> (this);
            d->setup (heroes.front (), GameScenario::s_hidden_map);
            d->signal_response ().connect
              ([this, heroes, d] (Gtk::ResponseType) mutable
               {
                 Hero *hero = heroes.front ();
                 Army::Stat stat = d->get_selected_stat ();
                 Player *p = hero->getOwner ();
                 p->heroGainsLevel (hero, stat);
                 heroes.pop_front ();
                 m_next_hero (heroes);
                 delete d;
               });
          };

        m_next_hero (hero_list);
      }

    void army_gets_medal (std::list<Army*> army_list, int medal_type)
      {
        m_next_army = [this, medal_type](std::list<Army*> armies) mutable
          {
            if (armies.empty ())
              {
                if (medal_type == 2)
                  m_signal_ruinfight_finished.emit ();
                else
                  {
                    medal_type++;
                    army_gets_medal
                      (m_result.get_medalists (medal_type), medal_type);
                  }
                return;
              }
            auto d = LwDialog::build<ArmyAwardedMedalDialog> (this);
            d->setup (armies.front (), medal_type);
            d->signal_response ().connect
              ([this, d, armies] (Gtk::ResponseType) mutable
               {
                 armies.pop_front ();
                 m_next_army (armies);
                 delete d;
               });
          };

        m_next_army (army_list);
      }

    sigc::signal<void()> signal_ruinfight_finished ()
      {
        return m_signal_ruinfight_finished;
      }
private:
    Gtk::Button *m_continue_button;
    Gtk::Label *m_label;
    sigc::signal<void()> m_signal_ruinfight_finished;
    std::function<void(std::list<Hero*>)> m_next_hero;
    std::function<void(std::list<Army*>)> m_next_army;
    FightResult m_result;
};
#endif
