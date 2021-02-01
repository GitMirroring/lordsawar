//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014, 2015, 2020, 2021 Ben Asselstine
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
#include "players-editor-actions.h"
#include "herotemplates.h"

#define method(x) sigc::mem_fun(*this, &PlayersDialog::x)

PlayersDialog::PlayersDialog(Gtk::Window &parent, CreateScenarioRandomize *random)
  : LwEditorDialog(parent, "players-dialog.ui")
{
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));
  d_random = random;
  d_changed = false;

  xml->get_widget("randomize_gold_button", randomize_gold_button);
  randomize_gold_button->signal_clicked().connect (method(on_randomize_gold_pressed));
  xml->get_widget("undo_button", undo_button);
  undo_button->signal_clicked ().connect (method (on_undo_activated));
  xml->get_widget("redo_button", redo_button);
  redo_button->signal_clicked ().connect (method (on_redo_activated));
  xml->get_widget("players_grid", players_grid);
  int px = FontSize::getInstance ()->get_height () / 2;
  players_grid->property_row_spacing () = px;
  players_grid->property_column_spacing () = px;
  Gdk::RGBA white = Gdk::RGBA ("white");
  players_grid->override_background_color(white);

  // add default players
  default_player_names.push_back (random->getPlayerName(Shield::WHITE));
  default_player_names.push_back (random->getPlayerName(Shield::GREEN));
  default_player_names.push_back (random->getPlayerName(Shield::YELLOW));
  default_player_names.push_back (random->getPlayerName(Shield::LIGHT_BLUE));
  default_player_names.push_back (random->getPlayerName(Shield::ORANGE));
  default_player_names.push_back (random->getPlayerName(Shield::DARK_BLUE));
  default_player_names.push_back (random->getPlayerName(Shield::RED));
  default_player_names.push_back (random->getPlayerName(Shield::BLACK));

  Playerlist *pl = Playerlist::getInstance();

  // merge defined players with predefined
  std::vector<Player *> players_to_add (default_player_names.size(), 0);
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
  connect_signals ();
  update ();
}

PlayersDialog::~PlayersDialog()
{
  delete umgr;
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

Gtk::Entry * PlayersDialog::add_entry_for_player_name (Glib::ustring name)
{
  Gtk::Entry *e = Gtk::manage (new Gtk::Entry ());
  e->set_text (name);
  e->property_hexpand () = true;
  player_name_entries.push_back (e);
  return e;
}

Gtk::Button* PlayersDialog::add_button_for_player_heroes ()
{
  Gtk::Button *b = Gtk::manage (new Gtk::Button());
  b->set_label (_("Heroes"));
  b->set_focus_on_click (false);
  int px = FontSize::getInstance ()->get_height ();
  b->property_margin_right () = px;
  player_heroes_buttons.push_back (b);
  return b;
}

Gtk::SpinButton* PlayersDialog::add_spinbutton_for_player_gold(int gold)
{
  Gtk::SpinButton *b = Gtk::manage (new Gtk::SpinButton());
  b->set_adjustment (Gtk::Adjustment::create (0, 0, 10000));
  b->set_value (gold);
  player_gold_spinbuttons.push_back (b);
  return b;
}

Gtk::ComboBoxText* PlayersDialog::add_combo_for_player_type (Player *p)
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
  int px = FontSize::getInstance ()->get_height ();
  c->property_margin_left () = px;
  player_type_comboboxes.push_back (c);
  return c;
}

void PlayersDialog::add_player(int row, Glib::ustring name, int gold, Player *p)
{
  players_grid->attach (*add_combo_for_player_type (p), 0, row + 1);
  players_grid->attach (*add_entry_for_player_name (name), 1, row + 1);
  players_grid->attach (*add_spinbutton_for_player_gold (gold), 2, row + 1);
  players_grid->attach (*add_button_for_player_heroes (), 3, row + 1);
  sensitize_row (row);
}

void PlayersDialog::on_player_type_changed (int row)
{
  int oldtype = -1;
  Player *p = Playerlist::getInstance ()->getPlayer (row);
  if (p)
    oldtype = p->getType ();
  umgr->add (new PlayersEditorAction_Type (row, oldtype));
  sensitize_row (row);
  d_changed = true;
  update_player (row);
}

void PlayersDialog::on_player_name_changed (int row)
{
  Glib::ustring oldname = "";
  Player *p = Playerlist::getInstance ()->getPlayer (row);
  if (p)
    oldname = p->getName ();
  umgr->add (new PlayersEditorAction_Name (row, oldname));
  d_changed = true;
  update_player (row);
}

void PlayersDialog::on_player_gold_edited (const Glib::ustring &text, int *p, int row)
{
  guint32 oldgp = 0;
  Player *player = Playerlist::getInstance ()->getPlayer (row);
  if (player)
    oldgp = player->getGold ();
  umgr->add (new PlayersEditorAction_Gold (row, oldgp));
  (void) p;
  (void) text;
  d_changed = true;
  player_gold_spinbuttons[row]->set_value (atoi (player_gold_spinbuttons[row]->get_text ().c_str ()));
  update_player (row);
}

void PlayersDialog::on_player_heroes_clicked (int row)
{
  PlayersEditorAction_Heroes *action =
    new PlayersEditorAction_Heroes
    (row, HeroTemplates::getInstance ()->getHeroes (row));
  HeroesDialog d (*dialog, row, player_name_entries[row]->get_text ());
  if (d.run ())
    {
      umgr->add (action);
      d_changed = true;
    }
  else
    delete action;
}

void PlayersDialog::on_randomize_gold_pressed()
{
  std::list<guint32> gp;
  for (guint32 i = 0; i < player_gold_spinbuttons.size (); i++)
    gp.push_back (guint32(player_gold_spinbuttons[i]->get_value ()));

  umgr->add (new PlayersEditorAction_RandomizeGold (gp));
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

void PlayersDialog::on_undo_activated ()
{
  umgr->undo ();
  if (umgr->undoEmpty ())
    d_changed = false;
  update ();
  return;
}

void PlayersDialog::on_redo_activated ()
{
  umgr->redo ();
  d_changed = true;
  update ();
}

void PlayersDialog::update ()
{
  disconnect_signals ();
  for (auto p : *Playerlist::getInstance ())
    {
      if (p == Playerlist::getInstance ()->getNeutral ())
        continue;
      auto c = player_type_comboboxes[p->getId ()];
      switch (p->getType ())
        {
        case Player::HUMAN: c->set_active (1); break;
        case Player::AI_FAST: c->set_active (2); break;
        case Player::AI_SMART: c->set_active (3); break;
        default: c->set_active (0); break;
        }
      if (player_name_entries[p->getId ()]->get_text () != p->getName ())
        player_name_entries[p->getId ()]->set_text (p->getName ());
      player_gold_spinbuttons[p->getId ()]->set_value (p->getGold ());
      sensitize_row (p->getId ());
    }
  connect_signals ();
}

void PlayersDialog::connect_signals ()
{
  int row = 0;
  for (auto c : player_type_comboboxes)
    {
      connections.push_back
        (c->signal_changed ().connect
         (sigc::bind (method (on_player_type_changed), row)));
      row++;
    }

  row = 0;
  for (auto e : player_name_entries)
    {
      connections.push_back
        (e->signal_changed ().connect
         (sigc::bind (method (on_player_name_changed), row)));
      row++;
    }

  row = 0;
  for (auto g : player_gold_spinbuttons)
    {
      connections.push_back
        (g->signal_insert_text().connect
         (sigc::bind(method(on_player_gold_edited), row)));
      row++;
    }

  row = 0;
  for (auto h : player_heroes_buttons)
    {
      connections.push_back
        (h->signal_clicked ().connect
         (sigc::bind (method (on_player_heroes_clicked), row)));
      row++;
    }
}

void PlayersDialog::disconnect_signals ()
{
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}

UndoAction *PlayersDialog::executeAction (UndoAction *action2)
{
  PlayersEditorAction *action =
    dynamic_cast<PlayersEditorAction*>(action2);
  UndoAction *out = NULL;

  switch (action->getType ())
    {
      case PlayersEditorAction::TYPE:
          {
            PlayersEditorAction_Type *a =
              dynamic_cast<PlayersEditorAction_Type*>(action);

            int type = -1;
            Player *p = Playerlist::getInstance ()->getPlayer (a->getIndex ());
            if (p)
              type = p->getType ();

            int row = a->getIndex ();
            out = new PlayersEditorAction_Type (row, type);
            disconnect_signals ();
            switch (a->getPlayerType ())
              {
              case -1:
                player_type_comboboxes[row]->set_active (0);
                break;
              case 0:
                player_type_comboboxes[row]->set_active (1);
                break;
              case 1:
                player_type_comboboxes[row]->set_active (2);
                break;
              case 2:
                player_type_comboboxes[row]->set_active (2);
                break;
              case 4:
                player_type_comboboxes[row]->set_active (3);
                break;
              case 8:
                player_type_comboboxes[row]->set_active (0);
                break;
              }
            update_player (a->getIndex ());
            connect_signals ();
          }
        break;
      case PlayersEditorAction::NAME:
          {
            PlayersEditorAction_Name *a =
              dynamic_cast<PlayersEditorAction_Name*>(action);
            int row = a->getIndex ();
            Glib::ustring oldname = "";
            Player *p = Playerlist::getInstance ()->getPlayer (row);
            if (p)
              oldname = p->getName ();
            out = new PlayersEditorAction_Name (row, oldname);
            if (p)
              p->setName (a->getName ());
          } 
        break;
      case PlayersEditorAction::GOLD:
          {
            PlayersEditorAction_Gold *a =
              dynamic_cast<PlayersEditorAction_Gold*>(action);

            guint32 gp = 0;
            Player *p = Playerlist::getInstance ()->getPlayer (a->getIndex ());
            if (p)
              gp = p->getGold ();
            out = new PlayersEditorAction_Gold (a->getIndex (), gp);
            if (p)
              p->setGold (a->getGold ());
          }
        break;
      case PlayersEditorAction::RANDOMIZE_GOLD:
          {
            PlayersEditorAction_RandomizeGold *a =
              dynamic_cast<PlayersEditorAction_RandomizeGold*>(action);
            std::list<guint32> gp;
            for (guint32 i = 0; i < player_gold_spinbuttons.size (); i++)
              gp.push_back (guint32(player_gold_spinbuttons[i]->get_value ()));
            out = new PlayersEditorAction_RandomizeGold (gp);
            int row = 0;
            disconnect_signals ();
            for (auto g : a->getPlayersGold ())
              {
                player_gold_spinbuttons[row]->set_value (g);
                Player *p = Playerlist::getInstance ()->getPlayer (row);
                if (p)
                  p->setGold (g);
                row++;
              }
            connect_signals ();
          }
        break;
      case PlayersEditorAction::HEROES:
          {
            PlayersEditorAction_Heroes *a =
              dynamic_cast<PlayersEditorAction_Heroes*>(action);
            out = new PlayersEditorAction_Heroes
              (a->getIndex (),
               HeroTemplates::getInstance ()->getHeroes (a->getIndex ()));
            HeroTemplates::getInstance ()->replaceHeroes (a->getIndex (),
                                                          a->getHeroes ());
            a->clearHeroes ();
          }
        break;
    }
  return out;
}
