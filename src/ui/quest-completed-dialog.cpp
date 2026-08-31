//  Copyright (C) 2007, 2008, 2009, 2012, 2014, 2017 Ben Asselstine
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#include <config.h>

#include <gtkmm.h>
#include <sigc++/functors/mem_fun.h>

#include "SightMap.h"
#include "quest-completed-dialog.h"

#include "ucompose.hpp"
#include "hero.h"
#include "defs.h"
#include "ruin.h"
#include "Item.h"

#define method(x) sigc::mem_fun(*this, &QuestCompletedDialog::x)

QuestCompletedDialog::QuestCompletedDialog(Gtk::Window &parent, Quest *q, Reward *r)
 : LwDialog(parent, "quest-assigned-dialog.ui")
{
  reward = r;
  quest = q;

  xml->get_widget("map_image", map_image);

  questmap = new QuestMap(quest);
  questmap->map_changed.connect (method(on_map_changed));

  Gtk::EventBox *map_eventbox;
  xml->get_widget("map_eventbox", map_eventbox);

  dialog->set_title(String::ucompose(_("Quest for %1"), q->getHero()->getName()));

  xml->get_widget("label", label);
  Glib::ustring s = String::ucompose(_("%1 completed the quest!"),
                                     q->getHero()->getName());
  s += "\n\n";
  // add messages from the quest
  std::queue<Glib::ustring> msgs;
  quest->getSuccessMsg(msgs);
  while (!msgs.empty())
    {
      s += msgs.front();
      s += "\n\n";
      msgs.pop();
    }
  if (reward->getType() == Reward::GOLD)
    {
      guint32 gold = dynamic_cast<Reward_Gold*>(reward)->getGold();
      s += String::ucompose(ngettext("You have been rewarded with %1 gold piece.",
                                     "You have been rewarded with %1 gold pieces.",
                                     gold), gold);
    }
  else if (reward->getType() == Reward::ALLIES)
    {
      guint32 num = dynamic_cast<Reward_Allies*>(reward)->getNoOfAllies();
      s += String::ucompose(ngettext("You have been rewarded with %1 ally.",
                                     "You have been rewarded with %1 allies.",
                                     num), num);
    }
  else if (reward->getType() == Reward::ITEM)
    {
      Item *item = dynamic_cast<Reward_Item*>(reward)->getItem();
      s += String::ucompose(_("You have been rewarded with the %1."), 
                            item->getName());
    }
  else if (reward->getType() == Reward::RUIN)
    {
      Ruin *ruin = dynamic_cast<Reward_Ruin*>(reward)->getRuin();
      s += String::ucompose(_("You are shown the site of %1\n"), ruin->getName());
      questmap->set_target(ruin->getPos());
      if (ruin->getReward() == NULL)
        ruin->populateWithRandomReward();
      Reward *ruin_reward = ruin->getReward();
      if (ruin_reward->getType() == Reward::ALLIES)
        s += _("where powerful allies can be found!");
      else if (ruin_reward->getType() == Reward::ITEM)
        {
          Item *item = dynamic_cast<Reward_Item*>(ruin_reward)->getItem();
          s += String::ucompose(_("where the %1 can be found!"), 
                                item->getName());
        }
      else if (ruin_reward->getType() == Reward::MAP)
        s += _("where a map can be found!");
      else if (ruin_reward->getType() == Reward::RUIN)
        s += _("where nothing can be found!");
      else if (ruin_reward->getType() == Reward::GOLD)
        s += _("where gold can be found!");
      else //this one shouldn't happen
        s += _("where something important can be found!");
    }

  label->set_text(s);
}

void QuestCompletedDialog::run()
{
  questmap->resize();
  questmap->draw();

  dialog->show_all();
  dialog->run();
}

void QuestCompletedDialog::on_map_changed(Cairo::RefPtr<Cairo::Surface> map)
{
  map_image->property_pixbuf() =
    Gdk::Pixbuf::create(map, 0, 0, 
                        questmap->get_width(), questmap->get_height());
}

