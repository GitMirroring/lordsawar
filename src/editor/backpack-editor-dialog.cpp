//  Copyright (C) 2009, 2011, 2014, 2020, 2021 Ben Asselstine
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
#include "backpack-editor-actions.h"

#define method(x) sigc::mem_fun(*this, &BackpackEditorDialog::x)

BackpackEditorDialog::BackpackEditorDialog(Gtk::Window &parent, Backpack *pack)
 : LwEditorDialog(parent, "backpack-editor-dialog.ui")
{
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));
  d_changed = false;
  backpack = pack;

  xml->get_widget("remove_button", remove_button);
  xml->get_widget("edit_button", edit_button);
  xml->get_widget("add_button", add_button);
  xml->get_widget("undo_button", undo_button);
  undo_button->signal_activate ().connect (method (on_undo_activated));
  xml->get_widget("redo_button", redo_button);
  redo_button->signal_activate ().connect (method (on_redo_activated));
  remove_button->signal_clicked().connect(method(on_remove_item_clicked));
  add_button->signal_clicked().connect(method(on_add_item_clicked));
  edit_button->signal_clicked().connect(method(on_edit_item_clicked));

  item_list = Gtk::ListStore::create(item_columns);
  xml->get_widget("treeview", item_treeview);
  item_treeview->set_model(item_list);
  item_treeview->append_column(_("Name"), item_columns.name);
  item_treeview->append_column(_("Attributes"), item_columns.attributes);

  fill_bag ();
  connect_signals ();
  update ();
}

BackpackEditorDialog::~BackpackEditorDialog()
{
  delete umgr;
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
      umgr->add (new BackpackEditorAction_Remove (backpack));
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
      umgr->add (new BackpackEditorAction_Add (backpack));
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

int BackpackEditorDialog::getCurIndex ()
{
  int idx = -1;
  Gtk::TreeIter i = item_treeview->get_selection()->get_selected();
  if (i)
    {
      auto path = item_treeview->get_model ()->get_path (i);
      idx = atoi (path.to_string ().c_str ());
    }
  return idx;
}
Item *BackpackEditorDialog::getCurItem ()
{
  Gtk::TreeIter i = item_treeview->get_selection()->get_selected();
  if (i)
    {
      Item *item = (*i)[item_columns.item];
      return item;
    }
  return NULL;
}

void BackpackEditorDialog::on_edit_item_clicked()
{
  Item *item = getCurItem ();
  if (item)
    {
      BackpackEditorAction_Edit *action =
        new BackpackEditorAction_Edit (getCurIndex (), item);
      ItemEditorDialog d (*dialog, item);
      if (d.run ())
        {
          umgr->add (action);
          d_changed = true;
        }
      else
        delete action;
    }
  update ();
}

void BackpackEditorDialog::on_undo_activated ()
{
  umgr->undo ();
  if (umgr->undoEmpty ())
    d_changed = false;
  update ();
}

void BackpackEditorDialog::on_redo_activated ()
{
  umgr->redo ();
  d_changed = true;
  update ();
}

void BackpackEditorDialog::connect_signals ()
{
  umgr->connect_signals ();
  connections.push_back
    (item_treeview->get_selection()->signal_changed().connect
     (method(on_item_selection_changed)));
}

void BackpackEditorDialog::disconnect_signals ()
{
  umgr->disconnect_signals ();
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}

void BackpackEditorDialog::update ()
{
  disconnect_signals ();
  update_buttons ();
  auto i = backpack->begin ();
  for (Gtk::TreeIter j = item_list->children().begin(),
       jend = item_list->children().end(); j != jend; ++j, ++i)
    {
      (*j)[item_columns.item] = *i;
      (*j)[item_columns.name] = (*i)->getName ();
      (*j)[item_columns.attributes] = (*i)->getBonusDescription();
    }
  connect_signals ();
}

Item* BackpackEditorDialog::getItemByIndex (BackpackEditorAction_Index *a)
{
  auto path = Gtk::TreePath (String::ucompose ("%1", a->getIndex ()));
  auto iterrow = item_treeview->get_model ()->get_iter (path);
  Gtk::TreeModel::Row row = *iterrow;
  Item *item = row[item_columns.item];
  return item;
}

UndoAction *BackpackEditorDialog::executeAction (UndoAction *action2)
{
  BackpackEditorAction *action = dynamic_cast<BackpackEditorAction*>(action2);
  UndoAction *out = NULL;

  switch (action->getType ())
    {
      case BackpackEditorAction::ADD:
          {
            BackpackEditorAction_Add *a =
              dynamic_cast<BackpackEditorAction_Add*>(action);
            out = new BackpackEditorAction_Add (backpack);
            backpack->removeAllFromBackpack ();
            backpack->add (a->getBackpack ());
            fill_bag ();
          }
        break;
      case BackpackEditorAction::REMOVE:
          {
            BackpackEditorAction_Remove *a =
              dynamic_cast<BackpackEditorAction_Remove*>(action);
            out = new BackpackEditorAction_Remove (backpack);
            backpack->removeAllFromBackpack ();
            backpack->add (a->getBackpack ());
            fill_bag ();
          }
        break;
      case BackpackEditorAction::EDIT:
          {
            BackpackEditorAction_Edit *a =
              dynamic_cast<BackpackEditorAction_Edit*>(action);
            out = new BackpackEditorAction_Edit (getCurIndex (),
                                                 getCurItem ());
            Item *item = getItemByIndex (a);
            std::replace (backpack->begin (), backpack->end (), item,
                          new Item (*a->getItem ()));
            delete item;
          }
        break;
    }
  return out;
}
