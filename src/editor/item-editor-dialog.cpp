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

#include "item-editor-dialog.h"

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

#define method(x) sigc::mem_fun(*this, &ItemEditorDialog::x)

ItemEditorDialog::ItemEditorDialog(Gtk::Window &parent, Item *item)
 : LwEditorDialog(parent, "item-editor-dialog.ui")
{
  d_changed = false;
  d_item = item;

  xml->get_widget("name_entry",  name_entry);
  name_entry->set_text (d_item->getName ());
  name_entry->signal_changed().connect(method(on_name_changed));

  xml->get_widget("bonus_label",  bonus_label);
  bonus_label->set_text (d_item->getBonusDescription ());

  xml->get_widget("uses_spinbutton",  uses_spinbutton);
  uses_spinbutton->set_value (d_item->getNumberOfUsesLeft ());
  uses_spinbutton->signal_value_changed().connect(method(on_uses_changed));
  uses_spinbutton->set_sensitive (d_item->isUsable ());
}

void ItemEditorDialog::on_name_changed ()
{
  d_changed = true;
  d_item->setName (String::utrim (name_entry->get_text ()));
}

void ItemEditorDialog::hide()
{
  dialog->hide();
}

bool ItemEditorDialog::run()
{
  dialog->show_all();
  dialog->run ();
  return d_changed;
}

void ItemEditorDialog::on_uses_changed ()
{
  d_changed = true;
  d_item->setNumberOfUsesLeft (uses_spinbutton->get_value ());
}
