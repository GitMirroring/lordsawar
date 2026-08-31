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
#ifndef RUIN_SEARCHED_DIALOG_H
#define RUIN_SEARCHED_DIALOG_H
#include "ruin.h"
#include "stack.h"
#include "reward.h"
class RuinSearchedDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "ruin-searched.ui";
      }

    RuinSearchedDialog (BaseObjectType* o,
                        const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_continue_button = load <Gtk::Button> ("continue_button");
        m_label = load <Gtk::Label> ("label");
      }

    void setup (Ruin *ruin, Stack *stack, Reward *reward)
      {
        set_response (m_continue_button, Gtk::ResponseType::ACCEPT);

        Glib::ustring hero_name = stack->getFirstHero ()->getName ();

        Glib::ustring s = "";
        switch (reward->getType ())
          {
          case Reward::GOLD:
              {
                Reward_Gold *gold = dynamic_cast<Reward_Gold*> (reward);
                if (ruin->hasSage ())
                  s = String::ucompose (_("%1 is given %2 gold pieces."),
                                        hero_name, gold->getGold ());
                else
                  s = String::ucompose (_("%1 finds %2 gold pieces."),
                                        hero_name, gold->getGold ());
              }
            break;

          case Reward::ALLIES:
              {
                Reward_Allies *allies = dynamic_cast<Reward_Allies*> (reward);
                if (ruin->hasSage( ))
                  s = String::ucompose (_("%1 is given %2 allies!"),
                                        hero_name, allies->getNoOfAllies ());
                else
                  s = String::ucompose (_("%1 finds %2 allies!"),
                                        hero_name, allies->getNoOfAllies ());
              }
            break;

          case Reward::ITEM:
              {
                Reward_Item *item = dynamic_cast<Reward_Item*> (reward);
                if (ruin->hasSage ())
                  s = String::ucompose (_("%1 is given the %2!"), hero_name,
                                        item->getItem ()->getName ());
                else
                  s = String::ucompose (_("%1 finds the %2!"),
                                        hero_name, item->getItem()->getName ());
              }
            break;

          case Reward::MAP:
              {
                Reward_Map *map = dynamic_cast<Reward_Map*> (reward);
                if (ruin->hasSage ())
                  s = String::ucompose (_("%1 is given a %2!"),
                                        hero_name, map->getName ());
                else
                  s = String::ucompose (_("%1 finds a %2!"),
                                        hero_name, map->getName ());
              }
            break;

          case Reward::RUIN:
              {
                Reward_Ruin *rr = dynamic_cast<Reward_Ruin*> (reward);
                auto ruin_name = rr->getRuin ()->getName ();
                if (ruin->hasSage ())
                  s = String::ucompose (_("%1 is shown the site of %2!"),
                                        hero_name, ruin_name);
                else
                  s = String::ucompose (_("%1 learns about the site of %2!"),
                                        hero_name, ruin_name);
              }
            break;
          }

        m_label->set_text (s);

        signal_response ().connect
          ([this](Gtk::ResponseType)
           {
             hide ();
           });
      }
private:
    Gtk::Button *m_continue_button = NULL;
    Gtk::Label *m_label = NULL;
};
#endif
