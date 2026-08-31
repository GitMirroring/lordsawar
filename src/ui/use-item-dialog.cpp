//  Copyright (C) 2010, 2014 Ben Asselstine
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
#include "use-item-dialog.h"
#include "ucompose.hpp"
#include "Item.h"

UseItemDialog::UseItemDialog(Gtk::Window &parent, std::list<Item*> items)
 : LwDialog(parent, "use-item-dialog.ui")
{
  selected_item = 0;

  xml->get_widget("select_button", select_button);
  xml->get_widget("items_treeview", items_treeview);
  items_list = Gtk::ListStore::create(items_columns);
  items_treeview->set_model(items_list);
  items_treeview->append_column("", items_columns.name);
  items_treeview->set_headers_visible(false);

  for (std::list<Item*>::iterator iter = items.begin();
       iter != items.end(); ++iter)
    addItem(*iter);

  guint32 max = items.size();
  if (max)
    {
      Gtk::TreeModel::Row row;
      row = items_treeview->get_model()->children()[0];
      if(row)
        items_treeview->get_selection()->select(row);
    }
}

void UseItemDialog::addItem(Item *item)
{
  Gtk::TreeIter i = items_list->append();
  (*i)[items_columns.name] = item->getName();
  (*i)[items_columns.attributes] = item->getBonusDescription();
  (*i)[items_columns.item] = item;
}

void UseItemDialog::run()
{
  dialog->show_all();
  int response = dialog->run();

  if (response != Gtk::RESPONSE_ACCEPT)
    selected_item = 0;
  else
    {
      Glib::RefPtr<Gtk::TreeSelection> selection = 
        items_treeview->get_selection();
      Gtk::TreeModel::iterator iterrow = selection->get_selected();

      if (iterrow) 
        {
          Gtk::TreeModel::Row row = *iterrow;
          selected_item = row[items_columns.item];
        }
    }
}

