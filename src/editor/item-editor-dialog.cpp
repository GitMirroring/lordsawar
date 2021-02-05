//  Copyright (C) 2020, 2021 Ben Asselstine
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
#include "item-editor-actions.h"

#define method(x) sigc::mem_fun(*this, &ItemEditorDialog::x)

ItemEditorDialog::ItemEditorDialog(Gtk::Window &parent, Item *item)
 : LwEditorDialog(parent, "item-editor-dialog.ui")
{
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));
  d_changed = false;
  d_item = item;

  xml->get_widget("name_entry", name_entry);
  umgr->addCursor (name_entry);
  xml->get_widget("bonus_label", bonus_label);
  xml->get_widget("uses_spinbutton", uses_spinbutton);
  xml->get_widget("undo_button", undo_button);
  undo_button->signal_activate ().connect (method (on_undo_activated));
  xml->get_widget("redo_button", redo_button);
  redo_button->signal_activate ().connect (method (on_redo_activated));
  connect_signals ();
  update ();
}

ItemEditorDialog::~ItemEditorDialog()
{
  delete umgr;
}

void ItemEditorDialog::on_name_changed ()
{
  umgr->add (new ItemEditorAction_Name (d_item->getName (), umgr, name_entry));
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

void ItemEditorDialog::on_uses_text_changed()
{
  umgr->add (new ItemEditorAction_Uses (d_item->getNumberOfUsesLeft ()));
  uses_spinbutton->set_value(atoi(uses_spinbutton->get_text().c_str()));
  on_uses_changed();
}

void ItemEditorDialog::on_uses_changed ()
{
  d_changed = true;
  d_item->setNumberOfUsesLeft (uses_spinbutton->get_value ());
}

void ItemEditorDialog::on_undo_activated ()
{
  umgr->undo ();
  if (umgr->undoEmpty ())
    d_changed = false;
  update ();
}

void ItemEditorDialog::on_redo_activated ()
{
  umgr->redo ();
  d_changed = true;
  update ();
}

void ItemEditorDialog::connect_signals ()
{
  umgr->connect_signals ();
  connections.push_back
    (uses_spinbutton->signal_insert_text().connect
     (sigc::hide(sigc::hide(method(on_uses_text_changed)))));
  connections.push_back
    (name_entry->signal_changed().connect(method(on_name_changed)));
}

void ItemEditorDialog::disconnect_signals ()
{
  umgr->disconnect_signals ();
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}

void ItemEditorDialog::update ()
{
  disconnect_signals ();
  name_entry->set_text (d_item->getName ());
  bonus_label->set_text (d_item->getBonusDescription ());
  uses_spinbutton->set_value (d_item->getNumberOfUsesLeft ());
  uses_spinbutton->set_sensitive (d_item->isUsable ());
  umgr->setCursors ();
  connect_signals ();
}

UndoAction *ItemEditorDialog::executeAction (UndoAction *action2)
{
  ItemEditorAction *action = dynamic_cast<ItemEditorAction*>(action2);
  UndoAction *out = NULL;

  switch (action->getType ())
    {
      case ItemEditorAction::USES:
          {
            ItemEditorAction_Uses *a =
              dynamic_cast<ItemEditorAction_Uses*>(action);
            out = new ItemEditorAction_Uses (d_item->getNumberOfUsesLeft ());

            d_item->setNumberOfUsesLeft (a->getUses ());
          }
        break;
      case ItemEditorAction::NAME:
          {
            ItemEditorAction_Name *a =
              dynamic_cast<ItemEditorAction_Name*>(action);
            out = new ItemEditorAction_Name (d_item->getName (), umgr,
                                             name_entry);

            d_item->setName (a->getName ());
          } 
        break;
    }
  return out;
}
