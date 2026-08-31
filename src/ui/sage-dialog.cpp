//  Copyright (C) 2007, 2008, 2009, 2014, 2015, 2017 Ben Asselstine
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
#include "sage-dialog.h"

#include "defs.h"
#include "snd.h"
#include "ruin.h"
#include "rewardlist.h"
#include "playerlist.h"
#include "Item.h"
#include "rnd.h"
#include "ucompose.hpp"
#include "armyproto.h"

#define method(x) sigc::mem_fun(*this, &SageDialog::x)

SageDialog::SageDialog(Gtk::Window &parent, Sage *s, Hero *h, Ruin *r)
 : LwDialog(parent, "sage-dialog.ui")
{
  ruin = r;
  hero = h;
  sage = s;

  rewards_list = Gtk::ListStore::create(rewards_columns);
  xml->get_widget("rewardtreeview", rewards_treeview);
  rewards_treeview->set_model(rewards_list);
  rewards_treeview->append_column("", rewards_columns.name);
  rewards_treeview->get_selection()->signal_changed().connect
    (method(on_reward_selected));

  xml->get_widget("map_image", map_image);
  xml->get_widget("continue_button", continue_button);

  ruinmap = new RuinMap(ruin, NULL);
  ruinmap->map_changed.connect(method(on_map_changed));

  Gtk::EventBox *map_eventbox;
  xml->get_widget("map_eventbox", map_eventbox);

  dialog->set_title(_("A Sage!"));

  for(Sage::iterator it = sage->begin(); it != sage->end(); ++it)
    addReward(*it);

  continue_button->set_sensitive(false);
}

Reward *SageDialog::grabSelectedReward()
{
  Glib::RefPtr<Gtk::TreeView::Selection> sel = rewards_treeview->get_selection();
  Gtk::TreeModel::iterator it = sel->get_selected();
  Gtk::TreeModel::Row row = *it;
  return row[rewards_columns.reward];
}

Reward *SageDialog::run()
{
  ruinmap->resize();
  ruinmap->draw();

  Snd::getInstance()->play("hero", 1);
  dialog->show_all();
  dialog->run();
  Snd::getInstance()->halt();

  sage->selectReward(grabSelectedReward());
  return sage->getSelectedReward();
}

void SageDialog::on_map_changed(Cairo::RefPtr<Cairo::Surface> map)
{
  map_image->property_pixbuf() =
    Gdk::Pixbuf::create(map, 0, 0, ruinmap->get_width(), ruinmap->get_height());
}

void SageDialog::addReward(Reward *reward)
{
  Gtk::TreeIter i = rewards_list->append();
  switch (reward->getType())
    {
    case Reward::GOLD:
      (*i)[rewards_columns.name] = _("Gold");
      break;
    case Reward::ITEM:
      {
        Reward_Item *item = static_cast<Reward_Item*>(reward);
        (*i)[rewards_columns.name] = item->getItem()->getName();
      }
      break;
    case Reward::ALLIES:
      (*i)[rewards_columns.name] = _("Allies");
      break;
    case Reward::MAP:
	{
	  Reward_Map *m = static_cast<Reward_Map*>(reward);
	  (*i)[rewards_columns.name] = String::capitalize(m->getName());
	}
      break;
    case Reward::RUIN:
        {
	  Reward_Ruin *rr = static_cast<Reward_Ruin*>(reward);
          Ruin *r = rr->getRuin();
          if (r->getReward()->getType() == Reward::ITEM)
            (*i)[rewards_columns.name] =
              static_cast<Reward_Item*>(r->getReward())->getItem()->getName();
          else if (r->getReward()->getType() == Reward::ALLIES)
            (*i)[rewards_columns.name] = _("Allies");
          else if (r->getReward()->getType() == Reward::MAP)
            (*i)[rewards_columns.name] = r->getReward()->getName();
        }
      break;
    }
  (*i)[rewards_columns.reward] = reward;
}

void SageDialog::on_reward_selected()
{
  continue_button->set_sensitive(true);
}

