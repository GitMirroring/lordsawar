//  Copyright (C) 2008, 2009, 2011, 2014, 2017, 2020, 2021 Ben Asselstine
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

#include "reward-editor-dialog.h"

#include "ucompose.hpp"
#include "defs.h"
#include "reward.h"
#include "ruin.h"
#include "Item.h"
#include "army.h"
#include "GameMap.h"
#include "select-item-dialog.h"
#include "select-army-dialog.h"
#include "select-hidden-ruin-dialog.h"
#include "armyproto.h"
#include "SightMap.h"
#include "army-chooser-button.h"
#include "reward-editor-actions.h"

#define method(x) sigc::mem_fun(*this, &RewardEditorDialog::x)

RewardEditorDialog::RewardEditorDialog(Gtk::Window &parent, Player *player, bool hidden_ruins, Reward *r)
 : LwEditorDialog(parent, "reward-editor-dialog.ui")
{
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));
  d_player = player;
  d_changed = false;
  d_hidden_ruins = hidden_ruins;
  setup_rewards (r);
  xml->get_widget("undo_button", undo_button);
  undo_button->signal_activate ().connect (method (on_undo_activated));
  xml->get_widget("redo_button", redo_button);
  redo_button->signal_activate ().connect (method (on_redo_activated));

  xml->get_widget("reward_type_combobox", reward_type_combobox);
  xml->get_widget("notebook", notebook);
  xml->get_widget("gold_spinbutton", gold_spinbutton);
  xml->get_widget("randomize_gold_button", randomize_gold_button);
  randomize_gold_button->signal_clicked().connect (method(on_randomize_gold_clicked));
  xml->get_widget("item_button", item_button);
  item_button->signal_clicked().connect (method(on_item_clicked));
  xml->get_widget("randomize_item_button", randomize_item_button);
  randomize_item_button->signal_clicked().connect (method(on_randomize_item_clicked));

  xml->get_widget("num_allies_spinbutton", num_allies_spinbutton);
  ally_button =
    new ArmyChooserButton (parent, xml, "ally_button", d_player,
                          SelectArmyDialog::SELECT_REWARDABLE_ARMY);
  xml->get_widget("randomize_allies_button", randomize_allies_button);
  randomize_allies_button->signal_clicked().connect (method(on_randomize_allies_clicked));

  xml->get_widget("map_x_spinbutton", map_x_spinbutton);
  xml->get_widget("map_y_spinbutton", map_y_spinbutton);
  xml->get_widget("map_width_spinbutton", map_width_spinbutton);
  xml->get_widget("map_height_spinbutton", map_height_spinbutton);
  xml->get_widget("randomize_map_button", randomize_map_button);
  randomize_map_button->signal_clicked().connect (method(on_randomize_map_clicked));
  map_x_spinbutton->set_range (0, GameMap::getInstance()->getWidth() - 1);
  map_y_spinbutton->set_range (0, GameMap::getInstance()->getHeight() - 1);
  map_width_spinbutton->set_range (1, GameMap::getInstance()->getWidth());
  map_height_spinbutton->set_range (1, GameMap::getInstance()->getHeight());

  xml->get_widget("hidden_ruin_button", hidden_ruin_button);
  hidden_ruin_button->signal_clicked().connect (method(on_hidden_ruin_clicked));
  xml->get_widget("randomize_hidden_ruin_button", randomize_hidden_ruin_button);
  randomize_hidden_ruin_button->signal_clicked().connect
    (method(on_randomize_hidden_ruin_clicked));

  connect_signals ();
  update ();
}

void RewardEditorDialog::setup_rewards (Reward *r)
{
  if (r && r->getType () == Reward::GOLD)
    d_gold_reward = new Reward_Gold (*dynamic_cast<Reward_Gold*>(r));
  else
    d_gold_reward = new Reward_Gold (100);

  if (r && r->getType () == Reward::ALLIES)
    d_allies_reward = new Reward_Allies (*dynamic_cast<Reward_Allies*>(r));
  else
    d_allies_reward = new Reward_Allies ();

  if (r && r->getType () == Reward::ITEM)
    d_item_reward = new Reward_Item (*dynamic_cast<Reward_Item*>(r));
  else
    d_item_reward = new Reward_Item ();

  if (r && r->getType () == Reward::MAP)
    d_map_reward = new Reward_Map (*dynamic_cast<Reward_Map*>(r));
  else
    d_map_reward = new Reward_Map (Vector<int>(0,0), "", 1, 1);

  if (r && r->getType () == Reward::RUIN)
    d_ruin_reward = new Reward_Ruin (*dynamic_cast<Reward_Ruin*>(r));
  else
    d_ruin_reward = new Reward_Ruin ();

  if (!r)
    d_reward = d_gold_reward;
  else
    switch_reward_type (r->getType ());
}

RewardEditorDialog::~RewardEditorDialog ()
{
  delete d_gold_reward;
  delete d_allies_reward;
  delete d_item_reward;
  delete d_map_reward;
  delete d_ruin_reward;
  delete umgr;
  notebook->property_show_tabs () = false;
}

void RewardEditorDialog::fill_in_reward_info()
{
  if (d_reward->getType() == Reward::GOLD)
    {
      Reward_Gold *r = static_cast<Reward_Gold*>(d_reward);
      gold_spinbutton->set_value(r->getGold());
      reward_type_combobox->set_active (0);
    }
  else if (d_reward->getType() == Reward::ITEM)
    {
      update_item_name();
      reward_type_combobox->set_active (1);
    }
  else if (d_reward->getType() == Reward::ALLIES)
    {
      Reward_Allies *r = static_cast<Reward_Allies*>(d_reward);
      num_allies_spinbutton->set_value(r->getNoOfAllies());
      if (r->getArmy ())
        ally_button->select (r->getArmy()->getId ());
      else
        ally_button->clear_selected_army ();
      reward_type_combobox->set_active (2);
    }
  else if (d_reward->getType() == Reward::MAP)
    {
      Reward_Map *r = static_cast<Reward_Map*>(d_reward);
      map_x_spinbutton->set_value(r->getLocation().x);
      map_y_spinbutton->set_value(r->getLocation().y);
      map_width_spinbutton->set_value(r->getSightMap()->w);
      map_height_spinbutton->set_value(r->getSightMap()->h);
      reward_type_combobox->set_active (3);
    }
  else if (d_reward->getType() == Reward::RUIN)
    {
      update_hidden_ruin_name();
      reward_type_combobox->set_active (4);
    }
  notebook->property_page() = reward_type_combobox->get_active_row_number();
}

bool RewardEditorDialog::run()
{
  dialog->show_all();
  int response = dialog->run();
  d_reward->setName(d_reward->getDescription());
  if (response == Gtk::RESPONSE_REJECT)
    {
      d_reward = NULL;
      d_changed = true;
    }
  if (d_reward && d_reward->getName () == "")
    d_reward = NULL;

  return d_changed;
}

void RewardEditorDialog::on_randomize_gold_clicked()
{
  if (dynamic_cast<Reward_Gold*>(d_reward))
    {
      umgr->add (new RewardEditorAction_RandomizeGold (d_reward));
      Reward_Gold *r = dynamic_cast<Reward_Gold*>(d_reward);
      r->setGold (Reward_Gold::getRandomGoldPieces());
      d_changed = true;
    }
  update ();
}

void RewardEditorDialog::on_item_clicked()
{
  Reward_Item *r = dynamic_cast<Reward_Item*>(d_reward);
  SelectItemDialog d(*dialog, d_item_reward->getItem () != NULL);
  d.run();
  guint32 id = 0;
  const ItemProto *itemproto = d.get_selected_item(id);
  if (itemproto)
    {
      if (r->getItem ())
        {
          if (r->getItem ()->getId () != id)
            {
              umgr->add (new RewardEditorAction_Item (d_reward));
              on_clear_item_clicked();
              r->setItem (new Item(*itemproto, id));
              update_item_name();
            }
        }
      else
        {
          umgr->add (new RewardEditorAction_Item (d_reward));
          r->setItem (new Item(*itemproto, id));
          update_item_name();
        }
    }
  else
    {
      if (r->getItem () != NULL)
        {
          umgr->add (new RewardEditorAction_Item (d_reward));
          on_clear_item_clicked ();
        }
    }
  d_changed = true;
}

void RewardEditorDialog::on_clear_item_clicked()
{
  if (dynamic_cast<Reward_Item*>(d_reward))
    {
      Reward_Item *r = dynamic_cast<Reward_Item*>(d_reward);
      Item *item = r->getItem ();
      if (item)
        delete item;
      r->setItem (NULL);
    }
  update_item_name();
}

void RewardEditorDialog::on_randomize_item_clicked()
{
  if (dynamic_cast<Reward_Item*>(d_reward))
    {
      umgr->add (new RewardEditorAction_RandomizeItem (d_reward));
      on_clear_item_clicked();
      Reward_Item *r = dynamic_cast<Reward_Item*>(d_reward);
      r->setItem (Reward_Item::getRandomItem());
      d_changed = true;
      update ();
    }
}

void RewardEditorDialog::update_item_name()
{
  if (dynamic_cast<Reward_Item*>(d_reward))
    {
      Reward_Item *r = dynamic_cast<Reward_Item*>(d_reward);
      Glib::ustring name;
      if (r->getItem ())
        name = r->getItem ()->getName();
      else
        name = _("No item set");

      item_button->set_label(name);
    }
}

void RewardEditorDialog::on_ally_selected(const ArmyProto *a)
{
  if (dynamic_cast<Reward_Allies*>(d_reward))
    {
      Reward_Allies *r = dynamic_cast<Reward_Allies*> (d_reward);
      if (a)
        {
          if (a != r->getArmy())
            {
              umgr->add (new RewardEditorAction_AllyType (d_reward));
              d_changed = true;
            }
          r->clearAllies ();
          r->setArmy (a);
        }
      else
        {
          if (r->getArmy () != NULL)
            {
              umgr->add (new RewardEditorAction_AllyType (d_reward));
              d_changed = true;
              r->clearAllies ();
            }
        }
      update ();
    }
}

void RewardEditorDialog::on_randomize_allies_clicked()
{
  if (dynamic_cast<Reward_Allies*>(d_reward))
    {
      Reward_Allies *r = dynamic_cast<Reward_Allies*> (d_reward);
      const ArmyProto *a = Reward_Allies::randomArmyAlly();
      if (!a)
        return;
      umgr->add (new RewardEditorAction_RandomizeAlly (d_reward));
      r->clearAllies ();
      r->setArmy (a);
      r->setNoOfAllies (Reward_Allies::getRandomAmountOfAllies());
      d_changed = true;
    }
  update ();

}

void RewardEditorDialog::on_randomize_map_clicked()
{
  if (dynamic_cast<Reward_Map*>(d_reward))
    {
      umgr->add (new RewardEditorAction_RandomizeMap (d_reward));
      Reward_Map *r = dynamic_cast<Reward_Map*> (d_reward);
      int x, y, width, height;
      Reward_Map::getRandomMap(&x, &y, &width, &height);
      r->getSightMap ()->x = x;
      r->getSightMap ()->y = y;
      r->getSightMap ()->w = width;
      r->getSightMap ()->h = height;
      d_changed = true;
    }
  update ();
}

void RewardEditorDialog::on_hidden_ruin_clicked()
{
  SelectHiddenRuinDialog d(*dialog);
  d.run();
  if (d.get_selected_hidden_ruin())
    {
      on_clear_hidden_ruin_clicked();

      const Ruin *ruin = d.get_selected_hidden_ruin ();
      if (ruin == NULL)
        d_ruin_reward->clearRuin ();
      else
        d_ruin_reward->setRuinPos (ruin->getPos ());
      update_hidden_ruin_name();
    }
  else
    {
      on_clear_hidden_ruin_clicked();
      update_hidden_ruin_name();
    }
}

void RewardEditorDialog::on_clear_hidden_ruin_clicked()
{
  if (dynamic_cast<Reward_Ruin*>(d_reward))
    {
      Reward_Ruin *r = dynamic_cast<Reward_Ruin*>(d_reward);
      r->clearRuin ();
      update_hidden_ruin_name();
    }
}

void RewardEditorDialog::on_randomize_hidden_ruin_clicked()
{
  if (dynamic_cast<Reward_Ruin*>(d_reward))
    {
      Reward_Ruin *r = dynamic_cast<Reward_Ruin*>(d_reward);

      Ruin *ruin = Reward_Ruin::getRandomHiddenRuin();
      if (ruin)
        {
          umgr->add (new RewardEditorAction_RandomRuin (d_reward));
          on_clear_hidden_ruin_clicked();
          r->setRuinPos (ruin->getPos ());
          update_hidden_ruin_name();
        }
    }
}

void RewardEditorDialog::update_hidden_ruin_name()
{
  if (dynamic_cast<Reward_Ruin*>(d_reward))
    {
      Reward_Ruin *r = dynamic_cast<Reward_Ruin*>(d_reward);
      Glib::ustring name;
      if (r->getRuin ())
        name = r->getRuin ()->getName();
      else
        name = _("No Ruin");
      hidden_ruin_button->set_label(name);
    }
}

bool RewardEditorDialog::switch_reward_type (Reward::Type t)
{
  if (t != d_reward->getType ())
    {
      switch (t)
        {
        case Reward::GOLD: d_reward = d_gold_reward; break;
        case Reward::ALLIES: d_reward = d_allies_reward; break;
        case Reward::ITEM: d_reward = d_item_reward; break;
        case Reward::MAP: d_reward = d_map_reward; break;
        case Reward::RUIN: d_reward = d_ruin_reward; break;
        }
      return true;
    }
  return false;
}

Reward::Type RewardEditorDialog::row_to_reward_type (int row)
{
  //sync up with the list store .glade file
  switch (row)
    {
    case 0:
      return Reward::GOLD;
    case 1:
      return Reward::ITEM;
    case 2:
      return Reward::ALLIES;
    case 3:
      return Reward::MAP;
    case 4:
      return Reward::RUIN;
    default:
      return Reward::GOLD;
    }
}

void RewardEditorDialog::on_type_changed()
{
  Reward::Type new_type =
    row_to_reward_type (reward_type_combobox->get_active_row_number ());

  notebook->property_page() = reward_type_combobox->get_active_row_number();
  RewardEditorAction_Type *action = new RewardEditorAction_Type (d_reward);
  if (switch_reward_type (new_type))
    {
      umgr->add (action);
      d_changed = true;
      update ();
    }
  else
    delete action;
}

void RewardEditorDialog::on_undo_activated ()
{
  umgr->undo ();
  if (umgr->undoEmpty ())
    d_changed = false;
  update ();
}

void RewardEditorDialog::on_redo_activated ()
{
  d_changed = true;
  umgr->redo ();
  update ();
}

void RewardEditorDialog::update ()
{
  disconnect_signals ();
  fill_in_reward_info ();
  connect_signals ();
}

void RewardEditorDialog::on_map_x_text_changed ()
{
  int value = atoi(map_x_spinbutton->get_text().c_str());
  int newvalue = value;
  int max = map_width_spinbutton->get_value () + GameMap::getWidth ();
  if (value >= max)
    newvalue = max - 1;
  else if (value < 0)
    newvalue = 0;

  Reward_Map *r = dynamic_cast<Reward_Map*>(d_reward);
  if (r->getSightMap ()->x != newvalue)
    {
      umgr->add (new RewardEditorAction_XCoord (d_reward));
      r->getSightMap ()->x = newvalue;
      d_changed = true;
    }
  update ();
}

void RewardEditorDialog::on_map_y_text_changed ()
{
  int value = atoi(map_y_spinbutton->get_text().c_str());
  int max = map_height_spinbutton->get_value () + GameMap::getHeight ();
  int newvalue = value;
  if (value >= max)
    newvalue = max - 1;
  else if (value < 0)
    newvalue = 0;
  Reward_Map *r = dynamic_cast<Reward_Map*>(d_reward);
  if (r->getSightMap ()->y != newvalue)
    {
      umgr->add (new RewardEditorAction_YCoord (d_reward));
      r->getSightMap ()->y = newvalue;
      d_changed = true;
    }
  update ();
}

void RewardEditorDialog::on_map_width_text_changed ()
{
  int value = atoi(map_width_spinbutton->get_text().c_str());
  int max = GameMap::getWidth () - map_x_spinbutton->get_value ();
  int newvalue = value;
  if (value >= max)
    newvalue = max - 1;
  else if (value < 1)
    newvalue = 1;
  Reward_Map *r = dynamic_cast<Reward_Map*>(d_reward);
  if (r->getSightMap ()->w != newvalue)
    {
      umgr->add (new RewardEditorAction_Width (d_reward));
      r->getSightMap ()->w = newvalue;
      d_changed = true;
    }
  update ();
}

void RewardEditorDialog::on_map_height_text_changed ()
{
  int value = atoi(map_height_spinbutton->get_text().c_str());
  int max = GameMap::getHeight () - map_y_spinbutton->get_value ();
  int newvalue = value;
  if (value >= max)
    newvalue = max - 1;
  else if (value < 1)
    newvalue = 1;
  Reward_Map *r = dynamic_cast<Reward_Map*>(d_reward);
  if (r->getSightMap ()->h != newvalue)
    {
      umgr->add (new RewardEditorAction_Height (d_reward));
      r->getSightMap ()->h = newvalue;
      d_changed = true;
    }
  update ();
}

void RewardEditorDialog::on_num_allies_text_changed ()
{
  guint32 value = atoi(num_allies_spinbutton->get_text().c_str());
  Reward_Allies *r = dynamic_cast<Reward_Allies*>(d_reward);
  if (r->getNoOfAllies () != value)
    {
      umgr->add (new RewardEditorAction_AllyCount (d_reward));
      r->setNoOfAllies (value);
      d_changed = true;
    }
  update ();
}

void RewardEditorDialog::on_gold_text_changed ()
{
  guint32 value = atoi(gold_spinbutton->get_text().c_str());
  Reward_Gold *r = dynamic_cast<Reward_Gold*>(d_reward);
  if (r->getGold () != value)
    {
      umgr->add (new RewardEditorAction_Gold (d_reward));
      r->setGold (value);
      d_changed = true;
    }
  update ();
}

void RewardEditorDialog::connect_signals ()
{
  connections.push_back
    (reward_type_combobox->signal_changed().connect (method(on_type_changed)));
  connections.push_back
    (ally_button->army_selected.connect (method (on_ally_selected)));
  connections.push_back
    (map_x_spinbutton->signal_insert_text().connect
     (sigc::hide(sigc::hide(method(on_map_x_text_changed)))));
  connections.push_back
    (map_y_spinbutton->signal_insert_text().connect
     (sigc::hide(sigc::hide(method(on_map_y_text_changed)))));
  connections.push_back
    (map_width_spinbutton->signal_insert_text().connect
     (sigc::hide(sigc::hide(method(on_map_width_text_changed)))));
  connections.push_back
    (map_height_spinbutton->signal_insert_text().connect
     (sigc::hide(sigc::hide(method(on_map_height_text_changed)))));
  connections.push_back
    (num_allies_spinbutton->signal_insert_text().connect
     (sigc::hide(sigc::hide(method(on_num_allies_text_changed)))));
  connections.push_back
    (gold_spinbutton->signal_insert_text().connect
     (sigc::hide(sigc::hide(method(on_gold_text_changed)))));
}

void RewardEditorDialog::disconnect_signals ()
{
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}

UndoAction*
RewardEditorDialog::executeAction (UndoAction *action2)
{
  RewardEditorAction *action = dynamic_cast<RewardEditorAction*>(action2);
  UndoAction *out = NULL;

    switch (action->getType ())
      {
      case RewardEditorAction::TYPE:
          {
            RewardEditorAction_Type *a =
              dynamic_cast<RewardEditorAction_Type*>(action);
            out = new RewardEditorAction_Type (d_reward);
            switch_reward_type (a->getReward ()->getType ());
          }
        break;
      case RewardEditorAction::GOLD_PIECES:
          {
            RewardEditorAction_Gold *a =
              dynamic_cast<RewardEditorAction_Gold*>(action);
            out = new RewardEditorAction_Gold (d_reward);
            dynamic_cast<Reward_Gold*>(d_reward)->setGold
              (dynamic_cast<Reward_Gold*>(a->getReward ())->getGold ());
          }
        break;
      case RewardEditorAction::RANDOMIZE_GOLD:
          {
            RewardEditorAction_RandomizeGold *a =
              dynamic_cast<RewardEditorAction_RandomizeGold*>(action);
            out = new RewardEditorAction_RandomizeGold (d_reward);
            dynamic_cast<Reward_Gold*>(d_reward)->setGold
              (dynamic_cast<Reward_Gold*>(a->getReward ())->getGold ());
          }
        break;
      case RewardEditorAction::ITEM:
          {
            RewardEditorAction_Item *a =
              dynamic_cast<RewardEditorAction_Item*>(action);
            out = new RewardEditorAction_Item (d_reward);
            Reward_Item *r = dynamic_cast<Reward_Item*>(d_reward);
            if (r->getItem ())
              delete r->getItem ();
            Reward_Item *newr = dynamic_cast<Reward_Item*>(a->getReward ());
            if (newr->getItem ())
              r->setItem (new Item (*newr->getItem ()));
            else
              r->setItem (NULL);
          }
        break;
      case RewardEditorAction::RANDOMIZE_ITEM:
          {
            RewardEditorAction_RandomizeItem *a =
              dynamic_cast<RewardEditorAction_RandomizeItem*>(action);
            out = new RewardEditorAction_RandomizeItem (d_reward);
            Reward_Item *r = dynamic_cast<Reward_Item*>(d_reward);
            if (r->getItem ())
              delete r->getItem ();
            Reward_Item *newr = dynamic_cast<Reward_Item*>(a->getReward ());
            if (newr->getItem ())
              r->setItem (new Item (*newr->getItem ()));
            else
              r->setItem (NULL);
          }
        break;
      case RewardEditorAction::ALLY_TYPE:
          {
            RewardEditorAction_AllyType *a =
              dynamic_cast<RewardEditorAction_AllyType*>(action);
            out = new RewardEditorAction_AllyType (d_reward);
            Reward_Allies *r = dynamic_cast<Reward_Allies*>(d_reward);
            r->setArmy
              (dynamic_cast<Reward_Allies*>(a->getReward ())->getArmy ());
          }
        break;
      case RewardEditorAction::RANDOMIZE_ALLY:
          {
            RewardEditorAction_RandomizeAlly *a =
              dynamic_cast<RewardEditorAction_RandomizeAlly*>(action);
            out = new RewardEditorAction_RandomizeAlly (d_reward);
            Reward_Allies *r = dynamic_cast<Reward_Allies*>(d_reward);
            r->setArmy
              (dynamic_cast<Reward_Allies*>(a->getReward ())->getArmy ());
            dynamic_cast<Reward_Allies*>(d_reward)->setNoOfAllies
              (dynamic_cast<Reward_Allies*>(a->getReward ())->getNoOfAllies ());
          }
        break;
      case RewardEditorAction::ALLY_COUNT:
          {
            RewardEditorAction_AllyCount *a =
              dynamic_cast<RewardEditorAction_AllyCount*>(action);
            out = new RewardEditorAction_AllyCount (d_reward);
            dynamic_cast<Reward_Allies*>(d_reward)->setNoOfAllies
              (dynamic_cast<Reward_Allies*>(a->getReward ())->getNoOfAllies ());
          }
        break;
      case RewardEditorAction::XCOORD:
          {
            RewardEditorAction_XCoord *a =
              dynamic_cast<RewardEditorAction_XCoord*>(action);
            out = new RewardEditorAction_XCoord (d_reward);
            dynamic_cast<Reward_Map*>(d_reward)->getSightMap ()->x =
              (dynamic_cast<Reward_Map*>(a->getReward ())->getSightMap()->x);
          }
        break;
      case RewardEditorAction::YCOORD:
          {
            RewardEditorAction_YCoord *a =
              dynamic_cast<RewardEditorAction_YCoord*>(action);
            out = new RewardEditorAction_YCoord (d_reward);
            dynamic_cast<Reward_Map*>(d_reward)->getSightMap ()->y =
              (dynamic_cast<Reward_Map*>(a->getReward ())->getSightMap()->y);
          }
        break;
      case RewardEditorAction::WIDTH:
          {
            RewardEditorAction_Width *a =
              dynamic_cast<RewardEditorAction_Width*>(action);
            out = new RewardEditorAction_Width (d_reward);
            dynamic_cast<Reward_Map*>(d_reward)->getSightMap ()->w =
              (dynamic_cast<Reward_Map*>(a->getReward ())->getSightMap()->w);
          }
        break;
      case RewardEditorAction::HEIGHT:
          {
            RewardEditorAction_Height *a =
              dynamic_cast<RewardEditorAction_Height*>(action);
            out = new RewardEditorAction_Height (d_reward);
            dynamic_cast<Reward_Map*>(d_reward)->getSightMap ()->h =
              (dynamic_cast<Reward_Map*>(a->getReward ())->getSightMap()->h);
          }
        break;
      case RewardEditorAction::RANDOMIZE_MAP:
          {
            RewardEditorAction_RandomizeMap *a =
              dynamic_cast<RewardEditorAction_RandomizeMap*>(action);
            out = new RewardEditorAction_RandomizeMap (d_reward);
            dynamic_cast<Reward_Map*>(d_reward)->getSightMap ()->x =
              (dynamic_cast<Reward_Map*>(a->getReward ())->getSightMap()->x);
            dynamic_cast<Reward_Map*>(d_reward)->getSightMap ()->y =
              (dynamic_cast<Reward_Map*>(a->getReward ())->getSightMap()->y);
            dynamic_cast<Reward_Map*>(d_reward)->getSightMap ()->w =
              (dynamic_cast<Reward_Map*>(a->getReward ())->getSightMap()->w);
            dynamic_cast<Reward_Map*>(d_reward)->getSightMap ()->h =
              (dynamic_cast<Reward_Map*>(a->getReward ())->getSightMap()->h);
          }
        break;
      case RewardEditorAction::HIDDEN_RUIN:
          {
            RewardEditorAction_HiddenRuin *a =
              dynamic_cast<RewardEditorAction_HiddenRuin*>(action);
            out = new RewardEditorAction_HiddenRuin (d_reward);
            Reward_Ruin *r = dynamic_cast<Reward_Ruin*>(a->getReward ());
            if (r->getRuin ())
              dynamic_cast<Reward_Ruin*>(d_reward)->setRuinPos
                (r->getRuin ()->getPos ());
            else
              dynamic_cast<Reward_Ruin*>(d_reward)->clearRuin ();
          }
        break;
      case RewardEditorAction::RANDOM_RUIN:
          {
            RewardEditorAction_RandomRuin *a =
              dynamic_cast<RewardEditorAction_RandomRuin*>(action);
            out = new RewardEditorAction_RandomRuin (d_reward);
            Reward_Ruin *r = dynamic_cast<Reward_Ruin*>(a->getReward ());
            if (r->getRuin ())
              dynamic_cast<Reward_Ruin*>(d_reward)->setRuinPos
                (r->getRuin ()->getPos ());
            else
              dynamic_cast<Reward_Ruin*>(d_reward)->clearRuin ();
          }
        break;
      }
    return out;
}
