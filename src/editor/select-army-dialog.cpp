//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014, 2020 Ben Asselstine
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
#include <assert.h>

#include "gui/input-helpers.h"
#include "select-army-dialog.h"

#include "ucompose.hpp"
#include "playerlist.h"
#include "defs.h"
#include "armyproto.h"
#include "ImageCache.h"
#include "armysetlist.h"
#include "font-size.h"

#define method(x) sigc::mem_fun(*this, &SelectArmyDialog::x)

SelectArmyDialog::SelectArmyDialog(Gtk::Window &parent, Mode mode,
                                   Player *p, int pre_selected_type)
 : LwEditorDialog(parent, "select-army-dialog.ui")
{
  d_clear = pre_selected_type != -1;
  army_info_tip = NULL;
  player = p;
  selected_army = NULL;

  xml->get_widget("army_info_label1", army_info_label1);
  xml->get_widget("army_info_label2", army_info_label2);
  xml->get_widget("select_button", select_button);
  xml->get_widget("clear_button", clear_button);

  xml->get_widget("army_toggles_table", toggles_table);

  fill_in_army_toggles(mode);
  toggles_table->signal_selected_children_changed().connect (method (on_army_selected));
  if (selectable.empty () == false)
    {
      if (pre_selected_type > -1)
        preselect_army ((guint32)pre_selected_type);
      else
        toggles_table->select_child (*toggles_table->get_child_at_index (0));
      on_army_selected ();
    }
}

void SelectArmyDialog::preselect_army (guint32 army_type)
{
  for (guint32 i = 0; i < selectable.size (); i++)
    if (selectable[i]->getId () == army_type)
      toggles_table->select_child (*toggles_table->get_child_at_index (i));
}

void SelectArmyDialog::run()
{
  dialog->show_all();
  clear_button->set_visible (d_clear);
  int response = dialog->run();

  if (response != Gtk::RESPONSE_ACCEPT)
    selected_army = NULL;
}

void SelectArmyDialog::fill_in_army_toggles(Mode mode)
{
  const Armysetlist* al = Armysetlist::getInstance();

  if (!player)
    player = Playerlist::getInstance()->getNeutral();
  int armyset = player->getArmyset();

  // fill in selectable armies
  selectable.clear();
  Armyset *as = al->get(armyset);
  for (Armyset::iterator j = as->begin(); j != as->end(); ++j)
    {
      const ArmyProto *a = al->getArmy(armyset, (*j)->getId());
      if (mode == SELECT_NORMAL_WITH_HERO)
        selectable.push_back(a);
      else if (mode == SELECT_NORMAL && a->isHero () == false)
        selectable.push_back(a);
      else if (mode == SELECT_RUIN_DEFENDER && a->getDefendsRuins())
        selectable.push_back(a);
      else if (mode == SELECT_REWARDABLE_ARMY && a->getAwardable())
        selectable.push_back(a);
    }

  // fill in army options
  army_toggles.clear();
  toggles_table->foreach(sigc::mem_fun(toggles_table, &Gtk::Container::remove));
  guint32 fs = FontSize::getInstance ()->get_height ();
  for (unsigned int i = 0; i < selectable.size(); ++i)
    {

      Glib::RefPtr<Gdk::Pixbuf> pixbuf
        = ImageCache::getInstance()->getArmyPic(armyset,
                                                selectable[i]->getId(),
                                                player, NULL, false,
                                                fs)->to_pixbuf();
      Gtk::Image *toggle = manage(new Gtk::Image (pixbuf));
      army_toggles.push_back(toggle);
      toggles_table->add (*toggle);
      toggle->show_all();
    }

  ignore_toggles = false;
}

void SelectArmyDialog::fill_in_army_info()
{
  Glib::ustring s1, s2;

  if (!selected_army)
    {
      s1 = _("No army");
      s1 += "\n\n\n";
      s2 = "\n\n\n";
    }
  else
    {
      const ArmyProto *a = selected_army;

      // fill in first column
      s1 += a->getName();
      s1 += "\n";
      s1 += String::ucompose(_("Strength: %1"), a->getStrength());
      s1 += "\n";
      s1 += String::ucompose(_("Moves: %1"), a->getMaxMoves());
      s1 += "\n";
      s1 += String::ucompose(_("Sight: %1"), a->getSight());

      // fill in second column
      s2 += "\n";
      s2 += String::ucompose(_("Upkeep: %1"), a->getUpkeep());
      s2 += "\n";
      s2 += String::ucompose(_("Bonus: %1"), a->getArmyBonusDescription());
      s2 += "\n";
      Glib::ustring bonus = a->getMoveBonusDescription();
      if (bonus == "")
        bonus = _("None");
      s2 += String::ucompose(_("Move Bonus: %1"), bonus);
    }

  army_info_label1->set_text (s1);
  army_info_label2->set_text (s2);
}

void SelectArmyDialog::on_army_selected ()
{
  int idx = toggles_table->get_selected_children().front ()->get_index ();
  selected_army = selectable[idx];
  fill_in_army_info ();
}
