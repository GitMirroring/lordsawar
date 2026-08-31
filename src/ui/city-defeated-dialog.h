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
#ifndef CITY_DEFEATED_DIALOG_H
#define CITY_DEFEATED_DIALOG_H
#include "rnd.h"
#include "quest-manager.h"
#include "city.h"
#include "quest-city-occupy.h"
#include "quest-pillage-gold.h"
#include "quest-city-sack.h"
#include "quest-city-raze.h"
class CityDefeatedDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "city-defeated.ui";
      }

    CityDefeatedDialog (BaseObjectType* o,
                        const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_occupy_button = load <Gtk::Button> ("occupy_button");
        m_pillage_button = load <Gtk::Button> ("pillage_button");
        m_sack_button = load <Gtk::Button> ("sack_button");
        m_raze_button = load <Gtk::Button> ("raze_button");
        m_picture = load <Gtk::Picture> ("picture");
        m_label = load <Gtk::Label> ("label");
      }

    void setup (City *city)
      {
        set_response (m_occupy_button, Gtk::ResponseType::ACCEPT);
        set_response (m_pillage_button, Gtk::ResponseType::CLOSE);
        set_response (m_sack_button, Gtk::ResponseType::OK);
        set_response (m_raze_button, Gtk::ResponseType::REJECT);
        setup_label (city);

        int i = ImageCache::DIALOG_CONQUERED_CITY;
        auto im = ImageCache::instance ()->getDialogPic (i);
        m_picture->set_paintable (im->to_texture ());

        switch (GameScenarioOptions::s_sacking_mode)
          {
          case GameParameters::SACKING_ALWAYS:
          case GameParameters::SACKING_ON_CAPTURE:
            m_sack_button->set_sensitive (true);
            m_pillage_button->set_sensitive (true);
            break;

          case GameParameters::SACKING_ON_QUEST:
          case GameParameters::SACKING_NEVER:
            m_sack_button->set_sensitive (false);
            m_pillage_button->set_sensitive (false);
            break;
          }
        m_raze_button->set_sensitive
          (GameScenarioOptions::s_razing_cities == GameParameters::ON_CAPTURE ||
           GameScenarioOptions::s_razing_cities == GameParameters::ALWAYS);

        auto p = Playerlist::getActiveplayer ();
        auto h = p->getActivestack ()->getFirstHero ();

        bool quest_default = false;
        if (h) /* if there was a hero in the stack */
          {
            bool pillage, sack, raze, occupy;
            if (hero_has_quest_here (p->getActivestack (), city, 
                                     &pillage, &sack, &raze, &occupy))
              {
                if (pillage && city->getNoOfProductionBases () > 0)
                  {
                    quest_default = true;
                    m_pillage_button->set_sensitive (true);
                    set_default_widget (*m_pillage_button);
                    m_pillage_button->add_css_class ("suggested-action");
                  }

                if (sack && city->getNoOfProductionBases () > 1)
                  {
                    quest_default = true;
                    m_sack_button->set_sensitive (true);
                    m_sack_button->add_css_class ("suggested-action");
                    set_default_widget (*m_sack_button);
                  }

                if (raze)
                  {
                    quest_default = true;
                    m_raze_button->set_sensitive (true);
                    m_raze_button->add_css_class ("suggested-action");
                    set_default_widget (*m_raze_button);
                  }

                if (occupy)
                  {
                    quest_default = true;
                    m_occupy_button->add_css_class ("suggested-action");
                    set_default_widget (*m_occupy_button);
                  }
              }
          }

        if (quest_default == false)
          set_default_widget (*m_occupy_button);
        if (city->getNoOfProductionBases () <= 0)
          m_pillage_button->set_sensitive (false);

        if (city->getNoOfProductionBases () <= 1)
          m_sack_button->set_sensitive (false);

        signal_response ().connect
          ([this](Gtk::ResponseType)
           {
             hide ();
           });
      }

    private:
        Gtk::Button *m_occupy_button;
        Gtk::Button *m_pillage_button;
        Gtk::Button *m_sack_button;
        Gtk::Button *m_raze_button;
        Gtk::Picture *m_picture;
        Gtk::Label *m_label;

        void setup_label (City *city)
          {
            Glib::ustring s = "";
            switch (Rnd::rand() % 4)
              {
              case 0:
                s =
                  String::ucompose
                  (_("%1, you have triumphed in the battle of %2."),
                   get_name (), city->getName ());
                break;

              case 1:
                s =
                  String::ucompose
                  (_("%1, you have claimed victory in the battle of %2."),
                   get_name (), city->getName ());
                break;

              case 2:
                s =
                  String::ucompose
                  (_("%1, you have shown no mercy in the battle of %2."),
                   get_name (), city->getName ());
                break;
              case 3:
                s =
                  String::ucompose
                  (_("%1, you have slain your foes in the battle of %2."),
                   get_name (), city->getName ());
                break;
              }
            s += "\n\n";

            s += _("The city is yours! Will you...");
            m_label->set_text (s);
          }

        Glib::ustring get_name ()
          {
            Glib::ustring name;
            Player *p = Playerlist::getActiveplayer ();
            Army *h = NULL;
            if (p->getActivestack ())
              h = p->getActivestack ()->getFirstHero ();
            if (h)
              name = h->getName ();
            else
              name = p->getName ();
            return name;
          }
      
        bool hero_has_quest_here (Stack *s, City *c, bool *pillage, bool *sack,
                                  bool *raze, bool *occupy)
          {
            *pillage = false;
            *sack = false;
            *raze = false;
            *occupy = false;

            auto quests =
              QuestsManager::instance ()->getPlayerQuests
              (Playerlist::getActiveplayer ());

            /* loop over all quests */
            /* for each quest, check the quest type */
            for (auto i = quests.begin (); i != quests.end (); ++i)
              {
                if ((*i) == NULL)
                  continue;
                if ((*i)->isPendingDeletion () == true)
                  continue;
                switch ((*i)->getType ())
                  {
                  case Quest::CITYSACK:
                    if (dynamic_cast<QuestCitySack*>((*i))->getCity () != c)
                      continue;
                    break;
                  case Quest::CITYRAZE:
                    if (dynamic_cast<QuestCityRaze*>((*i))->getCity () != c)
                      continue;
                    break;
                  case Quest::CITYOCCUPY:
                    if (dynamic_cast<QuestCityOccupy*>((*i))->getCity () != c)
                      continue;
                    break;
                  case Quest::PILLAGEGOLD:
                    *pillage = true;
                    *sack = true;
                    break;
                  }

                if ((*i)->getType () == Quest::CITYOCCUPY ||
                    (*i)->getType () == Quest::CITYRAZE ||
                    (*i)->getType () == Quest::CITYSACK)
                  {
                    /* now check if the quest's hero is in our stack */
                    for (auto it = s->begin (); it != s->end (); ++it)
                      {
                        if ((*it)->isHero ())
                          {
                            if ((*it)->getId () == (*i)->getHeroId ())
                              {
                                /* hey we found one */
                                if ((*i)->getType () == Quest::CITYSACK)
                                  *sack = true;
                                else if ((*i)->getType () == Quest::CITYRAZE)
                                  *raze = true;
                                else if ((*i)->getType () == Quest::CITYOCCUPY)
                                  *occupy = true;
                              }
                          }
                      }

                  }
              }
            if (*raze || *sack || *occupy)
              return true;
            else
              return false;
          }
};
#endif
