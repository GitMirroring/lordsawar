//  Copyright (C) 2008, 2009, 2011, 2014, 2015, 2020, 2021 Ben Asselstine
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
#include "itemlist-editor-actions.h"
#include "timed-message-dialog.h"
#include "herotemplates.h"
#define method(x) sigc::mem_fun(*this, &ItemlistDialog::x)

ItemlistDialog::ItemlistDialog(Gtk::Window &parent)
 : LwEditorDialog(parent, "itemlist-dialog.ui")
{
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));
  d_changed = false;
  d_warn_itemlist_change_affects_herotemplates = false;
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

  fill_items ();

  d_item = NULL;
  guint32 max = d_itemlist->size();
  if (max)
    {
      Gtk::TreeModel::Row row;
      row = items_treeview->get_model()->children()[0];
      if(row)
        items_treeview->get_selection()->select(row);
    }
  connect_signals ();
  update ();
}

ItemlistDialog::~ItemlistDialog()
{
  delete umgr;
  delete kill_army_type_button;
  delete summon_army_type_button;
  delete defender_army_type_button;
}

void ItemlistDialog::fill_items ()
{
  items_list->clear ();
  for (Itemlist::iterator iter = d_itemlist->begin();
       iter != d_itemlist->end(); ++iter)
    addItemProto((*iter).second);
}

void ItemlistDialog::load_widgets ()
{
  xml->get_widget("name_entry", name_entry);
  umgr->addCursor (name_entry);
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
  xml->get_widget("undo_button", undo_button);
  xml->get_widget("redo_button", redo_button);
}

void ItemlistDialog::connect_signals ()
{
  umgr->connect_signals ();
  connections.push_back(items_treeview->get_selection()->signal_changed().connect (method(on_item_selected)));
  connections.push_back (name_entry->signal_changed().connect (method(on_name_changed)));
  connections.push_back (add_item_button->signal_clicked().connect (method(on_add_item_clicked)));
  connections.push_back (remove_item_button->signal_clicked().connect (method(on_remove_item_clicked)));
  connections.push_back (kill_army_type_button->army_selected.connect(method(on_kill_army_type_selected)));
  connections.push_back (summon_army_type_button->army_selected.connect(method(on_summon_army_type_selected)));
  connections.push_back (disease_city_switch->property_active().signal_changed().connect(method(on_disease_city_toggled)));
  connections.push_back (disease_armies_percent_spinbutton->signal_insert_text().connect (sigc::hide(sigc::hide(method(on_disease_armies_percent_text_changed)))));
  connections.push_back (raise_defenders_switch->property_active().signal_changed().connect (method(on_raise_defenders_toggled)));
  connections.push_back (defender_army_type_button->army_selected.connect (method(on_defender_type_selected)));
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
  connections.push_back (uses_spinbutton->signal_insert_text().connect
    (sigc::hide(sigc::hide(method(on_uses_text_changed)))));
  connections.push_back (steal_percent_spinbutton->signal_insert_text().connect
    (sigc::hide(sigc::hide(method(on_steal_percent_text_changed)))));
  connections.push_back (add_mp_spinbutton->signal_insert_text().connect
    (sigc::hide(sigc::hide(method(on_add_mp_text_changed)))));
  connections.push_back (building_type_to_summon_on_combobox->signal_changed ().connect (method (on_building_type_to_summon_on_changed)));
  connections.push_back (plantable_switch->property_active().signal_changed().connect (method(on_plantable_toggled)));
  connections.push_back (undo_button->signal_activate().connect
                         (method(on_undo_activated)));
  connections.push_back (redo_button->signal_activate().connect
                         (method(on_redo_activated)));
}

void ItemlistDialog::disconnect_signals ()
{
  umgr->disconnect_signals ();
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
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
      clear_item_info ();
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
  update ();
}

void ItemlistDialog::clear_item_info()
{
  name_entry->set_text ("");
  add1str_switch->set_active (false);
  add2str_switch->set_active (false);
  add3str_switch->set_active (false);
  add1stack_switch->set_active (false);
  add2stack_switch->set_active (false);
  add3stack_switch->set_active (false);
  flystack_switch->set_active (false);
  doublemovestack_switch->set_active (false);
  add2goldpercity_switch->set_active (false);
  add3goldpercity_switch->set_active (false);
  add4goldpercity_switch->set_active (false);
  add5goldpercity_switch->set_active (false);
  steals_gold_switch->set_active (false);
  pickup_bags_switch->set_active (false);
  add_mp_switch->set_active (false);
  sinks_ships_switch->set_active (false);
  banish_worms_switch->set_active (false);
  burn_bridge_switch->set_active (false);
  capture_keeper_switch->set_active (false);
  uses_spinbutton->set_value (0);
  steal_percent_spinbutton->set_value (0);
  add_mp_spinbutton->set_value (0);
  summon_monster_switch->set_active (false);
  building_type_to_summon_on_combobox->set_active (false);
  disease_city_switch->set_active (false);
  disease_armies_percent_spinbutton->set_value (0);
  kill_army_type_button->clear_selected_army ();
  summon_army_type_button->clear_selected_army ();
  raise_defenders_switch->set_active (false);
  num_defenders_spinbutton->set_value (0);
  defender_army_type_button->clear_selected_army ();
  persuade_neutral_city_switch->set_active (false);
  teleport_to_city_switch->set_active (false);
  plantable_switch->set_active (false);
  item_vbox->set_sensitive (false);
}

void ItemlistDialog::fill_item_info(ItemProto *item)
{
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
}

void ItemlistDialog::on_name_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = items_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      umgr->add (new ItemListEditorAction_Name (getCurIndex (),
                                                d_item->getName (),
                                                umgr, name_entry));
      Gtk::TreeModel::Row row = *iterrow;
      ItemProto *a = row[items_columns.item];
      a->setName(name_entry->get_text());
      row[items_columns.name] = name_entry->get_text();
      d_changed = true;
    }
}

void ItemlistDialog::on_add_item_clicked()
{
  umgr->add (new ItemListEditorAction_Add (Itemlist::getInstance ()->copy (),
                                           getCurIndex ()));
  //add a new empty item to the itemlist
  ItemProto *a = new ItemProto(_("Untitled"));
  d_itemlist->add(a);
  //add it to the treeview
  Gtk::TreeIter i = items_list->append();
  a->setName(_("Untitled"));
  (*i)[items_columns.name] = a->getName();
  (*i)[items_columns.item] = a;
  items_treeview->get_selection()->select(i);
  items_treeview->scroll_to_row (items_treeview->get_model ()->get_path (i));

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
      umgr->add (new ItemListEditorAction_Remove
                 (Itemlist::getInstance ()->copy (), getCurIndex ()));
      Gtk::TreeModel::Row row = *iterrow;
      ItemProto *a = row[items_columns.item];
      items_list->erase(iterrow);
      d_itemlist->remove(a);
      d_changed = true;
      if (HeroTemplates::getInstance ()->removeItemAffectsStartingItemIds
          ((guint32)getCurIndex ()))
        {
          d_warn_itemlist_change_affects_herotemplates = true;
          TimedMessageDialog
            d(*dialog, _("Removing this item has broken one or more hero starting items.\nWe'd like to automatically fix this for you but we can't!\nYou must manually fix them."), 0);
          d.run_and_hide ();
        }
    }
}

ItemProto* ItemlistDialog::getCurItem ()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = items_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      // Row selected
      Gtk::TreeModel::Row row = *iterrow;

      return row[items_columns.item];
    }
  return NULL;
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
  umgr->add (new ItemListEditorAction_AddStr
             (getCurIndex (), 1, getCurItem ()->getBonus (ItemProto::ADD1STR)));
  on_switch_toggled(add1str_switch, ItemProto::ADD1STR);
}

void ItemlistDialog::on_add2str_toggled()
{
  umgr->add (new ItemListEditorAction_AddStr
             (getCurIndex (), 2, getCurItem ()->getBonus (ItemProto::ADD2STR)));
  on_switch_toggled(add2str_switch, ItemProto::ADD2STR);
}

void ItemlistDialog::on_add3str_toggled()
{
  umgr->add (new ItemListEditorAction_AddStr
             (getCurIndex (), 3, getCurItem ()->getBonus (ItemProto::ADD3STR)));
  on_switch_toggled(add3str_switch, ItemProto::ADD3STR);
}

void ItemlistDialog::on_add1stack_toggled()
{
  umgr->add (new ItemListEditorAction_AddStack
             (getCurIndex (), 1,
              getCurItem ()->getBonus (ItemProto::ADD1STACK)));
  on_switch_toggled(add1stack_switch, ItemProto::ADD1STACK);
}

void ItemlistDialog::on_add2stack_toggled()
{
  umgr->add (new ItemListEditorAction_AddStack
             (getCurIndex (), 2,
              getCurItem ()->getBonus (ItemProto::ADD2STACK)));
  on_switch_toggled(add2stack_switch, ItemProto::ADD2STACK);
}

void ItemlistDialog::on_add3stack_toggled()
{
  umgr->add (new ItemListEditorAction_AddStack
             (getCurIndex (), 3,
              getCurItem ()->getBonus (ItemProto::ADD3STACK)));
  on_switch_toggled(add3stack_switch, ItemProto::ADD3STACK);
}

void ItemlistDialog::on_flystack_toggled()
{
  umgr->add (new ItemListEditorAction_FlyStack
             (getCurIndex (), getCurItem ()->getBonus (ItemProto::FLYSTACK)));
  on_switch_toggled(flystack_switch, ItemProto::FLYSTACK);
}

void ItemlistDialog::on_doublemovestack_toggled()
{
  umgr->add (new ItemListEditorAction_DoubleMovementStack
             (getCurIndex (),
              getCurItem ()->getBonus (ItemProto::DOUBLEMOVESTACK)));
  on_switch_toggled(doublemovestack_switch, ItemProto::DOUBLEMOVESTACK);
}

void ItemlistDialog::on_add2goldpercity_toggled()
{
  umgr->add (new ItemListEditorAction_AddGoldPerCity
             (getCurIndex (), 2,
              getCurItem ()->getBonus (ItemProto::ADD2GOLDPERCITY)));
  on_switch_toggled(add2goldpercity_switch, ItemProto::ADD2GOLDPERCITY);
}

void ItemlistDialog::on_add3goldpercity_toggled()
{
  umgr->add (new ItemListEditorAction_AddGoldPerCity
             (getCurIndex (), 3,
              getCurItem ()->getBonus (ItemProto::ADD3GOLDPERCITY)));
  on_switch_toggled(add2goldpercity_switch, ItemProto::ADD3GOLDPERCITY);
}

void ItemlistDialog::on_add4goldpercity_toggled()
{
  umgr->add (new ItemListEditorAction_AddGoldPerCity
             (getCurIndex (), 4,
              getCurItem ()->getBonus (ItemProto::ADD4GOLDPERCITY)));
  on_switch_toggled(add4goldpercity_switch, ItemProto::ADD4GOLDPERCITY);
}

void ItemlistDialog::on_add5goldpercity_toggled()
{
  umgr->add (new ItemListEditorAction_AddGoldPerCity
             (getCurIndex (), 5,
              getCurItem ()->getBonus (ItemProto::ADD5GOLDPERCITY)));
  on_switch_toggled(add5goldpercity_switch, ItemProto::ADD5GOLDPERCITY);
}

void ItemlistDialog::on_steals_gold_toggled()
{
  umgr->add (new ItemListEditorAction_StealsGold
             (getCurIndex (), getCurItem ()->getBonus (ItemProto::STEAL_GOLD)));
  on_switch_toggled(steals_gold_switch, ItemProto::STEAL_GOLD);
  steal_percent_spinbutton->set_sensitive
    (steals_gold_switch->get_active());
  if (steals_gold_switch->get_active () == false)
    steal_percent_spinbutton->set_value (1);
}

void ItemlistDialog::on_pickup_bags_toggled()
{
  umgr->add (new ItemListEditorAction_PickupBags
             (getCurIndex (),
              getCurItem ()->getBonus (ItemProto::PICK_UP_BAGS)));
  on_switch_toggled(pickup_bags_switch, ItemProto::PICK_UP_BAGS);
}

void ItemlistDialog::on_add_mp_toggled()
{
  umgr->add (new ItemListEditorAction_AddMovement
             (getCurIndex (),
              getCurItem ()->getBonus (ItemProto::ADD_2MP_STACK)));
  on_switch_toggled(add_mp_switch, ItemProto::ADD_2MP_STACK);
  add_mp_spinbutton->set_sensitive (add_mp_switch->get_active());
  if (add_mp_switch->get_active () == false)
    add_mp_spinbutton->set_value (1);
}

void ItemlistDialog::on_sinks_ships_toggled()
{
  umgr->add (new ItemListEditorAction_SinkShips
             (getCurIndex (),
              getCurItem ()->getBonus (ItemProto::SINK_SHIPS)));
  on_switch_toggled(sinks_ships_switch, ItemProto::SINK_SHIPS);
}
	
void ItemlistDialog::on_banish_worms_toggled()
{
  umgr->add (new ItemListEditorAction_BanishWorms
             (getCurIndex (),
              getCurItem ()->getBonus (ItemProto::BANISH_WORMS)));
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
  umgr->add (new ItemListEditorAction_BurnBridge
             (getCurIndex (),
              getCurItem ()->getBonus (ItemProto::BURN_BRIDGE)));
  on_switch_toggled(burn_bridge_switch, ItemProto::BURN_BRIDGE);
}

void ItemlistDialog::on_uses_text_changed()
{
  umgr->add (new ItemListEditorAction_Uses
             (getCurIndex (), getCurItem ()->getNumberOfUsesLeft ()));
  uses_spinbutton->set_value(atoi(uses_spinbutton->get_text().c_str()));
  on_uses_changed();
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
  umgr->add (new ItemListEditorAction_SelectBanishArmy
             (getCurIndex (), d_item->getArmyTypeToKill (),
              d_item->hasArmyTypeToKill ()));
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
  umgr->add (new ItemListEditorAction_CaptureKeeper
             (getCurIndex (), getCurItem ()->getBonus
              (ItemProto::CAPTURE_KEEPER)));
  on_switch_toggled(capture_keeper_switch, ItemProto::CAPTURE_KEEPER);
}
    
void ItemlistDialog::on_summon_monster_toggled()
{
  umgr->add (new ItemListEditorAction_SummonMonster
             (getCurIndex (), getCurItem ()->getBonus
              (ItemProto::SUMMON_MONSTER)));
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
  umgr->add (new ItemListEditorAction_SelectSummonArmy
             (getCurIndex (), d_item->getArmyTypeToSummon (),
              d_item->hasArmyTypeToSummon ()));
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
  umgr->add (new ItemListEditorAction_DiseaseCity
             (getCurIndex (), getCurItem ()->getBonus
              (ItemProto::DISEASE_CITY)));
  on_switch_toggled(disease_city_switch, ItemProto::DISEASE_CITY);
  disease_armies_percent_spinbutton->set_sensitive
    (disease_city_switch->get_active());
  if (disease_city_switch->get_active () == false)
    disease_armies_percent_spinbutton->set_value (1);
}

void ItemlistDialog::on_persuade_neutral_city_toggled()
{
  umgr->add (new ItemListEditorAction_PersuadeNeutralCity
             (getCurIndex (), getCurItem ()->getBonus
              (ItemProto::PERSUADE_NEUTRALS)));
  on_switch_toggled(persuade_neutral_city_switch, 
                         ItemProto::PERSUADE_NEUTRALS);
}

void ItemlistDialog::on_teleport_to_city_toggled()
{
  umgr->add (new ItemListEditorAction_TeleportToCity
             (getCurIndex (), getCurItem ()->getBonus
              (ItemProto::TELEPORT_TO_CITY)));
  on_switch_toggled(teleport_to_city_switch, 
                         ItemProto::TELEPORT_TO_CITY);
}

void ItemlistDialog::on_raise_defenders_toggled()
{
  umgr->add (new ItemListEditorAction_RaiseDefenders
             (getCurIndex (), getCurItem ()->getBonus
              (ItemProto::RAISE_DEFENDERS)));
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
  umgr->add (new ItemListEditorAction_StealPercent
             (getCurIndex (), getCurItem ()->getPercentGoldToSteal ()));
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
  umgr->add (new ItemListEditorAction_DiseaseArmies
             (getCurIndex (), getCurItem ()->getPercentArmiesToKill ()));
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
  umgr->add (new ItemListEditorAction_AddMp
             (getCurIndex (), getCurItem ()->getMovementPointsToAdd ()));
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
  umgr->add (new ItemListEditorAction_NumDefenders
             (getCurIndex (), getCurItem ()->getNumberOfArmiesToRaise ()));
  num_defenders_spinbutton->set_value(atoi(num_defenders_spinbutton->get_text().c_str()));
  on_num_defenders_changed();
}

void ItemlistDialog::on_defender_type_selected(const ArmyProto *a)
{
  umgr->add (new ItemListEditorAction_SelectRaiseArmy
             (getCurIndex (), d_item->getArmyTypeToRaise (),
              d_item->hasArmyTypeToRaise ()));
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
  umgr->add (new ItemListEditorAction_BuildingTypeToSummonOn
             (getCurIndex (), getCurItem ()->getBuildingTypeToSummonOn ()));
  guint32 row = building_type_to_summon_on_combobox->get_active_row_number ();
  d_item->setBuildingTypeToSummonOn(row);
}

void ItemlistDialog::on_plantable_toggled()
{
  umgr->add (new ItemListEditorAction_Plantable
             (getCurIndex (), getCurItem ()->getBonus
              (ItemProto::PLANT_TO_VECTOR)));
  on_switch_toggled(plantable_switch, ItemProto::PLANT_TO_VECTOR);
}

void ItemlistDialog::on_undo_activated ()
{
  umgr->undo ();
  if (umgr->undoEmpty ())
    d_changed = false;
  update ();
  return;
}

void ItemlistDialog::on_redo_activated ()
{
  umgr->redo ();
  d_changed = true;
  update ();
}

void ItemlistDialog::update ()
{
  disconnect_signals ();
  update_item_panel ();
  update_itemlist_buttons();
  umgr->setCursors ();
  connect_signals ();
}

ItemProto* ItemlistDialog::getItemByIndex (ItemListEditorAction_Index *a)
{
  auto path = Gtk::TreePath (String::ucompose ("%1", a->getIndex ()));
  auto iterrow = items_treeview->get_model ()->get_iter (path);
  Gtk::TreeModel::Row row = *iterrow;
  ItemProto *item = row[items_columns.item];
  return item;
}

int ItemlistDialog::getCurIndex ()
{
  int idx = -1;
  Gtk::TreeIter i = items_treeview->get_selection()->get_selected();
  if (i)
    {
      auto path = items_treeview->get_model ()->get_path (i);
      idx = atoi (path.to_string ().c_str ());
    }
  return idx;
}
UndoAction *ItemlistDialog::executeAction (UndoAction *action2)
{
  ItemListEditorAction *action = dynamic_cast<ItemListEditorAction*>(action2);
  UndoAction *out = NULL;

  switch (action->getType ())
    {
      case ItemListEditorAction::ADD:
          {
            ItemListEditorAction_Add *a =
              dynamic_cast<ItemListEditorAction_Add*>(action);
            out = new ItemListEditorAction_Add
              (Itemlist::getInstance ()->copy (), d_itemlist->size ());
            Itemlist::reset (a->getItemList ());
            d_itemlist = Itemlist::getInstance();
            a->clearItemList ();
            items_treeview->get_selection ()->unselect_all ();
            fill_items ();
          }
        break;
      case ItemListEditorAction::REMOVE:
          {
            ItemListEditorAction_Remove *a =
              dynamic_cast<ItemListEditorAction_Remove*>(action);
            out = new ItemListEditorAction_Remove
              (Itemlist::getInstance ()->copy (), a->getIndex ());
            Itemlist::reset (a->getItemList ());
            d_itemlist = Itemlist::getInstance();
            a->clearItemList ();
            items_treeview->get_selection ()->unselect_all ();
            fill_items ();
          }
        break;
      case ItemListEditorAction::BUILDING_TYPE_TO_SUMMON_ON:
          {
            ItemListEditorAction_BuildingTypeToSummonOn *a =
              dynamic_cast<ItemListEditorAction_BuildingTypeToSummonOn*>(action);
            int row = 
              building_type_to_summon_on_combobox->get_active_row_number ();
            out = new ItemListEditorAction_BuildingTypeToSummonOn
              (a->getIndex (), row);
            getItemByIndex (a)->setBuildingTypeToSummonOn (a->getRow ());
          } 
        break;
      case ItemListEditorAction::DISEASE_CITY:
          {
            ItemListEditorAction_DiseaseCity *a =
              dynamic_cast<ItemListEditorAction_DiseaseCity*>(action);
            out = new ItemListEditorAction_DiseaseCity
              (a->getIndex (), disease_city_switch->get_active ());
            if (a->getActive ())
              getItemByIndex (a)->addBonus (ItemProto::DISEASE_CITY);
            else
              getItemByIndex (a)->removeBonus (ItemProto::DISEASE_CITY);
          }
        break;
      case ItemListEditorAction::DISEASE_ARMIES_PERCENT:
          {
            ItemListEditorAction_DiseaseArmies *a =
              dynamic_cast<ItemListEditorAction_DiseaseArmies*>(action);
            out = new ItemListEditorAction_DiseaseArmies
              (a->getIndex (), disease_armies_percent_spinbutton->get_value ());
            getItemByIndex (a)->setPercentArmiesToKill (a->getNum ());
          }
        break;
      case ItemListEditorAction::RAISE_DEFENDERS:
          {
            ItemListEditorAction_RaiseDefenders *a =
              dynamic_cast<ItemListEditorAction_RaiseDefenders*>(action);
            out = new ItemListEditorAction_RaiseDefenders
              (a->getIndex (), raise_defenders_switch->get_active ());
            if (a->getActive ())
              getItemByIndex (a)->addBonus (ItemProto::RAISE_DEFENDERS);
            else
              getItemByIndex (a)->removeBonus (ItemProto::RAISE_DEFENDERS);
          }
        break;
      case ItemListEditorAction::NUM_DEFENDERS:
          {
            ItemListEditorAction_NumDefenders *a =
              dynamic_cast<ItemListEditorAction_NumDefenders*>(action);
            out = new ItemListEditorAction_NumDefenders
              (a->getIndex (), num_defenders_spinbutton->get_value ());
            getItemByIndex (a)->setNumberOfArmiesToRaise(a->getNum ());
          }
        break;
      case ItemListEditorAction::PERSUADE_NEUTRAL_CITY:
          {
            ItemListEditorAction_PersuadeNeutralCity *a =
              dynamic_cast<ItemListEditorAction_PersuadeNeutralCity*>(action);
            out = new ItemListEditorAction_PersuadeNeutralCity
              (a->getIndex (), persuade_neutral_city_switch->get_active ());
            if (a->getActive ())
              getItemByIndex (a)->addBonus (ItemProto::PERSUADE_NEUTRALS);
            else
              getItemByIndex (a)->removeBonus (ItemProto::PERSUADE_NEUTRALS);
          }
        break;
      case ItemListEditorAction::TELEPORT_TO_CITY:
          {
            ItemListEditorAction_TeleportToCity *a =
              dynamic_cast<ItemListEditorAction_TeleportToCity*>(action);
            out = new ItemListEditorAction_TeleportToCity
              (a->getIndex (), teleport_to_city_switch->get_active ());
            if (a->getActive ())
              getItemByIndex (a)->addBonus (ItemProto::TELEPORT_TO_CITY);
            else
              getItemByIndex (a)->removeBonus (ItemProto::TELEPORT_TO_CITY);
          }
        break;
      case ItemListEditorAction::ADDSTR:
          {
            ItemListEditorAction_AddStr *a =
              dynamic_cast<ItemListEditorAction_AddStr*>(action);
            bool state = false;
            switch (a->getNum ())
              {
              default:
              case 1: state = add1str_switch->get_active ();
                break;
              case 2: state = add2str_switch->get_active ();
                break;
              case 3: state = add3str_switch->get_active ();
                break;
              }
            out = new ItemListEditorAction_AddStr (a->getIndex (), a->getNum (),
                                                   state);
            ItemProto::Bonus b = ItemProto::ADD1STR;
            switch (a->getNum ())
              {
              default:
              case 1:  b = ItemProto::ADD1STR; break;
              case 2:  b = ItemProto::ADD2STR; break;
              case 3:  b = ItemProto::ADD3STR; break;
              }
            if (a->getActive ())
              getItemByIndex (a)->addBonus (b);
            else
              getItemByIndex (a)->removeBonus (b);
          }
        break;
      case ItemListEditorAction::ADDSTACK:
          {
            ItemListEditorAction_AddStack *a =
              dynamic_cast<ItemListEditorAction_AddStack*>(action);
            bool state = false;
            switch (a->getNum ())
              {
              default:
              case 1: state = add1stack_switch->get_active ();
                break;
              case 2: state = add2stack_switch->get_active ();
                break;
              case 3: state = add3stack_switch->get_active ();
                break;
              }
            out = new ItemListEditorAction_AddStack (a->getIndex (),
                                                     a->getNum (), state);
            ItemProto::Bonus b = ItemProto::ADD1STACK;
            switch (a->getNum ())
              {
              default:
              case 1:  b = ItemProto::ADD1STACK; break;
              case 2:  b = ItemProto::ADD2STACK; break;
              case 3:  b = ItemProto::ADD3STACK; break;
              }
            if (a->getActive ())
              getItemByIndex (a)->addBonus (b);
            else
              getItemByIndex (a)->removeBonus (b);
          }
        break;
      case ItemListEditorAction::FLY_STACK:
          {
            ItemListEditorAction_FlyStack *a =
              dynamic_cast<ItemListEditorAction_FlyStack*>(action);
            out = new ItemListEditorAction_FlyStack
              (a->getIndex (), flystack_switch->get_active ());
            if (a->getActive ())
              getItemByIndex (a)->addBonus (ItemProto::FLYSTACK);
            else
              getItemByIndex (a)->removeBonus (ItemProto::FLYSTACK);
          }
        break;
      case ItemListEditorAction::DOUBLE_MOVEMENT_STACK:
          {
            ItemListEditorAction_DoubleMovementStack *a =
              dynamic_cast<ItemListEditorAction_DoubleMovementStack*>(action);
            out = new ItemListEditorAction_DoubleMovementStack
              (a->getIndex (), doublemovestack_switch->get_active ());
            if (a->getActive ())
              getItemByIndex (a)->addBonus (ItemProto::DOUBLEMOVESTACK);
            else
              getItemByIndex (a)->removeBonus (ItemProto::DOUBLEMOVESTACK);
          }
        break;
      case ItemListEditorAction::ADD_GOLD_PER_CITY:
          {
            ItemListEditorAction_AddGoldPerCity *a =
              dynamic_cast<ItemListEditorAction_AddGoldPerCity*>(action);
            bool state = false;
            switch (a->getNum ())
              {
              default:
              case 2: state = add2goldpercity_switch->get_active ();
                break;
              case 3: state = add3goldpercity_switch->get_active ();
                break;
              case 4: state = add4goldpercity_switch->get_active ();
                break;
              case 5: state = add5goldpercity_switch->get_active ();
                break;
              }
            out = new ItemListEditorAction_AddGoldPerCity (a->getIndex (),
                                                           a->getNum (), state);
            ItemProto::Bonus b = ItemProto::ADD2GOLDPERCITY;
            switch (a->getNum ())
              {
              default:
              case 2:  b = ItemProto::ADD2GOLDPERCITY; break;
              case 3:  b = ItemProto::ADD3GOLDPERCITY; break;
              case 4:  b = ItemProto::ADD4GOLDPERCITY; break;
              case 5:  b = ItemProto::ADD5GOLDPERCITY; break;
              }
            if (a->getActive ())
              getItemByIndex (a)->addBonus (b);
            else
              getItemByIndex (a)->removeBonus (b);
          }
        break;
      case ItemListEditorAction::STEALS_GOLD:
          {
            ItemListEditorAction_StealsGold *a =
              dynamic_cast<ItemListEditorAction_StealsGold*>(action);
            out = new ItemListEditorAction_StealsGold
              (a->getIndex (), steals_gold_switch->get_active ());
            if (a->getActive ())
              getItemByIndex (a)->addBonus (ItemProto::STEAL_GOLD);
            else
              getItemByIndex (a)->removeBonus (ItemProto::STEAL_GOLD);
          }
        break;
      case ItemListEditorAction::PICKUP_BAGS:
          {
            ItemListEditorAction_PickupBags *a =
              dynamic_cast<ItemListEditorAction_PickupBags*>(action);
            out = new ItemListEditorAction_PickupBags
              (a->getIndex (), pickup_bags_switch->get_active ());
            if (a->getActive ())
              getItemByIndex (a)->addBonus (ItemProto::PICK_UP_BAGS);
            else
              getItemByIndex (a)->removeBonus (ItemProto::PICK_UP_BAGS);
          }
        break;
      case ItemListEditorAction::ADD_MOVEMENT:
          {
            ItemListEditorAction_AddMovement *a =
              dynamic_cast<ItemListEditorAction_AddMovement*>(action);
            out = new ItemListEditorAction_AddMovement
              (a->getIndex (), add_mp_switch->get_active ());
            if (a->getActive ())
              getItemByIndex (a)->addBonus (ItemProto::ADD_2MP_STACK);
            else
              getItemByIndex (a)->removeBonus (ItemProto::ADD_2MP_STACK);
          }
        break;
      case ItemListEditorAction::SINKS_SHIPS:
          {
            ItemListEditorAction_SinkShips *a =
              dynamic_cast<ItemListEditorAction_SinkShips*>(action);
            out = new ItemListEditorAction_SinkShips
              (a->getIndex (), sinks_ships_switch->get_active ());
            if (a->getActive ())
              getItemByIndex (a)->addBonus (ItemProto::SINK_SHIPS);
            else
              getItemByIndex (a)->removeBonus (ItemProto::SINK_SHIPS);
          }
        break;
      case ItemListEditorAction::BANISH_WORMS:
          {
            ItemListEditorAction_BanishWorms *a =
              dynamic_cast<ItemListEditorAction_BanishWorms*>(action);
            out = new ItemListEditorAction_BanishWorms
              (a->getIndex (), banish_worms_switch->get_active ());
            if (a->getActive ())
              getItemByIndex (a)->addBonus (ItemProto::BANISH_WORMS);
            else
              getItemByIndex (a)->removeBonus (ItemProto::BANISH_WORMS);
          }
        break;
      case ItemListEditorAction::BURN_BRIDGE:
          {
            ItemListEditorAction_BurnBridge *a =
              dynamic_cast<ItemListEditorAction_BurnBridge*>(action);
            out = new ItemListEditorAction_BurnBridge
              (a->getIndex (), burn_bridge_switch->get_active ());
            if (a->getActive ())
              getItemByIndex (a)->addBonus (ItemProto::BURN_BRIDGE);
            else
              getItemByIndex (a)->removeBonus (ItemProto::BURN_BRIDGE);
          }
        break;
      case ItemListEditorAction::CAPTURE_KEEPER:
          {
            ItemListEditorAction_CaptureKeeper *a =
              dynamic_cast<ItemListEditorAction_CaptureKeeper*>(action);
            out = new ItemListEditorAction_CaptureKeeper
              (a->getIndex (), capture_keeper_switch->get_active ());
            if (a->getActive ())
              getItemByIndex (a)->addBonus (ItemProto::CAPTURE_KEEPER);
            else
              getItemByIndex (a)->removeBonus (ItemProto::CAPTURE_KEEPER);
          }
        break;
      case ItemListEditorAction::SUMMON_MONSTER:
          {
            ItemListEditorAction_SummonMonster *a =
              dynamic_cast<ItemListEditorAction_SummonMonster*>(action);
            out = new ItemListEditorAction_SummonMonster
              (a->getIndex (), summon_monster_switch->get_active ());
            if (a->getActive ())
              getItemByIndex (a)->addBonus (ItemProto::SUMMON_MONSTER);
            else
              getItemByIndex (a)->removeBonus (ItemProto::SUMMON_MONSTER);
          }
        break;
      case ItemListEditorAction::USES:
          {
            ItemListEditorAction_Uses *a =
              dynamic_cast<ItemListEditorAction_Uses*>(action);
            out = new ItemListEditorAction_Uses
              (a->getIndex (), uses_spinbutton->get_value ());
            getItemByIndex (a)->setNumberOfUsesLeft(a->getNum ());
          }
        break;
      case ItemListEditorAction::STEAL_PERCENT:
          {
            ItemListEditorAction_StealPercent *a =
              dynamic_cast<ItemListEditorAction_StealPercent*>(action);
            out = new ItemListEditorAction_StealPercent
              (a->getIndex (), steal_percent_spinbutton->get_value ());
            getItemByIndex (a)->setPercentGoldToSteal(a->getNum ());
          }
        break;
      case ItemListEditorAction::ADD_MP:
          {
            ItemListEditorAction_AddMp *a =
              dynamic_cast<ItemListEditorAction_AddMp*>(action);
            out = new ItemListEditorAction_AddMp
              (a->getIndex (), add_mp_spinbutton->get_value ());
            getItemByIndex (a)->setMovementPointsToAdd(a->getNum ());
          }
        break;
      case ItemListEditorAction::PLANTABLE:
          {
            ItemListEditorAction_Plantable *a =
              dynamic_cast<ItemListEditorAction_Plantable*>(action);
            out = new ItemListEditorAction_Plantable
              (a->getIndex (), plantable_switch->get_active ());
            if (a->getActive ())
              getItemByIndex (a)->addBonus (ItemProto::PLANT_TO_VECTOR);
            else
              getItemByIndex (a)->removeBonus (ItemProto::PLANT_TO_VECTOR);
          }
        break;
      case ItemListEditorAction::NAME:
          {
            ItemListEditorAction_Name *a =
              dynamic_cast<ItemListEditorAction_Name*>(action);
            out = new ItemListEditorAction_Name
              (a->getIndex (), getItemByIndex (a)->getName (), umgr,
               name_entry);

            getItemByIndex (a)->setName (a->getName ());
            auto iterrow = items_treeview->get_model ()->get_iter
              (String::ucompose ("%1", a->getIndex ()));
            if (iterrow)
              {
                Gtk::TreeModel::Row row = *iterrow;
                row[items_columns.name] = a->getName ();
              }
          }
        break;
      case ItemListEditorAction::SELECT_BANISH_ARMY:
          {
            ItemListEditorAction_SelectBanishArmy *a =
              dynamic_cast<ItemListEditorAction_SelectBanishArmy*>(action);
            out = new ItemListEditorAction_SelectBanishArmy
              (a->getIndex (), getItemByIndex (a)->getArmyTypeToKill (),
               getItemByIndex (a)->hasArmyTypeToKill ());
            if (a->getPresent () == false)
              getItemByIndex (a)->clearArmyTypeToKill ();
            else
              getItemByIndex (a)->setArmyTypeToKill (a->getArmyType ());
          }
        break;
      case ItemListEditorAction::SELECT_SUMMON_ARMY:
          {
            ItemListEditorAction_SelectSummonArmy *a =
              dynamic_cast<ItemListEditorAction_SelectSummonArmy*>(action);
            out = new ItemListEditorAction_SelectSummonArmy
              (a->getIndex (), getItemByIndex (a)->getArmyTypeToSummon (),
               getItemByIndex (a)->hasArmyTypeToSummon ());
            if (a->getPresent () == false)
              getItemByIndex (a)->clearArmyTypeToSummon ();
            else
              getItemByIndex (a)->setArmyTypeToSummon (a->getArmyType ());
          }
        break;
      case ItemListEditorAction::SELECT_RAISE_ARMY:
          {
            ItemListEditorAction_SelectRaiseArmy *a =
              dynamic_cast<ItemListEditorAction_SelectRaiseArmy*>(action);
            out = new ItemListEditorAction_SelectRaiseArmy
              (a->getIndex (), getItemByIndex (a)->getArmyTypeToRaise (),
               getItemByIndex (a)->hasArmyTypeToRaise ());
            if (a->getPresent () == false)
              getItemByIndex (a)->clearArmyTypeToRaise ();
            else
              getItemByIndex (a)->setArmyTypeToRaise (a->getArmyType ());
          }
        break;
    }
  return out;
}
