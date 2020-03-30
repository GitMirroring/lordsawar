//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2012, 2014, 2017, 2020 Ben Asselstine
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

#include "select-army-dialog.h"
#include "select-reward-dialog.h"
#include "reward-editor-dialog.h"
#include "RenamableLocation.h"

#define method(x) sigc::mem_fun(*this, &RuinEditorDialog::x)

RuinEditorDialog::RuinEditorDialog(Gtk::Window &parent, Ruin *r, CreateScenarioRandomize *randomizer)
 : LwEditorDialog(parent, "ruin-editor-dialog.ui")
{
  d_randomizer = randomizer;
  ruin = r;
  reward = NULL;

  xml->get_widget("name_entry", name_entry);
  name_entry->set_max_length (MAX_LENGTH_FOR_RUIN_NAME);
  name_entry->signal_changed ().connect (method (on_name_changed));
  name_entry->set_text(ruin->getName());
  xml->get_widget("description_entry", description_entry);
  description_entry->signal_changed ().connect
    (method (on_description_changed));
  description_entry->set_text(ruin->getDescription());

  xml->get_widget("type_spinbutton", type_spinbutton);
  type_spinbutton->signal_changed().connect (method(on_type_changed));
  type_spinbutton->signal_insert_text().connect
    (sigc::hide(sigc::hide(method(on_type_text_changed))));
  type_spinbutton->set_value(ruin->getType());

  xml->get_widget("keeper_button", keeper_button);
  keeper_button->signal_clicked().connect(method(on_keeper_clicked));

  xml->get_widget("randomize_name_button", randomize_name_button);
  randomize_name_button->signal_clicked().connect(method(on_randomize_name_clicked));

  xml->get_widget("randomize_keeper_button", randomize_keeper_button);
  randomize_keeper_button->signal_clicked().connect
    (method(on_randomize_keeper_clicked));

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
  on_hidden_toggled();

  xml->get_widget("new_reward_hbox", new_reward_hbox);
  xml->get_widget("random_reward_switch", random_reward_switch);
  random_reward_switch->property_active().signal_changed().connect(method(on_new_reward_toggled));

  xml->get_widget("reward_button", reward_button);
  reward_button->signal_clicked().connect(method(on_reward_clicked));

  if (ruin->getReward() == NULL)
    random_reward_switch->set_active (true);
  else
    {
      reward = ruin->getReward();
      random_reward_switch->set_active (false);
    }

  set_reward_name();
}

int RuinEditorDialog::run()
{
  dialog->show_all();
  return dialog->run();
}
/*
int RuinEditorDialog::run()
{
    dialog->show_all();
    int response = dialog->run();

    if (response == Gtk::RESPONSE_ACCEPT)
    {
        Location *l = ruin;
        RenamableLocation *renamable_ruin = static_cast<RenamableLocation*>(l);
        renamable_ruin->setName(name_entry->get_text());
	renamable_ruin->setDescription(description_entry->get_text());
	ruin->setType(type_entry->get_value_as_int());

	// get rid of old occupant and insert new
	Stack *occupant = ruin->getOccupant();
	if (occupant)
	    delete occupant;

	if (keeper->empty())
          {
	    delete keeper;
	    keeper = 0;
          }
        else
          ruin->setOccupant(keeper);

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
          ruin->setOwner(NULL);
	keeper = 0;
      ruin->setReward(reward);
    }
    else
    {
        //put the ruin name back.
	if (name_entry->get_text() != Ruin::getDefaultName())
	  d_randomizer->pushRandomRuinName(name_entry->get_text());
	delete keeper;
	keeper = 0;
    }
    return response;
}
*/

void RuinEditorDialog::set_keeper_name()
{
  Glib::ustring name;
  if (ruin->getOccupant () && !ruin->getOccupant ()->empty())
    name = ruin->getOccupant ()->getStrongestArmy()->getName();
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
  Player *neutral = Playerlist::getInstance()->getNeutral();
  SelectArmyDialog d(*dialog, true, neutral, false, true);
  d.run();

  const ArmyProto *army = d.get_selected_army();
  if (army)
    {
      Stack *occupant = new Stack(0, ruin->getPos());
      Army *a = new Army(*army, neutral);
      occupant->push_back(a);
      ruin->setOccupant(occupant);
    }
  else
    {
      if (ruin->getOccupant())
        ruin->clearOccupant ();
    }

  set_keeper_name();
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

void RuinEditorDialog::on_randomize_keeper_clicked()
{
  Stack *occupant = new Stack(0, ruin->getPos());
  Army *a = d_randomizer->getRandomRuinKeeper
    (Playerlist::getInstance()->getNeutral());
  occupant->push_back (a);
  ruin->setOccupant (occupant);
  set_keeper_name();
}

void RuinEditorDialog::on_new_reward_toggled()
{
  if (random_reward_switch->get_active () == true)
    {
      if (reward)
        {
          delete reward;
          reward = NULL;
        }
      set_reward_name();
    }
  new_reward_hbox->set_sensitive(!random_reward_switch->get_active());
}

void RuinEditorDialog::on_reward_clicked()
{
  Player *neutral = Playerlist::getInstance ()->getNeutral();
  RewardEditorDialog d(*dialog, neutral, false, NULL);
  d.run();
  if (reward)
    {
      delete reward;
      reward = NULL;
    }
  if (d.get_reward())
    reward = d.get_reward();
  set_reward_name();
}

void RuinEditorDialog::set_reward_name()
{
  Glib::ustring name;
  if (reward)
    name = reward->getName();
  else
    name = _("No reward");

  reward_button->set_label(name);
}

void RuinEditorDialog::on_name_changed ()
{
  Location *l = ruin;
  RenamableLocation *renamable_ruin = static_cast<RenamableLocation*>(l);
  renamable_ruin->setName(name_entry->get_text());
}

void RuinEditorDialog::on_description_changed ()
{
  ruin->setDescription(description_entry->get_text());
}

void RuinEditorDialog::on_type_changed ()
{
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
