//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2012, 2014, 2017, 2020, 2021 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>
#include <gtkmm.h>

#include "ruin-editor-dialog.h"

#include "ucompose.hpp"
#include "playerlist.h"
#include "CreateScenarioRandomize.h"
#include "defs.h"
#include "ruin.h"
#include "stack.h"
#include "army.h"
#include "reward.h"
#include "rewardlist.h"

#include "select-army-dialog.h"
#include "rewardlist-dialog.h"
#include "RenamableLocation.h"
#include "keeper-editor-dialog.h"
#include "keeper.h"
#include "undo-mgr.h"
#include "ruin-editor-actions.h"
#include "reward-editor-dialog.h"

#define method(x) sigc::mem_fun(*this, &RuinEditorDialog::x)

RuinEditorDialog::RuinEditorDialog(Gtk::Window &parent, Ruin *r, CreateScenarioRandomize *randomizer)
 : LwEditorDialog(parent, "ruin-editor-dialog.ui")
{
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));
  d_changed = false;
  d_randomizer = randomizer;
  ruin = r;

  xml->get_widget("name_entry", name_entry);
  umgr->addCursor (name_entry);
  xml->get_widget("description_entry", description_entry);
  umgr->addCursor (description_entry);
  xml->get_widget("type_spinbutton", type_spinbutton);
  xml->get_widget("keeper_button", keeper_button);
  keeper_button->signal_clicked().connect(method(on_keeper_clicked));
  xml->get_widget("new_keeper_hbox", new_keeper_hbox);
  xml->get_widget("random_keeper_switch", random_keeper_switch);
  xml->get_widget("randomize_name_button", randomize_name_button);
  randomize_name_button->signal_clicked().connect(method(on_randomize_name_clicked));

  xml->get_widget("hidden_switch", hidden_switch);
  // setup the player combo
  player_combobox = manage(new Gtk::ComboBoxText);

  for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
       end = Playerlist::getInstance()->end(); i != end; ++i)
    {
      if ((*i) != Playerlist::getInstance ()->getNeutral ())
        player_combobox->append((*i)->getName());
      else
        player_combobox->append(_("Hero's player"));
    }

  Gtk::Alignment *alignment;
  xml->get_widget("player_alignment", alignment);
  alignment->add(*player_combobox);
  xml->get_widget("new_reward_hbox", new_reward_hbox);
  xml->get_widget("random_reward_switch", random_reward_switch);
  d_random_reward_active = ruin->getReward () == NULL;
  xml->get_widget("reward_button", reward_button);
  reward_button->signal_clicked().connect(method(on_reward_clicked));
  xml->get_widget("undo_button", undo_button);
  undo_button->signal_activate ().connect (method (on_undo_activated));
  xml->get_widget("redo_button", redo_button);
  redo_button->signal_activate ().connect (method (on_redo_activated));

  connect_signals ();
  update ();
}

RuinEditorDialog::~RuinEditorDialog()
{
  delete umgr;
}

bool RuinEditorDialog::run()
{
  dialog->show_all();
  dialog->run();
  return d_changed;
}

void RuinEditorDialog::update_keeper_name()
{
  Glib::ustring name;
  Keeper *keeper = ruin->getOccupant ();
  if (keeper)
    {
      if (keeper->getName () == "")
        name = _("No keeper");
      else
        name = keeper->getName ();
    }
  else
    name = _("No keeper");

  keeper_button->set_label(name);
}

void RuinEditorDialog::on_hidden_toggled()
{
  umgr->add (new RuinEditorAction_OnlySeenBy (ruin));
  ruin->setHidden(hidden_switch->get_active());
  if (hidden_switch->get_active ())
    ruin->setOwner (Playerlist::getInstance ()->getNeutral ());
  update ();
}

void RuinEditorDialog::update_hidden_status ()
{
  d_changed = true;
  ruin->setHidden(hidden_switch->get_active());
  if (hidden_switch->get_active())
    {
      // set owner
      int c = 0, row = player_combobox->get_active_row_number();
      Player *player = Playerlist::getInstance()->getNeutral();
      for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
           end = Playerlist::getInstance()->end(); i != end; ++i, ++c)
        if (c == row)
          {
            player = *i;
            break;
          }
      ruin->setOwner(player);
    }
  else
    {
      int c = 0;
      for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
           end = Playerlist::getInstance()->end(); i != end; ++i, ++c)
        if (*i== Playerlist::getInstance()->getNeutral())
          player_combobox->set_active (c);
      ruin->setOwner(NULL);
    }
}

void RuinEditorDialog::on_keeper_clicked()
{
  umgr->add (new RuinEditorAction_Keeper (ruin));
  KeeperEditorDialog d(*dialog, ruin->getOccupant (), ruin->getPos (),
                       d_randomizer);

  if (d.run())
    {
      d_changed = true;
      Keeper *keeper = d.get_keeper ();
      ruin->setOccupant (keeper);
      update ();
    }
}

void RuinEditorDialog::on_randomize_name_clicked()
{
  umgr->add (new RuinEditorAction_RandomizeName (ruin));
  Glib::ustring existing_name = name_entry->get_text();
  if (existing_name == Ruin::getDefaultName())
    ruin->setName(d_randomizer->popRandomRuinName());
  else
    {
      ruin->setName(d_randomizer->popRandomRuinName());
      d_randomizer->pushRandomRuinName(existing_name);
    }
  update ();
}

void RuinEditorDialog::on_new_keeper_toggled()
{
  umgr->add (new RuinEditorAction_RandomKeeper (ruin));
  d_changed = true;
  if (random_keeper_switch->get_active () == true)
    ruin->setOccupant (NULL);
  else
    {
      Keeper *keeper = new Keeper (NULL, ruin->getPos ());
      ruin->setOccupant (keeper);
    }
  update ();
}

void RuinEditorDialog::on_new_reward_toggled()
{
  umgr->add (new RuinEditorAction_RandomReward (ruin, d_random_reward_active));
  d_random_reward_active = random_reward_switch->get_active ();
  if (random_reward_switch->get_active () == true)
    {
      if (ruin->getReward () != NULL)
        d_changed = true;
      ruin->setReward (NULL);
      update_reward_name ();
    }
  new_reward_hbox->set_sensitive(!random_reward_switch->get_active());
}

void RuinEditorDialog::on_reward_clicked()
{
  RuinEditorAction_Reward *action = new RuinEditorAction_Reward (ruin);
  if (ruin->getReward ())
    {
      Player *neutral = Playerlist::getInstance()->getNeutral();
      RewardEditorDialog d (*dialog, neutral, true, ruin->getReward ());
      if (d.run ())
        {
          if (d.get_reward ())
            {
              umgr->add (action);
              ruin->setReward (Reward::copy (d.get_reward ()));
              d_changed = true;
            }
          else
            {
              if (ruin->getReward () != NULL)
                {
                  umgr->add (action);
                  ruin->setReward (NULL);
                  d_changed = true;
                }
              else
                delete action;
            }
        }
      else
        delete action;
    }
  else
    {
      RewardlistDialog d(*dialog, true, false);
      bool changed = d.run();
      umgr->add (action);
      if (changed)
        d_changed = true;
      if (d.get_reward ())
        {
          d_changed = true;
          ruin->setReward (Reward::copy (d.get_reward ()));
          Rewardlist::getInstance()->deleteReward (d.get_reward ());
        }
      else
        {
          ruin->setReward (NULL);
        }
    }

  update ();
}

void RuinEditorDialog::update_reward_name()
{
  Reward *reward = ruin->getReward ();
  Glib::ustring name;
  if (reward)
    name = reward->getName();
  else
    name = _("No reward");

  reward_button->set_label(name);
}

void RuinEditorDialog::on_name_changed ()
{
  umgr->add (new RuinEditorAction_Name (ruin, umgr, name_entry));
  d_changed = true;
  Location *l = ruin;
  RenamableLocation *renamable_ruin = static_cast<RenamableLocation*>(l);
  renamable_ruin->setName(name_entry->get_text());
  update ();
}

void RuinEditorDialog::on_description_changed ()
{
  umgr->add (new RuinEditorAction_Description (ruin, umgr, description_entry));
  d_changed = true;
  ruin->setDescription(description_entry->get_text());
  update ();
}

void RuinEditorDialog::on_type_changed ()
{
  d_changed = true;
  if (type_spinbutton->get_value() >= RUIN_TYPES)
    type_spinbutton->set_value(RUIN_TYPES - 1);
  else
    ruin->setType(int(type_spinbutton->get_value()));
}

void RuinEditorDialog::on_type_text_changed ()
{
  umgr->add (new RuinEditorAction_Type (ruin));
  type_spinbutton->set_value(atoi(type_spinbutton->get_text().c_str()));
  on_type_changed();
  update ();
}

void RuinEditorDialog::on_hidden_ruin_player_changed ()
{
  umgr->add (new RuinEditorAction_OnlySeenPlayer (ruin));
  update_hidden_status ();
}

void RuinEditorDialog::on_undo_activated ()
{
  umgr->undo ();
  if (umgr->undoEmpty ())
    d_changed = false;
  update ();
  return;
}

void RuinEditorDialog::on_redo_activated ()
{
  umgr->redo ();
  d_changed = true;
  update ();
}

void RuinEditorDialog::update ()
{
  disconnect_signals ();
  update_reward_name();
  update_keeper_name();
  name_entry->set_text(ruin->getName());
  description_entry->set_text(ruin->getDescription());
  type_spinbutton->set_value(ruin->getType());
  random_keeper_switch->set_active (ruin->getOccupant () == NULL);
  hidden_switch->set_active(ruin->isHidden());
  player_combobox->set_sensitive (hidden_switch->get_active ());
  random_reward_switch->set_active (d_random_reward_active);
  int c = 0, player_no = 0;
  for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
       end = Playerlist::getInstance()->end(); i != end; ++i, ++c)
    {
      Player *player = *i;
      if (player == ruin->getOwner())
        player_no = c;
    }

  player_combobox->set_active(player_no);
  new_reward_hbox->set_sensitive(!random_reward_switch->get_active());
  new_keeper_hbox->set_sensitive(!random_keeper_switch->get_active());

  umgr->setCursors ();
  connect_signals ();
}

void RuinEditorDialog::disconnect_signals ()
{
  umgr->disconnect_signals ();
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}

void RuinEditorDialog::connect_signals ()
{
  umgr->connect_signals ();
  connections.push_back
    (name_entry->signal_changed ().connect (method (on_name_changed)));
  connections.push_back
    (description_entry->signal_changed ().connect
     (method (on_description_changed)));
  connections.push_back
    (random_keeper_switch->property_active().signal_changed().connect
     (method(on_new_keeper_toggled)));
  connections.push_back
    (hidden_switch->property_active().signal_changed().connect
     (method(on_hidden_toggled)));
  connections.push_back
    (player_combobox->signal_changed ().connect
     (method (on_hidden_ruin_player_changed)));
  connections.push_back
    (random_reward_switch->property_active().signal_changed().connect
     (method(on_new_reward_toggled)));
  connections.push_back
    (type_spinbutton->signal_insert_text().connect
     (sigc::hide(sigc::hide(method(on_type_text_changed)))));
}

UndoAction*
RuinEditorDialog::executeAction (UndoAction *action2)
{
  RuinEditorAction *action = dynamic_cast<RuinEditorAction*>(action2);
  UndoAction *out = NULL;

    switch (action->getType ())
      {
      case RuinEditorAction::NAME:
          {
            RuinEditorAction_Name *a =
              dynamic_cast<RuinEditorAction_Name*>(action);
            out = new RuinEditorAction_Name (ruin, umgr, name_entry);
            ruin->setName (a->getRuin ()->getName ());
          }
        break;
      case RuinEditorAction::RANDOMIZE_NAME:
          {
            RuinEditorAction_RandomizeName *a =
              dynamic_cast<RuinEditorAction_RandomizeName*>(action);
            out = new RuinEditorAction_RandomizeName (ruin);
            ruin->setName (a->getRuin ()->getName ());
          }
        break;
      case RuinEditorAction::DESCRIPTION:
          {
            RuinEditorAction_Description *a =
              dynamic_cast<RuinEditorAction_Description*>(action);
            out = new RuinEditorAction_Description (ruin, umgr,
                                                    description_entry);
            ruin->setDescription (a->getRuin ()->getDescription ());
          }
        break;
      case RuinEditorAction::RANDOM_KEEPER:
          {
            RuinEditorAction_RandomKeeper *a =
              dynamic_cast<RuinEditorAction_RandomKeeper*>(action);
            out = new RuinEditorAction_RandomKeeper (ruin);
            Keeper *keeper = a->getRuin ()->getOccupant ();
            if (keeper)
              ruin->setOccupant (new Keeper (*keeper));
            else
              ruin->setOccupant (NULL);
          }
        break;
      case RuinEditorAction::KEEPER:
          {
            RuinEditorAction_Keeper *a =
              dynamic_cast<RuinEditorAction_Keeper*>(action);
            out = new RuinEditorAction_Keeper (ruin);
            Keeper *keeper = a->getRuin ()->getOccupant ();
            if (keeper)
              ruin->setOccupant (new Keeper (*keeper));
            else
              ruin->setOccupant (NULL);
          }
        break;
      case RuinEditorAction::ONLY_SEEN_BY:
          {
            RuinEditorAction_OnlySeenBy *a =
              dynamic_cast<RuinEditorAction_OnlySeenBy*>(action);
            out = new RuinEditorAction_OnlySeenBy (ruin);
            ruin->setHidden (a->getRuin ()->isHidden ());
            ruin->setOwner (a->getRuin ()->getOwner ());
          }
        break;
      case RuinEditorAction::ONLY_SEEN_PLAYER:
          {
            RuinEditorAction_OnlySeenPlayer *a =
              dynamic_cast<RuinEditorAction_OnlySeenPlayer*>(action);
            out = new RuinEditorAction_OnlySeenPlayer (ruin);
            ruin->setOwner (a->getRuin ()->getOwner ());
          }
        break;
      case RuinEditorAction::TYPE:
          {
            RuinEditorAction_Type *a =
              dynamic_cast<RuinEditorAction_Type*>(action);
            out = new RuinEditorAction_Type (ruin);
            ruin->setType (a->getRuin ()->getType ());
          }
        break;
      case RuinEditorAction::RANDOM_REWARD:
          {
            RuinEditorAction_RandomReward *a =
              dynamic_cast<RuinEditorAction_RandomReward*>(action);
            out = new RuinEditorAction_RandomReward (ruin,
                                                     d_random_reward_active);
            disconnect_signals ();
            random_reward_switch->set_active (a->getActive ());
            d_random_reward_active = a->getActive ();
            Reward *reward = a->getRuin ()->getReward ();
            if (!reward)
              ruin->setReward (reward);
            else
              ruin->setReward (Reward::copy (reward));
            connect_signals ();
          }
        break;
      case RuinEditorAction::REWARD:
          {
            RuinEditorAction_Reward *a =
              dynamic_cast<RuinEditorAction_Reward*>(action);
            out = new RuinEditorAction_Reward (ruin);
            Reward *reward = a->getRuin ()->getReward ();
            if (!reward)
              ruin->setReward (reward);
            else
              ruin->setReward (Reward::copy (reward));
          }
        break;
      }
    return out;
}
