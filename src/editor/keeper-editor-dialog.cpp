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

#include "keeper-editor-dialog.h"

#include "army-chooser-button.h"
#include "ucompose.hpp"
#include "defs.h"
#include "keeper.h"
#include "playerlist.h"
#include "select-army-dialog.h"
#include "armyproto.h"
#include "CreateScenarioRandomize.h"
#include "keeper-editor-actions.h"

#define method(x) sigc::mem_fun(*this, &KeeperEditorDialog::x)

KeeperEditorDialog::KeeperEditorDialog(Gtk::Window &parent, Keeper *k,
                                       Vector<int> pos,
                                       CreateScenarioRandomize *r)
 : LwEditorDialog(parent, "keeper-dialog.ui"), d_changed (false), d_pos (pos),
    d_randomizer (r)
{
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));
  keeper_button =
    new ArmyChooserButton (parent, xml, "keeper_button",
                           Playerlist::getInstance ()->getNeutral (),
                           SelectArmyDialog::SELECT_RUIN_DEFENDER);
  xml->get_widget ("name_entry", name_entry);
  umgr->addCursor (name_entry);
  xml->get_widget("randomize_button", randomize_button);
  xml->get_widget("undo_button", undo_button);
  undo_button->signal_activate ().connect (method (on_undo_activated));
  xml->get_widget("redo_button", redo_button);
  redo_button->signal_activate ().connect (method (on_redo_activated));

  if (k)
    {
      d_keeper = new Keeper (*k);
      if (d_keeper->getStack ())
        keeper_button->select (d_keeper->getStack ()->front()->getTypeId ());
    }
  else
    d_keeper = new Keeper (NULL, pos);

  randomize_button->signal_clicked().connect (method(on_randomize_clicked));

  connect_signals ();
  update ();
}

void KeeperEditorDialog::on_keeper_selected (const ArmyProto *a)
{
  umgr->add (new KeeperEditorAction_Keeper (d_keeper));
  d_changed = true;
  if (a)
    {
      d_keeper->clearStack ();
      ArmyProto *defender = new ArmyProto(*a);
      d_keeper->add (defender, d_pos);
      update ();
    }
  else
    {
      d_keeper->clearStack ();
      d_keeper->setName ("");
      update ();
    }
}

KeeperEditorDialog::~KeeperEditorDialog()
{
  delete keeper_button;
  delete umgr;
}

void KeeperEditorDialog::fill_in_keeper_info()
{
  name_entry->set_text (d_keeper->getName ());
}

bool KeeperEditorDialog::run()
{
  dialog->show_all();
  int response = dialog->run();

  if (response != Gtk::RESPONSE_ACCEPT)
    {
      if (d_keeper)
	delete d_keeper;
      d_keeper = NULL;
      d_changed = true;
    }
  return d_changed;
}

void KeeperEditorDialog::on_randomize_clicked()
{
  Keeper *k = d_randomizer->getRandomRuinKeeper (d_pos);
  if (!k)
    return;
  umgr->add (new KeeperEditorAction_Randomize (d_keeper));
  d_changed = true;

  if (d_keeper)
    delete d_keeper;
  d_keeper = k;
  disconnect_signals ();
  keeper_button->select (k->getStack()->front ()->getTypeId());
  connect_signals ();

  update ();
}

void KeeperEditorDialog::on_name_changed ()
{
  umgr->add (new KeeperEditorAction_Name (d_keeper, umgr, name_entry));
  d_changed = true;
  d_keeper->setName (String::utrim (name_entry->get_text ()));
}

void KeeperEditorDialog::on_undo_activated ()
{
  umgr->undo ();
  if (umgr->undoEmpty ())
    d_changed = false;
  update ();
}

void KeeperEditorDialog::on_redo_activated ()
{
  d_changed = true;
  umgr->redo ();
  update ();
}

void KeeperEditorDialog::update ()
{
  disconnect_signals ();
  fill_in_keeper_info();
  if (d_keeper->getStack())
    keeper_button->select (d_keeper->getStack()->front ()->getTypeId());
  else
    keeper_button->clear_selected_army ();
  umgr->setCursors ();
  connect_signals ();
}

void KeeperEditorDialog::connect_signals ()
{
  umgr->connect_signals ();
  connections.push_back
    (name_entry->signal_changed().connect (method(on_name_changed)));
  connections.push_back
    (keeper_button->army_selected.connect (method (on_keeper_selected)));
}

void KeeperEditorDialog::disconnect_signals ()
{
  umgr->disconnect_signals ();
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}

UndoAction*
KeeperEditorDialog::executeAction (UndoAction *action2)
{
  KeeperEditorAction *action = dynamic_cast<KeeperEditorAction*>(action2);
  UndoAction *out = NULL;

    switch (action->getType ())
      {
      case KeeperEditorAction::NAME:
          {
            KeeperEditorAction_Name *a =
              dynamic_cast<KeeperEditorAction_Name*>(action);
            out = new KeeperEditorAction_Name (d_keeper, umgr, name_entry);
            d_keeper->setName (a->getKeeper ()->getName ());
          }
        break;
      case KeeperEditorAction::RANDOMIZE:
          {
            KeeperEditorAction_Randomize *a =
              dynamic_cast<KeeperEditorAction_Randomize*>(action);
            out = new KeeperEditorAction_Randomize (d_keeper);
            delete d_keeper;
            d_keeper = new Keeper (*a->getKeeper ());
          }
        break;
      case KeeperEditorAction::KEEPER:
          {
            KeeperEditorAction_Keeper *a =
              dynamic_cast<KeeperEditorAction_Keeper*>(action);
            out = new KeeperEditorAction_Keeper (d_keeper);
            delete d_keeper;
            d_keeper = new Keeper (*a->getKeeper ());
          }
        break;
      }
    return out;
}
