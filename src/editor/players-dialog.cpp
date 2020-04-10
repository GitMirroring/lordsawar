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

#define method(x) sigc::mem_fun(*this, &PlayersDialog::x)

namespace
{
    Glib::ustring player_type_to_string(guint32 type)
    {
	switch (type)
	{
	case Player::HUMAN: return HUMAN_PLAYER_TYPE;
	case Player::AI_FAST: return EASY_PLAYER_TYPE;
	case Player::AI_SMART: return HARD_PLAYER_TYPE;
	default: return NO_PLAYER_TYPE;
	}
    }
}

PlayersDialog::PlayersDialog(Gtk::Window &parent, CreateScenarioRandomize *random)
  : LwEditorDialog(parent, "players-dialog.ui"),
    type_column(_("Type"), type_renderer),
    gold_column(_("Gold"), gold_renderer),
    name_column(_("Name"), name_renderer)
{
  d_random = random;
  d_changed = false;

  // setup the player settings
  player_list = Gtk::ListStore::create(player_columns);

  xml->get_widget("randomize_gold_button", randomize_gold_button);
  randomize_gold_button->signal_clicked().connect (method(on_randomize_gold_pressed));
  xml->get_widget("heroes_button", heroes_button);
  heroes_button->signal_clicked().connect (method(on_edit_heroes_pressed));
  xml->get_widget("player_treeview", player_treeview);
  player_treeview->set_model(player_list);

  // the type column
  player_type_list = Gtk::ListStore::create(player_type_columns);
  Gtk::TreeModel::iterator i;
  i = player_type_list->append();
  (*i)[player_type_columns.type] = HUMAN_PLAYER_TYPE;
  i = player_type_list->append();
  (*i)[player_type_columns.type] = EASY_PLAYER_TYPE;
  i = player_type_list->append();
  (*i)[player_type_columns.type] = HARD_PLAYER_TYPE;
  i = player_type_list->append();
  (*i)[player_type_columns.type] = NO_PLAYER_TYPE;

  type_renderer.property_model() = player_type_list;
  type_renderer.property_text_column() = 0;
  type_renderer.property_has_entry() = false;
  type_renderer.property_editable() = true;

  type_renderer.signal_edited().connect(method(on_type_edited));
  type_column.set_cell_data_func(type_renderer, method(cell_data_type));
  player_treeview->append_column(type_column);

  // name column
  name_renderer.property_editable() = true;
  name_renderer.signal_edited().connect (method(on_name_edited));
  name_column.set_cell_data_func(name_renderer, method(cell_data_name));
  player_treeview->append_column(name_column);

  // gold column
  gold_renderer.property_editable() = true;
  gold_renderer.signal_edited().connect (method(on_gold_edited));
  gold_column.set_cell_data_func (gold_renderer, method(cell_data_gold));
  player_treeview->append_column(gold_column);

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
	add_player(player_type_to_string(player->getType()),
		   player->getName(), player->getGold(), player);
	++current_name;
      }
    else
      {
	int gold = 0;
	add_player(NO_PLAYER_TYPE, *current_name, gold, 0);
	++current_name;
      }
  player_treeview->set_cursor (Gtk::TreePath ("0"));
}

GameParameters::Player PlayersDialog::to_player (Gtk::TreeModel::iterator i)
{
  GameParameters::Player player;
  Glib::ustring type = (*i)[player_columns.type];
  player.name = (*i)[player_columns.name];
  if (type == HUMAN_PLAYER_TYPE)
    player.type = GameParameters::Player::HUMAN;
  else if (type == EASY_PLAYER_TYPE)
    player.type = GameParameters::Player::EASY;
  else if (type == HARD_PLAYER_TYPE)
    player.type = GameParameters::Player::HARD;
  else if (type == NO_PLAYER_TYPE)
    player.type = GameParameters::Player::OFF;
  Gtk::TreeModel::Path path = player_treeview->get_model()->get_path (i);
  player.id = atoi (path.to_string ().c_str ());
  return player;
}

void PlayersDialog::update_player ()
{
  Gtk::TreeModel::Path path;
  Gtk::TreeViewColumn* focus_column;
  player_treeview->get_cursor (path, focus_column);
  Gtk::TreeModel::iterator i = player_treeview->get_model()->get_iter(path);
  GameParameters::Player player = to_player (i);
  Playerlist::getInstance ()->syncPlayer(player);
  Player *p = Playerlist::getInstance ()->getPlayer(player.id);
  (*i)[player_columns.player] = p;
  if (p)
    p->setGold((*i)[player_columns.gold]);
}

bool PlayersDialog::run()
{
  dialog->show_all();
  dialog->run();
  return d_changed;
}

void PlayersDialog::cell_data_type(Gtk::CellRenderer *renderer,
                                   const Gtk::TreeIter& i)
{
  dynamic_cast<Gtk::CellRendererText*>(renderer)->property_text()
    = (*i)[player_columns.type];
}

void PlayersDialog::on_type_edited(const Glib::ustring &path,
                                   const Glib::ustring &new_text)
{
  (*player_list->get_iter(Gtk::TreePath(path)))[player_columns.type] = new_text;
  d_changed = true;
  update_player ();
}

void PlayersDialog::add_player(const Glib::ustring &type,
                               const Glib::ustring &name, int gold, Player *player)
{
  Gtk::TreeIter i = player_list->append();
  (*i)[player_columns.type] = type;
  (*i)[player_columns.name] = name;
  (*i)[player_columns.gold] = gold;
  (*i)[player_columns.player] = player;

  player_treeview->get_selection()->select(i);
}

void PlayersDialog::cell_data_gold(Gtk::CellRenderer *renderer,
				  const Gtk::TreeIter& i)
{
  dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_adjustment()
    = Gtk::Adjustment::create((*i)[player_columns.gold], 0, 10000, 1);
  dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_text() =
    String::ucompose("%1", (*i)[player_columns.gold]);
}

void PlayersDialog::on_gold_edited(const Glib::ustring &path,
				   const Glib::ustring &new_text)
{
  Gtk::TreeIter i = player_list->get_iter(Gtk::TreePath(path));
  if ((*i)[player_columns.type] != NO_PLAYER_TYPE)
    {
      int gold = atoi(new_text.c_str());
      (*i)[player_columns.gold] = gold;
      d_changed = true;
      update_player ();
    }
}

void PlayersDialog::cell_data_name(Gtk::CellRenderer *renderer,
				  const Gtk::TreeIter& i)
{
  dynamic_cast<Gtk::CellRendererText*>(renderer)->property_text() =
    String::ucompose("%1", (*i)[player_columns.name]);
}

void PlayersDialog::on_name_edited(const Glib::ustring &path,
				   const Glib::ustring &new_text)
{
  Gtk::TreeIter i = player_list->get_iter(Gtk::TreePath(path));
  Glib::ustring type = (*i)[player_columns.type];
  if (type != NO_PLAYER_TYPE)
    {
      (*i)[player_columns.name] = new_text;
      d_changed = true;
      update_player ();
    }
}

void PlayersDialog::on_randomize_gold_pressed()
{
  for (Gtk::TreeIter i = player_list->children().begin(),
       end = player_list->children().end(); i != end; ++i)
    {
      if ((*i)[player_columns.type] != NO_PLAYER_TYPE)
        {
          int gold = 0;
          d_random->getBaseGold(100, &gold);
          gold = d_random->adjustBaseGold(gold);
          (*i)[player_columns.gold] = gold;
          Player *p = (*i)[player_columns.player];
          p->setGold (gold);
          d_changed = true;
        }
    }
}

void PlayersDialog::on_edit_heroes_pressed ()
{
  Gtk::TreeIter i = player_treeview->get_selection()->get_selected();
  Gtk::TreeModel::Path path = player_treeview->get_model()->get_path (i);
  guint32 player_id = atoi (path.to_string ().c_str ());
  Glib::ustring name = (*i)[player_columns.name];
  HeroesDialog d (*dialog, player_id, name);
  if (d.run ())
    d_changed = true;
}
