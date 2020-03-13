//  Copyright (C) 2017, 2020 Ben Asselstine
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

#include "battle-calculator-dialog.h"

#include "ucompose.hpp"
#include "defs.h"
#include "stack.h"
#include "army.h"
#include "armyproto.h"
#include "hero.h"
#include "heroproto.h"
#include "ImageCache.h"
#include "playerlist.h"
#include "stacklist.h"
#include "stacktile.h"
#include "hero-editor-dialog.h"
#include "GameMap.h"
#include "tileset.h"
#include "fight.h"
#include "font-size.h"

#include "select-army-dialog.h"

#define method(x) sigc::mem_fun(*this, &BattleCalculatorDialog::x)

BattleCalculatorDialog::BattleCalculatorDialog(Gtk::Window &parent, std::list<Army*> &attackers, std::list<Army *> &defenders)
 : LwEditorDialog(parent, "battle-calculator-dialog.ui"),
    attacker_strength_column(_("Strength"), attacker_strength_renderer),
    defender_strength_column(_("Strength"), defender_strength_renderer),
    d_attackers(attackers), d_defenders(defenders)
{
  attacker_player_combobox = NULL;
  attacker_player_combobox = manage(new Gtk::ComboBoxText);
  for (auto p : *Playerlist::getInstance())
    attacker_player_combobox->append(p->getName());
  attacker_player_combobox->set_active(0);
  attacker_player_combobox->signal_changed().connect (method(on_attacker_player_changed));
  Gtk::Box *box1;
  xml->get_widget("attacker_hbox", box1);
  box1->pack_start(*attacker_player_combobox, Gtk::PACK_SHRINK);

  defender_player_combobox = NULL;
  defender_player_combobox = manage(new Gtk::ComboBoxText);
  for (auto p : *Playerlist::getInstance())
    defender_player_combobox->append(p->getName());
  defender_player_combobox->set_active(0);
  defender_player_combobox->signal_changed().connect (method(on_defender_player_changed));
  Gtk::Box *box2;
  xml->get_widget("defender_hbox", box2);
  box2->pack_start(*defender_player_combobox, Gtk::PACK_SHRINK);

  attackers_list = Gtk::ListStore::create(combatant_columns);
  xml->get_widget("attackers_treeview", attackers_treeview);
  attackers_treeview->set_model(attackers_list);

  attackers_treeview->append_column("", combatant_columns.image);

  attacker_strength_renderer.property_editable() = true;
  attacker_strength_renderer.signal_edited().connect(method(on_attacker_strength_edited));
  attacker_strength_column.set_cell_data_func(attacker_strength_renderer, method(cell_data_attacker_strength));
  attackers_treeview->append_column(attacker_strength_column);
  attackers_treeview->append_column(_("Augmented Str"), combatant_columns.augmented_strength);
  attackers_treeview->append_column(_("HP"), combatant_columns.hp);
  for (guint32 i = 0; i < attackers_treeview->get_n_columns(); i++)
    attackers_treeview->get_column(i)->set_expand();

  defenders_list = Gtk::ListStore::create(combatant_columns);
  xml->get_widget("defenders_treeview", defenders_treeview);
  defenders_treeview->set_model(defenders_list);

  defenders_treeview->append_column("", combatant_columns.image);

  defender_strength_renderer.property_editable() = true;
  defender_strength_renderer.signal_edited().connect(method(on_defender_strength_edited));
  defender_strength_column.set_cell_data_func(defender_strength_renderer, method(cell_data_defender_strength));
  defenders_treeview->append_column(defender_strength_column);
  defenders_treeview->append_column(_("Augmented Str"), combatant_columns.augmented_strength);
  defenders_treeview->append_column(_("HP"), combatant_columns.hp);
  for (guint32 i = 0; i < attackers_treeview->get_n_columns(); i++)
    defenders_treeview->get_column(i)->set_expand();
  xml->get_widget("fortified_switch", fortified_switch);

  xml->get_widget("attacker_add_button", attacker_add_button);
  xml->get_widget("attacker_remove_button", attacker_remove_button);
  xml->get_widget("attacker_copy_button", attacker_copy_button);
  xml->get_widget("attacker_edit_hero_button", attacker_edit_hero_button);

  attacker_add_button->signal_clicked().connect(method(on_attacker_add_clicked));
  attacker_remove_button->signal_clicked().connect(method(on_attacker_remove_clicked));
  attacker_copy_button->signal_clicked().connect(method(on_attacker_copy_clicked));
  attacker_edit_hero_button->signal_clicked().connect(method(on_attacker_edit_hero_clicked));

  attackers_treeview->get_selection()->signal_changed().connect(method(on_attacker_selection_changed));

  xml->get_widget("defender_add_button", defender_add_button);
  xml->get_widget("defender_remove_button", defender_remove_button);
  xml->get_widget("defender_copy_button", defender_copy_button);
  xml->get_widget("defender_edit_hero_button", defender_edit_hero_button);

  defender_add_button->signal_clicked().connect(method(on_defender_add_clicked));
  defender_remove_button->signal_clicked().connect(method(on_defender_remove_clicked));
  defender_copy_button->signal_clicked().connect(method(on_defender_copy_clicked));
  defender_edit_hero_button->signal_clicked().connect(method(on_defender_edit_hero_clicked));

  defenders_treeview->get_selection()->signal_changed().connect(method(on_defender_selection_changed));
  xml->get_widget("fight_button", fight_button);
  fight_button->signal_clicked().connect(method(on_fight_clicked));
  xml->get_widget("fight100_button", fight100_button);
  fight100_button->signal_clicked().connect(method(on_fight100_clicked));

  xml->get_widget("city_switch", city_switch);
  city_switch->property_active().signal_changed().connect(method(on_city_toggled));
  xml->get_widget("terrain_box", terrain_box);
  terrain_combobox = manage(new Gtk::ComboBoxText);
  Tileset *tileset = GameMap::getTileset();
  for (auto t : *tileset)
    terrain_combobox->append(t->getName());
  terrain_combobox->set_active(0);
  terrain_box->pack_start(*terrain_combobox, Gtk::PACK_SHRINK);
  xml->get_widget("die_sides_combobox", die_sides_combobox);
  for (auto a : d_attackers)
    add_attacker_army (a, false);
  for (auto a : d_defenders)
    add_defender_army (a, false);
  set_button_sensitivity();
}

int BattleCalculatorDialog::run()
{
  dialog->show_all();
  return dialog->run();
}

Player *BattleCalculatorDialog::get_attacker_player()
{
  int c = 0, row = attacker_player_combobox->get_active_row_number();
  Player *player = Playerlist::getInstance()->getNeutral();
  for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
       end = Playerlist::getInstance()->end(); i != end; ++i, ++c)
    if (c == row)
      {
	player = *i;
	break;
      }
  return player;
}

void BattleCalculatorDialog::on_attacker_copy_clicked()
{
  Gtk::TreeIter i = attackers_treeview->get_selection()->get_selected();
  if (i)
    {
      Player *player = get_attacker_player();
      Army *army = (*i)[combatant_columns.army];
      Army *new_army = new Army(*army, player);
      new_army->assignNewId();
      add_attacker_army(new_army, true);
    }

  set_button_sensitivity();
}

void BattleCalculatorDialog::on_attacker_add_clicked()
{
  SelectArmyDialog d(*dialog, get_attacker_player(), true);
  d.run();

  Player *player = get_attacker_player();
  const ArmyProto *army = d.get_selected_army();
  if (army)
    {
      if (army->isHero() == true)
        {
          HeroProto *hp = new HeroProto(*army);
          hp->setOwnerId(player->getId());
          add_attacker_army(new Hero (*hp), true);
          delete hp;
        }
      else
        add_attacker_army(new Army(*army, player), true);
    }
}
    

void BattleCalculatorDialog::on_attacker_edit_hero_clicked()
{
  Gtk::TreeIter i = attackers_treeview->get_selection()->get_selected();
  if (i)
    {
      Army *army = (*i)[combatant_columns.army];
      Hero *hero = dynamic_cast<Hero*>(army);
      HeroEditorDialog d(*dialog, hero);
      d.run();
    }
}

void BattleCalculatorDialog::on_attacker_remove_clicked()
{
  Gtk::TreeIter i = attackers_treeview->get_selection()->get_selected();
  if (i)
    {
      Army *army = (*i)[combatant_columns.army];
      d_attackers.erase(std::remove (d_attackers.begin(), d_attackers.end(), army), d_attackers.end());
      delete army;
      attackers_list->erase(i);

    }

  set_button_sensitivity();
}

void BattleCalculatorDialog::add_attacker_army(Army *a, bool add)
{
  if (add)
    d_attackers.push_back(a);
  ImageCache *gc = ImageCache::getInstance();
  Gtk::TreeIter i = attackers_list->append();
  (*i)[combatant_columns.army] = a;
  guint32 fs = FontSize::getInstance ()->get_height ();
  (*i)[combatant_columns.image] = gc->getDialogArmyPic (a, fs)->to_pixbuf ();
  (*i)[combatant_columns.strength] = a->getStat(Army::STRENGTH, false);
  (*i)[combatant_columns.hp] = a->getStat(Army::HP, false);

  attackers_treeview->get_selection()->select(i);

  set_button_sensitivity();
}

void BattleCalculatorDialog::on_attacker_selection_changed()
{
  set_button_sensitivity();
}

void BattleCalculatorDialog::on_attacker_player_changed()
{
  ImageCache *gc = ImageCache::getInstance();
  Player *player = get_attacker_player();
  set_button_sensitivity();

  guint32 fs = FontSize::getInstance ()->get_height ();
  for (Gtk::TreeIter j = attackers_list->children().begin(),
       jend = attackers_list->children().end(); j != jend; ++j)
    {
      Army *a = (*j)[combatant_columns.army];
      a->setOwner (player);
      (*j)[combatant_columns.image] =
        gc->getDialogArmyPic(a, fs)->to_pixbuf ();
    }
}

void BattleCalculatorDialog::cell_data_attacker_strength(Gtk::CellRenderer *renderer, const Gtk::TreeIter& i)
{
    dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_adjustment()
          = Gtk::Adjustment::create((*i)[combatant_columns.strength], 
                                    MIN_STRENGTH_FOR_ARMY_UNITS, 
                                    MAX_STRENGTH_FOR_ARMY_UNITS, 1);
    dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_text() = 
      String::ucompose("%1", (*i)[combatant_columns.strength]);
}

void BattleCalculatorDialog::on_attacker_strength_edited(const Glib::ustring &path, const Glib::ustring &new_text)
{
  int str = atoi(new_text.c_str());
  if (str < (int)MIN_STRENGTH_FOR_ARMY_UNITS || str > 
      (int)MAX_STRENGTH_FOR_ARMY_UNITS)
    return;
  (*attackers_list->get_iter(Gtk::TreePath(path)))[combatant_columns.strength] = str;
}

void BattleCalculatorDialog::set_button_sensitivity()
{
  int att = 0, def = 0;
    {
      int max_number_of_attackers = 8;
      Gtk::TreeIter i = attackers_treeview->get_selection()->get_selected();
      att = attackers_list->children().size();
      attacker_add_button->set_sensitive(att < max_number_of_attackers);
      attacker_copy_button->set_sensitive(i && att < max_number_of_attackers);
      attacker_remove_button->set_sensitive(i);
      if (i)
        {
          Army *army = (*i)[combatant_columns.army];
          if (army->isHero())
            {
              attacker_edit_hero_button->set_sensitive(true);
              attacker_copy_button->set_sensitive(false);
            }
          else
            attacker_edit_hero_button->set_sensitive(false);
        }
      else
        attacker_edit_hero_button->set_sensitive(false);
    }
    {
      int max_number_of_defenders = 32;
      Gtk::TreeIter i = defenders_treeview->get_selection()->get_selected();
      def = defenders_list->children().size();
      defender_add_button->set_sensitive(def < max_number_of_defenders);
      defender_copy_button->set_sensitive(i && def < max_number_of_defenders);
      defender_remove_button->set_sensitive(i);
      if (i)
        {
          Army *army = (*i)[combatant_columns.army];
          if (army->isHero())
            {
              defender_edit_hero_button->set_sensitive(true);
              defender_copy_button->set_sensitive(false);
            }
          else
            defender_edit_hero_button->set_sensitive(false);
        }
      else
        defender_edit_hero_button->set_sensitive(false);
    }
  fight_button->set_sensitive (att && def);
  fight100_button->set_sensitive (att && def);
}

Player *BattleCalculatorDialog::get_defender_player()
{
  int c = 0, row = defender_player_combobox->get_active_row_number();
  Player *player = Playerlist::getInstance()->getNeutral();
  for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
       end = Playerlist::getInstance()->end(); i != end; ++i, ++c)
    if (c == row)
      {
	player = *i;
	break;
      }
  return player;
}

void BattleCalculatorDialog::on_defender_copy_clicked()
{
  Gtk::TreeIter i = defenders_treeview->get_selection()->get_selected();
  if (i)
    {
      Player *player = get_defender_player();
      Army *army = (*i)[combatant_columns.army];
      Army *new_army = new Army(*army, player);
      new_army->assignNewId();
      add_defender_army(new_army, true);
    }

  set_button_sensitivity();
}

void BattleCalculatorDialog::on_defender_add_clicked()
{
  SelectArmyDialog d(*dialog, get_defender_player(), true);
  d.run();

  Player *player = get_defender_player();
  const ArmyProto *army = d.get_selected_army();
  if (army)
    {
      if (army->isHero() == true)
        {
          HeroProto *hp = new HeroProto(*army);
          hp->setOwnerId(player->getId());
          add_defender_army(new Hero(*hp), true);
          delete hp;
        }
      else
        add_defender_army(new Army(*army, player), true);
    }
}

void BattleCalculatorDialog::on_defender_edit_hero_clicked()
{
  Gtk::TreeIter i = defenders_treeview->get_selection()->get_selected();
  if (i)
    {
      Army *army = (*i)[combatant_columns.army];
      Hero *hero = dynamic_cast<Hero*>(army);
      HeroEditorDialog d(*dialog, hero);
      d.run();
    }
}

void BattleCalculatorDialog::on_defender_remove_clicked()
{
  Gtk::TreeIter i = defenders_treeview->get_selection()->get_selected();
  if (i)
    {
      Army *army = (*i)[combatant_columns.army];
      d_defenders.erase(std::remove (d_defenders.begin(), d_defenders.end(), army), d_defenders.end());
      delete army;
      defenders_list->erase(i);
    }

  set_button_sensitivity();
}

void BattleCalculatorDialog::add_defender_army(Army *a, bool add)
{
  if (add)
    d_defenders.push_back(a);
  ImageCache *gc = ImageCache::getInstance();
  Gtk::TreeIter i = defenders_list->append();
  (*i)[combatant_columns.army] = a;
  guint32 fs = FontSize::getInstance ()->get_height ();
  (*i)[combatant_columns.image] = gc->getDialogArmyPic (a, fs)->to_pixbuf ();
  (*i)[combatant_columns.strength] = a->getStat(Army::STRENGTH, false);
  //(*i)[combatant_columns.augmented_strength] =
    //a->getStat(Army::STRENGTH, false);
  (*i)[combatant_columns.hp] = a->getStat(Army::HP, false);

  defenders_treeview->get_selection()->select(i);

  set_button_sensitivity();
}

void BattleCalculatorDialog::on_defender_selection_changed()
{
  set_button_sensitivity();
}

void BattleCalculatorDialog::on_defender_player_changed()
{
  ImageCache *gc = ImageCache::getInstance();
  Player *player = get_defender_player();
  set_button_sensitivity();

  guint32 fs = FontSize::getInstance ()->get_height ();
  for (Gtk::TreeIter j = defenders_list->children().begin(),
       jend = defenders_list->children().end(); j != jend; ++j)
    {
      Army *a = (*j)[combatant_columns.army];
      a->setOwner (player);
      (*j)[combatant_columns.image] =
        gc->getDialogArmyPic(a, fs)->to_pixbuf ();
    }
}

void BattleCalculatorDialog::cell_data_defender_strength(Gtk::CellRenderer *renderer, const Gtk::TreeIter& i)
{
    dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_adjustment()
          = Gtk::Adjustment::create((*i)[combatant_columns.strength], 
                                    MIN_STRENGTH_FOR_ARMY_UNITS, 
                                    MAX_STRENGTH_FOR_ARMY_UNITS, 1);
    dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_text() = 
      String::ucompose("%1", (*i)[combatant_columns.strength]);
}

void BattleCalculatorDialog::on_defender_strength_edited(const Glib::ustring &path, const Glib::ustring &new_text)
{
  int str = atoi(new_text.c_str());
  if (str < (int)MIN_STRENGTH_FOR_ARMY_UNITS || str > 
      (int)MAX_STRENGTH_FOR_ARMY_UNITS)
    return;
  (*defenders_list->get_iter(Gtk::TreePath(path)))[combatant_columns.strength] = str;
}

void BattleCalculatorDialog::on_city_toggled()
{
  terrain_combobox->set_sensitive(!city_switch->property_active());
}

Fight::Result BattleCalculatorDialog::run_battle ()
{
  //load the attackers into a single stack
  std::list<Stack*> attackers;
  Stack *stack = new Stack(get_attacker_player(), Vector<int>(-1,-1));
  attackers.push_back (stack);
  for (auto a: attackers_list->children())
    {
      Army *army = (*a)[combatant_columns.army];
      stack->add (army);
    }

  //load the defenders into a bunch of stacks
  std::list<Stack*> defenders;
  stack = new Stack(get_defender_player(), Vector<int>(-1,-1));
  defenders.push_back (stack);
  for (auto d: defenders_list->children())
    {
      if (stack->size() == 8)
        {
          stack = new Stack(get_defender_player(), Vector<int>(-1,-1));
          defenders.push_back (stack);
        }
      Army *army = (*d)[combatant_columns.army];
      stack->add (army);
    }
  Tileset *tileset = GameMap::getTileset();
  int row = terrain_combobox->get_active_row_number();
  bool water = (*tileset)[row]->getType() == Tile::WATER;
  //put them all in the water if we're doing that.
  for (auto s : attackers)
    for (auto a : *s)
      a->setInShip(water);
  for (auto s : defenders)
    for (auto a : *s)
      a->setInShip(water);

  //fortify them if we're doing that
  for (auto s : defenders)
    for (auto a : *s)
      a->setFortified(fortified_switch->get_active());
  Fight f(attackers, defenders, city_switch->get_active(), Tile::Type(row));
  for (auto a: attackers_list->children())
    {
      Army *army = (*a)[combatant_columns.army];
      (*a)[combatant_columns.augmented_strength] =
        f.getModifiedStrengthBonus(army);
    }
  for (auto d: defenders_list->children())
    {
      Army *army = (*d)[combatant_columns.army];
      (*d)[combatant_columns.augmented_strength] =
        f.getModifiedStrengthBonus(army);
    }
  f.battle(die_sides_combobox->get_active_row_number() == 1);
  for (auto a: attackers_list->children())
    {
      Army *army = (*a)[combatant_columns.army];
      (*a)[combatant_columns.hp] = army->getHP();
    }
  for (auto d: defenders_list->children())
    {
      Army *army = (*d)[combatant_columns.army];
      (*d)[combatant_columns.hp] = army->getHP();
    }
  //reset the HP
  std::map<guint32,guint32> initial_hitpoints = f.getInitialHPs();
  for (auto a: attackers_list->children())
    {
      Army *army = (*a)[combatant_columns.army];
      army->setHP(initial_hitpoints[army->getId()]);
    }
  for (auto d: defenders_list->children())
    {
      Army *army = (*d)[combatant_columns.army];
      army->setHP(initial_hitpoints[army->getId()]);
    }
  //delete the stacks we made, but keep the armies
  for (auto s: attackers)
    {
      s->clear();
      delete s;
    }
  for (auto d: defenders)
    {
      d->clear();
      delete d;
    }
  return f.getResult();
}

void BattleCalculatorDialog::on_fight_clicked()
{
  run_battle ();
}

void BattleCalculatorDialog::on_fight100_clicked()
{
  int attacker_wins = 0;
  int defender_wins = 0;
  for (int i = 0; i < 100; i++)
    {
      switch (run_battle())
        {
        case Fight::ATTACKER_WON:
          attacker_wins++;
          break;
        case Fight::DEFENDER_WON:
          defender_wins++;
          break;
        case Fight::DRAW:
          break;
        }
    }

  //show results
    {
      Gtk::Dialog *d = new Gtk::Dialog();
      d->property_transient_for() = dialog;
      d->add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_ACCEPT);
      Gtk::Box *box = d->get_content_area ();
      d->set_title (_("Battle Outcome"));
      Glib::ustring s = 
        String::ucompose(ngettext("The attacker won %1 battle and lost %2.",
                                  "The attacker won %1 battles and lost %2.",
                                  attacker_wins),
                         attacker_wins, defender_wins);
      Gtk::Label l;
      l.set_text (s);
      l.set_margin_left (10);
      l.set_margin_right (10);
      l.set_margin_top (10);
      l.set_margin_bottom (10);
      box->add(l);
      box->show_all();
      d->run();
      delete d;
    }
}
