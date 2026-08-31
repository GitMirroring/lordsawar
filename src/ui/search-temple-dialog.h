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
#ifndef SEARCH_TEMPLE_DIALOG_H
#define SEARCH_TEMPLE_DIALOG_H
#include "player-list.h"
#include "player.h"
#include "hero.h"
#include "snd.h"
class SearchTempleDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "search-temple.ui";
      }

    SearchTempleDialog (BaseObjectType* o,
                        const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_accept_button = load <Gtk::Button> ("accept_button");
        m_label = load <Gtk::Label> ("label");
      }

    void setup (Hero *hero, Temple *temple, int num_armies_blessed)
      {
        if (num_armies_blessed > 0)
          Snd::instance ()->play ("bless", 1);

        set_title (temple->getName ());

        set_response (m_close_button, Gtk::ResponseType::CLOSE);
        set_response (m_accept_button, Gtk::ResponseType::ACCEPT);

        bool hasHero = hero != NULL;
        bool ask_quest = false;
        if (GameScenarioOptions::s_play_with_quests ==
            GameParameters::ONE_QUEST_PER_PLAYER)
          {
            if (QuestsManager::instance ()->getPlayerQuests
                (Playerlist::getActiveplayer ()).size () == 0 && hasHero)
              ask_quest = true;
          }
        else if (GameScenarioOptions::s_play_with_quests ==
                 GameParameters::ONE_QUEST_PER_HERO)
          {
            if (hasHero && hero->hasQuest () == false)
              ask_quest = true;
          }

        Glib::ustring s;

        if (num_armies_blessed > 0)
          s = String::ucompose
            (ngettext ("%1 army has been blessed!",
                       "%1 armies have been blessed!", num_armies_blessed),
             num_armies_blessed);
        else
          s = _("We have already blessed thee!");

        s += "\n" + _("Seek more blessings in far temples!");
        if (ask_quest)
          s += "\n\n" + _("Do you seek a quest?");
        m_label->set_text (s);

        if (ask_quest == false)
          {
            m_accept_button->set_visible (false);
            m_close_button->set_label (_("_Close"));
          }

        signal_response ().connect
          ([this, hero, temple](Gtk::ResponseType resp)
           {
             switch (resp)
               {
               case Gtk::ResponseType::ACCEPT:
                 m_temple_searched.emit (true);
                 hide ();
                 break;

               default:
                 m_temple_searched.emit (false);
                 hide ();
                 break;
               }
           });
      }

    sigc::signal<void(bool)> signal_temple_searched ()
      {
        return m_temple_searched;
      }
private:
    Gtk::Button *m_close_button = NULL;
    Gtk::Button *m_accept_button = NULL;
    Gtk::Label *m_label = NULL;
    sigc::signal<void(bool)> m_temple_searched;
};
#endif
