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
#ifndef QUEST_COMPLETED_DIALOG_H
#define QUEST_COMPLETED_DIALOG_H
#include "quest.h"
#include "quest-map.h"
class QuestCompletedDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "quest-completed.ui";
      }

    QuestCompletedDialog (BaseObjectType* o,
                       const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_label = load <Gtk::Label> ("label");
        m_button = load <Gtk::Button> ("continue_button");
        m_map_drawing_area = load <Gtk::DrawingArea> ("map_drawing_area");
      }

    void setup (Quest *quest)
      {
        set_title (String::ucompose (_("Quest for %1"), quest->getHeroName ()));

        Reward *reward = quest->getReward ();
        set_response (m_button, Gtk::ResponseType::CLOSE);

        m_quest_map = new QuestMap (quest);
        m_quest_map->map_changed.connect
          ([this] (Cairo::RefPtr<Cairo::Surface> map)
           {
             cairo_surface_to_drawing_area (map, m_map_drawing_area);
           });

        m_quest_map->resize ();
        m_quest_map->draw ();

        Glib::ustring s =
          String::ucompose (_("%1 completed the quest!"),
                            quest->getHeroName ());
        s += "\n\n";

        // add messages from the quest
        std::queue<Glib::ustring> msgs;
        quest->getSuccessMsg (msgs);
        while (!msgs.empty ())
          {
            s += msgs.front () + "\n";
            msgs.pop ();
            if (!msgs.empty ())
              s += "\n\n";
          }

        if (reward->getType () == Reward::GOLD)
          {
            guint32 gold = dynamic_cast<Reward_Gold*>(reward)->getGold ();
            s += String::ucompose
              (ngettext ("You have been rewarded with %1 gold piece.",
                         "You have been rewarded with %1 gold pieces.",
                         gold), gold);
          }
        else if (reward->getType () == Reward::ALLIES)
          {
            guint32 num =
              dynamic_cast<Reward_Allies*>(reward)->getNoOfAllies ();
            s += String::ucompose
              (ngettext ("You have been rewarded with %1 ally.",
                         "You have been rewarded with %1 allies.",
                         num), num);
          }
        else if (reward->getType () == Reward::ITEM)
          {
            Item *item = dynamic_cast<Reward_Item*>(reward)->getItem ();
            s += String::ucompose (_("You have been rewarded with the %1."), 
                                   item->getName ());
          }
        else if (reward->getType () == Reward::RUIN)
          {
            Ruin *ruin = dynamic_cast<Reward_Ruin*>(reward)->getRuin ();
            s += String::ucompose (_("You are shown the site of %1\n"),
                                   ruin->getName ());
            m_quest_map->set_target (ruin->getPos ());
            if (ruin->getReward () == NULL)
              ruin->populateWithRandomReward ();
            Reward *ruin_reward = ruin->getReward ();
            switch (ruin_reward->getType ())
              {
              case Reward::ALLIES:
                s += _("where powerful allies can be found!");
                break;

              case Reward::ITEM:
                  {
                    Item *item =
                      dynamic_cast<Reward_Item*>(ruin_reward)->getItem ();
                    s += String::ucompose
                      (_("where the %1 can be found!"), item->getName ());
                  }
                break;

              case Reward::MAP:
                s += _("where a map can be found!");
                break;

              case Reward::RUIN:
                s += _("where nothing can be found!");
                //we don't get this one
                break;

              case Reward::GOLD:
                s += _("where gold can be found!");
                break;

              default:
                s += _("where something important can be found!");
                break;
              }
          }
        s += "\n" + _("Well done!");

        m_label->set_text (s);
        m_label->set_margin (6);
        m_label->set_justify (Gtk::Justification::CENTER);
        m_label->set_hexpand (true);

        signal_response ().connect
          ([this](Gtk::ResponseType)
           {
             hide ();
           });
      }
private:
    Gtk::Label *m_label;
    Gtk::Button *m_button;
    Gtk::DrawingArea *m_map_drawing_area = NULL;
    QuestMap* m_quest_map = NULL;
};
#endif
