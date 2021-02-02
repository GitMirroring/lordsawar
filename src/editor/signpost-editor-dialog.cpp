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

#include "signpost-editor-dialog.h"

#include "ucompose.hpp"
#include "defs.h"
#include "CreateScenarioRandomize.h"
#include "signpost.h"
#include "rnd.h"
#include "signpost-editor-actions.h"

#define method(x) sigc::mem_fun(*this, &SignpostEditorDialog::x)

SignpostEditorDialog::SignpostEditorDialog(Gtk::Window &parent, Signpost *s, CreateScenarioRandomize *randomizer)
 : LwEditorDialog(parent, "signpost-editor-dialog.ui")
{
  d_orig_message = s->getName ();
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));
  d_randomizer = randomizer;
  signpost = s;
  d_changed = false;

  xml->get_widget("sign_textview", sign_textview);
  sign_textview->get_buffer()->set_text(s->getName());
  umgr->addCursor (sign_textview);
  xml->get_widget("randomize_button", randomize_button);
  randomize_button->signal_clicked().connect(method(on_randomize_clicked));
  xml->get_widget("undo_button", undo_button);
  undo_button->signal_activate().connect(method(on_undo_activated));
  xml->get_widget("redo_button", redo_button);
  redo_button->signal_activate().connect(method(on_redo_activated));
  connect_signals ();
}

SignpostEditorDialog::~SignpostEditorDialog ()
{
  delete umgr;
}

void SignpostEditorDialog::on_sign_changed ()
{
  d_changed = true;
  SignpostEditorAction_Message *action =
    new SignpostEditorAction_Message (signpost->getName (), umgr,
                                      sign_textview);
  umgr->add (action);
  signpost->setName(sign_textview->get_buffer()->get_text());
}

bool SignpostEditorDialog::run()
{
  dialog->run();
  if (d_orig_message == signpost->getName () && d_changed == true)
    d_changed = false;
  return d_changed;
}

void SignpostEditorDialog::on_randomize_clicked()
{
  Glib::ustring existing_name = sign_textview->get_buffer()->get_text();
  bool dynamic = ((Rnd::rand() % d_randomizer->getNumSignposts()) == 0);
  SignpostEditorAction_Message *action =
    new SignpostEditorAction_Message (signpost->getName (), umgr,
                                      sign_textview);
  umgr->add (action);
  if (existing_name == DEFAULT_SIGNPOST)
    {
      if (dynamic)
	  signpost->setName (d_randomizer->getDynamicSignpost(signpost));
      else
	  signpost->setName (d_randomizer->popRandomSignpost());
    }
  else
    {
      if (dynamic)
	  signpost->setName (d_randomizer->getDynamicSignpost(signpost));
      else
        {
          signpost->setName (d_randomizer->popRandomSignpost());
          d_randomizer->pushRandomSignpost(existing_name);
        }
    }
  d_changed = true;
  update ();
}

void SignpostEditorDialog::on_undo_activated ()
{
  umgr->undo ();
  if (umgr->undoEmpty ())
    d_changed = false;
  update ();
  return;
}

void SignpostEditorDialog::on_redo_activated ()
{
  d_changed = true;
  umgr->redo ();
  update ();
}

void SignpostEditorDialog::update ()
{
  disconnect_signals ();
  sign_textview->get_buffer()->set_text(signpost->getName());
  umgr->setCursors ();
  connect_signals ();
}

void SignpostEditorDialog::connect_signals ()
{
  umgr->connect_signals ();
  connections.push_back
    (sign_textview->get_buffer()->signal_changed().connect
     (method(on_sign_changed)));
}

void SignpostEditorDialog::disconnect_signals ()
{
  umgr->disconnect_signals ();
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}

UndoAction *SignpostEditorDialog::executeAction (UndoAction *action2)
{
  SignpostEditorAction *action = dynamic_cast<SignpostEditorAction*>(action2);
  UndoAction *out = NULL;

    switch (action->getType ())
      {
      case SignpostEditorAction::MESSAGE:
          {
            SignpostEditorAction_Message *a =
              dynamic_cast<SignpostEditorAction_Message*>(action);
            out = new SignpostEditorAction_Message (signpost->getName (), umgr,
                                                    sign_textview);
            signpost->setName (a->getMessage ());
            break;
          } 
      }
    return out;
}
