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

SelectArmyDialog::SelectArmyDialog(Gtk::Window &parent, bool clear, Player *p,
                                   bool hero_too, bool defends_ruins,
                                   bool awardable)
 : LwEditorDialog(parent, "select-army-dialog.ui")
{
  d_clear = clear;
  army_info_tip = NULL;
  d_hero_too = hero_too;
  d_defends_ruins = defends_ruins;
  player = p;
  d_awardable = awardable;
  selected_army = 0;

  xml->get_widget("army_info_label1", army_info_label1);
  xml->get_widget("army_info_label2", army_info_label2);
  xml->get_widget("select_button", select_button);
  xml->get_widget("clear_button", clear_button);

  xml->get_widget("army_toggles_table", toggles_table);

  fill_in_army_toggles();
}

void SelectArmyDialog::run()
{
  dialog->show_all();
  clear_button->set_visible (d_clear);
  int response = dialog->run();

  if (response != Gtk::RESPONSE_ACCEPT)
    selected_army = 0;
}

void SelectArmyDialog::on_army_toggled(Gtk::ToggleButton *toggle)
{
  if (ignore_toggles)
    return;

  selected_army = 0;
  ignore_toggles = true;
  for (unsigned int i = 0; i < army_toggles.size(); ++i) {
    if (toggle == army_toggles[i])
      selected_army = selectable[i];

    army_toggles[i]->set_active(toggle == army_toggles[i]);
  }
  ignore_toggles = false;

  fill_in_army_info();
  set_select_button_state();
}

void SelectArmyDialog::fill_in_army_toggles()
{
  const Armysetlist* al = Armysetlist::getInstance();

  if (!player)
    player = Playerlist::getInstance()->getNeutral();
  int armyset = player->getArmyset();
  bool pushed_back = false;

  // fill in selectable armies
  selectable.clear();
  Armyset *as = al->get(armyset);
  for (Armyset::iterator j = as->begin(); j != as->end(); ++j)
    {
      const ArmyProto *a = al->getArmy(armyset, (*j)->getId());
      if (a->isHero() && d_hero_too == false)
        continue;
      if ((d_defends_ruins && a->getDefendsRuins()) ||
          (!d_defends_ruins && !d_awardable))
        {
          pushed_back = true;
          selectable.push_back(a);
        }
      if (((d_awardable && a->getAwardable()) ||
           (!d_defends_ruins && !d_awardable)) && !pushed_back)
        selectable.push_back(a);
      pushed_back = false;
    }

  // fill in army options
  army_toggles.clear();
  toggles_table->foreach(sigc::mem_fun(toggles_table, &Gtk::Container::remove));
  toggles_table->insert_row (0);
  toggles_table->insert_column (0);
  const int no_columns = 6;
  guint32 fs = FontSize::getInstance ()->get_height ();
  for (unsigned int i = 0; i < selectable.size(); ++i)
    {
      Gtk::ToggleButton *toggle = manage(new Gtk::ToggleButton);

      Glib::RefPtr<Gdk::Pixbuf> pixbuf
        = ImageCache::getInstance()->getArmyPic(armyset,
                                                selectable[i]->getId(),
                                                player, NULL, false,
                                                fs)->to_pixbuf();

      toggle->add(*manage(new Gtk::Image(pixbuf)));
      army_toggles.push_back(toggle);
      int x = i % no_columns;
      int y = i / no_columns;
      toggles_table->attach(*toggle, x, y, 1 , 1);
      toggle->show_all();

      toggle->signal_toggled().connect(sigc::bind(method(on_army_toggled), toggle));
      toggle->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
      toggle->signal_button_press_event().connect
        (sigc::bind(method(on_army_button_event), toggle), false);

      toggle->signal_button_release_event().connect
        (sigc::bind(method(on_army_button_event), toggle), false);
    }

  ignore_toggles = false;
  if (!army_toggles.empty())
    army_toggles[0]->set_active(true);
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

      // fill in second column
      s2 += "\n";
      s2 += String::ucompose(_("Upkeep: %1"), a->getUpkeep());
    }

  army_info_label1->set_markup("<i>" + s1 + "</i>");
  army_info_label2->set_markup("<i>" + s2 + "</i>");
}

void SelectArmyDialog::set_select_button_state()
{
  select_button->set_sensitive(selected_army);
}

bool SelectArmyDialog::on_army_button_event(GdkEventButton *e, Gtk::ToggleButton *toggle)
{
  MouseButtonEvent event = to_input_event(e);
  if (event.button == MouseButtonEvent::RIGHT_BUTTON
      && event.state == MouseButtonEvent::PRESSED) {
    int slot = -1;
    for (unsigned int i = 0; i < army_toggles.size(); ++i) {
      if (toggle == army_toggles[i])
	slot = i;
    }
    assert(slot != -1);

    const ArmyProto *army = selectable[slot];

    if (army)
      {
	if (army_info_tip)
	  delete army_info_tip;
	army_info_tip = new ArmyInfoTip(toggle, army);
      }
    return true;
  }
  else if (event.button == MouseButtonEvent::RIGHT_BUTTON
	   && event.state == MouseButtonEvent::RELEASED) {
      {
	if (army_info_tip)
	  {
	    delete army_info_tip;
	    army_info_tip = NULL;
	  }
      }
    return true;
  }

  return false;
}
