//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014, 2020, 2021 Ben Asselstine
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

#include "temple-editor-dialog.h"

#include "ucompose.hpp"
#include "CreateScenarioRandomize.h"
#include "defs.h"
#include "temple.h"
#include "RenamableLocation.h"
#include "temple-editor-actions.h"

#define method(x) sigc::mem_fun(*this, &TempleEditorDialog::x)

TempleEditorDialog::TempleEditorDialog(Gtk::Window &parent, Temple *t, CreateScenarioRandomize *randomizer)
 : LwEditorDialog(parent, "temple-editor-dialog.ui")
{
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));
  d_randomizer = randomizer;
  temple = t;
  d_changed = false;

  xml->get_widget("name_entry", name_entry);
  umgr->addCursor (name_entry);
  xml->get_widget("description_entry", description_entry);
  umgr->addCursor (description_entry);
  xml->get_widget("type_spinbutton", type_spinbutton);
  xml->get_widget("randomize_name_button", randomize_name_button);
  randomize_name_button->signal_clicked().connect
    (sigc::mem_fun(this, &TempleEditorDialog::on_randomize_name_clicked));
  xml->get_widget("undo_button", undo_button);
  undo_button->signal_activate ().connect (method (on_undo_activated));
  xml->get_widget("redo_button", redo_button);
  redo_button->signal_activate ().connect (method (on_redo_activated));
  connect_signals ();
  update ();
}

TempleEditorDialog::~TempleEditorDialog()
{
  delete umgr;
}

void TempleEditorDialog::on_type_changed ()
{
  if (type_spinbutton->get_value() >= TEMPLE_TYPES)
    type_spinbutton->set_value(TEMPLE_TYPES - 1);
  else
    temple->setType(int(type_spinbutton->get_value()));
  d_changed = true;
}

void TempleEditorDialog::on_type_text_changed ()
{
  umgr->add (new TempleEditorAction_Type (Temple::Type (temple->getType ())));
  type_spinbutton->set_value(atoi(type_spinbutton->get_text().c_str()));
  on_type_changed();
}

bool TempleEditorDialog::run()
{
  dialog->show_all();
  dialog->run();
  return d_changed;
}

void TempleEditorDialog::on_randomize_name_clicked()
{
  umgr->add (new TempleEditorAction_RandomizeName (temple->getName ()));
  Glib::ustring existing_name = name_entry->get_text();
  if (existing_name == Temple::getDefaultName())
    name_entry->set_text(d_randomizer->popRandomTempleName());
  else
    {
      name_entry->set_text(d_randomizer->popRandomTempleName());
      d_randomizer->pushRandomTempleName(existing_name);
    }
}

void TempleEditorDialog::on_description_changed ()
{
  umgr->add (new TempleEditorAction_Description (temple->getDescription (),
                                                 umgr, description_entry));
  d_changed = true;
  temple->setDescription(description_entry->get_text());
}

void TempleEditorDialog::on_name_changed ()
{
  umgr->add (new TempleEditorAction_Name (temple->getName (), umgr,
                                          name_entry));
  d_changed = true;
  Location *l = temple;
  RenamableLocation *renamable_temple = static_cast<RenamableLocation*>(l);
  renamable_temple->setName(name_entry->get_text());
}

void TempleEditorDialog::on_undo_activated ()
{
  umgr->undo ();
  if (umgr->undoEmpty ())
    d_changed = false;
  update ();
}

void TempleEditorDialog::on_redo_activated ()
{
  umgr->redo ();
  d_changed = true;
  update ();
}

void TempleEditorDialog::connect_signals ()
{
  umgr->connect_signals ();
  connections.push_back
    (name_entry->signal_changed().connect (method(on_name_changed)));
  connections.push_back
    (description_entry->signal_changed().connect
     (method(on_description_changed)));
  connections.push_back
    (type_spinbutton->signal_insert_text().connect
     (sigc::hide(sigc::hide(method(on_type_text_changed)))));
}

void TempleEditorDialog::disconnect_signals ()
{
  umgr->disconnect_signals ();
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}

void TempleEditorDialog::update ()
{
  disconnect_signals ();
  name_entry->set_text(temple->getName());
  description_entry->set_text(temple->getDescription());
  type_spinbutton->set_value(temple->getType());
  umgr->setCursors ();
  connect_signals ();
}

UndoAction *TempleEditorDialog::executeAction (UndoAction *action2)
{
  TempleEditorAction *action = dynamic_cast<TempleEditorAction*>(action2);
  UndoAction *out = NULL;

  switch (action->getType ())
    {
      case TempleEditorAction::NAME:
          {
            TempleEditorAction_Name *a =
              dynamic_cast<TempleEditorAction_Name*>(action);
            out = new TempleEditorAction_Name (temple->getName (), umgr,
                                               name_entry);
            temple->setName (a->getName ());
          } 
        break;
      case TempleEditorAction::RANDOMIZE_NAME:
          {
            TempleEditorAction_RandomizeName *a =
              dynamic_cast<TempleEditorAction_RandomizeName*>(action);
            out = new TempleEditorAction_RandomizeName (temple->getName ());
            temple->setName (a->getName ());
          }
        break;
      case TempleEditorAction::DESCRIPTION:
          {
            TempleEditorAction_Description *a =
              dynamic_cast<TempleEditorAction_Description*>(action);
            out = new TempleEditorAction_Description
              (temple->getDescription (), umgr, description_entry);
            temple->setDescription (a->getDescription ());
          } 
        break;
      case TempleEditorAction::TYPE:
          {
            TempleEditorAction_Type *a =
              dynamic_cast<TempleEditorAction_Type*>(action);
            out = new TempleEditorAction_Type
              (Temple::Type (temple->getType ()));
            temple->setType (a->getTempleType ());
          }
        break;
    }
  return out;
}
