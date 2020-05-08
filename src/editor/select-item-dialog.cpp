//  Copyright (C) 2008, 2009, 2011, 2014, 2020 Ben Asselstine
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
#include <assert.h>

#include "select-item-dialog.h"

#include "ucompose.hpp"
#include "defs.h"
#include "Item.h"
#include "Itemlist.h"

SelectItemDialog::SelectItemDialog(Gtk::Window &parent, bool clear)
 : LwEditorDialog(parent, "select-item-dialog.ui")
{
  d_clear = clear;
  selected_item = 0;

  xml->get_widget("select_button", select_button);
  xml->get_widget("clear_button", clear_button);

  xml->get_widget("items_treeview", items_treeview);
  items_list = Gtk::ListStore::create(items_columns);
  items_treeview->set_model(items_list);
  items_treeview->append_column("", items_columns.name);
  items_treeview->append_column("", items_columns.attributes);
  items_treeview->set_headers_visible(false);

  Itemlist *itemlist = Itemlist::getInstance();
  for (Itemlist::iterator iter = itemlist->begin();
       iter != itemlist->end(); ++iter)
    addItemProto((*iter).second);

  guint32 max = itemlist->size();
  if (max)
    {
      Gtk::TreeModel::Row row;
      row = items_treeview->get_model()->children()[0];
      if(row)
        items_treeview->get_selection()->select(row);
    }
}

void SelectItemDialog::addItemProto(ItemProto *item)
{
  Gtk::TreeIter i = items_list->append();
  (*i)[items_columns.name] = item->getName();
  (*i)[items_columns.attributes] = item->getBonusDescription();
  (*i)[items_columns.item] = item;
}

void SelectItemDialog::run()
{
  dialog->show_all();
  clear_button->set_visible (d_clear);
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
          selected_item_type_id = 0;
          for (Itemlist::iterator iter = Itemlist::getInstance()->begin();
               iter != Itemlist::getInstance()->end(); ++iter)
            {
              if ((*iter).second == selected_item)
                break;
              selected_item_type_id++;
            }
        }
    }
}

