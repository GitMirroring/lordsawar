//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014, 2015, 2020 Ben Asselstine
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

#include <assert.h>
#include <gtkmm.h>
#include <sigc++/functors/mem_fun.h>

#include "players-dialog.h"

#include "defs.h"
#include "File.h"
#include "player.h"
#include "playerlist.h"
#include "armysetlist.h"
#include "stacklist.h"
#include "citylist.h"
#include "ucompose.hpp"
#include "game-parameters.h"
#include "CreateScenarioRandomize.h"
#include "heroes-dialog.h"
#include "font-size.h"

#define method(x) sigc::mem_fun(*this, &PlayersDialog::x)

PlayersDialog::PlayersDialog(Gtk::Window &parent, CreateScenarioRandomize *random)
  : LwEditorDialog(parent, "players-dialog.ui")
{
  d_random = random;
  d_changed = false;

  xml->get_widget("randomize_gold_button", randomize_gold_button);
  randomize_gold_button->signal_clicked().connect (method(on_randomize_gold_pressed));
  xml->get_widget("players_grid", players_grid);
  int px = FontSize::getInstance ()->get_height () / 2;
  players_grid->property_row_spacing () = px;
  players_grid->property_column_spacing () = px;
  Gdk::RGBA white = Gdk::RGBA ("white");
  players_grid->override_background_color(white);

  // add default players
  default_player_names.push_back(random->getPlayerName(Shield::WHITE));
  default_player_names.push_back(random->getPlayerName(Shield::GREEN));
  default_player_names.push_back(random->getPlayerName(Shield::YELLOW));
  default_player_names.push_back(random->getPlayerName(Shield::LIGHT_BLUE));
  default_player_names.push_back(random->getPlayerName(Shield::ORANGE));
  default_player_names.push_back(random->getPlayerName(Shield::DARK_BLUE));
  default_player_names.push_back(random->getPlayerName(Shield::RED));
  default_player_names.push_back(random->getPlayerName(Shield::BLACK));

  Playerlist *pl = Playerlist::getInstance();

  // merge defined players with predefined
  std::vector<Player *> players_to_add(default_player_names.size(), 0);
  for (Playerlist::iterator j = pl->begin(); j != pl->end(); ++j)
    {
      Player *player = *j;
      if (player == pl->getNeutral())
	continue;
      players_to_add[player->getId()] = player;
    }

  player_name_seq::iterator current_name = default_player_names.begin();
  for (unsigned int j = 0; j < players_to_add.size(); ++j)
    if (players_to_add[j])
      {
	Player *player = players_to_add[j];
	add_player(j,
		   player->getName(), player->getGold(), player);
	++current_name;
      }
    else
      {
	int gold = 0;
	add_player(j, *current_name, gold, NULL);
	++current_name;
      }
}

GameParameters::Player PlayersDialog::to_player (int row)
{
  GameParameters::Player player;
  Glib::ustring type = player_type_comboboxes[row]->get_active_text ();
  player.name = player_name_entries[row]->get_text ();
  if (type == HUMAN_PLAYER_TYPE)
    player.type = GameParameters::Player::HUMAN;
  else if (type == EASY_PLAYER_TYPE)
    player.type = GameParameters::Player::EASY;
  else if (type == HARD_PLAYER_TYPE)
    player.type = GameParameters::Player::HARD;
  else if (type == NO_PLAYER_TYPE)
    player.type = GameParameters::Player::OFF;
  player.id = row;
  return player;
}

void PlayersDialog::update_player (int row)
{
  GameParameters::Player player = to_player (row);
  Playerlist::getInstance ()->syncPlayer(player);
  Player *p = Playerlist::getInstance ()->getPlayer(player.id);
  if (p)
    p->setGold(player_gold_spinbuttons[row]->get_value ());
}

bool PlayersDialog::run()
{
  dialog->show_all();
  dialog->run();
  return d_changed;
}

Gtk::Entry * PlayersDialog::add_entry_for_player_name(int row,
                                                      Glib::ustring name)
{
  Gtk::Entry *e = Gtk::manage (new Gtk::Entry ());
  e->set_text (name);
  e->property_hexpand () = true;
  e->signal_changed ().connect(sigc::bind (method (on_player_name_changed), row));
  player_name_entries.push_back (e);
  return e;
}

Gtk::Button* PlayersDialog::add_button_for_player_heroes (int row)
{
  Gtk::Button *b = Gtk::manage (new Gtk::Button());
  b->set_label (_("Heroes"));
  b->signal_clicked ().connect(sigc::bind (method (on_player_heroes_clicked), row));
  int px = FontSize::getInstance ()->get_height ();
  b->property_margin_right () = px;
  player_heroes_buttons.push_back (b);
  return b;
}

Gtk::SpinButton* PlayersDialog::add_spinbutton_for_player_gold(int row,
                                                               int gold)
{
  Gtk::SpinButton *b = Gtk::manage (new Gtk::SpinButton());
  b->set_adjustment (Gtk::Adjustment::create (0, 0, 10000));
  b->set_value (gold);
  b->signal_changed ().connect(sigc::bind (method (on_player_gold_changed), row));
  b->signal_insert_text().connect (sigc::bind(method(on_player_gold_edited), row));
  player_gold_spinbuttons.push_back (b);
  return b;
}

Gtk::ComboBoxText* PlayersDialog::add_combo_for_player_type (int row, Player *p)
{
  Gtk::ComboBoxText *c = Gtk::manage (new Gtk::ComboBoxText ());

  c->append (NO_PLAYER_TYPE);
  c->append (HUMAN_PLAYER_TYPE);
  c->append (EASY_PLAYER_TYPE);
  c->append (HARD_PLAYER_TYPE);
  if (p == NULL)
    c->set_active (0);
  else
    {
      switch (p->getType ())
        {
        case Player::HUMAN: c->set_active (1); break;
        case Player::AI_FAST: c->set_active (2); break;
        case Player::AI_SMART: c->set_active (3); break;
        default: c->set_active (0); break;
        }
    }
  c->signal_changed ().connect(sigc::bind (method (on_player_type_changed), row));
  int px = FontSize::getInstance ()->get_height ();
  c->property_margin_left () = px;
  player_type_comboboxes.push_back (c);
  return c;
}

void PlayersDialog::add_player(int row, Glib::ustring name, int gold, Player *p)
{
  players_grid->attach (*add_combo_for_player_type (row, p), 0, row + 1);
  players_grid->attach (*add_entry_for_player_name (row, name), 1, row + 1);
  players_grid->attach (*add_spinbutton_for_player_gold (row, gold), 2, row + 1);
  players_grid->attach (*add_button_for_player_heroes (row), 3, row + 1);
  sensitize_row (row);
}

void PlayersDialog::on_player_type_changed (int row)
{
  sensitize_row (row);
  d_changed = true;
  update_player (row);
}

void PlayersDialog::on_player_name_changed (int row)
{
  d_changed = true;
  update_player (row);
}

void PlayersDialog::on_player_gold_changed (int row)
{
  d_changed = true;
  update_player (row);
}

void PlayersDialog::on_player_gold_edited (const Glib::ustring &text, int *p, int row)
{
  (void) p;
  (void) text;
  d_changed = true;
  update_player (row);
}

void PlayersDialog::on_player_heroes_clicked (int row)
{
  HeroesDialog d (*dialog, row, player_name_entries[row]->get_text ());
  if (d.run ())
    d_changed = true;
}

void PlayersDialog::on_randomize_gold_pressed()
{
  d_changed = true;
  for (guint32 i = 0; i < player_type_comboboxes.size (); i++)
    {
      if (player_type_comboboxes[i]->get_active_row_number () == 0)
        continue;
      int gold = 0;
      d_random->getBaseGold(100, &gold);
      gold = d_random->adjustBaseGold(gold);
      Player *p = Playerlist::getInstance()->getPlayer (i);
      p->setGold (gold);
      player_gold_spinbuttons[i]->set_value (gold);
    }
}

void PlayersDialog::sensitize_row (int i)
{
  bool sens = player_type_comboboxes[i]->get_active_row_number () != 0;
  player_name_entries[i]->property_sensitive () = sens;
  player_heroes_buttons[i]->property_sensitive () = sens;
  player_gold_spinbuttons[i]->property_sensitive () = sens;
}
