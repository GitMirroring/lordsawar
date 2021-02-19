//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007-2009, 2012, 2014, 2015, 2017, 2020, 2021 Ben Asselstine
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
#include "stack-editor-actions.h"
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
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));
  stack = s;
  min_size = m;
  player_combobox = 0;
  d_changed = false;

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
  strength_column.set_cell_data_func(strength_renderer, method(cell_data_strength));
  army_treeview->append_column(strength_column);

  moves_renderer.property_editable() = true;
  moves_column.set_cell_data_func(moves_renderer, method(cell_data_moves));
  army_treeview->append_column(moves_column);

  upkeep_renderer.property_editable() = true;
  upkeep_column.set_cell_data_func(upkeep_renderer, method(cell_data_upkeep));
  army_treeview->append_column(upkeep_column);

  army_treeview->append_column(_("Name"), army_columns.name);

  xml->get_widget("fortified_switch", fortified_switch);
  xml->get_widget("add_button", add_button);
  xml->get_widget("remove_button", remove_button);
  xml->get_widget("copy_button", copy_button);
  xml->get_widget("edit_hero_button", edit_hero_button);
  xml->get_widget("undo_button", undo_button);
  undo_button->signal_activate ().connect (method (on_undo_activated));
  xml->get_widget("redo_button", redo_button);
  redo_button->signal_activate ().connect (method (on_redo_activated));

  add_button->signal_clicked().connect(method(on_add_clicked));
  remove_button->signal_clicked().connect(method(on_remove_clicked));
  copy_button->signal_clicked().connect(method(on_copy_clicked));
  edit_hero_button->signal_clicked().connect(method(on_edit_hero_clicked));

  fill_armies ();
  connect_signals ();
  update ();
}

void StackEditorDialog::fill_armies ()
{
  army_list->clear ();
  for (Stack::iterator i = stack->begin(), end = stack->end(); i != end; ++i)
    add_army(*i);
}

StackEditorDialog::~StackEditorDialog ()
{
  delete umgr;
}

bool StackEditorDialog::run()
{
  dialog->show_all();
  dialog->run();
  return d_changed;
}

/*
void StackEditorDialog::update_armies ()
{
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

  changeOwnership (stack->getOwner (), get_selected_player ());
}
*/

bool StackEditorDialog::changeOwnership (Player *old_player, Player *new_player)
{
  if (old_player->getId() != new_player->getId())
    {
      Player *old_active = Playerlist::getActiveplayer();
      Playerlist::getInstance()->setActiveplayer(new_player);
      Stack *new_stack = new Stack(*stack);
      GameMap::getStacks(new_stack->getPos())->remove(stack);
      stack->sdying.emit(stack);
      old_player->deleteStack(stack);
      new_stack->setPlayer(new_player);
      GameMap::getInstance()->putStack(new_stack);
      Playerlist::getInstance()->setActiveplayer(old_active);
      stack = new_stack;

      Stack::iterator i = stack->begin ();
      for (Gtk::TreeIter j = army_list->children().begin(),
           jend = army_list->children().end(); j != jend; ++j, ++i)
        (*j)[army_columns.army] = *i;
      return true;
    }
  return false;
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
      umgr->add (new StackEditorAction_Copy (new Stack (*stack)));
      Player *player = get_selected_player();
      Army *army = (*i)[army_columns.army];
      Army *new_army = new Army(*army, player);
      new_army->assignNewId();
      stack->add (new_army);
      add_army(new_army);
      d_changed = true;
      update ();
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
      bool ship = stack->hasShip();
      umgr->add (new StackEditorAction_Add (new Stack (*stack)));
      if (army->isHero() == true)
        {
          HeroProto *hp = new HeroProto(*army);
          Hero *hero = new Hero(*hp);
          hero->setOwnerId (player->getId ());
          hero->setInShip(ship);
          stack->add (hero);
          add_army(hero);
          delete hp;
        }
      else
        {
          Army *newarmy = new Army(*army, player);
          newarmy->setInShip(ship);
          stack->add (newarmy);
          add_army(newarmy);
        }
      d_changed = true;
    }
  update ();
}

void StackEditorDialog::on_edit_hero_clicked()
{
  Gtk::TreeIter i = army_treeview->get_selection()->get_selected();
  if (i)
    {
      Army *army = (*i)[army_columns.army];
      Hero *hero = dynamic_cast<Hero*>(army);
      StackEditorAction_HeroDetails *action =
        new StackEditorAction_HeroDetails (getCurIndex (), hero);

      HeroEditorDialog d(*dialog, hero);
      if (d.run())
        {
          umgr->add (action);
          (*i)[army_columns.name] = hero->getName();
          d_changed = true;
        }
      else
        delete action;
    }
  update ();
}

void StackEditorDialog::on_remove_clicked()
{
  Gtk::TreeIter i = army_treeview->get_selection()->get_selected();
  if (i)
    {
      umgr->add (new StackEditorAction_Remove (new Stack (*stack)));
      Army *army = (*i)[army_columns.army];
      army_list->erase(i);
      auto it = std::find(stack->begin(), stack->end(), army);
      if (it != stack->end())
        stack->flErase (it);
      d_changed = true;
      update ();
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
  umgr->add (new StackEditorAction_Fortify (stack->getFortified ()));
  stack->setFortified(fortified_switch->get_active());
  update ();
  d_changed = true;
}

void StackEditorDialog::on_player_changed()
{
  umgr->add (new StackEditorAction_Owner (stack->getOwner (),
                                          stack->getFortified ()));
  ImageCache *gc = ImageCache::getInstance();
  Player *player = get_selected_player();
  if (player == Playerlist::getInstance()->getNeutral())
    fortified_switch->set_active(false);
  set_button_sensitivity();

  changeOwnership (stack->getOwner (), player);
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
      
  d_changed = true;
  update ();
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
  (void)path;
  int str = atoi(new_text.c_str());
  if (str < (int)MIN_STRENGTH_FOR_ARMY_UNITS || str >
      (int)MAX_STRENGTH_FOR_ARMY_UNITS)
    return;
  umgr->add (new StackEditorAction_Strength (getCurIndex (),
                                             getCurArmy ()->getStrength ()));
  getCurArmy ()->setStrength (str);
  d_changed = true;
  update ();
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
  (void)path;
  int moves = atoi(new_text.c_str());
  if (moves < (int)MIN_MOVES_FOR_ARMY_UNITS || moves >
      (int)MAX_MOVES_FOR_ARMY_UNITS)
    return;
  umgr->add (new StackEditorAction_Moves (getCurIndex (),
                                          getCurArmy ()->getMaxMoves ()));
  getCurArmy ()->setMaxMoves (moves);
  d_changed = true;
  update ();
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
  (void)path;
  int upkeep = atoi(new_text.c_str());
  if (upkeep < (int)MIN_UPKEEP_FOR_ARMY_UNITS ||
      upkeep > (int)MAX_UPKEEP_FOR_ARMY_UNITS)
    return;
  umgr->add (new StackEditorAction_Upkeep (getCurIndex (),
                                           getCurArmy ()->getUpkeep ()));
  getCurArmy ()->setUpkeep (upkeep);
  d_changed = true;
  update ();
}

int StackEditorDialog::getCurIndex ()
{
  int idx = -1;
  Gtk::TreeIter i = army_treeview->get_selection()->get_selected();
  if (i)
    {
      auto path = army_treeview->get_model ()->get_path (i);
      idx = atoi (path.to_string ().c_str ());
    }
  return idx;
}

Army* StackEditorDialog::getCurArmy ()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = army_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      // Row selected
      Gtk::TreeModel::Row row = *iterrow;

      return row[army_columns.army];
    }
  return NULL;
}

void StackEditorDialog::on_undo_activated ()
{
  umgr->undo ();
  if (umgr->undoEmpty ())
    d_changed = false;
  update ();
}

void StackEditorDialog::on_redo_activated ()
{
  umgr->redo ();
  d_changed = true;
  update ();
}

void StackEditorDialog::update ()
{
  disconnect_signals ();
  fortified_switch->set_active(stack->getFortified());
  int c = 0, player_no = 0;
  for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
       end = Playerlist::getInstance()->end(); i != end; ++i, ++c)
    {
      if ((*i)== stack->getOwner())
        player_no = c;
    }
  player_combobox->set_active(player_no);

  ImageCache *gc = ImageCache::getInstance();
  Player *player = stack->getOwner ();
  guint32 fs = FontSize::getInstance ()->get_height ();
  for (Gtk::TreeIter j = army_list->children().begin(),
       jend = army_list->children().end(); j != jend; ++j)
    {
      Army *a = (*j)[army_columns.army];
      (*j)[army_columns.image] = gc->getArmyPic(player->getArmyset(),
                                                a->getTypeId(),
                                                player, NULL, false,
                                                fs)->to_pixbuf();
      (*j)[army_columns.strength] = a->getStrength ();
      (*j)[army_columns.moves] = a->getMaxMoves ();
      (*j)[army_columns.upkeep] = a->getUpkeep ();
    }
  set_button_sensitivity();
  connect_signals ();
}

void StackEditorDialog::connect_signals ()
{
  connections.push_back
    (player_combobox->signal_changed().connect (method(on_player_changed)));
  connections.push_back
    (strength_renderer.signal_edited().connect(method(on_strength_edited)));
  connections.push_back
    (moves_renderer.signal_edited().connect(method(on_moves_edited)));
  connections.push_back
    (upkeep_renderer.signal_edited().connect(method(on_upkeep_edited)));
  connections.push_back
    (fortified_switch->property_active().signal_changed().connect
     (method(on_fortified_toggled)));
  connections.push_back
    (army_treeview->get_selection()->signal_changed().connect
     (method(on_selection_changed)));
}

void StackEditorDialog::disconnect_signals ()
{
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}

Army* StackEditorDialog::getArmyByIndex (StackEditorAction_Index *a)
{
  auto path = Gtk::TreePath (String::ucompose ("%1", a->getIndex ()));
  auto iterrow = army_treeview->get_model ()->get_iter (path);
  Gtk::TreeModel::Row row = *iterrow;
  Army *army = row[army_columns.army];
  return army;
}

void StackEditorDialog::populate_stack_with_armies (Stack *s)
{
  for (auto it = stack->begin (); it != stack->end (); ++it)
    delete *it;
  stack->clear ();
  for (auto it = s->begin (); it != s->end (); ++it)
    {
      if ((*it)->isHero ())
        stack->add (new Hero (*dynamic_cast<Hero*>(*it)));
      else
        stack->add (new Army (*(*it)));
    }
}

UndoAction *StackEditorDialog::executeAction (UndoAction *action2)
{
  StackEditorAction *action = dynamic_cast<StackEditorAction*>(action2);
  UndoAction *out = NULL;

  switch (action->getType ())
    {
      case StackEditorAction::OWNER:
          {
            StackEditorAction_Owner *a =
              dynamic_cast<StackEditorAction_Owner*>(action);
            out = new StackEditorAction_Owner (stack->getOwner (),
                                               stack->getFortified ());
            stack->setFortified (a->getFortify ());
            disconnect_signals ();
            changeOwnership (stack->getOwner (), a->getOwner ());
            connect_signals ();
          }
        break;
      case StackEditorAction::FORTIFY:
          {
            StackEditorAction_Fortify *a =
              dynamic_cast<StackEditorAction_Fortify*>(action);
            out = new StackEditorAction_Fortify (stack->getFortified ());
            stack->setFortified (a->getFortify ());
          }
        break;
      case StackEditorAction::ADD:
          {
            StackEditorAction_Add *a =
              dynamic_cast<StackEditorAction_Add*>(action);
            out = new StackEditorAction_Add (new Stack (*stack));
            populate_stack_with_armies (a->getStack ());
            disconnect_signals ();
            fill_armies ();
            connect_signals ();
          }
        break;
      case StackEditorAction::REMOVE:
          {
            StackEditorAction_Remove *a =
              dynamic_cast<StackEditorAction_Remove*>(action);
            out = new StackEditorAction_Remove (new Stack (*stack));
            populate_stack_with_armies (a->getStack ());
            disconnect_signals ();
            fill_armies ();
            connect_signals ();
          }
        break;
      case StackEditorAction::COPY:
          {
            StackEditorAction_Copy *a =
              dynamic_cast<StackEditorAction_Copy*>(action);
            out = new StackEditorAction_Copy (new Stack (*stack));
            populate_stack_with_armies (a->getStack ());
            disconnect_signals ();
            fill_armies ();
            connect_signals ();
          }
        break;
      case StackEditorAction::STRENGTH:
          {
            StackEditorAction_Strength *a =
              dynamic_cast<StackEditorAction_Strength*>(action);
            out = new StackEditorAction_Strength
              (a->getIndex (), getArmyByIndex (a)->getStrength ());

            getArmyByIndex (a)->setStrength (a->getStrength ());
          }
        break;
      case StackEditorAction::MOVES:
          {
            StackEditorAction_Moves *a =
              dynamic_cast<StackEditorAction_Moves*>(action);
            out = new StackEditorAction_Moves
              (a->getIndex (), getArmyByIndex (a)->getMaxMoves ());

            getArmyByIndex (a)->setMaxMoves (a->getMoves ());
          }
        break;
      case StackEditorAction::UPKEEP:
          {
            StackEditorAction_Upkeep *a =
              dynamic_cast<StackEditorAction_Upkeep*>(action);
            out = new StackEditorAction_Upkeep
              (a->getIndex (), getArmyByIndex (a)->getUpkeep ());

            getArmyByIndex (a)->setUpkeep (a->getUpkeep ());
          }
        break;
      case StackEditorAction::HERO_DETAILS:
          {
            StackEditorAction_HeroDetails *a =
              dynamic_cast<StackEditorAction_HeroDetails*>(action);
            out = new StackEditorAction_HeroDetails
              (a->getIndex (), dynamic_cast<Hero*>(getArmyByIndex (a)));

            guint32 row = 0;
            for (auto it = stack->begin (); it != stack->end (); ++it, row++)
              {
                if (row == a->getIndex ())
                  {
                    delete *it;
                    *it = new Hero (*a->getHero ());
                    break;
                  }
              }
          }
        break;
    }
  return out;
}
