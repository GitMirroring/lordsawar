//  Copyright (C) 2009, 2011, 2014, 2020 Ben Asselstine
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

#include "backpack-editor-dialog.h"

#include "ucompose.hpp"
#include "defs.h"
#include "Item.h"
#include "ItemProto.h"
#include "Backpack.h"
#include "select-item-dialog.h"
#include "item-editor-dialog.h"

#define method(x) sigc::mem_fun(*this, &BackpackEditorDialog::x)

BackpackEditorDialog::BackpackEditorDialog(Gtk::Window &parent, Backpack *pack)
 : LwEditorDialog(parent, "backpack-editor-dialog.ui")
{
  d_changed = false;
  backpack = pack;

  xml->get_widget("remove_button", remove_button);
  xml->get_widget("edit_button", edit_button);
  xml->get_widget("add_button", add_button);
  remove_button->signal_clicked().connect(method(on_remove_item_clicked));
  add_button->signal_clicked().connect(method(on_add_item_clicked));
  edit_button->signal_clicked().connect(method(on_edit_item_clicked));

  item_list = Gtk::ListStore::create(item_columns);
  xml->get_widget("treeview", item_treeview);
  item_treeview->set_model(item_list);
  item_treeview->append_column(_("Name"), item_columns.name);
  item_treeview->append_column(_("Attributes"), item_columns.attributes);

  item_treeview->get_selection()->signal_changed().connect(method(on_item_selection_changed));
  fill_bag ();
}

void BackpackEditorDialog::hide()
{
  dialog->hide();
}

bool BackpackEditorDialog::run()
{
  update_buttons ();
  dialog->show_all();
  dialog->run ();
  return d_changed;
}

void BackpackEditorDialog::on_item_selection_changed()
{
  update_buttons ();
}

void BackpackEditorDialog::on_remove_item_clicked()
{
  Gtk::TreeIter i = item_treeview->get_selection()->get_selected();
  if (i)
    {
      Item *item = (*i)[item_columns.item];
      backpack->removeFromBackpack(item);
      item_list->erase(item_treeview->get_selection()->get_selected());
      on_item_selection_changed();
      d_changed = true;
    }
}

void BackpackEditorDialog::on_add_item_clicked()
{
  SelectItemDialog d(*dialog);
  d.run();
  guint32 id = 0;
  const ItemProto *itemproto = d.get_selected_item(id);
  if (itemproto)
    {
      Item *item = new Item(*itemproto, id);
      backpack->addToBackpack(item);
      add_item(item);
      on_item_selection_changed();
      d_changed = true;
    }
}

void BackpackEditorDialog::add_item(Item *item)
{
  Gtk::TreeIter i = item_list->append();
  (*i)[item_columns.name] = item->getName();
  (*i)[item_columns.attributes] = item->getBonusDescription();
  (*i)[item_columns.item] = item;
}

void BackpackEditorDialog::fill_bag()
{
  item_list->clear();
  for (Backpack::iterator i = backpack->begin(); i != backpack->end(); ++i)
    add_item(*i);
  return;
}

void BackpackEditorDialog::update_buttons ()
{
  Glib::RefPtr<Gtk::TreeSelection> selection =
    item_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();
  if (iterrow)
    {
      remove_button->set_sensitive (true);
      edit_button->set_sensitive (true);
    }
  else
    {
      remove_button->set_sensitive (false);
      edit_button->set_sensitive (false);
    }
}

void BackpackEditorDialog::on_edit_item_clicked()
{
  Gtk::TreeIter i = item_treeview->get_selection()->get_selected();
  if (i)
    {
      Item *item = (*i)[item_columns.item];
      ItemEditorDialog d (*dialog, item);
      if (d.run ())
        d_changed = true;
    }
}

