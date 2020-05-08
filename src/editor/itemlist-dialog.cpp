//  Copyright (C) 2008, 2009, 2011, 2014, 2015, 2020 Ben Asselstine
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

#include <iostream>
#include <iomanip>
#include <assert.h>

#include <sigc++/functors/mem_fun.h>
#include <gtkmm.h>

#include "itemlist-dialog.h"
#include "defs.h"
#include "Configuration.h"
#include "Itemlist.h"
#include "Tile.h"
#include "playerlist.h"
#include "armysetlist.h"
#include "ucompose.hpp"
#include "army-chooser-button.h"
#define method(x) sigc::mem_fun(*this, &ItemlistDialog::x)

ItemlistDialog::ItemlistDialog(Gtk::Window &parent)
 : LwEditorDialog(parent, "itemlist-dialog.ui")
{
  d_changed = false;
  d_itemlist = Itemlist::getInstance();

  kill_army_type_button =
    new ArmyChooserButton (parent, xml, "kill_army_type_button",
                           Playerlist::getInstance()->getNeutral (),
                           SelectArmyDialog::SELECT_RUIN_DEFENDER);

  summon_army_type_button =
    new ArmyChooserButton (parent, xml, "summon_army_type_button",
                           Playerlist::getInstance()->getNeutral (),
                           SelectArmyDialog::SELECT_NORMAL);

  defender_army_type_button =
    new ArmyChooserButton (parent, xml, "defender_army_type_button",
                           Playerlist::getInstance()->getNeutral (),
                           SelectArmyDialog::SELECT_NORMAL);
  load_widgets ();

  items_list = Gtk::ListStore::create(items_columns);
  items_treeview->set_model(items_list);
  items_treeview->append_column("", items_columns.name);
  items_treeview->set_headers_visible(false);

  for (Itemlist::iterator iter = d_itemlist->begin();
       iter != d_itemlist->end(); ++iter)
    addItemProto((*iter).second);

  d_item = NULL;
  guint32 max = d_itemlist->size();
  if (max)
    {
      Gtk::TreeModel::Row row;
      row = items_treeview->get_model()->children()[0];
      if(row)
        items_treeview->get_selection()->select(row);
    }
  update_item_panel();
  update_itemlist_buttons();
}

ItemlistDialog::~ItemlistDialog()
{
  delete kill_army_type_button;
  delete summon_army_type_button;
  delete defender_army_type_button;
}

void ItemlistDialog::load_widgets ()
{
  xml->get_widget("name_entry", name_entry);
  xml->get_widget("items_treeview", items_treeview);
  xml->get_widget("add_item_button", add_item_button);
  xml->get_widget("remove_item_button", remove_item_button);
  xml->get_widget("item_vbox", item_vbox);
  xml->get_widget("building_type_to_summon_on_combobox", 
                  building_type_to_summon_on_combobox);
  xml->get_widget("disease_city_switch", disease_city_switch);
  xml->get_widget("disease_armies_percent_spinbutton", 
                  disease_armies_percent_spinbutton);

  xml->get_widget("raise_defenders_switch", raise_defenders_switch);
  xml->get_widget("num_defenders_spinbutton", num_defenders_spinbutton);
  xml->get_widget("persuade_neutral_city_switch", 
                  persuade_neutral_city_switch);
  xml->get_widget("teleport_to_city_switch", 
                  teleport_to_city_switch);
  xml->get_widget("add1str_switch", add1str_switch);
  xml->get_widget("add2str_switch", add2str_switch);
  xml->get_widget("add3str_switch", add3str_switch);
  xml->get_widget("add1stack_switch", add1stack_switch);
  xml->get_widget("add2stack_switch", add2stack_switch);
  xml->get_widget("add3stack_switch", add3stack_switch);
  xml->get_widget("flystack_switch", flystack_switch);
  xml->get_widget("doublemovestack_switch", doublemovestack_switch);
  xml->get_widget("add2goldpercity_switch", add2goldpercity_switch);
  xml->get_widget("add3goldpercity_switch", add3goldpercity_switch);
  xml->get_widget("add4goldpercity_switch", add4goldpercity_switch);
  xml->get_widget("add5goldpercity_switch", add5goldpercity_switch);
  xml->get_widget("steals_gold_switch", steals_gold_switch);
  xml->get_widget("pickup_bags_switch", pickup_bags_switch);
  xml->get_widget("add_mp_switch", add_mp_switch);
  xml->get_widget("sinks_ships_switch", sinks_ships_switch);
  xml->get_widget("banish_worms_switch", banish_worms_switch);
  xml->get_widget("burn_bridge_switch", burn_bridge_switch);
  xml->get_widget("capture_keeper_switch", capture_keeper_switch);
  xml->get_widget("summon_monster_switch", summon_monster_switch);
  xml->get_widget("uses_spinbutton", uses_spinbutton);
  xml->get_widget("steal_percent_spinbutton", steal_percent_spinbutton);
  xml->get_widget("add_mp_spinbutton", add_mp_spinbutton);
  xml->get_widget("plantable_switch", plantable_switch);
}

void ItemlistDialog::connect_signals ()
{
  connections.push_back(items_treeview->get_selection()->signal_changed().connect (method(on_item_selected)));
  connections.push_back (name_entry->signal_changed().connect (method(on_name_changed)));
  connections.push_back (add_item_button->signal_clicked().connect (method(on_add_item_clicked)));
  connections.push_back (remove_item_button->signal_clicked().connect (method(on_remove_item_clicked)));
  connections.push_back (kill_army_type_button->army_selected.connect(method(on_kill_army_type_selected)));
  connections.push_back (summon_army_type_button->army_selected.connect(method(on_summon_army_type_selected)));
  connections.push_back (disease_city_switch->property_active().signal_changed().connect(method(on_disease_city_toggled)));
  connections.push_back (disease_armies_percent_spinbutton->signal_changed().connect(method(on_disease_armies_percent_changed)));
  connections.push_back (disease_armies_percent_spinbutton->signal_insert_text().connect (sigc::hide(sigc::hide(method(on_disease_armies_percent_text_changed)))));
  connections.push_back (raise_defenders_switch->property_active().signal_changed().connect (method(on_raise_defenders_toggled)));
  connections.push_back (defender_army_type_button->army_selected.connect (method(on_defender_type_selected)));
  connections.push_back (num_defenders_spinbutton->signal_changed().connect (method(on_num_defenders_changed)));
  connections.push_back (num_defenders_spinbutton->signal_insert_text().connect
    (sigc::hide(sigc::hide(method(on_num_defenders_text_changed)))));
  connections.push_back (persuade_neutral_city_switch->property_active().signal_changed().connect (method(on_persuade_neutral_city_toggled)));
  connections.push_back (teleport_to_city_switch->property_active().signal_changed().connect (method(on_teleport_to_city_toggled)));
  connections.push_back (add1str_switch->property_active().signal_changed().connect(method(on_add1str_toggled)));
  connections.push_back (add2str_switch->property_active().signal_changed().connect(method(on_add2str_toggled)));
  connections.push_back (add3str_switch->property_active().signal_changed().connect(method(on_add3str_toggled)));
  connections.push_back (add1stack_switch->property_active().signal_changed().connect(method(on_add1stack_toggled)));
  connections.push_back (add2stack_switch->property_active().signal_changed().connect(method(on_add2stack_toggled)));
  connections.push_back (add3stack_switch->property_active().signal_changed().connect(method(on_add3stack_toggled)));
  connections.push_back (flystack_switch->property_active().signal_changed().connect(method(on_flystack_toggled)));
  connections.push_back (doublemovestack_switch->property_active().signal_changed().connect(method(on_doublemovestack_toggled)));
  connections.push_back (add2goldpercity_switch->property_active().signal_changed().connect (method(on_add2goldpercity_toggled)));
  connections.push_back (add3goldpercity_switch->property_active().signal_changed().connect (method(on_add3goldpercity_toggled)));
  connections.push_back (add4goldpercity_switch->property_active().signal_changed().connect (method(on_add4goldpercity_toggled)));
  connections.push_back (add5goldpercity_switch->property_active().signal_changed().connect (method(on_add5goldpercity_toggled)));
  connections.push_back (steals_gold_switch->property_active().signal_changed().connect (method(on_steals_gold_toggled)));
  connections.push_back (pickup_bags_switch->property_active().signal_changed().connect(method(on_pickup_bags_toggled)));
  connections.push_back (add_mp_switch->property_active().signal_changed().connect(method(on_add_mp_toggled)));
  connections.push_back (sinks_ships_switch->property_active().signal_changed().connect(method(on_sinks_ships_toggled)));
  connections.push_back (banish_worms_switch->property_active().signal_changed().connect(method(on_banish_worms_toggled)));
  connections.push_back (burn_bridge_switch->property_active().signal_changed().connect(method(on_burn_bridge_toggled)));
  connections.push_back (capture_keeper_switch->property_active().signal_changed().connect(method(on_capture_keeper_toggled)));
  connections.push_back (summon_monster_switch->property_active().signal_changed().connect (method(on_summon_monster_toggled)));
  connections.push_back (uses_spinbutton->signal_changed().connect(method(on_uses_changed)));
  connections.push_back (steal_percent_spinbutton->signal_changed().connect
    (method(on_steal_percent_changed)));
  connections.push_back (steal_percent_spinbutton->signal_insert_text().connect
    (sigc::hide(sigc::hide(method(on_steal_percent_text_changed)))));
  connections.push_back (add_mp_spinbutton->signal_changed().connect (method(on_add_mp_changed)));
  connections.push_back (add_mp_spinbutton->signal_insert_text().connect
    (sigc::hide(sigc::hide(method(on_add_mp_text_changed)))));
  connections.push_back (building_type_to_summon_on_combobox->signal_changed ().connect (method (on_building_type_to_summon_on_changed)));
  connections.push_back (plantable_switch->property_active().signal_changed().connect (method(on_plantable_toggled)));
}

void ItemlistDialog::disconnect_signals ()
{
  for (auto c : connections)
    c.disconnect ();
}

void
ItemlistDialog::update_itemlist_buttons()
{
  if (!items_treeview->get_selection()->get_selected())
    remove_item_button->set_sensitive(false);
  else
    remove_item_button->set_sensitive(true);
  if (d_itemlist == NULL)
    add_item_button->set_sensitive(false);
  else
    add_item_button->set_sensitive(true);
}

void
ItemlistDialog::update_item_panel()
{
  //if nothing selected in the treeview, then we don't show anything in
  //the item panel
  if (items_treeview->get_selection()->get_selected() == 0)
    {
      //clear all values
      name_entry->set_text("");
      item_vbox->set_sensitive(false);
      return;
    }
  item_vbox->set_sensitive(true);
  Glib::RefPtr<Gtk::TreeSelection> selection = items_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      // Row selected
      Gtk::TreeModel::Row row = *iterrow;

      ItemProto *a = row[items_columns.item];
      d_item = a;
      fill_item_info(a);
    }
}

void ItemlistDialog::addItemProto(ItemProto *itemproto)
{
  Gtk::TreeIter i = items_list->append();
  (*i)[items_columns.name] = itemproto->getName();
  (*i)[items_columns.item] = itemproto;
}

void ItemlistDialog::on_item_selected()
{
  update_item_panel();
  update_itemlist_buttons();
}

void ItemlistDialog::fill_item_info(ItemProto *item)
{
  disconnect_signals ();
  name_entry->set_text(item->getName());
  add1str_switch->set_active(item->getBonus(ItemProto::ADD1STR));
  add2str_switch->set_active(item->getBonus(ItemProto::ADD2STR));
  add3str_switch->set_active(item->getBonus(ItemProto::ADD3STR));
  add1stack_switch->set_active(item->getBonus(ItemProto::ADD1STACK));
  add2stack_switch->set_active(item->getBonus(ItemProto::ADD2STACK));
  add3stack_switch->set_active(item->getBonus(ItemProto::ADD3STACK));
  flystack_switch->set_active(item->getBonus(ItemProto::FLYSTACK));
  doublemovestack_switch->set_active 
    (item->getBonus(ItemProto::DOUBLEMOVESTACK));
  add2goldpercity_switch->set_active
    (item->getBonus(ItemProto::ADD2GOLDPERCITY));
  add3goldpercity_switch->set_active
    (item->getBonus(ItemProto::ADD3GOLDPERCITY));
  add4goldpercity_switch->set_active
    (item->getBonus(ItemProto::ADD4GOLDPERCITY));
  add5goldpercity_switch->set_active
    (item->getBonus(ItemProto::ADD5GOLDPERCITY));
  steals_gold_switch->set_active (item->getBonus(ItemProto::STEAL_GOLD));
  pickup_bags_switch->set_active (item->getBonus(ItemProto::PICK_UP_BAGS));
  add_mp_switch->set_active (item->getBonus(ItemProto::ADD_2MP_STACK));
  sinks_ships_switch->set_active (item->getBonus(ItemProto::SINK_SHIPS));
  banish_worms_switch->set_active (item->getBonus(ItemProto::BANISH_WORMS));
  burn_bridge_switch->set_active (item->getBonus(ItemProto::BURN_BRIDGE));
  capture_keeper_switch->set_active 
    (item->getBonus(ItemProto::CAPTURE_KEEPER));
  uses_spinbutton->set_value(double(item->getNumberOfUsesLeft()));
  steal_percent_spinbutton->set_value(item->getPercentGoldToSteal());
  steal_percent_spinbutton->set_sensitive
    (steals_gold_switch->get_active());
  add_mp_spinbutton->set_value(item->getMovementPointsToAdd());
  add_mp_spinbutton->set_sensitive (add_mp_switch->get_active());
  summon_monster_switch->set_active 
    (item->getBonus(ItemProto::SUMMON_MONSTER));
  building_type_to_summon_on_combobox->set_active(item->getBuildingTypeToSummonOn());
  building_type_to_summon_on_combobox->set_sensitive(summon_monster_switch->get_active());
  disease_city_switch->set_active(item->getBonus(ItemProto::DISEASE_CITY));
  disease_armies_percent_spinbutton->set_sensitive(disease_city_switch->get_active());
  disease_armies_percent_spinbutton->set_value(item->getPercentArmiesToKill());

  if (item->hasArmyTypeToKill ())
    kill_army_type_button->select (item->getArmyTypeToKill ());
  else
    kill_army_type_button->clear_selected_army ();
  kill_army_type_button->set_sensitive(banish_worms_switch->get_active());

  if (item->hasArmyTypeToSummon ())
    summon_army_type_button->select (item->getArmyTypeToSummon ());
  else
    summon_army_type_button->clear_selected_army ();

  summon_army_type_button->set_sensitive(summon_monster_switch->get_active());

  raise_defenders_switch->set_active
    (item->getBonus(ItemProto::RAISE_DEFENDERS));
  num_defenders_spinbutton->set_sensitive
    (raise_defenders_switch->get_active());
  num_defenders_spinbutton->set_value(item->getNumberOfArmiesToRaise());

  if (item->hasArmyTypeToRaise ())
    defender_army_type_button->select (item->getArmyTypeToRaise());
  else
    defender_army_type_button->clear_selected_army ();
  defender_army_type_button->set_sensitive(raise_defenders_switch->get_active());

  persuade_neutral_city_switch->set_active 
    (item->getBonus(ItemProto::PERSUADE_NEUTRALS));
  teleport_to_city_switch->set_active 
    (item->getBonus(ItemProto::TELEPORT_TO_CITY));

  plantable_switch->set_active (item->getBonus(ItemProto::PLANT_TO_VECTOR));
  connect_signals ();
}

void ItemlistDialog::on_name_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = items_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ItemProto *a = row[items_columns.item];
      a->setName(name_entry->get_text());
      row[items_columns.name] = name_entry->get_text();
      d_changed = true;
    }
}

void ItemlistDialog::on_add_item_clicked()
{
  //add a new empty item to the itemlist
  ItemProto *a = new ItemProto(_("Untitled"));
  d_itemlist->add(a);
  //add it to the treeview
  Gtk::TreeIter i = items_list->append();
  a->setName(_("Untitled"));
  (*i)[items_columns.name] = a->getName();
  (*i)[items_columns.item] = a;
  d_changed = true;
}

void ItemlistDialog::on_remove_item_clicked()
{
  //erase the selected row from the treeview
  //remove the item from the itemlist
  Glib::RefPtr<Gtk::TreeSelection> selection = items_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ItemProto *a = row[items_columns.item];
      items_list->erase(iterrow);
      d_itemlist->remove(a);
      d_changed = true;
    }
}

void ItemlistDialog::on_switch_toggled(Gtk::Switch *sw, ItemProto::Bonus bonus)
{
  Glib::RefPtr<Gtk::TreeSelection> selection = items_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      // Row selected
      Gtk::TreeModel::Row row = *iterrow;

      d_item = row[items_columns.item];
    }
  else
    return;
  if (sw->get_active())
    d_item->addBonus(bonus);
  else
    d_item->removeBonus(bonus);
  d_changed = true;
}

void ItemlistDialog::on_add1str_toggled()
{
  on_switch_toggled(add1str_switch, ItemProto::ADD1STR);
}

void ItemlistDialog::on_add2str_toggled()
{
  on_switch_toggled(add2str_switch, ItemProto::ADD2STR);
}

void ItemlistDialog::on_add3str_toggled()
{
  on_switch_toggled(add3str_switch, ItemProto::ADD3STR);
}

void ItemlistDialog::on_add1stack_toggled()
{
  on_switch_toggled(add1stack_switch, ItemProto::ADD1STACK);
}

void ItemlistDialog::on_add2stack_toggled()
{
  on_switch_toggled(add2stack_switch, ItemProto::ADD2STACK);
}

void ItemlistDialog::on_add3stack_toggled()
{
  on_switch_toggled(add3stack_switch, ItemProto::ADD3STACK);
}

void ItemlistDialog::on_flystack_toggled()
{
  on_switch_toggled(flystack_switch, ItemProto::FLYSTACK);
}

void ItemlistDialog::on_doublemovestack_toggled()
{
  on_switch_toggled(doublemovestack_switch, ItemProto::DOUBLEMOVESTACK);
}

void ItemlistDialog::on_add2goldpercity_toggled()
{
  on_switch_toggled(add2goldpercity_switch, ItemProto::ADD2GOLDPERCITY);
}

void ItemlistDialog::on_add3goldpercity_toggled()
{
  on_switch_toggled(add2goldpercity_switch, ItemProto::ADD3GOLDPERCITY);
}

void ItemlistDialog::on_add4goldpercity_toggled()
{
  on_switch_toggled(add4goldpercity_switch, ItemProto::ADD4GOLDPERCITY);
}

void ItemlistDialog::on_add5goldpercity_toggled()
{
  on_switch_toggled(add5goldpercity_switch, ItemProto::ADD5GOLDPERCITY);
}

void ItemlistDialog::on_steals_gold_toggled()
{
  on_switch_toggled(steals_gold_switch, ItemProto::STEAL_GOLD);
  steal_percent_spinbutton->set_sensitive
    (steals_gold_switch->get_active());
  if (steals_gold_switch->get_active () == false)
    steal_percent_spinbutton->set_value (1);
}

void ItemlistDialog::on_pickup_bags_toggled()
{
  on_switch_toggled(pickup_bags_switch, ItemProto::PICK_UP_BAGS);
}

void ItemlistDialog::on_add_mp_toggled()
{
  on_switch_toggled(add_mp_switch, ItemProto::ADD_2MP_STACK);
  add_mp_spinbutton->set_sensitive (add_mp_switch->get_active());
  if (add_mp_switch->get_active () == false)
    add_mp_spinbutton->set_value (1);
}

void ItemlistDialog::on_sinks_ships_toggled()
{
  on_switch_toggled(sinks_ships_switch, ItemProto::SINK_SHIPS);
}
	
void ItemlistDialog::on_banish_worms_toggled()
{
  on_switch_toggled(banish_worms_switch, ItemProto::BANISH_WORMS);
  kill_army_type_button->set_sensitive(banish_worms_switch->get_active());
  if (banish_worms_switch->get_active() == false)
    {
      d_item->clearArmyTypeToKill ();
      kill_army_type_button->clear_selected_army ();
    }
}

void ItemlistDialog::on_burn_bridge_toggled()
{
  on_switch_toggled(burn_bridge_switch, ItemProto::BURN_BRIDGE);
}

void ItemlistDialog::on_uses_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = items_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      // Row selected
      Gtk::TreeModel::Row row = *iterrow;
      d_item = row[items_columns.item];
  
      d_item->setNumberOfUsesLeft(int(uses_spinbutton->get_value()));
      d_changed = true;
    }
  else
    return;
}

void ItemlistDialog::on_kill_army_type_selected (const ArmyProto *a)
{
    if (a)
      d_item->setArmyTypeToKill(a->getId());
    else
      {
        d_item->clearArmyTypeToKill ();
        banish_worms_switch->property_active () = false;
      }
    d_changed = true;
}

void ItemlistDialog::on_capture_keeper_toggled()
{
  on_switch_toggled(capture_keeper_switch, ItemProto::CAPTURE_KEEPER);
}
    
void ItemlistDialog::on_summon_monster_toggled()
{
  on_switch_toggled(summon_monster_switch, ItemProto::SUMMON_MONSTER);
  summon_army_type_button->set_sensitive
    (summon_monster_switch->get_active());
  building_type_to_summon_on_combobox->set_sensitive
    (summon_monster_switch->get_active());
  if (summon_monster_switch->get_active() == false)
    {
      d_item->clearArmyTypeToSummon();
      building_type_to_summon_on_combobox->set_active (0);
      summon_army_type_button->clear_selected_army ();
    }
}
    
void ItemlistDialog::on_summon_army_type_selected(const ArmyProto *a)
{
  if (a)
    d_item->setArmyTypeToSummon(a->getId());
  else
    {
      d_item->clearArmyTypeToSummon ();
      building_type_to_summon_on_combobox->set_active (0);
      summon_monster_switch->property_active () = false;
    }

  d_changed = true;
}

void ItemlistDialog::on_disease_city_toggled()
{
  on_switch_toggled(disease_city_switch, ItemProto::DISEASE_CITY);
  disease_armies_percent_spinbutton->set_sensitive
    (disease_city_switch->get_active());
  if (disease_city_switch->get_active () == false)
    disease_armies_percent_spinbutton->set_value (1);
}

void ItemlistDialog::on_persuade_neutral_city_toggled()
{
  on_switch_toggled(persuade_neutral_city_switch, 
                         ItemProto::PERSUADE_NEUTRALS);
}

void ItemlistDialog::on_teleport_to_city_toggled()
{
  on_switch_toggled(teleport_to_city_switch, 
                         ItemProto::TELEPORT_TO_CITY);
}

void ItemlistDialog::on_raise_defenders_toggled()
{
  on_switch_toggled(raise_defenders_switch, ItemProto::RAISE_DEFENDERS);
  num_defenders_spinbutton->set_sensitive
    (raise_defenders_switch->get_active());
  defender_army_type_button->set_sensitive
    (raise_defenders_switch->get_active());
  if (raise_defenders_switch->get_active() == false)
    {
      d_item->clearArmyTypeToRaise();
      defender_army_type_button->clear_selected_army ();
      num_defenders_spinbutton->set_value (1);
    }
}

void ItemlistDialog::on_steal_percent_changed()
{
  if (d_item)
    {
      d_item->setPercentGoldToSteal(steal_percent_spinbutton->get_value());
      d_changed = true;
    }
}

void ItemlistDialog::on_steal_percent_text_changed()
{
  steal_percent_spinbutton->set_value(atoi(steal_percent_spinbutton->get_text().c_str()));
  on_steal_percent_changed();
}

void ItemlistDialog::on_disease_armies_percent_changed()
{
  if (d_item)
    {
      d_item->setPercentArmiesToKill
        (disease_armies_percent_spinbutton->get_value());
      d_changed = true;
    }
}

void ItemlistDialog::on_disease_armies_percent_text_changed()
{
  disease_armies_percent_spinbutton->set_value(atoi(disease_armies_percent_spinbutton->get_text().c_str()));
  on_disease_armies_percent_changed();
}

void ItemlistDialog::on_add_mp_changed()
{
  if (d_item)
    {
      d_item->setMovementPointsToAdd (add_mp_spinbutton->get_value());
      d_changed = true;
    }
}

void ItemlistDialog::on_add_mp_text_changed()
{
  add_mp_spinbutton->set_value(atoi(add_mp_spinbutton->get_text().c_str()));
  on_add_mp_changed();
}

void ItemlistDialog::on_num_defenders_changed()
{
  if (d_item)
    {
      d_item->setNumberOfArmiesToRaise(num_defenders_spinbutton->get_value());
      d_changed = true;
    }
}

void ItemlistDialog::on_num_defenders_text_changed()
{
  num_defenders_spinbutton->set_value(atoi(num_defenders_spinbutton->get_text().c_str()));
  on_num_defenders_changed();
}

void ItemlistDialog::on_defender_type_selected(const ArmyProto *a)
{
  if (a)
    d_item->setArmyTypeToRaise(a->getId());
  else
    {
      d_item->clearArmyTypeToRaise ();
      raise_defenders_switch->property_active () = false;
      num_defenders_spinbutton->set_value(1);
    }

  d_changed = true;
}

void ItemlistDialog::on_building_type_to_summon_on_changed ()
{
  guint32 row = building_type_to_summon_on_combobox->get_active_row_number ();
  d_item->setBuildingTypeToSummonOn(row);
}

void ItemlistDialog::on_plantable_toggled()
{
  on_switch_toggled(plantable_switch, ItemProto::PLANT_TO_VECTOR);
}
