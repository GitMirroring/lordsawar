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

#define method(x) sigc::mem_fun(*this, &RuinEditorDialog::x)

RuinEditorDialog::RuinEditorDialog(Gtk::Window &parent, Ruin *r, CreateScenarioRandomize *randomizer)
 : LwEditorDialog(parent, "ruin-editor-dialog.ui")
{
  d_changed = false;
  d_randomizer = randomizer;
  ruin = r;

  xml->get_widget("name_entry", name_entry);
  name_entry->set_text(ruin->getName());
  name_entry->signal_changed ().connect (method (on_name_changed));
  xml->get_widget("description_entry", description_entry);
  description_entry->set_text(ruin->getDescription());
  description_entry->signal_changed ().connect
    (method (on_description_changed));

  xml->get_widget("type_spinbutton", type_spinbutton);
  type_spinbutton->set_value(ruin->getType());
  type_spinbutton->signal_insert_text().connect
    (sigc::hide(sigc::hide(method(on_type_text_changed))));

  xml->get_widget("keeper_button", keeper_button);
  keeper_button->signal_clicked().connect(method(on_keeper_clicked));
  xml->get_widget("new_keeper_hbox", new_keeper_hbox);
  xml->get_widget("random_keeper_switch", random_keeper_switch);
  random_keeper_switch->set_active (ruin->getOccupant () == NULL);
  random_keeper_switch->property_active().signal_changed().connect(method(on_new_keeper_toggled));

  xml->get_widget("randomize_name_button", randomize_name_button);
  randomize_name_button->signal_clicked().connect(method(on_randomize_name_clicked));

  set_keeper_name();

  xml->get_widget("hidden_switch", hidden_switch);
  hidden_switch->set_active(ruin->isHidden());
  hidden_switch->property_active().signal_changed().connect(method(on_hidden_toggled));
  // setup the player combo
  player_combobox = manage(new Gtk::ComboBoxText);

  int c = 0, player_no = 0;
  for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
       end = Playerlist::getInstance()->end(); i != end; ++i, ++c)
    {
      Player *player = *i;
      player_combobox->append(player->getName());
      if (player == ruin->getOwner())
        player_no = c;
    }

  player_combobox->set_active(player_no);
  player_combobox->signal_changed ().connect
    (method (on_hidden_ruin_player_changed));


  Gtk::Alignment *alignment;
  xml->get_widget("player_alignment", alignment);
  alignment->add(*player_combobox);
  player_combobox->set_sensitive (hidden_switch->get_active ());

  xml->get_widget("new_reward_hbox", new_reward_hbox);
  xml->get_widget("random_reward_switch", random_reward_switch);
  random_reward_switch->set_active (ruin->getReward () == NULL);
  random_reward_switch->property_active().signal_changed().connect(method(on_new_reward_toggled));

  xml->get_widget("reward_button", reward_button);
  reward_button->signal_clicked().connect(method(on_reward_clicked));

  set_reward_name();
}

bool RuinEditorDialog::run()
{
  dialog->show_all();
  dialog->run();
  return d_changed;
}

void RuinEditorDialog::set_keeper_name()
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
  if (hidden_switch->get_active())
    player_combobox->set_sensitive (true);
  else
    player_combobox->set_sensitive (false);
  update_hidden_status ();
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
  KeeperEditorDialog d(*dialog, ruin->getOccupant (), ruin->getPos (),
                       d_randomizer);

  if (d.run())
    {
      d_changed = true;
      Keeper *keeper = d.get_keeper ();
      ruin->setOccupant (keeper);
      set_keeper_name();
      if (keeper == NULL)
        random_keeper_switch->set_active (true);
    }
}

void RuinEditorDialog::on_randomize_name_clicked()
{
  Glib::ustring existing_name = name_entry->get_text();
  if (existing_name == Ruin::getDefaultName())
    name_entry->set_text(d_randomizer->popRandomRuinName());
  else
    {
      name_entry->set_text(d_randomizer->popRandomRuinName());
      d_randomizer->pushRandomRuinName(existing_name);
    }
}

void RuinEditorDialog::on_new_keeper_toggled()
{
  d_changed = true;
  if (random_keeper_switch->get_active () == true)
    {
      ruin->setOccupant (NULL);
      set_keeper_name();
    }
  else
    {
      Keeper *keeper = new Keeper (NULL, ruin->getPos ());
      ruin->setOccupant (keeper);
      set_keeper_name();
    }
  new_keeper_hbox->set_sensitive(!random_keeper_switch->get_active());
}

void RuinEditorDialog::on_new_reward_toggled()
{
  if (random_reward_switch->get_active () == true)
    {
      if (ruin->getReward () != NULL)
        d_changed = true;
      ruin->setReward (NULL);
      set_reward_name();
    }
  new_reward_hbox->set_sensitive(!random_reward_switch->get_active());
}

void RuinEditorDialog::on_reward_clicked()
{
  //d_changed = true;
  //this is a dog's breakfast right here.  wow.
  //ruin rewards are not in the rewards list, so we have to push it on
  //and off.
  //but the edit in the reward list editor can make it go away,
  //so we have to be careful about dangling pointers.
  if (ruin->getReward ())
    {
      Reward *copy = Reward::copy (ruin->getReward ());
      Rewardlist::getInstance ()->push_front (copy);
      RewardlistDialog d(*dialog, true, true);
      bool changed = d.run();
      if (changed)
        d_changed = true;
      if (d.get_reward())
        {
          d_changed = true;
          ruin->setReward (Reward::copy (d.get_reward ()));
          //if (d.get_reward () != Rewardlist::getInstance ()->front ())
          //Rewardlist::getInstance()->deleteReward
          //(Rewardlist::getInstance()->front ());
          Rewardlist::getInstance()->deleteReward (d.get_reward ());
        }
      else
        {
          if (ruin->getReward () != NULL)
            d_changed = true;
          ruin->setReward (NULL);
          Rewardlist::getInstance()->deleteReward
            (Rewardlist::getInstance()->front ());
          random_reward_switch->set_active (true);
        }
    }
  else
    {
      RewardlistDialog d(*dialog, true, false);
      bool changed = d.run();
      if (changed)
        d_changed = true;
      if (d.get_reward ())
        {
          d_changed = true;
          ruin->setReward (Reward::copy (d.get_reward ()));
          Rewardlist::getInstance()->deleteReward (d.get_reward ());
        }
      else
        random_reward_switch->set_active (true);
    }

  set_reward_name();
}

void RuinEditorDialog::set_reward_name()
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
  d_changed = true;
  Location *l = ruin;
  RenamableLocation *renamable_ruin = static_cast<RenamableLocation*>(l);
  renamable_ruin->setName(name_entry->get_text());
}

void RuinEditorDialog::on_description_changed ()
{
  d_changed = true;
  ruin->setDescription(description_entry->get_text());
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
  type_spinbutton->set_value(atoi(type_spinbutton->get_text().c_str()));
  on_type_changed();
}

void RuinEditorDialog::on_hidden_ruin_player_changed ()
{
  update_hidden_status ();
}
