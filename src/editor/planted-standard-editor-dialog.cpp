//  Copyright (C) 2020 Ben Asselstine
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
#include <vector>

#include "planted-standard-editor-dialog.h"

#include "ucompose.hpp"
#include "defs.h"
#include "Item.h"
#include "select-item-dialog.h"
#include "playerlist.h"
#include "player.h"
#include "MapBackpack.h"
#include "Item.h"
#include "GameMap.h"
#include "Itemlist.h"

#define method(x) sigc::mem_fun(*this, &PlantedStandardEditorDialog::x)

PlantedStandardEditorDialog::PlantedStandardEditorDialog(Gtk::Window &parent, Vector<int> tile)
 : LwEditorDialog(parent, "planted-standard-editor-dialog.ui")
{
  d_changed = false;
  Gtk::Box *box;
  xml->get_widget("owner_box", box);
  d_map_backpack = GameMap::getInstance ()->getBackpack(tile);
  owner_combobox = Gtk::manage (new Gtk::ComboBoxText);

  d_item = d_map_backpack->getFirstPlantedItem ();
  Player *p = Playerlist::getInstance()->getActiveplayer ();
  int c = 0, player_no = p->getId ();
  for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
       end = Playerlist::getInstance()->end(); i != end; ++i, ++c)
    {
      Player *player = *i;
      owner_combobox->append(player->getName());
      if (d_item && player == d_item->getPlantableOwner ())
        player_no = c;
    }

  owner_combobox->set_active(player_no);
  owner_combobox->signal_changed().connect(method(on_owner_changed));
  box->pack_start(*owner_combobox, Gtk::PACK_SHRINK);

  xml->get_widget("orig_owner_box", box);
  orig_owner_combobox = Gtk::manage (new Gtk::ComboBoxText);
  c = 0;
  player_no = p->getId ();
  for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
       end = Playerlist::getInstance()->end(); i != end; ++i, ++c)
    {
      Player *player = *i;
      orig_owner_combobox->append(player->getName());
      if (d_item && player == d_item->getPlantableOriginalOwner ())
        player_no = c;
    }
  orig_owner_combobox->set_active(player_no);
  orig_owner_combobox->signal_changed().connect(method(on_orig_owner_changed));
  box->pack_start(*orig_owner_combobox, Gtk::PACK_SHRINK);


  xml->get_widget("name_entry",  name_entry);
  if (d_item != NULL)
    name_entry->set_text (d_item->getName ());
  else
    {
      Glib::ustring name = String::ucompose(_("%1 Standard"),  p->getName ());
      name_entry->set_text (name);
      d_item = new Item (name, true, p);
      d_item->addBonus(Item::ADD1STACK);
      d_item->setPlanted (true);
      d_item->setPlantableOwnerId (p->getId ());
      d_map_backpack->addToBackpack (d_item);
      d_changed = true;
    }
  name_entry->signal_changed().connect(method(on_name_changed));
  xml->get_widget("bonus_label",  bonus_label);
  bonus_label->set_text (d_item->getBonusDescription ());
}

void PlantedStandardEditorDialog::on_owner_changed ()
{
  int row = owner_combobox->get_active_row_number ();
  int c = 0;
  for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
       end = Playerlist::getInstance()->end(); i != end; ++i, ++c)
    {
      if (c == row)
        d_item->setPlantableOwnerId ((*i)->getId());
    }
  d_changed = true;
}

void PlantedStandardEditorDialog::on_orig_owner_changed ()
{
  int row = orig_owner_combobox->get_active_row_number ();
  int c = 0;
  for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
       end = Playerlist::getInstance()->end(); i != end; ++i, ++c)
    {
      if (c == row)
        d_item->setPlantableOriginalOwnerId ((*i)->getId());
    }
  d_changed = true;
}

void PlantedStandardEditorDialog::on_name_changed ()
{
  d_changed = true;
  d_item->setName (String::utrim (name_entry->get_text ()));
}

void PlantedStandardEditorDialog::hide()
{
  dialog->hide();
}

bool PlantedStandardEditorDialog::run()
{
  dialog->show_all();
  dialog->run ();
  return d_changed;
}
