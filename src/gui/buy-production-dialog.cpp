//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007-2009, 2011, 2014, 2015, 2017, 2020 Ben Asselstine
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

#include "buy-production-dialog.h"

#include "input-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "army.h"
#include "city.h"
#include "ImageCache.h"
#include "armysetlist.h"
#include "playerlist.h"
#include "shield.h"
#include "font-size.h"

#define method(x) sigc::mem_fun(*this, &BuyProductionDialog::x)

BuyProductionDialog::BuyProductionDialog(Gtk::Window &parent, City *c)
 : LwDialog(parent, "buy-production-dialog.ui")
{
    army_info_tip = NULL;
    city = c;
    selected_army = NO_ARMY_SELECTED;
    
    xml->get_widget("unit_label", unit_label);
    xml->get_widget("cost_label", cost_label);
    xml->get_widget("time_label", time_label);
    xml->get_widget("moves_label", moves_label);
    xml->get_widget("strength_label", strength_label);
    xml->get_widget("upkeep_label", upkeep_label);
    xml->get_widget("buy_button", buy_button);

    unit_label->set_text ("--");
    cost_label->set_text ("--");
    time_label->set_text ("--");
    moves_label->set_text ("--");
    strength_label->set_text ("--");
    upkeep_label->set_text ("--");

    Gtk::FlowBox *toggles_table; 
    xml->get_widget("production_toggles_table", toggles_table);
    
    const Armysetlist* al = Armysetlist::getInstance();

    Player *p = Playerlist::getInstance()->getActiveplayer();
    // fill in purchasable armies
    Armyset *as = al->get(p->getArmyset());
    for (Armyset::iterator j = as->begin(); j != as->end(); ++j)
      {
        const ArmyProto *a = al->getArmy (p->getArmyset(), (*j)->getId());
        if (a->getNewProductionCost() > 0)
          purchasables.push_back(a);
      }

    // fill in production options
    //const int no_columns = 4;
    for (unsigned int i = 0; i < purchasables.size(); ++i)
    {
	Gtk::ToggleButton *toggle = manage(new Gtk::ToggleButton);
	Gtk::Image *image = new Gtk::Image();
	toggle->add(*manage(image));
	production_toggles.push_back(toggle);
        fill_pixbuf (i);

	//int x = i % no_columns;
	//int y = i / no_columns;
	//toggles_table->attach(*toggle, x, y, 1 , 1);
	toggles_table->add (*toggle);
	toggle->show_all();

	toggle->signal_toggled().connect(sigc::bind(method(on_production_toggled), toggle));
	toggle->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
	toggle->signal_button_press_event().connect(sigc::bind(method(on_production_button_event), toggle), false);
	
	toggle->signal_button_release_event().connect(sigc::bind(method(on_production_button_event), toggle), false);
    }

    ignore_toggles = false;
    production_toggles[0]->set_active(true);
}

void
BuyProductionDialog::fill_pixbuf (int i)
{
  Player *p = Playerlist::getInstance()->getActiveplayer();
  ImageCache *gc = ImageCache::getInstance();
  bool greyed_out = false;
  guint32 selected = Shield::NEUTRAL;
  if (city->hasProductionBase(purchasables[i]) == true)
    greyed_out = true;
  if ((int)purchasables[i]->getNewProductionCost() >
      city->getOwner()->getGold())
    greyed_out = true;
  if (production_toggles[i]->get_active())
    selected = p->getId();
  Glib::RefPtr<Gdk::Pixbuf> pix
    = gc->getCircledArmyPic(p->getArmyset(), purchasables[i]->getId(), p, NULL,
                            greyed_out, selected, true,
                            FontSize::getInstance ()->get_height ())->to_pixbuf();
  Gtk::Image *image =
    dynamic_cast<Gtk::Image*>(production_toggles[i]->get_child());
  image->property_pixbuf() = pix;
}

BuyProductionDialog::~BuyProductionDialog()
{
  if (army_info_tip)
    delete army_info_tip;
}

void BuyProductionDialog::run()
{
    dialog->show();
    int response = dialog->run();

    if (response != Gtk::RESPONSE_ACCEPT)
	selected_army = NO_ARMY_SELECTED;
}

void BuyProductionDialog::on_production_toggled(Gtk::ToggleButton *toggle)
{
  int i = lookup_slot (toggle);
  if (toggle->get_active () == false)
    {
      if (i != -1)
        fill_pixbuf (i);
    }

    if (ignore_toggles)
	return;
    
    ignore_toggles = true;
    if (i == -1)
      selected_army = NO_ARMY_SELECTED;
    else
      selected_army = i;
    for (unsigned int j = 0; j < production_toggles.size(); ++j)
      production_toggles[j]->set_active(toggle == production_toggles[j]);
    ignore_toggles = false;
    fill_pixbuf (selected_army);
    fill_in_production_info();
    set_buy_button_state();
}

void BuyProductionDialog::fill_in_production_info()
{
  const ArmyProto *a = army_id_to_army();
  if (a)
    {
      unit_label->set_text(a->getName());
      cost_label->set_text(String::ucompose("%1", a->getNewProductionCost()));
      time_label->set_text(String::ucompose ("%1", a->getProduction()));
      moves_label->set_text(String::ucompose ("%1", a->getMaxMoves()));
      strength_label->set_text(String::ucompose ("%1", a->getStrength()));
      upkeep_label->set_text(String::ucompose ("%1", a->getUpkeep()));
    }
}

void BuyProductionDialog::set_buy_button_state()
{
    bool can_buy = true;
    
    if (selected_army == NO_ARMY_SELECTED)
	can_buy = false;
    else
    {
	int gold = city->getOwner()->getGold();
	const ArmyProto *a = army_id_to_army();
	
	if (int(a->getNewProductionCost()) > gold ||
	    city->hasProductionBase(selected_army))
	    can_buy = false;
    }
    
    buy_button->set_sensitive(can_buy);
}

const ArmyProto *BuyProductionDialog::army_id_to_army()
{
    return purchasables[selected_army];
}

int BuyProductionDialog::lookup_slot (Gtk::ToggleButton *toggle)
{
  int slot = -1;
  for (unsigned int i = 0; i < production_toggles.size(); ++i)
    {
      if (toggle == production_toggles[i])
        slot = i;
    }
  return slot;
}

bool BuyProductionDialog::on_production_button_event(GdkEventButton *e, Gtk::ToggleButton *toggle)
{
    MouseButtonEvent event = to_input_event(e);
    if (event.button == MouseButtonEvent::RIGHT_BUTTON
	&& event.state == MouseButtonEvent::PRESSED) {
	int slot = lookup_slot (toggle);
	assert(slot != -1);

	const ArmyProto *army = purchasables[slot];

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
