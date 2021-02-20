//  Copyright (C) 2015, 2020, 2021 Ben Asselstine
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

#include "fight-order-editor-dialog.h"

#include <gtkmm.h>
#include "player.h"
#include "armysetlist.h"
#include "ImageCache.h"
#include "playerlist.h"
#include "font-size.h"
#include "fight-order-editor-actions.h"

#define method(x) sigc::mem_fun(*this, &FightOrderEditorDialog::x)

FightOrderEditorDialog::FightOrderEditorDialog(Gtk::Window &parent)
 : LwEditorDialog(parent, "fight-order-editor-dialog.ui")
{
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));
  d_changed = false;
  armies_list = Gtk::ListStore::create(armies_columns);
  xml->get_widget("treeview", armies_treeview);
  armies_treeview->set_model(armies_list);
  armies_treeview->append_column("", armies_columns.image);
  armies_treeview->append_column("", armies_columns.name);
  armies_treeview->set_reorderable(true);

  player_combobox = new Gtk::ComboBoxText;
  int counter = 0;
  for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
       end = Playerlist::getInstance()->end(); i != end; ++i, counter++)
    {
      player_combobox->append((*i)->getName());
      if (*i == Playerlist::getActiveplayer())
        {
          player_combobox->set_active_text((*i)->getName());
          owner_row = counter;
          player_combobox->set_active (counter);
        }
    }

  Gtk::Box *alignment;
  xml->get_widget("players_alignment", alignment);
  alignment->add(*Gtk::manage(player_combobox));
  player_combobox->show_all();

  xml->get_widget("make_same_button", make_same_button);
  make_same_button->signal_clicked().connect (method (on_make_same_button_clicked));

  xml->get_widget("undo_button", undo_button);
  undo_button->signal_activate ().connect (method (on_undo_activated));
  xml->get_widget("redo_button", redo_button);
  redo_button->signal_activate ().connect (method (on_redo_activated));

  connect_signals ();
  update ();
}

FightOrderEditorDialog::~FightOrderEditorDialog()
{
  delete umgr;
}

void FightOrderEditorDialog::hide()
{
  dialog->hide();
}

bool FightOrderEditorDialog::run()
{
    dialog->show();
    dialog->run();
    return d_changed;
}

void FightOrderEditorDialog::addArmyType(guint32 army_type, Player *player)
{
    ImageCache *gc = ImageCache::getInstance();
  Gtk::TreeIter i = armies_list->append();
  Armysetlist *alist = Armysetlist::getInstance();
  const ArmyProto *a = alist->getArmy(player->getArmyset(), army_type);
  (*i)[armies_columns.name] = a->getName();
  (*i)[armies_columns.image] = 
    gc->getCircledArmyPic(player->getArmyset(), army_type, player, NULL,
                          false, player->getId(), true,
                          FontSize::getInstance ()->get_height ())->to_pixbuf();
  (*i)[armies_columns.army_type] = a->getId();
}

void FightOrderEditorDialog::on_make_same_button_clicked()
{
  FightOrderEditorAction_MakeSame *action =
    new FightOrderEditorAction_MakeSame (owner_row, get_all_fight_orders ());
  bool modified = false;
  Player *player = get_selected_player();
  for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
       end = Playerlist::getInstance()->end(); i != end; ++i)
    {
      if ((*i) != player)
        {
          if (player->getFightOrder () != (*i)->getFightOrder ())
            {
              (*i)->setFightOrder(player->getFightOrder());
              modified = true;
            }
        }
    }
  if (modified)
    {
      umgr->add (action);
      d_changed = true;
    }
  else
    delete action;
}

Player *FightOrderEditorDialog::get_selected_player()
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

void FightOrderEditorDialog::on_player_changed()
{
  umgr->add (new FightOrderEditorAction_Owner (owner_row));
  owner_row = player_combobox->get_active_row_number ();
  update ();
}

void FightOrderEditorDialog::fill_armies(Player *player)
{
  armies_list->clear();
  std::list<guint32> fight_order = player->getFightOrder();
  for (std::list<guint32>::iterator it = fight_order.begin();
       it != fight_order.end(); ++it)
    addArmyType(*it, player);
}

void FightOrderEditorDialog::on_army_reordered ()
{
  Player *player = get_selected_player();
  umgr->add (new FightOrderEditorAction_Order (player->getId (),
                                               player->getFightOrder ()));
  std::list<guint32> fight_order;
  for (Gtk::TreeIter i = armies_list->children().begin(),
       end = armies_list->children().end(); i != end; ++i) 
    fight_order.push_back((*i)[armies_columns.army_type]);
  player->setFightOrder(fight_order);
  d_changed = true;
}

void FightOrderEditorDialog::on_undo_activated ()
{
  umgr->undo ();
  if (umgr->undoEmpty ())
    d_changed = false;
  update ();
}

void FightOrderEditorDialog::on_redo_activated ()
{
  umgr->redo ();
  d_changed = true;
  update ();
}

void FightOrderEditorDialog::update ()
{
  disconnect_signals ();
  player_combobox->set_active (owner_row);
  fill_armies (get_selected_player ());
  make_same_button->set_sensitive(Playerlist::getInstance()->size() != 1);
  connect_signals ();
}

void FightOrderEditorDialog::connect_signals ()
{
  connections.push_back
    (player_combobox->signal_changed().connect (method(on_player_changed)));
  connections.push_back
    (armies_treeview->signal_drag_end().connect
     (sigc::hide(method(on_army_reordered))));
}

void FightOrderEditorDialog::disconnect_signals ()
{
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}

std::list<guint32> FightOrderEditorDialog::get_fight_order (guint32 id)
{
  Player *p = Playerlist::getInstance ()->getPlayer (id);
  return p->getFightOrder ();
}

std::list<std::list<guint32> >FightOrderEditorDialog::get_all_fight_orders ()
{
  std::list<std::list<guint32> > orders;
  for (auto p : *Playerlist::getInstance ())
    orders.push_back (p->getFightOrder ());
  return orders;
}

UndoAction *FightOrderEditorDialog::executeAction (UndoAction *action2)
{
  FightOrderEditorAction *action =
    dynamic_cast<FightOrderEditorAction*>(action2);
  UndoAction *out = NULL;

  switch (action->getType ())
    {
      case FightOrderEditorAction::ORDER:
          {
            FightOrderEditorAction_Order *a =
              dynamic_cast<FightOrderEditorAction_Order*>(action);
            out = new FightOrderEditorAction_Order
              (a->getPlayerId (), get_fight_order (a->getPlayerId ()));

            Player *p =
              Playerlist::getInstance ()->getPlayer (a->getPlayerId ());
            p->setFightOrder (a->getFightOrder ());
          } 
        break;
      case FightOrderEditorAction::MAKE_SAME:
          {
            FightOrderEditorAction_MakeSame *a =
              dynamic_cast<FightOrderEditorAction_MakeSame*>(action);
            out = new FightOrderEditorAction_MakeSame
              (owner_row, get_all_fight_orders ());

            owner_row = a->getRow ();
            guint32 id = 0;
            for (auto o : a->getFightOrders ())
              {
                Player *p = Playerlist::getInstance ()->getPlayer (id);
                p->setFightOrder (o);
                id++;
              }
          }
        break;
      case FightOrderEditorAction::OWNER:
          {
            FightOrderEditorAction_Owner *a =
              dynamic_cast<FightOrderEditorAction_Owner*>(action);
            out = new FightOrderEditorAction_Owner (owner_row);

            owner_row = a->getRow ();
          }
        break;
    }
  return out;
}
