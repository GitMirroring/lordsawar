//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007-2009, 2012, 2014, 2015, 2017, 2020 Ben Asselstine
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

#include "stack-editor-dialog.h"

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
#include "font-size.h"

#include "select-army-dialog.h"

#define method(x) sigc::mem_fun(*this, &StackEditorDialog::x)

namespace
{
  //FIXME this should be MAX_STACK_SIZE from defs.h
    int const max_stack_size = 8;
}

StackEditorDialog::StackEditorDialog(Gtk::Window &parent, Stack *s, int m)
 : LwEditorDialog(parent, "stack-editor-dialog.ui"),
    strength_column(_("Strength"), strength_renderer),
	moves_column(_("Max Moves"), moves_renderer),
	upkeep_column(_("Upkeep"), upkeep_renderer)
{
  stack = s;
  min_size = m;
  player_combobox = 0;

  if (stack->getOwner())
    {
      // setup the player combo
      player_combobox = manage(new Gtk::ComboBoxText);

      int c = 0, player_no = 0;
      for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
           end = Playerlist::getInstance()->end(); i != end; ++i, ++c)
        {
          Player *player = *i;
          player_combobox->append(player->getName());
          if (player == stack->getOwner())
            player_no = c;
        }

      player_combobox->set_active(player_no);
      player_combobox->signal_changed().connect (method(on_player_changed));

      Gtk::Box *box;
      xml->get_widget("player_hbox", box);
      box->pack_start(*player_combobox, Gtk::PACK_SHRINK);
    }

  // setup the army list
  army_list = Gtk::ListStore::create(army_columns);

  xml->get_widget("army_treeview", army_treeview);
  army_treeview->set_model(army_list);

  army_treeview->append_column("", army_columns.image);

  strength_renderer.property_editable() = true;
  strength_renderer.signal_edited().connect(method(on_strength_edited));
  strength_column.set_cell_data_func(strength_renderer, method(cell_data_strength));
  army_treeview->append_column(strength_column);

  moves_renderer.property_editable() = true;
  moves_renderer.signal_edited().connect(method(on_moves_edited));
  moves_column.set_cell_data_func(moves_renderer, method(cell_data_moves));
  army_treeview->append_column(moves_column);

  upkeep_renderer.property_editable() = true;
  upkeep_renderer.signal_edited().connect(method(on_upkeep_edited));
  upkeep_column.set_cell_data_func(upkeep_renderer, method(cell_data_upkeep));
  army_treeview->append_column(upkeep_column);

  army_treeview->append_column(_("Name"), army_columns.name);

  xml->get_widget("fortified_switch", fortified_switch);
  fortified_switch->set_active(stack->getFortified());
  fortified_switch->property_active().signal_changed().connect(method(on_fortified_toggled));

  xml->get_widget("add_button", add_button);
  xml->get_widget("remove_button", remove_button);
  xml->get_widget("copy_button", copy_button);
  xml->get_widget("edit_hero_button", edit_hero_button);

  add_button->signal_clicked().connect(method(on_add_clicked));
  remove_button->signal_clicked().connect(method(on_remove_clicked));
  copy_button->signal_clicked().connect(method(on_copy_clicked));
  edit_hero_button->signal_clicked().connect(method(on_edit_hero_clicked));

  army_treeview->get_selection()->signal_changed().connect(method(on_selection_changed));

  for (Stack::iterator i = stack->begin(), end = stack->end(); i != end; ++i)
    add_army(*i);
  set_button_sensitivity();
}

int StackEditorDialog::run()
{
  dialog->show_all();
  return dialog->run();
}

void StackEditorDialog::update_armies ()
{
  // remove removed armies from stack
  for (Stack::iterator i = stack->begin(), end = stack->end(); i != end;)
    {
      Army *a = *i;
      ++i;

      bool found = false;
      for (Gtk::TreeIter j = army_list->children().begin(),
           jend = army_list->children().end(); j != jend; ++j)
        if ((*j)[army_columns.army] == a)
          {
            found = true;
            break;
          }

      if (!found)
        {
          stack->remove(a);
          delete a;
        }
    }

  //set the stats for all of the armies in our list
  for (Gtk::TreeIter j = army_list->children().begin(),
       jend = army_list->children().end(); j != jend; ++j)
    {
      Army *a = (*j)[army_columns.army];
      a->setStat(Army::STRENGTH, (*j)[army_columns.strength]);
      a->setMaxMoves((*j)[army_columns.moves]);
      a->setStat(Army::MOVES, (*j)[army_columns.moves]);
      a->setUpkeep((*j)[army_columns.upkeep]);
    }
  bool ship = stack->hasShip();
  // add added armies to stack
  for (Gtk::TreeIter j = army_list->children().begin(),
       jend = army_list->children().end(); j != jend; ++j)
    {
      Army *a = (*j)[army_columns.army];

      a->setInShip(ship);
      if (std::find(stack->begin(), stack->end(), a) == stack->end())
        stack->push_back(a);
    }

  // now set allegiance, it's important to do it after possibly new stack
  // armies have been added
  // this also helps the stack ship icon show up when it's needed.

  if (get_selected_player()->getId() != stack->getOwner()->getId())
    {
      Player *player = get_selected_player();
      Player *old_active = Playerlist::getActiveplayer();
      Playerlist::getInstance()->setActiveplayer(player);
      Stack *new_stack = new Stack(*stack);
      GameMap::getStacks(new_stack->getPos())->remove(stack);
      stack->sdying.emit(stack);
      stack->getOwner()->deleteStack(stack);
      new_stack->setPlayer(player);
      GameMap::getInstance()->putStack(new_stack);
      Playerlist::getInstance()->setActiveplayer(old_active);
      stack = new_stack;
  
      Stack::iterator i = stack->begin ();
      for (Gtk::TreeIter j = army_list->children().begin(),
           jend = army_list->children().end(); j != jend; ++j, ++i)
      (*j)[army_columns.army] = *i;
    }
}

Player *StackEditorDialog::get_selected_player()
{
  int c = 0, row = player_combobox->get_active_row_number();
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

void StackEditorDialog::on_copy_clicked()
{
  Gtk::TreeIter i = army_treeview->get_selection()->get_selected();
  if (i)
    {
      Player *player = get_selected_player();
      Army *army = (*i)[army_columns.army];
      Army *new_army = new Army(*army, player);
      new_army->assignNewId();
      add_army(new_army);
      update_armies ();
    }

  set_button_sensitivity();
}

void StackEditorDialog::on_add_clicked()
{
  SelectArmyDialog d(*dialog, SelectArmyDialog::SELECT_NORMAL_WITH_HERO,
                     stack->getOwner(), -1);
  d.run();

  Player *player = get_selected_player();
  const ArmyProto *army = d.get_selected_army();
  if (army)
    {
      if (army->isHero() == true)
        {
          HeroProto *hp = new HeroProto(*army);
          hp->setOwnerId(player->getId());
          add_army(new Hero(*hp));
          delete hp;
        }
      else
        add_army(new Army(*army, player));
      update_armies ();
    }
}

void StackEditorDialog::on_edit_hero_clicked()
{
  Gtk::TreeIter i = army_treeview->get_selection()->get_selected();
  if (i)
    {
      Army *army = (*i)[army_columns.army];
      Hero *hero = dynamic_cast<Hero*>(army);
      HeroEditorDialog d(*dialog, hero);
      d.run();
      (*i)[army_columns.name] = hero->getName();
      update_armies ();
    }

}

void StackEditorDialog::on_remove_clicked()
{
  Gtk::TreeIter i = army_treeview->get_selection()->get_selected();
  if (i)
    {
      Army *army = (*i)[army_columns.army];
      army_list->erase(i);
      if (std::find(stack->begin(), stack->end(), army) == stack->end())
        delete army;
      update_armies ();
    }

  set_button_sensitivity();
}

void StackEditorDialog::add_army(Army *a)
{
  ImageCache *gc = ImageCache::getInstance();
  Gtk::TreeIter i = army_list->append();
  (*i)[army_columns.army] = a;
  guint32 fs = FontSize::getInstance ()->get_height ();
  (*i)[army_columns.image] = gc->getArmyPic(a->getOwner()->getArmyset(),
                                            a->getTypeId(), a->getOwner(),
                                            NULL, false, fs)->to_pixbuf();
  (*i)[army_columns.strength] = a->getStat(Army::STRENGTH, false);
  (*i)[army_columns.moves] = a->getStat(Army::MOVES, false);
  (*i)[army_columns.upkeep] = a->getUpkeep();
  (*i)[army_columns.name] = a->getName();

  army_treeview->get_selection()->select(i);

  set_button_sensitivity();
}

void StackEditorDialog::on_selection_changed()
{
  set_button_sensitivity();
}

void StackEditorDialog::set_button_sensitivity()
{
  Gtk::TreeIter i = army_treeview->get_selection()->get_selected();
  int armies = army_list->children().size();
  add_button->set_sensitive(armies < max_stack_size);
  copy_button->set_sensitive(armies < max_stack_size);
  remove_button->set_sensitive(armies > min_size && i);
  if (i)
    {
      Army *army = (*i)[army_columns.army];
      if (army->isHero())
        {
          edit_hero_button->set_sensitive(true);
          copy_button->set_sensitive(false);
        }
      else
        edit_hero_button->set_sensitive(false);
    }
  Player *player = get_selected_player();
  bool neutral = player == Playerlist::getInstance()->getNeutral();
  bool can_defend = GameMap::getInstance()->can_defend (stack);
  if (neutral || !can_defend)
    fortified_switch->set_sensitive(false);
  else
    fortified_switch->set_sensitive(true);
}

void StackEditorDialog::on_fortified_toggled()
{
  stack->setFortified(fortified_switch->get_active());
}

void StackEditorDialog::on_player_changed()
{
  ImageCache *gc = ImageCache::getInstance();
  Player *player = get_selected_player();
  if (player == Playerlist::getInstance()->getNeutral())
    fortified_switch->set_active(false);
  set_button_sensitivity();

  guint32 fs = FontSize::getInstance ()->get_height ();
  for (Gtk::TreeIter j = army_list->children().begin(),
       jend = army_list->children().end(); j != jend; ++j)
    {
      Army *a = (*j)[army_columns.army];
      (*j)[army_columns.image] = gc->getArmyPic(player->getArmyset(),
                                                a->getTypeId(),
                                                player, NULL, false,
                                                fs)->to_pixbuf();
    }
      
  update_armies ();
}

void StackEditorDialog::cell_data_strength(Gtk::CellRenderer *renderer,
				     const Gtk::TreeIter& i)
{
  dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_adjustment()
    = Gtk::Adjustment::create((*i)[army_columns.strength],
                              MIN_STRENGTH_FOR_ARMY_UNITS,
                              MAX_STRENGTH_FOR_ARMY_UNITS, 1);
  dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_text() =
    String::ucompose("%1", (*i)[army_columns.strength]);
}

void StackEditorDialog::on_strength_edited(const Glib::ustring &path,
				   const Glib::ustring &new_text)
{
  int str = atoi(new_text.c_str());
  if (str < (int)MIN_STRENGTH_FOR_ARMY_UNITS || str >
      (int)MAX_STRENGTH_FOR_ARMY_UNITS)
    return;
  (*army_list->get_iter(Gtk::TreePath(path)))[army_columns.strength] = str;
  update_armies ();
}

void StackEditorDialog::cell_data_moves(Gtk::CellRenderer *renderer,
				  const Gtk::TreeIter& i)
{
  dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_adjustment()
    = Gtk::Adjustment::create((*i)[army_columns.moves],
                              MIN_MOVES_FOR_ARMY_UNITS,
                              MAX_MOVES_FOR_ARMY_UNITS, 1);
  dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_text() =
    String::ucompose("%1", (*i)[army_columns.moves]);
}

void StackEditorDialog::on_moves_edited(const Glib::ustring &path,
				   const Glib::ustring &new_text)
{
  int moves = atoi(new_text.c_str());
  if (moves < (int)MIN_MOVES_FOR_ARMY_UNITS || moves >
      (int)MAX_MOVES_FOR_ARMY_UNITS)
    return;
  (*army_list->get_iter(Gtk::TreePath(path)))[army_columns.moves] = moves;
  update_armies ();
}

void StackEditorDialog::cell_data_upkeep(Gtk::CellRenderer *renderer,
				   const Gtk::TreeIter& i)
{
  dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_adjustment()
    = Gtk::Adjustment::create((*i)[army_columns.upkeep],
                              MIN_UPKEEP_FOR_ARMY_UNITS,
                              MAX_UPKEEP_FOR_ARMY_UNITS, 1);
  dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_text() =
    String::ucompose("%1", (*i)[army_columns.upkeep]);
}

void StackEditorDialog::on_upkeep_edited(const Glib::ustring &path,
				   const Glib::ustring &new_text)
{
  int upkeep = atoi(new_text.c_str());
  if (upkeep < (int)MIN_UPKEEP_FOR_ARMY_UNITS ||
      upkeep > (int)MAX_UPKEEP_FOR_ARMY_UNITS)
    return;
  (*army_list->get_iter(Gtk::TreePath(path)))[army_columns.upkeep] = upkeep;
  update_armies ();
}
