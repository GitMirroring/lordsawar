//  Copyright (C) 2015, 2020 Ben Asselstine
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

#define method(x) sigc::mem_fun(*this, &FightOrderEditorDialog::x)

FightOrderEditorDialog::FightOrderEditorDialog(Gtk::Window &parent)
 : LwEditorDialog(parent, "fight-order-editor-dialog.ui")
{
  modified = false;
  armies_list = Gtk::ListStore::create(armies_columns);
  xml->get_widget("treeview", armies_treeview);
  armies_treeview->set_model(armies_list);
  armies_treeview->append_column("", armies_columns.image);
  armies_treeview->append_column("", armies_columns.name);
  armies_treeview->set_reorderable(true);
  armies_treeview->signal_drag_end().connect(sigc::hide(method(on_army_reordered)));
  fill_armies(Playerlist::getActiveplayer());

  player_combobox = new Gtk::ComboBoxText;
  for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
       end = Playerlist::getInstance()->end(); i != end; ++i)
    {
      player_combobox->append((*i)->getName());
      if (*i == Playerlist::getActiveplayer())
        player_combobox->set_active_text((*i)->getName());
    }

  player_combobox->signal_changed().connect (method(on_player_changed));
  Gtk::Alignment *alignment;
  xml->get_widget("players_alignment", alignment);
  alignment->add(*Gtk::manage(player_combobox));
  player_combobox->show_all();

  xml->get_widget("make_same_button", make_same_button);
  make_same_button->signal_clicked().connect (method (on_make_same_button_clicked));
  make_same_button->set_sensitive(Playerlist::getInstance()->size() != 1);
}

void FightOrderEditorDialog::hide()
{
  dialog->hide();
}

int FightOrderEditorDialog::run()
{
    dialog->show();
    int response = dialog->run();

    if (response == Gtk::RESPONSE_ACCEPT)
      {
      }
    return response;
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
  Player *player = get_selected_player();
  for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
       end = Playerlist::getInstance()->end(); i != end; ++i)
    {
      if ((*i) != player)
        (*i)->setFightOrder(player->getFightOrder());
    }
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
  Player *player = get_selected_player();
  fill_armies(player);
}

void FightOrderEditorDialog::fill_armies(Player *player)
{
  armies_list->clear();
  std::list<guint32> fight_order = player->getFightOrder();
  std::list<guint32>::iterator it = fight_order.begin();
  for (; it != fight_order.end(); it++)
    addArmyType(*it, player);
}

void FightOrderEditorDialog::on_army_reordered ()
{
  Player *player = get_selected_player();
  std::list<guint32> fight_order;
  for (Gtk::TreeIter i = armies_list->children().begin(),
       end = armies_list->children().end(); i != end; ++i) 
    fight_order.push_back((*i)[armies_columns.army_type]);
  player->setFightOrder(fight_order);
  modified = true;
}
